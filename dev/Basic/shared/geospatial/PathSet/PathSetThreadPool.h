/*
 * PathSetThreadPool.h
 *
 *  Created on: Feb 18, 2014
 *      Author: redheli
 */

#pragma once
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <vector>
#include "geospatial/PathSetManager.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"

using namespace std;

namespace sim_mob
{
/**
WorkerThread class
This class needs to be sobclassed by the user.
*/
class PathSetWorkerThread{
public:
	virtual void executeThis();
    PathSetWorkerThread();
    virtual ~PathSetWorkerThread();

public:
	StreetDirectory::Graph* graph;
	StreetDirectory::Vertex* fromVertex;
	StreetDirectory::Vertex* toVertex;
	const sim_mob::Node *fromNode;
	const sim_mob::Node *toNode;
	const RoadSegment* excludeSeg;
	std::map<const RoadSegment*, std::set<StreetDirectory::Edge> > *segmentLookup;
	SinglePath *s;
	PathSet *ps;
	bool hasPath;
	///used by local profilers to report to the profiler in higher level.
	std::string dbgStr;
};

}
