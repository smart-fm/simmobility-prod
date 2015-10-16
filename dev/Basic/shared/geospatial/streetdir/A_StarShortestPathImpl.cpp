//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include <cmath>

//for caching
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "geospatial/network/BusStop.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/PolyLine.hpp"
#include "geospatial/network/TurningGroup.hpp"
#include "A_StarShortestPathImpl.hpp"
#include "logging/Log.hpp"

using std::map;
using std::set;
using std::vector;
using std::string;
using namespace sim_mob;

boost::shared_mutex A_StarShortestPathImpl::GraphSearchMutex;

A_StarShortestPathImpl::A_StarShortestPathImpl(const RoadNetwork& network):isValidSegGraph(false)
{
	initLinkDrivingNetwork(network);
}

StreetDirectory::Edge A_StarShortestPathImpl::addSimpleEdge(StreetDirectory::Graph& graph, StreetDirectory::Vertex& fromV, StreetDirectory::Vertex& toV,
															WayPoint wp, double length)
{
	StreetDirectory::Edge edge;
	bool ok;

	if (length < 0.0)
	{
		length = A_StarShortestPathImpl::euclideanDist(boost::get(boost::vertex_name, graph, fromV), boost::get(boost::vertex_name, graph, toV));
	}

	boost::tie(edge, ok) = boost::add_edge(fromV, toV, graph);
	boost::put(boost::edge_name, graph, edge, wp);
	boost::put(boost::edge_weight, graph, edge, length);

	return edge;
}

StreetDirectory::Vertex A_StarShortestPathImpl::addSimpleVertex(StreetDirectory::Graph& graph, const Point& pos)
{
	StreetDirectory::Vertex ver = boost::add_vertex(graph);
	boost::put(boost::vertex_name, graph, ver, pos);
	return ver;
}

StreetDirectory::Vertex A_StarShortestPathImpl::findStartingVertex(const RoadSegment *rs, const SegmentVertexLookup& segVerLookup)
{
	SegmentVertexLookup::const_iterator from = segVerLookup.find(rs);

	if (from == segVerLookup.end())
	{
		throw std::runtime_error("The starting road segment was not found in the vertex map");
	}

	//For simply nodes, this will be sufficient.
	StreetDirectory::Vertex fromVertex = (from->second).first;

	return fromVertex;
}

StreetDirectory::Vertex A_StarShortestPathImpl::findEndingVertex(const RoadSegment* rs, const SegmentVertexLookup& segVerLookup)
{
	SegmentVertexLookup::const_iterator to = segVerLookup.find(rs);

	if (to == segVerLookup.end())
	{
		throw std::runtime_error("The end road segment was not found in the vertex map");
	}

	//For simply nodes, this will be sufficient.
	StreetDirectory::Vertex toVertex = (to->second).second;

	return toVertex;
}

void A_StarShortestPathImpl::initLinkDrivingNetwork(const RoadNetwork& roadNetwork)
{
	const std::map<unsigned int, Link *>& links = roadNetwork.getMapOfIdVsLinks();
	NodeLookup nodeLookup;

	//Add our initial set of vertices. Iterate through Links to ensure no un-used Node are added.
	std::map<unsigned int, Link *>::const_iterator iter;
	for (iter = links.begin(); iter != links.end(); ++iter)
	{
		procAddDrivingNodes(drivingLinkMap, iter->second, nodeLookup);
	}

	//Proceed through our Links, adding each link path.
	for (iter = links.begin(); iter != links.end(); ++iter)
	{
		procAddDrivingLinks(drivingLinkMap, iter->second, nodeLookup, drivingLinkEdgeLookup);
	}

	//Now add all Intersection edges (turning connectors)
	for (NodeLookup::const_iterator it = nodeLookup.begin(); it != nodeLookup.end(); it++)
	{
		procAddDrivingLinkConnectors(drivingLinkMap, (it->first), nodeLookup);
	}

	//Finally, add our "master" node vertices
	procAddStartNodesAndEdges(drivingLinkMap, nodeLookup, &drivingNodeLookup);
}

