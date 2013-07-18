/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
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

#include "boost/tuple/tuple.hpp"

using namespace sim_mob::long_term;
using std::list;
using std::endl;
using sim_mob::Math;

#define TIME_ON_MARKET 2
#define TIME_INTERVAL 7

namespace {

    double ExpectationFunction(double x, const boost::tuple<double, double, double>& params) {
        double v = params.get<0>();
        double theta = params.get<1>();
        double alpha = params.get<2>();
        double price = x; // last expectation (V(t+1))
        // Calculates the bids distribution using F(X) = X/Price where F(V(t+1)) = V(t+1)/Price
        double bidsDistribution = v / price;
        // Calculates the probability of not having any bid greater than v.
        double priceProb = pow(Math::E, -((theta / pow(price, alpha)) * (1 - bidsDistribution)));
        // Calculates expected maximum bid.
        double p1 = pow(price, 2 * alpha + 1);
        double p2 = (price * (theta * pow(price, -alpha) - 1));
        double p3 = pow(Math::E, (theta * pow(price, -alpha)* (bidsDistribution - 1)));
        double p4 = (price - theta * v * pow(price, -alpha));
        double expectedMaxBid = (p1 * (p2 + p3 * p4)) / (theta * theta);
        return (v * priceProb + (1 - priceProb) * expectedMaxBid) - (0.01f * price);
    }

    double CalculateHedonicPrice(const Unit& unit, const SellerParams& params) {
        return unit.GetRent() +
                (unit.GetRent() * params.GetUnitRentWeight() +
                unit.GetTypeId() * params.GetUnitTypeWeight() +
                unit.GetStorey() * params.GetUnitStoreyWeight() +
                unit.GetArea() * params.GetUnitAreaWeight());
    }

    int currentExpectationIndex = 0;
}

HouseholdSellerRole::HouseholdSellerRole(HouseholdAgent* parent, Household* hh,
        const SellerParams& params, HousingMarket* market)
: LT_AgentRole(parent), market(market), hh(hh), currentTime(0, 0),
hasUnitsToSale(true), params(params) {
}

HouseholdSellerRole::~HouseholdSellerRole() {
    maxBidsOfDay.clear();
}

void HouseholdSellerRole::Update(timeslice now) {
    if (hasUnitsToSale) {
        list<Unit*> units;
        GetParent()->GetUnits(units);
        for (list<Unit*>::iterator itr = units.begin(); itr != units.end();
                itr++) {
            // Decides to put the house on market.
            if ((*itr)->IsAvailable()) {
                double hedonicPrice = CalculateHedonicPrice(*(*itr), params);
                (*itr)->SetHedonicPrice(hedonicPrice);
                (*itr)->SetAskingPrice(hedonicPrice);
                CalculateUnitExpectations(*(*itr));
                // Put the asking price for given time.
                market->AddUnit((*itr));
            }
        }
        hasUnitsToSale = false;
    }
    if (now.ms() > currentTime.ms()) {
        // Day has changed we need to notify the last day winners.
        NotifyWinnerBidders();

    }
    // Verify if is time to adjust prices for units.
    if (TIME_INTERVAL > 0) {
        int index = floor(now.ms() / TIME_INTERVAL);
        if (index > currentExpectationIndex) {
            currentExpectationIndex = index;
            AdjustNotSelledUnits();
        }
    }
    //update current time.
    currentTime = now;
}

