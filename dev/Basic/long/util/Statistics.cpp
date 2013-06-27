/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Statistics.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on June 14, 2013, 11:15 AM
 */
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include "Statistics.hpp"
#include "logging/Log.hpp"
#include "Common.hpp"

using std::endl;
using std::map;
using std::pair;
using std::string;
using namespace sim_mob;
using namespace sim_mob::long_term;

typedef boost::unordered_map<int, long> StatsMap;
typedef pair<int, long> StatsMapEntry;

StatsMap statistics;
boost::shared_mutex statisticsMutex;

inline string ToString(Statistics::StatsParameter param) {
    switch (param) {
        case Statistics::N_BIDS: return "Total number of bids";
        case Statistics::N_ACCEPTED_BIDS: return "Total number of accepted bids";
        case Statistics::N_BID_RESPONSES: return "Total number of bid responses";
        default: return "";
    }
}

void Statistics::Increment(Statistics::StatsParameter param) {
    Increment(param, 1);
}

void Statistics::Decrement(Statistics::StatsParameter param) {
    Decrement(param, 1);
}

void Statistics::Decrement(Statistics::StatsParameter param, long value) {
    Increment(param, value * (-1));
}

void Statistics::Increment(Statistics::StatsParameter param, long value) {
	boost::upgrade_lock<boost::shared_mutex> up_lock(statisticsMutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    StatsMap::iterator mapItr = statistics.find(param);
    if (mapItr != statistics.end()) {
        (mapItr->second) += value;
    } else {
        statistics.insert(StatsMapEntry(param, value));
    }
}

void Statistics::Print() {
	boost::upgrade_lock<boost::shared_mutex> up_lock(statisticsMutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    for (StatsMap::iterator itr = statistics.begin(); itr != statistics.end(); itr++) {
        string paramName = ToString((Statistics::StatsParameter)(itr->first));
        long value = itr->second;
        LogOut(paramName << ": " << value << endl);
    }
}
