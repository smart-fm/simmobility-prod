//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include "entities/Person.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/BusStop.hpp"

namespace sim_mob {

struct cmp_person_distToSegmentEnd : public std::greater<Person*> {
	/**
	 * compare 2 persons
	 * x > y if x is further away from the end of segment than y.
	 */
	bool operator() (const Person* x, const Person* y) const;
};

/*
 * SupplyParams is the place holder for storing the parameters of the
 * speed density function for this road segment.
 * \author Harish Loganathan
 */
struct SupplyParams {
public:
	SupplyParams(const sim_mob::RoadSegment* rdSeg, double statsLength);

	const double getAlpha() const {
		return alpha;
	}

	const double getBeta() const {
		return beta;
	}

	const double getCapacity() const {
		return capacity;
	}

	const double getFreeFlowSpeed() const {
		return freeFlowSpeed;
	}

	const double getJamDensity() const {
		return jamDensity;
	}

	const double getMinDensity() const {
		return minDensity;
	}

	const double getMinSpeed() const {
		return minSpeed;
	}

private:
	const double freeFlowSpeed;  ///<Maximum speed of the road segment
	const double jamDensity;     ///<density during traffic jam in vehicles / m
	const double minDensity;     ///<minimum traffic density in vehicles / m
	const double minSpeed;       ///<minimum speed in the segment
	const double capacity;       ///<segment capacity in vehicles/second
	const double alpha;          ///<Model parameter of speed density function
	const double beta;           ///<Model parameter of speed density function
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
 * Data structure to store persons in a lane. Persons are maintained with relative
 * ordering which reflects their positions in the lane during simulation.
 * The queuing and moving counts of the lane is also tracked by this class.
 * Used by mid term supply.
 *
 * \author Harish Loganathan
 */
class LaneStats {
private:
	//typedefs
	typedef std::deque<sim_mob::Person*> PersonList;
	/**number of persons queueing in lane*/
	unsigned int queueCount;
	/**number of queuing persons at the start of the current tick*/
	unsigned int initialQueueCount;
	/**position of the last updated person in lane*/
	double positionOfLastUpdatedAgent;
	/**geospatial lane corresponding to this lane stats*/
	const sim_mob::Lane* lane;
	/**virtual lane to hold newly starting persons*/
	const bool laneInfinity;
	/**length of the lane (corresponds to length of segment stats of this lane stats)*/
	double length;
	/**counter to track number of persons in this lane*/
	unsigned int numPersons;
	/**tracks the queueing length of this segment*/
	double queueingLength;
	/**tracks the moving length of this segment*/
	double movingLength;

	/*
	 * laneAgentsCopy is a copy of laneAgents taken at the start of each tick
	 * solely for iterating the agents. laneAgentsIt will iterate on laneAgentsCopy
	 * and stays intact. Any handover of agents to the next segment is done by
	 * removing the agent from laneAgents and adding to laneAgents of the next segment.
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

	/**
	 * adds person to laneAgents list
	 * @param person the person to be added
	 */
	void addPerson(sim_mob::Person* person);

	/**
	 * update the queue count of lane based on the queuing status of person
	 * @param person the person whose queuing status has changed
	 */
	void updateQueueStatus(sim_mob::Person* person);

	/**
	 * removes the person from the lane
	 * @param person the person to be removed
	 * @param wasQueuing the queuing status of person to manage queue count
	 */
	void removePerson(sim_mob::Person* person, bool wasQueuing);

	/**
	 * removes the person at the front in laneAgents list
	 * @param isQueuingBfrUpdate queuing status of the person at front to manage queue count
	 * @return pointer to the dequeued person
	 */
	sim_mob::Person* dequeue(const sim_mob::Person* person, bool isQueuingBfrUpdate);

	/**
	 * gets the number of queuing persons in lane
	 * @return number of queuing persons in lane
	 */
	unsigned int getQueuingAgentsCount() const;

	/**
	 * gets the number of moving persons in lane
	 * @return number of moving persons in lane
	 */
	unsigned int getMovingAgentsCount() const;

	/**
	 * copies laneAgents to laneAgentsCopy; sets laneAgentsIt to the start of laneAgentsCopy
	 * NOTE: this must be called once per time-tick
	 */
	void resetIterator();

	/**
	 * gets the person from laneAgentsIt and increments laneAgentsIt
	 * @return the next person to be updated in lane
	 */
	sim_mob::Person* next();

	/**
	 * initializes the parameters of lane
	 * @param vehSpeed speed of vehicles in lane
	 * @param capacity of the segment stats containing this lane stats (in vehicles/s)
	 */
	void initLaneParams(double vehSpeed, double capacity);

	/**
	 * updates the output counter of lane
	 */
	void updateOutputCounter();

	/**
	 * updates the output flow rate of lane
	 * @param newFlowRate new output flow rate
	 */
	void updateOutputFlowRate(double newFlowRate);

