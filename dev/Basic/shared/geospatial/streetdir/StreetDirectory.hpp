//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include "geospatial/network/Point.hpp"
#include "geospatial/network/WayPoint.hpp"

namespace sim_mob
{

class Node;
class BusStop;
class Link;
class RoadSegment;
class RoadNetwork;

enum TimeRange
{
	MorningPeak = 0,
	EveningPeak = 1,
	OffPeak = 2,
	Default = 3,
	HighwayBias_Distance = 4,
	HighwayBias_MorningPeak = 5,
	HighwayBias_EveningPeak = 6,
	HighwayBias_OffPeak = 7,
	HighwayBias_Default = 8,
	Random
};

class StreetDirectory : private boost::noncopyable
{
public:
	/**
	 * Internal typedef to StreetDirectory representing:
	 * key:		The "vertex_name" property.
	 * value:	The Point representing that vertex location. This point is not 100% accurate,
	 *			and should be considered a rough guideline as to the vertex location.
	 * This is just a handy way of retrieving data "stored" at a given vertex.
	 */
	typedef boost::property<boost::vertex_name_t, Point> VertexProperties;

	/**
	 * Internal typedef to StreetDirectory representing:
	 * keys:	The "edge_weight" and "edge_name" properties.
	 * values:	The Euclidean distance of this edge, and the WayPoint which represents this edge traversal.
	 * The distance is needed for our A* search, and the WayPoint is used when returning the actual results.
	 */
	typedef boost::property<boost::edge_weight_t, double, boost::property<boost::edge_name_t, WayPoint> > EdgeProperties;

	/**
	 * Internal typedef to StreetDirectory representing:
	 * The actual graph, bound to its VertexProperties and EdgeProperties (see those comments for details on what they store).
	 * You can use StreetDirectory::Graph to mean "a graph" in all contexts.
	 */
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProperties, EdgeProperties> Graph;

	/**
	 * Internal typedef to StreetDirectory representing:
	 * A Vertex within our graph. Internally, these are defined as some kind of integer, but you should
	 * simply treat this as an identifying handle.
	 * You can use StreetDirectory::Vertex to mean "a vertex" in all contexts.
	 */
	typedef Graph::vertex_descriptor Vertex;

	/**
	 * Internal typedef to StreetDirectory representing:
	 * An Edge within our graph. Internally, these are defined as pairs of integers (fromVertex, toVertex),
	 * but you should simply treat this as an identifying handle.
	 * You can use StreetDirectory::Edge to mean "an edge" in all contexts.
	 */
	typedef Graph::edge_descriptor Edge;

	/**
	 * A return value for "Driving/WalkingVertex"
	 */
	struct VertexDesc
	{
		/**Is this a valid value? If false, treat as "null"*/
		bool valid;

		/**The outgoing Vertex (used for "source" master nodes).*/
		Vertex source;

		/**The incoming Vertex (used for "sink" master nodes).*/
		Vertex sink;

		VertexDesc(bool valid = false) :
		valid(valid), source(Vertex()), sink(Vertex())
		{
		}
	};

	/**
	 * Provides an implementation of the StreetDirectory's shortest-path lookup functionality.
	 */
	class ShortestPathImpl
	{
	protected:
		/**
		 * Retrieves a Vertex based on a Node. A flag in the return value is false to indicate failure.
		 *
		 * @param Node is a reference to the input Node
		 *
		 * @return a VertexDesc with a flag value to indicate success or failure
		 */
		virtual VertexDesc DrivingVertex(const Node& n) const = 0;

		/**
		 * Retrieve a Vertex based on a BusStop. A flag in the return value is false to indicate failure.
		 *
		 * @param BusStop is a reference to the input bus stop
		 *
		 * @return a VertexDesc with a flag value to indicate success or failure
		 */
		virtual VertexDesc DrivingVertex(const BusStop& b) const = 0;

		/**
		 * Retrieves the shortest driving path from original point to destination
		 *
		 * @param from is original vertex in the graph
		 * @param to is destination vertex in the graph
		 * @param blackList is the black list to mask some edges in the graph
		 *
		 * @return the shortest path result.
		 */
		virtual std::vector<WayPoint> GetShortestDrivingPath(const VertexDesc &from, const VertexDesc &to, const std::vector<const RoadSegment *> &blackList) const = 0;

