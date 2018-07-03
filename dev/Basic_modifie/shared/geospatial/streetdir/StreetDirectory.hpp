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
#include "entities/params/PT_NetworkEntities.hpp"

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
	HighwayBiasDistance = 4,
	HighwayBiasMorningPeak = 5,
	HighwayBiasEveningPeak = 6,
	HighwayBiasOffPeak = 7,
	HighwayBiasDefault = 8,
	Random
};

enum PT_CostLabel{
	KshortestPath=0,
	LabelingApproach1,
	LabelingApproach2,
	LabelingApproach3,
	LabelingApproach4,
	LabelingApproach5,
	LabelingApproach6,
	LabelingApproach7,
	LabelingApproach8,
	LabelingApproach9,
	LabelingApproach10,
	SimulationApproach1,
	SimulationApproach2,
	SimulationApproach3,
	SimulationApproach4,
	SimulationApproach5,
	SimulationApproach6,
	SimulationApproach7,
	SimulationApproach8,
	SimulationApproach9,
	SimulationApproach10,
	weightLabelscount
};

class StreetDirectory : private boost::noncopyable
{
public:
	/**
	 * Below public Transport graph is defined. We used different graph than private transit with a purpose
	 * of not using multiple graphs instead single graph of all Pathset Generation algorithms
	 *
	 */

	/*
	 * Just a typedef for edgeId and vertexId of the public network data. Not to get confused with graph edges and vertex.
	 */
	typedef int PT_EdgeId;
	typedef std::string PT_VertexId;

	/*
	 * This is the public transport edge properties. Different weights are being assigned used by different algorithms.
	 * In this way we get rid of using multiple graphs.
	 */
	struct PT_EdgeProperties{
		PT_EdgeId edge_id;

		/** This weight used by both K-shortest path and Link elimination approach algorithms.*/
		double kShortestPathWeight;

		/** Weights used by Labeling Approach */
		double labelingApproach1Weight;
		double labelingApproach2Weight;
		double labelingApproach3Weight;
		double labelingApproach4Weight;
		double labelingApproach5Weight;
		double labelingApproach6Weight;
		double labelingApproach7Weight;
		double labelingApproach8Weight;
		double labelingApproach9Weight;
		double labelingApproach10Weight;

		/**Weights used by Simulation approach*/
		double simulationApproach1Weight;
		double simulationApproach2Weight;
		double simulationApproach3Weight;
		double simulationApproach4Weight;
		double simulationApproach5Weight;
		double simulationApproach6Weight;
		double simulationApproach7Weight;
		double simulationApproach8Weight;
		double simulationApproach9Weight;
		double simulationApproach10Weight;
	};

	/*
	 * This is the public transport graph vertex property. It uses vertexId as vertex_name
	 */
	typedef boost::property<boost::vertex_name_t, std::string> PT_VertexProperties;

	/*
    	 * Definition of public transport graph is a directed graph with above defined edge and vertex properties
    	 */
    	typedef boost::adjacency_list<boost::vecS,
                                      boost::vecS,
                                      boost::directedS,
                                      PT_VertexProperties,
                                      PT_EdgeProperties> PublicTransitGraph;

    	typedef PublicTransitGraph::vertex_descriptor PT_Vertex;

    	typedef PublicTransitGraph::edge_descriptor PT_Edge;

    	/*
    	 * Its an abstract class for the public transport shortest path implementation .
    	 * This class is extended by A_StarPublicTransitShortestPathImpl class in A_StarPublicTransitShortestPathImpl.hpp
    	 */
    	class PublicTransitShortestPathImpl
	{
    	public:
    		/*
    	 	 * Pure virtual function to get shortest path in public transport network given pair of vertices
    	 	 */
    		virtual std::vector<PT_NetworkEdge> searchShortestPath(const PT_VertexId& from, const PT_VertexId& to,const PT_CostLabel cost)=0;

    		/*
    		 * Pure virtual function to get shortest path along with some blacklisted edges in public transport network given pair of vertices
    		 */
    		virtual std::vector<PT_NetworkEdge> searchShortestPathWithBlacklist(const StreetDirectory::PT_VertexId& from,const StreetDirectory::PT_VertexId& to, const std::set<StreetDirectory::PT_EdgeId>& blackList, double& cost)=0;
    		/*
    		 * Pure virtual Function to get first K shortest paths in public transport network given pair of vertices.
    		 */
    		virtual void searchK_ShortestPaths(uint32_t k, const StreetDirectory::PT_VertexId& from,const StreetDirectory::PT_VertexId& to, std::vector< std::vector<PT_NetworkEdge> > & outPathList)=0;
    		friend class StreetDirectory;
    	};

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
         	 * @param timeRange is time range, default value is peak time in the morning
        	 * @param randomGraphId is random graph index, default value is 0.
        	 *
		 * @return a VertexDesc with a flag value to indicate success or failure
		 */
        	virtual VertexDesc DrivingVertex(const Node& n, TimeRange timeRange = Default, int randomGraphIdx = 0) const = 0;

        	/**
        	 * Retrieves a Vertex based on a Link. A flag in the return value is false to indicate failure.
        	 *
        	 * @param link is a reference to the input Link
        	 * @param timeRange is time range, default value is peak time in the morning
        	 * @param randomGraphId is random graph index, default value is 0.
        	 *
        	 * @return a VertexDesc with a flag value to indicate success or failure
        	 */
        	virtual VertexDesc DrivingVertex(const Link& n, TimeRange timeRange = Default, int randomGraphIdx = 0) const = 0;

