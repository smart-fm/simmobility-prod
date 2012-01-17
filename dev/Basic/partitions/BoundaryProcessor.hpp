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

#include "GenConfig.h"
#ifndef SIMMOB_DISABLE_MPI

#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"

#include "BoundaryProcessingPackage.hpp"
#include "BoundarySegment.hpp"
#include "PartitionConfigure.hpp"
#include "SimulationScenario.hpp"

#include <map>
#include <vector>
#include <iostream>
#include <string>

namespace sim_mob {


/**
 * \author Xu Yan
 */
class BoundaryProcessor {
public:

	BoundaryProcessor()
	{
		entity_group = NULL;
		singal_group = NULL;
		scenario = NULL;
		partition_config = NULL;
	}
	/**
	 * initialization
	 */

	void setEntityWorkGroup(WorkGroup* entity_group, WorkGroup* singal_group);

	void loadInBoundarySegment(std::string boundary_segment_id, BoundarySegment* boundary);

	void setConfigure(PartitionConfigure* partition_config, SimulationScenario* scenario);

	void initBoundaryTrafficItems();

	/**
	 * Service
	 */
	std::string boundaryProcessing(int time_step);

	std::string outputAllEntities(frame_t time_step);

	std::string releaseMPIEnvironment();

private:

	std::set<const Entity*> boundaryRealTrafficItems;

	std::map<std::string, BoundarySegment*> boundary_segments;
//	std::map<std::string, std::vector<Agent*>*> fake_agents;

	sim_mob::WorkGroup* entity_group;
	sim_mob::WorkGroup* singal_group;

	PartitionConfigure* partition_config;
	SimulationScenario* scenario;

private:
	/**
	 * Step 1: set each fake agent to be "can be removed", before boundary processing
	 */
	void clearFakeAgentFlag();

	/**
	 * Step 2: Check agent
	 */
	std::string checkBoundaryAgents(BoundaryProcessingPackage& package);


	/**
	 * Step 3: Get Data
	 */
	std::string getDataInPackage(BoundaryProcessingPackage& package);
	void processPackageData(std::string data);

	/**
	 * Step 4: processing packages
	 */
	std::string processBoundaryPackages(
			std::vector<std::string>& all_packages);


private:
	/**
	 * location decision
	 */
	bool isAgentCrossed(BoundarySegment* boundary, Agent const* agent, bool is_down_boundary);
	bool isAgentInFeedbackorForward(BoundarySegment* boundary,
			Agent const* agent, bool is_down_boundary);

private:

	//process agents and Worker Group
	void changeAgentToFake(Agent * agent);
	void insertOneAgentToWorkerGroup(Agent * agent);
	void insertOneFakeAgentToWorkerGroup(Agent * agent);
	void removeOneFakeAgentFromWorkerGroup(Agent * agent);

private:
	//Others
	sim_mob::BoundarySegment* getBoundarySegmentByID(std::string segmentID);
	bool isAgentInLocalPartition(unsigned int agent_id, bool includeFakeAgent);
	bool isCrossAgentShouldBeInsert(const sim_mob::Agent* agent);
	Person* getFakePersonById(unsigned int agent_id);
	std::vector<Agent const *> agentsInSegmentBoundary(sim_mob::BoundarySegment* boundary_segment);

	void releaseFakeAgentMemory(sim_mob::Entity* agent);
};

}

#endif
