//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RoadNetwork.hpp"

#include "Node.hpp"
#include "UniNode.hpp"
#include "MultiNode.hpp"
#include "Point2D.hpp"
#include "util/GeomHelpers.hpp"
#include <cmath>

using std::set;
using std::pair;
using std::vector;
using namespace sim_mob;


namespace {


//Sort RoadSegments by ID
struct RS_ID_Sorter {
  bool operator() (const sim_mob::RoadSegment* a, const sim_mob::RoadSegment* b) {
	  return a->getSegmentID() < b->getSegmentID();
  }
};

} //End anon namespace


void sim_mob::RoadNetwork::ForceGenerateAllLaneEdgePolylines(sim_mob::RoadNetwork& rn)
{
	//Set of road segments, sorted by ID.
	set<RoadSegment*, RS_ID_Sorter> cachedSegments;

	//Add all RoadSegments to our list of cached segments.
	for (set<UniNode*>::const_iterator it = rn.getUniNodes().begin(); it != rn.getUniNodes().end(); it++) {
		//TODO: Annoying const-cast
		vector<const RoadSegment*> segs = (*it)->getRoadSegments();
		for (vector<const RoadSegment*>::const_iterator segIt=segs.begin(); segIt!=segs.end(); segIt++) {
			cachedSegments.insert(const_cast<RoadSegment*>(*segIt));
		}
	}
	for (vector<MultiNode*>::const_iterator it = rn.getNodes().begin(); it != rn.getNodes().end(); it++) {
		set<RoadSegment*> segs = (*it)->getRoadSegments();
		cachedSegments.insert(segs.begin(), segs.end());
	}

	//Now retrieve lane edge line zero (which will trigger generation of all other lane/edge lines).
	for (std::set<RoadSegment*>::const_iterator it = cachedSegments.begin(); it != cachedSegments.end(); it++) {
		(*it)->getLaneEdgePolyline(0);
	}
}




Node* sim_mob::RoadNetwork::locateNode(const Point2D& position, bool includeUniNodes, int maxDistCM) const
{
//	std::cout << "Finding a node (from a total of " << nodes.size() <<") at position [" << position.getX() << " , " << position.getY() << "]  => " << std::endl;
	//First, check the MultiNodes, since these will always be candidates
	int minDist = maxDistCM+1;
	Node* candidate = nullptr;
	for (vector<MultiNode*>::const_iterator it=nodes.begin(); (it!=nodes.end())&&(minDist!=0); it++) {
//		std::cout << "Checking the node at [" << (*it)->location.getX() << " , " << (*it)->location.getY() << std::endl;
		int newDist = sim_mob::dist((*it)->location, position);
		if (newDist < minDist) {
			minDist = newDist;
			candidate = *it;
		}
	}

	//Next, check the UniNodes, if the flag is set.
	if (includeUniNodes) {
		for (set<UniNode*>::const_iterator it=segmentnodes.begin(); (it!=segmentnodes.end())&&(minDist!=0); it++) {
			int newDist = sim_mob::dist((*it)->location, position);
			if (newDist < minDist) {
				minDist = newDist;
				candidate = *it;
			}
		}
	}
//	std::cout << candidate << std::endl;
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


