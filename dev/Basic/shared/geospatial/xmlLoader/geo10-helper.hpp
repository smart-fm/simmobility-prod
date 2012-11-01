#pragma once

///\file geo10-helper.hpp
///  This file contains the "helper" namespace, which contains various classes and typedefs useful for
///  working with the geo10 parsers. It is included with the skeleton file, so all classes should have access to it.

#include <map>
#include <set>

#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"

namespace sim_mob {
namespace xml {
namespace helper {

//Was: geo_UniNode_Connectors_type
typedef std::set<std::pair<unsigned long,unsigned long> > UniNodeConnectors;

//Was: geo_MultiNode_Connectors_type
typedef std::map<unsigned long, helper::UniNodeConnectors > MultiNodeConnectors;

///POD type for returning from "Nodes".
struct NodesRes {
	std::set<sim_mob::UniNode*> uniNodes;
	std::set<sim_mob::MultiNode*> multiNodes;
};


}}} //End sim_mob::xml::helper namespace