	/**
	 * updates the accept rate of lane
	 * @param upSpeed lane speed
	 */
	void updateAcceptRate(double upSpeed);

	/**
	 * This function prints all agents in laneAgents
	 * @param copy boolean to determine whether to print laneAgentsCopy or laneAgents
	 */
	void printAgents(bool copy = false) const;

	/**
	 * Verifies if the invariant that the order in laneAgents of each lane matches
	 * with the ordering w.r.t the distance to the end of segment
	 */
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

	/**parameters for this lane*/
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
	friend class sim_mob::aimsun::Loader;

protected:
	//typedefs
	typedef std::deque<sim_mob::Person*> PersonList;
	typedef std::map<const sim_mob::Lane*, sim_mob::LaneStats* > LaneStatsMap;
	typedef std::vector<const sim_mob::BusStop*> BusStopList;

	/**road segment which contains this SegmentStats*/
	const sim_mob::RoadSegment* roadSegment;
	/**
	 * List of bus stops in this SegmentStats. One SegmentStats can have
	 * multiple stops. This design allows us to handle the case where there are
	 * different bus stops located close to each other. Allowing multiple stops
	 * eliminates the need for splitting the segment into very short segments.
	 */
	BusStopList busStops;
	/**
	 * A segment can have multiple segment stats. This gives the position of this
	 * SegmentStats in segment.
	 */
	uint8_t positionInRoadSegment;
	/**
	 * Map containing LaneStats for every lane of the segment.
	 * This map includes lane infinity.
	 */
	LaneStatsMap laneStatsMap;
	/**
	 * A map which stores the unprocessed person who is closest to the end of
	 * this SegmentStats for each lane in the seg stats. This is used for
	 * selecting the next person to update.
	 * TODO: remove this, if TopCMerge seems to work perfectly well.
	 */
	std::map<const sim_mob::Lane*, sim_mob::Person* > frontalAgents;
	/**length of this SegmentStats in cm*/
	double length;
	/**speed of vehicles in segment for each frame*/
	double segVehicleSpeed;
	/**speed of pedestrians on this segment for each frame--not used at the moment*/
	double segPedSpeed;
	/**vehicle density of this segment stats in PCU/cm*/
	double segDensity;
	/**number of lanes in this SegmentStats which is meant for vehicles*/
	int numVehicleLanes;
	/**
	 * counter which stores the number of vehicles which crossed the mid-point
	 * of this segment stats in every tick
	 */
	unsigned int segFlow;
	/**
	 * counter which tracks the number of Persons currently on this SegmentStats
	 */
	unsigned int numPersons;
	/**
	 * structure to store parameters pertinent to supply
	 */
	sim_mob::SupplyParams supplyParams;
	/**
	 * adds a bus stop to the list of stops
	 * @param stop bus stop to be added
	 */
	void addBusStop(const sim_mob::BusStop* stop);

public:
	SegmentStats(const sim_mob::RoadSegment* rdSeg, double length);
	~SegmentStats();

	enum SegmentVehicleOrdering {
		SEGMENT_ORDERING_BY_DISTANCE_TO_INTERSECTION,
		SEGMENT_ORDERING_BY_DRIVING_TIME_TO_INTERSECTION
	};

	SegmentVehicleOrdering orderBySetting;

	const sim_mob::RoadSegment* getRoadSegment() const {
		return roadSegment;
	}

	int getNumVehicleLanes() const {
		return numVehicleLanes;
	}

	double getLength() const {
		return length;
	}

	unsigned int getNumPersons() const {
		return numPersons;
	}

	uint8_t getPositionInRoadSegment() const {
		return positionInRoadSegment;
	}

	double getSegSpeed(bool hasVehicle) const;
	unsigned int getSegFlow();
	void incrementSegFlow();
	void resetSegFlow();
	unsigned int getInitialQueueCount(const Lane* lane) const;

	/**
	 * adds person to lane in segment
	 * @param lane the lane to add the person in
	 * @param person the person to be added
	 */
	void addAgent(const sim_mob::Lane* lane, sim_mob::Person* person);

	/**
	 * removes person from lane
	 * @param lane the lane to remove the person from
	 * @param person the person to remove
	 * @param wasQueuing the queuing status of person at the start of the tick
	 */
	void removeAgent(const sim_mob::Lane* lane, sim_mob::Person* person, bool wasQueuing);

	/**
	 * removes person in the front from lane
	 * @param person the person to remove
	 * @param lane the lane to remove the person from
	 * @param isQueuingBfrUpdate the queuing status of person at the start of the tick
	 * @return the dequeued person
	 */
	sim_mob::Person* dequeue(const sim_mob::Person* person, const sim_mob::Lane* lane, bool isQueuingBfrUpdate);

	/**
	 * returns a reference to the list of persons in lane
	 * @param lane the lane from which the list of persons is requested
	 * @return reference to the list of persons in lane
	 */
	std::deque<Person*>& getPersons(const sim_mob::Lane* lane);

