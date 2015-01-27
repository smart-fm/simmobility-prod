//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Building.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
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
        			  int storeysBelowGround = 0, std::tm fromDate = std::tm(), std::tm toDate = std::tm(), std::string buildingStatus = EMPTY_STR,
        			  float	grossSqMRes = 0, float grossSqMOffice = 0, float grossSqMRetail = 0, float grossSqMOther = 0);

            virtual ~Building();

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
            std::string getBuildingStatus() const;

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


            /*
             * setters
             */
            void setBuildingStatus(const std::string& buildingStatus);
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

            /**
             * Assign operator.
             * @param source to assign.
             * @return Building instance reference.
             */
            Building& operator=(const Building& source);

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
            std::string buildingStatus;
            float	grossSqMRes;
            float	grossSqMOffice;
            float	grossSqMRetail;
            float	grossSqMOther;

        };
    }
}
