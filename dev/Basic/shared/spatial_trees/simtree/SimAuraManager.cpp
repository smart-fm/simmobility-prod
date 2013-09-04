/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "SimAuraManager.hpp"

#include "entities/Agent.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/Lane.hpp"
#include "spatial_trees/shared_funcs.hpp"

using namespace sim_mob;
using namespace sim_mob::temp_spatial;

void sim_mob::SimAuraManager::update(int time_step) {
//	std::cout << "SimAuraManager::update:" << time_step << std::endl;
//	std::cout << "====BEFORE===" << std::endl;

//	if (time_step % 100 == 0)
//	{
//		std::cout << "--------------------------" << std::endl;
//		tree_sim.display();
//	}

//	static int count;
	tree_sim.updateAllInternalAgents();

	//update new agents
//	std::cout << "new_agents:" << new_agents.size() << std::endl;

	for (std::vector<Agent const*>::iterator it = new_agents.begin(); it != new_agents.end(); ++it) {
		Agent* one_ = const_cast<Agent*>(*it);

//		std::cout << "new_agents, ID:" << (*it)->getId() << std::endl;

//		if(one_->getId() == 4051)
//		{
//			std::cout << "one_->getId() is 4051," << one_->can_remove_by_RTREE << std::endl;
//		}

		if (one_->isNonspatial()) {
//			std::cout << "one_->getId() is 4051 is removed" << std::endl;
			continue;
		}

		if (one_->can_remove_by_RTREE == false) {
//			std::cout << "one_->getId() is 4051 is inserted" << std::endl;
			tree_sim.insertAgentBasedOnOD(one_);
		}
	}

	new_agents.clear();

#ifdef SIM_TREE_USE_REBALANCE
	//meausre unbalance
	tree_sim.measureUnbalance(time_step);
#endif

//	std::cout << "====AFTER===" << std::endl;
//	tree_sim.checkLeaf();
//	std::cout << "====AFTER Finished===" << std::endl;

//	tree_sim.display();

//balance processing
//	tree_sim.measureUnbalance();

//	for middle term
//	sim_mob::AuraManager::instance().densityMap.clear();
//	for (std::vector<Entity*>::iterator itr = Agent::all_agents.begin(); itr != Agent::all_agents.end(); ++itr) {
//		Person* an_agent = dynamic_cast<Person*>(*itr);
//		if (!an_agent)
//			continue;
//
////		Driver* test = dynamic_cast<Driver*> (an_agent->getRole());
////		if (!test)
////			continue;
//
//		if (an_agent->isToBeRemoved() == false) {
////			updateDensity(an_agent);
//		}
//	}
//
//	for (std::vector<Entity*>::iterator it = Agent::all_agents.begin(); it != Agent::all_agents.end(); ++it) {
//		updateDensity(dynamic_cast<Agent const*>(*it));
//	}

}

/**
 *Build the Sim-Tree Structure
 */
void sim_mob::SimAuraManager::init() {
#ifdef SIM_TREE_USE_REBALANCE
	//nothing
//	tree_sim.build_tree_structure("data//density_pattern_30K_2");
//	tree_sim.build_tree_structure("shared//spatial_trees//simtree//density_pattern_bugis_auto_study");
	tree_sim.build_tree_structure("shared//spatial_trees//simtree//density_pattern_sg_auto_study");
	tree_sim.init_rebalance_settings();
//	tree_sim.display();
#else
//	tree_sim.build_tree_structure("shared//3rd-party//density_pattern_large_bugis_128");
	tree_sim.build_tree_structure("shared//spatial_trees//simtree//density_pattern_sg_20mins");
//	tree_sim.build_tree_structure("shared//spatial_trees//simtree//density_pattern_bugis_auto_study");
//	tree_sim.display();
#endif

//	first_update = 0;
}

void sim_mob::SimAuraManager::registerNewAgent(Agent const* ag) {
	new_agents.push_back(ag);
}

std::vector<Agent const *> sim_mob::SimAuraManager::agentsInRect(Point2D const & lowerLeft, Point2D const & upperRight) const {
	SimRTree::BoundingBox box;
	box.edges[0].first = lowerLeft.getX();
	box.edges[1].first = lowerLeft.getY();
	box.edges[0].second = upperRight.getX();
	box.edges[1].second = upperRight.getY();

//	std::cout << "Query: " << box.edges[0].first << "," << box.edges[1].first << "," << box.edges[0].second << "," << box.edges[1].second << std::endl;

	return tree_sim.rangeQuery(box);
}

std::vector<Agent const *> sim_mob::SimAuraManager::nearbyAgents(Point2D const & position, Lane const & lane, centimeter_t distanceInFront, centimeter_t distanceBehind) const {
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

//	std::cout << "Query==========" << left << "," << bottom << "," << right << "," << top << std::endl;

	return agentsInRect(lowerLeft, upperRight);
}

std::vector<Agent const *> sim_mob::SimAuraManager::advanaced_agentsInRect(const Point2D& lowerLeft, const Point2D& upperRight, TreeItem* item) const {
	SimRTree::BoundingBox box;
	box.edges[0].first = lowerLeft.getX();
	box.edges[1].first = lowerLeft.getY();
	box.edges[0].second = upperRight.getX();
	box.edges[1].second = upperRight.getY();

	//	std::cout << "Query: " << box.edges[0].first << "," << box.edges[1].first << "," << box.edges[0].second << "," << box.edges[1].second << std::endl;

#ifdef SIM_TREE_BOTTOM_UP_QUERY
	return tree_sim.rangeQuery(box, item);
#else
	return tree_sim.rangeQuery(box);
#endif
}

std::vector<Agent const *> sim_mob::SimAuraManager::advanaced_nearbyAgents(const Point2D& position, const Lane& lane, centimeter_t distanceInFront, centimeter_t distanceBehind, TreeItem* item) const {
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

	//	std::cout << "Query==========" << left << "," << bottom << "," << right << "," << top << std::endl;

	return advanaced_agentsInRect(lowerLeft, upperRight, item);
}

void sim_mob::SimAuraManager::checkLeaf() {
//	std::cout << "=====START====" << std::endl;
//	tree_sim.checkLeaf();
//	std::cout << "====START Finished===" << std::endl;
}
