/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once
#include <boost/unordered_set.hpp>
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "entities/Agent.hpp"

namespace sim_mob {

class LaneAgents {
public:
	LaneAgents() : queueCount(0) {}
	std::vector<sim_mob::Agent*> laneAgents;
	void addAgent(sim_mob::Agent* ag);
	void addAgents(std::vector<sim_mob::Agent*> agents, unsigned int numQueuing);
	void removeAgent(sim_mob::Agent* ag);
	sim_mob::Agent* dequeue();
	int getQueuingAgentsCount();
	int getMovingAgentsCount();

	void resetIterator();
	sim_mob::Agent* next();

	void initLaneStats(const sim_mob::Lane* lane);
	void updateOutputCounter(const sim_mob::Lane* lane);
	void updateOutputFlowRate(const sim_mob::Lane* lane, double newFlowRate);
	void updateAcceptRate(const sim_mob::Lane* lane);
	double getUpstreamSpeed(const Lane* lane);

	struct SupplyStats {
		double outputFlowRate;
		double origOutputFlowRate;
		int outputCounter;
		double acceptRate;
		double fraction;
	}supplyStats;

private:
	unsigned int queueCount;

	std::vector<sim_mob::Agent*>::iterator laneAgentsIt;
};

/**
 * Keeps a lane wise count of moving and queuing vehicles in a road segment.
 * Used by mid term supply
 */
class AgentKeeper {
	friend class sim_mob::LaneAgents;

private:
	const sim_mob::RoadSegment* roadSegment;
	std::map<const sim_mob::Lane*, sim_mob::LaneAgents* > laneAgentsMap;

	std::map<const sim_mob::Lane*, sim_mob::Agent* > frontalAgents;

	bool allAgentsProcessed();
	sim_mob::Agent* agentClosestToStopLine();

	bool downstreamCopy;
	std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > prevTickLaneCountsFromOriginal;

public:
	AgentKeeper(const sim_mob::RoadSegment* rdSeg, bool isDownstream = false);

	//TODO: in all functions which gets lane as a parameter, we must check if the lane belongs to the road segment.
	void addAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag);
	void absorbAgents(sim_mob::AgentKeeper* agKeeper);
	void removeAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag);
	sim_mob::Agent* dequeue(const sim_mob::Lane* lane);
	bool isFront(const sim_mob::Lane* lane, sim_mob::Agent* agent);
	std::vector<Agent*> getAgents(const sim_mob::Lane* lane);
	const sim_mob::RoadSegment* getRoadSegment() const;
	std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > getAgentCountsOnLanes();
	std::pair<unsigned int, unsigned int> getLaneAgentCounts(const sim_mob::Lane* lane); //returns std::pair<queuingCount, movingCount>
	unsigned int numAgentsInLane(const sim_mob::Lane* lane);
	unsigned int numMovingInSegment();
	unsigned int numQueueingInSegment();

	sim_mob::Agent* getNext();
	void resetFrontalAgents();
	LaneAgents::SupplyStats getSupplyStats(const Lane* lane);

	bool isDownstreamCopy() const {
		return downstreamCopy;
	}

	std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > getPrevTickLaneCountsFromOriginal() const;
	void setPrevTickLaneCountsFromOriginal(std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > prevTickLaneCountsFromOriginal);

	/**
	 * laneInfinity stores the agents which are added to this road segment (when they have just become active) and
	 * their lane and moving/queuing status is still unknown. The frame_init function of the agent's role will have to
	 * remove the agent from this priority queue and put them on moving/queuing vehicle lists on appropriate lane.
	 */
	sim_mob::StartTimePriorityQueue laneInfinity;
};

}
