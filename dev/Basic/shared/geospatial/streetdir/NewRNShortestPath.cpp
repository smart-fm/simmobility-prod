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

simmobility_network::SMStreetDirectory::SMNodeVertexDesc  NewRNShortestPath::addNode(simmobility_network::Node* node)
{
	SMStreetDirectory::SMVertex source = boost::add_vertex(const_cast<SMStreetDirectory::SMGraph &>(graph));
	SMStreetDirectory::SMVertex sink = boost::add_vertex(const_cast<SMStreetDirectory::SMGraph &>(graph));

	// store
	simmobility_network::SMStreetDirectory::SMNodeVertexDesc vd(node);
	vd.source = source;
	vd.sink = sink;

	nodeLookup.insert(std::make_pair(node,vd));

//	nodeLookup[node] = std::make_pair(source, sink);

	// insert to graph
	simmobility_network::Point ps = *(node->getLocation());
	boost::put(boost::vertex_name, graph, source, ps);
	boost::put(boost::vertex_name, graph, sink, ps);

	return vd;
}
void NewRNShortestPath::addTurningGroup(simmobility_network::TurningGroup* tg)
{

}
void NewRNShortestPath::addTurningPath(simmobility_network::TurningPath* tp)
{
	// find parent link
	simmobility_network::Link* fromLink = tp->fromLane->parentSegment->parentLink;
	simmobility_network::Link* toLink = tp->toLane->parentSegment->parentLink;
	// find from link's start vertex
	SMStreetDirectory::SMVertex from;
	std::map<const simmobility_network::Link*,simmobility_network::SMStreetDirectory::SMLinkVertexDesc>::iterator it = linkLookup.find(fromLink);
	if( it== linkLookup.end() )
	{
		// never add this link to graph?
		throw std::runtime_error("addTurningPath: link not find in graph");
	}
	else{
		simmobility_network::SMStreetDirectory::SMLinkVertexDesc vd = it->second;
		from = vd.to; // turningpath 's from vertex is from link's end vertex
	}
	// find to link's start vertex
	SMStreetDirectory::SMVertex to;
	it = linkLookup.find(toLink);
	if( it== linkLookup.end() )
	{
		// never add this link to graph?
		throw std::runtime_error("addTurningPath: link not find in graph");
	}
	else{
		simmobility_network::SMStreetDirectory::SMLinkVertexDesc vd = it->second;
		to = vd.from; // turningpath 's to vertex is to link's start vertex
	}

	// create edge
	SMStreetDirectory::SMEdge turningPathEdge;
	bool ok;
	boost::tie(turningPathEdge, ok) = boost::add_edge(from, to, graph);
	boost::put(boost::edge_name, graph, turningPathEdge, WayPoint(tp));
	//set edgeWeight TODO
	double edgeWeight = tp->getLength();
	boost::put(boost::edge_weight, graph, turningPathEdge, edgeWeight);

	//store
	SMStreetDirectory::SMTurningPathVertexDesc vd(tp);
	vd.from = from;
	vd.to = to;
	turningPathLookup.insert(std::make_pair(tp,vd));

}
void NewRNShortestPath::addLink(simmobility_network::Link* link)
{


	// 2.0 create two vertices
	// 2.1 add from vertex, position is lane zero's first polyline point
	SMStreetDirectory::SMVertex fromVertex = boost::add_vertex(const_cast<SMStreetDirectory::SMGraph &>(graph));
	simmobility_network::RoadSegment* rs = link->roadSegments[0];
	simmobility_network::Lane* lane = rs->lanes[0];
	simmobility_network::Point fromPoint = lane->polyLine->getFirstPoint();
	boost::put(boost::vertex_name, graph, fromVertex, fromPoint);

	// 2.2 add to vertex
	SMStreetDirectory::SMVertex toVertex = boost::add_vertex(const_cast<SMStreetDirectory::SMGraph &>(graph));
	int size = link->roadSegments.size();
	rs = link->roadSegments[size-1];
	lane = rs->lanes[0];
	simmobility_network::Point toPoint = lane->polyLine->getLastPoint();
	boost::put(boost::vertex_name, graph, toVertex, toPoint);

	// 3.0 Create an edge link from ,to vertice
	SMStreetDirectory::SMEdge linkEdge;
	bool ok;
	boost::tie(linkEdge, ok) = boost::add_edge(fromVertex, toVertex, graph);
	boost::put(boost::edge_name, graph, linkEdge, WayPoint(link));

	//set edgeWeight TODO
	double edgeWeight = link->getLength();
	boost::put(boost::edge_weight, graph, linkEdge, edgeWeight);

	//Save this in our lookup.
	simmobility_network::SMStreetDirectory::SMLinkVertexDesc vd(link);
	vd.from = fromVertex;
	vd.to = toVertex;
	linkLookup.insert(std::make_pair(link,vd));

	// 1.0 find from,to nodes' vertice
	simmobility_network::Node* fromNode = link->getFromNode();
	simmobility_network::Node* toNode = link->getToNode();

	std::map<const simmobility_network::Node*, simmobility_network::SMStreetDirectory::SMNodeVertexDesc >::iterator it = nodeLookup.find(fromNode);
	simmobility_network::SMStreetDirectory::SMNodeVertexDesc fromVD;
	if( it == nodeLookup.end() ){
		// add node to graph
		fromVD = addNode(fromNode);
	}
	else{
		fromVD = it->second;
	}

	it = nodeLookup.find(toNode);
	simmobility_network::SMStreetDirectory::SMNodeVertexDesc toVD;
	if( it == nodeLookup.end() ){
		// add node to graph
		toVD = addNode(toNode);
	}
	else{
		toVD = it->second;
	}

	// link source vertex and from vertex
	SMStreetDirectory::SMEdge sourceEdge;
	boost::tie(sourceEdge, ok) = boost::add_edge(fromVD.source, fromVertex, graph);
	boost::put(boost::edge_name, graph, sourceEdge, WayPoint());// invalid waypoint
	boost::put(boost::edge_weight, graph, sourceEdge, 0);

	// link to vertex and sink vertex
	SMStreetDirectory::SMEdge toEdge;
	boost::tie(toEdge, ok) = boost::add_edge(toVertex,toVD.sink, graph);
	boost::put(boost::edge_name, graph, toEdge, WayPoint());// invalid waypoint
	boost::put(boost::edge_weight, graph, toEdge, 0);
}
SMStreetDirectory::SMNodeVertexDesc NewRNShortestPath::DrivingVertex(const simmobility_network::Node& n)
{

}
std::vector<simmobility_network::WayPoint> NewRNShortestPath::GetShortestDrivingPath(simmobility_network::Node* from,
														 simmobility_network::Node* to,
														 std::vector<const simmobility_network::Link*> blacklist)
{

}
