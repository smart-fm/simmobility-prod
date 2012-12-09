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
	LaneStats() : queueCount(0), initialQueueCount(0), laneParams(new LaneParams()) {}
	std::vector<sim_mob::Agent*> laneAgents;

	/**
	 * laneAgentsCopy is a copy of laneAgents taken at the start of each tick solely for iterating the agents.
	 * laneAgentsIt will iterate on laneAgentsCopy and stays intact. Any handover of agents to the next segment
	 * is done by removing the agent from laneAgents and adding to laneAgents of the next segment. ~ Harish
	 */
	std::vector<sim_mob::Agent*> laneAgentsCopy;

	void addAgent(sim_mob::Agent* ag);
	void addAgents(std::vector<sim_mob::Agent*> agents, unsigned int numQueuing);
	void removeAgent(sim_mob::Agent* ag);
	void clear();
	sim_mob::Agent* dequeue();
	unsigned int getQueuingAgentsCount();
	unsigned int getMovingAgentsCount();

	void resetIterator();
	sim_mob::Agent* next();

	void initLaneParams(const sim_mob::Lane* lane, double vehSpeed, double pedSpeed);
	void updateOutputCounter(const sim_mob::Lane* lane);
	void updateOutputFlowRate(const sim_mob::Lane* lane, double newFlowRate);
	void updateAcceptRate(const sim_mob::Lane* lane, double upSpeed);

	unsigned int getInitialQueueCount() const {
		return initialQueueCount;
	}

	void setInitialQueueCount(unsigned int initialQueueCount) {
		this->initialQueueCount = initialQueueCount;
	}

	LaneParams* laneParams;

private:
	unsigned int queueCount;
	unsigned int initialQueueCount;

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

	bool downstreamCopy;
	std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > prevTickLaneCountsFromOriginal;

	double segVehicleSpeed; //speed of vehicles in segment for each frame
	double segPedSpeed; //speed of pedestrians on this segment for each frame--not used at the moment
	double segDensity;
	double lastAcceptTime;
public:
	SegmentStats(const sim_mob::RoadSegment* rdSeg, bool isDownstream = false);

	//TODO: in all functions which gets lane as a parameter, we must check if the lane belongs to the road segment.
	void addAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag);
	void absorbAgents(sim_mob::SegmentStats* segStats);
	void removeAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag);
	void clear();
	sim_mob::Agent* dequeue(const sim_mob::Lane* lane);
	bool isFront(const sim_mob::Lane* lane, sim_mob::Agent* agent);
	std::vector<Agent*> getAgents(const sim_mob::Lane* lane);
	const sim_mob::RoadSegment* getRoadSegment() const;
	std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > getAgentCountsOnLanes();
	std::pair<unsigned int, unsigned int> getLaneAgentCounts(const sim_mob::Lane* lane); //returns std::pair<queuingCount, movingCount>
	unsigned int numAgentsInLane(const sim_mob::Lane* lane);

	sim_mob::Agent* getNext();
	void resetFrontalAgents();

	bool isDownstreamCopy() const {
		return downstreamCopy;
	}

	bool hasAgents();

	std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > getPrevTickLaneCountsFromOriginal() const;
	void setPrevTickLaneCountsFromOriginal(std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > prevTickLaneCountsFromOriginal);

	unsigned int numMovingInSegment(bool hasVehicle);
	unsigned int numQueueingInSegment(bool hasVehicle);

	sim_mob::LaneParams* getLaneParams(const Lane* lane);
	double speed_density_function(bool hasVehicle, double segDensity);
	void restoreLaneParams(const Lane* lane);
	void updateLaneParams(const Lane* lane, double newOutputFlowRate);
	void updateLaneParams(timeslice frameNumber);
	void reportSegmentStats(timeslice frameNumber);
	double getSegSpeed(bool hasVehicle);
	double getDensity(bool hasVehicle);
	unsigned int getInitialQueueCount(const Lane* l);
	/**
	 * laneInfinity stores the new agents added to this road segment when they have just become active and their lane
	 * and moving/queuing status is still unknown. The frame_init function of the agent's role will have to remove
	 * the agent from this priority queue and put them on moving/queuing vehicle lists on appropriate lane.
	 */
	sim_mob::StartTimePriorityQueue laneInfinity;
};

}
