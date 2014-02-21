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
                    deleteBidsCounter(msg.getBid().getUnitId());
                    break;
                }
                case NOT_AVAILABLE:
                {
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
    HousingMarket* market = getParent()->getMarket();
    const Household* household = getParent()->getHousehold();
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    const HousingMarket::EntryMap& entries = market->getAvailableEntries();
    // choose the unit to bid with max surplus.
    const HousingMarket::Entry* maxEntry = nullptr;
    float maxSurplus = -1;
    for (HousingMarket::EntryMap::const_iterator itr = entries.begin();
            itr != entries.end(); itr++) {
        const HousingMarket::Entry* entry = &(itr->second);
        if ((entry->getOwner() != getParent())) {
            float surplus = luaModel.calculateSurplus(*entry,
                    getBidsCounter(entry->getUnitId()));
            if (surplus > maxSurplus) {
                maxSurplus = surplus;
                maxEntry = entry;
            }
        }
    }
    // Exists some unit to bid.
    if (maxEntry) {
        float bidValue = maxSurplus +
                luaModel.calulateWP(*household, maxEntry->getUnit());

        if (maxEntry->getOwner() && bidValue > 0.0f) {
            bid(maxEntry->getOwner(), Bid(maxEntry->getUnitId(),
                    household->getId(), getParent(), bidValue, now));
            return true;
        }
    }
    return false;
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
