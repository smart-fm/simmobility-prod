//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdSellerRole.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 16, 2013, 5:13 PM
 */
#include <cmath>
#include "HouseholdSellerRole.hpp"
#include "util/Statistics.hpp"
#include "util/Math.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "model/HM_Model.hpp"
#include "message/MessageBus.hpp"
#include "model/lua/LuaProvider.hpp"
#include "message/LT_Message.hpp"
#include "core/DataManager.hpp"


using namespace sim_mob::long_term;
using namespace sim_mob::messaging;
using std::vector;
using std::endl;
using sim_mob::Math;

namespace {
    const int TIME_ON_MARKET = 10;
    const int TIME_INTERVAL = 7;

    const std::string LOG_BID_RECEIVED = "Agent: [%1%] received a bid: [%2%] at [%3%].";

    inline void printBid(const HouseholdAgent& agent, const Bid& bid,
            const timeslice& now) {
        boost::format fmtr = boost::format(LOG_BID_RECEIVED) % agent.getId() % bid % now.ms();
        PrintOut(fmtr.str() << endl);
    }

    /**
     * Decides over a given bid for a given expectation.
     * @param bid given by the bidder.
     * @return true if accepts the bid or false otherwise.
     */
    inline bool decide(const Bid& bid, const ExpectationEntry& entry) {
        return bid.getValue() > entry.expectation;
    }

    inline void replyBid(const Bid& bid, const BidResponse& response) {
        MessageBus::PostMessage(bid.getBidder(), LTMID_BID_RSP,
                MessageBus::MessagePtr(new BidMessage(bid, response)));
    }
}

HouseholdSellerRole::HouseholdSellerRole(HouseholdAgent* parent)
: LT_AgentRole(parent), currentTime(0, 0), hasUnitsToSale(true),
currentExpectationIndex(0), startSellingTime(0), selling(false) {

}

HouseholdSellerRole::~HouseholdSellerRole() {
    maxBidsOfDay.clear();
}

void HouseholdSellerRole::update(timeslice now) {
    if (selling){
        if (now.ms() > currentTime.ms()) {
            // Day has changed we need to notify the last day winners.
            notifyWinnerBidders();

        }
        // Verify if is time to adjust prices for units.
        if (TIME_INTERVAL > 0) {
            int index = floor(abs(now.ms() - startSellingTime) / TIME_INTERVAL);
            if (index > currentExpectationIndex) {
                currentExpectationIndex = index;
                adjustNotSelledUnits();
            }
        }
    }
    if (hasUnitsToSale) {
        const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
        HousingMarket* market = getParent()->getMarket();
        const vector<BigSerial>& unitIds = getParent()->getUnitIds();
        //get values from parent.
        const Unit* unit = nullptr;
        for (vector<BigSerial>::const_iterator itr = unitIds.begin();
                itr != unitIds.end(); itr++) {
            // Decides to put the house on market.
            BigSerial unitId = *itr;
            unit = DataManagerSingleton::getInstance().getUnitById(unitId);
            double hedonicPrice = luaModel.calculateHedonicPrice(*unit);
            calculateUnitExpectations(*unit);
            market->addEntry(HousingMarket::Entry(getParent(), *unit,
                    hedonicPrice, hedonicPrice));
            selling = true;
            startSellingTime = now.ms();
        }
        hasUnitsToSale = false;
    }
    //update current time.
    currentTime = now;
}

