//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <set>
#include <vector>
#include "entities/Person_MT.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/PT_Stop.hpp"

namespace sim_mob
{
namespace medium
{

class BusStopAgent;
class Conflux;
class SegmentStats;

/**
 * helper to compare two persons by distance to end of segment
 */
struct GreaterDistToSegmentEnd: public std::greater<Person_MT*>
{
	/**
	 * compare 2 persons
	 * x > y if x is further away from the end of segment than y.
	 */
	bool operator()(const Person_MT* x, const Person_MT* y) const;
};

/*
 * SupplyParams is the place holder for storing the parameters of the
 * speed density function for this road segment.
 * \author Harish Loganathan
 */
struct SupplyParams
{
public:
	SupplyParams(const RoadSegment* rdSeg, double statsLength);

	const double getAlpha() const
	{
		return alpha;
	}

	const double getBeta() const
	{
		return beta;
	}

	const double getCapacity() const
	{
		return capacity;
	}

	const double getFreeFlowSpeed() const
	{
		return freeFlowSpeed;
	}

	const double getJamDensity() const
	{
		return jamDensity;
	}

	const double getMinDensity() const
	{
		return minDensity;
	}

	const double getMinSpeed() const
	{
		return minSpeed;
	}

private:
	double freeFlowSpeed;  ///<Maximum speed of the road segment in m/s
	double jamDensity;     ///<density during traffic jam in vehicles/m
	double minDensity;     ///<minimum traffic density in vehicles/m
	double minSpeed;       ///<minimum speed in the segment in m/s
	double capacity;       ///<segment capacity in vehicles/s
	const double alpha;          ///<Model parameter of speed density function
	const double beta;           ///<Model parameter of speed density function
};

/**
 * Data structure to store lane specific parameters for supply.
 *
 * \author Melani Jayasuriya
 */
class LaneParams
{
	friend class LaneStats;
	friend class SegmentStats;

private:
	double outputFlowRate; //vehicles/s
	double origOutputFlowRate; //vehicles/s
	int outputCounter;
	double acceptRate;
	double fraction;
	double lastAcceptTime;

public:
	LaneParams() :
			outputFlowRate(0.0), origOutputFlowRate(0.0), outputCounter(0), acceptRate(0.0), fraction(0.0), lastAcceptTime(0.0)
	{
	}

	double getOutputFlowRate()
	{
		return outputFlowRate;
	}
	int getOutputCounter()
	{
		return outputCounter;
	}
	double getAcceptRate()
	{
		return acceptRate;
	}

	void setOutputCounter(int count)
	{
		outputCounter = count;
	}
	void decrementOutputCounter();
	void setOutputFlowRate(double output)
	{
		outputFlowRate = output;
	}
	void setOrigOutputFlowRate(double orig)
	{
		origOutputFlowRate = orig;
	}

	void setLastAccept(double lastAccept)
	{
		lastAcceptTime = lastAccept;
	}
	double getLastAccept()
	{
		return lastAcceptTime;
	}
};

/**
 * Data structure to store persons in a lane. Persons are maintained with relative
 * ordering which reflects their positions in the lane during simulation.
 * The queuing and moving counts of the lane is also tracked by this class.
 * Used by mid term supply.
 *
 * \author Harish Loganathan
 */
class LaneStats
{
private:
	//typedefs
	typedef std::deque<Person_MT*> PersonList;

	/** number of persons queueing in lane */
	unsigned int queueCount;

	/** number of queuing persons at the start of the current tick */
	double initialQueueLength;

	/** end position of the last updated person in this lane */
	double positionOfLastUpdatedAgent;

	/** geospatial lane corresponding to this lane stats */
	const Lane* lane;

	/** virtual lane to hold newly starting persons */
	const bool laneInfinity;

	/** length of the lane in m (corresponds to length of segment stats of this lane stats) */
	double length;

	/** counter to track number of persons in this lane */
	unsigned int numPersons;

	/** tracks the queuing length of this segment in m*/
	double queueLength;

	/** tracks the moving length of this segment in m*/
	double totalLength;

	/** set of downstream links connected to this lanestats */
	std::set<const Link*> connectedDownstreamLinks;

