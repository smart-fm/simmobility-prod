//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>

#include "entities/Agent.hpp"
#include "entities/signal/Signal.hpp"
#include "boost/thread/shared_mutex.hpp"

namespace sim_mob {

class Person;
class RoadSegment;
class SegmentStats;
class MultiNode;
class Worker;


namespace aimsun
{
//Forward declaration
class Loader;
}

struct cmp_person_remainingTimeThisTick : public std::greater<Person*> {
  bool operator() (const Person* x, const Person* y) const;
};

/**
 * Sort all persons in non-increasing order of remaining times in the current tick
 * @param personList list of persons to be sorted
 */
void sortPersons_DecreasingRemTime(std::deque<Person*>& personList);

/**
 * Class representing a grouping of an intersection along with the links which
 * are directly upstream to that intersection.
 *
 * \author Harish Loganathan
 */
class Conflux : public sim_mob::Agent {

	friend class sim_mob::aimsun::Loader;

private:
	/**
	 *  MultiNode (intersection) around which this conflux is constructed
	 */
	const sim_mob::MultiNode* multiNode;

	/**
	 * Signal at the multinode of this conflux (if any).
	 * Note: Signals do not work in the mid-term. This is useless for now.
	 */
	const sim_mob::Signal* signal;

	/**
	 * Link-wise list of road segments in this conflux
	 * stores std::map<link upstream to multiNode, segments on link>
	 */
	std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> > upstreamSegmentsMap;

	/**
	 * virtual queues are used to hold persons who want to move in from adjacent
	 * confluxes when this conflux is not processed for the current tick yet.
	 * Each link in the conflux has 1 virtual queue.
	 */
	std::map<sim_mob::Link*, std::deque<sim_mob::Person*> > virtualQueuesMap;

	/**
	 * data structure to hold a pointer to a road segment on each link to
	 * keep track of the current segment that is being processed.
	 */
	std::map<sim_mob::Link*, const sim_mob::RoadSegment*> currSegsOnUpLinks;

	/**
	 * segments on downstream links
	 * These links conceptually belong to the adjacent confluxes.
	 */
	std::set<const sim_mob::RoadSegment*> downstreamSegments;

	/**
	 *  Map which stores the SegmentStats for all road segments on upstream links
	 *  The Segment stats in-turn contain LaneStats which contain the persons.
	 */
	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*> segmentAgents;

	/**
	 *  Worker to which this conflux belongs to.
	 *  Note: this
	 */
	sim_mob::Worker* parentWorker;

	/**
	 * structure to store the frontal agents in each road segment
	 */
	std::map<const sim_mob::RoadSegment*, sim_mob::Person* > candidateAgents;

	/**
	 * mutex to protect virtual queues
	 */
	boost::recursive_mutex mutexOfVirtualQueue;

	/**
	 * For each downstream link, this map stores the number of persons that can
	 * be accepted by that link from this conflux in the current tick
	 */
	std::map<sim_mob::Link*, unsigned int> vqBounds;

	/**holds the current frame number for which this conflux is being processed*/
	timeslice currFrameNumber;

	/**list of persons performing activities within the vicinity of this conflux*/
	std::deque<Person*> activityPerformers;

	/** function to call persons' updates if the MultiNode is signalized */
	void updateSignalized();

	/** function to call persons' updates if the MultiNode is not signalized */
	void updateUnsignalized();

	/**
	 * moves the person and does housekeeping for the conflux
	 * @param person the person to move
	 * */
	void updateAgent(sim_mob::Person* person);

	/** calls frame_tick() of the movement facet for the person's role*/
	UpdateStatus perform_person_move(timeslice now, Person* person);

	/** calls frame_init of the movement facet for the person's role*/
	bool call_movement_frame_init(timeslice now, Person* person);

	/** calls frame_tick of the movement facet for the person's role*/
	Entity::UpdateStatus call_movement_frame_tick(timeslice now, Person* person);

	/** calls frame_tick of the movement facet for the person's role*/
	void call_movement_frame_output(timeslice now, Person* person);

	/** function to initialize candidate agents in each tick*/
	void initCandidateAgents();

	/** sets the iterators on currSegToUpLinks to the segments at the end of the links*/
	void resetCurrSegsOnUpLinks();

