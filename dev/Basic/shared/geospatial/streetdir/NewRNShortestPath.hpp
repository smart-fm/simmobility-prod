//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
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
#include "SMStreetDirectory.hpp"

#include "geospatial/simmobility_network/RoadNetwork.hpp"
#include "geospatial/simmobility_network/WayPoint.hpp"
#include "geospatial/streetdir/SMStreetDirectory.hpp"
#include "geospatial/simmobility_network/TurningPath.hpp"

namespace simmobility_network {



class NewRNShortestPath : public SMStreetDirectory::SMShortestPathImpl
{
public:
    explicit NewRNShortestPath(const simmobility_network::RoadNetwork* network);
    virtual ~NewRNShortestPath();

    virtual SMStreetDirectory::SMNodeVertexDesc DrivingVertex(const simmobility_network::Node& node);


	virtual std::vector<WayPoint> GetShortestDrivingPath(simmobility_network::Node* from,
														 simmobility_network::Node* to,
														 std::vector<const simmobility_network::Link*> blacklist);
	std::vector<simmobility_network::WayPoint> searchShortestPath(const SMStreetDirectory::SMVertex& fromVertex,
																const SMStreetDirectory::SMVertex& toVertex) const;

public:
	void buildGraph();
	simmobility_network::SMStreetDirectory::SMNodeVertexDesc  addNode(simmobility_network::Node* node);
	void addLink(simmobility_network::Link* link);
	void addTurningGroup(simmobility_network::TurningGroup* tg);
	void addTurningPath(simmobility_network::TurningPath* tp);

public:
    SMStreetDirectory::SMGraph graph;
    //Used for locking modifications to the graph
	static boost::shared_mutex GraphSearchMutex_;

    //Lookup the master Node for each Node-related vertex.
    //Note: The first item is the "source" vertex, used to search *from* that Node.
    //The second item is the "sink" vertex, used to search *to* that Node.
    std::map<const simmobility_network::Node*, simmobility_network::SMStreetDirectory::SMNodeVertexDesc > nodeLookup;
    std::map<const simmobility_network::Link*,simmobility_network::SMStreetDirectory::SMLinkVertexDesc> linkLookup;
    std::map<const simmobility_network::TurningPath*,simmobility_network::SMStreetDirectory::SMTurningPathVertexDesc> turningPathLookup;
private:
    const simmobility_network::RoadNetwork* network;

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
