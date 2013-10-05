//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RDUAuraManager.hpp"

#include "spatial_trees/shared_funcs.hpp"
#include "entities/Agent.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadSegment.hpp"
#include "entities/Person.hpp"


using namespace sim_mob;
using namespace sim_mob::temp_spatial;

bool sim_mob::RDUAuraManager::has_one_agent_du(int agent_id)
{
	return tree_du.has_one_agent(agent_id);
}

void sim_mob::RDUAuraManager::update(int time_step, const std::set<sim_mob::Agent*>& removedAgentPointers)
{
	for (std::set<Entity*>::iterator itr = Agent::all_agents.begin(); itr != Agent::all_agents.end(); ++itr) {
		Agent* an_agent = dynamic_cast<Agent*>(*itr);
		if ((!an_agent) || an_agent->isNonspatial()) {
			continue;
		}

		bool canBeRemoved = (removedAgentPointers.find(an_agent)!=removedAgentPointers.end());

		if (tree_du.has_one_agent(an_agent->getId()) && canBeRemoved) {
			tree_du.remove(an_agent);
		} else if (tree_du.has_one_agent(an_agent->getId()) && !canBeRemoved) {
			tree_du.update(an_agent->getId(), an_agent->xPos, an_agent->yPos);
		} else if (tree_du.has_one_agent(an_agent->getId()) == false && !canBeRemoved) {
			tree_du.insert(an_agent);
		} else {
			std::cout << "---------------------" << std::endl;
			std::cout << "error:" << std::endl;
			std::cout << "an_agent->getId():" << an_agent->getId() << std::endl;
			std::cout << "an_agent->can_remove_by_RTREE:" << canBeRemoved << std::endl;
		}
	}
}

std::vector<Agent const *> sim_mob::RDUAuraManager::agentsInRect(Point2D const & lowerLeft, Point2D const & upperRight, const sim_mob::Agent* refAgent) const
{
	R_tree_DU::BoundingBox box;
	box.edges[0].first = lowerLeft.getX();
	box.edges[1].first = lowerLeft.getY();
	box.edges[0].second = upperRight.getX();
	box.edges[1].second = upperRight.getY();

	return tree_du.query(box);
}

std::vector<Agent const *> sim_mob::RDUAuraManager::nearbyAgents(Point2D const & position, Lane const & lane, centimeter_t distanceInFront, centimeter_t distanceBehind, const sim_mob::Agent* refAgent) const
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

	return agentsInRect(lowerLeft, upperRight, nullptr);

}

