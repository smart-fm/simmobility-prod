#pragma once

///\file geo10-helper.hpp
///  This file contains the "helper" namespace, which contains various classes and typedefs useful for
///  working with the geo10 parsers. It is included with the skeleton file, so all classes should have access to it.

#include <map>
#include <set>

#include "geospatial/Node.hpp"
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



///The "Bookkeeping" class exists as an optimization for lookup (the other option is to propagate IDs back to the RoadNetwork level).
///  A new object of this class type is passed by reference to all relevant classes in the parser construction phase.
///  Note that this class cannot optimize items which are referenced by ID *before* they are created.
///This class is not thread-safe, but it would be fairly simple to make it so (via a multiple-readers/single-writer paradigm; see
///  Shared<>'s Locked<> version for sample code). Anyway, for now we are not reading the road network in parallel.
class Bookkeeping {
public:
	void addNode(const sim_mob::Node* node) {
		unsigned long id = node->getID();
		if (nodeLookup.count(id)>0) {
			throw std::runtime_error("Node already registered with bookkeeper.");
		}
		nodeLookup[id] = node;
	}
	const sim_mob::Node* getNode(unsigned long id) const {
		std::map<unsigned long, const sim_mob::Node*>::const_iterator it = nodeLookup.find(id);
		if (it!=nodeLookup.end()) {
			return it->second;
		}
		throw std::runtime_error("No Node exists in bookkeeper with the requested id.");
	}

private:
	std::map<unsigned long, const sim_mob::Node*> nodeLookup;
};


}}} //End sim_mob::xml::helper namespace

