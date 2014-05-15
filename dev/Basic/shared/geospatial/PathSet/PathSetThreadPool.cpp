/*
 * PathSetThreadPool.cpp
 *
 *  Created on: Feb 18, 2014
 *      Author: redheli
 */

#include "PathSetThreadPool.h"

/*
    Thread Pool implementation for unix / linux environments
    Copyright (C) 2008 Shobhit Gupta

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include "PathSetThreadPool.h"
#include "geospatial/streetdir/PublicTransportStreetDirectory.h"

using namespace std;

pthread_mutex_t sim_mob::PathSetThreadPool::mutexSync = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sim_mob::PathSetThreadPool::mutexWorkCompletion = PTHREAD_MUTEX_INITIALIZER;

sim_mob::PathSetWorkerThread::PathSetWorkerThread()
{
	s = new sim_mob::SinglePath();
	hasPath = false;
}
sim_mob::PathSetWorkerThread::~PathSetWorkerThread()
{
	if(s) delete s; s=NULL;
}
void sim_mob::PathSetWorkerThread::executeThis()
{
//	sim_mob::threadArg *ta(static_cast<sim_mob::threadArg *>(arg));
//		if(!ta)
//		{
//			std::cout<<"pathset thread no arg"<<std::endl;
//			return NULL;
//		}

		//Convert the blacklist into a list of blocked Vertices.
		std::set<StreetDirectory::Edge> blacklistV;
	//	const RoadSegment* exclude_seg = NULL;
		if(excludeSeg)
		{
//		std::vector<const RoadSegment*> blacklist; blacklist.push_back(excludeSeg);
//		for (vector<const RoadSegment*>::iterator it=blacklist.begin(); it!=blacklist.end(); it++) {
			std::map<const RoadSegment*, std::set<StreetDirectory::Edge> >::const_iterator lookIt = segmentLookup->find(excludeSeg);
			if (lookIt!=segmentLookup->end()) {
				blacklistV.insert(lookIt->second.begin(), lookIt->second.end());
			}
//		}
		}
//		std::cout<<"pathset thread "<<pthread_self()<<" 1"<<std::endl;
	    //NOTE: choiceSet[] is an interesting optimization, but we don't need to save cycles (and we definitely need to save memory).
	    //      The within-day choice set model should have this kind of optimization; for us, we will simply search each time.
	    //TODO: Perhaps caching the most recent X searches might be a good idea, though. ~Seth.

		vector<WayPoint> wp;
		if(!s)
		{
			std::cout<<"in thread new failed "<<std::endl;
		}
		s->shortestWayPointpath.clear();
		if (blacklistV.empty()) {
	//		vector<WayPoint> res;
//			std::cout<<"pathset thread "<<pthread_self()<<" 2"<<std::endl;
				std::list<StreetDirectory::Vertex> partialRes;

				//Lock for read access.
	//			{
	//			boost::shared_lock<boost::shared_mutex> lock(GraphSearchMutex_);

				//Use A* to search for a path
				//Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
				//...which is available under the terms of the Boost Software License, 1.0
				vector<StreetDirectory::Vertex> p(boost::num_vertices(*graph));  //Output variable
				vector<double> d(boost::num_vertices(*graph));  //Output variable
				try {
					boost::astar_search(
						*graph,
						*fromVertex,
						sim_mob::A_StarShortestTravelTimePathImpl::distance_heuristic_graph(graph, *toVertex),
						boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(sim_mob::A_StarShortestTravelTimePathImpl::astar_goal_visitor(*toVertex))
					);
				} catch (sim_mob::A_StarShortestTravelTimePathImpl::found_goal& goal) {
					//Build backwards.
					for (StreetDirectory::Vertex v=*toVertex;;v=p[v]) {
						partialRes.push_front(v);
					    if(p[v] == v) {
					    	break;
					    }
					}

					//Now build forwards.
					std::list<StreetDirectory::Vertex>::const_iterator prev = partialRes.end();
					for (std::list<StreetDirectory::Vertex>::const_iterator it=partialRes.begin(); it!=partialRes.end(); it++) {
						//Add this edge.
						if (prev!=partialRes.end()) {
							//This shouldn't fail.
							std::pair<StreetDirectory::Edge, bool> edge = boost::edge(*prev, *it, *graph);
							if (!edge.second) {
								Warn() <<"ERROR: Boost can't find an edge that it should know about." <<std::endl;
	//							return std::vector<WayPoint>();
							}

							//Retrieve, add this edge's WayPoint.
							WayPoint w = boost::get(boost::edge_name, *graph, edge.first);
							wp.push_back(w);
	//						result->push_back(wp);
//							s->shortestWayPointpath.push_back(w);
//							if(w.type_==WayPoint::ROAD_SEGMENT){
//								s->shortestSegPath.insert(std::make_pair(w.roadSegment_,w));
//							}
						}

						//Save for later.
						prev = it;
					}
				}
	//			} //End boost mutex lock for read access.

	//			return res;
		} else {
			// TODO
	//		return searchShortestPathWithBlacklist(drivingMap_, fromV, toV, blacklistV);
			//Lock for upgradable access, then upgrade to exclusive access.
				//NOTE: Locking is probably not necessary, but for now I'd rather just be extra-safe.
	//			boost::upgrade_lock<boost::shared_mutex> lock(GraphSearchMutex_);
	//			boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
//			std::cout<<"pathset thread "<<pthread_self()<<" 3"<<std::endl;
				//Filter it.
				sim_mob::A_StarShortestPathImpl::blacklist_edge_constraint filter(blacklistV);
				boost::filtered_graph<StreetDirectory::Graph, sim_mob::A_StarShortestPathImpl::blacklist_edge_constraint> filtered(*graph, filter);


				////////////////////////////////////////
				// TODO: This code is copied (since filtered_graph is not the same as adjacency_list) from searchShortestPath.
				////////////////////////////////////////
	//			vector<WayPoint> res;
				std::list<StreetDirectory::Vertex> partialRes;

				vector<StreetDirectory::Vertex> p(boost::num_vertices(filtered));  //Output variable
				vector<double> d(boost::num_vertices(filtered));  //Output variable

				//Use A* to search for a path
				//Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
				//...which is available under the terms of the Boost Software License, 1.0
				try {
					boost::astar_search(
							filtered,
						*fromVertex,
						sim_mob::A_StarShortestPathImpl::distance_heuristic_filtered(&filtered, *toVertex),
						boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(sim_mob::A_StarShortestPathImpl::astar_goal_visitor(*toVertex))
					);
				} catch (sim_mob::A_StarShortestPathImpl::found_goal& goal) {
					//Build backwards.
//					std::cout<<"pathset thread "<<pthread_self()<<" 4"<<std::endl;
					for (StreetDirectory::Vertex v=*toVertex;;v=p[v]) {
						partialRes.push_front(v);
					    if(p[v] == v) {
					    	break;
					    }
					}
//					std::cout<<"pathset thread "<<pthread_self()<<" 5"<<std::endl;
					//Now build forwards.
					std::list<StreetDirectory::Vertex>::const_iterator prev = partialRes.end();
					for (std::list<StreetDirectory::Vertex>::const_iterator it=partialRes.begin(); it!=partialRes.end(); it++) {
						//Add this edge.
						if (prev!=partialRes.end()) {
							//This shouldn't fail.
							std::pair<StreetDirectory::Edge, bool> edge = boost::edge(*prev, *it, filtered);
							if (!edge.second) {
								Warn() <<"ERROR: Boost can't find an edge that it should know about." <<std::endl;
	//							return std::vector<WayPoint>();
							}
//							std::cout<<"pathset thread "<<pthread_self()<<" 6"<<std::endl;
							//Retrieve, add this edge's WayPoint.
							WayPoint w = boost::get(boost::edge_name, filtered, edge.first);
							wp.push_back(w);
//							s->shortestWayPointpath.push_back(w);
//							if(w.type_==WayPoint::ROAD_SEGMENT){
//								s->shortestSegPath.insert(std::make_pair(w.roadSegment_,w));
//							}
						}

						//Save for later.
						prev = it;
					}
				}//catch

		}//else
//		std::cout<<"in thread path size: "<<wp.size()<<std::endl;
		if(wp.empty())
		{
			// no path
//			if(excludeSeg)
//			{
//				Print()<<"workerThread: no path for nodes : no path for nodes and ex seg"<<fromNode->originalDB_ID.getLogItem()<<
//					toNode->originalDB_ID.getLogItem()<<
//					excludeSeg->originalDB_ID.getLogItem()<<std::endl;
//			}
//			else
//			{
//				Print()<<"workerThread: no path for nodes"<<fromNode->originalDB_ID.getLogItem()<<
//								toNode->originalDB_ID.getLogItem()<<std::endl;
//			}
	//			return s;
//			s = NULL;
			hasPath = false;
		}
		else
		{
			// make sp id
			std::string id = sim_mob::makeWaypointsetString(wp);
//				s = new SinglePath();

				// fill data
				s->pathSet = ps;
				s->isNeedSave2DB = true;
				hasPath = true;
				s->init(wp);
//				s->initWithWaypoints(wp);
		//		s->shortestWayPointpath = convertWaypoint2Point(wp);//stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*fromNode), stdir->DrivingVertex(*toNode),blacklist);
		//		s->shortestSegPath = sim_mob::generateSegPathByWaypointPathP(s->shortestWayPointpath);
				sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(s);
//				std::cout<<"in thread path sizeeeeeeeee: "<<wp.size()<<std::endl;
				s->highWayDistance = sim_mob::calculateHighWayDistance(s);
				s->fromNode = fromNode;
				s->toNode = toNode;
				s->excludeSeg = excludeSeg;


				s->pathset_id = ps->id;
				s->length = sim_mob::generateSinglePathLength(s->shortestWayPointpath);

				s->id = id;
				s->scenario = ps->scenario;
				s->pathsize=0;

				s->travel_cost = sim_mob::getTravelCost2(s);
				s->travle_time = s->pathSet->psMgr->getTravelTime(s);

	//			wp_spPool.insert(std::make_pair(id,s));
//				std::cout<<"in thread path size: "<<s->shortestWayPointpath.size()<<std::endl;
		}
}
//sim_mob::PublicTransportPathSetWorkerThread::PublicTransportPathSetWorkerThread()
//{
////	s = new sim_mob::SinglePath();
//	hasPath = false;
//}
//sim_mob::PublicTransportPathSetWorkerThread::~PublicTransportPathSetWorkerThread()
//{
////	if(s) delete s; s=NULL;
//}
//void sim_mob::PublicTransportPathSetWorkerThread::executeThis()
//{
//
//}
sim_mob::PathSetThreadPool::PathSetThreadPool()
{
        PathSetThreadPool(2);
}

sim_mob::PathSetThreadPool::PathSetThreadPool(int maxThreads)
{
   if (maxThreads < 1)  maxThreads=1;

   //mutexSync = PTHREAD_MUTEX_INITIALIZER;
   //mutexWorkCompletion = PTHREAD_MUTEX_INITIALIZER;

   pthread_mutex_lock(&mutexSync);
   this->maxThreads = maxThreads;
   this->queueSize = maxThreads;
   //workerQueue = new WorkerThread *[maxThreads];
   workerQueue.resize(maxThreads, NULL);
   topIndex = 0;
   bottomIndex = 0;
   incompleteWork = 0;
   sem_init(&availableWork, 0, 0);
   sem_init(&availableThreads, 0, queueSize);
   pthread_mutex_unlock(&mutexSync);
}

void sim_mob::PathSetThreadPool::initializeThreads()
{
   for(int i = 0; i<maxThreads; ++i)
        {
                pthread_t tempThread;
                pthread_create(&tempThread, NULL, &PathSetThreadPool::threadExecute, (void *) this );
                 //threadIdVec[i] = tempThread;
                // create worker
                PathSetWorkerThread *worker = new PathSetWorkerThread();
                workerPool.push_back(worker);
   }
}

sim_mob::PathSetThreadPool::~PathSetThreadPool()
{
   workerQueue.clear();
   for(int i=0;i<workerPool.size();++i)
   {
	   if(workerPool[i]) delete workerPool[i];
   }
   workerPool.clear();

   destroyPool(2);
}

sim_mob::PathSetWorkerThread* sim_mob::PathSetThreadPool::getWorker()
{
	sim_mob::PathSetWorkerThread* worker = NULL;
	if(workerIndex >= workerPool.size())
	{
		worker = new sim_mob::PathSetWorkerThread();
		workerPool.push_back(worker);
	}
	else
	{
		worker = workerPool[workerIndex];
	}
	workerIndex++;
	worker->hasPath = false;
	return worker;
}
void sim_mob::PathSetThreadPool::initWorkerIndex()
{
	workerIndex = 0;
}
void sim_mob::PathSetThreadPool::destroyPool(int maxPollSecs)
{
        while( incompleteWork>0 )
        {
                //cout << "Work is still incomplete=" << incompleteWork << endl;
                sleep(maxPollSecs);
        }
        cout << "All Done!! Wow! That was a lot of work!" << endl;
        sem_destroy(&availableWork);
        sem_destroy(&availableThreads);
        pthread_mutex_destroy(&mutexSync);
        pthread_mutex_destroy(&mutexWorkCompletion);

}


bool sim_mob::PathSetThreadPool::assignWork(PathSetWorkerThread *workerThread)
{
        pthread_mutex_lock(&mutexWorkCompletion);
                incompleteWork++;
                //cout << "assignWork...incomapleteWork=" << incompleteWork << endl;
        pthread_mutex_unlock(&mutexWorkCompletion);

        sem_wait(&availableThreads);

        pthread_mutex_lock(&mutexSync);
                //workerVec[topIndex] = workerThread;
                workerQueue[topIndex] = workerThread;
                //cout << "Assigning Worker[" << workerThread->id << "] Address:[" << workerThread << "] to Queue index [" << topIndex << "]" << endl;
                if(queueSize !=1 )
                        topIndex = (topIndex+1) % (queueSize-1);
                sem_post(&availableWork);
        pthread_mutex_unlock(&mutexSync);
        return true;
}

bool sim_mob::PathSetThreadPool::fetchWork(PathSetWorkerThread **workerArg)
{
        sem_wait(&availableWork);

        pthread_mutex_lock(&mutexSync);
        PathSetWorkerThread * workerThread = workerQueue[bottomIndex];
                workerQueue[bottomIndex] = NULL;
                *workerArg = workerThread;
                if(queueSize !=1 )
                        bottomIndex = (bottomIndex+1) % (queueSize-1);
                sem_post(&availableThreads);
        pthread_mutex_unlock(&mutexSync);
    return true;
}

void *sim_mob::PathSetThreadPool::threadExecute(void *param)
{
	PathSetWorkerThread *worker = NULL;

        while(((PathSetThreadPool *)param)->fetchWork(&worker))
        {
                if(worker)
                {
                        worker->executeThis();
                        //cout << "worker[" << worker->id << "]\tdelete address: [" << worker << "]" << endl;
//                        delete worker;
//                        worker = NULL;
                }

                pthread_mutex_lock( &(((PathSetThreadPool *)param)->mutexWorkCompletion) );
                //cout << "Thread " << pthread_self() << " has completed a Job !" << endl;
                ((PathSetThreadPool *)param)->incompleteWork--;
                pthread_mutex_unlock( &(((PathSetThreadPool *)param)->mutexWorkCompletion) );
        }
        return 0;
}
