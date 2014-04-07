//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include "entities/Person.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"

namespace sim_mob {

struct cmp_person_distToSegmentEnd : public std::greater<Person*> {
	bool operator() (const Person* x, const Person* y) const;
};

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
private:
	typedef std::deque<sim_mob::Person*> PersonList;
	unsigned int queueCount;
	unsigned int initialQueueCount;
	double positionOfLastUpdatedAgent;
	const sim_mob::Lane* lane;
	const bool laneInfinity;
	double length;
	unsigned int numPersons;

	/**
	 * laneAgentsCopy is a copy of laneAgents taken at the start of each tick solely for iterating the agents.
	 * laneAgentsIt will iterate on laneAgentsCopy and stays intact. Any handover of agents to the next segment
	 * is done by removing the agent from laneAgents and adding to laneAgents of the next segment. ~ Harish
	 */
	PersonList laneAgentsCopy;
	PersonList::iterator laneAgentsIt;

public:

	PersonList laneAgents;

	LaneStats(const sim_mob::Lane* laneInSegment, double length, bool isLaneInfinity = false) :
		queueCount(0), initialQueueCount(0), laneParams(new LaneParams()),
		positionOfLastUpdatedAgent(-1.0), lane(laneInSegment), length(length),
		laneInfinity(isLaneInfinity), numPersons(0) {}
	~LaneStats() {
		safe_delete_item(laneParams);
	}

	void addPerson(sim_mob::Person* p);
	void updateQueueStatus(sim_mob::Person* p);
	void removePerson(sim_mob::Person* p, bool wasQueuing);
	sim_mob::Person* dequeue(bool isQueuingBfrUpdate);
	sim_mob::Person* dequeue(const sim_mob::Person* person, bool isQueuingBfrUpdate);
	unsigned int getQueuingAgentsCount() const;
	unsigned int getMovingAgentsCount() const;

	void resetIterator();
	sim_mob::Person* next();

	void initLaneParams(const sim_mob::Lane* lane, double vehSpeed, double pedSpeed);
	void updateOutputCounter(const sim_mob::Lane* lane);
	void updateOutputFlowRate(const sim_mob::Lane* lane, double newFlowRate);
	void updateAcceptRate(const sim_mob::Lane* lane, double upSpeed);

	// This function prints all agents in laneAgents
	void printAgents(bool copy = false) const;

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

	double getLength() const {
		return length;
	}

	unsigned int getNumPersons() const {
		return numPersons;
	}

	LaneParams* laneParams;
};

/**
 * Keeps a lane wise count of moving and queuing vehicles in a road segment.
 * Keeps a map of LaneStats corresponding
 * to each lane in the road segment. Used by mid term supply.
 *
 * \author Harish Loganathan
 */
class SegmentStats {

protected:
	typedef std::deque<sim_mob::Person*> PersonList;
	typedef std::map<const sim_mob::Lane*, sim_mob::LaneStats* > LaneStatsMap;
	const sim_mob::RoadSegment* roadSegment;
	LaneStatsMap laneStatsMap;
	std::map<const sim_mob::Lane*, sim_mob::Person* > frontalAgents;
	double length;

	double segVehicleSpeed; //speed of vehicles in segment for each frame
	double segPedSpeed; //speed of pedestrians on this segment for each frame--not used at the moment
	double segDensity;
	double lastAcceptTime;
	int numVehicleLanes;
	unsigned int segFlow;
	unsigned int numPersons;

public:
	SegmentStats(const sim_mob::RoadSegment* rdSeg, double length);
	~SegmentStats();

	enum VehicleType {
		CAR, BUS, NONE
	};

	enum SegmentVehicleOrdering {
		SEGMENT_ORDERING_BY_DISTANCE_TO_INTERSECTION,
		SEGMENT_ORDERING_BY_DRIVING_TIME_TO_INTERSECTION
	};

	SegmentVehicleOrdering orderBySetting;

	//TODO: in all functions which gets lane as a parameter, we must check if the lane belongs to the road segment.
	void addAgent(const sim_mob::Lane* lane, sim_mob::Person* p);
	void removeAgent(const sim_mob::Lane* lane, sim_mob::Person* ag, bool wasQueuing);
	sim_mob::Person* dequeue(const sim_mob::Lane* lane, bool isQueuingBfrUpdate);
	sim_mob::Person* dequeue(const sim_mob::Person* person, const sim_mob::Lane* lane, bool isQueuingBfrUpdate);

	std::deque<Person*>& getPersons(const sim_mob::Lane* lane);
	std::deque<Person*> getPersons();

	void updateLinkDrivingTimes(double drivingTimeToEndOfLink);
	void topCMergeLanesInSegment(PersonList& mergedPersonList);

	const sim_mob::RoadSegment* getRoadSegment() const;
	std::pair<unsigned int, unsigned int> getLaneAgentCounts(const sim_mob::Lane* lane) const; //returns std::pair<queuingCount, movingCount>
	unsigned int numAgentsInLane(const sim_mob::Lane* lane);
	void updateQueueStatus(const sim_mob::Lane* lane, sim_mob::Person* p);

	void resetFrontalAgents();
	sim_mob::Person* agentClosestToStopLineFromFrontalAgents();

	bool hasAgents();
	bool canAccommodate(SegmentStats::VehicleType type);

	unsigned int numMovingInSegment(bool hasVehicle) const;
	unsigned int numQueueingInSegment(bool hasVehicle) const;

	double getPositionOfLastUpdatedAgentInLane(const Lane* lane) const;
	void setPositionOfLastUpdatedAgentInLane(double positionOfLastUpdatedAgentInLane, const Lane* lane);
	void resetPositionOfLastUpdatedAgentOnLanes();

	sim_mob::LaneParams* getLaneParams(const Lane* lane) const;
	double speedDensityFunction(bool hasVehicle, double segDensity);
	void restoreLaneParams(const Lane* lane);
	void updateLaneParams(const Lane* lane, double newOutputFlowRate);
	void updateLaneParams(timeslice frameNumber);

	std::string reportSegmentStats(timeslice frameNumber);

	double getSegSpeed(bool hasVehicle) const;
	double getDensity(bool hasVehicle);
	unsigned int getSegFlow();
	void incrementSegFlow();
	void resetSegFlow();
	unsigned int getInitialQueueCount(const Lane* lane) const;
	unsigned int computeExpectedOutputPerTick();

	// This function prints all agents in this segment
	void printAgents();

	int getNumVehicleLanes() const {
		return numVehicleLanes;
	}

	double getLength() const {
		return length;
	}

	unsigned int getNumPersons() const {
		return numPersons;
	}

	/**
	 * laneInfinity is an augmented lane in the roadSegment. laneInfinity will be used only by confluxes and related objects for now.
	 * The LaneStats object created for laneInfinity stores the new persons who will start at this roadSegment. A Person will be
	 * added to laneInfinity (LaneStats corresponding to laneInfinity) when his start time falls within the current tick. The actual lane
	 * and moving/queuing status is still unknown for persons in laneInfinity. The frame_init function of the agent's role will have
	 * to put the persons from laneInfinity on moving/queuing vehicle lists on appropriate real lane.
	 */
	const sim_mob::Lane* laneInfinity;

	std::stringstream debugMsgs; // handy to throw meaningful error messages
};

}