	/** selects the agent closest to the intersection from candidateAgents;*/
	sim_mob::Person* agentClosestToIntersection();

	/**
	 * removes the agent from the conflux and marks it for removal by the worker.
	 * The person gets removed from the simulation at the end of the current tick.
	 * @param ag the person to be removed
	 * @param prevRdSeg the segment where the person started in the current tick
	 * @param prevLane the lane from which the person started in the current tick
	 * @param wasQueuing flag indicating whether the person was queuing at the start of the tick
	 * @param wasActivityPerformer flag indicating whether the person was performing an activity at the start of the tick
	 */
	void killAgent(sim_mob::Person* ag, const sim_mob::RoadSegment* prevRdSeg,
			const sim_mob::Lane* prevLane, bool wasQueuing, bool wasActivityPerformer);

	/**
	 * Resets the remainingTime of persons who remain in
	 * lane infinities and virtual queues across ticks
	 * Note: This may include
	 * 1. newly starting persons who (were supposed to, but) could not get added
	 * to the simulation in the previous tick due to traffic congestion in their
	 * starting segment. (lane infinity)
	 * 2. Persons who got added to and remained virtual queue in the previous tick
	 */
	void resetPersonRemTimes();

protected:
	//NOTE: New Agents use frame_* methods, but Conflux is fine just using update()
	virtual bool frame_init(timeslice now) { throw std::runtime_error("frame_* methods are not required and are not implemented for Confluxes."); }
	virtual Entity::UpdateStatus frame_tick(timeslice now) { throw std::runtime_error("frame_* are not required and are not implemented for Confluxes."); }
	virtual void frame_output(timeslice now) { throw std::runtime_error("frame_* methods are not required and are not implemented for Confluxes."); }

public:
	Conflux(sim_mob::MultiNode* multinode, const MutexStrategy& mtxStrat, int id=-1);
	virtual ~Conflux() ;

	//Confluxes are non-spatial in nature.
	virtual bool isNonspatial() { return true; }

	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	// functions from agent
	virtual void load(const std::map<std::string, std::string>&) {}
	virtual Entity::UpdateStatus update(timeslice frameNumber);

	// Getters
	const sim_mob::MultiNode* getMultiNode() const {
		return multiNode;
	}

	const sim_mob::Signal* getSignal() const {
		return signal;
	}

	std::set<const sim_mob::RoadSegment*> getDownstreamSegments() {
		return downstreamSegments;
	}

	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*> getSegmentAgents() const {
		return segmentAgents;
	}

	sim_mob::Worker* getParentWorker() const {
		return parentWorker;
	}

	void setParentWorker(sim_mob::Worker* parentWorker) {
		this->parentWorker = parentWorker;
	}

	int getVehicleLaneCounts(const sim_mob::RoadSegment* segStats);

	bool hasSpaceInVirtualQueue(sim_mob::Link* lnk);
	void pushBackOntoVirtualQueue(sim_mob::Link* lnk, sim_mob::Person* p);

	/**
	 * adds a person into this conflux
	 * @param ag person to be added
	 * @param rdSeg starting road segment of ag
	 */
	void addAgent(sim_mob::Person* ag, const sim_mob::RoadSegment* rdSeg);

	/**
	 * get lanewise agent counts in a segment
	 * @param rdSeg segment for which the counts are needed
	 * @returns lane wise std::pair<queuingCount, movingCount>
	 */
	std::map<const sim_mob::Lane*, std::pair<unsigned int, unsigned int> >
	getLanewiseAgentCounts(const sim_mob::RoadSegment* rdSeg);

	/**
	 * The following 2 functions gets the moving and queuing counts of a segment respectively
	 */
	unsigned int numMovingInSegment(const sim_mob::RoadSegment* rdSeg, bool hasVehicle);
	unsigned int numQueueingInSegment(const sim_mob::RoadSegment* rdSeg, bool hasVehicle);

	/**Searches upstream and downstream segments to get the segmentStats for the requested road segment*/
	sim_mob::SegmentStats* findSegStats(const sim_mob::RoadSegment* rdSeg);