	/**
	 * get a list of all persons in the segment stats
	 * @return list of all persons in the segment stats
	 */
	std::deque<Person*> getPersons();

	/**
	 * updates the driving time to reach end of link of all persons in segment stats
	 * @param drivingTimeToEndOfLink the driving time to reach the end of link after exiting this segment stats
	 */
	void updateLinkDrivingTimes(double drivingTimeToEndOfLink);

	/**
	 * merges the persons in segment in one list, thus forming the order in which
	 * those persons need to be updated in this tick
	 * @param mergedPersonList output list of persons to be populated
	 */
	void topCMergeLanesInSegment(PersonList& mergedPersonList);

	/**
	 * returns the queuing and moiving persons count in lane
	 * @param lane the lane for which the counts are required
	 * @return std::pair<queuingCount, movingCount>
	 */
	std::pair<unsigned int, unsigned int> getLaneAgentCounts(const sim_mob::Lane* lane) const;

	/**
	 * returns the number of persons in lane
	 * @param lane the lane for which the number of persons is required
	 * @return the number of persons in lane
	 */
	unsigned int numAgentsInLane(const sim_mob::Lane* lane) const;

	/**
	 * updates the queue count in lane due to change in queuing status of person
	 * @param lane the lane whose queueing count needs to be updated
	 * @param person the person who changed his queuing status
	 */
	void updateQueueStatus(const sim_mob::Lane* lane, sim_mob::Person* person);

	/**
	 * resets the frontal agents
	 */
	void resetFrontalAgents();

	/**
	 * gets the person closest to end of segment
	 * @return person closest to end of segment
	 */
	sim_mob::Person* personClosestToSegmentEnd();

	/**
	 * checks if the segment stats has persons
	 * @return true if numPersons > 0; false otherwise
	 */
	bool hasAgents() const;

	/**
	 * returns the number of agents moving in segment
	 * @param vehicleLanes boolean flag indicating whether we want the numbers from vehicle lanes
	 * @return the number of agents moving in segment
	 */
	unsigned int numMovingInSegment(bool vehicleLanes) const;

	/**
	 * returns the number of agents queuing in segment
	 * @param vehicleLanes boolean flag indicating whether we want the numbers from vehicle lanes
	 * @return the number of agents queuing in segment
	 */
	unsigned int numQueuingInSegment(bool hasVehicle) const;

	/**
	 * gets the position of last updated person in lane
	 * @param lane the requested lane in segment stats
	 * @return the distance of last updated person in lane from end of segment
	 */
	double getPositionOfLastUpdatedAgentInLane(const Lane* lane) const;

	/**
	 * sets the position of last updated person in lane
	 * @param positionOfLastUpdatedAgentInLane the distance of last updated person in lane from end of segment
	 * @param lane the requested lane in segment stats
	 */
	void setPositionOfLastUpdatedAgentInLane(double positionOfLastUpdatedAgentInLane, const Lane* lane);

	/**
	 * sets the position of last updated person in all lanes to -1,
	 * which means no person has been updated in those lanes. This is called once
	 * at the start of the tick before updated any person in the conflux.
	 */
	void resetPositionOfLastUpdatedAgentOnLanes();

	/**
	 * gets the lane params of lane
	 * @param lane the requested lane in segment stats
	 */
	sim_mob::LaneParams* getLaneParams(const Lane* lane) const;

	/**
	 * the speed density function for mid-term supply.
	 * Computes the speed of vehicles in segment, given the density
	 * @param segDensity the vehicle density of segment in vehicle/m
	 */
	double speedDensityFunction(const double segDensity) const;

	/**
	 * restore the lane params of lane to original values
	 * @param lane the requested lane in segment stats
	 */
	void restoreLaneParams(const Lane* lane);

	/**
	 * update the lane params of lane
	 * @param lane the requested lane in segment stats
	 */
	void updateLaneParams(const Lane* lane, double newOutputFlowRate);

	/**
	 * update the lane params of lane for a frame tick
	 * @param frameNumber the timeslice of current frame
	 */
	void updateLaneParams(timeslice frameNumber);

	/**
	 * report the statistics of this segment stats in string format
	 * @param frameNumber the timeslice of current frame
	 * @return the statistics of this segment stats in string format
	 */
	std::string reportSegmentStats(timeslice frameNumber);

	/**
	 * computes the density of the segment
	 * @param vehicleLanes boolean flag indicating whether we want the density from vehicle lanes
	 */
	double getDensity(bool vehicleLanes);

	/**
	 * computes the number of persons that can to leave this segment stats in
	 * 1 tick based on output flow rates of lanes
	 * @return number of persons that can to leave this segment stats
	 */
	unsigned int computeExpectedOutputPerTick();

	/**
	 * prints all agents in this segment
	 */
	void printAgents();

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

typedef boost::shared_ptr<SegmentStats> SegmentStatsPtr;

}
