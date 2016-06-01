/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <cmath>
#include "A_StarShortestPathImpl.hpp"
#include "conf/ConfigManager.hpp"
#include "util/GeomHelpers.hpp"

namespace sim_mob {

class Node;
class Link;
class RoadSegment;
class RoadNetwork;

class A_StarShortestTravelTimePathImpl: public A_StarShortestPathImpl {
public:
	explicit A_StarShortestTravelTimePathImpl(const sim_mob::RoadNetwork& network);
	virtual ~A_StarShortestTravelTimePathImpl() {}

public:
	/**
	 * retrieve a vertex in the travel-time graph
	 * @param node is a pointer to the node
	 * @return a VertexDesc which hold vertex in the graph
	 */
	virtual StreetDirectory::VertexDesc DrivingVertex(const sim_mob::Node& node) const;
//	/**
//	 * retrieve a vertex in the travel-time graph for the morning peak hours
//	 * @param node is a pointer to the node
//	 * @return a VertexDesc which hold vertex information in the graph
//	 */
//	StreetDirectory::VertexDesc DrivingVertexMorningPeak(const sim_mob::Node& node) const;
//	/**
//	 * retrieve a vertex in the travel-time graph for the evening peak hours
//	 * @param node is a pointer to the node
//	 * @return a VertexDesc which hold vertex information in the graph
//	 */
//	StreetDirectory::VertexDesc DrivingVertexEveningPeak(const sim_mob::Node& node) const;
//	/**
//	 * retrieve a vertex in the travel-time graph for the normal-time hours
//	 * @param node is a pointer to the node
//	 * @return a VertexDesc which hold vertex information in the graph
//	 */
//	StreetDirectory::VertexDesc DrivingVertexNormalTime(const sim_mob::Node& node) const;
	/**
	 * retrieve a vertex in the travel-time graph for the default-time hours
	 * @param node is a pointer to the node
	 * @return a VertexDesc which hold vertex information in the graph
	 */
	StreetDirectory::VertexDesc DrivingVertexDefault(const sim_mob::Node& node) const;
	/**
	 * retrieve a vertex in the travel-time graph with highway bias distance
	 * @param node is a pointer to the node
	 * @return a VertexDesc which hold vertex information in the graph
	 */
	StreetDirectory::VertexDesc DrivingVertexHighwayBiasDistance(const sim_mob::Node& node) const;
//	/**
//	 * retrieve a vertex in the travel-time graph with highway bias morning peak
//	 * @param node is a pointer to the node
//	 * @return a VertexDesc which hold vertex information in the graph
//	 */
//	StreetDirectory::VertexDesc DrivingVertexHighwayBiasMorningPeak(const sim_mob::Node& node) const;
//	/**
//	 * retrieve a vertex in the travel-time graph with highway bias evening peak
//	 * @param node is a pointer to the node
//	 * @return a VertexDesc which hold vertex information in the graph
//	 */
//	StreetDirectory::VertexDesc DrivingVertexHighwayBiasEveningPeak(const sim_mob::Node& node) const;
//	/**
//	 * retrieve a vertex in the travel-time graph with highway bias normal time
//	 * @param node is a pointer to the node
//	 * @return a VertexDesc which hold vertex information in the graph
//	 */
//	StreetDirectory::VertexDesc DrivingVertexHighwayBiasNormalTIme(const sim_mob::Node& node) const;
	/**
	 * retrieve a vertex in the travel-time graph with highway bias default time
	 * @param node is a pointer to the node
	 * @return a VertexDesc which hold vertex information in the graph
	 */
	StreetDirectory::VertexDesc DrivingVertexHighwayBiasDefault(const sim_mob::Node& node) const;
	/**
	 * retrieve a vertex in the travel-time graph with highway bias random time
	 * @param node is a pointer to the node
	 * @return a VertexDesc which hold vertex information in the graph
	 */
	StreetDirectory::VertexDesc DrivingVertexRandom(const sim_mob::Node& node,	unsigned int randomGraphId = 0) const;
	/**
	 * retrieve travel-time shortest driving path from original point to destination
	 * @param from is original vertex in the graph
	 * @param to is destination vertex in the graph
	 * @param blackList is the black list to mask some edge in the graph
	 * @return the shortest path result.
	 */
	virtual std::vector<sim_mob::WayPoint> GetShortestDrivingPath(
			const StreetDirectory::VertexDesc& from,const StreetDirectory::VertexDesc& to,
			const std::vector<const sim_mob::Link*>& blacklist) const;
	/**
	 * retrieve travel-time shortest driving path from original point to destination
	 * @param from is original vertex in the graph
	 * @param to is destination vertex in the graph
	 * @param blackList is the black list to mask some edge in the graph
	 * @param timeRange indicate what time range is wanted
	 * @param randomGraphId indicate the index of random-time graphs' group
	 * @return the shortest path result.
	 */
	std::vector<sim_mob::WayPoint> GetShortestDrivingPath(
			const StreetDirectory::VertexDesc& from, const StreetDirectory::VertexDesc& to,
			const std::vector<const sim_mob::Link*>& blacklist,
			sim_mob::TimeRange timeRange = sim_mob::Default, unsigned int randomGraphId = 0) const;

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
	static std::vector<WayPoint> searchShortestTTPathWithBlackList(const StreetDirectory::Graph& graph, const StreetDirectory::Vertex& fromVertex, const StreetDirectory::Vertex& toVertex, const std::set<StreetDirectory::Edge>& blacklist);
	/**
	 * Search the shortest path without black list.
	 *
	 * @param graph is the graph object
	 * @param fromVertex is a source vertex in the graph
	 * @param toVertex is a sink vertex in the graph
	 *
	 * @return a shortest path
	 */
	static std::vector<WayPoint> searchShortestTTPath(const StreetDirectory::Graph& graph, const StreetDirectory::Vertex& fromVertex, const StreetDirectory::Vertex& toVertex);

public:
	/**
	 * Distance heuristic for our A* search algorithm
	 */
	class DistanceHeuristicGraph : public boost::astar_heuristic<StreetDirectory::Graph, double>
	{
	private:
		const StreetDirectory::Graph* graph;
		StreetDirectory::Vertex goal;
		double maxSegSpeed;