	/**
	 * pointer to parent segment stats owning this lane stats
	 */
	const SegmentStats* parentStats;

public:
	PersonList laneAgents;

	LaneStats(const Lane* laneInSegment, double length, bool isLaneInfinity = false) :
			queueCount(0), initialQueueLength(0), laneParams(new LaneParams()), positionOfLastUpdatedAgent(-1.0), lane(laneInSegment), length(length),
			laneInfinity(isLaneInfinity), numPersons(0), queueLength(0), totalLength(0), parentStats(nullptr)
	{
	}

	~LaneStats()
	{
		safe_delete_item(laneParams);
	}

	/**
	 * adds a link as downstream to this lanestats
	 * @param downStreamLink a link which is downstream through this lanestats
	 * @return true if insertion was successful; false otherwise
	 */
	bool addDownstreamLink(const Link* downStreamLink);

	/**
	 * adds a set of links as downstream to this lanestats
	 * @param downStreamLinks a set of link which are downstream through this lanestats
	 */
	void addDownstreamLinks(const std::set<const Link*>& downStreamLinks);

	const std::set<const Link*>& getDownstreamLinks() const
	{
		return connectedDownstreamLinks;
	}

	/**
	 * adds person to laneAgents list
	 * @param person the person to be added
	 */
	void addPerson(Person_MT* person);

	/**
	 * update the queue count of lane based on the queuing status of person
	 * @param person the person whose queuing status has changed
	 */
	void updateQueueStatus(Person_MT* person);

	/**
	 * removes the person from the lane
	 * @param person the person to be removed
	 * @param wasQueuing the queuing status of person to manage queue count
	 * @param vehicleLength length of vehicle used by person
	 * @return true if removal was successful; false otherwise.
	 */
	bool removePerson(Person_MT* person, bool wasQueuing, double vehicleLength);

	/**
	 * removes the person at the front in laneAgents list
	 * @param isQueuingBfrUpdate queuing status of the person at front to manage queue count
	 * @param vehicleLength the length of vehicle used before update
	 * @return pointer to the dequeued person
	 */
	Person_MT* dequeue(const Person_MT* person, bool isQueuingBfrUpdate, double vehicleLength);

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
	 * initializes the parameters of lane
	 * @param vehSpeed speed of vehicles in lane in m/s
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
	 * @param upSpeed lane speed in m/s
	 */
	void updateAcceptRate(double upSpeed);

	/**
	 * This function prints all agents in laneAgents
	 */
	void printAgents() const;

	/**
	 * Verifies if the invariant that the order in laneAgents of each lane matches
	 * with the ordering w.r.t the distance to the end of segment
	 */
	void verifyOrdering();

	double getTotalVehicleLength() const
	{
		return totalLength;
	}

	double getQueueLength() const
	{
		return queueLength;
	}

	double getMovingLength() const;

	double getInitialQueueLength() const
	{
		return initialQueueLength;
	}

	void setInitialQueueLength(double initialQueueLength)
	{
		this->initialQueueLength = initialQueueLength;
	}

	double getPositionOfLastUpdatedAgent() const
	{
		return positionOfLastUpdatedAgent;
	}

	void setPositionOfLastUpdatedAgent(double positionOfLastUpdatedAgent)
	{
		this->positionOfLastUpdatedAgent = positionOfLastUpdatedAgent;
	}

	const Lane* getLane() const
	{
		return lane;
	}

	const bool isLaneInfinity() const
	{
		return laneInfinity;
	}

	double getLength() const
	{
		return length;
	}

	unsigned int getNumPersons() const
	{
		return numPersons;
	}

	const SegmentStats* getParentStats() const
	{
		return parentStats;
	}

	void setParentStats(const SegmentStats* parentStats)
	{
		this->parentStats = parentStats;
	}

	/** parameters for this lane */
	LaneParams* laneParams;
};

/**
 * Keeps a lane wise count of moving and queuing vehicles in a road segment.
 * Keeps a map of LaneStats corresponding
 * to each lane in the road segment. Used by mid term supply.
 *
 * \author Harish Loganathan
 */
class SegmentStats
{
protected:
	friend class Conflux;

