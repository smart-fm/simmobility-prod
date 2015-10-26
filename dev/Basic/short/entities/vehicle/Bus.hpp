//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file Bus.hpp
 *
 * \author Seth N. Hetu
 */

#pragma once

#include <vector>

#include "entities/Agent.hpp"
#include "BusRoute.hpp"
#include "Vehicle.hpp"

namespace sim_mob {

class Person;
class BusController;
class PackageUtils;
class UnPackageUtils;

/**
 * A bus is similar to a Vehicle, except that it follows a BusRoute and has a passenger count.
 */
class Bus : public sim_mob::Vehicle {
public:
	Bus(const BusRoute& route, const Vehicle* clone, std::string busLine_)
	: Vehicle(*clone), passengerCount(0), busCapacity(40), 
	  route(route), ptCheck(0,0), DistThreshold(2000), busline(busLine_),
	  busNumber(0), timeOfReachingBusStop(0)
	{}

	int getPassengerCount() const { return passengerCount; }
	
	int getBusCapacity() const { return busCapacity; }
	
	void setPassengerCount(int val) { passengerCount = val; }
	
	void setBusNumber(int &n) { busNumber = n; }
	
	int getBusNumber() { return busNumber; }

	double getTimeOfReachingBusStop() { return timeOfReachingBusStop; }
	
	std::vector<const sim_mob::Agent*> passengers_distribition;//added by Meenu
	
	std::string getBusLineID() { return busline; }
	
	std::vector<sim_mob::Person*> passengers_inside_bus;//added by Meenu

private:
	int passengerCount;
	
	int busCapacity;
	
	BusRoute route;
	
	Point ptCheck;// Later connect to Stops, calculate the position to some stops
	
	double DistThreshold;
	
	std::string busline;

	int busNumber;

	double timeOfReachingBusStop;

	friend class BusController;
	
	//Serialization-related friends
	friend class PackageUtils;
	
	friend class UnPackageUtils;
};

} // namespace sim_mob
