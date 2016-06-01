//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "SimAuraManager.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Agent.hpp"
#include "geospatial/network/Point.hpp"
#include "geospatial/network/Lane.hpp"
#include "spatial_trees/shared_funcs.hpp"

using namespace sim_mob;
using namespace sim_mob::spatial;

void sim_mob::SimAuraManager::update(int time_step, const std::set<sim_mob::Entity *> &removedAgentPointers)
{
	tree_sim.updateAllInternalAgents(agent_connector_map, removedAgentPointers);

	for (std::vector<Agent const*>::iterator it = new_agents.begin(); it != new_agents.end(); ++it)
	{
		Agent* one_ = const_cast<Agent*> (*it);
		if (one_->isNonspatial())
		{
			continue;
		}

		if (removedAgentPointers.find(one_) == removedAgentPointers.end())
		{
			tree_sim.insertAgentBasedOnOD(one_, agent_connector_map);
		}
	}

	new_agents.clear();
	tree_sim.measureUnbalance(time_step, agent_connector_map);
}

/**
 *Build the Sim-Tree Structure
 */
void sim_mob::SimAuraManager::init()
{
	agent_connector_map.clear();

	tree_sim.buildTreeStructure();
	tree_sim.initRebalanceSettings();
}

void sim_mob::SimAuraManager::registerNewAgent(Agent const* ag)
{
	new_agents.push_back(ag);
}

std::vector<Agent const *> sim_mob::SimAuraManager::agentsInRect(Point const & lowerLeft, Point const & upperRight, const sim_mob::Agent* refAgent) const
{
	//Can we use the optimized bottom-up query?
	if (refAgent)
	{
		std::map<const sim_mob::Agent*, TreeItem*>::const_iterator it = agent_connector_map.find(refAgent);
		if (it != agent_connector_map.end() && it->second)
		{
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

std::vector<Agent const *> sim_mob::SimAuraManager::nearbyAgents(const Point &position, const WayPoint &wayPoint, double distanceInFront, double distanceBehind,
																 const sim_mob::Agent *refAgent) const
{
	//Use the optimized bottom-up query, please read the paper to get more insights on "bottom-up query"
	//The idea is to start the search from the agent's location, instead of from the root node
	if (refAgent)
	{
		std::map<const sim_mob::Agent*, TreeItem*>::const_iterator it = agent_connector_map.find(refAgent);
		if (it != agent_connector_map.end() && it->second)
		{
			return nearbyAgentsBottomUpQuery(position, wayPoint, distanceInFront, distanceBehind, it->second);
		}
	}

	/*
	 * Otherwise, start the search from the root node
	 * The code below is basically build a search rectangle using position and distance
	 * 1) lowerLeft
	 * 2) upperRight
	 */

	// Find the stretch of the poly-line that <position> is in.
	std::vector<PolyPoint> points;	
	
	if(wayPoint.type == WayPoint::LANE)
	{
		points = wayPoint.lane->getPolyLine()->getPoints();
	}
	else
	{
		points = wayPoint.turningPath->getPolyLine()->getPoints();
	}
	
	Point p1, p2;
	for (size_t index = 0; index < points.size() - 1; index++)
	{
		p1 = points[index];
		p2 = points[index + 1];
		if (isInBetween(position, p1, p2))
			break;
	}

	// Adjust <p1> and <p2>.  The current approach is simplistic.  <distanceInFront> and
	// <distanceBehind> may extend beyond the stretch marked out by <p1> and <p2>.
	adjust(p1, p2, position, distanceInFront, distanceBehind);

	if (p1.getX() < 0 || p2.getX() < 0)
	{
		std::vector<Agent const *> empty;
		return empty;
	}

	// Calculate the search rectangle.  We use a quick and accurate method.  However the
	// inaccuracy only makes the search rectangle bigger.
	double left = 0, right = 0, bottom = 0, top = 0;
	if (p1.getX() > p2.getX())
	{
		left = p2.getX();
		right = p1.getX();
	}
	else
	{
		left = p1.getX();
		right = p2.getX();
	}
	if (p1.getY() > p2.getY())
	{
		top = p1.getY();
		bottom = p2.getY();
	}
	else
	{
		top = p2.getY();
		bottom = p1.getY();
	}

	double halfWidth = getAdjacentPathWidth(wayPoint) / 2;
	left -= halfWidth;
	right += halfWidth;
	top += halfWidth;
	bottom -= halfWidth;

	Point lowerLeft(left, bottom);
	Point upperRight(right, top);

	return agentsInRect(lowerLeft, upperRight, nullptr);
}

std::vector<Agent const *> sim_mob::SimAuraManager::agentsInRectBottomUpQuery(const Point& lowerLeft, const Point& upperRight, TreeItem* item) const
{
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

std::vector<Agent const *> sim_mob::SimAuraManager::nearbyAgentsBottomUpQuery(const Point &position, const WayPoint &wayPoint, double distanceInFront, double distanceBehind,
																			  TreeItem* item) const
{
	// Find the stretch of the poly-line that <position> is in.
	std::vector<PolyPoint> points;
	
	if(wayPoint.type == WayPoint::LANE)
	{
		points = wayPoint.lane->getPolyLine()->getPoints();
	}
	else
	{
		points = wayPoint.turningPath->getPolyLine()->getPoints();
	}
	
	Point p1, p2;
	for (size_t index = 0; index < points.size() - 1; index++)
	{
		p1 = points[index];
		p2 = points[index + 1];
		if (isInBetween(position, p1, p2))
		{
			break;
		}
	}

	// Adjust <p1> and <p2>.  The current approach is simplistic.  <distanceInFront> and
	// <distanceBehind> may extend beyond the stretch marked out by <p1> and <p2>.
	adjust(p1, p2, position, distanceInFront, distanceBehind);

	// Calculate the search rectangle.  We use a quick and accurate method.  However the
	// inaccuracy only makes the search rectangle bigger.
	double left = 0, right = 0, bottom = 0, top = 0;
	if (p1.getX() > p2.getX())
	{
		left = p2.getX();
		right = p1.getX();
	}
	else
	{
		left = p1.getX();
		right = p2.getX();
	}
	if (p1.getY() > p2.getY())
	{
		top = p1.getY();
		bottom = p2.getY();
	}
	else
	{
		top = p2.getY();
		bottom = p1.getY();
	}

	double halfWidth = getAdjacentPathWidth(wayPoint) / 2;
	left -= halfWidth;
	right += halfWidth;
	top += halfWidth;
	bottom -= halfWidth;

	Point lowerLeft(left, bottom);
	Point upperRight(right, top);

	return agentsInRectBottomUpQuery(lowerLeft, upperRight, item);
}
