/* Copyright Singapore-MIT Alliance for Research and Technology */

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

#include "StreetDirectory.hpp"


namespace sim_mob {

class Node;
class Link;


class A_StarShortestPathImpl : public StreetDirectory::ShortestPathImpl {
public:
    explicit A_StarShortestPathImpl(const RoadNetwork& network);
    virtual ~A_StarShortestPathImpl() {}

protected:
	virtual StreetDirectory::VertexDesc DrivingVertex(const Node& n) const;
	virtual StreetDirectory::VertexDesc WalkingVertex(const Node& n) const;
	virtual StreetDirectory::VertexDesc DrivingVertex(const BusStop& b) const;
	virtual StreetDirectory::VertexDesc WalkingVertex(const BusStop& b) const;

    virtual std::vector<WayPoint> GetShortestDrivingPath(StreetDirectory::VertexDesc from, StreetDirectory::VertexDesc to, std::vector<const sim_mob::RoadSegment*> blacklist) const;
    virtual std::vector<WayPoint> GetShortestWalkingPath(StreetDirectory::VertexDesc from, StreetDirectory::VertexDesc to) const;


    virtual void updateEdgeProperty();

    virtual void printDrivingGraph(std::ostream& outFile) const;

    virtual void printWalkingGraph(std::ostream& outFile) const;

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
    //std::vector<Node*> nodes_; // "Internal" uni-nodes that are created when building the maps.

    //Lookup the master Node for each Node-related vertex.
    //Note: The first item is the "source" vertex, used to search *from* that Node. The second item is the "sink" vertex, used to search *to* that Node.
    std::map<const Node*, std::pair<StreetDirectory::Vertex,StreetDirectory::Vertex> > drivingNodeLookup_;
    std::map<const Node*, std::pair<StreetDirectory::Vertex,StreetDirectory::Vertex> > walkingNodeLookup_;

    //Lookup for road segments. This maps each RoadSegment to *all* Edges that are related to it (usually only 1, but
    //  bus stops may include a few "virtual" segments related to this Segment).
    //Note that this map only contains Edges directly related to a Segment; Intersections between segments are not counted.
    std::map<const RoadSegment*, std::set<StreetDirectory::Edge> > drivingSegmentLookup_;

    //Lookups for Bus Stops; the "source" and "sink" for these are the same.
    std::map<const BusStop*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> > drivingBusStopLookup_;
    std::map<const BusStop*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> > walkingBusStopLookup_;

    //Used for locking modifications to the graph (currently only affects the blacklisted search).
    static boost::shared_mutex GraphSearchMutex_;

private:
    //Initialize
    void initDrivingNetworkNew(const std::vector<sim_mob::Link*>& links);
    void initWalkingNetworkNew(const std::vector<sim_mob::Link*>& links);

    //New processing code: Driving path
    void procAddDrivingNodes(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, std::map<const Node*, VertexLookup>& nodeLookup);
    void procAddDrivingLinks(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup, std::map<const RoadSegment*, std::set<StreetDirectory::Edge> >& resSegLookup);
    void procAddDrivingBusStops(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup, std::map<const BusStop*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >& resLookup, std::map<const RoadSegment*, std::set<StreetDirectory::Edge> >& resSegLookup);
    void procAddDrivingLaneConnectors(StreetDirectory::Graph& graph, const MultiNode* node, const std::map<const Node*, VertexLookup>& nodeLookup);

    //New processing code: Walking path
    void procAddWalkingNodes(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, std::map<const Node*, VertexLookup>& nodeLookup, std::map<const Node*, VertexLookup>& tempNodes);
    void procResolveWalkingMultiNodes(StreetDirectory::Graph& graph, const std::map<const Node*, VertexLookup>& unresolvedNodes, std::map<const Node*, VertexLookup>& nodeLookup);
    void procAddWalkingLinks(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup);
    void procAddWalkingBusStops(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup, std::map<const BusStop*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >& resLookup);
    void procAddWalkingCrossings(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup, std::set<const Crossing*>& completed);

    //New processing code: Shared
    void procAddStartNodesAndEdges(StreetDirectory::Graph& graph, const std::map<const Node*, VertexLookup>& allNodes, std::map<const Node*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >& resLookup);
    bool checkIfExist(std::vector<std::vector<WayPoint> > & paths, std::vector<WayPoint> & path);

    //Internal printing code.
    void printGraph(std::ostream& outFile, const std::string& graphType, const StreetDirectory::Graph& graph) const;

