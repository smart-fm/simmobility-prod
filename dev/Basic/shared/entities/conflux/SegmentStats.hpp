/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <string>
#include <boost/unordered_set.hpp>
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "entities/Agent.hpp"

namespace sim_mob {

class LaneParams {
	friend class LaneStats;
	friend class SegmentStats;

private:
	double outputFlowRate;
	double origOutputFlowRate;
	int outputCounter;
	double acceptRate;
	double fraction;

public:
	LaneParams() : outputFlowRate(0.0), origOutputFlowRate(0.0), outputCounter(0),
			acceptRate(0.0), fraction(0.0){}

	double getOutputFlowRate() {return outputFlowRate;}
	int getOutputCounter() {return outputCounter;}
	double getAcceptRate() {return acceptRate;}

	void setOutputFlowRate(double output) {outputFlowRate = output;}
	void setOrigOutputFlowRate(double orig) {origOutputFlowRate = orig;}
};

class LaneStats {

public:
	LaneStats() : queueCount(0), laneParams(new LaneParams()) {}
	std::vector<sim_mob::Agent*> laneAgents;
	void addAgent(sim_mob::Agent* ag);
	void removeAgent(sim_mob::Agent* ag);
	sim_mob::Agent* dequeue();
	int getQueuingAgentsCount();
	int getMovingAgentsCount();

	void resetIterator();
	sim_mob::Agent* next();

	void initLaneParams(const sim_mob::Lane* lane, double upSpeed);
	void updateOutputCounter(const sim_mob::Lane* lane);
	void updateOutputFlowRate(const sim_mob::Lane* lane, double newFlowRate);
	void updateAcceptRate(const sim_mob::Lane* lane, double upSpeed);
	LaneParams* laneParams;

private:
	unsigned int queueCount;

	std::vector<sim_mob::Agent*>::iterator laneAgentsIt;
};

/**
 * Keeps a lane wise count of moving and queuing vehicles in a road segment.
 * Used by mid term supply
 */
class SegmentStats {

private:
	const sim_mob::RoadSegment* roadSegment;
	std::map<const sim_mob::Lane*, sim_mob::LaneStats* > laneStatsMap;

	std::map<const sim_mob::Lane*, sim_mob::Agent* > frontalAgents;

	bool allAgentsProcessed();
	sim_mob::Agent* agentClosestToStopLine();

public:
	SegmentStats(const sim_mob::RoadSegment* rdSeg);

	//TODO: in all functions which gets lane as a parameter, we must check if the lane belongs to the road segment.
	void addAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag);
	void removeAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag);
	sim_mob::Agent* dequeue(const sim_mob::Lane* lane);
	bool isFront(const sim_mob::Lane* lane, sim_mob::Agent* agent);
	std::vector<Agent*> getAgents(const sim_mob::Lane* lane);
	const sim_mob::RoadSegment* getRoadSegment() const;
	std::map<sim_mob::Lane*, std::pair<int, int> > getAgentCountsOnLanes();
	std::pair<int, int> getLaneAgentCounts(const sim_mob::Lane* lane); //returns std::pair<queuingCount, movingCount>
	unsigned int numMovingInSegment(bool hasVehicle);
	unsigned int numQueueingInSegment(bool hasVehicle);

	sim_mob::Agent* getNext();
	void resetFrontalAgents();
	sim_mob::LaneParams* getLaneParams(const Lane* lane);
	double speed_density_function(bool hasVehicle);
	void restoreLaneParams(const Lane* lane);
	void updateLaneParams(const Lane* lane, double newOutputFlowRate);
	// TODO: Check if this function is really required
	//void merge(sim_mob::AgentKeeper*);

	/**
	 * laneInfinity stores the agents which are added to this road segment (when they have just become active) and
	 * their lane and moving/queuing status is still unknown. The frame_init function of the agent's role will have to
	 * remove the agent from this priority queue and put them on moving/queuing vehicle lists on appropriate lane.
	 */
	sim_mob::StartTimePriorityQueue laneInfinity;
};

}
