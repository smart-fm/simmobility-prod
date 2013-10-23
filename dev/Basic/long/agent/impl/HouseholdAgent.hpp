//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdAgent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 16, 2013, 6:36 PM
 */
#pragma once
#include "agent/LT_Agent.hpp"
#include "core/HousingMarket.hpp"
#include "database/entity/Household.hpp"
#include "event/EventListener.hpp"
#include "role/LT_Role.hpp"


namespace sim_mob {

    namespace long_term {
        
        /**
         * Represents an Long-Term household agent.
         * An household agent has the following capabilities:
         * - Sell units.
         * - Bid units. 
         */
        class HouseholdAgent : 
        public LT_Agent, public UnitHolder{
        public:
            HouseholdAgent(int id, Household* hh, HousingMarket* market);
            virtual ~HouseholdAgent();
        protected:
            /**
             * Inherited from LT_Agent.
             */
            virtual void HandleMessage(messaging::Message::MessageType type, 
                    const messaging::Message& message);
            void onStart(timeslice now);
            bool OnFrameInit(timeslice now);
            sim_mob::Entity::UpdateStatus OnFrameTick(timeslice now);
            void OnFrameOutput(timeslice now);
        private:
            virtual void OnEvent(event::EventId eventId, event::EventPublisher* sender, const event::EventArgs& args);
            virtual void OnEvent(event::EventId eventId, event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);
        private:
            HousingMarket* market;
            Household* hh;
            LT_Role* bidderRole;
            LT_Role* sellerRole;
        };
    }
}

