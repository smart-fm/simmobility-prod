/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file Bus.hpp
 *
 * \author Seth N. Hetu
 */

#pragma once

#include "Vehicle.hpp"
#include "BusRoute.hpp"


namespace sim_mob {

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

/**
 * A bus is similar to a Vehicle, except that it follows a BusRoute and has a passenger count.
 */
class Bus : public sim_mob::Vehicle {
public:
	Bus(const BusRoute& route, const Vehicle* clone)
	: Vehicle(*clone), passengerCount(0), route(route)
	{}

	BusRoute& getRoute() { return route; }
	int getPassengerCount() const { return passengerCount; }
	void setPassengerCount(int val) { passengerCount = val; }

private:
	int passengerCount;
	BusRoute route;


#ifndef SIMMOB_DISABLE_MPI
public:
	friend class sim_mob::PackageUtils;
	friend class sim_mob::UnPackageUtils;
#endif
};

} // namespace sim_mob
