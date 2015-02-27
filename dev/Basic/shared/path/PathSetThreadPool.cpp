/**
 * PathSetThreadPool.cpp
 *
 *  Created on: Feb 18, 2014
 *      Author: redheli
 */


#include "PathSetThreadPool.hpp"
#include "Path.hpp"
#include "geospatial/streetdir/AStarShortestTravelTimePathImpl.hpp"
#include "geospatial/streetdir/A_StarShortestPathImpl.hpp"

#include <stdlib.h>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/astar_search.hpp>
using namespace std;

namespace{
sim_mob::BasicLogger & logger = sim_mob::Logger::log("pathset.log");
}

sim_mob::PathSetWorkerThread::PathSetWorkerThread():s(nullptr)
{
	hasPath = false;
	dbgStr = "";
}
sim_mob::PathSetWorkerThread::~PathSetWorkerThread() { }

//1.Create Blacklist
//2.clear the shortestWayPointpath
//3.Populate a vector<WayPoint> with a blacklist involved
//or
//3.Populate a vector<WayPoint> without blacklist involvement
//4.populate a singlepath instance
void sim_mob::PathSetWorkerThread::run() {
	//Convert the blacklist into a list of blocked Vertices.
	std::set<StreetDirectory::Edge> blacklistV;
	std::map<const RoadSegment*, std::set<StreetDirectory::Edge> >::const_iterator lookIt;
	for(std::set<const RoadSegment*>::iterator it(excludeSeg.begin()); it != excludeSeg.end(); it++)
	{
		if((lookIt = segmentLookup->find(*it)) != segmentLookup->end())
		{
			blacklistV.insert(lookIt->second.begin(), lookIt->second.end());
		}
	}
	//used for error checking and validation
	unsigned long dbgPrev = 0;
	std::pair<StreetDirectory::Edge, bool> dbgPrevEdge;
	//output container
	vector<WayPoint> wps;

	if (blacklistV.empty())
	{
		std::list<StreetDirectory::Vertex> partialRes;
		//Use A* to search for a path
		//Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
		vector<StreetDirectory::Vertex> p(boost::num_vertices(*graph)); //Output variable
		vector<double> d(boost::num_vertices(*graph));  //Output variable
		try {
			boost::astar_search(*graph, fromVertex,
					sim_mob::A_StarShortestTravelTimePathImpl::distance_heuristic_graph(
							graph, toVertex),
					boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(
							sim_mob::A_StarShortestTravelTimePathImpl::astar_goal_visitor(toVertex)));
		}
		catch (sim_mob::A_StarShortestTravelTimePathImpl::found_goal& goal)
		{
			//Build backwards.
			for (StreetDirectory::Vertex v = toVertex;; v = p[v])
			{
				partialRes.push_front(v);
				if (p[v] == v)
				{
					break;
				}
			}
			//Now build forwards.
			std::list<StreetDirectory::Vertex>::const_iterator prev = partialRes.end();
			for (std::list<StreetDirectory::Vertex>::const_iterator it = partialRes.begin(); it != partialRes.end(); it++)
			{
				//Add this edge.
				if (prev != partialRes.end())
				{
					//This shouldn't fail.
					std::pair<StreetDirectory::Edge, bool> edge = boost::edge(*prev, *it, *graph);
					if (!edge.second) {
						Warn()
								<< "ERROR: Boost can't find an edge that it should know about."
								<< std::endl;
					}
					//Retrieve, add this edge's WayPoint.
					WayPoint wp = boost::get(boost::edge_name, *graph,edge.first);
					//todo this problem occurs during "highway bias distance" generation. dont know why, discarding the repeated segment
					if (wp.type_ == WayPoint::ROAD_SEGMENT && wp.roadSegment_->getId() == dbgPrev)
					{
							logger << dbgStr
									<< " 1ERROR-exeThis:: repeating segment found in path from "
									<< " seg: " << dbgPrev << " edge: " <<  edge.first << "  prev edge:" <<
									dbgPrevEdge.first << "   " << edge.second << "  " << dbgPrevEdge.second << "  " <<
									WayPoint(boost::get(boost::edge_name, *graph,dbgPrevEdge.first)).roadSegment_->getId() << "\n";
						dbgPrev = wps.rbegin()->roadSegment_->getId();
						dbgPrevEdge = edge;
					}
					else
					{
						wps.push_back(wp);
					}
				}
				//Save for later.
				prev = it;
			}
		}
	}
	else
	{
		//logger << "Blacklist NOT empty" << blacklistV.size() << std::endl;
		//Filter it.
		sim_mob::A_StarShortestPathImpl::blacklist_edge_constraint filter(blacklistV);
		boost::filtered_graph<StreetDirectory::Graph,sim_mob::A_StarShortestPathImpl::blacklist_edge_constraint> filtered(*graph, filter);
		////////////////////////////////////////
		// TODO: This code is copied (since filtered_graph is not the same as adjacency_list) from searchShortestPath.
		////////////////////////////////////////
		std::list<StreetDirectory::Vertex> partialRes;

		vector<StreetDirectory::Vertex> p(boost::num_vertices(filtered)); //Output variable
		vector<double> d(boost::num_vertices(filtered));  //Output variable

		//Use A* to search for a path
		//Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
		//...which is available under the terms of the Boost Software License, 1.0
		try {
			boost::astar_search(filtered, fromVertex,
					sim_mob::A_StarShortestPathImpl::distance_heuristic_filtered(&filtered, toVertex),
					boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(sim_mob::A_StarShortestPathImpl::astar_goal_visitor(toVertex)));
		}
		catch (sim_mob::A_StarShortestPathImpl::found_goal& goal)
		{
			//Build backwards.
			for (StreetDirectory::Vertex v = toVertex;; v = p[v])
			{
				partialRes.push_front(v);
				if (p[v] == v)
				{
					break;
				}
			}
			//Now build forwards.
			std::list<StreetDirectory::Vertex>::const_iterator prev = partialRes.end();
			for (std::list<StreetDirectory::Vertex>::const_iterator it = partialRes.begin(); it != partialRes.end(); it++)
			{
				//Add this edge.
				if (prev != partialRes.end())
				{
					//This shouldn't fail.
					std::pair<StreetDirectory::Edge, bool> edge = boost::edge(*prev, *it, filtered);
					if (!edge.second) {
						std::cerr << "ERROR: Boost can't find an edge that it should know about." << std::endl;
					}
					//Retrieve, add this edge's WayPoint.
					WayPoint wp = boost::get(boost::edge_name, filtered,edge.first);
					//todo this problem occurs during "highway bias distance" generation. dont know why, discarding the repeated segment
					if (wp.type_ == WayPoint::ROAD_SEGMENT && wp.roadSegment_->getId() == dbgPrev)
					{
							logger << dbgStr
									<< " 1ERROR-exeThis:: repeating segment found in path from "
									<< " seg: " << dbgPrev << " edge: " <<  edge.first << "  prev edge:" <<
									dbgPrevEdge.first << "   " << edge.second << "  " << dbgPrevEdge.second << "  " <<
									WayPoint(boost::get(boost::edge_name, *graph,dbgPrevEdge.first)).roadSegment_->getId() << "\n";
						dbgPrev = wps.rbegin()->roadSegment_->getId();
						dbgPrevEdge = edge;
					}
					else
					{
						wps.push_back(wp);
					}
				}

				//Save for later.
				prev = it;
			}
		}				//catch

	}				//else Blacklist


	if (wps.empty()) {
		hasPath = false;
	} else {
		// make sp id
		std::string id = sim_mob::makeWaypointsetString(wps);
		if(!id.size()){
			hasPath = false;
			logger << "Error: Empty choice!!! yet valid=>" << dbgStr <<  "\n";
		}
		else{
			logger << "Path generated through:" << dbgStr <<  ":" << id << "\n" ;
			s = new sim_mob::SinglePath();
			// fill data
			s->isNeedSave2DB = true;
			hasPath = true;
			s->pathSetId = ps->id;
			s->scenario = ps->scenario + dbgStr;
			s->init(wps);
			s->id = id;
			s->pathSize = 0;
			if(this->s->path.begin()->roadSegment_->getStart()->getID() != this->ps->subTrip.fromLocation.node_->getID())
			{
				safe_delete_item(s);
				hasPath = false;
				/**
				 * todo I havent yet figured out what this bug is, but it happens, mainly for random perturbation(time), discarding for now-vahid
				 * it may not be an issue after solving some multithreaded generation issue
				 */
				 logger << ps->scenario << dbgStr << " generation Mismatch : " << this->s->path.begin()->roadSegment_->getStart()->getID() << "   " <<  this->ps->subTrip.fromLocation.node_->getID() <<  "  " << fromVertex << "," << toVertex << std::endl;
			}
		}
	}
}

