/*
 * HouseHoldHitsSample.hpp
 *
 *  Created on: Jun 24, 2015
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class HouseHoldHitsSample {
        public:
        	HouseHoldHitsSample(std::string houseHoldHitsId = std::string(),BigSerial houseHoldId = INVALID_ID, BigSerial groupId = INVALID_ID);

            virtual ~HouseHoldHitsSample();

            /**
             * Getters and Setters
             */
            std::string getHouseholdHitsId() const;
            BigSerial getHouseholdId() const;
            BigSerial getGroupId() const;

            /**
             * Operator to print the DistanceMRT data.
             */
            friend std::ostream& operator<<(std::ostream& strm,
                    const HouseHoldHitsSample& data);
        private:
            friend class HouseHoldHitsSampleDao;
        private:
            std::string houseHoldHitsId;
            BigSerial houseHoldId;
            BigSerial groupId;
        };
    }
}

