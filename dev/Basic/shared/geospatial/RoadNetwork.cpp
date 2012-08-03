/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "RoadNetwork.hpp"

#include "Node.hpp"
#include "UniNode.hpp"
#include "MultiNode.hpp"
#include "Point2D.hpp"

#include <cmath>

using std::vector;
using std::set;
using namespace sim_mob;


namespace {
//Temporary
int dist(const Point2D& p1, const Point2D& p2) {
	double dx = p2.getX() - p1.getX();
	double dy = p2.getY() - p1.getY();
	double d2 = sqrt(dx*dx + dy*dy);
	return (int)d2;
}

int dist(const Point2D& p1, double xPos, double yPos) {
	double dx = xPos - p1.getX();
	double dy = yPos - p1.getY();
	double d2 = sqrt(dx*dx + dy*dy);
	return (int)d2;
}

} //End anon namespace



Node* sim_mob::RoadNetwork::locateNode(const Point2D& position, bool includeUniNodes, int maxDistCM) const
{
	//First, check the MultiNodes, since these will always be candidates
	int minDist = maxDistCM+1;
	Node* candidate = nullptr;
	for (vector<MultiNode*>::const_iterator it=nodes.begin(); (it!=nodes.end())&&(minDist!=0); it++) {
		int newDist = dist((*it)->location, position);
		if (newDist < minDist) {
			minDist = newDist;
			candidate = *it;
		}
	}

	//Next, check the UniNodes, if the flag is set.
	if (includeUniNodes) {
		for (set<UniNode*>::const_iterator it=segmentnodes.begin(); (it!=segmentnodes.end())&&(minDist!=0); it++) {
			int newDist = dist((*it)->location, position);
			if (newDist < minDist) {
				minDist = newDist;
				candidate = *it;
			}
		}
	}

	return candidate;
}

Node* sim_mob::RoadNetwork::locateNode(double xPos, double yPos, bool includeUniNodes, int maxDistCM) const
{
	//First, check the MultiNodes, since these will always be candidates
	int minDist = maxDistCM+1;
	Node* candidate = nullptr;
	for (vector<MultiNode*>::const_iterator it=nodes.begin(); (it!=nodes.end())&&(minDist!=0); it++) {
		int newDist = dist((*it)->location, xPos, yPos);
		if (newDist < minDist) {
			minDist = newDist;
			candidate = *it;
		}
	}

	//Next, check the UniNodes, if the flag is set.
	if (includeUniNodes) {
		for (set<UniNode*>::const_iterator it=segmentnodes.begin(); (it!=segmentnodes.end())&&(minDist!=0); it++) {
			int newDist = dist((*it)->location, xPos, yPos);
			if (newDist < minDist) {
				minDist = newDist;
				candidate = *it;
			}
		}
	}

	return candidate;
}

