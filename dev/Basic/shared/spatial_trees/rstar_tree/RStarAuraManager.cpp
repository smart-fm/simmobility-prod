//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RStarAuraManager.hpp"

#include <cassert>

#include "spatial_trees/shared_funcs.hpp"
#include "entities/Person.hpp"
#include "entities/Agent.hpp"
#include "geospatial/Lane.hpp"

using namespace sim_mob;
using namespace sim_mob::temp_spatial;

void RStarAuraManager::update(int time_step, const std::set<sim_mob::Agent*>& removedAgentPointers)
{
	tree_rstar.Remove(R_tree::AcceptAny(), R_tree::RemoveLeaf());
	assert(tree_rstar.GetSize() == 0);

	for (std::set<Entity*>::iterator itr = Agent::all_agents.begin(); itr != Agent::all_agents.end(); ++itr) {
		Agent* ag = dynamic_cast<Agent*>(*itr);
		if ((!ag) || ag->isNonspatial()) {
			continue;
		}

		if(ag->xPos.get() < 10000000 || ag->yPos.get() < 1000000) {
            Warn() << "A driver's location (x or y) is out of map, X:" << ag->xPos.get() << ",Y:" << ag->yPos.get() << std::endl;
            continue;
		}

		if (removedAgentPointers.find(ag)==removedAgentPointers.end()) {
			//->can_remove_by_RTREE == false) {
			tree_rstar.insert(ag);
		}
	}
}

std::vector<Agent const *> RStarAuraManager::agentsInRect(Point2D const & lowerLeft, Point2D const & upperRight, const sim_mob::Agent* refAgent) const
{
	R_tree::BoundingBox box;
	box.edges[0].first = lowerLeft.getX();
	box.edges[1].first = lowerLeft.getY();
	box.edges[0].second = upperRight.getX();
	box.edges[1].second = upperRight.getY();

	return tree_rstar.query(box);
}

std::vector<Agent const *> RStarAuraManager::nearbyAgents(Point2D const & position, Lane const & lane, centimeter_t distanceInFront, centimeter_t distanceBehind, const sim_mob::Agent* refAgent) const
{
	// Find the stretch of the lane's polyline that <position> is in.
	std::vector<Point2D> const & polyline = lane.getPolyline();
	Point2D p1, p2;
	for (size_t index = 0; index < polyline.size() - 1; index++) {
		p1 = polyline[index];
		p2 = polyline[index + 1];
		if (isInBetween(position, p1, p2))
		break;
	}

	// Adjust <p1> and <p2>.  The current approach is simplistic.  <distanceInFront> and
	// <distanceBehind> may extend beyond the stretch marked out by <p1> and <p2>.
	adjust(p1, p2, position, distanceInFront, distanceBehind);

        if(p1.getX() < 0 || p2.getX() < 0)
        {
            std::vector<Agent const *> empty;
            return empty;
        }


	// Calculate the search rectangle.  We use a quick and accurate method.  However the
	// inaccurancy only makes the search rectangle bigger.
	centimeter_t left = 0, right = 0, bottom = 0, top = 0;
	if (p1.getX() > p2.getX()) {
		left = p2.getX();
		right = p1.getX();
	} else {
		left = p1.getX();
		right = p2.getX();
	}
	if (p1.getY() > p2.getY()) {
		top = p1.getY();
		bottom = p2.getY();
	} else {
		top = p2.getY();
		bottom = p1.getY();
	}

	centimeter_t halfWidth = getAdjacentLaneWidth(lane) / 2;
	left -= halfWidth;
	right += halfWidth;
	top += halfWidth;
	bottom -= halfWidth;

	Point2D lowerLeft(left, bottom);
	Point2D upperRight(right, top);
	return agentsInRect(lowerLeft, upperRight, nullptr);
}

