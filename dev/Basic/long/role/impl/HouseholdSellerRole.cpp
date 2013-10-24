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
#include "message/LT_Message.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "util/Statistics.hpp"
#include "util/Math.hpp"
#include "message/MessageBus.hpp"
#include "boost/tuple/tuple.hpp"
#include "model/lua/LuaProvider.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::messaging;
using std::list;
using std::endl;
using sim_mob::Math;

namespace {
    const int TIME_ON_MARKET = 10;
    const int TIME_INTERVAL = 7;
}

HouseholdSellerRole::HouseholdSellerRole(HouseholdAgent* parent, Household* hh, 
        HousingMarket* market)
: LT_AgentRole(parent), market(market), hh(hh), currentTime(0, 0),
hasUnitsToSale(true), currentExpectationIndex(0) {
}

HouseholdSellerRole::~HouseholdSellerRole() {
    maxBidsOfDay.clear();
}

void HouseholdSellerRole::update(timeslice now) {
    if (hasUnitsToSale) {
        list<Unit*> units;
        getParent()->getUnits(units);
        const HM_LuaModel& model = LuaProvider::getHM_Model();
        for (list<Unit*>::iterator itr = units.begin(); itr != units.end();
                itr++) {
            // Decides to put the house on market.
            if ((*itr)->IsAvailable()) {
                double hedonicPrice = model.calculateHedonicPrice(*(*itr));
                (*itr)->SetHedonicPrice(hedonicPrice);
                (*itr)->SetAskingPrice(hedonicPrice);
                calculateUnitExpectations(*(*itr));
                // Put the asking price for given time.
                market->addUnit((*itr));
            }
        }
        hasUnitsToSale = false;
    }
    if (now.ms() > currentTime.ms()) {
        // Day has changed we need to notify the last day winners.
        notifyWinnerBidders();

    }
    // Verify if is time to adjust prices for units.
    if (TIME_INTERVAL > 0) {
        int index = floor(now.ms() / TIME_INTERVAL);
        if (index > currentExpectationIndex) {
            currentExpectationIndex = index;
            adjustNotSelledUnits();
        }
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
            Unit* unit = getParent()->getUnitById(msg.getBid().getUnitId());
            PrintOut("Seller: [" << getParent()->getId() <<
                    "] received a bid: " << msg.getBid() <<
                    " at day: " << currentTime.ms() << endl);
            bool decision = false;
            ExpectationEntry entry;
            if (unit && unit->IsAvailable() && getCurrentExpectation(*unit, entry)) {
                //verify if is the bid satisfies the asking price.
                decision = decide(msg.getBid(), entry);
                if (decision) {
                    //get the maximum bid of the day
                    Bids::iterator bidItr = maxBidsOfDay.find(unit->GetId());
                    Bid* maxBidOfDay = nullptr;
                    if (bidItr != maxBidsOfDay.end()) {
                        maxBidOfDay = &(bidItr->second);
                    }

                    if (!maxBidOfDay) {
                        maxBidsOfDay.insert(BidEntry(unit->GetId(),
                                msg.getBid()));
                    } else if (maxBidOfDay->getValue() < msg.getBid().getValue()) {
                        // bid is higher than the current one of the day.
                        // it is necessary to notify the old max bidder
                        // that his bid was not accepted.
                        //reply to sender.
                        MessageBus::PostMessage(maxBidOfDay->getBidder(), LTMID_BID_RSP, 
                                MessageBus::MessagePtr(new BidMessage(Bid(*maxBidOfDay), BETTER_OFFER)));
                        maxBidsOfDay.erase(unit->GetId());
                        //update the new bid and bidder.
                        maxBidsOfDay.insert(BidEntry(unit->GetId(), msg.getBid()));
                    } else {
                        MessageBus::PostMessage(msg.getBid().getBidder(), 
                                LTMID_BID_RSP, MessageBus::MessagePtr(new BidMessage(Bid(msg.getBid()), 
                                BETTER_OFFER)));
                    }
                } else {
                    MessageBus::PostMessage(msg.getBid().getBidder(),LTMID_BID_RSP,
                            MessageBus::MessagePtr(new BidMessage(Bid(msg.getBid()), NOT_ACCEPTED)));
                }
            } else {
                // Sellers is not the owner of the unit or unit is not available.
                MessageBus::PostMessage(msg.getBid().getBidder(), LTMID_BID_RSP,
                        MessageBus::MessagePtr(new BidMessage(Bid(msg.getBid()), NOT_AVAILABLE)));
            }
            Statistics::increment(Statistics::N_BIDS);
            break;
        }
        default:break;
    }
}

bool HouseholdSellerRole::decide(const Bid& bid, const ExpectationEntry& entry) {
    return bid.getValue() > entry.expectation;
}

void HouseholdSellerRole::adjustNotSelledUnits() {
    list<Unit*> units;
    getParent()->getUnits(units);
    for (list<Unit*>::iterator itr = units.begin(); itr != units.end(); itr++) {
        if ((*itr)->IsAvailable()) {
            adjustUnitParams((**itr));
        }
    }
}

void HouseholdSellerRole::adjustUnitParams(Unit& unit) {
    ExpectationEntry entry;
    if (getCurrentExpectation(unit, entry)) {
        unit.SetAskingPrice(entry.price);
    }
}

void HouseholdSellerRole::notifyWinnerBidders() {
    for (Bids::iterator itr = maxBidsOfDay.begin(); itr != maxBidsOfDay.end();
            itr++) {
        Bid* maxBidOfDay = &(itr->second);
        MessageBus::PostMessage(maxBidOfDay->getBidder(), LTMID_BID_RSP, 
                MessageBus::MessagePtr(new BidMessage(Bid(*maxBidOfDay), ACCEPTED)));
        
        Unit* unit = getParent()->getUnitById(maxBidOfDay->getUnitId());
        if (unit && unit->IsAvailable()) {
            unit->SetAvailable(false);
            unit = getParent()->removeUnit(unit->GetId());
            // clean all expectations for the unit.
            unitExpectations.erase(unit->GetId());
        }
    }
    // notify winners.
    maxBidsOfDay.clear();
}

void HouseholdSellerRole::calculateUnitExpectations(const Unit& unit) {
    ExpectationList expectationList;
    LuaProvider::getHM_Model().calulateUnitExpectations(unit, TIME_ON_MARKET, expectationList);
    
    unitExpectations.erase(unit.GetId());
    unitExpectations.insert(ExpectationMapEntry(unit.GetId(), expectationList));

    for (int i = 0; i < TIME_ON_MARKET; i++) {
       PrintOut("Seller:["<< hh->GetId() << "] Price:[" << expectationList[i].price << "] Expectation:[" << expectationList[i].expectation << "]." << endl);
    }
}

bool HouseholdSellerRole::getCurrentExpectation(const Unit& unit, ExpectationEntry& outEntry) {
    ExpectationMap::iterator expectations = unitExpectations.find(unit.GetId());
    if (expectations != unitExpectations.end()) {
        if (currentExpectationIndex < expectations->second.size()) {
            ExpectationEntry expectation = expectations->second.at(currentExpectationIndex);
            outEntry.expectation = expectation.expectation;
            outEntry.price = expectation.price;
            return true;
        }
    }
    return false;
}

