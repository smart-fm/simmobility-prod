/*
 * PartitionManager.hpp
 * Target:
 * 1. Support functions for main function
 *    [Map decomposition]
 *    [Dynamic Load Balance]
 *    [Boundary Agents Passing]
 *    [Communication]
 * 2. Support PM inner logic
 * 3. Just one interface between Main and PM-related service
 */

#pragma once

#include "GenConfig.h"
#ifndef SIMMOB_DISABLE_MPI



#include <vector>
#include <string>

#include "PartitionConfigure.hpp"
#include "SimulationScenario.hpp"
#include "BoundaryProcessor.hpp"
#include "metrics/Frame.hpp"

namespace sim_mob {

class SimulationScenario;
class BoundarySegment;
class SimulationScenario;
class PartitionConfigure;
class BoundaryProcessor;

class PartitionManager
{

public:

	/**
	 *Only one object can be created
	 */
	static PartitionManager &
	instance()
	{
		return instance_;
	}

	/**
	 *  If the No. of computers > 1: start MPI Environment;
	 *  If the No. of computers == 1: not start MPI Environment;
	 */
	std::string startMPIEnvironment(int argc, char* argv[], bool config_adaptive_load_balance = false,
			bool config_measure_cost = false);

	std::string stopMPIEnvironment();

	void initBoundaryTrafficItems();

	/*
	 * Called in the beginning of simulation
	 */
	void setEntityWorkGroup(WorkGroup* entity_group, WorkGroup* singal_group);
	void loadInBoundarySegment(std::string boundary_segment_id, BoundarySegment* boundary);
	void updateRandomSeed();

	/**
	 * Called for each time step
	 */
	std::string crossPCboundaryProcess(int time_step);
	std::string crossPCBarrier();
	std::string outputAllEntities(frame_t time_step);
	std::string adaptiveLoadBalance();

public:
	SimulationScenario* scenario;
	PartitionConfigure* partition_config;

	/**
	 * Temp use to count time step
	 */
	static int count;

private:
	static PartitionManager instance_;
	BoundaryProcessor processor;

};
}

#endif
