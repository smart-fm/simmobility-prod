/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file Bus.hpp
 *
 * \author Seth N. Hetu
 */

#pragma once

#include "Vehicle.hpp"
#include "BusRoute.hpp"
#include "entities/BusController.hpp"


namespace sim_mob {

class BusController;
class PackageUtils;
class UnPackageUtils;

/**
 * A bus is similar to a Vehicle, except that it follows a BusRoute and has a passenger count.
 */
class Bus : public sim_mob::Vehicle {
public:
	Bus(const BusRoute& route, const Vehicle* clone)
	: Vehicle(*clone), passengerCount(0), route(route), ptCheck(0,0), DistThreshold(2000), busCapacity(200)
	{}

//	BusRoute& getRoute() { return route; }
	int getPassengerCount() const { return passengerCount; }
	int getBusCapacity() const { return busCapacity; }
	void setPassengerCount(int val) { passengerCount = val; }
	void setBusNumber(int &n) { busNumber = n; }
	int getBusNumber() { return busNumber; }
	//bool isSendToBusController(BusController &busctrller);
	std::vector<const sim_mob::Agent*> passengers;//added by Meenu

private:
	int passengerCount;
	int busCapacity;
	BusRoute route;
	DPoint ptCheck;// Later connect to Stops, calculate the position to some stops
	double DistThreshold;

	int busNumber;

	friend class BusController;
	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;
};

} // namespace sim_mob
