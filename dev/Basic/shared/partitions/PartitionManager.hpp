/*
 * \file PartitionManager.hpp
 * Target:
 * 1. Support functions for main function
 *    [Map decomposition]
 *    [Boundary Agents Passing]
 *    [Communication]
 * 2. Support PM inner logic
 * 3. Just one interface between Main and PM-related service
 */

#pragma once

#include <vector>
#include <string>

#include "util/LangHelpers.hpp"
#include "BoundaryProcessor.hpp"
#include "metrics/Frame.hpp"

namespace sim_mob {

class SimulationScenario;
class BoundarySegment;
class PartitionConfigure;


/**
 * \author Xu Yan
 */
class PartitionManager
{
public:

	/**
	 *Only one object can be created
	 */
	static PartitionManager& instance() CHECK_MPI_THROW ;

	/**
	 *Init
	 *called in main()
	 */
	std::string startMPIEnvironment(int argc, char* argv[], bool config_adaptive_load_balance = false,
			bool config_measure_cost = false) CHECK_MPI_THROW ;

	/**
	 * Release MPI Resource
	 */
	std::string stopMPIEnvironment() CHECK_MPI_THROW ;

	/**
	 * Used when loading network;
	 * register boundary segments;
	 */
	void loadInBoundarySegment(std::string boundary_segment_id, BoundarySegment* boundary) CHECK_MPI_THROW ;

	/*
	 * Used to register items, like Signal near boundary;
	 */
	void initBoundaryTrafficItems() CHECK_MPI_THROW ;

	/*
	 * Called in the beginning of simulation
	 */
	void setEntityWorkGroup(WorkGroup* entity_group, WorkGroup* singal_group) CHECK_MPI_THROW ;

//	void updateRandomSeed() CHECK_MPI_THROW ;

	/**
	 * short-term and middle-term have different tools
	 */
	void setBoundaryProcessor(BoundaryProcessor* boundary_tool);

	/**
	 * Called for each time step
	 */
	std::string crossPCboundaryProcess(int time_step) CHECK_MPI_THROW ;
	std::string crossPCBarrier() CHECK_MPI_THROW ;
//	std::string outputAllEntities(uint32_t currTime) CHECK_MPI_THROW ;
	std::string adaptiveLoadBalance() CHECK_MPI_THROW ;

public:
	SimulationScenario* scenario;
	PartitionConfigure* partition_config;

private:
	//Temp use to count time step
	static int count;

private:
	static PartitionManager instance_;
	BoundaryProcessor* boundary_processor;
};

}


