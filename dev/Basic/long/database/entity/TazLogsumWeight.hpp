//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)
/*
 * TazLogsumWeight.hpp
 *
 *  Created on: 25 Jun, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
        class TazLogsumWeight
        {
        public:
        	TazLogsumWeight(int groupLogsum = 0, int individualId = 0, double weight = .0, int householdId = 0);

            virtual ~TazLogsumWeight();

            /**
             * Getters and Setters
             */
            int getGroupLogsum() const;
            int getIndividualId() const;
            double getWeight() const;
            int getHouseholdId() const;

            void setGroupLogsum( int value);
            void setIndividualId( int value );
            void setWeight( double weight );
            void setHouseholdId( int value );

            /**
             * Operator to print the Template data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const TazLogsumWeight& data);

        private:
            friend class TazLogsumWeightDao;

            int groupLogsum;
            int individualId;
            double weight;
            int householdId;
        };
    }
}
