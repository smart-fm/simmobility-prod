/*
 * HouseholdPlanningArea.hpp
 *
 *  Created on: 8 Mar 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class HouseholdPlanningArea {
        public:
            HouseholdPlanningArea(BigSerial houseHoldId = INVALID_ID, BigSerial tazId = INVALID_ID, BigSerial tazName = INVALID_ID,std::string planningArea = std::string());

            virtual ~HouseholdPlanningArea();

            /**
             * Getters and Setters
             */
            BigSerial getHouseHoldId() const;
            const std::string& getPlanningArea() const;
            BigSerial getTazId() const;
            BigSerial getTazName() const;

            void setHouseHoldId(BigSerial houseHoldId);
            void setPlanningArea(const std::string& planningArea);
            void setTazId(BigSerial tazId) ;
            void setTazName(BigSerial tazname) ;

        private:
            friend class HouseholdPlanningAreaDao;
        private:
            BigSerial houseHoldId;
            BigSerial tazId;
            BigSerial tazName;
            std::string planningArea;
        };
    }
}
