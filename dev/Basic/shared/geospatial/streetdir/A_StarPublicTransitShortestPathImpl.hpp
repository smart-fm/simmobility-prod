//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "geospatial/network/Point.hpp"
#include "util/GeomHelpers.hpp"
#include "util/LangHelpers.hpp"
#include <map>
#include <vector>
#include <string>
#include <ostream>

#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/astar_search.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>

#include "entities/params/PT_NetworkEntities.hpp"
#include "StreetDirectory.hpp"


namespace sim_mob {


class A_StarPublicTransitShortestPathImpl : public StreetDirectory::PublicTransitShortestPathImpl {
public:
    A_StarPublicTransitShortestPathImpl(const std::map<int,PT_NetworkEdge>& ,const std::map<std::string,PT_NetworkVertex>&);
    virtual ~A_StarPublicTransitShortestPathImpl() {}

public:
    /**
     * search shortest path in public transit network
     * @param from is original Id
     * @param to is destination Id
     * @param cost indicate the cost type when searching
     * @return searching result
     */
    std::vector<PT_NetworkEdge> searchShortestPath(const StreetDirectory::PT_VertexId& from,const StreetDirectory::PT_VertexId& to,const PT_CostLabel cost);

    /**
     * search k-shortest path in public transit network
     * @param k indicate how many times to repeat to search
     * @param from is original Id
     * @param to is destination Id
     * @param outPathList hold searching result
     */
    void searchK_ShortestPaths(uint32_t k, const StreetDirectory::PT_VertexId& from, const StreetDirectory::PT_VertexId& to, std::vector< std::vector<PT_NetworkEdge> > & outPathList);

    /**
     * search shortest path in public transit network
     * @param from is original Id
     * @param to is destination Id
     * @param blackList include black list of edges
     * @param pathCost indicate the cost type when searching
     * @return searching result
     */
    std::vector<PT_NetworkEdge> searchShortestPathWithBlacklist(const StreetDirectory::PT_VertexId& from,const StreetDirectory::PT_VertexId& to, const std::set<StreetDirectory::PT_EdgeId>& blackList, double& pathCost);

private:
    /**
     * initialize the public transit graph from public transit network
     * @param ptEdges hold all edges in public network graph
     * @param ptVertexes hold all vertices in public network graph
     */
    void initPublicNetwork(const std::map<int,PT_NetworkEdge>& ptEdges ,const std::map<std::string,PT_NetworkVertex>& ptVertexes);
    /**
     * process adding vertices to public graph from public transit vertex
     * @param graph is public transit graph
     * @param ptVertex is a vertex in public transit network
     */
    void procAddPublicNetworkVertices(StreetDirectory::PublicTransitGraph& graph,const PT_NetworkVertex& ptVertex);
    /**
     * process to add public transit edges to the graph
     * @param graph is public transit graph
     * @param ptEdge hold edge information in public transit network
     */
    void procAddPublicNetworkEdges(StreetDirectory::PublicTransitGraph& graph,const PT_NetworkEdge& ptEdge);
    /**
     * process to add weight value to the public transit graph
     * @param edge is the edge of public transit graph
     * @param graph is public transit graph
     * @param ptEdge hold edge information in public transit network
     */
    void procAddPublicNetworkEdgeWeights(const StreetDirectory::PT_Edge& edge,StreetDirectory::PublicTransitGraph& graph,const PT_NetworkEdge& ptEdge);
    /**
     * get cost for k-shortest path
     * @param ptEdge hold edge information in public transit network
     */
    double getK_ShortestPathCost(const PT_NetworkEdge& ptEdge);
    /**
     * get weight value for labeling approach
     * @param label is the index in the group of labeling approach
     * @param ptEdge hold edge information in public transit network
     */
    double getLabelingApproachWeights(int label,PT_NetworkEdge ptEdge);
    /**
     * get weight value for simulation approach
     * @param ptEdge hold edge information in public transit network
     */
    double getSimulationApproachWeights(PT_NetworkEdge ptEdge);

private:
    /**public transit graph*/
    StreetDirectory::PublicTransitGraph publicTransitMap;

    /**store the map from the index to the edge*/
    std::map<const int,StreetDirectory::PT_Edge> edgeMap;

    /**store the map from the index to the vertex*/
    std::map<const std::string,StreetDirectory::PT_Vertex> vertexMap;

    /**Used to terminate our search*/
    struct PT_FoundGoal {};

    /**
     * Goal visitor: terminates when a goal has been found.
     */
    class PT_GoalVisitor : public boost::default_astar_visitor {
    public:
    	PT_GoalVisitor(StreetDirectory::PT_Vertex goal) : goal(goal) {}

    	template <class Graph>
    	void examine_vertex(StreetDirectory::PT_Vertex u, const Graph& g) {
    		if(u == goal){
    			throw PT_FoundGoal();
    		}
    	}
    private:
    	StreetDirectory::PT_Vertex goal;
    };
	/**
	 * black list constraint for our A* search algorithm
	 */
	struct PT_EdgeConstraint {
    	PT_EdgeConstraint(const std::set<StreetDirectory::PT_Edge>& blacklist =
				std::set<StreetDirectory::PT_Edge>()) :	blacklist(blacklist) {
		}

		bool operator()(const StreetDirectory::PT_Edge& e) const {
			/**Include the edge if it's not in the blacklist.*/
			return blacklist.find(e) == blacklist.end();
		}

	private:
		std::set<StreetDirectory::PT_Edge> blacklist;
	};
	/**
	 * Distance heuristic for our A* search algorithm
	 */
	class PT_HeuristicGraph: public boost::astar_heuristic<
			StreetDirectory::PublicTransitGraph, double> {
	public:
		PT_HeuristicGraph(const StreetDirectory::PublicTransitGraph* graph,	StreetDirectory::PT_Vertex goal) :
				graph(graph), goal(goal) {
		}
		double operator()(StreetDirectory::PT_Vertex v) {
			return 0.0;
		}
	private:
		const StreetDirectory::PublicTransitGraph* graph;
		StreetDirectory::PT_Vertex goal;
	};

};
}
