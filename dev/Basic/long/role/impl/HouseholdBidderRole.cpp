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

HouseholdBidderRole::HouseholdBidderRole(HouseholdAgent* parent)
: LT_AgentRole(parent), waitingForResponse(false),
lastTime(0, 0), bidOnCurrentDay(false), biddingEntry(nullptr) {
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
                    biddingEntry = nullptr;
                    deleteBidsCounter(msg.getBid().getUnitId());
                    Statistics::increment(Statistics::N_ACCEPTED_BIDS);
                    break;
                }
                case NOT_ACCEPTED:
                {
                    incrementBidsCounter(msg.getBid().getUnitId());
                    break;
                }
                case BETTER_OFFER:
                {
                    biddingEntry = nullptr;
                    deleteBidsCounter(msg.getBid().getUnitId());
                    break;
                }
                case NOT_AVAILABLE:
                {
                    biddingEntry = nullptr;
                    deleteBidsCounter(msg.getBid().getUnitId());
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

    if (!biddingEntry) {
        biddingEntry = pickEntryToBid();
    }
    // Following the new assumptions of the model each household will stick on the 
    // unit where he is bidding until he gets rejected for seller by NOT_AVAILABLE/BETTER_OFFER 
    // or the the surplus for the given unit is 0. This last means that the household
    // does not have more margin of negotiation then is better look for another unit.
    if (biddingEntry) {
        double surplus = luaModel.calculateSurplus(*biddingEntry,
                getBidsCounter(biddingEntry->getUnitId()));
        //If the surplus is 0 means the bidder has reached the maximum 
        //number of bids that he can do for the current entry.
        if (surplus > 0) {
            const Unit* unit = model->getUnitById(biddingEntry->getUnitId());
            const HM_Model::TazStats* stats = model->getTazStatsByUnitId(biddingEntry->getUnitId());
            if (unit && stats) {
                double wp = luaModel.calulateWP(*household, *unit, *stats);
                double bidValue = wp - surplus;

                if (biddingEntry->getOwner() && bidValue > 0.0f) {
                    bid(biddingEntry->getOwner(), Bid(biddingEntry->getUnitId(),
                            household->getId(), getParent(), bidValue, now, wp,
                            surplus));
                    return true;
                }
            }
        } else {
            biddingEntry = nullptr;
            return bidUnit(now); // try to bid again.
        }
    }
    return false;
}

const HousingMarket::Entry* HouseholdBidderRole::pickEntryToBid() const {
    HousingMarket* market = getParent()->getMarket();
    //get available entries (for preferable zones if exists)
    HousingMarket::ConstEntryList entries;
    if (getParent()->getPreferableZones().empty()) {
        market->getAvailableEntries(entries);
    } else {
        market->getAvailableEntries(getParent()->getPreferableZones(), entries);
    }
    
    // choose the unit to bid with max surplus.
    /*const HousingMarket::Entry* maxEntry = nullptr;
    double maxSurplus = -1;
    for (HousingMarket::ConstEntryList::const_iterator itr = entries.begin();
            itr != entries.end(); itr++) {
        const HousingMarket::Entry* entry = *itr;
        if ((entry->getOwner() != getParent())) {
            double surplus = luaModel.calculateSurplus(*entry,
                    getBidsCounter(entry->getUnitId()));
            if (surplus > maxSurplus) {
                maxSurplus = surplus;
                maxEntry = entry;
            }
        }
    }*/

    if (!entries.empty()) {
        int randomIndex = Utils::generateInt(0, (entries.size() - 1));
        return entries.at(randomIndex);
    }
    return nullptr;
}

int HouseholdBidderRole::getBidsCounter(const BigSerial& unitId) {
    BidsCounterMap::iterator mapItr = bidsPerUnit.find(unitId);
    if (mapItr != bidsPerUnit.end()) {
        return mapItr->second;
    } else {
        bidsPerUnit.insert(BidCounterEntry(unitId, 1));
        return 1;
    }
}

void HouseholdBidderRole::incrementBidsCounter(const BigSerial& unitId) {
    BidsCounterMap::iterator mapItr = bidsPerUnit.find(unitId);
    if (mapItr != bidsPerUnit.end()) {
        (mapItr->second)++;
    } else {
        bidsPerUnit.insert(BidCounterEntry(unitId, 1));
    }
}

void HouseholdBidderRole::deleteBidsCounter(const BigSerial& unitId) {
    bidsPerUnit.erase(unitId);
}
