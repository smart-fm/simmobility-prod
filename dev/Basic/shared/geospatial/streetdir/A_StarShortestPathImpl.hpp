//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once


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

namespace sim_mob
{

class Node;
class Link;
class RoadSegment;
class RoadNetwork;

class A_StarShortestPathImpl : public StreetDirectory::ShortestPathImpl
{
protected:

	/**
	 * Helper class: Identifies a Node exactly. This requires the incoming/outgoing RoadSegment(s), and the generated Vertex.
	 */
	struct NodeDescriptor
	{
		const Link* before;
		const Link* after;
		StreetDirectory::Vertex v;

		NodeDescriptor() : before(nullptr), after(nullptr), v(false)
		{
		}

		bool operator==(const NodeDescriptor& other) const
		{
			return (this->before == other.before) && (this->after == other.after);
		}
	};

	/**
	 * Helper class: Represents a vertex in our graph as a "Node" in SimMobility. Maintains mappings to the original node
	 */
	struct VertexLookup
	{
		const Node* node;
		std::vector<NodeDescriptor> vertices;

		VertexLookup() : node(nullptr)
		{
		}
	};

	typedef std::map<const Node*, VertexLookup> NodeLookup;
	typedef std::map<const Node*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> > NodeVertexLookup;
	typedef std::map<const RoadSegment*, std::set<StreetDirectory::Edge> > SegmentEdgeLookup;
	typedef std::map<const Link*, std::set<StreetDirectory::Edge> > LinkEdgeLookup;
	typedef std::map<const Link*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> > LinkVertexLookup;
	typedef std::map<const RoadSegment*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> > SegmentVertexLookup;
	typedef std::map<const BusStop*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> > StopVertexLookup;

private:
	/**
	 * Initialize  segments-based graph
	 *
	 * @param roadNetwork is the reference to network object
	 */
	void initSegDrivingNetwork(const RoadNetwork& roadNetwork);

	/**
	 * Initializes the links-based graph
	 *
	 * @param roadNetwork is the reference to network object
	 */
	void initLinkDrivingNetwork(const RoadNetwork& roadNetwork);
	/**
	 * Processes driving path for each segment
	 *
	 * @param graph is the graph object
	 * @param link is a pointer to one link
	 * @param nodeLookup is a lookup table to record node with associated vertex
	 * @param resSegEdgeLookup is a lookup table about the map from segments to edges
	 * @param resSegVerLookup is a lookup table about the map from segments to vertex
	 */
	void procAddDrivingSegments(StreetDirectory::Graph& graph, const Link* link, const NodeLookup& nodeLookup, SegmentEdgeLookup& resSegEdgeLookup, SegmentVertexLookup& resSegVerLookup);
	/**
	 * Processes the  driving path at each bus stop
	 *
	 * @param graph is the graph object
	 * @param link is a pointer to one link
	 * @param nodeLookup is a lookup table to record node with associated vertex
	 * @param resSegVerLookup is a lookup table about the map from segments to vertex
	 * @param resSegEdgeLookup is a lookup table about the map from segments to edges
	 * @param resStopVerLookup is a lookup table about the map from stop to vertex
	 */
	void procAddDrivingBusStops(StreetDirectory::Graph& graph, const Link* link, const SegmentVertexLookup& segVerLookup, SegmentEdgeLookup& resSegLookup, StopVertexLookup& resStopVerLookup);
	/**
	 * Helper method: Add an edge, approximate the distance if necessary.
	 *
	 * @param graph is the graph object
	 * @param fromVertex is a source vertex in the graph
	 * @param toVertex is a sink vertex in the graph
	 * @param wp is a WayPoint which is held at the edge
	 * @param length is put as the weight at the edge
	 *
	 * @return an edge which successfully inserted into the graph
	 */
	static StreetDirectory::Edge addSimpleEdge(StreetDirectory::Graph& graph, StreetDirectory::Vertex& fromVertex, StreetDirectory::Vertex& toVertex, WayPoint wp, double length = -1.0);
	/**
	 * Helper method: Adds a vertex in the graph.
	 *
	 * @param graph is the graph object
	 * @param pos is a location point held in the vertex
	 *
	 * @return a vertex which successfully inserted into the graph
	 */
	static StreetDirectory::Vertex addSimpleVertex(StreetDirectory::Graph& graph, const Point& pos);
	/**
	 * Helper method: Finds the starting vertex for a given road segment.
	 *
	 * @param rs is a pointer of road segment
	 * @param segVerLookup is a lookup table about the map from segments to vertex
	 *
	 * @return a starting vertex for a given road segment
	 */
	static StreetDirectory::Vertex findStartingVertex(const RoadSegment* rs, const SegmentVertexLookup& segVerLookup);

