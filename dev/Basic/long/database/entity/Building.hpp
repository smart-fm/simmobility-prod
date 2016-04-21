//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Building.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * Author : Gishara Premarathne <gishara@smart.mit.edu>
 *
 * Created on May 8, 2013, 3:04 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{

    namespace long_term
    {

        class Building
        {
        public:
        	Building( BigSerial fmBuildingId = INVALID_ID, BigSerial fmProjectId = INVALID_ID, BigSerial fmParcelId = INVALID_ID, int storeysAboveGround = 0,
        			  int storeysBelowGround = 0, std::tm fromDate = std::tm(), std::tm toDate = std::tm(), int buildingStatus = 0,float	grossSqMRes = 0, float grossSqMOffice = 0,
					  float grossSqMRetail = 0, float grossSqMOther = 0,std::tm lastChangedDate = std::tm(),int freehold = 0,float floorSpace = 0,std::string buildingType = std::string(),BigSerial slaAddressId = INVALID_ID);

            virtual ~Building();

            Building( const Building &source);

            /**
             * Assign operator.
             * @param source to assign.
             * @return Building instance reference.
             */
            Building& operator=(const Building& source);

            /**
             * Gets unique identifier of the Building Type.
             * @return id.
             */
            BigSerial getFmBuildingId() const;

            /**
             * Gets the project id  id.
             * @return fm_project_id.
             */
            BigSerial getFmProjectId() const;

            /**
             * Gets the unique fm_parcel_id.
             * @return fm_parcel_id.
             */
            BigSerial getFmParcelId() const;

            /**
             * Gets the number of floors above ground.
             * @return storeys_above_ground.
             */
            int	getStoreysAboveGround() const;

            /**
             * Gets the number of basement floors.
             * @return storeys_below_ground.
             */
            int getStoreysBelowGround() const;

            /**
             * Gets the date from which the building became operational.
             * @return from_date.
             */
            std::tm getFromDate() const;

            /**
             * Gets the date till which the building was operational.
             * @return to_date.
             */
            std::tm getToDate() const;

            /**
             * Gets the building status.
             * @return building_status.
             */
            int getBuildingStatus() const;

            /**
             * Gets the area used for residential purposes.
             * @return gross_sq_m_res.
             */
            float	getGrossSqmRes() const;

            /**
             * Gets the area used for office.
             * @return gross_sq_m_office.
             */
            float	getGrossSqmOffice() const;

            /**
             * Gets the retail space area.
             * @return gross_sq_m_retail.
             */
            float	getGrossSqmRetail() const;

            /**
             * Gets the other space area.
             * @return gross_sq_m_other.
             */
            float	getGrossSqmOther() const;

            std::tm getLastChangedDate() const;

            const std::string& getBuildingType() const;
            float getFloorSpace() const ;
            int getFreehold() const;
            BigSerial getSlaAddressId() const;
            /*
             * setters
             */
            void setBuildingStatus(int buildingStatus);
            void setFmBuildingId(BigSerial fmBuildingId);
            void setFmParcelId(BigSerial fmParcelId);
            void setFmProjectId(BigSerial fmProjectId);
            void setFromDate(const std::tm& fromDate);
            void setGrossSqMOffice(float grossSqMOffice);
            void setGrossSqMOther(float grossSqMOther);
            void setGrossSqMRes(float grossSqMRes);
            void setGrossSqMRetail(float grossSqMRetail);
            void setStoreysAboveGround(int storeysAboveGround);
            void setStoreysBelowGround(int storeysBelowGround);
            void setToDate(const std::tm& toDate);
            void setLastChangedDate(const std::tm& lastChangedDate);
            void setBuildingType(const std::string& buildingType);
            void setFloorSpace(float floorSpace);
            void setFreehold(int freehold);
            void setSlaAddressId(BigSerial slaAddressId);


            /**
             * Operator to print the Building data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const Building& data);

        private:
            friend class BuildingDao;

            BigSerial fmBuildingId;
            BigSerial fmProjectId;
            BigSerial fmParcelId;
            int storeysAboveGround;
            int	storeysBelowGround;
            std::tm fromDate;
            std::tm toDate;
            int buildingStatus;
            float	grossSqMRes;
            float	grossSqMOffice;
            float	grossSqMRetail;
            float	grossSqMOther;
            std::tm lastChangedDate;
            int freehold;
            float floorSpace;
            std::string buildingType;
            BigSerial slaAddressId;

        };
    }
}
