//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <set>

#include "metrics/Length.hpp"

namespace sim_mob {

//Forward declarations
class Agent;
class Lane;
class Point2D;

struct TreeItem;

/**
 * Parent (abstract) class for new tree functionality.
 */
class TreeImpl {
public:
	virtual ~TreeImpl() {}

	///Perform any necessary initialization required by this Tree. Called once, after construction.
	virtual void init() = 0;

	///Update the structure.
	//Note: The pointers in removedAgentPointers will be deleted after this time tick; do *not*
	//      save them anywhere.
	virtual void update(int time_step, const std::set<sim_mob::Agent*>& removedAgentPointers) = 0;

	///Return the Agents within a given rectangle.
	virtual std::vector<Agent const *> agentsInRect(const Point2D& lowerLeft, const Point2D& upperRight, const sim_mob::Agent* refAgent) const = 0;

	///Return Agents near to a given Position, with offsets (and Lane) taken into account.
	virtual std::vector<Agent const *> nearbyAgents(const Point2D& position, const Lane& lane, centimeter_t distanceInFront, centimeter_t distanceBehind, const sim_mob::Agent* refAgent) const = 0;

	///Return the Agents within a given rectangle.
	//virtual std::vector<Agent const *> advanced_agentsInRect(const Point2D& lowerLeft, const Point2D& upperRight, TreeItem* item) const = 0;

	///Return Agents near to a given Position, with offsets (and Lane) taken into account.
	//virtual std::vector<Agent const *> advanced_nearbyAgents(const Point2D& position, const Lane& lane, centimeter_t distanceInFront, centimeter_t distanceBehind, TreeItem* item) const = 0;

	///Register a new Agent, so that the spatial index is aware of this person.
	virtual void registerNewAgent(const Agent* ag) = 0;
};



}