	/**
	 * Helper method: Finds the ending vertex for a given road segment.
	 *
	 * @param rs is a pointer of road segment
	 * @param segVerLookup is a lookup table about the map from segments to vertex
	 *
	 * @return a ending vertex for a given road segment
	 */
	static StreetDirectory::Vertex findEndingVertex(const RoadSegment* rs, const SegmentVertexLookup& segVerLookup);
	/**
	 * Helper method: Computes the euclidean distance between two points.
	 *
	 * @param pt1 is first point
	 * @param pt2 is second point
	 *
	 * @return the euclidean distance between two points
	 */
	static double euclideanDist(const Point& pt1, const Point& pt2);

protected:
	/**
	 * Processes the driving path for link connectors
	 *
	 * @param graph is the graph object
	 * @param link is a pointer to one link
	 * @param nodeLookup is a lookup table to record node with associated vertex
	 */
	void procAddDrivingLinkConnectors(StreetDirectory::Graph& graph, const Node* node, const NodeLookup& nodeLookup);
	/**
	 * Processes the driving path for master nodes
	 *
	 * @param graph is the graph object
	 * @param allNodes is a lookup table to record node with associated vertex
	 * @param resLookup is a lookup table to record master nodes
	 */
	void procAddStartNodesAndEdges(StreetDirectory::Graph& graph, const NodeLookup& allNodes, NodeVertexLookup* resLookup = nullptr);
	/**
	 * Processes the driving path for each link
	 *
	 * @param graph is the graph object
	 * @param link is a pointer to one link
	 * @param nodeLookup is a lookup table to record node with associated vertex
	 * @param LinkEdgeLookup is a lookup table about the map from links to edges
	 * @param weight is the weight value used in the edge
	 */
	void procAddDrivingLinks(StreetDirectory::Graph& graph, const Link* link, const NodeLookup& nodeLookup, LinkEdgeLookup& resLinkEdgeLookup, LinkVertexLookup& resLinkVertexLookup,double weight=-1.0);
	/**
	 * Processes the driving path at each node
	 * @param graph is the graph object
	 * @param link is a pointer to one link
	 * @param nodeLookup is a lookup table to record node with associated vertex
	 */
	void procAddDrivingNodes(StreetDirectory::Graph& graph, const Link* link, NodeLookup& nodeLookup);
	/**
	 * Prints the graph to a given file
	 *
	 * @param outFile is out stream object for output
	 * @param graphType is to record the type of graph, driving or walking
	 * @param graph is the graph object
	 */
	void printGraph(std::ostream& outFile, const std::string& graphType, const StreetDirectory::Graph& graph) const;

public:

	/**
	 * black list constraint for our A* search algorithm
	 */
	struct BlackListEdgeConstraint
	{
	private:
		std::set<StreetDirectory::Edge> blackList;

	public:

		BlackListEdgeConstraint(const std::set<StreetDirectory::Edge>& list = std::set<StreetDirectory::Edge>()) : blackList(list)
		{
		}

		bool operator()(const StreetDirectory::Edge& e) const
		{
			//Include the edge if it's not in the blacklist.
			return blackList.find(e) == blackList.end();
		}
	};

	/**
	 * Distance heuristic for our A* search algorithm
	 */
	class DistanceHeuristicGraph : public boost::astar_heuristic<StreetDirectory::Graph, double>
	{
	private:
		const StreetDirectory::Graph* graph;
		StreetDirectory::Vertex goal;

	public:

		DistanceHeuristicGraph(const StreetDirectory::Graph* graph, StreetDirectory::Vertex goal) : graph(graph), goal(goal)
		{
		}

