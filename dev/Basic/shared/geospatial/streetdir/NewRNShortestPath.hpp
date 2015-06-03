//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "geospatial/Point2D.hpp"
#include "util/GeomHelpers.hpp"
#include "util/LangHelpers.hpp"

#include <map>
#include <vector>
#include <string>
#include <ostream>

#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/astar_search.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>
#include "SMStreetDirectory.hpp"

#include "geospatial/simmobility_network/RoadNetwork.hpp"
#include "geospatial/simmobility_network/WayPoint.hpp"
#include "geospatial/streetdir/SMStreetDirectory.hpp"
#include "geospatial/simmobility_network/TurningPath.hpp"

namespace simmobility_network {



class NewRNShortestPath : public SMStreetDirectory::SMShortestPathImpl
{
public:
    explicit NewRNShortestPath(const simmobility_network::RoadNetwork* network);
    virtual ~NewRNShortestPath();

    virtual SMStreetDirectory::SMNodeVertexDesc DrivingVertex(const simmobility_network::Node& node);


	virtual std::vector<WayPoint> GetShortestDrivingPath(simmobility_network::Node* from,
														 simmobility_network::Node* to,
														 std::vector<const simmobility_network::Link*> blacklist);

public:
	void buildGraph();
	simmobility_network::SMStreetDirectory::SMNodeVertexDesc  addNode(simmobility_network::Node* node);
	void addLink(simmobility_network::Link* link);
	void addTurningGroup(simmobility_network::TurningGroup* tg);
	void addTurningPath(simmobility_network::TurningPath* tp);

public:
    SMStreetDirectory::SMGraph graph;

    //Lookup the master Node for each Node-related vertex.
    //Note: The first item is the "source" vertex, used to search *from* that Node.
    //The second item is the "sink" vertex, used to search *to* that Node.
    std::map<const simmobility_network::Node*, simmobility_network::SMStreetDirectory::SMNodeVertexDesc > nodeLookup;
    std::map<const simmobility_network::Link*,simmobility_network::SMStreetDirectory::SMLinkVertexDesc> linkLookup;
    std::map<const simmobility_network::TurningPath*,simmobility_network::SMStreetDirectory::SMTurningPathVertexDesc> turningPathLookup;
private:
    const simmobility_network::RoadNetwork* network;

};

}// end namespace
