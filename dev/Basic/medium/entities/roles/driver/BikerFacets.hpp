//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "DriverFacets.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "DriverUpdateParams.hpp"
#include "Biker.hpp"

/*
 * BikerFacets.hpp
 *
 */

namespace sim_mob {
namespace medium
{
class Biker;

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
	 * Foo y;
	 * Foo x(y);      // is copy-construction
	 * Foo x = y;     // is semantically assignment, but the compiler elides the assignment and does copy-construction, equivalent to the above
	 * Foo x; x = y;  // is default-construction followed by assignment, different (and possibly less efficient) than the above
	 *
	 * This assignment overload is required because we do something similar to
	 * the third case for BusRouteTracker. The default assignment operator does
	 * a member wise assignment. Since this class has a vector and an iterator to
	 * its vector as its members, the assignment of the iterator done by the
	 * default assignment operator is incorrect (invalid). This assignment is
	 * handled explicitly by this function.
	 *
	 * @param rhsTracker rhs of assignment
	 * @return reference to lhs of assignment after executing this function
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
	void printBusRoute();

private:
	/**
	 * iterator to the vector of bus stops representing the next stop for the
	 * bus driver to (possibly) serve
	 */
	std::vector<const BusStop*>::iterator nextStopIt;
};

/**
 * Behavior facet of Biker role
 * \author Harish Loganathan
 */
class BikerBehavior: public DriverBehavior {
public:
	explicit BikerBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~BikerBehavior();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	sim_mob::medium::Biker* getParentBiker() const {
		return parentBiker;
	}

	void setParentBiker(sim_mob::medium::Biker* parentBiker) {
		if(!parentBiker) { throw std::runtime_error("parentBiker cannot be NULL"); }
		this->parentBiker = parentBiker;
	}

protected:
	sim_mob::medium::Biker* parentBiker;
};

/**
 * Movement facet of Biker role
 * \author Harish Loganathan
 */
class BikerMovement: public DriverMovement {
public:
	explicit BikerMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~BikerMovement();

	//Virtual overrides
	virtual void frame_init();

	sim_mob::medium::Biker* getParentBiker() const {
		return parentBiker;
	}

	void setParentBiker(sim_mob::medium::Biker* parentBiker) {
		if(!parentBiker) {
			throw std::runtime_error("parentBiker cannot be NULL");
		}
		this->parentBiker = parentBiker;
	}

protected:
	/**pointer to parent bus driver*/
	sim_mob::medium::Biker* parentBiker;
};

}
}

