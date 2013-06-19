/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HouseholdAgent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 16, 2013, 6:36 PM
 */
#pragma once
#include "agent/LT_Agent.hpp"
#include "core/HousingMarket.hpp"
#include "database/entity/Household.hpp"
#include "role/LT_Role.hpp"


using namespace sim_mob;
using std::vector;
using std::string;
using std::map;

namespace sim_mob {

    namespace long_term {

        /**
         * Represents an Long-Term household agent.
         * An household agent has the following capabilities:
         * - Sell units.
         * - Bid units. 
         */
        class HouseholdAgent : public LT_Agent, public UnitHolder {
        public:
            HouseholdAgent(int id, Household* hh, HousingMarket* market);
            virtual ~HouseholdAgent();
        protected:
            /**
             * Inherited from LT_Agent.
             */
            virtual void HandleMessage(MessageType type,
                    MessageReceiver& sender, const Message& message);
            bool OnFrameInit(timeslice now);
            Entity::UpdateStatus OnFrameTick(timeslice now, int messageCounter);
            void OnFrameOutput(timeslice now);
        private:
            HousingMarket* market;
            Household* hh;
            LT_Role* currentRole;
        };
    }
}