		double operator()(StreetDirectory::Vertex v)
		{
			const Point atPos = boost::get(boost::vertex_name, *graph, v);
			const Point goalPos = boost::get(boost::vertex_name, *graph, goal);
			return A_StarShortestPathImpl::euclideanDist(atPos, goalPos);
		}
	};

	/**
	 * Distance heuristic filter for A* search with blacklist.
	 */
	class DistanceHeuristicFiltered : public boost::astar_heuristic<boost::filtered_graph<StreetDirectory::Graph, BlackListEdgeConstraint>, double>
	{
	private:
		const boost::filtered_graph<StreetDirectory::Graph, BlackListEdgeConstraint>* graph;
		StreetDirectory::Vertex goal;

	public:

		DistanceHeuristicFiltered(const boost::filtered_graph<StreetDirectory::Graph, BlackListEdgeConstraint>* graph, StreetDirectory::Vertex goal) :
		graph(graph), goal(goal)
		{
		}

		double operator()(StreetDirectory::Vertex v)
		{
			const Point atPos = boost::get(boost::vertex_name, *graph, v);
			const Point goalPos = boost::get(boost::vertex_name, *graph, goal);
			return A_StarShortestPathImpl::euclideanDist(atPos, goalPos);
		}
	};

	/**
	 * Used to terminate our search
	 */
	struct Goal
	{
	};

	/**
	 * Goal visitor: terminates when a goal has been found.
	 */
	class GoalVisitor : public boost::default_astar_visitor
	{
	private:
		StreetDirectory::Vertex goal;

	public:

		GoalVisitor(StreetDirectory::Vertex goal) : goal(goal)
		{
		}

		template<class Graph>
		void examine_vertex(StreetDirectory::Vertex u, const Graph& g)
		{
			if (u == goal)
			{
				throw Goal();
			}
		}
	};

	/**
	 * Search shortest path with black list.
	 *
	 * @param graph is the graph object
	 * @param fromVertex is a source vertex in the graph
	 * @param toVertex is a sink vertex in the graph
	 * @param blackList is a black list used to block edges in the graph
	 *
	 * @return a shortest path
	 */
	static std::vector<WayPoint> searchShortestPathWithBlackList(const StreetDirectory::Graph& graph, const StreetDirectory::Vertex& fromVertex, const StreetDirectory::Vertex& toVertex, const std::set<StreetDirectory::Edge>& blacklist);
	/**
	 * Search the shortest path without black list.
	 *
	 * @param graph is the graph object
	 * @param fromVertex is a source vertex in the graph
	 * @param toVertex is a sink vertex in the graph
	 *
	 * @return a shortest path
	 */
	static std::vector<WayPoint> searchShortestPath(const StreetDirectory::Graph& graph, const StreetDirectory::Vertex& fromVertex, const StreetDirectory::Vertex& toVertex);

public:
	explicit A_StarShortestPathImpl(const RoadNetwork& network);
	virtual ~A_StarShortestPathImpl() {}
	A_StarShortestPathImpl();

	/**A map for drivers, containing road-segments as edges.*/
	StreetDirectory::Graph drivingSegMap;

	/**A map for pedestrians, containing side-walks and crossings as edges.*/
	StreetDirectory::Graph walkingMap;

	/**A map for drivers, containing road-links as edges.*/
	StreetDirectory::Graph drivingLinkMap;

	/**
	 * Lookup the master Node for each Node-related vertex in driving graph.The first item is the "source" vertex, used to search *from* that Node.
	 * The second item is the "sink" vertex, used to search *to* that Node.
	 */
	NodeVertexLookup drivingNodeLookup;

	/**
	 * Lookup the master Node for each Node-related vertex in walking graph.The first item is the "source" vertex, used to search *from* that Node.
	 * The second item is the "sink" vertex, used to search *to* that Node.
	 */
	NodeVertexLookup walkingNodeLookup;

	/**
	 * Lookup for road segments. This maps each RoadSegment to *all* Edges that are related to it
	 * (usually only 1, but bus stops may include a few "virtual" segments related to this Segment).
	 */
	SegmentEdgeLookup drivingSegmentEdgeLookup;

	/**Lookup for road links. This maps each Links to *all* Edges that are related to it.*/
	LinkEdgeLookup drivingLinkEdgeLookup;

