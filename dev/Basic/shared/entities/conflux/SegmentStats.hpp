//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include "entities/Person.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"

namespace sim_mob {

/**
 * Data structure to store lane specific parameters for supply.
 *
 * \author Melani Jayasuriya
 */
class LaneParams {
	friend class LaneStats;
	friend class SegmentStats;

private:
	double outputFlowRate;
	double origOutputFlowRate;
	int outputCounter;
	double acceptRate;
	double fraction;
	double lastAcceptTime;

public:
	LaneParams() : outputFlowRate(0.0), origOutputFlowRate(0.0), outputCounter(0),
			acceptRate(0.0), fraction(0.0), lastAcceptTime(0.0){}

	double getOutputFlowRate() {return outputFlowRate;}
	int getOutputCounter() {return outputCounter;}
	double getAcceptRate() {return acceptRate;}

	void setOutputCounter(int count) { outputCounter = count; }
	void setOutputFlowRate(double output) {outputFlowRate = output;}
	void setOrigOutputFlowRate(double orig) {origOutputFlowRate = orig;}

	void setLastAccept(double lastAccept) {lastAcceptTime = lastAccept;}
	double getLastAccept() {return lastAcceptTime;}
};

/**
 * Data structure to store and maintain persons in a lane. Persons are maintained with relative ordering which
 * reflects their positions in the real lane during simulation. The queuing and moving counts of the lane is
 * also tracked by this class. Used by mid term supply.
 *
 * \author Harish Loganathan
 */
class LaneStats {

public:

	std::deque<sim_mob::Person*> laneAgents;

	LaneStats(const sim_mob::Lane* laneInSegment, bool isLaneInfinity = false) :
		queueCount(0), initialQueueCount(0), laneParams(new LaneParams()), positionOfLastUpdatedAgent(-1.0), lane(laneInSegment), laneInfinity(isLaneInfinity), debugMsgs(std::stringstream::out) {}
	~LaneStats() {
		safe_delete_item(laneParams);
	}

	void addPerson(sim_mob::Person* p);
	void updateQueueStatus(sim_mob::Person* p);
	void removePerson(sim_mob::Person* p, bool wasQueuing);
	void clear();
	sim_mob::Person* dequeue(bool isQueuingBfrUpdate);
	sim_mob::Person* dequeue(const sim_mob::Person* person, bool isQueuingBfrUpdate);
	unsigned int getQueuingAgentsCount();
	unsigned int getMovingAgentsCount();

	void resetIterator();
	sim_mob::Person* next();

	void initLaneParams(const sim_mob::Lane* lane, double vehSpeed, double pedSpeed);
	void updateOutputCounter(const sim_mob::Lane* lane);
	void updateOutputFlowRate(const sim_mob::Lane* lane, double newFlowRate);
	void updateAcceptRate(const sim_mob::Lane* lane, double upSpeed);

	// This function prints all agents in laneAgents
	void printAgents(bool copy = false);

	/*Verifies if the invariant that the order in laneAgents of each lane matches with the ordering w.r.t the distance to the end of segment*/
	void verifyOrdering();

	unsigned int getInitialQueueCount() const {
		return initialQueueCount;
	}

	void setInitialQueueCount(unsigned int initialQueueCount) {
		this->initialQueueCount = initialQueueCount;
	}

	double getPositionOfLastUpdatedAgent() const {
		return positionOfLastUpdatedAgent;
	}

	void setPositionOfLastUpdatedAgent(double positionOfLastUpdatedAgent) {
		this->positionOfLastUpdatedAgent = positionOfLastUpdatedAgent;
	}

	const sim_mob::Lane* getLane() const {
		return lane;
	}

	const bool isLaneInfinity() const {
		return laneInfinity;
	}

	LaneParams* laneParams;

	//TODO: To be removed after debugging.
	std::stringstream debugMsgs;

private:
	unsigned int queueCount;
	unsigned int initialQueueCount;
	double positionOfLastUpdatedAgent;
	const sim_mob::Lane* lane;
	const bool laneInfinity;

	/**
	 * laneAgentsCopy is a copy of laneAgents taken at the start of each tick solely for iterating the agents.
	 * laneAgentsIt will iterate on laneAgentsCopy and stays intact. Any handover of agents to the next segment
	 * is done by removing the agent from laneAgents and adding to laneAgents of the next segment. ~ Harish
	 */
	std::deque<sim_mob::Person*> laneAgentsCopy;
	std::deque<sim_mob::Person*>::iterator laneAgentsIt;
};

/**
 * Keeps a lane wise count of moving and queuing vehicles in a road segment. Keeps a map of LaneStats corresponding
 * to each lane in the road segment. Used by mid term supply.
 *
 * \author Harish Loganathan
 */
class SegmentStats {

private:
	const sim_mob::RoadSegment* roadSegment;
	std::map<const sim_mob::Lane*, sim_mob::LaneStats* > laneStatsMap;

