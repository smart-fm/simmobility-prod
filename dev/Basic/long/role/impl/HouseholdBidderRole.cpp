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
    inline void bid(MessageHandler* owner, const Bid& bid)
    {
        MessageBus::PostMessage(owner, LTMID_BID, MessageBus::MessagePtr(new BidMessage(bid)));
    }
}

HouseholdBidderRole::CurrentBiddingEntry::CurrentBiddingEntry (const BigSerial unitId, const double wp, double lastSurplus) : unitId(unitId), wp(wp), tries(0), lastSurplus(lastSurplus)
{
}

HouseholdBidderRole::CurrentBiddingEntry::~CurrentBiddingEntry()
{
    invalidate();
}

BigSerial HouseholdBidderRole::CurrentBiddingEntry::getUnitId() const
{
    return unitId;
}

double HouseholdBidderRole::CurrentBiddingEntry::getWP() const
{
    return wp;
}

long int HouseholdBidderRole::CurrentBiddingEntry::getTries() const
{
    return tries;
}

void HouseholdBidderRole::CurrentBiddingEntry::incrementTries(int quantity)
{
    tries += quantity;
}

bool HouseholdBidderRole::CurrentBiddingEntry::isValid() const
{
    return (unitId != INVALID_ID);
}

void HouseholdBidderRole::CurrentBiddingEntry::invalidate()
{
    unitId = INVALID_ID;
    tries = 0;
    wp = 0;
}
                
HouseholdBidderRole::HouseholdBidderRole(HouseholdAgent* parent): LT_AgentRole(parent), waitingForResponse(false), lastTime(0, 0), bidOnCurrentDay(false){}

HouseholdBidderRole::~HouseholdBidderRole(){}

void HouseholdBidderRole::update(timeslice now)
{
    //can bid another house if it is not waiting for any 
    //response and if it not the same day
    if (!waitingForResponse && lastTime.ms() < now.ms())
    {
        bidOnCurrentDay = false;
    }

    if (isActive())
    {
        if (!waitingForResponse && !bidOnCurrentDay && bidUnit(now))
        {
            waitingForResponse = true;
            bidOnCurrentDay = true;
        }
    }

    lastTime = now;
}

void HouseholdBidderRole::HandleMessage(Message::MessageType type, const Message& message)
{
    switch (type)
    {
        case LTMID_BID_RSP:// Bid response received 
        {
            const BidMessage& msg = MSG_CAST(BidMessage, message);
            switch (msg.getResponse())
            {
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
                {
                    break;
                }
                case NOT_AVAILABLE:
                {
                    biddingEntry.invalidate();
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

bool HouseholdBidderRole::bidUnit(timeslice now)
{
    HousingMarket* market = getParent()->getMarket();
    const Household* household = getParent()->getHousehold();
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    const HM_Model* model = getParent()->getModel();
    
    // Following the new assumptions of the model each household will stick on the 
    // unit where he is bidding until he gets rejected for seller by NOT_AVAILABLE/BETTER_OFFER 
    // or the the speculation for the given unit is 0. This last means that the household
    // does not have more margin of negotiation then is better look for another unit.
    const HousingMarket::Entry* entry = market->getEntryById(biddingEntry.getUnitId());

    if (!entry || !biddingEntry.isValid())
    {
        //if unit is not available or entry is not valid then
        //just pick another unit to bid.
        if(pickEntryToBid())
        {
            entry = market->getEntryById(biddingEntry.getUnitId());
            //PrintOut("Household " << household->getId() << " is picking a new unit " << biddingEntry.getUnitId() << "to bid on." << std::endl );
        }   
    }
    
    if (entry && biddingEntry.isValid())
    {
        double speculation = luaModel.calculateSpeculation(*entry, biddingEntry.getTries());

        //If the speculation is 0 means the bidder has reached the maximum 
        //number of bids that he can do for the current entry.
        if (speculation > 0)
        {
            const Unit* unit = model->getUnitById(entry->getUnitId());
            const HM_Model::TazStats* stats = model->getTazStatsByUnitId(entry->getUnitId());

            if (unit && stats)
            {
                double bidValue = biddingEntry.getWP() - speculation;

                if (entry->getOwner() && bidValue > 0.0f)
                {
                	//PrintOut("\033[1;36mHousehold " << std::dec << household->getId() << " submitted a bid on unit " << biddingEntry.getUnitId() << "\033[0m\n" );
                	PrintOut("Household " << std::dec << household->getId() << " submitted a bid of $" << bidValue << "[wp:$" << biddingEntry.getWP() << ",sp:$" << speculation  << ",bids:"  <<   biddingEntry.getTries() << ",ap:$" << entry->getAskingPrice() << "] on unit " << biddingEntry.getUnitId() << "." << std::endl );

                    bid(entry->getOwner(), Bid(entry->getUnitId(), household->getId(), getParent(), bidValue, now, biddingEntry.getWP(), speculation));
                    return true;
                }
            }
        }
        else
        {
            biddingEntry.invalidate();
            return bidUnit(now); // try to bid again.
        }
    }
    return false;
}

bool HouseholdBidderRole::pickEntryToBid()
{
    const Household* household = getParent()->getHousehold();
    HousingMarket* market = getParent()->getMarket();
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    const HM_Model* model = getParent()->getModel();
    //get available entries (for preferable zones if exists)
    HousingMarket::ConstEntryList entries;

    if (getParent()->getPreferableZones().empty())
    {
        market->getAvailableEntries(entries);
    }
    else
    {
        market->getAvailableEntries(getParent()->getPreferableZones(), entries);
    }

    const HousingMarket::Entry* maxEntry = nullptr;
    double maxWP = 0; // holds the wp of the entry with maximum surplus.

    // choose the unit to bid with max surplus.
    for (HousingMarket::ConstEntryList::const_iterator itr = entries.begin(); itr != entries.end(); itr++)
    {
        const HousingMarket::Entry* entry = *itr;

        if(entry->getOwner() != getParent())
        {
            const Unit* unit = model->getUnitById(entry->getUnitId());
            const HM_Model::TazStats* stats = model->getTazStatsByUnitId(entry->getUnitId());

            bool flatEligibility = true;

            if( unit->getUnitType() == 2 && household->getTwoRoomHdbEligibility()  == false)
            	flatEligibility = false;

            if( unit->getUnitType() == 3 && household->getThreeRoomHdbEligibility() == false )
                flatEligibility = false;

            if( unit->getUnitType() == 4 && household->getFourRoomHdbEligibility() == false )
                flatEligibility = false;


            if ( unit && stats && flatEligibility )
            {
                double wp = luaModel.calulateWP(*household, *unit, *stats);

                if (wp >= entry->getAskingPrice() && (wp - entry->getAskingPrice()) > maxWP)
                {
                    maxWP = wp;
                    maxEntry = entry;
                }
            }
        }
    }

    biddingEntry = CurrentBiddingEntry((maxEntry) ? maxEntry->getUnitId() : INVALID_ID, maxWP);
    return biddingEntry.isValid();
}








