//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once
#include <map>
#include <set>
#include <string>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

namespace sim_mob
{

/**
 * simple struct to hold information pertinent to an edge in the rail transit graph
 *
 * \author Harish Loganathan
 */
struct RTS_NetworkEdge
{
public:
	RTS_NetworkEdge();
	virtual ~RTS_NetworkEdge();

	double getEdgeTravelTime() const
	{
		return edgeTravelTime;
	}

	void setEdgeTravelTime(double edgeTravelTime)
	{
		this->edgeTravelTime = edgeTravelTime;
	}

	const std::string& getFromStationId() const
	{
		return fromStationId;
	}

	void setFromStationId(const std::string& fromStationId)
	{
		this->fromStationId = fromStationId;
	}

	const std::string& getToStationId() const
	{
		return toStationId;
	}

	void setToStationId(const std::string& toStationId)
	{
		this->toStationId = toStationId;
	}

	bool isTransferEdge() const
	{
		return transferEdge;
	}

	void setTransferEdge(bool transferEdge)
	{
		this->transferEdge = transferEdge;
	}

private:
	std::string fromStationId;
	std::string toStationId;
	bool transferEdge;
	double edgeTravelTime; //in seconds
};



/**
 * Singleton class encapsulating a graph representation of the rail transit system and exposes
 * shortest path finding algorithm in the rail transit network
 *
 * \note "RT" used in the names of the class members abbreviates "Rail Transit"
 *
 * \author Harish Loganathan
 */
struct VertexStruct;


class RailTransit
{
private:
	/** station name is the only property of vertices */
	typedef boost::property<boost::vertex_name_t, std::string> RT_VertexProperties;

	/** travel time in seconds is the only property of edges*/
	typedef boost::property<boost::edge_weight_t, double, boost::property<boost::edge_name_t, bool> > RT_EdgeProperties;

	/**
	 * The rail transit graph is a directed graph with above defined edge and vertex properties
	 */
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, RT_VertexProperties, RT_EdgeProperties> RailTransitGraph;

	typedef RailTransitGraph::vertex_descriptor RT_Vertex;

	typedef RailTransitGraph::edge_descriptor RT_Edge;

    /** the adjacency list representing the rail transit system*/
    RailTransitGraph railTransitGraph;

    /** lookup of station id => vertex in railTransitGraph */
    std::map<std::string, RT_Vertex> rtVertexLookup;

    /** single instance of RailTransit*/
    static RailTransit railTransit;

    /**
     * RailTransit is noncopyable
     */
    RailTransit(const RailTransit&) = delete;
    RailTransit& operator=(const RailTransit&) = delete;

    struct RT_FoundGoal
    {
    };

    /**
     * Goal visitor: terminates when a goal has been found.
     */
    class RT_GoalVisitor : public boost::default_dijkstra_visitor
    {
    public:
    	RT_GoalVisitor(RT_Vertex goal) :
    			goal(goal)
    	{
    	}

    	template<class Graph>
    	void examine_vertex(RT_Vertex u, const Graph& g)
    	{
    		if (u == goal)
    		{
    			throw RT_FoundGoal();
    		}
    	}
    private:
    	RT_Vertex goal;
    };

    /**
     * finds the vertex in rail transit graph corresponding to the supplied station name
     * @param vertexName station id
     * @return reference to RT_Vertex in rail transit graph;
     */
    bool findVertex(const std::string& vertexName,RailTransit::RT_Vertex& vertex) const;

public:
    RailTransit();
    virtual ~RailTransit();

    /**
     * returns reference to the single instance of RailTransit class
     */
    static RailTransit& getInstance();

    /**
     * loads vertex and edge data and builds RailTransitGraph adjacency list
     * @param vertices set of vertices for the graph (set container eliminates duplicates)
     * @param edges list of edges in the graph
     */
    void initGraph(const std::set<std::string>& vertices, const std::vector<RTS_NetworkEdge>& edges);

    /**
     * RTS has many train lines and interchanges connecting these lines.
     * Given a pair of train stations, this function fetches boarding and alighting stop in each line in the shortest path
     * between the pair of train stations supplied. This will help to identify the transfer points in the RT path.
     *
     * @param origin the origin train station id
     * @param destination the destination train station id
     * @return sequence of boarding and alighting stops along the shortest path
     */
    std::vector<std::string> fetchBoardAlightStopSeq(std::string origin, std::string dest) const;

};

}
