/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Utils.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on June 12, 2013, 4:59 PM
 */

#include "Utils.hpp"

using namespace sim_mob;

// Thread local random numbers. 
boost::thread_specific_ptr<boost::mt19937> floatProvider;
boost::thread_specific_ptr<boost::mt19937> intProvider;

inline void InitRandomProvider(boost::thread_specific_ptr<boost::mt19937>& provider) {
    // The first time called by the current thread then just create one.
    if (!provider.get()) {
        provider.reset(new boost::mt19937(static_cast<unsigned> (std::time(0))));
    }
}

float Utils::GenerateFloat(float min, float max) {
    InitRandomProvider(floatProvider);
    boost::uniform_real<float> distribution(min, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > gen(*(floatProvider.get()), distribution);
    return gen();
}

int Utils::GenerateInt(int min, int max) {
    InitRandomProvider(intProvider);
    boost::uniform_int<int> distribution(min, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<int> > gen(*(intProvider.get()), distribution);
    return gen();
}