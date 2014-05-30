/*
 * PathSetThreadPool.h
 *
 *  Created on: Feb 18, 2014
 *      Author: redheli
 */

#ifndef PATHSETTHREADPOOL_H_
#define PATHSETTHREADPOOL_H_

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

#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <vector>
#include "geospatial/PathSetManager.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
//#include "geospatial/streetdir/PublicTransportStreetDirectory.h"


using namespace std;
namespace sim_mob
{
/*
WorkerThread class
This class needs to be sobclassed by the user.
*/
class PathSetWorkerThread{
public:
//    int id;

	virtual void executeThis();
    PathSetWorkerThread();
    virtual ~PathSetWorkerThread();

public:
	StreetDirectory::Graph* graph;
	StreetDirectory::Vertex* fromVertex;
	StreetDirectory::Vertex* toVertex;
	const sim_mob::Node *fromNode;
	const sim_mob::Node *toNode;
//	std::vector<const RoadSegment*> *blacklist;
	const RoadSegment* excludeSeg;
	std::map<const RoadSegment*, std::set<StreetDirectory::Edge> > *segmentLookup;
//	std::vector<WayPoint> *result;
	SinglePath *s;
	PathSet *ps;
	bool hasPath;
	///used by local profilers to report to the profiler in higher level.
	sim_mob::Profiler *parentProfiler;
};

//class PublicTransportPathSetWorkerThread : public PathSetWorkerThread{
//public:
//	void executeThis();
//	PublicTransportPathSetWorkerThread();
//	virtual ~PublicTransportPathSetWorkerThread();
//
//public:
//	PublicTransportStreetDirectory* sdir;
//	PublicTransportStreetDirectory::publicTransportGraph* g;
////	PublicTransportStreetDirectory::publicTransportGraph* ptGraph;
////	PublicTransportStreetDirectory::publicTransportGraphVertex* ptGraphVertx;
////	PublicTransportStreetDirectory::publicTransportGraphEdge* ptGraphEdge;
//};

}
#endif /* PATHSETTHREADPOOL_H_ */
