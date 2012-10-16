/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "Driver.hpp"
#include "entities/vehicle/BusRoute.hpp"
#include <vector>

namespace sim_mob
{

//Forward declarations
class DriverUpdateParams;
class PackageUtils;
class UnPackageUtils;
class BusStop;
class Person;


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

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(frame_t frameNumber);

	// get distance to bus stop (meter)
	double distanceToNextBusStop() const;

	// get distance to bus stop of particular segment (meter)
	double getDistanceToBusStopOfSegment(const RoadSegment* rs) const;

	bool isBusFarawayBusStop() const;
	bool isBusApproachingBusStop() const;
	bool isBusArriveBusStop() const;
	bool isBusLeavingBusStop() const;
	void busAccelerating(DriverUpdateParams& p);
	std::vector<const sim_mob::BusStop*> findBusStopInPath(const std::vector<const sim_mob::RoadSegment*>& path) const;

	double getPositionX() const;
	double getPositionY() const;
	Vehicle* initializePath_bus(bool allocateVehicle);

	mutable double lastTickDistanceToBusStop;
	Shared<BusStop*> lastVisited_BusStop; // can get some passenger count, passenger information and busStop information
	Shared<int> lastVisited_BusStopSequenceNum; // last visited busStop sequence number m, reset by BusDriver, What Time???(needed for query the last Stop m -->realStop Times)
	Shared<unsigned int> real_DepartureTime; // set by BusController, reset once stop at only busStop j (j belong to the small set of BusStops)
	Shared<unsigned int> real_ArrivalTime; // set by BusDriver, reset once stop at any busStop

protected:
	//Override the following behavior
	virtual double linkDriving(DriverUpdateParams& p);

//Basic data
private:
	//BusRoute route;
	const DemoBusStop* nextStop;
	std::vector<DemoBusStop> stops;
	std::vector<DemoBusStop> arrivedStops;
	double waitAtStopMS;
	std::vector<const sim_mob::BusStop*> busStops;


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
