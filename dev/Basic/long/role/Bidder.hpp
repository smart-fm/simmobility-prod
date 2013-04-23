/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Seller.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 4, 2013, 5:13 PM
 */
#pragma once
#include "role/LT_Role.hpp"
#include "entity/HousingMarket.hpp"
#include "event/LT_EventArgs.hpp"

namespace sim_mob {

    namespace long_term {
        /**
         * Bidder role.
         * 
         * An agent with this role active means that 
         * he is looking for a new house. 
         */
        class Bidder : public LT_Role, public MessageReceiver {
        public:
            Bidder(LT_Agent* parent, HousingMarket* market);
            virtual ~Bidder();
            
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
            bool BidUnit();
            float CalculateSurplus(const Unit& unit);
            float CalculateWP();
        private:
            HousingMarket* market;
            volatile bool waitingForResponse;
            timeslice lastTime;
        };
    }
}