void A_StarShortestPathImpl::initSegDrivingNetwork(const RoadNetwork& roadNetwork)
{
	const std::map<unsigned int, Link *>& links = roadNetwork.getMapOfIdVsLinks();
	NodeLookup nodeLookup;

	//Add our initial set of vertices. Iterate through Links to ensure no un-used Node are added.
	std::map<unsigned int, Link *>::const_iterator iter;
	for (iter = links.begin(); iter != links.end(); ++iter)
	{
		procAddDrivingNodes(drivingSegMap, iter->second, nodeLookup);
	}

	//Proceed through our Links, adding each RoadSegment path. Split vertices as required.
	for (iter = links.begin(); iter != links.end(); ++iter)
	{
		procAddDrivingSegments(drivingSegMap, iter->second, nodeLookup, drivingSegmentEdgeLookup, drivingSegVertexLookup);
		procAddDrivingBusStops(drivingSegMap, iter->second, drivingSegVertexLookup, drivingSegmentEdgeLookup, drivingBusStopLookup);
	}

	//Now add all Intersection edges (turning connectors)
	for (NodeLookup::const_iterator it = nodeLookup.begin(); it != nodeLookup.end(); it++)
	{
		procAddDrivingLinkConnectors(drivingSegMap, (it->first), nodeLookup);
	}

	//Finally, add our "master" node vertices
	procAddStartNodesAndEdges(drivingSegMap, nodeLookup, &drivingNodeLookup);
	isValidSegGraph = true;
}

void A_StarShortestPathImpl::procAddDrivingNodes(StreetDirectory::Graph& graph, const Link* link, NodeLookup& nodeLookup)
{
	//Skip empty link
	if (!link)
	{
		return;
	}

	if (link->getRoadSegments().size() == 0)
	{
		throw std::runtime_error("The link does not contain any segments");
	}

	NodeDescriptor ndSource;
	ndSource.after = link;
	ndSource.before = nullptr;
	const Node* sNode = link->getFromNode();
	if (nodeLookup.count(sNode) == 0)
	{
		nodeLookup[sNode] = VertexLookup();
		nodeLookup[sNode].node = sNode;
	}

	//Construction of vertex at the node
	RoadSegment* seg = link->getRoadSegments().front();
	if (!seg || !seg->getPolyLine())
	{
		throw std::runtime_error("The segment does not have an associated poly-line");
	}
	PolyPoint sPos = seg->getPolyLine()->getFirstPoint();
	ndSource.v = addSimpleVertex(graph, sPos);
	nodeLookup[sNode].vertices.push_back(ndSource);

	NodeDescriptor ndSink;
	ndSink.after = nullptr;
	ndSink.before = link;
	const Node* dNode = link->getToNode();
	if (nodeLookup.count(dNode) == 0)
	{
		nodeLookup[dNode] = VertexLookup();
		nodeLookup[dNode].node = dNode;
	}

	//Construction of vertex at the node
	seg = link->getRoadSegments().back();
	if (!seg || !seg->getPolyLine())
	{
		throw std::runtime_error("The segment does not have an associated poly-line");
	}
	PolyPoint dPos = seg->getPolyLine()->getLastPoint();
	ndSink.v = addSimpleVertex(graph, dPos);
	nodeLookup[dNode].vertices.push_back(ndSink);
}

void A_StarShortestPathImpl::procAddDrivingBusStops(StreetDirectory::Graph& graph, const Link* link, const SegmentVertexLookup& segVerLookup,
													SegmentEdgeLookup& resSegEdgeLookup, StopVertexLookup& resStopVerLookup)
{
	//Skip empty link
	if (!link)
	{
		return;
	}

	//Iterate through all obstacles on all RoadSegments and find Bus Stop obstacles.
	const vector<RoadSegment*>& roadway = link->getRoadSegments();
	for (vector<RoadSegment*>::const_iterator rsIt = roadway.begin(); rsIt != roadway.end(); rsIt++)
	{
		const RoadSegment* rs = *rsIt;
		const std::map<double, RoadItem *> obstacles = rs->getObstacles();
		for (map<double, RoadItem *>::const_iterator it = obstacles.begin(); it != obstacles.end(); it++)
		{
			const BusStop* stop = dynamic_cast<const BusStop*> (it->second);
			if (!stop)
			{
				continue;
			}

			//Retrieve the original "start" and "end" Nodes for this segment.
			StreetDirectory::Vertex fromVertex = findStartingVertex(rs, segVerLookup);
			StreetDirectory::Vertex toVertex = findEndingVertex(rs, segVerLookup);

			//At this point, we have the Bus Stop, as well as the Road Segment that it appears on.
			//We need to add a Vertex for the stop itself, and connect an incoming and outgoing Edge to it.
			Point stopPoint = stop->getStopLocation();
			StreetDirectory::Vertex vStop = addSimpleVertex(graph, stopPoint);

			//Add the Bus vertex to a lookup
			if (resStopVerLookup.count(stop) > 0)
			{
				throw std::runtime_error("Duplicate Bus Stop in lookup.");
			}
			resStopVerLookup[stop] = std::make_pair(vStop, vStop);

			//Add the new route.
			StreetDirectory::Edge e1 = addSimpleEdge(graph, fromVertex, vStop, WayPoint(rs));
			StreetDirectory::Edge e2 = addSimpleEdge(graph, vStop, toVertex, WayPoint(rs));

			//Save them in our lookup.
			resSegEdgeLookup[rs].insert(e1);
			resSegEdgeLookup[rs].insert(e2);
		}
	}
}