	/**
	 * supply params related functions
	 */
	double getOutputFlowRate(const Lane* lane);
	int getOutputCounter(const Lane* lane);
	void setOutputCounter(const Lane* lane, int count);
	double getAcceptRate(const Lane* lane);
	double getSegmentSpeed(const RoadSegment* rdSeg, bool hasVehicle);
	void updateSupplyStats(const Lane* lane, double newOutputFlowRate);
	void restoreSupplyStats(const Lane* lane);
	std::pair<unsigned int, unsigned int> getLaneAgentCounts(const sim_mob::Lane* lane); //returns std::pair<queuingCount, movingCount>
	unsigned int getInitialQueueCount(const Lane* lane);
	double getLastAccept(const Lane* lane);
	void setLastAccept(const Lane* lane, double lastAccept);
	void resetPositionOfLastUpdatedAgentOnLanes();
	void updateLaneParams(const Lane* lane, double newOutFlowRate);
	void restoreLaneParams(const Lane* lane);
	double getSegmentFlow(const RoadSegment* rdSeg);
	void incrementSegmentFlow(const RoadSegment* rdSeg);
	void resetSegmentFlows();

	/** updates lane params for all lanes within the conflux */
	void updateAndReportSupplyStats(timeslice frameNumber);

	/**process persons in the virtual queue*/
	void processVirtualQueues();

	//TODO: To be removed after debugging.
	std::stringstream debugMsgs;

	//=======link travel time computation for current frame tick =================
	struct linkTravelTimes
	{
	public:
		double linkTravelTime_;
		unsigned int agentCount_;

		linkTravelTimes(double linkTravelTime, unsigned int agentCount)
		: linkTravelTime_(linkTravelTime), agentCount_(agentCount) {}
	};

	std::map<const Link*, linkTravelTimes> LinkTravelTimesMap;
	void setLinkTravelTimes(Person* ag, double linkExitTime);
	void resetLinkTravelTimes(timeslice frameNumber);
	void reportLinkTravelTimes(timeslice frameNumber);

	//=======road segment travel time computation for current frame tick =================
	struct rdSegTravelTimes
	{
	public:
		double rdSegTravelTime_;
		unsigned int agentCount_;

		rdSegTravelTimes(double rdSegTravelTime, unsigned int agentCount)
		: rdSegTravelTime_(rdSegTravelTime), agentCount_(agentCount) {}
	};

	std::map<const RoadSegment*, rdSegTravelTimes> RdSegTravelTimesMap;
	void setRdSegTravelTimes(Person* ag, double rdSegExitTime);
	void resetRdSegTravelTimes(timeslice frameNumber);
	void reportRdSegTravelTimes(timeslice frameNumber);
	bool insertTravelTime2TmpTable(timeslice frameNumber,
			std::map<const RoadSegment*, sim_mob::Conflux::rdSegTravelTimes>& rdSegTravelTimesMap);
	//================ end of road segment travel time computation ========================

	double getPositionOfLastUpdatedAgentInLane(const Lane* lane);
	const Lane* getLaneInfinity(const RoadSegment* rdSeg);

	/**
	 * returns the time to reach the end of the link from a road segment on that
	 * link
	 * @param seg the road segment from which the time is to be measured
	 * @param distanceToEndOfSeg remaining distance in seg
	 */
	double computeTimeToReachEndOfLink(const sim_mob::RoadSegment* seg, double distanceToEndOfSeg);

	/**
	 * update the number of persons that can be added to the downstream confluxes
	 * from this conflux
	 */
	unsigned int resetOutputBounds();

	/**
	 * get a list of all persons in this conflux
	 */
	std::deque<sim_mob::Person*> getAllPersons();

	/**
	 * get a list of all persons in this conflux
	 * ORDER_BY_WHAT = 0: by location
	 * ORDER_BY_WHAT = 1: by time
	 */
	std::deque<sim_mob::Person*> getAllPersonsUsingTopCMerge();

	/**
	 * get number of persons in lane infinities of this conflux
	 */
	unsigned int getNumRemainingInLaneInfinity();

	/**
	 * determines if this conflux is connected to any conflux that belongs to
	 * another worker; also determines if all connected confluxes have the same
	 * worker or not
	 */
	void findBoundaryConfluxes();

	bool isBoundary; //A conflux that receives person from at least one conflux that belongs to another worker
	bool isMultipleReceiver; //A conflux that receives persons from confluxes that belong to multiple other workers
};

} /* namespace sim_mob */

