//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Utils.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on June 12, 2013, 4:59 PM
 */
#pragma once

#include <vector>
#include <list>
#include <string>
#include <utility>
#include <sstream>
//#include <sys/time.h>

//NOTE: This is a Linux-only class. We need to ifdef it out if not available 
//      (or move it to entities/profile/ProfileBuilder as a static method.)
#include <sys/time.h>

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

        /**
         * Convert argc/argv into a vector of strings representing each argument.
         */
        static std::vector<std::string> ParseArgs(int argc, char* argv[]);

        /**
         *
         */
        static void PrintAndDeleteLogFiles(const std::list<std::string>& logFileNames);

        //Helper for XML parsing. Source value looks like this: "3000 : 6000", spaces optional.
        //\todo This is mostly in the wrong place; our whole "util" directory needs some reorganization.
        static std::pair<double, double> parse_scale_minmax(const std::string& src);

        /**
         * Convert any kind of number format into a string.
         * @param number input
         * @return to a string
         */
        template <typename T>
        static std::string numberToString(const T& number)
        {
        	std::ostringstream stream;
            stream << number;
            return stream.str();
        }
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
        void start();

        /**
         * Stops the watch.
         */
        void stop();
        
        /**
         * Gets the time taken between the last Start-Stop call.
         * @return time in seconds or -1 if the watch is running.
         */
        double getTime() const;
        
    private:
        time_t now;
        time_t end;
        volatile bool running;
    };
}
