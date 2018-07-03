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
        	VehicleOwnershipChanges(BigSerial householdId = INVALID_ID, int oldehicleOwnershipOptionId = 0,int newVehicleOwnershipOptionId = 0,std::tm startDate = std::tm(),bool liveInTp = false, bool workInTp = false, double randomNum = 0);

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
		int getOldVehicleOwnershipOptionId() const;
		int getNewVehicleOwnershipOptionId() const;

		void setHouseholdId(BigSerial householdId);
		void setStartDate(const std::tm& startDate);
		void setOldVehicleOwnershipOptionId(int oldVehicleOwnershipOptionId);
		void setNewVehicleOwnershipOptionId(int nwVehicleOwnershipOptionId);

		bool isLiveInTp() const;
		void setLiveInTp(bool liveInTp);

		bool isWorkInTp() const;
		void setWorkInTp(bool workInTp);

		double getRandomNum() const;
		void setRandomNum(double randomNum);

        private:
            friend class VehicleOwnershipChangesDao;
        private:
            BigSerial householdId;
            int oldVehicleOwnershipOptionId;
            int newVehicleOwnershipOptionId;
            std::tm startDate;
            bool liveInTp;
            bool workInTp;
            double randomNum;
        };
    }
}
