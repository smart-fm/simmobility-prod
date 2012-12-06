/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <iostream>
#include <vector>
#include <set>

/*namespace geo {
class Links_pimpl;
class RoadNetwork_t_pimpl;
class Nodes_pimpl;
class Intersections_pimpl;
class roundabouts_pimpl;
class UniNodes_pimpl;
class GeoSpatial_t_pimpl;
}*/

namespace sim_mob
{

//Forward declarations
class Node;
class UniNode;
class MultiNode;
class Point2D;
class Link;
class Conflux;

namespace aimsun
{
//Forward declaration
class Loader;
}


/**
 * The side of the road on which cars drive.
 *
 * \author Seth N. Hetu
 *
 * For the USA, this is DRIVES_ON_RIGHT;
 * for Singapore it is DRIVES_ON_LEFT. This affects the context of "can_turn_right_on_red",
 * and may affect lane merging rules.
 */
enum DRIVING_SIDE {
	DRIVES_ON_LEFT,
	DRIVES_ON_RIGHT
};


/**
 * The main Road Network. (Currently, this class only contains the "drivingSide" variable)
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 */

/*Added  by vahid*/

class RoadNetwork {
	friend class sim_mob::aimsun::Loader;
public:
	RoadNetwork() { drivingSide=DRIVES_ON_LEFT; } //TEMP

	DRIVING_SIDE drivingSide;

	///Retrieve list of all Uni/MultiNodes (intersections & roundabouts) in this Road Network.
	///
	///\todo This needs to eventually have some structure; see the wiki for an example.
	std::vector<sim_mob::MultiNode*>& getNodes() { return nodes; }
	const std::vector<sim_mob::MultiNode*>& getNodes() const { return nodes; }
	std::set<sim_mob::UniNode*>& getUniNodes() { return segmentnodes; }
	const std::set<sim_mob::UniNode*>& getUniNodes() const { return segmentnodes; }

	///Retrieve a list of all Links (high-level paths between MultiNodes) in this Road Network.
	std::vector<sim_mob::Link*>& getLinks() { return links; }
	const std::vector<sim_mob::Link*>& getLinks() const { return links; }

	///Find the closest Node.
	///If includeUniNodes is false, then only Intersections and Roundabouts are searched.
	///If no node is found within maxDistCM, the match fails and nullptr is returned.
	sim_mob::Node* locateNode(const sim_mob::Point2D& position, bool includeUniNodes=false, int maxDistCM=100) const;
	sim_mob::Node* locateNode(double xPos, double yPos, bool includeUniNodes=false, int maxDistCM=100) const;

	//Temporary; added for the XML loader
	void setLinks(const std::vector<sim_mob::Link*>& lnks) { this->links = lnks; }
	void setSegmentNodes(const std::set<sim_mob::UniNode*>& sn) { this->segmentnodes = sn; }
	void addNodes(const std::vector<sim_mob::MultiNode*>& vals) {
		nodes.insert(nodes.begin(),vals.begin(),vals.end());
	}

//private:
	//Temporary: Geometry will eventually make specifying nodes and links easier.
	std::vector<sim_mob::MultiNode*> nodes;
	std::vector<sim_mob::Link*> links;

//	Link_m links_m;

	//Temporary: Not exposed publicly
	std::set<sim_mob::UniNode*> segmentnodes;

	//todo remove public from here
public:
	//todo check whether the network is sealed -vahid
	//NOTE: We check if the network is sealed in the config file, not the RoadNetwork.
	//      I've changed the getNodes() etc. functions to be non-const. ~Seth
	/*std::vector<sim_mob::MultiNode*>& getNodesRW() { return nodes; }
	std::set<sim_mob::UniNode*>& getUniNodesRW() { return segmentnodes; }
	std::vector<sim_mob::Link*>& getLinksRW() { return links; }*/

};





}
