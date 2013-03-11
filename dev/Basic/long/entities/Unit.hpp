/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Property.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 3:05 PM
 */
#pragma once

#include "conf/settings/DisableMPI.h"
#include "entities/Entity.hpp"

using std::map;
using std::string;
using std::vector;
using namespace sim_mob;

namespace sim_mob {

    namespace long_term {

        /**
         * Represents a unit to buy/rent.
         * It can be the following:
         *  - Apartment
         *  - House
         *  - Garage
         */
        class Unit : public Entity {
        public:
            Unit(unsigned int id);
            virtual ~Unit();

            /**
             * Inherited from Entity.
             */
            virtual Entity::UpdateStatus update(timeslice now);

        protected:
            /**
             * Inherited from Entity.
             */
            virtual void buildSubscriptionList(vector<BufferedBase*>& subsList);
        };
    }
}

