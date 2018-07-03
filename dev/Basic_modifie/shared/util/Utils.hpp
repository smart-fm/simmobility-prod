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

#include <ctime>
#include <vector>
#include <list>
#include <string>
#include <utility>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/random.hpp>
#include <boost/nondet_random.hpp>
#include "boost/generator_iterator.hpp"
#include <stdexcept>
#include "boost/lexical_cast.hpp"

namespace sim_mob {

    class Utils {
    public:

        /**
         * Generates a new float value.
         * @param min minimum limit.
         * @param max maximum limit.
         * @return the generated value. 
         */
        static float generateFloat(float min, float max);

        static double uRandom();
        static double nRandom(double mean, double stddev);

        /**
         * Generates a new integer value.
         * @param min limit.
         * @param max limit.
         * @return the generated value. 
         */
        static int generateInt(int min, int max);

        /**
         * Convert argc/argv into a vector of strings representing each argument.
         */
        static std::vector<std::string> parseArgs(int argc, char* argv[]);

        /**
         * Merges log files. 
         */
        static void printAndDeleteLogFiles(const std::list<std::string>& logFileNames,std::string outputFileName="out.txt");

        //Helper for XML parsing. Source value looks like this: "3000 : 6000", spaces optional.
        //\todo This is mostly in the wrong place; our whole "util" directory needs some reorganization.
        static std::pair<double, double> parseScaleMinmax(const std::string& src);

        /**
         * Restricts a value to be within a specified range.
         * @param value to clamp
         * @param min The minimum value. 
         * @param max The maximum value. 
         * @return If value is less than min, min will be returned,
         * If value is greater than max, max will be returned, otherwise returns
         * value.
         */
        template<typename T>
        static T clamp(const T& value, const T& min, const T& max) {
            return (value < min) ? min : (value > max) ? max : value;
        }

        /**
         * Converts the given data to string.
         * 
         * If the type is not primitive type then
         * it must implement the operator:
         * 
         * friend ostream& operator<<(std::ostream& strm, const MyType& data);
         * 
         * @param data to convert to string.
         * @return string with data.
         */
        template<typename T>
        static std::string toStr(const T& data) {
            std::ostringstream stream;
            stream << data;
            return stream.str();
        }

        /**
         * Converts meter to feet.
         * @param meter value to convert.
         * @return value in feet.
         */
        static double toFeet(const double meter);

        /**
         * Converts feet to meter.
         * @param feet value to convert.
         * @return value in meter.
         */
        static double toMeter(const double feet);
        typedef boost::shared_ptr<boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > > ND_PTR;
        template<typename T>
        static ND_PTR initDistribution(std::pair<T , T > range){

        	//normal distribution
        	boost::mt19937 rng; // I don't seed it on purpouse (it's not relevant)
        	boost::normal_distribution<> nd(range.first, range.second);
        	boost::shared_ptr<boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > > distributionPtr;
        	distributionPtr.reset(new boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > (rng, nd));
        	return distributionPtr;
        }
        /**
         * Converts the given value (in centimeters) to meters.
         * @param cmValue
         * @return value in meters
         */
        template<typename T> static T cmToMeter(const T& cmValue){
            return (T)(cmValue/(T)100);
        }
        
        /**
         * Converts given string to defined type.
         * This function throws exception in case of error during the conversion.
         * @param value to convert.
         * @return value in the defined type if the conversion is valid 
         * otherwise an exception is thrown.
         */
        template<typename T> static T cast(const std::string& value) {
            try {
                return boost::lexical_cast<T>(value);
            } catch (boost::bad_lexical_cast&) {
                throw std::runtime_error("Cannot convert <" + value + ">");
            }
        }
        
        /**
         * Converts given string to defined type.
         * This function does not throws exception in the case of error during the conversion. 
         * It returns the given errorValue instead.
         * @param value to convert.
         * @param errorValue value to return in the case of error.
         * @return value in the defined type or the errorValue if conversion has failed.
         */
        template<typename T> static T cast(const std::string& value, const T& errorValue) {
            try {
                return boost::lexical_cast<T>(value);
            } catch (boost::bad_lexical_cast&) {
            }
            return errorValue;
        }

        /**
         *  convert text string to double array
         *  @param str text string like "1 2 3" or "1,2,3"
         *  @array store extracted numbers from text
         */
        static void convertStringToArray(std::string& str,std::vector<double>& array);

        static double urandom();
        static int brandom(double p);
    	///	extract id string from aimsun id
    	static std::string getNumberFromAimsunId(std::string &aimsunid);

    	/**
    	 * convert longitude,latitude into x,y coordinate in WGS84
    	 * @param x hold longitude as input and also store final x coordinate after conversion
    	 * @param y hold latitude as input and also store final y coordinate after conversion
    	 */
    	static void convertWGS84_ToUTM(double& x, double& y);
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