    //Helper functions
    static StreetDirectory::Edge AddSimpleEdge(StreetDirectory::Graph& graph, StreetDirectory::Vertex& fromV, StreetDirectory::Vertex& toV, sim_mob::WayPoint wp);
    static StreetDirectory::Vertex FindStartingVertex(const sim_mob::RoadSegment* rs, const std::map<const Node*, VertexLookup>& nodeLookup);
    static StreetDirectory::Vertex FindEndingVertex(const sim_mob::RoadSegment* rs, const std::map<const Node*, VertexLookup>& nodeLookup);




    //TODO: Replace with a space partition later.
    std::map<const Node*, std::pair<StreetDirectory::Vertex,StreetDirectory::Vertex> >::const_iterator
    searchVertex(const std::map<const Node*, std::pair<StreetDirectory::Vertex,StreetDirectory::Vertex> >& srcNodes, const Point2D& point) const;

    std::vector<WayPoint> searchShortestPathWithBlacklist(const StreetDirectory::Graph& graph, const StreetDirectory::Vertex& fromVertex, const StreetDirectory::Vertex& toVertex, const std::set<StreetDirectory::Edge>& blacklist) const;
    std::vector<WayPoint> searchShortestPath(const StreetDirectory::Graph& graph, const StreetDirectory::Vertex& fromVertex, const StreetDirectory::Vertex& toVertex) const;

    //Distance heuristic for our A* search algorithm
    //Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
    //...which is available under the terms of the Boost Software License, 1.0
    class distance_heuristic_graph : public boost::astar_heuristic<StreetDirectory::Graph, double> {
    public:
    	distance_heuristic_graph(const StreetDirectory::Graph* graph, StreetDirectory::Vertex goal) : m_graph(graph), m_goal(goal) {}
      double operator()(StreetDirectory::Vertex v) {
    	  const Point2D atPos = boost::get(boost::vertex_name, *m_graph, v);
    	  const Point2D goalPos = boost::get(boost::vertex_name, *m_graph, m_goal);

    	  return sim_mob::dist(atPos, goalPos);
      }
    private:
      const StreetDirectory::Graph* m_graph;
      StreetDirectory::Vertex m_goal;
    };


    struct blacklist_edge_constraint {
    	blacklist_edge_constraint(const std::set<StreetDirectory::Edge>& blacklist=std::set<StreetDirectory::Edge>()) : blacklist(blacklist)
    	{}

    	bool operator()(const StreetDirectory::Edge& e) const {
    		//Include the edge if it's not in the blacklist.
    		return blacklist.find(e)==blacklist.end();
    	}

    private:
    	std::set<StreetDirectory::Edge> blacklist;
    };


    //NOTE: For some reason, template deduction doesn't seem to work if we use "distance_heuristic<filtered>".
    //      For now I'm just separating these into their own classes; it's easier than trying to fix cascading template error messages.
    class distance_heuristic_filtered : public boost::astar_heuristic<boost::filtered_graph<StreetDirectory::Graph, blacklist_edge_constraint>, double> {
    public:
    	distance_heuristic_filtered(const boost::filtered_graph<StreetDirectory::Graph, blacklist_edge_constraint>* graph, StreetDirectory::Vertex goal) : m_graph(graph), m_goal(goal) {}
      double operator()(StreetDirectory::Vertex v) {
    	  const Point2D atPos = boost::get(boost::vertex_name, *m_graph, v);
    	  const Point2D goalPos = boost::get(boost::vertex_name, *m_graph, m_goal);

    	  return sim_mob::dist(atPos, goalPos);
      }
    private:
      const boost::filtered_graph<StreetDirectory::Graph, blacklist_edge_constraint>* m_graph;
      StreetDirectory::Vertex m_goal;
    };

    //Used to terminate our search (todo: is there a better way?)
    struct found_goal {};

    //Goal visitor: terminates when a goal has been found.
    //Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
    //...which is available under the terms of the Boost Software License, 1.0
    class astar_goal_visitor : public boost::default_astar_visitor {
    public:
      astar_goal_visitor(StreetDirectory::Vertex goal) : m_goal(goal) {}

      template <class Graph>
      void examine_vertex(StreetDirectory::Vertex u, const Graph& g) {
        if(u == m_goal)
          throw found_goal();
      }
    private:
      StreetDirectory::Vertex m_goal;
    };
};



}
