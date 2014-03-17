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
            Building(BigSerial id = INVALID_ID, BigSerial typeId = INVALID_ID,
                    BigSerial parcelId = INVALID_ID, 
                    BigSerial tenureId = INVALID_ID, int builtYear = 0,
                    double landedArea = .0f, int storeys = 0, int parkingSpaces = 0);

            virtual ~Building();

            /**
             * Gets unique identifier of the Building Type.
             * @return id.
             */
            BigSerial getId() const;

            /**
             * Gets unique identifier of the Type Type.
             * @return id.
             */
            BigSerial getTypeId() const;

            /**
             * Gets unique identifier of the Project Type.
             * @return id.
             */
            BigSerial getParcelId() const;
            
            /**
             * Gets unique identifier of the building tenure.
             * @return id.
             */
            BigSerial getTenureId() const;

            /**
             * Gets the year that the building was built.
             * @return the year that the building was built.
             */
            int getBuiltYear() const;

            /**
             * Gets number of storeys.
             * @return storeys number.
             */
            int getStoreys() const;

            /**
             * Gets number of stories.
             * @return stories number.
             */
            int getParkingSpaces() const;
            
            /**
             * Gets the landed area of the building.
             * @return area value.
             */
            double getLandedArea() const;
                
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
            BigSerial id;
            BigSerial typeId;
            BigSerial parcelId;
            BigSerial tenureId;
            int builtYear;
            int storeys;
            int parkingSpaces;
            double landedArea;
        };
    }
}
