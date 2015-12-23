/*
 * AMODTypes.hpp
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#pragma once

#include <functional>
#include <vector>
#include <string>

namespace sim_mob {

namespace amod {

/// Stores Positions of objects
struct Position {
    double x, y;
    bool valid;
    Position() : x(0), y(0), valid(true) {}
    Position(double xpos, double ypos) : x(xpos), y(ypos), valid(true) {}

    bool operator==(const Position &other) const {
	return (x == other.x) && (y == other.y) && valid && other.valid;
    }

    bool operator!=(const Position &other) const {
	return !(*this == other);
    }

    int size() const {
	return 2;
    }

    Position &operator=(const Position &rhs) {
	x = rhs.x;
	y = rhs.y;
	return *this;
    }

    double& operator[](unsigned int i) {
	if (i>=2) {
	    throw std::runtime_error("index error");
	}
	return (i == 0) ? x : y;
    }

    double operator[](unsigned int i) const {
	if (i>=2) {
	    throw std::runtime_error("index error");
	}
	return (i == 0) ? x : y;
    }
};

enum ReturnCode{
    FAILED,
    SUCCESS,
    CANNOT_GET_VEHICLE,
    CANNOT_GET_CUSTOMER,
    VEHICLE_NOT_AT_CUSTOMER_LOCATION,
    VEHICLE_DOES_NOT_HAVE_CUSTOMER,
    VEHICLE_CANNOT_BE_DISPATCHED,
    VEHICLE_IS_NOT_FREE,
    CUSTOMER_IS_NOT_FREE,
    SOURCE_EQUALS_DESTINATION,
    ERROR_READING_BOOKINGS_FILE,
    SIMULATOR_IS_NULLPTR,
    LOGGER_IS_NULLPTR,
    NO_PATH_TO_DESTINATION,
    INVALID_STATION_ID,
    ERROR_READING_DEMAND_HIST_FILE,
    CANNOT_OPEN_LOGFILE,
    CUSTOMER_NOT_AT_SOURCE,
};

const std::vector<std::string> kErrorStrings = {
    "Failed",
    "Success",
    "Failed to get vehicle with given Id",
    "Failed to get customer with given Id",
    "Vehicle not at customer location",
    "Vehicle not associated with given customer",
    "Vehicle cannot be dispatched",
    "Vehicle is not free",
    "Customer is not free",
    "Source and Destination are the same",
    "Error Reading Bookings file",
    "Simulator is nullptr",
    "Logger is nullptr",
    "No path to destination",
    "Invalid station id",
    "Error Reading Demand Histogram file",
    "Cannot open logfile",
    "Customer is not at the source node specified by booking"
};
    
}

}

namespace std {
template <>
struct hash<sim_mob::amod::Position>
{
    std::size_t operator()(const sim_mob::amod::Position& p) const
    {
	std::hash<int> hashFn;
	std::size_t hashVal = hashFn(p.x);
	return ( (hashVal << 4) ^ (hashVal >> 28) ^ hashFn(p.y) );
    }
};

template <>
struct hash<std::pair<int, int>>
{
    std::size_t operator()(const std::pair<int,int>& k) const
    {
	using std::size_t;
	using std::hash;

	return ((hash<int>()(k.first)
		 ^ (hash<int>()(k.second) << 1)) >> 1);
    }
};
}

