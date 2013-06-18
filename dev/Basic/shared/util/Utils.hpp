/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Utils.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on June 12, 2013, 4:59 PM
 */
#pragma once

#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/random.hpp>

namespace sim_mob {

    class Utils {
    public:

        /**
         * Generates a new float value.
         * @param min minimum limit.
         * @param max maximum limit.
         * @return the generated value. 
         */
        static float GenerateFloat(float min, float max);

        /**
         * Generates a new integer value.
         * @param min limit.
         * @param max limit.
         * @return the generated value. 
         */
        static int GenerateInt(int min, int max);
    };

    /**
     * This class measures the running time of something.
     */
    class StopWatch {
    public:
        StopWatch();

        /**
         * Starts the watch.
         */
        void Start();

        /**
         * Stops the watch.
         */
        void Stop();
        
        /**
         * Gets the time taken between the last Start-Stop call.
         * @return time in seconds or -1 if the watch is running.
         */
        double GetTime();
        
    private:
        time_t now;
        time_t end;
        volatile bool running;
    };
}