	//typedefs
	typedef std::deque<Person_MT*> PersonList;
	typedef std::map<const Lane*, LaneStats*> LaneStatsMap;
	typedef std::vector<const BusStop*> BusStopList;
	typedef std::vector<BusStopAgent*> BusStopAgentList;
	typedef std::map<const BusStop*, PersonList> StopBusDriversMap;

	/** road segment which contains this SegmentStats */
	const RoadSegment* roadSegment;

	/** Conflux to which this segment stats belongs to */
	Conflux* parentConflux;

	/**
	 * List of bus stops in this SegmentStats. One SegmentStats can have
	 * multiple stops. This design allows us to handle the case where there are
	 * different bus stops located close to each other. Allowing multiple stops
	 * eliminates the need for splitting the segment into very short segments.
	 */
	BusStopList busStops;

	/** BusStopAgents for bus stops in this segment stats */
	BusStopAgentList busStopAgents;

	/** stop wise list of bus drivers currently serving the stop */
	StopBusDriversMap busDrivers;

	/**
	 * A segment can have multiple segment stats. This gives the position of this
	 * SegmentStats in segment.
	 */
	uint16_t statsNumberInSegment;

	/**
	 * Map containing LaneStats for every lane of the segment.
	 * This map includes lane infinity.
	 */
	LaneStatsMap laneStatsMap;

	/**
	 * outermost lane in this segment stats.
	 * This is a proper lane in the segment which is chosen by buses, taxis and
	 * other vehicles (before moving into the virtual stopping lane) when they
	 * have to stop in this segment stats.
	 */
	const Lane* outermostLane;

	/** length of this SegmentStats in m */
	double length;

	/** speed of vehicles in segment for each frame in m/s */
	double segVehicleSpeed;

	/** vehicle density of this segment stats in PCU/m */
	double segDensity;

	/** number of lanes in this SegmentStats which is meant for vehicles */
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
	SupplyParams supplyParams;

	/**
	 * map of lanes connected to each downstream link
	 */
	std::map<const Link*, std::vector<LaneStats*> > laneGroup;

public:
	SegmentStats(const RoadSegment* rdSeg, Conflux* parentConflux, double length);
	~SegmentStats();

	enum SegmentVehicleOrdering
	{
		SEGMENT_ORDERING_BY_DISTANCE_TO_INTERSECTION, SEGMENT_ORDERING_BY_DRIVING_TIME_TO_INTERSECTION
	};

	SegmentVehicleOrdering orderBySetting;

	const RoadSegment* getRoadSegment() const
	{
		return roadSegment;
	}

	Conflux* getParentConflux() const
	{
		return parentConflux;
	}

	int getNumVehicleLanes() const
	{
		return numVehicleLanes;
	}

	double getLength() const
	{
		return length;
	}

	unsigned int getNumPersons() const
	{
		return numPersons;
	}

	size_t getNumStops() const
	{
		return busStops.size();
	}

	uint16_t getStatsNumberInSegment() const
	{
		return statsNumberInSegment;
	}

	const Lane* getOutermostLane() const
	{
		return outermostLane;
	}

	double getSegSpeed(bool hasVehicle) const
	{
		if (hasVehicle)
		{
			return segVehicleSpeed;
		}
		return 0.0;
	}

	unsigned int getSegFlow()
	{
		return segFlow;
	}

	/** increments segment flow counter by 1 */
	void incrementSegFlow();

	/** resets segment flow counter to 0 */
	void resetSegFlow();

	/**
	 * fetches queue length at start of tick
	 * @param lane the lane for which queue length is requested
	 * @return
	 */
	double getInitialQueueLength(const Lane* lane) const;

	/**
	 * adds person to lane in segment
	 * @param lane the lane to add the person in
	 * @param person the person to be added
	 */
	void addAgent(const Lane* lane, Person_MT* person);

	/**
	 * adds a bus stop to the list of stops
	 * @param stop bus stop to be added
	 */
	void addBusStop(const BusStop* stop);

	/**
	 * adds bus stop agent
	 * @param busStopAgent is a pointer to a bus stop agent
	 */
	void addBusStopAgent(BusStopAgent* busStopAgent);

