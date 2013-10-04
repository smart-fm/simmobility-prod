//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "SimAuraManager.hpp"

#include "entities/Agent.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/Lane.hpp"
#include "spatial_trees/shared_funcs.hpp"

using namespace sim_mob;
using namespace sim_mob::temp_spatial;

void sim_mob::SimAuraManager::update(int time_step, const std::set<sim_mob::Agent*>& removedAgentPointers) {
	tree_sim.updateAllInternalAgents(agent_connector_map, removedAgentPointers);

	for (std::vector<Agent const*>::iterator it = new_agents.begin(); it != new_agents.end(); ++it) {
		Agent* one_ = const_cast<Agent*>(*it);
		if (one_->isNonspatial()) {
			continue;
		}

		if (removedAgentPointers.find(one_)==removedAgentPointers.end()) {
			tree_sim.insertAgentBasedOnOD(one_, agent_connector_map);
		}
	}

	new_agents.clear();

#ifdef SIM_TREE_USE_REBALANCE
	//meausre unbalance
	tree_sim.measureUnbalance(time_step);
#endif

}

/**
 *Build the Sim-Tree Structure
 */
void sim_mob::SimAuraManager::init() {
	agent_connector_map.clear();

#ifdef SIM_TREE_USE_REBALANCE
	//nothing
	tree_sim.build_tree_structure("shared//spatial_trees//simtree//density_pattern_sg_auto_study");
	tree_sim.init_rebalance_settings();
#else
	tree_sim.build_tree_structure("shared//spatial_trees//simtree//density_pattern_sg_20mins");
#endif

}

void sim_mob::SimAuraManager::registerNewAgent(Agent const* ag) {
	new_agents.push_back(ag);
}

std::vector<Agent const *> sim_mob::SimAuraManager::agentsInRect(Point2D const & lowerLeft, Point2D const & upperRight, const sim_mob::Agent* refAgent) const {
	//Can we use the optimized bottom-up query?
	if (refAgent) {
		std::map<const sim_mob::Agent*, TreeItem*>::const_iterator it = agent_connector_map.find(refAgent);
		if (it!=agent_connector_map.end() && it->second) {
			return agentsInRectBottomUpQuery(lowerLeft, upperRight, it->second);
		}
	}

	SimRTree::BoundingBox box;
	box.edges[0].first = lowerLeft.getX();
	box.edges[1].first = lowerLeft.getY();
	box.edges[0].second = upperRight.getX();
	box.edges[1].second = upperRight.getY();

	return tree_sim.rangeQuery(box);
}

std::vector<Agent const *> sim_mob::SimAuraManager::nearbyAgents(Point2D const & position, Lane const & lane, centimeter_t distanceInFront, centimeter_t distanceBehind, const sim_mob::Agent* refAgent) const {
	//Can we use the optimized bottom-up query?
	if (refAgent) {
		std::map<const sim_mob::Agent*, TreeItem*>::const_iterator it = agent_connector_map.find(refAgent);
		if (it!=agent_connector_map.end() && it->second) {
			return nearbyAgentsBottomUpQuery(position, lane, distanceInFront, distanceBehind, it->second);
		}
	}

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

std::vector<Agent const *> sim_mob::SimAuraManager::agentsInRectBottomUpQuery(const Point2D& lowerLeft, const Point2D& upperRight, TreeItem* item) const {
	SimRTree::BoundingBox box;
	box.edges[0].first = lowerLeft.getX();
	box.edges[1].first = lowerLeft.getY();
	box.edges[0].second = upperRight.getX();
	box.edges[1].second = upperRight.getY();

#ifdef SIM_TREE_BOTTOM_UP_QUERY
	return tree_sim.rangeQuery(box, item);
#else
	return tree_sim.rangeQuery(box);
#endif
}

std::vector<Agent const *> sim_mob::SimAuraManager::nearbyAgentsBottomUpQuery(const Point2D& position, const Lane& lane, centimeter_t distanceInFront, centimeter_t distanceBehind, TreeItem* item) const {
	// Find the stretch of the lane's polyline that <position> is in.
	std::vector<Point2D> const & polyline = lane.getPolyline();
	Point2D p1, p2;
	for (size_t index = 0; index < polyline.size() - 1; index++) {
		p1 = polyline[index];
		p2 = polyline[index + 1];
		if (isInBetween(position, p1, p2)) {
			break;
		}
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


	return agentsInRectBottomUpQuery(lowerLeft, upperRight, item);
}

