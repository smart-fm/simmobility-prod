/**
 * PathSetThreadPool.cpp
 *
 *  Created on: Feb 18, 2014
 *      Author: redheli
 */

#include <stdlib.h>
#include "PathSetThreadPool.h"

using namespace std;


sim_mob::PathSetWorkerThread::PathSetWorkerThread()
{
	s = new sim_mob::SinglePath();
	hasPath = false;
	dbgStr = "";
}
sim_mob::PathSetWorkerThread::~PathSetWorkerThread()
{
	if(s) delete s; s=NULL;
}

//1.Create Blacklist
//2.clear the shortestWayPointpath
//3.Populate a vector<WayPoint> with a blacklist involved
//or
//3.Populate a vector<WayPoint> without blacklist involvement
//4.populate a singlepath instance
void sim_mob::PathSetWorkerThread::executeThis() {
	//Convert the blacklist into a list of blocked Vertices.
	std::set<StreetDirectory::Edge> blacklistV;
	if (excludeSeg) {
		std::map<const RoadSegment*, std::set<StreetDirectory::Edge> >::const_iterator lookIt =
				segmentLookup->find(excludeSeg);
		if (lookIt != segmentLookup->end()) {
			blacklistV.insert(lookIt->second.begin(), lookIt->second.end());
		}
	}
	//NOTE: choiceSet[] is an interesting optimization, but we don't need to save cycles (and we definitely need to save memory).
	//      The within-day choice set model should have this kind of optimization; for us, we will simply search each time.
	//TODO: Perhaps caching the most recent X searches might be a good idea, though. ~Seth.

	vector<WayPoint> wp;
	if (!s) {
		std::cout << "in thread new failed " << std::endl;
	}
	s->shortestWayPointpath.clear();
	if (blacklistV.empty()) {
		std::list<StreetDirectory::Vertex> partialRes;
		//Use A* to search for a path
		//Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
		vector<StreetDirectory::Vertex> p(boost::num_vertices(*graph)); //Output variable
		vector<double> d(boost::num_vertices(*graph));  //Output variable
		try {
			boost::astar_search(*graph, *fromVertex,
					sim_mob::A_StarShortestTravelTimePathImpl::distance_heuristic_graph(
							graph, *toVertex),
					boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(
							sim_mob::A_StarShortestTravelTimePathImpl::astar_goal_visitor(*toVertex)));
		} catch (sim_mob::A_StarShortestTravelTimePathImpl::found_goal& goal) {
			//Build backwards.
			for (StreetDirectory::Vertex v = *toVertex;; v = p[v]) {
				partialRes.push_front(v);
				if (p[v] == v) {
					break;
				}
			}
			//Now build forwards.
			std::list<StreetDirectory::Vertex>::const_iterator prev =
					partialRes.end();
			for (std::list<StreetDirectory::Vertex>::const_iterator it =
					partialRes.begin(); it != partialRes.end(); it++) {
				//Add this edge.
				if (prev != partialRes.end()) {
					//This shouldn't fail.
					std::pair<StreetDirectory::Edge, bool> edge = boost::edge(
							*prev, *it, *graph);
					if (!edge.second) {
						Warn()
								<< "ERROR: Boost can't find an edge that it should know about."
								<< std::endl;
					}
					//Retrieve, add this edge's WayPoint.
					WayPoint w = boost::get(boost::edge_name, *graph,
							edge.first);
					wp.push_back(w);
				}
				//Save for later.
				prev = it;
			}
		}
	} else {
		//Filter it.
		sim_mob::A_StarShortestPathImpl::blacklist_edge_constraint filter(
				blacklistV);
		boost::filtered_graph<StreetDirectory::Graph,
				sim_mob::A_StarShortestPathImpl::blacklist_edge_constraint> filtered(
				*graph, filter);
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
			boost::astar_search(filtered, *fromVertex,
					sim_mob::A_StarShortestPathImpl::distance_heuristic_filtered(
							&filtered, *toVertex),
					boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(
							sim_mob::A_StarShortestPathImpl::astar_goal_visitor(*toVertex)));
		} catch (sim_mob::A_StarShortestPathImpl::found_goal& goal) {
			//Build backwards.
			for (StreetDirectory::Vertex v = *toVertex;; v = p[v]) {
				partialRes.push_front(v);
				if (p[v] == v) {
					break;
				}
			}
			//Now build forwards.
			std::list<StreetDirectory::Vertex>::const_iterator prev =
					partialRes.end();
			for (std::list<StreetDirectory::Vertex>::const_iterator it =
					partialRes.begin(); it != partialRes.end(); it++) {
				//Add this edge.
				if (prev != partialRes.end()) {
					//This shouldn't fail.
					std::pair<StreetDirectory::Edge, bool> edge = boost::edge(
							*prev, *it, filtered);
					if (!edge.second) {
						Warn()
								<< "ERROR: Boost can't find an edge that it should know about."
								<< std::endl;
					}
					//Retrieve, add this edge's WayPoint.
					WayPoint w = boost::get(boost::edge_name, filtered,
							edge.first);
					wp.push_back(w);
				}

				//Save for later.
				prev = it;
			}
		}				//catch

	}				//else



	if (wp.empty()) {
		hasPath = false;
	} else {
		// make sp id
		std::string id = sim_mob::makeWaypointsetString(wp);
		if(!id.size()){
			sim_mob::Logger::log["pathset"] << "Error: Empty choice!!! yet valid=>" << dbgStr <<  "\n";
		}
		else{
			sim_mob::Logger::log["path_set"] << "Path generated through:" << dbgStr <<  ":" << id << "\n" ;
		}
		// fill data
		s->pathSet = ps;
		s->isNeedSave2DB = true;
		hasPath = true;
		s->init(wp);
		sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(s);
		s->highWayDistance = sim_mob::calculateHighWayDistance(s);
		s->fromNode = fromNode;
		s->toNode = toNode;
		s->pathset_id = ps->id;
		s->length = sim_mob::generateSinglePathLength(s->shortestWayPointpath);
		s->id = id;
		s->scenario = ps->scenario;
		s->pathsize = 0;
		s->travel_cost = sim_mob::getTravelCost2(s);
		s->travle_time = s->pathSet->psMgr->getTravelTime(s);
	}
}

