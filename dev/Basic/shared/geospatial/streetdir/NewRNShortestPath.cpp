//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "NewRNShortestPath.hpp"

using namespace sim_mob;

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
	StreetDirectory::Vertex source = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
	StreetDirectory::Vertex sink = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));

	nodeLookup[node] = std::make_pair(source, sink);

	simmobility_network::Point ps = *(node->getLocation());
	Point2D p(ps.getX(),ps.getY());
	boost::put(boost::vertex_name, graph, source, p);
	boost::put(boost::vertex_name, graph, sink, p);
}
StreetDirectory::VertexDesc NewRNShortestPath::DrivingVertex(const simmobility_network::Node& n)
{

}
std::vector<WayPoint> NewRNShortestPath::GetShortestDrivingPath(StreetDirectory::VertexDesc from,
															StreetDirectory::VertexDesc to,
															std::vector<const sim_mob::RoadSegment*> blacklist)
{

}
