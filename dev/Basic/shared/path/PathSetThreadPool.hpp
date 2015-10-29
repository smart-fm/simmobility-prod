/*
 * PathSetThreadPool.hpp
 *
 *  Created on: Feb 18, 2014
 *      Author: redheli
 */

#pragma once
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <vector>
#include "path/PathSetManager.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"

using namespace std;

namespace sim_mob
{
/// main class responsible for generating paths based on the input configuration
class PathSetWorkerThread{
public:
	virtual void run();
    PathSetWorkerThread();
    virtual ~PathSetWorkerThread();

public:
	StreetDirectory::Graph* graph;
	StreetDirectory::Vertex fromVertex;
	StreetDirectory::Vertex toVertex;
	const sim_mob::Node *fromNode;
	const sim_mob::Node *toNode;
	std::set<const Link*> excludeSeg;
	std::map<const Link*, std::set<StreetDirectory::Edge> > *linkLookup;
	SinglePath *path;
	boost::shared_ptr<sim_mob::PathSet> pathSet;
	bool hasPath;
	bool timeBased;
	///used by local profilers to report to the profiler in higher level.
	std::string dbgStr;
};

}
