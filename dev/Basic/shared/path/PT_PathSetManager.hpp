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
using std::vector;
namespace sim_mob{

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
	void makePathset(sim_mob::Node* from,sim_mob::Node* to);
	PT_NetworkVertex getVertexFromNode(sim_mob::Node*);
	std::string getVertexIdFromNode(sim_mob::Node*);
	void getLabelingApproachPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet);
	void getkShortestPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet);
	void getLinkEliminationApproachPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet);
	void getSimulationApproachPaths(StreetDirectory::PT_VertexId fromId,StreetDirectory::PT_VertexId toId,PT_PathSet& ptPathSet);
	const int LabelPoolSize=10;
	const int simulationApproachPoolSize=10;
};

}
