/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
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
            Building();
            virtual ~Building();

            /**
             * Gets unique identifier of the Building Type.
             * @return id.
             */
            BigSerial GetId() const;

            /**
             * Gets the number total units in the building.
             * @return number of total units in the building.
             */
            int GetNumberOfUnits() const;

            /**
             * Gets the number of residential units.
             * @return number of residential units.
             */
            int GetNumberOfResidentialUnits() const;

            /**
             * Gets the number of business units.
             * @return number of business units.
             */
            int GetNumberOfBusinessUnits() const;

            /**
             * Gets the year of construction.
             * @return the year of construction.
             */
            int GetYear() const;

            /**
             * Gets the land area value.
             * @return land area value.
             */
            double GetArea() const;

            /**
             * Gets the value average income of the building.
             * @return improvement value.
             */
            double GetAverageIncome() const;

            /**
             * Gets the main race.
             * @return race.
             */
            Race GetMainRace() const;

            /**
             * Gets number of stories.
             * @return stories number.
             */
            int GetNumberOfStories() const;

            /**
             * Gets Distance to CDB.
             * @return distance to the center of the business district.
             */
            double GetDistanceToCDB() const;

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
                        << "\"units\":\"" << data.numberOfUnits << "\","
                        << "\"residentialUnits\":\"" << data.numberOfResidentialUnits << "\","
                        << "\"businessUnits\":\"" << data.numberOfBusinessUnits << "\","
                        << "\"stories\":\"" << data.numberOfStories << "\","
                        << "\"area\":\"" << data.area << "\","
                        << "\"year\":\"" << data.year << "\","
                        << "\"stories\":\"" << data.numberOfStories << "\","
                        << "\"averageIncome\":\"" << data.averageIncome << "\","
                        << "\"mainRace\":\"" << data.mainRace << "\","
                        << "\"distanceToCDB\":\"" << data.distanceToCDB << "\""
                        << "}";
            }

        private:
            friend class BuildingDao;
        private:
            unsigned long id;
            int numberOfUnits;
            int numberOfResidentialUnits;
            int numberOfBusinessUnits;
            int numberOfStories;
            double area;
            int year; //year of construction
            double averageIncome;
            Race mainRace;
            double distanceToCDB;
        };
    }
}

