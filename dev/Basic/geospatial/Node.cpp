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


}
#endif
