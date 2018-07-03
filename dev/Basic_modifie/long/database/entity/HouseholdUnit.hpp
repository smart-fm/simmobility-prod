/*
 * HouseholdUnit.hpp
 *
 *  Created on: 28 Mar 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class HouseholdUnit {
        public:
        	HouseholdUnit(BigSerial houseHoldId = INVALID_ID,BigSerial unitId = INVALID_ID, std::tm moveInDate = std::tm());

            virtual ~HouseholdUnit();

            /**
             * Getters and Setters
             */
            BigSerial getHouseHoldId() const;
            void setHouseHoldId(BigSerial houseHoldId);
            const std::tm& getMoveInDate() const;
            void setMoveInDate(std::tm moveInDate);
            BigSerial getUnitId() const;
            void setUnitId(BigSerial unitId);


        private:
            friend class HouseholdUnitDao;
        private:
            BigSerial houseHoldId;
            BigSerial unitId;
            std::tm moveInDate;
        };
    }
}

