/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "GenConfig.h"
#include "util/LangHelpers.hpp"
#include "metrics/Length.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/Node.hpp"
#include "util/GeomHelpers.hpp"

#include <map>
#include <vector>
#include <string>

#include <boost/graph/astar_search.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include "StreetDirectory.hpp"


namespace sim_mob {

class Link;


class A_StarShortestPathImpl : public StreetDirectory::ShortestPathImpl {
public:
    explicit A_StarShortestPathImpl(const RoadNetwork& network);
    virtual ~A_StarShortestPathImpl();

protected:
    virtual std::vector<WayPoint> GetShortestDrivingPath(const Node& fromNode, const Node& toNode) const;

    virtual std::vector<WayPoint> shortestWalkingPath(const Point2D& fromPoint, const Point2D& toPoint) const;

    virtual void updateEdgeProperty();

    virtual void printDrivingGraph() const;

    virtual void printWalkingGraph() const;

private:
    ///Helper class:identify a Node exactly. This requires the incoming/outgoing RoadSegment(s), and the generated Vertex.
    struct NodeDescriptor {
    	const RoadSegment* before;
    	const RoadSegment* after;
    	int beforeLaneID; //Only used by the Walking graph
    	int afterLaneID;  //Only used by the Walking graph
    	Point2D tempPos; //Only used by Walking graph UniNodes, temporarily.
    	StreetDirectory::Vertex v;

    	//Equality comparison (be careful here!)
    	bool operator== (const NodeDescriptor& other) const {
    		return  (this->before==other.before) && (this->after==other.after) &&
    				(this->beforeLaneID==other.beforeLaneID) && (this->afterLaneID==other.afterLaneID);
    	}
    };

    ///Helper class:represents a vertex in our graph as a "Node" in Sim Mobility. Maintains mappings to the
    ///   original node and, if applicable, to any offshoot (child) nodes.
    struct VertexLookup {
    	const sim_mob::Node* origNode;
    	bool isUni;

    	//Lookup information. This is interpreted in two different ways:
    	//  1) UniNodes store the segment before/after this Node. This is inherent in the structure of the UniNode itself.
    	//  2) MultiNodes store the segment before/after this Intersection. This is derived from the Lane Connectors.
    	std::vector<NodeDescriptor> vertices;
    };

private:
    StreetDirectory::Graph drivingMap_; // A map for drivers, containing road-segments as edges.
    StreetDirectory::Graph walkingMap_; // A map for pedestrians, containing side-walks and crossings as edges.
    std::vector<Node*> nodes_; // "Internal" uni-nodes that are created when building the maps.

    //Lookup the master Node for each Node-related vertex.
    //Note: The first item is the "source" vertex, used to search *from* that Node. The second item is the "sink" vertex, used to search *to* that Node.
    std::map<const Node*, std::pair<StreetDirectory::Vertex,StreetDirectory::Vertex> > drivingNodeLookup_;
    std::map<const Node*, std::pair<StreetDirectory::Vertex,StreetDirectory::Vertex> > walkingNodeLookup_;

private:
    //Initialize
    void initDrivingNetworkNew(const std::vector<sim_mob::Link*>& links);
    void initWalkingNetworkNew(const std::vector<sim_mob::Link*>& links);

    //New processing code: Driving path
    void procAddDrivingNodes(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, std::map<const Node*, VertexLookup>& nodeLookup);
    void procAddDrivingLinks(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup);
    void procAddDrivingLaneConnectors(StreetDirectory::Graph& graph, const MultiNode* node, const std::map<const Node*, VertexLookup>& nodeLookup);

    //New processing code: Walking path
    void procAddWalkingNodes(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, std::map<const Node*, VertexLookup>& nodeLookup, std::map<const Node*, VertexLookup>& tempNodes);
    void procResolveWalkingMultiNodes(StreetDirectory::Graph& graph, const std::map<const Node*, VertexLookup>& unresolvedNodes, std::map<const Node*, VertexLookup>& nodeLookup);
    void procAddWalkingLinks(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup);
    void procAddWalkingCrossings(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup, std::set<const Crossing*>& completed);

    //New processing code: Shared
    void procAddStartNodesAndEdges(StreetDirectory::Graph& graph, const std::map<const Node*, VertexLookup>& allNodes, std::map<const Node*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >& resLookup);
    bool checkIfExist(std::vector<std::vector<WayPoint> > & paths, std::vector<WayPoint> & path);

    //Internal printing code.
    void printGraph(const std::string& graphType, const StreetDirectory::Graph& graph) const;

    //TODO: Replace with a space partition later.
    std::map<const Node*, std::pair<StreetDirectory::Vertex,StreetDirectory::Vertex> >::const_iterator
    searchVertex(const std::map<const Node*, std::pair<StreetDirectory::Vertex,StreetDirectory::Vertex> >& srcNodes, const Point2D& point) const;

    std::vector<WayPoint> searchShortestPath(const StreetDirectory::Graph& graph, const StreetDirectory::Vertex& fromVertex, const StreetDirectory::Vertex& toVertex) const;

    //Distance heuristic for our A* search algorithm
    //Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
    //...which is available under the terms of the Boost Software License, 1.0
    template <class Graph, class CostType>
    class distance_heuristic : public boost::astar_heuristic<Graph, CostType> {
    public:
      distance_heuristic(const Graph* graph, StreetDirectory::Vertex goal) : m_graph(graph), m_goal(goal) {}
      CostType operator()(StreetDirectory::Vertex v) {
    	  const Node* atPos = boost::get(boost::vertex_name, *m_graph, v);
    	  const Node* goalPos = boost::get(boost::vertex_name, *m_graph, m_goal);

    	  return sim_mob::dist(atPos->getLocation(), goalPos->getLocation());
      }
    private:
      const Graph* m_graph;
      StreetDirectory::Vertex m_goal;
    };

    //Used to terminate our search (todo: is there a better way?)
    struct found_goal {};

    //Goal visitor: terminates when a goal has been found.
    //Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
    //...which is available under the terms of the Boost Software License, 1.0
    template <class Vertex>
    class astar_goal_visitor : public boost::default_astar_visitor {
    public:
      astar_goal_visitor(Vertex goal) : m_goal(goal) {}

      template <class Graph>
      void examine_vertex(Vertex u, Graph& g) {
        if(u == m_goal)
          throw found_goal();
      }
    private:
      Vertex m_goal;
    };
};



}
