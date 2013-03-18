/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "SimAuraManager.hpp"
#include "spatial_trees/shared_funcs.hpp"

using namespace sim_mob;

void sim_mob::SimAuraManager::update_sim(int time_step)
{
	//	static int count;
	tree_sim.updateAllInternalAgents();

	//update new agents

	for (std::vector<Agent const*>::iterator it = new_agents.begin(); it != new_agents.end(); ++it) {
		Agent* one_ = const_cast<Agent*>(*it);

		if (one_->can_remove_by_RTREE == false)
			tree_sim.insertAgentBasedOnOD(one_);
	}

	new_agents.clear();

#ifdef USE_REBALANCE
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
void sim_mob::SimAuraManager::init_sim()
{
#ifdef USE_REBALANCE
	//nothing
//	tree_sim.build_tree_structure("data//density_pattern_30K_2");
	tree_sim.build_tree_structure("shared//spatial_trees//simtree//density_pattern_large_bugis_auto_study");
//	tree_sim.display();
#else
//	tree_sim.build_tree_structure("shared//3rd-party//density_pattern_large_bugis_128");
	tree_sim.build_tree_structure("shared//3rd-party//density_pattern_large_bugis_128");
//	tree_sim.display();
#endif

//	first_update = 0;
}

void sim_mob::SimAuraManager::registerNewAgent_sim(Agent const* one_agent)
{
	new_agents.push_back(one_agent);

//	std::cout << "New Agent:" << one_agent->xPos << "," << one_agent->yPos << "," << one_agent->getId() << std::endl;
}

std::vector<Agent const *> sim_mob::SimAuraManager::agentsInRect_sim(Point2D const & lowerLeft, Point2D const & upperRight) const
{
	BoundingBox box;
	box.edges[0].first = lowerLeft.getX();
	box.edges[1].first = lowerLeft.getY();
	box.edges[0].second = upperRight.getX();
	box.edges[1].second = upperRight.getY();

	return tree_sim.rangeQuery(box);
}

std::vector<Agent const *> sim_mob::SimAuraManager::nearbyAgents_sim(Point2D const & position, Lane const & lane, centimeter_t distanceInFront, centimeter_t distanceBehind) const
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

//	std::cout << "Query==========" << left << "," << bottom << "," << right << "," << top << std::endl;

	return agentsInRect_sim(lowerLeft, upperRight);
}

void sim_mob::SimAuraManager::checkLeaf()
{
//	std::cout << "=====START====" << std::endl;
//	tree_sim.checkLeaf();
//	std::cout << "====START Finished===" << std::endl;
}