void A_StarShortestPathImpl::procAddDrivingSegments(StreetDirectory::Graph& graph, const Link* link, const NodeLookup& nodeLookup,
													SegmentEdgeLookup& resSegEdgeLookup, SegmentVertexLookup& resSegVerLookup)
{
	//Skip empty link
	if (!link)
	{
		return;
	}

	//Here, we are simply assigning one Edge per RoadSegment in the Link.
	const vector<RoadSegment*>& roadway = link->getRoadSegments();
	for (vector<RoadSegment*>::const_iterator it = roadway.begin(); it != roadway.end(); it++)
	{
		const RoadSegment* rs = *it;
		if (!rs || !rs->getPolyLine())
		{
			throw std::runtime_error("The segment does not have an associated poly-line");
		}
		StreetDirectory::Vertex fromVertex;
		StreetDirectory::Vertex toVertex;
		if (it == roadway.begin())
		{
			NodeLookup::const_iterator from = nodeLookup.find(link->getFromNode());
			if (from == nodeLookup.end())
			{
				throw std::runtime_error("The starting node of the link was not found");
			}
			if (from->second.vertices.empty())
			{
				Warn() << "Warning: The from node of link the has no known mapped vertices (1)."
						<< std::endl;
				continue;
			}
			fromVertex = from->second.vertices.front().v;
			if (from->second.vertices.size() > 1)
			{
				bool error = true;
				for (std::vector<NodeDescriptor>::const_iterator it = from->second.vertices.begin(); it != from->second.vertices.end(); it++)
				{
					if (link == it->after)
					{
						fromVertex = it->v;
						error = false;
					}
				}
				if (error)
				{
					throw std::runtime_error("Unable to find Node with proper outgoing link in the \"from\" vertex map.");
				}
			}
			PolyPoint sPos = rs->getPolyLine()->getFirstPoint();
			PolyPoint dPos = rs->getPolyLine()->getLastPoint();
			toVertex = addSimpleVertex(graph, dPos);
		}
		else if (it == --roadway.end())
		{
			NodeLookup::const_iterator to = nodeLookup.find(link->getToNode());
			if (to == nodeLookup.end())
			{
				throw std::runtime_error("The end node was not found");
			}
			if (to->second.vertices.empty())
			{
				Warn() << "Warning: The to nodes of the link has no known mapped vertices (2)."
						<< std::endl;
				continue;
			}
			toVertex = to->second.vertices.front().v;
			if (to->second.vertices.size() > 1)
			{
				bool error = true;
				for (std::vector<NodeDescriptor>::const_iterator it = to->second.vertices.begin(); it != to->second.vertices.end(); it++)
				{
					if (rs->getParentLink() == it->before)
					{
						toVertex = it->v;
						error = false;
					}
				}
				if (error)
				{
					throw std::runtime_error("Unable to find Node with proper outgoing Link in \"to\" vertex map.");
				}
			}
			PolyPoint sPos = rs->getPolyLine()->getFirstPoint();
			PolyPoint dPos = rs->getPolyLine()->getLastPoint();
			fromVertex = addSimpleVertex(graph, sPos);
		}
		else
		{
			PolyPoint sPos = rs->getPolyLine()->getFirstPoint();
			PolyPoint dPos = rs->getPolyLine()->getLastPoint();
			fromVertex = addSimpleVertex(graph, sPos);
			toVertex = addSimpleVertex(graph, dPos);
		}

		//Create an edge.
		StreetDirectory::Edge edge = addSimpleEdge(graph, fromVertex, toVertex, WayPoint(rs), rs->getLength());
		//Save this in our lookup.
		resSegEdgeLookup[rs].insert(edge);
		resSegVerLookup[rs] = std::make_pair(fromVertex, toVertex);
	}
}

