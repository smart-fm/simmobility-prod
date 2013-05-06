/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Bid.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 5, 2013, 5:03 PM
 */
#pragma once

#include "Common.h"
#include "Types.h"

namespace sim_mob {

    namespace long_term {

        /**
         * Represents a bid to a unit.
         */
        class Bid {
        public:

            Bid(UnitId id, int bidderId, float value);
            Bid(const Bid& source);
            virtual ~Bid();

            /**
             * An operator to allow the bid copy.
             * @param source an Bid to be copied.
             * @return The Bid after modification
             */
            Bid& operator=(const Bid& source);

            /**
             * Gets the Unit unique identifier.
             * @return value with Unit identifier.
             */
            UnitId GetUnitId() const;

            /**
             * Gets the Bidder unique identifier.
             * @return value with Bidder identifier.
             */
            int GetBidderId() const;

            /**
             * Gets the value of the bid.
             * @return the value of the bid.
             */
            float GetValue() const;

        private:
            UnitId unitId;
            int bidderId;
            float value;
        };
    }
}