		/**
		 * Retrieves shortest driving path from original point to destination
		 *
		 * @param from is original vertex in the graph
		 * @param to is destination vertex in the graph
		 * @param blackList is the black list to mask some edge in the graph
		 *
		 * @return the shortest path result.
		 */
		virtual std::vector<WayPoint> GetShortestDrivingPath(const VertexDesc from, const VertexDesc to, const std::vector<const Link *> &blackList) const = 0;

		/**
		 * Prints the graph structure
		 * @param outFile is a output stream is original vertex in the graph
		 */
		virtual void printDrivingGraph(std::ostream& outFile) const = 0;

		/**friend class to access protected function*/
		friend class StreetDirectory;
	};

	/**
	 * Retrieves the current StreetDirectory instance. There can only be one StreetDirectory at any given time.
	 * @return the instance of the street directory
	 */
	static StreetDirectory& Instance()
	{
		return instance;
	}

	virtual ~StreetDirectory();

	/**
	 * Retrieves the implementation pointer to the shortest path based on distance
	 *
	 * @return the pointer of the implementation
	 */
	ShortestPathImpl* getDistanceImpl();

	/**
	 * Retrieves the implementation pointer to the shortest path based on travel time
	 * @return the pointer of the implementation
	 */
	ShortestPathImpl* getTravelTimeImpl();

	/**
	 * Return the distance-based shortest path to drive from one node to another. Performs a search (currently using
	 * the A* algorithm) from one node to another.
	 *
	 * The function may return an empty array if the "toNode" is not reachable from the "fromNode"
	 *
	 * The resulting array contains only ROAD_SEGMENT or LINK and NODE WayPoint types. NODES at the beginning or end
	 * of the array can be ignored; NODES in the middle represent Link Connectors.
	 *
	 * @param from is a template parameter to hold starting point which is node or bus stop now
	 * @param to is a template parameter to hold ending point which is node or bus stop now
	 * @param blackList take black list when searching shortest path
	 *
	 * @return the shortest path result.
	 */
	template<class EDGE = Link, class VERTEX_FROM = Node, class VERTEX_TO = Node>
	std::vector<WayPoint> SearchShortestDrivingPath(const VERTEX_FROM &from, const VERTEX_TO &to,
													std::vector<const EDGE*>& blackList = std::vector<const EDGE*>()) const
	{
		std::vector<WayPoint> res;
		if (!spImpl)
		{
			return res;
		}
		VertexDesc source = DrivingVertex(from);
		VertexDesc sink = DrivingVertex(to);
		res = spImpl->GetShortestDrivingPath(source, sink, blackList);
		return res;
	}

	/**
	 * Retrieves a vertex in the distance graph
	 *
	 * @param ver is a template parameter which is node or bus stop now
	 *
	 * @return a VertexDesc which hold vertex in the graph
	 */
	template<class VERTEX = Node>
	VertexDesc DrivingVertex(const VERTEX& ver) const
	{
		if (!spImpl)
		{
			return StreetDirectory::VertexDesc(false);
		}
		return spImpl->DrivingVertex(ver);
	}

	/**
	 * Retrieves a vertex in the time graph
	 *
	 * @param node
	 * @param timeRange is time range, default value is peak time in the morning
	 * @param randomGraphId is random graph index, default value is 0.
	 *
	 * @return a VertexDesc which hold vertex in the graph
	 */
	VertexDesc DrivingTimeVertex(const Node& node, TimeRange timeRange = MorningPeak, int randomGraphId = 0) const;

	/**
	 * Initialize the StreetDirectory object (to be invoked by the simulator kernel).
	 *
	 * @param network The road network that was loaded into the simulator.
	 */
	void Init(const RoadNetwork& network);

private:
	StreetDirectory();

	/**the single instance of the street directory */
	static StreetDirectory instance;

	/**Our current implementation of the shortest path searcher. */
	ShortestPathImpl* spImpl;

	/** shortest travel time path*/
	ShortestPathImpl* sttpImpl;
};
}

