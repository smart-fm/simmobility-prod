//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdBidderRole.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 16, 2013, 5:13 PM
 */
#pragma once
#include <boost/unordered_map.hpp>
#include "role/LT_Role.hpp"
#include "event/LT_EventArgs.hpp"
#include "database/entity/Household.hpp"
#include "core/HousingMarket.hpp"

namespace sim_mob {

    namespace long_term {

        class HouseholdAgent;

        /**
         * Bidder role for household.
         * 
         * This role starts to go to the market and choose the unit 
         * with maximum surplus for the bidder. After that the bidder 
         * will bid the unit and will *WAIT* for the response.
         * 
         * The bidder can only do a bid each day. 
         * If he is waiting for a response he will 
         * only able to do the next bid on the next day.
         */
        class HouseholdBidderRole : public LT_AgentRole<HouseholdAgent>{
        public:
            HouseholdBidderRole(HouseholdAgent* parent, Household* hh,
                    HousingMarket* market);
            virtual ~HouseholdBidderRole();

            /**
             * Inherited from LT_Role
             * @param currTime
             */
            virtual void update(timeslice currTime);
        protected:

            /**
             * Inherited from LT_Role
             */
            virtual void HandleMessage(messaging::Message::MessageType type,
                const messaging::Message& message);
        private:
            /**
             * Handler for wakeup event.
             * @param id of the event.
             * @param ctx context of the event.
             * @param sender EVentManager responsible for the fired event.
             * @param args {@link EM_EventArgs} instance.
             */
            void onWakeUp(event::EventId id, event::Context ctx,
                    event::EventPublisher* sender, const event::EM_EventArgs& args);

            /**
             * Handler for Market action event.
             * @param id of the event.
             * @param sender of the event.
             * @param args of the event.
             */
            void onMarketAction(event::EventId id, event::EventPublisher* sender,
                    const HM_ActionEventArgs& args);

            /**
             * Subscribes the role to all market generic events.
             */
            void followMarket();

            /**
             * UnSubscribes the role to all market generic events.
             */
            void unFollowMarket();

        private:
            friend class HouseholdAgent;
            
            /**
             * Helper method that goes to the market, gets the available units
             * and calculates the unit with maximum surplus. 
             * If the household as willingness to pay for a unit then makes a bit.  
             * @param now
             * @return 
             */
            bool bidUnit(timeslice now);
            
            /**
             * Gets the bids counter for the given unit.
             * @param unitId unit unique identifier.
             * @return number of bids.
             */
            int getBidsCounter(UnitId unitId);

            /**
             * Increments the bids counter for the given unit.
             * @param unitId unit unique identifier.
             */
            void incrementBidsCounter(UnitId unitId);
            
            /**
             * Deletes the counter for the given unit.
             * @param unitId unit unique identifier.
             */
            void deleteBidsCounter(UnitId unitId);

        private:
            Household* hh;
            HousingMarket* market;
            volatile bool waitingForResponse;
            timeslice lastTime;
            bool bidOnCurrentDay;
            typedef boost::unordered_map<UnitId, int> BidsCounterMap; // bids made per unit.  
            typedef std::pair<UnitId, int> BidCounterEntry;
            BidsCounterMap bidsPerUnit;
        };
    }
}