		/**
		 * Retrieve a Vertex based on a BusStop. A flag in the return value is false to indicate failure.
		 *
		 * @param BusStop is a reference to the input bus stop
        	 * @param timeRange is time range, default value is peak time in the morning
        	 * @param randomGraphId is random graph index, default value is 0.
        	 *
		 * @return a VertexDesc with a flag value to indicate success or failure
		 */
       		virtual VertexDesc DrivingVertex(const BusStop& b, TimeRange timeRange = Default, int randomGraphIdx = 0) const = 0;

		/**
		 * Retrieves shortest driving path from original point to destination
		 *
		 * @param from is original vertex in the graph
		 * @param to is destination vertex in the graph
		 * @param blackList is the black list to mask some edge in the graph
        	 * @param timeRange is time range, default value is peak time in the morning
        	 * @param randomGraphId is random graph index, default value is 0.
		 *
		 * @return the shortest path result.
		 */
        	virtual std::vector<WayPoint> GetShortestDrivingPath(const VertexDesc &from, const VertexDesc &to, const std::vector<const Link *> &blackList,
                                                             TimeRange timeRange = Default, int randomGraphIdx = 0) const = 0;

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
	ShortestPathImpl* getDistanceImpl() const;

	/**
	 * Retrieves the implementation pointer to the shortest path based on travel time
	 * @return the pointer of the implementation
	 */
	ShortestPathImpl* getTravelTimeImpl() const;

	/**
	 * Retrieves the implementation pointer to the shortest path based on public transit
	 * @return the pointer of the implementation
	 */
	PublicTransitShortestPathImpl* getPublicTransitShortestPathImpl() const;

	/**
	 * Return the distance-based shortest path to drive from one node to another. Performs a search (currently using
	 * the A* algorithm) from one node to another.
	 *
	 * The function may return an empty array if the "toNode" is not reachable from the "fromNode"
	 *
	 * The resulting array contains LINK or NODE WayPoint types. NODES at the beginning or end
	 * of the array can be ignored; NODES in the middle represent Link Connectors.
	 *
    	 * @param from is a parameter to hold starting node/link
    	 * @param to is a parameter to hold ending node/link
	 * @param blackList take black list when searching shortest path
	 *
	 * @return the shortest path result.
	 */
    	template<class OriginType, class DestinationType>
    	std::vector<WayPoint> SearchShortestDrivingPath(const OriginType &from, const DestinationType &to,
                                                    const std::vector<const Link*>& blackList = std::vector<const Link*>()) const
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
    	 * @param item is a parameter which can be node/link
    	 * @param timeRange is time range, default value is peak time in the morning
    	 * @param randomGraphId is random graph index, default value is 0.
    	 *
	 * @return a VertexDesc which hold vertex in the graph
	 */
    	template<class RoadItemType>
    	VertexDesc DrivingVertex(const RoadItemType& item, TimeRange timeRange = Default, int randomGraphId = 0) const
   	{
        	if(!spImpl)
        	{
            		return VertexDesc(false);
       		}
        	return spImpl->DrivingVertex(item, timeRange, randomGraphId);
    	}

	/**
	 * Retrieves a vertex in the time graph
	 *
     	 * @param item is a parameter which can be node/link
	 * @param timeRange is time range, default value is peak time in the morning
	 * @param randomGraphId is random graph index, default value is 0.
	 *
	 * @return a VertexDesc which hold vertex in the graph
	 */
    	template<class RoadItemType>
    	VertexDesc DrivingTimeVertex(const RoadItemType& item, TimeRange timeRange = MorningPeak, int randomGraphId = 0) const
    	{
        	if(!sttpImpl)
        	{
            		return VertexDesc(false);
        	}
        	return sttpImpl->DrivingVertex(item, timeRange, randomGraphId);
    	}
	/**
	 * Return the time-based shortest path to drive from to another. Performs a search (currently using
	 *  the A* algorithm) from one to another.
	 * @param from hold starting node
	 * @param to hold ending node
	 * @param blackList take black list when searching shortest path
	 * @param timeRange is time range, default value is peak time in the morning
	 * @param randomGraphId is random graph index, default value is 0.
	 * @return the shortest path result.
	 */
    	template<class RoadItemType1, class RoadItemType2>
	std::vector<sim_mob::WayPoint> SearchShortestDrivingTimePath(
            const RoadItemType1& from,	const RoadItemType1& to,
	    const std::vector<const sim_mob::Link*>& blacklist =std::vector<const sim_mob::Link*>(),
	    TimeRange timeRange = MorningPeak,unsigned int randomGraphIdx = 0) const
    	{
        	if (!sttpImpl) 
		{
            		return std::vector<sim_mob::WayPoint>();
        	}
        	VertexDesc source = DrivingTimeVertex(from, timeRange, randomGraphIdx);
        	VertexDesc sink = DrivingTimeVertex(to, timeRange, randomGraphIdx);
        	std::vector<sim_mob::WayPoint> res = sttpImpl->GetShortestDrivingPath(source, sink, blacklist, timeRange, randomGraphIdx);
        	return res;
    	}

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

    	/**Public Transit implementation*/
    	PublicTransitShortestPathImpl* ptImpl;
};
}

