//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdBidderRole.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 16, 2013, 5:13 PM
 */

#include <cmath>
#include <boost/format.hpp>
#include "HouseholdBidderRole.hpp"
#include "message/LT_Message.hpp"
#include "event/EventPublisher.hpp"
#include "event/EventManager.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "util/Statistics.hpp"
#include "message/MessageBus.hpp"
#include "model/lua/LuaProvider.hpp"
#include "model/HM_Model.hpp"
#include "entities/commsim/message/base/Message.hpp"
#include "core/AgentsLookup.hpp"
#include "core/DataManager.hpp"

using std::list;
using std::endl;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using boost::format;

namespace {

    /**
     * Send given bid to given owner.
     * @param owner of the unit
     * @param bid to send.
     */
    inline void bid(MessageHandler* owner, const Bid& bid) {
        MessageBus::PostMessage(owner, LTMID_BID,
                MessageBus::MessagePtr(new BidMessage(bid)));
    }
}
HouseholdBidderRole::CurrentBiddingEntry::CurrentBiddingEntry (const HousingMarket::Entry* entry, const double wp) :
entry(entry), wp(wp), tries(0)
{
}

HouseholdBidderRole::CurrentBiddingEntry::~CurrentBiddingEntry() {
    invalidate();
}

const HousingMarket::Entry* HouseholdBidderRole::CurrentBiddingEntry::getEntry() const {
    return entry;
}

double HouseholdBidderRole::CurrentBiddingEntry::getWP() const {
    return wp;
}

long int HouseholdBidderRole::CurrentBiddingEntry::getTries() const {
    return tries;
}

void HouseholdBidderRole::CurrentBiddingEntry::incrementTries(int quantity) {
    tries += quantity;
}

bool HouseholdBidderRole::CurrentBiddingEntry::isValid() const{
    return (entry != nullptr);
}

void HouseholdBidderRole::CurrentBiddingEntry::invalidate(){
    entry = nullptr;
    tries = 0;
    wp = 0;
}
                
HouseholdBidderRole::HouseholdBidderRole(HouseholdAgent* parent)
: LT_AgentRole(parent), waitingForResponse(false),
lastTime(0, 0), bidOnCurrentDay(false) {
}

HouseholdBidderRole::~HouseholdBidderRole() {
}

void HouseholdBidderRole::update(timeslice now) {
    //can bid another house if it is not waiting for any 
    //response and if it not the same day
    if (!waitingForResponse && lastTime.ms() < now.ms()) {
        bidOnCurrentDay = false;
    }

    if (isActive()) {
        if (!waitingForResponse && !bidOnCurrentDay && bidUnit(now)) {
            waitingForResponse = true;
            bidOnCurrentDay = true;
        }
    }
    lastTime = now;
}

void HouseholdBidderRole::HandleMessage(Message::MessageType type,
        const Message& message) {
    switch (type) {
        case LTMID_BID_RSP:// Bid response received 
        {
            const BidMessage& msg = MSG_CAST(BidMessage, message);
            switch (msg.getResponse()) {
                case ACCEPTED:// Bid accepted 
                {
                    getParent()->addUnitId(msg.getBid().getUnitId());
                    setActive(false);
                    biddingEntry.invalidate();
                    Statistics::increment(Statistics::N_ACCEPTED_BIDS);
                    break;
                }
                case NOT_ACCEPTED:
                {
                    biddingEntry.incrementTries();
                    break;
                }
                case BETTER_OFFER:
                case NOT_AVAILABLE:
                {
                    break;
                }
                default:break;
            }
            waitingForResponse = false;
            Statistics::increment(Statistics::N_BID_RESPONSES);
            break;
        }
        default:break;
    }
}

bool HouseholdBidderRole::bidUnit(timeslice now) {
    const Household* household = getParent()->getHousehold();
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    const HM_Model* model = getParent()->getModel();
    
    // Following the new assumptions of the model each household will stick on the 
    // unit where he is bidding until he gets rejected for seller by NOT_AVAILABLE/BETTER_OFFER 
    // or the the surplus for the given unit is 0. This last means that the household
    // does not have more margin of negotiation then is better look for another unit.
    if (biddingEntry.isValid() || pickEntryToBid()) {
        double surplus = luaModel.calculateSurplus(*(biddingEntry.getEntry()), 
                biddingEntry.getTries());
        //If the surplus is 0 means the bidder has reached the maximum 
        //number of bids that he can do for the current entry.
        if (surplus > 0) {
            const HousingMarket::Entry* entry = biddingEntry.getEntry();
            const Unit* unit = model->getUnitById(entry->getUnitId());
            const HM_Model::TazStats* stats = model->getTazStatsByUnitId(entry->getUnitId());
            if (unit && stats) {
                double bidValue = biddingEntry.getWP() - (biddingEntry.getWP() - entry->getAskingPrice()) - surplus;
                if (entry->getOwner() && bidValue > 0.0f) {
                    bid(entry->getOwner(), Bid(entry->getUnitId(),
                            household->getId(), getParent(), bidValue, now, biddingEntry.getWP(),
                            surplus));
                    return true;
                }
            }
        } else {
            biddingEntry.invalidate();
            return bidUnit(now); // try to bid again.
        }
    }
    return false;
}

bool HouseholdBidderRole::pickEntryToBid() {
    const Household* household = getParent()->getHousehold();
    HousingMarket* market = getParent()->getMarket();
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    const HM_Model* model = getParent()->getModel();
    //get available entries (for preferable zones if exists)
    HousingMarket::ConstEntryList entries;
    if (getParent()->getPreferableZones().empty()) {
        market->getAvailableEntries(entries);
    } else {
        market->getAvailableEntries(getParent()->getPreferableZones(), entries);
    }
    const HousingMarket::Entry* maxEntry = nullptr;
    double maxWP = 0; // holds the wp of the entry with maximum surplus.
    // choose the unit to bid with max surplus.
    for (HousingMarket::ConstEntryList::const_iterator itr = entries.begin();
            itr != entries.end(); itr++) {
        const HousingMarket::Entry* entry = *itr;
        if ((entry->getOwner() != getParent())) {
            const Unit* unit = model->getUnitById(entry->getUnitId());
            const HM_Model::TazStats* stats = model->getTazStatsByUnitId(entry->getUnitId());
            if (unit && stats) {
                double wp = luaModel.calulateWP(*household, *unit, *stats);
                if (wp >= entry->getAskingPrice() && (wp - entry->getAskingPrice()) > maxWP) {
                    maxWP = wp;
                    maxEntry = entry;
                }
            }
        }
    }

    biddingEntry = CurrentBiddingEntry(maxEntry, maxWP);
    return biddingEntry.isValid();
}