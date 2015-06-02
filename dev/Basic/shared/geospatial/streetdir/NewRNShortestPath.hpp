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
#include "StreetDirectory.hpp"

#include "geospatial/simmobility_network/RoadNetwork.hpp"
#include "geospatial/simmobility_network/WayPoint.hpp"

namespace sim_mob {



class NewRNShortestPath : public StreetDirectory::ShortestPathImpl
{
public:
    explicit NewRNShortestPath(const simmobility_network::RoadNetwork* network);
    virtual ~NewRNShortestPath();

    virtual StreetDirectory::VertexDesc DrivingVertex(const simmobility_network::Node& n);

    // no walking path, no bus stop
	virtual StreetDirectory::VertexDesc WalkingVertex(const simmobility_network::Node& n){ return StreetDirectory::VertexDesc();}
	virtual StreetDirectory::VertexDesc DrivingVertex(const BusStop& b){return StreetDirectory::VertexDesc();}
	virtual StreetDirectory::VertexDesc WalkingVertex(const BusStop& b){ return StreetDirectory::VertexDesc();}

	virtual std::vector<WayPoint> GetShortestDrivingPath(StreetDirectory::VertexDesc from,
															StreetDirectory::VertexDesc to,
															std::vector<const sim_mob::RoadSegment*> blacklist);
	virtual std::vector<WayPoint> GetShortestWalkingPath(StreetDirectory::VertexDesc from,
														StreetDirectory::VertexDesc to)
																{return std::vector<WayPoint>();}

	virtual void updateEdgeProperty() {}

	virtual void printDrivingGraph(std::ostream& outFile){}
	virtual void printWalkingGraph(std::ostream& outFile){}

public:
	std::vector<simmobility_network::WayPoint> GetShortestDrivingPath(simmobility_network::Node* from,
																	  simmobility_network::Node* to,
																	  std::vector<const sim_mob::RoadSegment*>& blacklist);

	void buildGraph();
	void addNode(simmobility_network::Node* node);
	void addLink(simmobility_network::Link* link);

public:
    StreetDirectory::Graph graph;

    //Lookup the master Node for each Node-related vertex.
    //Note: The first item is the "source" vertex, used to search *from* that Node.
    //The second item is the "sink" vertex, used to search *to* that Node.
    std::map<const simmobility_network::Node*, std::pair<StreetDirectory::Vertex,StreetDirectory::Vertex> > nodeLookup;

private:
    const simmobility_network::RoadNetwork* network;

};

}// end namespace
