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
                    BigSerial parcelId = INVALID_ID, int builtYear = 0,
                    double floorArea = .0f, int storeys = 0, int parkingSpaces = 0);

            virtual ~Building();

            /**
             * Gets unique identifier of the Building Type.
             * @return id.
             */
            BigSerial GetId() const;

            /**
             * Gets unique identifier of the Type Type.
             * @return id.
             */
            BigSerial GetTypeId() const;

            /**
             * Gets unique identifier of the Project Type.
             * @return id.
             */
            BigSerial GetParcelId() const;

            /**
             * Gets the year that the building was built.
             * @return the year that the building was built.
             */
            int GetBuiltYear() const;

            /**
             * Gets the floor area value.
             * @return floor area value.
             */
            double GetFloorArea() const;

            /**
             * Gets number of storeys.
             * @return storeys number.
             */
            int GetStoreys() const;

            /**
             * Gets number of stories.
             * @return stories number.
             */
            int GetParkingSpaces() const;
            
            /**
             * Gets number of residential units.
             * @return stories number.
             */
            int GetResidentialUnits() const;
            double GetLandArea() const;
            int GetImprovementValue() const;
            int GetTaxExempt() const;
            double GetNonResidentialSqft() const;
            double GetSqftPerUnit() const;
                
            /**
             * Assign operator.
             * @param source to assign.
             * @return Building instance reference.
             */
            Building& operator=(const Building& source);

            /**
             * Operator to print the Building data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const Building& data) {
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"typeId\":\"" << data.typeId << "\","
                        << "\"parcelId\":\"" << data.parcelId << "\","
                        << "\"builtYear\":\"" << data.builtYear << "\","
                        << "\"floorArea\":\"" << data.floorArea << "\","
                        << "\"storeys\":\"" << data.storeys << "\","
                        << "\"parkingSpaces\":\"" << data.parkingSpaces << "\","
                        << "\"residentialUnits\":\"" << data.residentialUnits << "\","
                        << "\"landArea\":\"" << data.landArea << "\","
                        << "\"improvementValue\":\"" << data.improvementValue << "\","
                        << "\"taxExempt\":\"" << data.taxExempt << "\","
                        << "\"nonResidentialSqft\":\"" << data.nonResidentialSqft << "\","
                        << "\"sqftPerUnit\":\"" << data.sqftPerUnit << "\""
                        << "}";
            }

        private:
            friend class BuildingDao;
        private:
            BigSerial id;
            BigSerial typeId;
            BigSerial parcelId;
            int builtYear;
            double floorArea;
            int storeys;
            int parkingSpaces;
            int residentialUnits;
            double landArea;
            int improvementValue;
            int taxExempt;
            double nonResidentialSqft;
            double sqftPerUnit;
        };
    }
}
