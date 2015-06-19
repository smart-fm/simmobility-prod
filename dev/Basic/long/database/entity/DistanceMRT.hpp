/*
 * DistanceMRT.hpp
 *
 *  Created on: Jun 2, 2015
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class DistanceMRT {
        public:
        	DistanceMRT(BigSerial houseHoldId = INVALID_ID, double distanceMrt = 0.0);

            virtual ~DistanceMRT();

            /**
             * Getters and Setters
             */
            BigSerial getHouseholdId() const;
            double getDistanceMrt() const;

            /**
             * Operator to print the DistanceMRT data.
             */
            friend std::ostream& operator<<(std::ostream& strm,
                    const DistanceMRT& data);
        private:
            friend class DistanceMRTDao;
        private:
            BigSerial houseHoldId;
            double distanceMrt;
        };
    }
}
