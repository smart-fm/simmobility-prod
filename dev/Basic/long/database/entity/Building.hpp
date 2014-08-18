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

namespace sim_mob {

    namespace long_term {

        class Building {
        public:
        	Building( BigSerial fm_building_id = INVALID_ID, BigSerial fm_project_id = INVALID_ID, BigSerial fm_parcel_id = INVALID_ID, int storeys_above_ground = 0, int storeys_below_ground = 0,
        						std::string from_date = EMPTY_STR, std::string to_date = EMPTY_STR, std::string building_status = EMPTY_STR, float	gross_sq_m_res = 0, float gross_sq_m_office = 0,
        						float gross_sq_m_retail = 0, float gross_sq_m_other = 0);

            virtual ~Building();


            /*
             *
             * Gets unique identifier of the Type Type.
             * @return id.

            BigSerial getTypeId() const;

			BigSerial getId() const;

             * Gets unique identifier of the Project Type.
             * @return id.

            BigSerial getParcelId() const;
            

             * Gets unique identifier of the building tenure.
             * @return id.

            BigSerial getTenureId() const;


             * Gets the year that the building was built.
             * @return the year that the building was built.

            int getBuiltYear() const;


             * Gets number of storeys.
             * @return storeys number.

            int getStoreys() const;

             * Gets number of stories.
             * @return stories number.
            int getParkingSpaces() const;
            

             * Gets the landed area of the building.
             * @return area value.
            double getLandedArea() const;
			*/

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
            std::string getFromDate() const;

            /**
             * Gets the date till which the building was operational.
             * @return to_date.
             */
            std::string getToDate() const;

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
                
            /**
             * Assign operator.
             * @param source to assign.
             * @return Building instance reference.
             */
            Building& operator=(const Building& source);

            /**
             * Operator to print the Building data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, 
                const Building& data);

        private:
            friend class BuildingDao;
        private:
            /*
            BigSerial id;
            BigSerial typeId;
            BigSerial parcelId;
            BigSerial tenureId;
            int builtYear;
            int storeys;
            int parkingSpaces;
            double landedArea;
            */

            BigSerial fm_building_id;
            BigSerial fm_project_id;
            BigSerial fm_parcel_id;
            int storeys_above_ground;
            int	storeys_below_ground;
            std::string from_date;
            std::string to_date;
            std::string building_status;
            float	gross_sq_m_res;
            float	gross_sq_m_office;
            float	gross_sq_m_retail;
            float	gross_sq_m_other;



        };
    }
}
