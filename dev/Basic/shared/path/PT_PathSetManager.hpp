//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)
/*
 * PT_PathSetManager.hpp
 *
 *  Created on: Mar 18, 2015
 *      Author: Prabhuraj
 */

#include "entities/params/PT_NetworkEntities.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "path/Path.hpp"
#include "util/threadpool/Threadpool.hpp"
#include <fstream>
using std::vector;
namespace sim_mob{
/*
 * This class is used for
 */
class PT_PathSetManager {
public:
	PT_PathSetManager();
	virtual ~PT_PathSetManager();
	static PT_PathSetManager _instance;

	static PT_PathSetManager& Instance()
	{
		return _instance;
	}
public:
	static boost::shared_ptr<sim_mob::batched::ThreadPool> threadpool_;
	boost::mutex fileExclusiveWrite;
	PT_PathSet makePathset(sim_mob::Node* from,sim_mob::Node* to);
	std::string getVertexIdFromNode(sim_mob::Node*);
	void getLabelingApproachPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet);
	void getkShortestPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet);
	void getLinkEliminationApproachPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet);
	void getSimulationApproachPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet);
	void writePathSetToFile(PT_PathSet &ptPathSet,unsigned int fromNodeId, unsigned int toNodeId);
	void PT_BulkPathSetGenerator();
	void writePathSetFileHeader();
	const int labelPoolSize;
	std::ofstream ptPathSetWriter;
};
struct PT_OD{
private:
	int startNode;
	int destNode;
public:
	PT_OD(int start,int dest)
	{
		startNode=start;
		destNode=dest;
	}
	int getStartNode() const{
		return startNode;
	}
	void setStartNode(int node){
		startNode=node;
	}
	int getDestNode() const{
		return destNode;
	}
	void setDestNode(int node){
		destNode=node;
	}
};
struct compare_OD: public std::less<PT_OD>
{
		bool operator() (const PT_OD& start, const PT_OD& dest) const;
};

}
