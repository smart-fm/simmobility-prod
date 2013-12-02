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

//Sort all agents in lane (based on remaining time this tick)
void sortPersons_DecreasingRemTime(std::deque<Person*> personList);

/**
 * Class representing an intersection along with the half-links (links are bidirectional. Half-link means one side
 * of the link which is unidirectional) which are upstream to the intersection. For all downstream half-links (which
 * conceptually belong to another Conflux), we maintain a temporary data structure.
 *
 * \author Harish Loganathan
 */
class Conflux : public sim_mob::Agent {

	friend class sim_mob::aimsun::Loader;

private:
	/**
	 *  MultiNode around which this conflux is constructed
	 */
	const sim_mob::MultiNode* multiNode;

	const sim_mob::Signal* signal;

	/**
	 *  convert factor from second to millisecond
	 */
	const float SECOND_MS;

	/**
	 * segments in this conflux (on upstream links)
	 * stores std::map<link which flows into the multinode, segments on the link>
	 */
	std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> > upstreamSegmentsMap;

	/**
	 * virtual queues are used to hold persons which come from previous conflux when the this conflux is not
	 * processed for the current tick. Each link in the conflux has a virtual queue
	 */
	std::map<sim_mob::Link*, std::deque<sim_mob::Person*> > virtualQueuesMap;

	/**
	 * keeps a pointer to a road segment on each link to keep track of the current segment that is being processed
	 */
	std::map<sim_mob::Link*, const sim_mob::RoadSegment*> currSegsOnUpLinks;

	/**
	 * segments on downstream links
	 * These links conceptually belong to the adjacent confluxes.
	 */
	std::set<const sim_mob::RoadSegment*> downstreamSegments;

	/**
	 *  Map to store the vehicle counts of each road segment on this conflux
	 */
	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*> segmentAgents;

	/**
	 *  Worker to which this conflux belongs to
	 */
	sim_mob::Worker* parentWorker;

	/**
	 * structure to store the frontal agents in each road segment
	 */
	std::map<const sim_mob::RoadSegment*, sim_mob::Person* > candidateAgents;

	/**
	 * cache the added lengths of road segments ahead in this link in this conflux
	 * E.g. If there are 3 consecutive segments A, B and C in a link and the end node of C is the end of the link
	 * this map stores (length-of-B+length-of-C) against A
	 */
	std::map<const sim_mob::RoadSegment*, double> lengthsOfSegmentsAhead;

	/**
	 * provide virtual queue protection
	 */
	boost::recursive_mutex mutexOfVirtualQueue;

	/**
	 * For each downstream link, this map stores the number of persons that can be
	 * accepted by that link in this conflux in this tick
	 */
	std::map<sim_mob::Link*, unsigned int> vqBounds;

	/**holds the current frame number for which this conflux is being processed*/
	timeslice currFrameNumber;

	std::deque<Person*> activityPerformers;

	/** function to call agents' updates if the MultiNode is signalized */
	void updateSignalized();

	/** function to call agents' updates if the MultiNode is not signalized */
	void updateUnsignalized();

	/** calls an Agent's update and does housekeeping for the conflux depending on the agent's new location */
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

	/** sets the iterators on currSegToUpLinks to the segments at the end of the half links*/
	void resetCurrSegsOnUpLinks();

	/** selects the agent closest to the intersection from candidateAgents;*/
	sim_mob::Person* agentClosestToIntersection();

	void killAgent(sim_mob::Person* ag, const sim_mob::RoadSegment* prevRdSeg, const sim_mob::Lane* prevLane, bool wasQueuing, bool wasActivityPerformer);

	void resetPersonRemTimesInVQ();

	//NOTE: New Agents use frame_* methods, but Conflux is fine just using update()
protected:
	virtual bool frame_init(timeslice now) { throw std::runtime_error("frame_* methods not supported for Confluxes."); }
	virtual Entity::UpdateStatus frame_tick(timeslice now) { throw std::runtime_error("frame_* methods not supported for Confluxes."); }
	virtual void frame_output(timeslice now) { throw std::runtime_error("frame_* methods not supported for Confluxes."); }

public:
	//constructors and destructor
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

	bool hasSpaceInVirtualQueue(sim_mob::Link* lnk);
	void pushBackOntoVirtualQueue(sim_mob::Link* lnk, sim_mob::Person* p);

	/**
	 * adds the agent into this conflux
	 */
	void addAgent(sim_mob::Person* ag, const sim_mob::RoadSegment* rdSeg);
	void addAgent(sim_mob::Person* ag, const sim_mob::Node* startingNode);

	/**
	 * get lanewise agent counts in a segment
	 */
	std::map<const sim_mob::Lane*, std::pair<unsigned int, unsigned int> > getLanewiseAgentCounts(const sim_mob::RoadSegment* rdSeg); //returns std::pair<queuingCount, movingCount>

	/**
	 * moving and queuing counts
	 */
	unsigned int numMovingInSegment(const sim_mob::RoadSegment* rdSeg, bool hasVehicle);
	unsigned int numQueueingInSegment(const sim_mob::RoadSegment* rdSeg, bool hasVehicle);

	/**Searches upstream and downstream segments to get the segmentStats for the requested road segment*/
	sim_mob::SegmentStats* findSegStats(const sim_mob::RoadSegment* rdSeg);

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
/*	void clearTravelTimesMap()
	{
		this->LinkTravelTimesMap.clear();
	}*/
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

	double computeTimeToReachEndOfLink(const sim_mob::RoadSegment* seg, double distanceToEndOfSeg);

	unsigned int resetOutputBounds();

	std::deque<sim_mob::Person*> getAllPersons();

	void findBoundaryConfluxes();

	unsigned int getNumRemainingInLaneInfinity();

	bool isBoundary; //A conflux that receives person from at least one conflux that belongs to another worker
	bool isMultipleReceiver; //A conflux that receives persons from confluxes that belong to multiple other workers
};

} /* namespace sim_mob */

