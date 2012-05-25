/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Node.hpp"

unsigned int sim_mob::Node::getID()const {return nodeId;}
void sim_mob::Node::setID(unsigned int id) { nodeId = id; }
#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "conf/simpleconf.hpp"

namespace sim_mob{

void Node::pack(PackageUtils& package, const Node* one_node)
{
	if (one_node == NULL) {
		bool is_NULL = true;
		package.packBasicData(is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package.packBasicData(is_NULL);
	}

	package.packPoint2D(one_node->location);
}

Node* Node::unpack(UnPackageUtils& unpackage)
{
	bool is_NULL = unpackage.unpackBasicData<bool> ();
	if (is_NULL) {
		return NULL;
	}

	 sim_mob::Point2D location = *(unpackage.unpackPoint2D());
	 const sim_mob::RoadNetwork& rn = ConfigParams::GetInstance().getNetwork();
	 return rn.locateNode(location, true);
}
}
#endif
