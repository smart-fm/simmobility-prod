//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "NewRNShortestPath.hpp"

using namespace simmobility_network;

boost::shared_mutex NewRNShortestPath::GraphSearchMutex_;

NewRNShortestPath::NewRNShortestPath(const simmobility_network::RoadNetwork* network)
	:network(network)
{
	// add node to graph first
	const std::map<unsigned int, simmobility_network::Node*>& nodes = network->getMapOfIdvsNodes();
	std::map<unsigned int, simmobility_network::Node*>::const_iterator it_node = nodes.begin();
	for(;it_node!=nodes.end();++it_node){
		simmobility_network::Node* node = it_node->second;
		addNode(node);
	}


	const std::map<unsigned int, simmobility_network::Link*>& links = network->getMapOfIdVsLinks();
	std::map<unsigned int, simmobility_network::Link*>::const_iterator it = links.begin();
	for(;it!=links.end();++it)
	{
		simmobility_network::Link* link = it->second;
		addLink(link);
	}
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
	std::map<unsigned int, std::map<unsigned int, simmobility_network::TurningPath *> >::iterator it=tg->turningPaths.begin();

	for(;it!=tg->turningPaths.end();++it){
		std::map<unsigned int, simmobility_network::TurningPath *>& tps = it->second;
		std::map<unsigned int, simmobility_network::TurningPath *>::iterator itt = tps.begin();
		for(;itt!=tps.end();++itt){
			simmobility_network::TurningPath* tp = itt->second;
			addTurningPath(tp);
		}
	}
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
	simmobility_network::RoadSegment* rs = link->getRoadSegment(0);//roadSegments[0];
	simmobility_network::Lane* lane = rs->getLane(0);//lanes[0];
	simmobility_network::Point fromPoint = lane->polyLine->getFirstPoint();
	boost::put(boost::vertex_name, graph, fromVertex, fromPoint);

	// 2.2 add to vertex
	SMStreetDirectory::SMVertex toVertex = boost::add_vertex(const_cast<SMStreetDirectory::SMGraph &>(graph));
	int size = link->getRoadSegments().size();
	rs = link->getRoadSegment(size);//roadSegments[size-1];
	lane = rs->getLane(0);//lanes[0];
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
		throw std::runtime_error("addLink: from node not find in graph");
	}
	else{
		fromVD = it->second;
	}

	it = nodeLookup.find(toNode);
	simmobility_network::SMStreetDirectory::SMNodeVertexDesc toVD;
	if( it == nodeLookup.end() ){
		// add node to graph
		throw std::runtime_error("addLink: to node not find in graph");
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
std::vector<simmobility_network::WayPoint>  NewRNShortestPath::searchShortestPath(const SMStreetDirectory::SMVertex& fromVertex,
																const SMStreetDirectory::SMVertex& toVertex) const
{
	std::vector<simmobility_network::WayPoint> res;
	std::list<SMStreetDirectory::SMVertex> partialRes;

	//Lock for read access.
	{
	boost::shared_lock<boost::shared_mutex> lock(GraphSearchMutex_);

	//Use A* to search for a path
	//Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
	//...which is available under the terms of the Boost Software License, 1.0
	std::vector<SMStreetDirectory::SMVertex> p(boost::num_vertices(graph));  //Output variable
	std::vector<double> d(boost::num_vertices(graph));  //Output variable
	try {
		boost::astar_search(
			graph,
			fromVertex,
			distance_heuristic_graph(&graph, toVertex),
			boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(astar_goal_visitor(toVertex))
		);
	} catch (found_goal& goal) {
		//Build backwards.
		for (SMStreetDirectory::SMVertex v=toVertex;;v=p[v]) {
			partialRes.push_front(v);
			if(p[v] == v) {
				break;
			}
		}

		//Now build forwards.
		std::list<SMStreetDirectory::SMVertex>::const_iterator prev = partialRes.end();
		for (std::list<SMStreetDirectory::SMVertex>::const_iterator it=partialRes.begin(); it!=partialRes.end(); it++) {
			//Add this edge.
			if (prev!=partialRes.end()) {
				//This shouldn't fail.
				std::pair<SMStreetDirectory::SMEdge, bool> edge = boost::edge(*prev, *it, graph);
				if (!edge.second) {
					std::cout <<"ERROR: Boost can't find an edge that it should know about." <<std::endl;
					return std::vector<WayPoint>();
				}

				//Retrieve, add this edge's WayPoint.
				WayPoint wp = boost::get(boost::edge_name, graph, edge.first);
				res.push_back(wp);
			}

			//Save for later.
			prev = it;
		}
	}
	} //End boost mutex lock for read access.

	return res;
}
std::vector<simmobility_network::WayPoint> NewRNShortestPath::GetShortestDrivingPath(simmobility_network::Node* from,
														 simmobility_network::Node* to,
														 std::vector<const simmobility_network::Link*> blacklist)
{
	std::vector<simmobility_network::WayPoint> res;
	SMStreetDirectory::SMVertex fromVertex;
	SMStreetDirectory::SMVertex toVertex;
	// find from node's source vertex
	std::map<const simmobility_network::Node*, simmobility_network::SMStreetDirectory::SMNodeVertexDesc >::iterator it = nodeLookup.find(from);
	if( it==nodeLookup.end() ){
		std::cout<<"GetShortestDrivingPath: from node not in graph "<<from->getNodeId()<<std::endl;
		return res;
	}
	else{
		SMStreetDirectory::SMNodeVertexDesc vd = it->second;
		fromVertex = vd.source;
	}
	// find to node's sink vertex
	it = nodeLookup.find(to);
	if( it==nodeLookup.end() ){
		std::cout<<"GetShortestDrivingPath: to node not in graph "<<to->getNodeId()<<std::endl;
		return res;
	}
	else{
		SMStreetDirectory::SMNodeVertexDesc vd = it->second;
		toVertex = vd.sink;
	}
	//
	res = searchShortestPath(fromVertex,toVertex);

	return res;


}
