/*
 * BoundaryProcessor.cpp
 *
 */

#include "BoundaryProcessor.hpp"

#include "conf/settings/DisableMPI.h"

#ifndef SIMMOB_DISABLE_MPI
#include "mpi.h"
#include <boost/mpi.hpp>

#include <limits>
#include "BoundaryProcessor.hpp"

#include "util/GeomHelpers.hpp"
#include "entities/AuraManager.hpp"
#include "logging/Log.hpp"
#include "workers/WorkGroup.hpp"

#include "partitions/PartitionManager.hpp"

namespace mpi = boost::mpi;

using std::string;
using std::vector;
using namespace sim_mob;


sim_mob::BoundaryProcessor::BoundaryProcessor(): BOUNDARY_PROCOSS_TAG(2)
{
	std::cout << "Do nothing" << std::endl;
//		downstream_ips.clear();
}


string sim_mob::BoundaryProcessor::boundaryProcessing(int time_step)
{
	std::cout << "Do nothing" << std::endl;
	return "";
}

void sim_mob::BoundaryProcessor::initBoundaryTrafficItems()
{
	std::cout << "Do nothing" << std::endl;
}

string sim_mob::BoundaryProcessor::releaseResources()
{
	std::cout << "Do nothing" << std::endl;
	return "";
}

void sim_mob::BoundaryProcessor::setEntityWorkGroup(WorkGroup* entity_group, WorkGroup* singal_group)
{
	std::cout << "Do nothing" << std::endl;
}

void sim_mob::BoundaryProcessor::loadInBoundarySegment(string id, BoundarySegment* boundary)
{
	std::cout << "Do nothing" << std::endl;
}

void sim_mob::BoundaryProcessor::setConfigure(PartitionConfigure* partition_config, SimulationScenario* scenario)
{
	std::cout << "Do nothing" << std::endl;
}


#endif
