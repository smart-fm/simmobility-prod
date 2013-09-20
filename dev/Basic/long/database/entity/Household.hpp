//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Household.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 22, 2013, 5:54 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class Household {
        public:
            Household(BigSerial id = INVALID_ID, double income = .0f,
                    int size = 0, int children = 0, int carOwnership = 0, 
                    BigSerial unitId = INVALID_ID, int housingDuration = 0);
            virtual ~Household();

            /**
             * Gets unique identifier of the household.
             * @return id.
             */
            BigSerial GetId() const;

            /**
             * Gets unit unique identifier.
             * @return unit unique identifier.
             */
            BigSerial GetUnitId() const;

            /**
             * Gets the size of the household.
             * @return  the size of the household.
             */
            int GetSize() const;

            /**
             * Gets the number of children in the household.
             * @return number of children value.
             */
            int GetChildren() const;

            /**
             * Gets the total income of the Household.
             * @return income value.
             */
            double GetIncome() const;

            /**
             * Gets the car ownership value.
             * @return the car ownership value.
             */
            int GetCarOwnership() const;

            /**
             * Gets housing duration value.
             * @return housing duration value.
             */
            int GetHousingDuration() const;

            /**
             * Assign operator.
             * @param source to assign.
             * @return Household instance reference.
             */
            Household& operator=(const Household& source);

            /**
             * Operator to print the Household data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const Household& data) {
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"unitId\":\"" << data.unitId << "\","
                        << "\"size\":\"" << data.size << "\","
                        << "\"children\":\"" << data.children << "\","
                        << "\"income\":\"" << data.income << "\","
                        << "\"carOwnership\":\"" << data.carOwnership << "\","
                        << "\"housingDuration\":\"" << data.housingDuration << "\""
                        << "}";
            }

        private:
            friend class HouseholdDao;
        private:
            BigSerial id;
            BigSerial unitId;
            int size;
            int children;
            double income;
            int carOwnership;
            int housingDuration;
        };
    }
}
