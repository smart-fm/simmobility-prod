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
            Household(unsigned long id = INVALID_ID, float income = .0f,
                    int numberOfMembers = 0);
            virtual ~Household();

            /**
             * Gets unique identifier of the household.
             * @return id.
             */
            BigSerial GetId() const;

            /**
             * Gets the number of individuals in the household.
             * @return number of individuals value.
             */
            int GetNumberOfIndividuals() const;

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
                        << "\"headAge\":\"" << data.headAge << "\","
                        << "\"cars\":\"" << data.numberOfCars << "\","
                        << "\"workers\":\"" << data.numberOfWorkers << "\","
                        << "\"children\":\"" << data.numberOfChildren << "\","
                        << "\"individuals\":\"" << data.numberOfIndividuals << "\","
                        << "\"income\":\"" << data.income << "\","
                        << "\"race\":\"" << data.race << "\""
                        << "}";
            }

        private:
            friend class HouseholdDao;
        private:
            unsigned long id;
            int headAge;
            int numberOfCars;
            int numberOfWorkers;
            int numberOfChildren;
            int numberOfIndividuals;
            double income;   
            Race race;
        };
    }
}
