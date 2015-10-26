//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "entities/params/PT_NetworkEntities.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "path/Path.hpp"
#include "util/threadpool/Threadpool.hpp"
#include <fstream>

using std::vector;
namespace sim_mob{
/*
 * This class is used for public path set
 */
class PT_PathSetManager {
public:
	PT_PathSetManager();
	virtual ~PT_PathSetManager();
	/**
	 * get single instance of public path set manager
	 */
	static PT_PathSetManager& Instance();
	/**
	 * generate all path set for all node pairs
	 */
	void BulkPathSetGenerator();

private:
	/**
	 * make public path set between two nodes
	 * @param from is original node
	 * @param to is destination node
	 * @return path set between two nodes
	 */
	PT_PathSet makePathset(const sim_mob::Node* from, const sim_mob::Node* to);
	/**
	 * get corresponding vertex id from the node
	 * @param node is a pointer to a node object
	 * @return vertex id in the graph
	 */
	std::string getVertexIdFromNode(const sim_mob::Node* node);
	/**
	 * get path set in the way of labeling approach
	 * @param fromId is original Id in the public transit graph
	 * @param toId is destination Id in the public transit graph
	 * @param ptPathSet hold the result of path set
	 */
	void getLabelingApproachPaths(const StreetDirectory::PT_VertexId& fromId, const StreetDirectory::PT_VertexId& toId, PT_PathSet& ptPathSet);
	/**
	 * get path set in the way of k shortest path
	 * @param fromId is original Id in the public transit graph
	 * @param toId is destination Id in the public transit graph
	 * @param ptPathSet hold the result of path set
	 */
	void getK_ShortestPaths(const StreetDirectory::PT_VertexId& fromId,	const StreetDirectory::PT_VertexId& toId, PT_PathSet& ptPathSet);
	/**
	 * get path set in the way of link elimination approach
	 * @param fromId is original Id in the public transit graph
	 * @param toId is destination Id in the public transit graph
	 * @param ptPathSet hold the result of path set
	 */
	void getLinkEliminationApproachPaths(const StreetDirectory::PT_VertexId& fromId,const StreetDirectory::PT_VertexId& toId, PT_PathSet& ptPathSet);
	/**
	 * get path set in the way of simulation approach
	 * @param fromId is original Id in the public transit graph
	 * @param toId is destination Id in the public transit graph
	 * @param ptPathSet hold the result of path set
	 */
	void getSimulationApproachPaths(const StreetDirectory::PT_VertexId& fromId,	const StreetDirectory::PT_VertexId& toId, PT_PathSet& ptPathSet);
	/**
	 * write out path set result
	 * @param ptPathSet hold the result of path set
	 * @param fromNodeId is original node Id
	 * @param toNodeId is destination node Id
	 */
	void writePathSetToFile(const PT_PathSet &ptPathSet,unsigned int fromNodeId, unsigned int toNodeId);
	/**
	 * write head information to path set result
	 */
	void writePathSetFileHeader();

private:
	/**
	 * store the instance of path set manager
	 */
	static PT_PathSetManager instance;
	/**
	 * the size of thread pool to handle labeling approach
	 */
	const int labelPoolSize;
	/**
	 * the thread pool to handle path set generation
	 */
	static boost::shared_ptr<sim_mob::batched::ThreadPool> threadpool;
	/**
	 * the locker for file writing
	 */
	boost::mutex fileExclusiveWrite;
	/**
	 * the out stream for result writing
	 */
	std::ofstream ptPathSetWriter;
};

}