    /** Lookup for road links mapping to its start/end vertex */
    LinkVertexLookup drivingLinkVertexLookup;

	/**Lookup for road segments mapping to vertexes.*/
	SegmentVertexLookup drivingSegVertexLookup;

	/**Lookups for Bus Stops in driving graph; the "source" and "sink" for these are the same.*/
	StopVertexLookup drivingBusStopLookup;

	/**Lookups for Bus Stops in walking graph; the "source" and "sink" for these are the same.*/
	StopVertexLookup walkingBusStopLookup;

	/**Used for locking modifications to the graph (currently only affects the blacklisted search).*/
	static boost::shared_mutex GraphSearchMutex;

	/**indicate whether segment graph is created*/
	bool isValidSegGraph;
	/**
	 * retrieve a vertex in the travel-time graph
	 * @param node is a pointer to the node
	 * @return a VertexDesc which hold vertex in the graph
	 */
	virtual StreetDirectory::VertexDesc DrivingVertex(const Node& node, TimeRange timeRange = Default, int randomGraphIdx = 0) const;
	/**
	 * retrieve a vertex in the travel-time graph
	 * @param link - link for which the Vertex is retrieved
	 * @return a VertexDesc which hold vertex in the graph
	 */
	virtual StreetDirectory::VertexDesc DrivingVertex(const Link& link, TimeRange timeRange = Default, int randomGraphIdx = 0) const;
	/**
	 * retrieve a vertex in the travel-time graph
	 * @param stop is a pointer to the bus stop
	 * @return a VertexDesc which hold vertex in the graph
	 */
	virtual StreetDirectory::VertexDesc DrivingVertex(const BusStop& stop, TimeRange timeRange = Default, int randomGraphIdx = 0) const;
	/**
	 * retrieve distance shortest driving path from original point to destination
	 * @param from is original vertex in the graph
	 * @param to is destination vertex in the graph
	 * @param blackList is the black list to mask some edges in the graph
	 * @return the shortest path result.
	 */
	virtual std::vector<WayPoint> GetShortestDrivingPath(const StreetDirectory::VertexDesc &from, const StreetDirectory::VertexDesc &to,
								const std::vector<const RoadSegment*> &blacklist) const;
	/**
	 * retrieve distance shortest driving path from original point to destination
	 * @param from is original vertex in the graph
	 * @param to is destination vertex in the graph
	 * @param blackList is the black list to mask some edges in the graph
	 * @return the shortest path result.
	 */
	virtual std::vector<WayPoint> GetShortestDrivingPath(const StreetDirectory::VertexDesc &from, const StreetDirectory::VertexDesc &to,
                                                        const std::vector<const Link*> &blacklist, TimeRange timeRange = Default, int randomGraphIdx = 0) const;
    /**
     * Return the distance-based shortest path to drive from to another. Performs a search (currently using
     *  the A* algorithm) from one to another.
     *
     * The function may return an empty array if \c toNode is not reachable from \c fromNode
     *
     * The resulting array contains only ROAD_SEGMENT or LINK and NODE WayPoint types. NODES at the beginning or end
     *  of the array can be ignored; NODES in the middle represent Link Connectors.
     *
     * @param from is a template parameter to hold starting point which is node or bus stop now
     * @param to is a template parameter to hold ending point which is node or bus stop now
     * @param blackList take black list when searching shortest path
     * @return the shortest path result.
     */
	template<class EDGE = sim_mob::Link,class VERTEX_FROM = sim_mob::Node,class VERTEX_TO = sim_mob::Node>
	std::vector<sim_mob::WayPoint> SearchShortestDrivingPath(const VERTEX_FROM &from, const VERTEX_TO &to,
			const std::vector<const EDGE*>& blackList =	std::vector<const EDGE*>()) const
	{
		std::vector<sim_mob::WayPoint> res;
		StreetDirectory::VertexDesc source = DrivingVertex(from);
		StreetDirectory::VertexDesc sink = DrivingVertex(to);
		res = GetShortestDrivingPath(source, sink, blackList);
		return res;
	}
	/**
	 * print out the graph structure
	 * @param outFile is a output stream is original vertex in the graph
	 */
	virtual void printDrivingGraph(std::ostream& outFile) const;
};

}
