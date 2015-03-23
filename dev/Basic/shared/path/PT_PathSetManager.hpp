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
	vector<PT_NetworkEdge> makePathset(sim_mob::Node* from,sim_mob::Node* to);
	PT_NetworkVertex getVertexFromNode(sim_mob::Node*);
};

}
