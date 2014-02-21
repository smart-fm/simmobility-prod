//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <map>

#include "conf/settings/DisableMPI.h"
#include "entities/roles/Role.hpp"
#include "geospatial/streetdir/WayPoint.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "util/DynamicVector.hpp"
#include "DriverUpdateParams.hpp"
#include "DriverFacets.hpp"

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

namespace sim_mob
{

class Agent;
class Person;
class Link;
class RoadSegment;
class Lane;
class Node;
class MultiNode;
class DPoint;
class UpdateParams;

namespace medium
{

class DriverBehavior;
class DriverMovement;
class BusDriverMovement;
/**
 * Medium-term Driver.
 * \author Seth N. Hetu
 * \author Melani Jayasuriya
 * \author Harish Loganathan
 */

class Driver : public sim_mob::Role, public UpdateWrapper<DriverUpdateParams> {
private:
	/** Helper class for grouping a Node and a Point2D together. */
	class NodePoint {
	public:
		Point2D point;
		const Node* node;
		NodePoint() : point(0,0), node(nullptr) {}
	};

public:
	Driver(Agent* parent, MutexStrategy mtxStrat, sim_mob::medium::DriverBehavior* behavior = nullptr, sim_mob::medium::DriverMovement* movement = nullptr);
	virtual ~Driver();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual overrides
	virtual void make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	//medium::DriverUpdateParams params;
	//to be moved to a DriverUpdateParam later
	const Lane* currLane;

protected:
	Vehicle* vehicle;
	NodePoint origin;
	NodePoint goal;

	friend class DriverBehavior;
	friend class DriverMovement;
	friend class BusDriverMovement;
};


} // namespace medium
} // namespace sim_mob