void A_StarShortestPathImpl::procAddDrivingLinks(StreetDirectory::Graph& graph, const Link* link, const NodeLookup& nodeLookup, LinkEdgeLookup& resLinkEdgeLookup)
{
	//Skip empty link
	if (!link)
	{
		return;
	}

	//Here, we are simply assigning one Edge per Link. This is mildly complicated by the fact that a Node*
	//  may be represented by multiple vertices.
	NodeLookup::const_iterator from = nodeLookup.find(link->getFromNode());
	NodeLookup::const_iterator to = nodeLookup.find(link->getToNode());
	if (from == nodeLookup.end() || to == nodeLookup.end())
	{
		throw std::runtime_error("link's nodes are unknown by the vertex map.");
	}
	if (from->second.vertices.empty() || to->second.vertices.empty())
	{
		Warn() << "Warning: link's nodes have no known mapped vertices (1)."
				<< std::endl;
		return;
	}

	//For simple nodes, this will be sufficient.
	StreetDirectory::Vertex fromVertex = from->second.vertices.front().v;
	StreetDirectory::Vertex toVertex = to->second.vertices.front().v;

	//If there are multiple options, search for the right one.
	//To accomplish this, just match our "before/after" tagged data. Note that before/after may be null.
	if (from->second.vertices.size() > 1)
	{
		bool error = true;
		for (std::vector<NodeDescriptor>::const_iterator it = from->second.vertices.begin(); it != from->second.vertices.end(); it++)
		{
			if (link == it->after)
			{
				fromVertex = it->v;
				error = false;
			}
		}
		if (error)
		{
			throw std::runtime_error("Unable to find Node with proper outgoing Link in \"from\" vertex map.");
		}
	}

	if (to->second.vertices.size() > 1)
	{
		bool error = true;
		for (std::vector<NodeDescriptor>::const_iterator it = to->second.vertices.begin(); it != to->second.vertices.end(); it++)
		{
			if (link == it->before)
			{
				toVertex = it->v;
				error = false;
			}
		}
		if (error)
		{
			throw std::runtime_error("Unable to find Node with proper outgoing Link in \"to\" vertex map.");
		}
	}

	//Create an edge.
	StreetDirectory::Edge edge = addSimpleEdge(graph, fromVertex, toVertex, WayPoint(link), link->getLength());
	//Save this in our lookup.
	resLinkEdgeLookup[link].insert(edge);
}

void A_StarShortestPathImpl::procAddDrivingLinkConnectors(StreetDirectory::Graph& graph, const Node* node, const NodeLookup& nodeLookup)
{
	//Skip nulled Nodes.
	if (!node)
	{
		return;
	}

	//We actually only care about Link to Link connections.
	set<std::pair<Link*, Link*> > connectors;
	RoadNetwork* roadNetwork = RoadNetwork::getInstance();
	const std::map<unsigned int, Link *>& mapOfIdToLinks = roadNetwork->getMapOfIdVsLinks();
	const std::map<unsigned int, std::map<unsigned int, TurningGroup *> >& turningGroups = node->getTurningGroups();
	for (std::map<unsigned int, std::map<unsigned int, TurningGroup *> >::const_iterator conIt = turningGroups.begin(); conIt != turningGroups.end(); conIt++)
	{
		for (std::map<unsigned int, TurningGroup *>::const_iterator it = conIt->second.begin(); it != conIt->second.end(); it++)
		{
			std::map<unsigned int, Link *>::const_iterator retIt;
			Link* from = nullptr;
			Link* to = nullptr;
			retIt = mapOfIdToLinks.find(conIt->first);
			if (retIt != mapOfIdToLinks.end())
			{
				from = retIt->second;
			}
			retIt = mapOfIdToLinks.find(it->first);
			if (retIt != mapOfIdToLinks.end())
			{
				to = retIt->second;
			}
			if (!from || !to)
			{
				Warn() << "ERROR: Boost can't original link or destination link."
						<< std::endl;
				continue;
			}
			connectors.insert(std::make_pair(from, to));
		}
	}

	//Now, add each "Link" connector.
	for (set<std::pair<Link*, Link*> >::iterator it = connectors.begin(); it != connectors.end(); it++)
	{
		//Sanity check:
		if (it->first->getToNode() != node || it->second->getFromNode() != node)
		{
			throw std::runtime_error("Node/Link mismatch in Edge constructor.");
		}

		//Various bookkeeping requirements:
		std::pair < StreetDirectory::Vertex, bool> fromVertex;
		fromVertex.second = false;
		std::pair < StreetDirectory::Vertex, bool> toVertex;
		toVertex.second = false;
		std::map<const Node*, VertexLookup>::const_iterator vertCandidates = nodeLookup.find(node);
		if (vertCandidates == nodeLookup.end())
		{
			throw std::runtime_error("Intersection's Node is unknown by the vertex map.");
		}

		//Find the "from" and "to" links' associated end vertices. Keep track of each.
		for (vector<NodeDescriptor>::const_iterator ndIt = vertCandidates->second.vertices.begin(); ndIt != vertCandidates->second.vertices.end(); ndIt++)
		{
			if (it->first == ndIt->before)
			{
				fromVertex.first = ndIt->v;
				fromVertex.second = true;
			}
			if (it->second == ndIt->after)
			{
				toVertex.first = ndIt->v;
				toVertex.second = true;
			}
		}

		//Ensure we have both
		if (!fromVertex.second || !toVertex.second)
		{
			throw std::runtime_error("Link connector has no associated vertex.");
		}

		//Create an edge.
		addSimpleEdge(graph, fromVertex.first, toVertex.first, WayPoint(node), 0.0);
	}
}

