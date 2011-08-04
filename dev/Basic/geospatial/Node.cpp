/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Node.hpp"
#include "LaneConnector.hpp"
#include "RoadSegment.hpp"

using std::vector;
using namespace sim_mob;


vector<LaneConnector*> sim_mob::Node::getConnectors(const Link* from) const
{
	//Simple case
	if (!from) {
		return connectors;
	}

	//Else, search.
	vector<LaneConnector*> res;
	for (vector<LaneConnector*>::const_iterator it=connectors.begin(); it!=connectors.end(); it++) {
		if ((*it)->getLaneFrom().first->getLink()==from) {
			res.push_back(*it);
		}
	}
	return res;
}


vector<RoadSegment*> sim_mob::Node::getItemsAt() const
{
	return itemsAt;
}

