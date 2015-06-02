//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "NewRNShortestPath.hpp"

using namespace simmobility_network;

NewRNShortestPath::NewRNShortestPath(const simmobility_network::RoadNetwork* network)
	:network(network)
{
	// TODO Auto-generated constructor stub

}

NewRNShortestPath::~NewRNShortestPath() {
	// TODO Auto-generated destructor stub
}

void NewRNShortestPath::addNode(simmobility_network::Node* node)
{
	SMStreetDirectory::SMVertex source = boost::add_vertex(const_cast<SMStreetDirectory::SMGraph &>(graph));
	SMStreetDirectory::SMVertex sink = boost::add_vertex(const_cast<SMStreetDirectory::SMGraph &>(graph));

	// store
	nodeLookup[node] = std::make_pair(source, sink);

	// insert to graph
	simmobility_network::Point ps = *(node->getLocation());
	boost::put(boost::vertex_name, graph, source, ps);
	boost::put(boost::vertex_name, graph, sink, ps);
}
void NewRNShortestPath::addLink(simmobility_network::Link* link)
{

//	//Create an edge.
//	StreetDirectory::Edge edge;
//	bool ok;
//	boost::tie(edge, ok) = boost::add_edge(fromVertex, toVertex, graph);
//	boost::put(boost::edge_name, graph, edge, WayPoint(rs));
//
//	//set edgeWeight
//	boost::put(boost::edge_weight, graph, edge, edgeWeight);
//
//	//Save this in our lookup.
//	resSegLookup[rs].insert(edge);
}
SMStreetDirectory::SMVertexDesc NewRNShortestPath::DrivingVertex(const simmobility_network::Node& n)
{

}
std::vector<simmobility_network::WayPoint> NewRNShortestPath::GetShortestDrivingPath(simmobility_network::Node* from,
														 simmobility_network::Node* to,
														 std::vector<const simmobility_network::Link*> blacklist)
{

}
