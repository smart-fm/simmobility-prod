/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "RStarAuraManager.hpp"

#include <cassert>

#include "spatial_trees/shared_funcs.hpp"
#include "entities/Person.hpp"
#include "entities/Agent.hpp"
#include "geospatial/Lane.hpp"

using namespace sim_mob;
using namespace sim_mob::temp_spatial;

void RStarAuraManager::update(int time_step)
{
	// cleanup the tree because we are going to rebuild it.
	tree_rstar.Remove(R_tree::AcceptAny(), R_tree::RemoveLeaf());
	assert(tree_rstar.GetSize() == 0);

	if (Agent::all_agents.empty()) {
		return;
	}

	for (std::vector<Entity*>::iterator itr = Agent::all_agents.begin(); itr != Agent::all_agents.end(); ++itr) {
		Agent* ag = dynamic_cast<Agent*>(*itr);
		if ((!ag) || ag->isNonspatial()) {
			continue;
		}

		if (ag->can_remove_by_RTREE == false) {
			tree_rstar.insert(ag);
		}
	}

//	boost::unordered_set<Entity const *> agents(Agent::all_agents.begin(), Agent::all_agents.end());
//
//	// We populate the tree incrementally by finding the agent that was nearest to the agent
//	// that was most recently inserted into the tree.  This will increase the chance that the
//	// agents in non-leaf nodes are close to each other, and therefore the overlaps of non-leaf
//	// nodes are not large.  Querying will be faster if the overlaps is small.
//	Agent const * agent = dynamic_cast<Agent const*>(*agents.begin());
//	if (!agent) {
//		throw std::runtime_error("all_agents is somehow storing an entity.");
//	}
//
//	sim_mob::AuraManager::instance().densityMap.clear(); //the following while loop counts again
//	while (agents.size() > 1) {
//		agents.erase(agent);
//
//		if (agent->isToBeRemoved() == false) {
//			tree_rstar.insert(agent);
//			updateDensity(agent);
//		}
//
//		agent = nearest_agent(agent, agents);
//	}

//	if (agent->isToBeRemoved() == false) {
//		tree_rstar.insert(agent);    // insert the last agent into the tree.
//		updateDensity(agent);
//	}

//	assert(tree_rstar.GetSize() == Agent::all_agents.size());

}

std::vector<Agent const *> RStarAuraManager::agentsInRect(Point2D const & lowerLeft, Point2D const & upperRight) const
{
	R_tree::BoundingBox box;
	box.edges[0].first = lowerLeft.getX();
	box.edges[1].first = lowerLeft.getY();
	box.edges[0].second = upperRight.getX();
	box.edges[1].second = upperRight.getY();
	return tree_rstar.query(box);
}

std::vector<Agent const *> RStarAuraManager::nearbyAgents(Point2D const & position, Lane const & lane, centimeter_t distanceInFront, centimeter_t distanceBehind) const
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
	return agentsInRect(lowerLeft, upperRight);
}

