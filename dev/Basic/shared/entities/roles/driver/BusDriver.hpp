/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "Driver.hpp"
#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"
#include "DriverUpdateParams.hpp"
#include <vector>
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

	// get distance to bus stop (meter)
	double distanceToNextBusStop() const;

	// get distance to bus stop of particular segment (meter)
	double getDistanceToBusStopOfSegment(const RoadSegment& roadSegment) const;

	bool isBusFarawayBusStop() const;
	bool isBusApproachingBusStop() const;
	bool isBusArriveBusStop() const;
	bool isBusLeavingBusStop() const;
	void busAccelerating(DriverUpdateParams& p);
	mutable double lastTickDistanceToBusStop;

	std::vector<sim_mob::BusStop *> findBusStopInPath(const std::vector<const sim_mob::RoadSegment*>& path) const;

	double getPositionX() const;
	double getPositionY() const;


//Basic data
protected:
	//Override the following behavior
//	virtual double updatePositionOnLink(DriverUpdateParams& p);
	virtual double linkDriving(DriverUpdateParams& p);

	Bus* bus;

private:
	//BusRoute route;
	const DemoBusStop* nextStop;
	std::vector<DemoBusStop> stops;
	std::vector<DemoBusStop> arrivedStops;
	double waitAtStopMS;
	std::vector<sim_mob::BusStop *> busStops;


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