void A_StarShortestPathImpl::procAddStartNodesAndEdges(StreetDirectory::Graph& graph, const NodeLookup& allNodes, NodeVertexLookup* resLookup)
{
	for (std::map<const Node*, VertexLookup>::const_iterator it = allNodes.begin(); it != allNodes.end(); it++)
	{
		StreetDirectory::Vertex source = addSimpleVertex(graph, it->first->getLocation());
		StreetDirectory::Vertex sink = addSimpleVertex(graph, it->first->getLocation());
		if (resLookup) {
			(*resLookup)[it->first] = std::make_pair(source, sink);
		}

		//Link to each child vertex. Assume a trivial distance.
		for (std::vector<NodeDescriptor>::const_iterator it2 =it->second.vertices.begin(); it2 != it->second.vertices.end();it2++) {
			//From source to "other"
			StreetDirectory::Vertex v =it2->v;
			if(!it2->after)
			{
				addSimpleEdge(graph, source, v, WayPoint(it->first), 1.0);
			}

			//From "other" to sink
			if(!it2->before)
			{
				addSimpleEdge(graph,v, sink, WayPoint(it->first), 1.0);
			}

		}
	}
}

StreetDirectory::VertexDesc A_StarShortestPathImpl::DrivingVertex(const Node& n) const
{
	StreetDirectory::VertexDesc res;

	//Convert the node to a vertex in the map.
	//It is possible that node are not represented by any vertex in the graph.
	NodeVertexLookup::const_iterator vertexIt = drivingNodeLookup.find(&n);
	if (vertexIt != drivingNodeLookup.end())
	{
		res.valid = true;
		res.source = vertexIt->second.first;
		res.sink = vertexIt->second.second;
		return res;
	}

	return res;
}

StreetDirectory::VertexDesc A_StarShortestPathImpl::DrivingVertex(const BusStop& b) const
{
	StreetDirectory::VertexDesc res;

	//Convert the node to a vertex in the map.
	//It is possible that bus stop are not represented by any vertex in the graph.
	StopVertexLookup::const_iterator vertexIt = drivingBusStopLookup.find(&b);
	if (vertexIt != drivingBusStopLookup.end())
	{
		res.valid = true;
		res.source = vertexIt->second.first;
		res.sink = vertexIt->second.second;
		return res;
	}

	return res;
}

