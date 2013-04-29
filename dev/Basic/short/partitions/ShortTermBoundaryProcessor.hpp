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

#include "BoundaryProcessingPackage.hpp"
#include "partitions/BoundarySegment.hpp"
#include "partitions/BoundaryProcessor.hpp"
#include "partitions/PartitionConfigure.hpp"
#include "partitions/SimulationScenario.hpp"

#include "metrics/Frame.hpp"

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
class ShortTermBoundaryProcessor: public sim_mob::BoundaryProcessor
{
public:

	ShortTermBoundaryProcessor();
	~ShortTermBoundaryProcessor()
	{
	}

	virtual void setConfigure(PartitionConfigure* partition_config, SimulationScenario* scenario) CHECK_MPI_THROW ;

	virtual void setEntityWorkGroup(WorkGroup* entity_group, WorkGroup* singal_group) CHECK_MPI_THROW ;

	/**
	 * initialization
	 */
	virtual void loadInBoundarySegment(std::string boundary_segment_id, BoundarySegment* boundary) CHECK_MPI_THROW;

	/**
	 *
	 */
	virtual void initBoundaryTrafficItems() CHECK_MPI_THROW;

	/**
	 * Service
	 */
	virtual std::string boundaryProcessing(int time_step) CHECK_MPI_THROW;

	//	std::string outputAllEntities(timeslice now) CHECK_MPI_THROW ;

	virtual std::string releaseResources() CHECK_MPI_THROW;

private:

	//one item might need to be sent to many other partitions
	std::set<const Entity*> boundaryRealTrafficItems;
	std::map<int, int> traffic_items_mapping_to;

	std::map<std::string, BoundarySegment*> boundary_segments;
//	std::map<std::string, std::vector<Agent*>*> fake_agents;

	WorkGroup* entity_group;
	WorkGroup* singal_group;

	PartitionConfigure* partition_config;
	SimulationScenario* scenario;

	std::set<int> neighbor_ips;
//	std::set<int> upstream_ips;
//	std::set<int> downstream_ips;

private:
	/**
	 * Step 1: set each fake agent to be "can be removed", before boundary processing
	 */
	void clearFakeAgentFlag() CHECK_MPI_THROW;

	/**
	 * Step 2: Check agent
	 */
	std::string checkBoundaryAgents(BoundaryProcessingPackage* downstream_packs) CHECK_MPI_THROW;

	/**
	 * Step 3: Get Data
	 */
	std::string getDataInPackage(BoundaryProcessingPackage& package) CHECK_MPI_THROW;
	void processPackageData(std::string data) CHECK_MPI_THROW;

	/**
	 * Step 4: processing packages
	 */
	std::string processBoundaryPackages(std::string all_packages[], int size) CHECK_MPI_THROW;

private:
	/**
	 * location decision
	 */
	bool isAgentCrossed(BoundarySegment* boundary, Agent const* agent, bool is_down_boundary) CHECK_MPI_THROW;
	bool isAgentInFeedbackorForward(BoundarySegment* boundary, Agent const* agent, bool is_down_boundary) CHECK_MPI_THROW;

private:

	//process agents and Worker Group
	void changeAgentToFake(Agent * agent) CHECK_MPI_THROW;
	void insertOneAgentToWorkerGroup(Agent * agent) CHECK_MPI_THROW;
	void insertOneFakeAgentToWorkerGroup(Agent * agent) CHECK_MPI_THROW;
	void removeOneFakeAgentFromWorkerGroup(Agent * agent) CHECK_MPI_THROW;

private:
	//Others
	BoundarySegment* getBoundarySegmentByID(std::string segmentID) CHECK_MPI_THROW;
	bool isAgentInLocalPartition(unsigned int agent_id, bool includeFakeAgent) CHECK_MPI_THROW;
	bool isCrossAgentShouldBeInsert(const Agent* agent) CHECK_MPI_THROW;
	Person* getFakePersonById(unsigned int agent_id) CHECK_MPI_THROW;
	std::vector<Agent const *> agentsInSegmentBoundary(BoundarySegment* boundary_segment) CHECK_MPI_THROW;

	void releaseFakeAgentMemory(Entity* agent) CHECK_MPI_THROW;

	//This enum is only used internally, so declare it as a private class member.
	enum Agent_Type
	{
		NO_TYPE = 0, DRIVER_TYPE, PEDESTRIAN_TYPE, PASSENGER_TYPE, SIGNAL_TYPE
	};

	Agent_Type getAgentTypeForSerialization(Agent const* agent) CHECK_MPI_THROW;
};

}