void HouseholdSellerRole::HandleMessage(Message::MessageType type,
        const Message& message) {
    switch (type) {
        case LTMID_BID:// Bid received 
        {
            const BidMessage& msg = MSG_CAST(BidMessage, message);
            BigSerial unitId = msg.getBid().getUnitId();
            bool decision = false;
            ExpectationEntry entry;
            if (getCurrentExpectation(unitId, entry)) {
                //verify if is the bid satisfies the asking price.
                decision = decide(msg.getBid(), entry);
                if (decision) {
                    //get the maximum bid of the day
                    Bids::iterator bidItr = maxBidsOfDay.find(unitId);
                    Bid* maxBidOfDay = nullptr;
                    if (bidItr != maxBidsOfDay.end()) {
                        maxBidOfDay = &(bidItr->second);
                    }

                    if (!maxBidOfDay) {
                        maxBidsOfDay.insert(BidEntry(unitId, msg.getBid()));
                    } else if (maxBidOfDay->getValue() < msg.getBid().getValue()) {
                        // bid is higher than the current one of the day.
                        // it is necessary to notify the old max bidder
                        // that his bid was not accepted.
                        //reply to sender.
                        replyBid(*maxBidOfDay, BETTER_OFFER);
                        maxBidsOfDay.erase(unitId);
                        //update the new bid and bidder.
                        maxBidsOfDay.insert(BidEntry(unitId, msg.getBid()));
                    } else {
                        replyBid(msg.getBid(), BETTER_OFFER);
                    }
                } else {
                    replyBid(msg.getBid(), NOT_ACCEPTED);
                }
            } else {
                // Sellers is not the owner of the unit or unit is not available.
                replyBid(msg.getBid(), NOT_AVAILABLE);
            }
            Statistics::increment(Statistics::N_BIDS);
            //print bid.
            printBid(*getParent(), msg.getBid(), currentTime);
            break;
        }
        default:break;
    }
}

void HouseholdSellerRole::adjustNotSelledUnits() {
    HousingMarket* market = getParent()->getMarket();
    const vector<BigSerial>& unitIds = getParent()->getUnitIds();
    const Unit* unit = nullptr;
    const HousingMarket::Entry* unitEntry = nullptr;
    for (vector<BigSerial>::const_iterator itr = unitIds.begin();
            itr != unitIds.end(); itr++) {
        BigSerial unitId = *itr;
        unitEntry = market->getEntryById(unitId);
        unit = DataManagerSingleton::getInstance().getUnitById(unitId);
        if (unitEntry && unit){
            ExpectationEntry entry;
            if (getCurrentExpectation(unitId, entry)) {
                //IMPROVE THIS PLEASE.....
                HousingMarket::Entry updatedEntry(*unitEntry);
                updatedEntry.setAskingPrice(entry.price);
                market->updateEntry(updatedEntry);
            }
        }
    }

void HouseholdSellerRole::notifyWinnerBidders() {
    HousingMarket* market = getParent()->getMarket();
    for (Bids::iterator itr = maxBidsOfDay.begin(); itr != maxBidsOfDay.end();
            itr++) {
        Bid& maxBidOfDay = itr->second;
        replyBid(maxBidOfDay, ACCEPTED);
        market->removeEntry(maxBidOfDay.getUnitId());
        getParent()->removeUnitId(maxBidOfDay.getUnitId());
        unitExpectations.erase(maxBidOfDay.getUnitId());
    }
    // notify winners.
    maxBidsOfDay.clear();
}

void HouseholdSellerRole::calculateUnitExpectations(const Unit& unit) {
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    ExpectationList expectationList;
    luaModel.calulateUnitExpectations(unit, TIME_ON_MARKET, expectationList);
    unitExpectations.erase(unit.getId());
    unitExpectations.insert(ExpectationMapEntry(unit.getId(), expectationList));
    for (int i = 0; i < TIME_ON_MARKET; i++) {
        PrintOut("Seller:[" << getParent()->getId() << "] Price:[" << expectationList[i].price << "] Expectation:[" << expectationList[i].expectation << "]." << endl);
    }
}

bool HouseholdSellerRole::getCurrentExpectation(const BigSerial& unitId, ExpectationEntry& outEntry) {
    ExpectationMap::iterator expectations = unitExpectations.find(unitId);
    if (expectations != unitExpectations.end()) {
        if (currentExpectationIndex < expectations->second.size()) {
            ExpectationEntry& expectation = expectations->second.at(currentExpectationIndex);
            outEntry.expectation = expectation.expectation;
            outEntry.price = expectation.price;
            return true;
        }
    }
    return false;
}

