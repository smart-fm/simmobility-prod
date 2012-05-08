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

	BoundaryProcessor();

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

	//one item might need to be sent to many other partitions
	std::set<const Entity*> boundaryRealTrafficItems;
	std::map<int, int > traffic_items_mapping_to;

	std::map<std::string, BoundarySegment*> boundary_segments;
//	std::map<std::string, std::vector<Agent*>*> fake_agents;

	WorkGroup* entity_group;
	WorkGroup* singal_group;

	PartitionConfigure* partition_config;
	SimulationScenario* scenario;

	std::set<int> neighbor_ips;
//	std::set<int> upstream_ips;
//	std::set<int> downstream_ips;

	//the Tag for MPI Messages
	const int BOUNDARY_PROCOSS_TAG;

private:
	/**
	 * Step 1: set each fake agent to be "can be removed", before boundary processing
	 */
	void clearFakeAgentFlag();

	/**
	 * Step 2: Check agent
	 */
	std::string checkBoundaryAgents(BoundaryProcessingPackage* downstream_packs);


	/**
	 * Step 3: Get Data
	 */
	std::string getDataInPackage(BoundaryProcessingPackage& package);
	void processPackageData(std::string data);

	/**
	 * Step 4: processing packages
	 */
	std::string processBoundaryPackages(std::string all_packages[], int size);


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
	BoundarySegment* getBoundarySegmentByID(std::string segmentID);
	bool isAgentInLocalPartition(unsigned int agent_id, bool includeFakeAgent);
	bool isCrossAgentShouldBeInsert(const Agent* agent);
	Person* getFakePersonById(unsigned int agent_id);
	std::vector<Agent const *> agentsInSegmentBoundary(BoundarySegment* boundary_segment);

	void releaseFakeAgentMemory(Entity* agent);

	//This enum is only used internally, so declare it as a private class member.
	enum Agent_Type {
		NO_TYPE = 0,
		DRIVER_TYPE,
		PEDESTRIAN_TYPE,
		PASSENGER_TYPE,
		SIGNAL_TYPE
	};

	Agent_Type getAgentTypeForSerialization(Agent const* agent);
};

}

