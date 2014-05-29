//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "DriverFacets.hpp"
#include "entities/misc/BusTrip.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "DriverUpdateParams.hpp"
#include "BusDriver.hpp"

/*
 * BusDriverFacets.hpp
 *
 */

namespace sim_mob {
namespace medium
{
class BusDriver;

/**
 * Helper class to track the progress of the bus along its route
 * \author Harish Loganathan
 */
class BusRouteTracker : public sim_mob::BusRouteInfo {
public:
	BusRouteTracker() : BusRouteInfo("") {}
	BusRouteTracker(const BusRouteInfo& routeInfo);
	BusRouteTracker(const BusRouteTracker& routeTracker);

	/**
	 * Operator overload for assignment.
	 * \note:
	 * Foo x(y);      // is copy-construction
	 * Foo x = y;     // is semantically assignment, but the compiler elides the assignment and does copy-construction, equivalent to the above
	 * Foo x; x = y;  // is default-construction followed by assignment, different (and possibly less efficient) than the above
	 *
	 * This is assignment overload is required because we do something similar
	 * the third case for BusRouteTracker. The default assignment operator does
	 * a member wise assignment. Since this class has a vector and an iterator to
	 * its the vector as its members, the assignment of the iterator done by the
	 * default assignment operator is incorrect (invalid). This assignment is
	 * handled explicitly by this function.
	 */
	BusRouteTracker& operator=(const BusRouteTracker& rhsTracker);

	/**
	 * gets the bus stop pointed by nextStopIt
	 * @return the bus stop pointed by nextStopIt if nextStopIt points to a
	 * valid stop; nullptr otherwise.
	 */
	const BusStop* getNextStop() const;
	/**
	 * increments the nextStopIt to point to the next stop along the route
	 */
	void updateNextStop();

	/**
	 * print bus route information
	 */
	void printBusRoue();

private:
	/**
	 * iterator to the vector of bus stops representing the next stop for the
	 * bus driver to (possibly) serve
	 */
	std::vector<const BusStop*>::iterator nextStopIt;
};

/**
 * Behavior facet of BusDriver role
 * \author Harish Loganathan
 */
class BusDriverBehavior: public DriverBehavior {
public:
	explicit BusDriverBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~BusDriverBehavior();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	sim_mob::medium::BusDriver* getParentBusDriver() const {
		return parentBusDriver;
	}

	void setParentBusDriver(sim_mob::medium::BusDriver* parentBusDriver) {
		if(!parentBusDriver) {
			throw std::runtime_error("parentBusDriver cannot be NULL");
		}
		this->parentBusDriver = parentBusDriver;
	}

protected:
	sim_mob::medium::BusDriver* parentBusDriver;
};

/**
 * Movement facet of BusDriver role
 * \author Harish Loganathan
 */
class BusDriverMovement: public DriverMovement {
public:
	explicit BusDriverMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~BusDriverMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	sim_mob::medium::BusDriver* getParentBusDriver() const {
		return parentBusDriver;
	}

	void setParentBusDriver(sim_mob::medium::BusDriver* parentBusDriver) {
		if(!parentBusDriver) {
			throw std::runtime_error("parentBusDriver cannot be NULL");
		}
		this->parentBusDriver = parentBusDriver;
	}

protected:
	virtual bool initializePath();

	virtual const sim_mob::Lane* getBestTargetLane(
			const sim_mob::SegmentStats* nextSegStats,
			const sim_mob::SegmentStats* nextToNextSegStats);

	/**
	 * In addition to the functionality of the base Driver class, bus drivers
	 * must check if they have to serve a stop at the end of this segment stats
	 * before moving to the next segment stats
	 * @param params driver update params for current tick
	 * @return true if successfully moved to next segment; false otherwise
	 */
	virtual bool moveToNextSegment(DriverUpdateParams& params);

	/**pointer to parent bus driver*/
	sim_mob::medium::BusDriver* parentBusDriver;

	/**list of bus stops for the bus line of this driver*/
	BusRouteTracker routeTracker;
};

}
}