void HouseholdSellerRole::HandleMessage(MessageType type, MessageReceiver& sender,
        const Message& message) {

    switch (type) {
        case LTMID_BID:// Bid received 
        {
            BidMessage* msg = MSG_CAST(BidMessage, message);
            Unit* unit = GetParent()->GetUnitById(msg->GetBid().GetUnitId());
            LogOut("Seller: [" << GetParent()->getId() <<
                    "] received a bid: " << msg->GetBid() <<
                    " at day: " << currentTime.ms() << endl);
            bool decision = false;
            ExpectationEntry entry;
            if (unit && unit->IsAvailable() && GetCurrentExpectation(*unit, entry)) {
                //verify if is the bid satisfies the asking price.
                decision = Decide(msg->GetBid(), entry);
                if (decision) {
                    //get the maximum bid of the day
                    Bids::iterator bidItr = maxBidsOfDay.find(unit->GetId());
                    Bid* maxBidOfDay = nullptr;
                    if (bidItr != maxBidsOfDay.end()) {
                        maxBidOfDay = &(bidItr->second);
                    }

                    if (!maxBidOfDay) {
                        maxBidsOfDay.insert(BidEntry(unit->GetId(),
                                msg->GetBid()));
                    } else if (maxBidOfDay->GetValue() < msg->GetBid().GetValue()) {
                        // bid is higher than the current one of the day.
                        // it is necessary to notify the old max bidder
                        // that his bid was not accepted.
                        //reply to sender.
                        maxBidOfDay->GetBidder()->Post(LTMID_BID_RSP, GetParent(),
                                new BidMessage(Bid(*maxBidOfDay), BETTER_OFFER));
                        maxBidsOfDay.erase(unit->GetId());
                        //update the new bid and bidder.
                        maxBidsOfDay.insert(BidEntry(unit->GetId(), msg->GetBid()));
                    } else {
                        sender.Post(LTMID_BID_RSP, GetParent(),
                                new BidMessage(Bid(msg->GetBid()), BETTER_OFFER));
                    }
                } else {
                    sender.Post(LTMID_BID_RSP, GetParent(),
                            new BidMessage(Bid(msg->GetBid()), NOT_ACCEPTED));
                }
            } else {
                // Sellers is not the owner of the unit or unit is not available.
                sender.Post(LTMID_BID_RSP, GetParent(),
                        new BidMessage(Bid(msg->GetBid()), NOT_AVAILABLE));
            }
            Statistics::Increment(Statistics::N_BIDS);
            break;
        }
        default:break;
    }
}

bool HouseholdSellerRole::Decide(const Bid& bid, const ExpectationEntry& entry) {
    return bid.GetValue() > entry.expectation;
}

void HouseholdSellerRole::AdjustNotSelledUnits() {
    list<Unit*> units;
    GetParent()->GetUnits(units);
    for (list<Unit*>::iterator itr = units.begin(); itr != units.end(); itr++) {
        if ((*itr)->IsAvailable()) {
            AdjustUnitParams((**itr));
        }
    }
}

void HouseholdSellerRole::AdjustUnitParams(Unit& unit) {
    ExpectationEntry entry;
    if (GetCurrentExpectation(unit, entry)) {
        unit.SetAskingPrice(entry.price);
    }
}

void HouseholdSellerRole::NotifyWinnerBidders() {
    for (Bids::iterator itr = maxBidsOfDay.begin(); itr != maxBidsOfDay.end();
            itr++) {
        Bid* maxBidOfDay = &(itr->second);
        maxBidOfDay->GetBidder()->Post(LTMID_BID_RSP, GetParent(),
                new BidMessage(Bid(*maxBidOfDay), ACCEPTED));
        Unit* unit = GetParent()->GetUnitById(maxBidOfDay->GetUnitId());
        if (unit && unit->IsAvailable()) {
            unit->SetAvailable(false);
            unit = GetParent()->RemoveUnit(unit->GetId());
            // clean all expectations for the unit.
            unitExpectations.erase(unit->GetId());
        }
    }
    // notify winners.
    maxBidsOfDay.clear();
}

void HouseholdSellerRole::CalculateUnitExpectations(const Unit& unit) {
    ExpectationList expectationList;
    double price = 20;
    double expectation = 4;
    double theta = params.GetExpectedEvents(); // 1.0f;
    double alpha = params.GetPriceImportance(); // 2.0f;
    for (int i = 0; i < TIME_ON_MARKET; i++) {
        ExpectationEntry entry;
        entry.price = Math::FindMaxArg(ExpectationFunction,
                price, boost::tuple<double, double, double>(expectation, theta, alpha),
                .001f, 100000);
        entry.expectation = ExpectationFunction(entry.price,
                boost::tuple<double, double, double>(expectation, theta, alpha));
        expectation = entry.expectation;
        expectationList.push_back(entry);
        LogOut("Expectation on: [" << i << std::setprecision(15) <<
                "] Unit: [" << unit.GetId() <<
                "] expectation: [" << entry.expectation <<
                "] price: [" << entry.price <<
                "] theta: [" << theta <<
                "] alpha: [" << alpha <<
                "]" << endl);
    }
    unitExpectations.erase(unit.GetId());
    unitExpectations.insert(ExpectationMapEntry(unit.GetId(), expectationList));
}

bool HouseholdSellerRole::GetCurrentExpectation(const Unit& unit, ExpectationEntry& outEntry) {
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

