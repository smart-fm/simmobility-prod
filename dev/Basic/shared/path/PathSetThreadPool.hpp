#pragma once

#include "path/Path.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"

using namespace std;

namespace sim_mob
{
/**
 * class responsible for generating paths based on configuration provided
 * \author Weng Zhiyong
 */
class PathSetWorkerThread
{
public:
	virtual void run();
	PathSetWorkerThread();
	virtual ~PathSetWorkerThread();

	StreetDirectory::Graph* graph;
	StreetDirectory::Vertex fromVertex;
	StreetDirectory::Vertex toVertex;
	const sim_mob::Node *fromNode;
	const sim_mob::Node *toNode;
	std::set<const Link*> excludedLinks;
	std::map<const Link*, std::set<StreetDirectory::Edge> > *linkLookup;
	SinglePath *path;
	boost::shared_ptr<sim_mob::PathSet> pathSet;
	bool hasPath;
	bool timeBased;
	///used by local profilers to report to the profiler in higher level.
	std::string dbgStr;
};

}
