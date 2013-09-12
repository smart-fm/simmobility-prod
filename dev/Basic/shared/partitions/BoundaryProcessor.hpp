//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * BoundaryProcessor.hpp
 * Target:
 * Implement 4 steps of boundary processing
 * Step 1:
 * Remove flag of fake agents, so that if the fake agent is not re-received, then can remove.
 * Step 2:
 * Check agent locations
 * Step 3:
 * Serialization and Transmission
 * Step 4:
 * Processing received package
 */

#pragma once

#include "conf/settings/DisableMPI.h"

#include "util/LangHelpers.hpp"
#include "PartitionConfigure.hpp"
#include "SimulationScenario.hpp"
#include "metrics/Frame.hpp"

#include "BoundarySegment.hpp"

#include <map>
#include <vector>
#include <iostream>
#include <string>

namespace sim_mob {

class WorkGroup;
class Entity;
class Agent;
class Person;

/**
 * \author Xu Yan
 */
class BoundaryProcessor {
public:

	explicit BoundaryProcessor();
	virtual ~BoundaryProcessor(){}

	/**
	 * initialization
	 */
	virtual void setConfigure(PartitionConfigure* partition_config, SimulationScenario* scenario) CHECK_MPI_THROW ;


	/**
	 * initialization
	 */
	virtual void setEntityWorkGroup(WorkGroup* entity_group, WorkGroup* singal_group) CHECK_MPI_THROW ;

	/**
	 *
	 */
	virtual void loadInBoundarySegment(std::string boundary_segment_id, BoundarySegment* boundary) CHECK_MPI_THROW ;

	/**
	 *
	 */
	virtual void initBoundaryTrafficItems() CHECK_MPI_THROW ;

	/**
	 * Service
	 */
	virtual std::string boundaryProcessing(int time_step) CHECK_MPI_THROW ;

//	std::string outputAllEntities(timeslice now) CHECK_MPI_THROW ;

	virtual std::string releaseResources() CHECK_MPI_THROW ;

private:

	//the Tag for MPI Messages
	const int BOUNDARY_PROCOSS_TAG;

};

}