	public:

		DistanceHeuristicGraph(const StreetDirectory::Graph* graph, StreetDirectory::Vertex goal) : graph(graph), goal(goal),
		maxSegSpeed(ConfigManager::GetInstance().PathSetConfig().maxSegSpeed)
		{
		}

		double operator()(StreetDirectory::Vertex v)
		{
			const Point atPos = boost::get(boost::vertex_name, *graph, v);
			const Point goalPos = boost::get(boost::vertex_name, *graph, goal);
			double dx = goalPos.getX() - atPos.getX();
			double dy = goalPos.getY() - atPos.getY();
			double dz = goalPos.getZ() - atPos.getZ();
			
			return (sqrt(dx * dx + dy * dy + dz * dz) / maxSegSpeed);
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
		double maxSegSpeed;

	public:

		DistanceHeuristicFiltered(const boost::filtered_graph<StreetDirectory::Graph, BlackListEdgeConstraint>* graph, StreetDirectory::Vertex goal) :
		graph(graph), goal(goal), maxSegSpeed(ConfigManager::GetInstance().PathSetConfig().maxSegSpeed)
		{
		}

		double operator()(StreetDirectory::Vertex v)
		{
			const Point atPos = boost::get(boost::vertex_name, *graph, v);
			const Point goalPos = boost::get(boost::vertex_name, *graph, goal);
			double dx = goalPos.getX() - atPos.getX();
			double dy = goalPos.getY() - atPos.getY();
			double dz = goalPos.getZ() - atPos.getZ();
			
			return (sqrt(dx * dx + dy * dy + dz * dz) / maxSegSpeed);
		}
	};
	
//	/**
//	 * the graph with the travel time based on 06:00:00AM-10:00:00AM
//	 */
//	StreetDirectory::Graph drivingMapMorningPeak;
//	/**
//	 * the graph with the travel time based on 17:00:00PM-20:00:00PM
//	 */
//	StreetDirectory::Graph drivingMapEveningPeak;
//	/**
//	 * the graph with travel time excluding morning & evening peak time
//	 */
//	StreetDirectory::Graph drivingMapNormalTime;
	/**
	 * the graph with default travel time
	 */
	StreetDirectory::Graph drivingMapDefault;
	/**
	 * the graph with travel time modified by highway bias distance.
	 */
	StreetDirectory::Graph drivingMapHighwayBiasDistance;
//	/**
//	 * the graph with travel time modified by highway bias morning peak.
//	 */
//	StreetDirectory::Graph drivingMapHighwayBiasMorningPeak;
//	/**
//	 * the graph with travel time modified by highway bias evening peak.
//	 */
//	StreetDirectory::Graph drivingMapHighwayBiasEveningPeak;
//	/**
//	 * the graph with travel time modified by highway bias normal time
//	 */
//	StreetDirectory::Graph drivingMapHighwayBiasNormalTime;
	/**
	 * the graph with travel time modified by highway bias default time
	 */
	StreetDirectory::Graph drivingMapHighwayBiasDefault;
	/**
	 * the graph with random travel time
	 */
	std::vector<StreetDirectory::Graph> drivingMapRandomPool;
//	/**
//	 * the map lookup from node to vertex in morning peak graph
//	 */
//	NodeVertexLookup drivingNodeLookupMorningPeak;
//	/**
//	 * the map lookup from node to vertex in evening peak graph
//	 */
//	NodeVertexLookup drivingNodeLookupEveningPeak;
//	/**
//	 * the map lookup from node to vertex in normal-time graph
//	 */
//	NodeVertexLookup drivingNodeLookupNormalTime;
	/**
	 * the map lookup from node to vertex in default-time graph
	 */
	NodeVertexLookup drivingNodeLookupDefault;
	/**
	 * the map lookup from node to vertex in travel-time graph with highway bias distance
	 */
	NodeVertexLookup drivingNodeLookupHighwayBiasDistance;
//	/**
//	 * the map lookup from node to vertex in travel-time graph with highway bias morning peak
//	 */
//	NodeVertexLookup drivingNodeLookupHighwayBiasMorningPeak;
//	/**
//	 * the map lookup from node to vertex in travel-time graph with highway bias evening peak
//	 */
//	NodeVertexLookup drivingNodeLookupHighwayBiasEveningPeak;
//	/**
//	 * the map lookup from node to vertex in travel-time graph with highway bias normal time
//	 */
//	NodeVertexLookup drivingNodeLookupHighwayBiasNormalTime;
	/**
	 * the map lookup from node to vertex in travel-time graph with highway bias default time
	 */
	NodeVertexLookup drivingNodeLookupHighwayBiasDefault;
	/**
	 * the map lookup from node to vertex in travel-time graph with random time
	 */
	std::vector<NodeVertexLookup> drivingNodeLookupRandomPool;
//	/**
//	 * the map lookup from link to edge in morning peak graph
//	 */
//	LinkEdgeLookup drivingLinkLookupMorningPeak;
//	/**
//	 * the map lookup from link to edge in evening peak graph
//	 */
//	LinkEdgeLookup drivingLinkLookupEveningPeak;
//	/**
//	 * the map lookup from link to edge in normal-time graph
//	 */
//	LinkEdgeLookup drivingLinkLookupNormalTime;
	/**
	 * the map lookup from link to edge in default-time graph
	 */
	LinkEdgeLookup drivingLinkLookupDefault;
	/**
	 * the map lookup from link to edge in travel-time graph with highway bias distance
	 */
	LinkEdgeLookup drivingLinkLookupHighwayBiasDistance;
//	/**
//	 * the map lookup from link to edge in travel-time graph with highway bias morning peak
//	 */
//	LinkEdgeLookup drivingLinkLookupHighwayBiasMorningPeak;
//	/**
//	 * the map lookup from link to edge in travel-time graph with highway bias evening peak
//	 */
//	LinkEdgeLookup drivingLinkLookupHighwayBiasEveningPeak;
//	/**
//	 * the map lookup from link to edge in travel-time graph with highway bias normal time
//	 */
//	LinkEdgeLookup drivingLinkLookupHighwayBiasNormalTime;
	/**
	 * the map lookup from link to edge in travel-time graph with highway bias default time
	 */
	LinkEdgeLookup drivingLinkLookupHighwayBiasDefault;
	/**
	 * the map lookup from link to edge in travel-time graph with random time
	 */
	std::vector<LinkEdgeLookup> drivingLinkLookupRandomPool;
private:
    /**
     * Initialize network graph
     * @param roadNetwork is the reference to network object
     */
	void initDrivingNetwork(const sim_mob::RoadNetwork& roadNetwork);
	/**
	 * Get edge weight for different time range
	 * @param timeRange indicate what time range is wanted
	 * @param link is a pointer to the current link
	 * @return weight value for input time range
	 */
	double getEdgeWeight(const sim_mob::Link* link, sim_mob::TimeRange timeRange);
};

}


