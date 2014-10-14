//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Node.hpp"

#include "conf/settings/DisableMPI.h"
#include "logging/Log.hpp"
#include "util/Utils.hpp"

unsigned int sim_mob::Node::getID()const {return nodeId;}
void sim_mob::Node::setID(unsigned int id) { nodeId = id; }
unsigned int sim_mob::Node::getAimsunId() const
	{
		unsigned int originId = 0;

		std::string aimsunId = originalDB_ID.getLogItem();
		std::string id = sim_mob::Utils::getNumberFromAimsunId(aimsunId);
		try {
			originId = boost::lexical_cast<int>(id);
		} catch( boost::bad_lexical_cast const& ) {
			Warn() << "Error: aimsun id string was not valid" << std::endl;
		}

		return originId;
	}
//sim_mob::Link* sim_mob::Node::getLinkLoc() const {return linkLoc;}
//void sim_mob::Node::setLinkLoc(sim_mob::Link* link) { linkLoc=link; }

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"

namespace sim_mob{


}
#endif
