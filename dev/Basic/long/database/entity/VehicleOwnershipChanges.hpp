/*
 * VehicleOwnershipChanges.hpp
 *
 *  Created on: Dec 22, 2015
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class VehicleOwnershipChanges {
        public:
        	VehicleOwnershipChanges(BigSerial householdId = INVALID_ID, int vehicleOwnershipOptionId = 0,std::tm startDate = std::tm());

            virtual ~VehicleOwnershipChanges();
            VehicleOwnershipChanges( const VehicleOwnershipChanges &source);
            /**
             * Assign operator.
             * @param source to assign.
             * @return DevelopmentPlan instance reference.
             */
            VehicleOwnershipChanges& operator=(const VehicleOwnershipChanges& source);

            /**
             * Getters and Setters
             */

		BigSerial getHouseholdId() const;
		const std::tm& getStartDate() const;
		int getVehicleOwnershipOptionId() const;

		void setHouseholdId(BigSerial householdId);
		void setStartDate(const std::tm& startDate);
		void setVehicleOwnershipOptionId(int vehicleOwnershipOptionId);

        private:
            friend class VehicleOwnershipChangesDao;
        private:
            BigSerial householdId;
            int vehicleOwnershipOptionId;
            std::tm startDate;
        };
    }
}
