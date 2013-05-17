/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HouseholdBidderRole.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 16, 2013, 5:13 PM
 */
#pragma once
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
         * The bidder only can do a new bid if he is in a different 
         * day of the last bid AND if he is not waiting for a response.
         */
        class HouseholdBidderRole : public LT_AgentRole<HouseholdAgent>,
        public MessageReceiver {
        public:
            HouseholdBidderRole(HouseholdAgent* parent, Household* hh,
                    HousingMarket* market);
            virtual ~HouseholdBidderRole();

            /**
             * Inherited from LT_Role
             * @param currTime
             */
            virtual void Update(timeslice currTime);
        protected:

            /**
             * Inherited from LT_Role
             */
            virtual void HandleMessage(MessageType type,
                    MessageReceiver& sender, const Message& message);
        private:
            /**
             * Handler for wakeup event.
             * @param id of the event.
             * @param ctx context of the event.
             * @param sender EVentManager responsible for the fired event.
             * @param args {@link EM_EventArgs} instance.
             */
            void OnWakeUp(EventId id, Context ctx,
                    EventPublisher* sender, const EM_EventArgs& args);

            /**
             * Handler for Market action event.
             * @param id of the event.
             * @param sender of the event.
             * @param args of the event.
             */
            void OnMarketAction(EventId id, EventPublisher* sender,
                    const HM_ActionEventArgs& args);

            /**
             * Subscribes the role to all market generic events.
             */
            void FollowMarket();

            /**
             * UnSubscribes the role to all market generic events.
             */
            void UnFollowMarket();

        private:
            friend class HouseholdAgent;
            bool BidUnit(timeslice now);
            float CalculateSurplus(const Unit& unit);
            float CalculateWP();
        private:
            Household* hh;
            HousingMarket* market;
            volatile bool waitingForResponse;
            timeslice lastTime;
            bool bidOnCurrentDay;
        };
    }
}