	/**
	 * Initializes all the bus stops in this segment stats.
	 * The bus stop agents corresponding to the stops in this segment stats are
	 * registered with the message bus in this function.
	 */
	void initializeBusStops();

	/**
	 * add bus driver to stop
	 * @param driver the bus driver to be added
	 */
	void addBusDriverToStop(Person_MT* driver, const BusStop* stop);

	/**
	 * remove bus driver from stop
	 * @param driver the bus driver to be removed
	 */
	void removeBusDriverFromStop(Person_MT* driver, const BusStop* stop);

	/**
	 * removes person from lane
	 * @param lane the lane to remove the person from
	 * @param person the person to remove
	 * @param wasQueuing the queuing status of person at the start of the tick
	 * @param vehicleLength the length of vehicle used before update
	 * @return true if removal was successful; false otherwise
	 */
	bool removeAgent(const Lane* lane, Person_MT* person, bool wasQueuing, double vehicleLength);

	/**
	 * removes person in the front from lane
	 * @param person the person to remove
	 * @param lane the lane to remove the person from
	 * @param isQueuingBfrUpdate the queuing status of person at the start of the tick
	 * @param vehicleLength the length of vehicle used before update
	 * @return the dequeued person
	 */
	Person_MT* dequeue(const Person_MT* person, const Lane* lane, bool isQueuingBfrUpdate, double vehicleLength);

	/**
	 * returns a reference to the list of persons in lane
	 * @param lane the lane from which the list of persons is requested
	 * @return reference to the list of persons in lane
	 */
	std::deque<Person_MT*>& getPersons(const Lane* lane);

	/**
	 * returns a reference to the list of bus stops
	 * @return reference to the list of bus stops
	 */
	std::vector<const BusStop*>& getBusStops();

	/**
	 * update bus stop agent so as to perform further tasks
	 * @param now is current time.
	 */
	void updateBusStopAgents(timeslice now);

	/**
	 * get a list of all persons in the segment stats
	 * @param out list for all persons in the segment stats
	 */
	void getPersons(std::deque<Person_MT*>& outList);

	/**
	 * get a list of all persons in the infinite lane
	 * @param out list for all persons in the infinite lane
	 * @param personIds for all persons ids in the segment stats
	 */
	void getInfinityPersons(std::deque<Person_MT*>& segAgents, std::string& personIds);

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
	std::pair<unsigned int, unsigned int> getLaneAgentCounts(const Lane* lane) const;

	/**
	 * gets the queue length of lane
	 * @param lane the lane for which queue length is requested
	 * @returns queue length of lane
	 */
	double getLaneQueueLength(const Lane* lane) const;

	/**
	 * gets the moving length of lane
	 * @param lane the lane for which moving length is requested
	 * @returns moving length of lane
	 */
	double getLaneMovingLength(const Lane* lane) const;

	/**
	 * Returns the total length of vehicles in lane.
	 * @param lane the lane for which moving length is requested
	 * @returns total length of vehicles in lane
	 */
	double getLaneTotalVehicleLength(const Lane* lane) const;

	/**
	 * Returns the sum of queuing lengths of all lanes in this seg stats.
	 * This function considers only vehicle lanes
	 * @returns total queuing length of this seg stats
	 */
	double getQueueLength() const;

	/**
	 * Returns the sum of moving lengths of all lanes in this seg stats
	 * This function considers only vehicle lanes
	 * @returns total moving length of this seg stats
	 */
	double getMovingLength() const;

	/**
	 * Returns the sum of lengths of vehicles in all lanes in this seg stats
	 * This function considers only vehicle lanes
	 * @return total length of vehicles in this seg stats
	 */
	double getTotalVehicleLength() const;

	/**
	 * returns the number of persons in lane
	 * @param lane the lane for which the number of persons is required
	 * @return the number of persons in lane
	 */
	unsigned int numAgentsInLane(const Lane* lane) const;

	/**
	 * updates the queue count in lane due to change in queuing status of person
	 * @param lane the lane whose queueing count needs to be updated
	 * @param person the person who changed his queuing status
	 */
	void updateQueueStatus(const Lane* lane, Person_MT* person);

