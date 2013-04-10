/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HousingMarket.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 4:13 PM
 */
#pragma once

#include "model/UnitHolder.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Represents the housing market.
         * Th main responsibility is the management of: 
         *  - avaliable units
         */
        class HousingMarket : public UnitHolder {
        public:
            HousingMarket();
            virtual ~HousingMarket();

        protected:
            /**
             * Inherited from UnitHolder.
             */
            bool Add(Unit* unit, bool reOwner);
            Unit* Remove(UnitId id, bool reOwner);
        };
    }
}