std::vector<WayPoint> A_StarShortestPathImpl::GetShortestDrivingPath(const StreetDirectory::VertexDesc &from, const StreetDirectory::VertexDesc &to,
																	 const std::vector<const Link*> &blacklist) const
{
	//check whether invalid or not.
	if (!(from.valid && to.valid))
	{
		return vector<WayPoint>();
	}

	StreetDirectory::Vertex fromV = from.source;
	StreetDirectory::Vertex toV = to.sink;

	//Convert the blacklist into a list of blocked Vertices.
	set<StreetDirectory::Edge> blacklistV;
	for (vector<const Link*>::const_iterator it = blacklist.begin(); it != blacklist.end(); it++)
	{
		LinkEdgeLookup::const_iterator lookIt = drivingLinkEdgeLookup.find(*it);
		if (lookIt != drivingLinkEdgeLookup.end())
		{
			blacklistV.insert(lookIt->second.begin(), lookIt->second.end());
		}
	}

	if (blacklistV.empty())
	{
		return searchShortestPath(drivingLinkMap, fromV, toV);
	}
	else
	{
		return searchShortestPathWithBlackList(drivingLinkMap, fromV, toV, blacklistV);
	}
}

vector<WayPoint> A_StarShortestPathImpl::GetShortestDrivingPath(const StreetDirectory::VertexDesc &from, const StreetDirectory::VertexDesc &to,
																const std::vector<const RoadSegment *> &blacklist) const
{
	//check whether invalid or not.
	if (!(from.valid && to.valid))
	{
		return vector<WayPoint>();
	}

	if(!isValidSegGraph)
	{
		return vector<WayPoint>();
	}

	StreetDirectory::Vertex fromV = from.source;
	StreetDirectory::Vertex toV = to.sink;

	//Convert the blacklist into a list of blocked Vertices.
	set<StreetDirectory::Edge> blacklistV;
	for (vector<const RoadSegment*>::const_iterator it = blacklist.begin(); it != blacklist.end(); it++)
	{
		SegmentEdgeLookup::const_iterator lookIt = drivingSegmentEdgeLookup.find(*it);
		if (lookIt != drivingSegmentEdgeLookup.end())
		{
			blacklistV.insert(lookIt->second.begin(), lookIt->second.end());
		}
	}

	if (blacklistV.empty())
	{
		return searchShortestPath(drivingSegMap, fromV, toV);
	}
	else
	{
		return searchShortestPathWithBlackList(drivingSegMap, fromV, toV, blacklistV);
	}
}

std::vector<WayPoint> A_StarShortestPathImpl::searchShortestPathWithBlackList(const StreetDirectory::Graph& graph, const StreetDirectory::Vertex& fromVertex,
																			  const StreetDirectory::Vertex& toVertex,
																			  const std::set<StreetDirectory::Edge>& blacklist)
{
	//Lock for read access.
	boost::shared_lock<boost::shared_mutex> lock(GraphSearchMutex);

	//Filter it.
	BlackListEdgeConstraint filter(blacklist);
	boost::filtered_graph<StreetDirectory::Graph, BlackListEdgeConstraint> filtered(graph, filter);

	vector<WayPoint> res;
	std::list<StreetDirectory::Vertex> partialRes;
	//Output variable for A* searching
	vector<StreetDirectory::Vertex> p(boost::num_vertices(filtered));
	//Output variable for A* searching
	vector<double> d(boost::num_vertices(filtered));

	//Use A* to search for a path
	try
	{
		boost::astar_search(filtered, fromVertex, DistanceHeuristicFiltered(&filtered, toVertex),
							boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(GoalVisitor(toVertex)));
	}
	catch (Goal& goal)
	{
		//Build backwards.
		for (StreetDirectory::Vertex v = toVertex;; v = p[v])
		{
			partialRes.push_front(v);
			if (p[v] == v)
			{
				break;
			}
		}

		//Now build forwards.
		std::list<StreetDirectory::Vertex>::const_iterator prev = partialRes.end();
		for (std::list<StreetDirectory::Vertex>::const_iterator it = partialRes.begin(); it != partialRes.end(); it++)
		{
			//Add this edge.
			if (prev != partialRes.end())
			{
				//This shouldn't fail.
				std::pair < StreetDirectory::Edge, bool> edge = boost::edge(*prev, *it, filtered);
				if (!edge.second)
				{
					Warn() << "ERROR: Boost can't find an edge that it should know about."
							<< std::endl;
					return std::vector<WayPoint>();
				}

				//Retrieve, add this edge's WayPoint.
				WayPoint wp = boost::get(boost::edge_name, filtered, edge.first);
				res.push_back(wp);
			}

			//Save for later.
			prev = it;
		}
	}

	return res;
}

