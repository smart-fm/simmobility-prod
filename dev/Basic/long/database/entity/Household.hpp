/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Household.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 22, 2013, 5:54 PM
 */
#pragma once

#include "Common.h"
#include "Types.h"

namespace sim_mob {
    namespace long_term {

        class Household {
        public:
            Household(int id = INVALID_ID, float income = .0f,
                    int numberOfMembers = 0);
            virtual ~Household();

            /**
             * Gets unique identifier of the household.
             * @return id.
             */
            int GetId() const;

            /**
             * Gets the building identifier.
             * @return id.
             */
            int GetBuildingId() const;

            /**
             * Gets the number of members in the household.
             * @return number of members value.
             */
            int GetNumberOfMembers() const;

            /**
             * Gets the number of children in the household.
             * @return number of children value.
             */
            int GetNumberOfChildren() const;

            /**
             * Gets the number of workers in the household.
             * @return number of workers value.
             */
            int GetNumberOfWorkers() const;

            /**
             * Gets the number of cars in the household.
             * @return number of cars value.
             */
            int GetNumberOfCars() const;

            /**
             * Gets head age of the Household.
             * @return head age.
             */
            int GetHeadAge() const;

            /**
             * Gets the total income of the Household.
             * @return income value.
             */
            float GetIncome() const;

            /**
             * Gets the race of the Household.
             * @return race value.
             */
            Race GetRace() const;

            /**
             * Gets the importance of distance of CDB to this household
             * @return weight of the distance to CDB.
             */
            float GetWeightDistanceToCDB() const;

            /**
             * Gets the importance of unit size to this household.
             * @return weight of the unit size.
             */
            float GetWeightUnitSize() const;

            /**
             * Gets the importance of income to this household.
             * @return weight of the income.
             */
            float GetWeightIncome() const;

            /**
             * Gets the weight that represents the urgency to buy a new home.
             * @return weight urgency to buy.
             */
            float GetWeightUrgencyToBuy() const;
            
            /**
             * Assign operator.
             * @param source to assign.
             * @return Household instance reference.
             */
            Household& operator=(const Household& source);

            /**
             * Operator to print the Household data.  
             */
            friend ostream& operator<<(ostream& strm, const Household& data) {
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"building_id\":\"" << data.buildingId << "\","
                        << "\"persons\":\"" << data.numberOfMembers << "\","
                        << "\"children\":\"" << data.numberOfChildren << "\","
                        << "\"workers\":\"" << data.numberOfWorkers << "\","
                        << "\"cars\":\"" << data.numberOfCars << "\","
                        << "\"head-age\":\"" << data.headAge << "\","
                        << "\"income\":\"" << data.income << "\","
                        << "\"race\":\"" << data.race << "\","
                        << "\"weightDistanceToCDB\":\"" << data.weightDistanceToCDB << "\","
                        << "\"weightSize\":\"" << data.weightUnitSize << "\","
                        << "\"weightIncome\":\"" << data.weightIncome << "\","
                        << "\"weightUrgencyToBuy\":\"" << data.weightUrgencyToBuy << "\""
                        << "}";
            }

        private:
            friend class HouseholdDao;
        private:
            int id;
            int buildingId;
            int numberOfMembers;
            int numberOfChildren;
            int numberOfWorkers;
            int numberOfCars;
            int headAge;
            float income;
            float weightIncome; // importance that this household has for the income (should be always 1).
            float weightDistanceToCDB; // importance that this household has for the distance to CDB.
            float weightUnitSize; // importance that this household has for the size of the unit.
            float weightUrgencyToBuy; // represents the urgency that the HH has to buy a new home.
            Race race;
        };
    }
}
