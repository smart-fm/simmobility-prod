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
	BusRouteTracker() {}
	BusRouteTracker(const BusRouteInfo& routeInfo);

	const BusStop* getNextStopIt() const;
	void updateNextStop();

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

	/**pointer to parent bus driver*/
	sim_mob::medium::BusDriver* parentBusDriver;

	/**list of bus stops for the bus line of this driver*/
	BusRouteTracker routeTracker;
};

}
}