vector<WayPoint> A_StarShortestPathImpl::searchShortestPath(const StreetDirectory::Graph& graph, const StreetDirectory::Vertex& fromVertex,
															const StreetDirectory::Vertex& toVertex)
{
	vector<WayPoint> res;
	std::list<StreetDirectory::Vertex> partialRes;

	//Lock for read access.
	boost::shared_lock<boost::shared_mutex> lock(GraphSearchMutex);
	//Output variable for A* searching
	vector<StreetDirectory::Vertex> p(boost::num_vertices(graph));
	//Output variable for A* searching
	vector<double> d(boost::num_vertices(graph));

	//Use A* to search for a path
	try
	{
		boost::astar_search(graph, fromVertex, DistanceHeuristicGraph(&graph, toVertex),
							boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(GoalVisitor(toVertex)));
	}
	catch (Goal& goal)
	{
		//Build backwards.
		for (StreetDirectory::Vertex v = toVertex;; v = p[v])
		{
			partialRes.push_front(v);
			if (p[v] == v)
			{
				break;
			}
		}

		//Now build forwards.
		std::list<StreetDirectory::Vertex>::const_iterator prev = partialRes.end();
		for (std::list<StreetDirectory::Vertex>::const_iterator it = partialRes.begin(); it != partialRes.end(); it++)
		{
			//Add this edge.
			if (prev != partialRes.end())
			{
				//This shouldn't fail.
				std::pair < StreetDirectory::Edge, bool> edge = boost::edge(*prev, *it, graph);
				if (!edge.second)
				{
					Warn() << "ERROR: Boost can't find an edge that it should know about."
							<< std::endl;
					return std::vector<WayPoint>();
				}

				//Retrieve, add this edge WayPoint.
				WayPoint wp = boost::get(boost::edge_name, graph, edge.first);
				/*if(wp.type==WayPoint::LINK){
					const vector<RoadSegment*>& segs = wp.link->getRoadSegments();
					for(vector<RoadSegment*>::const_iterator it=segs.begin(); it!=segs.end(); it++){
						Print()<<(*it)->getRoadSegmentId()<<std::endl;
					}
				}*/
				res.push_back(wp);
			}

			//Save for later.
			prev = it;
		}
	}

	return res;
}

double A_StarShortestPathImpl::euclideanDist(const Point& pt1, const Point& pt2)
{
	double dx = pt2.getX() - pt1.getX();
	double dy = pt2.getY() - pt1.getY();
	double dz = pt2.getZ() - pt1.getZ();
	return sqrt(dx * dx + dy * dy + dz * dz);
}

void A_StarShortestPathImpl::printDrivingGraph(std::ostream& outFile) const
{
	printGraph(outFile, "driving", drivingSegMap);
}

void A_StarShortestPathImpl::printGraph(std::ostream& outFile, const std::string& graphType, const StreetDirectory::Graph& graph) const
{
	//Print an identifier
	outFile << "(\"sd-graph\"" << "," << 0 << "," << &graph << ",{"
			<< "\"type\":\"" << graphType << "\"})" << std::endl;

	//Print each vertex
	{
		StreetDirectory::Graph::vertex_iterator iter, end;
		for (boost::tie(iter, end) = boost::vertices(graph); iter != end;
				++iter)
		{
			StreetDirectory::Vertex v = *iter;
			const Point pt = boost::get(boost::vertex_name, graph, v);
			outFile << "(\"sd-vertex\"" << "," << 0 << "," << v << ",{"
					<< "\"parent\":\"" << &graph << "\",\"xPos\":\""
					<< pt.getX() << "\",\"yPos\":\"" << pt.getY() << "\"})"
					<< std::endl;
		}
	}

	//Print each edge
	{
		StreetDirectory::Graph::edge_iterator iter, end;
		unsigned int id = 0;
		for (boost::tie(iter, end) = boost::edges(graph); iter != end; ++iter)
		{
			StreetDirectory::Edge ed = *iter;
			StreetDirectory::Vertex srcV = boost::source(ed, graph);
			StreetDirectory::Vertex destV = boost::target(ed, graph);
			outFile << "(\"sd-edge\"" << "," << 0 << "," << id++ << ",{"
					<< "\"parent\":\"" << &graph << "\",\"fromVertex\":\""
					<< srcV << "\",\"toVertex\":\"" << destV << "\"})"
					<< std::endl;
		}
	}
}
