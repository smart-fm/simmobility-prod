/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "Driver.hpp"
#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

namespace sim_mob
{

/**
 * This simple BusDriver class maintains a single, non-looping route with a series of
 *   stops. Most driving behavior is re-used from the Driver class. At bus stops, the
 *   BusDriver will simply pull over to the left-most lane and stop for a length of time.
 *
 * \author Seth N. Hetu
 */
class BusDriver : public sim_mob::Driver {
public:
	BusDriver(sim_mob::Person* parent, sim_mob::MutexStrategy mtxStrat);

	//Overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(frame_t frameNumber);

	//Functionality
	//void setRoute(const BusRoute& route);

	// get distance to bus stop (meter)
	double DistanceToNextBusStop();

	bool isBusApproachingBusStop();

//Basic data
protected:
	//Pointer to the vehicle this (bus) driver is controlling.
	//Vehicle* vehicle;  //NOTE: I'm not sure what the best way is in C++ to say that
	//                   //      the PARENT class maintains a vehicle but we maintain a Bus*.

	//Override the following behavior
	virtual double updatePositionOnLink(DriverUpdateParams& p);

	Bus* bus;

private:
	//BusRoute route;
	const DemoBusStop* nextStop;
	std::vector<DemoBusStop> stops;
	std::vector<DemoBusStop> arrivedStops;
	double waitAtStopMS;


	//Serialization, not implemented
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil){};
	virtual void unpack(UnPackageUtils& unpackageUtil){};

	virtual void packProxy(PackageUtils& packageUtil){};
	virtual void unpackProxy(UnPackageUtils& unpackageUtil){};
#endif

};



}
