/* Copyright Singapore-MIT Alliance for Research and Technology */

//TODO: Add comments to each of the methods ~ Harish

#pragma once
#include <boost/unordered_set.hpp>
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "entities/Agent.hpp"

namespace sim_mob {

/**
 * Keeps a lane wise count of moving and queuing vehicles in a road segment.
 * Used by mid term supply
 */
class AgentKeeper {

private:
	const sim_mob::RoadSegment* roadSegment;
	std::map<const sim_mob::Lane*, std::vector<sim_mob::Agent*> > queuingAgents;
	std::map<const sim_mob::Lane*, std::set<sim_mob::Agent*> > movingAgents;

public:
	AgentKeeper(const sim_mob::RoadSegment* rdSeg);
	void addQueuingAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag);
	void removeQueuingAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag);
	sim_mob::Agent* dequeue(const sim_mob::Lane* lane);
	bool isFront(const sim_mob::Lane* lane, sim_mob::Agent* agent);
	void addMovingAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag);
	void removeMovingAgent(const sim_mob::Lane* lane, sim_mob::Agent*);
	std::set<Agent*> getAgentsOnMovingVehicles(const sim_mob::Lane* lane);
	std::vector<Agent*> getAgentsOnQueuingVehicles(const sim_mob::Lane* lane);
	const sim_mob::RoadSegment* getRoadSegment() const;
	void merge(sim_mob::AgentKeeper*);

	/**
	 * laneInfinity stores the agents which are added to this road segment (when they have just become active) and
	 * their lane and moving/queuing status is still unknown. The frame_init function of the agent's role will have to
	 * remove the agent from this priority queue and put them on moving/queuing vehicle lists on appropriate lane.
	 */
	sim_mob::StartTimePriorityQueue laneInfinity;
};

}
