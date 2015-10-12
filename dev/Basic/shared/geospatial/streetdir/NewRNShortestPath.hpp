//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
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
#include "SMStreetDirectory.hpp"

#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/network/WayPoint.hpp"
#include "geospatial/streetdir/SMStreetDirectory.hpp"
#include "geospatial/network/TurningPath.hpp"

namespace sim_mob {

class NewRNShortestPath : public SMStreetDirectory::SMShortestPathImpl
{
public:
    explicit NewRNShortestPath(const RoadNetwork* network);
    virtual ~NewRNShortestPath();

    virtual SMStreetDirectory::SMNodeVertexDesc DrivingVertex(const Node& node);


	virtual std::vector<WayPoint> GetShortestDrivingPath(Node* from,
														 Node* to,
														 std::vector<const Link*> blacklist);
	std::vector<WayPoint> searchShortestPath(const SMStreetDirectory::SMVertex& fromVertex,
																const SMStreetDirectory::SMVertex& toVertex) const;

public:
	void buildGraph();
	SMStreetDirectory::SMNodeVertexDesc addNode(const Node *node);
	void addLink(const Link *link);
	void addTurningGroup(const TurningGroup *tg);
	void addTurningPath(const TurningPath *tp);

public:
    SMStreetDirectory::SMGraph graph;
    //Used for locking modifications to the graph
	static boost::shared_mutex GraphSearchMutex_;

    //Lookup the master Node for each Node-related vertex.
    //Note: The first item is the "source" vertex, used to search *from* that Node.
    //The second item is the "sink" vertex, used to search *to* that Node.
    std::map<const Node*, SMStreetDirectory::SMNodeVertexDesc > nodeLookup;
    std::map<const Link*,SMStreetDirectory::SMLinkVertexDesc> linkLookup;
    std::map<const TurningPath*,SMStreetDirectory::SMTurningPathVertexDesc> turningPathLookup;
private:
    const RoadNetwork* network;

public:
    class distance_heuristic_graph : public boost::astar_heuristic<SMStreetDirectory::SMGraph, double>
	{
		public:
			distance_heuristic_graph(const SMStreetDirectory::SMGraph* graph, SMStreetDirectory::SMVertex goal)
				: m_graph(graph), m_goal(goal) {}

			double operator()(SMStreetDirectory::SMVertex v)
			{
				return 0.0;
			}

		private:
			const SMStreetDirectory::SMGraph* m_graph;
			SMStreetDirectory::SMVertex m_goal;
	};

    //Used to terminate our search (todo: is there a better way?)
	struct found_goal {};

	//Goal visitor: terminates when a goal has been found.
	//Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
	//which is available under the terms of the Boost Software License, 1.0
	class astar_goal_visitor : public boost::default_astar_visitor
	{
		public:
			astar_goal_visitor(SMStreetDirectory::SMVertex goal) : m_goal(goal) {}

			template <class Graph>
			void examine_vertex(SMStreetDirectory::SMVertex u, const Graph& g)
			{
				if(u == m_goal) { throw found_goal(); }
			}

		private:
			SMStreetDirectory::SMVertex m_goal;
	};

};

}// end namespace
