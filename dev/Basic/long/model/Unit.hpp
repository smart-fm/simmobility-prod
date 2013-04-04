/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Property.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 3:05 PM
 */
#pragma once

#include "Common.h"

namespace sim_mob {

    namespace long_term {

        class UnitHolder;

        /**
         * Represents a unit to buy/rent.
         * It can be the following:
         *  - Apartment
         *  - House
         *  - Garage
         */
        class Unit {
        public:
            Unit(UnitId id);
            Unit(const Unit& source);
            virtual ~Unit();

            /**
             * An operator to allow the unit copy.
             * @param source an Unit to be copied.
             * @return The Unit after modification
             */
            Unit& operator=(const Unit& source);

            /**
             * Gets the Unit unique identifier.
             * @return value with Unit identifier.
             */
            UnitId GetId() const;

            /**
             * Verifies if home is available.
             * @return true if unit is available, false otherwise.
             */
            bool IsAvailable() const;

        private:
            UnitId id;
            bool available;
        };
    }
}

