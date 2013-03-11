/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HousingMarket.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 4:13 PM
 */
#pragma once

#include "conf/settings/DisableMPI.h"
#include "entities/Entity.hpp"
#include "entities/Unit.hpp"
#include "agent/Seller.hpp"
#include "agent/Household.hpp"
#include "event/EventDispatcher.hpp"
#include "event/GenericEventPublisher.hpp"
#include "Util.h"

using std::map;
using std::string;
using std::vector;
using namespace sim_mob;

namespace sim_mob {

    namespace long_term {

        /**
         * Represents the housing market.
         * Th main responsibility is the management of: 
         *  - bids
         *  - sells
         *  - avaliable units
         *  - available households
         * 
         * The market is an updateable entity to simulate the evolution.
         * 
         */
        class HousingMarket : public Entity, public GenericEventPublisher {
        public:
            HousingMarket(EventDispatcher* dispatcher, unsigned int id=-1);
            virtual ~HousingMarket();

            /**
             * Inherited from Entity.
             */
            virtual Entity::UpdateStatus update(timeslice now);

            /**
             * Registers a new unit to the market.
             * @param unit to add.
             */
            void RegisterUnit(Unit* unit);

            /**
             * Registers a new seller to the market.
             * @param seller to register.
             */
            void RegisterSeller(Seller* seller);

            /**
             * Registers a new household to the market.
             * @param household to register.
             */
            void RegisterHousehold(Household* household);

            /**
             * UnRegisters a unit from the market.
             * @param unit to add.
             */
            void UnRegisterUnit(Unit* unit);

            /**
             * UnRegisters a seller from the market.
             * @param seller to register.
             */
            void UnRegisterSeller(Seller* seller);

            /**
             * UnRegisters a household from the market.
             * @param household to register.
             */
            void UnRegisterHousehold(Household* household);

        protected:
            /**
             * Inherited from Entity.
             */
            virtual void buildSubscriptionList(vector<BufferedBase*>& subsList);
            
        private:
            EventDispatcher* dispatcher;

        };
    }
}

