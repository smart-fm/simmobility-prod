/* Copyright Singapore-MIT Alliance for Research and Technology */

/* *
 * Class representing an intersection along with the half-links (links are bidirectional. Half-link means one side
 * of the link which is unidirectional) which are upstream to the intersection. For all downstream half-links (which
 * conceptually belong to another Conflux), we maintain a temporary data structure.
 *
 * Conflux.hpp
 *
 *  Created on: Oct 2, 2012
 *      Author: Harish Loganathan
 */
#pragma once

#include<map>
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "entities/signal/Signal.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/RoadSegment.hpp"
#include "SegmentStats.hpp"
#include "workers/Worker.hpp"
#include "buffering/Buffered.hpp"
#include "buffering/BufferedDataManager.hpp"


namespace sim_mob {

class RoadSegment;
class SegmentStats;

namespace aimsun
{
//Forward declaration
class Loader;
}

class Conflux : public sim_mob::Agent {

	friend class sim_mob::aimsun::Loader;

private:
	/* MultiNode around which this conflux is constructed */
	const sim_mob::MultiNode* multiNode;

	const sim_mob::Signal* signal;

	/* segments in this conflux (on upstream links)
	 * stores std::map< Link connected to the intersection, direction of the half-link which flows into the intersection>
	 */
	std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> > upstreamSegmentsMap;

	/* keeps a pointer to a road segment on each link to keep track of the current segment that is being processed*/
	std::map<sim_mob::Link*, const sim_mob::RoadSegment*> currSegsOnUpLinks;

	/* segments on downstream links
	 * These half-links conceptually belong to another conflux.
	 */
	std::set<const sim_mob::RoadSegment*> downstreamSegments;

	/* Map to store the vehicle counts of each road segment on this conflux */
	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*> segmentAgents;

	/* This is a temporary storage data structure from which the agents would be moved to segmentAgents of
	 * another conflux during a flip (barrier synchronization). */
	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*> segmentAgentsDownstream;

	/* Worker to which this conflux belongs to*/
	sim_mob::Worker* parentWorker;

	/*structure to store the frontal agents in each road segment*/
	std::map<const sim_mob::RoadSegment*, sim_mob::Person* > candidateAgents;

	/* cache the added lengths of road segments ahead in this link in this conflux
	 * E.g. If there are 3 consecutive segments A, B and C in a half-link and the end node of C is the end of the link
	 * this map stores (length-of-B+length-of-C) against A */
	std::map<const sim_mob::RoadSegment*, double> lengthsOfSegmentsAhead;

	timeslice currFrameNumber;

	std::vector<Entity*> toBeRemoved;

	void prepareLengthsOfSegmentsAhead();

	/* function to call agents' updates if the MultiNode is signalized */
	void updateSignalized();

	/* function to call agents' updates if the MultiNode is not signalized */
	void updateUnsignalized();

	/* calls an Agent's update and does housekeeping for the conflux depending on the agent's new location */
	void updateAgent(sim_mob::Person* p);

	/* calls frame_tick() of the movement facet for the person's role*/
	UpdateStatus perform_person_move(timeslice now, Person* person);

	/* calls frame_init of the movement facet for the person's role*/
	bool call_movement_frame_init(timeslice now, Person* person);

	/* calls frame_tick of the movement facet for the person's role*/
	Entity::UpdateStatus call_movement_frame_tick(timeslice now, Person* person);

	/* calls frame_tick of the movement facet for the person's role*/
	void call_movement_frame_output(timeslice now, Person* person);

	/* function to initialize candidate agents in each tick*/
	void initCandidateAgents();

	/* sets the iterators on currSegToUpLinks to the segments at the end of the half links*/
	void resetCurrSegsOnUpLinks();

	/* selects the agent closest to the intersection from candidateAgents;*/
	sim_mob::Person* agentClosestToIntersection();

	void killAgent(sim_mob::Person* ag, const sim_mob::RoadSegment* prevRdSeg, const sim_mob::Lane* prevLane);

	/*Searches segmentAgents and segmentAgentsDownstream to get the segmentStats for a road segment in this conflux*/
	sim_mob::SegmentStats* findSegStats(const sim_mob::RoadSegment* rdSeg);

public:
	//constructors and destructor
	Conflux(sim_mob::MultiNode* multinode, const MutexStrategy& mtxStrat, int id=-1)
		: Agent(mtxStrat, id), multiNode(multinode), signal(StreetDirectory::instance().signalAt(*multinode)),
		  parentWorker(nullptr), currFrameNumber(0,0), debugMsgs(std::stringstream::out) {};
	virtual ~Conflux() {};

	//Confluxes are non-spatial in nature.
	virtual bool isNonspatial() { return true; }

	// functions from agent
	virtual void load(const std::map<std::string, std::string>&) {}
	virtual Entity::UpdateStatus update(timeslice frameNumber);

	//NOTE: New Agents use frame_* methods, but Conflux is fine just using update()
protected:
	virtual bool frame_init(timeslice now) { throw std::runtime_error("frame_* methods not supported for Confluxes."); }
	virtual Entity::UpdateStatus frame_tick(timeslice now) { throw std::runtime_error("frame_* methods not supported for Confluxes."); }
	virtual void frame_output(timeslice now) { throw std::runtime_error("frame_* methods not supported for Confluxes."); }

public:

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

	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*> getSegmentAgentsDownstream() const {
		return segmentAgentsDownstream;
	}

	sim_mob::Worker* getParentWorker() const {
		return parentWorker;
	}

	void setParentWorker(sim_mob::Worker* parentWorker) {
		this->parentWorker = parentWorker;
	}

	// adds the agent into this conflux
	void addAgent(sim_mob::Person* ag);

	// get agent counts in a segment
	// lanewise
	std::map<const sim_mob::Lane*, std::pair<unsigned int, unsigned int> > getLanewiseAgentCounts(const sim_mob::RoadSegment* rdSeg); //returns std::pair<queuingCount, movingCount>

	// moving and queuing counts
	unsigned int numMovingInSegment(const sim_mob::RoadSegment* rdSeg, bool hasVehicle);
	unsigned int numQueueingInSegment(const sim_mob::RoadSegment* rdSeg, bool hasVehicle);

	void absorbAgentsAndUpdateCounts(sim_mob::SegmentStats* sourceSegStats);
	void handoverDownstreamAgents();

	double getOutputFlowRate(const Lane* lane);
	int getOutputCounter(const Lane* lane);
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
	void resetLinkTravelTimes(timeslice frameNumber);

	/* updates lane params for all lanes within the conflux */
	void updateAndReportSupplyStats(timeslice frameNumber);

	//TODO: To be removed after debugging.
	std::stringstream debugMsgs;

	//for mid-term link travel time computation for current frame tick
	struct travelTimes
	{
	public:
		unsigned int linkTravelTime_;
		unsigned int agentCount_;

		travelTimes(unsigned int linkTravelTime, unsigned int agentCount)
		: linkTravelTime_(linkTravelTime),
		  agentCount_(agentCount)
		{
		}
	};
	std::map<const Link*, travelTimes> LinkTravelTimesMap;
	void setTravelTimes(Person* ag, double linkExitTime);
	void clearTravelTimesMap()
	{
		this->LinkTravelTimesMap.clear();
	}
	void reportLinkTravelTimes(timeslice frameNumber);
};

} /* namespace sim_mob */

