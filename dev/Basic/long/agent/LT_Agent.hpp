/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_Agent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 13, 2013, 6:36 PM
 */
#pragma once
#include "Common.h"
#include "entities/UpdateParams.hpp"
#include "conf/simpleconf.hpp"
#include "entities/Agent.hpp"
#include "event/EventManager.hpp"
#include "entity/HousingMarket.hpp"

using namespace sim_mob;
using std::vector;
using std::string;
using std::map;

namespace sim_mob {

    namespace long_term {
        class LT_Role;

        /**
         * Represents an Long-Term agent.
         * These agents can have different behaviors:
         * - Seller
         * - Bidder 
         * - Both
         * It will depend of the context.
         */
        class LT_Agent : public Agent, public UnitHolder {
        public:
            LT_Agent(int id, HousingMarket* market, float income, 
                     int numberOfMembers);
            virtual ~LT_Agent();

            /**
             * Inherited from Agent.
             */
            virtual void load(const map<string, string>& configProps);

            /**
             * Gets the EventManager reference from worker parent.
             * @return EventManager reference. 
             */
            EventManager& GetEventManager();

            /**
             * TODO: put this characteristics in another class
             * @return 
             */
            float GetIncome() const;
            float GetNumberOfMembers() const;

        protected:
            /**
             * Inherited from UnitHolder.
             */
            virtual void HandleMessage(MessageType type, 
                    MessageReceiver& sender, const Message& message);

            /**
             * Inherited from Agent.
             */
            virtual bool frame_init(timeslice now);
            virtual Entity::UpdateStatus frame_tick(timeslice now);
            virtual void frame_output(timeslice now);
            virtual bool isNonspatial();


        private:
            LT_Role* currentRole;
            HousingMarket* market;
            float income; // TODO: put this on separated class.
            int numberOfMembers; //on the family.
        };
    }
}