	/**
	 * checks if the segment stats has persons
	 * @return true if numPersons > 0; false otherwise
	 */
	bool hasPersons() const;

	/**
	 * checks if this Segment stats contains busStop in it
	 * @param busStop the stop to find
	 * @returns true if this segstats cotains busStop in its busStops list;
	 * 			false otherwise
	 */
	bool hasBusStop(const BusStop* busStop) const;

	/**
	 * checks if this Segment stats contains a busStop in it
	 * @returns true if this segstats cotains a busStop in its busStops list;
	 * 			false otherwise
	 */
	bool hasBusStop() const;

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
	LaneParams* getLaneParams(const Lane* lane) const;

	/**
	 * the speed density function for mid-term supply.
	 * Computes the speed of vehicles in segment, given the density
	 * @param segDensity the vehicle density of segment in vehicle/m
	 * @return speed of the segment in cm/s
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
	std::string reportSegmentStats(uint32_t frameNumber);

	/**
	 * computes the density of the moving part of the segment
	 * the density value computed here is meant to be used in speed density function
	 * @param vehicleLanes boolean flag indicating whether we want the density from vehicle lanes
	 * @return density in PCU/lane-m
	 */
	double getDensity(bool vehicleLanes);

	/**
	 * computes the density of the total segment-stats in vehicles/lane-km
	 * the density value computed here is meant to be used for outputs
	 * @param vehicleLanes boolean flag indicating whether we want the density from vehicle lanes
	 */
	double getTotalDensity(bool vehicleLanes);

	/**
	 * computes the number of persons that can to leave this segment stats in
	 * 1 tick based on output flow rates of lanes
	 * @return number of persons that can to leave this segment stats
	 */
	unsigned int computeExpectedOutputPerTick();

	/**
	 * performs message handler registrations for bus stops in this seg stats
	 */
	void registerBusStopAgents();

	/**
	 * checks whether lane stats for lane is connected (eventually) to the next down stream link
	 * @param downstreamLink next down stream link
	 * @param lane lane in this segstats
	 * @return true if connected; false otherwise.
	 */
	bool isConnectedToDownstreamLink(const Link* downstreamLink, const Lane* lane) const;

	/**
	 * returns the maximum allowed length of vehicles in the lane group of supplied lane
	 * @param nextLink link downstream to the segment of lane (to identify lane group)
	 * @return maximum allowed length of vehicles in the lane group of valid input lane
	 */
	double getAllowedVehicleLengthForLaneGroup(const Link* downstreamLink) const;

	/**
	 * returns the existing length of vehicles in the lane group of supplied lane
	 * @param nextLink link downstream to the segment of lane (to identify lane group)
	 * @return maximum allowed length of vehicles in the lane group of valid input lane
	 */
	double getVehicleLengthForLaneGroup(const Link* downstreamLink) const;

	/**
	 * prints all downstream links for all lanestats of this segment
	 */
	void printDownstreamLinks() const;

	/**
	 * prints all agents in this segment
	 */
	void printAgents() const;

	/**
	 * prints all stops in this segment stats
	 */
	void printBusStops() const;

	/**
	 * gets the output capacity of segmentStats
	 */
	double getCapacity() const;

	/**
	 * counts the total number of agents waiting in bus stops of this segment stats
	 */
	unsigned int getBusWaitersCount() const;

	/**
	 * tells whether this segment stats is short
	 * @return true if short; false otherwise
	 */
	bool isShortSegment() const;

	/**
	 * laneInfinity is an augmented lane in the roadSegment. laneInfinity will be used only by confluxes and related objects for now.
	 * The LaneStats object created for laneInfinity stores the new persons who will start at this roadSegment. A Person will be
	 * added to laneInfinity (LaneStats corresponding to laneInfinity) when his start time falls within the current tick. The actual lane
	 * and moving/queuing status is still unknown for persons in laneInfinity. The frame_init function of the agent's role will have
	 * to put the persons from laneInfinity on moving/queuing vehicle lists on appropriate real lane.
	 */
	Lane* laneInfinity;
};
} // namespace medium
} // namespace sim_mob