	std::map<const sim_mob::Lane*, sim_mob::Person* > frontalAgents;

	bool downstreamCopy;
	std::map<const sim_mob::Lane*, std::pair<unsigned int, unsigned int> > prevTickLaneCountsFromOriginal;

	double segVehicleSpeed; //speed of vehicles in segment for each frame
	double segPedSpeed; //speed of pedestrians on this segment for each frame--not used at the moment
	double segDensity;
	double lastAcceptTime;
	int numVehicleLanes;
	unsigned int segFlow;

public:
	SegmentStats(const sim_mob::RoadSegment* rdSeg, bool isDownstream = false);
	~SegmentStats() {
		for(std::map<const sim_mob::Lane*, sim_mob::LaneStats* >::iterator i=laneStatsMap.begin(); i!=laneStatsMap.end(); i++) {
			safe_delete_item(i->second);
		}
		safe_delete_item(laneInfinity);
	}

	enum VehicleType { car, bus, none };
	//TODO: in all functions which gets lane as a parameter, we must check if the lane belongs to the road segment.
	void addAgent(const sim_mob::Lane* lane, sim_mob::Person* p);
	void removeAgent(const sim_mob::Lane* lane, sim_mob::Person* ag, bool wasQueuing);
	sim_mob::Person* dequeue(const sim_mob::Lane* lane, bool isQueuingBfrUpdate);
	sim_mob::Person* dequeue(const sim_mob::Person* person, const sim_mob::Lane* lane, bool isQueuingBfrUpdate);
	bool isFront(const sim_mob::Lane* lane, sim_mob::Person* person);
	std::deque<Person*> getAgents(const sim_mob::Lane* lane);
	std::deque<Person*> getAgents();
	const sim_mob::RoadSegment* getRoadSegment() const;
	std::map<const sim_mob::Lane*, std::pair<unsigned int, unsigned int> > getAgentCountsOnLanes();
	std::pair<unsigned int, unsigned int> getLaneAgentCounts(const sim_mob::Lane* lane); //returns std::pair<queuingCount, movingCount>
	unsigned int numAgentsInLane(const sim_mob::Lane* lane);
	void updateQueueStatus(const sim_mob::Lane* lane, sim_mob::Person* p);

	void resetFrontalAgents();
	sim_mob::Person* agentClosestToStopLineFromFrontalAgents();

	bool hasAgents();
	bool canAccommodate(SegmentStats::VehicleType type);

	unsigned int numMovingInSegment(bool hasVehicle);
	unsigned int numQueueingInSegment(bool hasVehicle);

	double getPositionOfLastUpdatedAgentInLane(const Lane* lane);
	void setPositionOfLastUpdatedAgentInLane(double positionOfLastUpdatedAgentInLane, const Lane* lane);
	void resetPositionOfLastUpdatedAgentOnLanes();

	sim_mob::LaneParams* getLaneParams(const Lane* lane);
	double speed_density_function(bool hasVehicle, double segDensity);
	void restoreLaneParams(const Lane* lane);
	void updateLaneParams(const Lane* lane, double newOutputFlowRate);
	void updateLaneParams(timeslice frameNumber);

	std::string reportSegmentStats(timeslice frameNumber);

	double getSegSpeed(bool hasVehicle);
	double getDensity(bool hasVehicle);
	unsigned int getSegFlow();
	void incrementSegFlow();
	void resetSegFlow();
	unsigned int getInitialQueueCount(const Lane* l);
	unsigned int computeExpectedOutputPerTick();

	// This function prints all agents in this segment
	void printAgents();
	/**
	 * laneInfinity is an augmented lane in the roadSegment. laneInfinity will be used only by confluxes and related objects for now.
	 * The LaneStats object created for laneInfinity stores the new persons who will start at this roadSegment. A Person will be
	 * added to laneInfinity (LaneStats corresponding to laneInfinity) when his start time falls within the current tick. The actual lane
	 * and moving/queuing status is still unknown for persons in laneInfinity. The frame_init function of the agent's role will have
	 * to put the persons from laneInfinity on moving/queuing vehicle lists on appropriate real lane.
	 *
	 * Persons who are performing an activity are also stashed in laneInfinity. TODO: Double check this
	 */
	const sim_mob::Lane* laneInfinity;

	std::stringstream debugMsgs; // handy to throw meaningful error messages
};

}
