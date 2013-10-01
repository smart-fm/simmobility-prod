//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Statistics.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on June 14, 2013, 11:15 AM
 */
#pragma once

namespace sim_mob {
    
    namespace long_term {

        /**
         * Class for system statistics.
         */
        class Statistics {
        public:

            enum StatsParameter {
                N_BIDS = 0,
                N_BID_RESPONSES = 1,
                N_ACCEPTED_BIDS = 2
            };

            /**
             * Increments the given parameter by 1.
             * @param param to increment.
             */
            static void Increment(StatsParameter param);
            /**
             * Decrements the given parameter by 1.
             * @param param to decrement.
             */
            static void Decrement(StatsParameter param);

            /**
             * Increments the given parameter by given value.
             * @param param to increment.
             * @param value to add.
             */
            static void Increment(StatsParameter param, long value);

            /**
             * Decrements the given parameter by given value.
             * @param param to decrement.
             * @param value to subtract.
             */
            static void Decrement(StatsParameter param, long value);
            
            /**
             * Print out the current statistics.
             */
            static void Print();
        };
    }
}

