
#include "LT_Agent.hpp"

/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Individual.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 22, 2013, 5:46 PM
 */
#pragma once

#include "LT_Agent.hpp"
#include "Types.h"


namespace sim_mob {
    namespace long_term {

        class Individual : public LT_Agent {
        public:
            Individual(int id, HousingMarket* market, float income);
            virtual ~Individual();

            /**
             * Get the incoming of the Individual.
             * @return the incoming value.
             */
            float GetIncome() const;

            /**
             * Tells if Individual has or not a job.
             * @return True if has a job, false otherwise.
             */
            bool HasJob() const;

            /**
             * Returns the age of the Individual.
             * @return age value.
             */
            int GetAge() const;

            /**
             * Gets the number of cars in the individual.
             * @return number of cars value.
             */
            int GetNumberOfCars() const;


            /**
             * Gets the sex of the individual.
             * @return sex {@link Sex} value.
             */
            Sex GetSex() const;

            /**
             * Gets the race of the individual.
             * @return race {@link Race} value.
             */
            Race GetRace() const;

        private:
            float income;
            int age;
            int numberOfCars;
            Sex sex;
            Race race;
        };
    }
}
