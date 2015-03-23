//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "geospatial/Point2D.hpp"
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

#include "../../entities/params/PT_NetworkEntities.hpp"
#include "StreetDirectory.hpp"


namespace sim_mob {


class A_StarPublicTransitShortestPathImpl : public StreetDirectory::PublicTransitShortestPathImpl {
public:
    A_StarPublicTransitShortestPathImpl(std::vector<PT_NetworkEdge> pt_edgeslist,std::vector<PT_NetworkVertex> pt_verticeslist);
    virtual ~A_StarPublicTransitShortestPathImpl() {}


public:
    StreetDirectory::PublicTransitGraph publictransitMap_;

    void initPublicNetwork(std::vector<PT_NetworkEdge>& ptEdgesList,std::vector<PT_NetworkVertex>& ptVerticesList);

    void procAddPublicNetworkVertices(StreetDirectory::PublicTransitGraph& graph,PT_NetworkVertex ptVertex);

    void procAddPublicNetworkEdges(StreetDirectory::PublicTransitGraph& graph,PT_NetworkEdge ptEdge);

    void procAddPublicNetworkEdgeweights(const StreetDirectory::PT_Edge&,StreetDirectory::PublicTransitGraph&,PT_NetworkEdge&);

    std::vector<StreetDirectory::PT_EdgeId> searchShortestPath(PT_NetworkVertex,PT_NetworkVertex,PT_WeightLabels);

    std::vector<StreetDirectory::PT_EdgeId> searchShortestPathWithBlacklist(PT_NetworkVertex,PT_NetworkVertex,PT_WeightLabels,const std::set<StreetDirectory::PT_Edge>&,double&);

    int getKShortestPaths(PT_NetworkVertex&,PT_NetworkVertex&, std::vector< std::vector<StreetDirectory::PT_EdgeId> > &);

    //double sim_mob::A_StarPublicTransitShortestPathImpl::getKshortestPathcost(const PT_NetworkEdge&);

    double getKshortestPathcost(const PT_NetworkEdge&);

    typedef boost::property_map<StreetDirectory::PublicTransitGraph, boost::vertex_index_t>::type IndexMap;

    std::map<const int,StreetDirectory::PT_Edge> edgeMap;

    std::map<const std::string,StreetDirectory::PT_Vertex> vertexMap;
    //Used to terminate our search (todo: is there a better way?)
    struct found_goal {};

    //Goal visitor: terminates when a goal has been found.
    //Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
    //...which is available under the terms of the Boost Software License, 1.0
    class astar_goal_visitor : public boost::default_astar_visitor {
    public:
      astar_goal_visitor(StreetDirectory::PT_Vertex goal) : m_goal(goal) {}

      template <class Graph>
      void examine_vertex(StreetDirectory::PT_Vertex u, const Graph& g) {
        if(u == m_goal)
          throw found_goal();
      }
    private:
      StreetDirectory::PT_Vertex m_goal;
    };

    struct blacklist_PT_edge_constraint {
    	blacklist_PT_edge_constraint(const std::set<StreetDirectory::PT_Edge>& blacklist=std::set<StreetDirectory::PT_Edge>()) : blacklist(blacklist)
    	{}

    	bool operator()(const StreetDirectory::PT_Edge& e) const {
    		//Include the edge if it's not in the blacklist.
    		return blacklist.find(e)==blacklist.end();
    	}

    private:
    	std::set<StreetDirectory::PT_Edge> blacklist;
    };

    class distance_heuristic_graph_PT : public boost::astar_heuristic<StreetDirectory::PublicTransitGraph, double> {
       public:
       	distance_heuristic_graph_PT(const StreetDirectory::PublicTransitGraph* graph, StreetDirectory::PT_Vertex goal) : m_graph(graph), m_goal(goal) {}
         double operator()(StreetDirectory::PT_Vertex v) {
       	  //const Point2D atPos = boost::get(boost::vertex_name, *m_graph, v);
       	  //const Point2D goalPos = boost::get(boost::vertex_name, *m_graph, m_goal);

       	  //return sim_mob::dist(atPos, goalPos);
        	 return 0;
         }
       private:
         const StreetDirectory::PublicTransitGraph* m_graph;
         StreetDirectory::PT_Vertex m_goal;
       };

};
}
