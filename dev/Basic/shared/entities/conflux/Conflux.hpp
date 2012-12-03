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
#include "entities/signal/Signal.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/StreetDirectory.hpp"
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

class Conflux : sim_mob::Agent {

	friend class sim_mob::aimsun::Loader;
public:


private:
	/* MultiNode around which this conflux is constructed */
	const sim_mob::MultiNode* multiNode;

	const sim_mob::Signal* signal;

	/* segments in this conflux (on upstream links)
	 * stores std::map< Link connected to the intersection, direction of the half-link which flows into the intersection>
	 */
	std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> > upstreamSegmentsMap;

	/* keeps an iterator on each link to keep tract of the current segment that is being processed*/
	std::map<sim_mob::Link*, std::vector<sim_mob::RoadSegment*>::const_reverse_iterator > currSegsOnUpLinks;

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
	std::map<const sim_mob::RoadSegment*, sim_mob::Agent* > candidateAgents;

	/* cache the added lengths of road segments ahead in this link in this conflux
	 * E.g. If there are 3 consecutive segments A, B and C in a half-link and the end node of C is the end of the link
	 * this map stores (length-of-B+length-of-C) against A */
	std::map<const sim_mob::RoadSegment*, double> lengthsOfSegmentsAhead;

	frame_t currFrameNumber;

	void prepareLengthsOfSegmentsAhead();

	/* function to call agents' updates if the MultiNode is signalized */
	void updateSignalized();

	/* function to call agents' updates if the MultiNode is not signalized */
	void updateUnsignalized(frame_t frameNumber);

	/* calls an Agent's update and does housekeeping for the conflux depending on the agent's new location */
	void updateAgent(sim_mob::Agent* ag);

	/* function to initialize candidate agents in each tick*/
	void initCandidateAgents();

	/* sets the iterators on currSegToUpLinks to the segments at the end of the half links*/
	void resetCurrSegsOnUpLinks();

	/* selects the agent closest to the intersection from candidateAgents;*/
	sim_mob::Agent* agentClosestToIntersection();

	/* updates lane params for all lanes within the conflux */
	void updateSupplyStats(frame_t frameNumber);
	void reportSupplyStats(frame_t frameNumber);

public:
	//constructors and destructor
	Conflux(sim_mob::MultiNode* multinode, const MutexStrategy& mtxStrat, int id=-1)
		: Agent(mtxStrat, id), multiNode(multinode), signal(StreetDirectory::instance().signalAt(*multinode)),
		  parentWorker(nullptr), currFrameNumber(0), debugMsgs(std::stringstream::out) {};
	virtual ~Conflux() {};

	// functions from agent
	virtual void load(const std::map<std::string, std::string>&) {}
	virtual Entity::UpdateStatus update(frame_t frameNumber);

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

	// adds an agent who has just become active to this conflux
	void addStartingAgent(sim_mob::Agent* ag, sim_mob::RoadSegment* rdSeg);

	// adds the agent into this conflux (to segmentAgents list)
	void addAgent(sim_mob::Agent* ag);

	// adds this agent into segmentAgentsDownstream list
	void prepareAgentForHandover(sim_mob::Agent* ag);

	// get agent counts in a segment
	// lanewise
	std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > getLanewiseAgentCounts(const sim_mob::RoadSegment* rdSeg); //returns std::pair<queuingCount, movingCount>

	// moving and queuing counts
	unsigned int numMovingInSegment(const sim_mob::RoadSegment* rdSeg, bool hasVehicle);
	unsigned int numQueueingInSegment(const sim_mob::RoadSegment* rdSeg, bool hasVehicle);

	void absorbAgentsAndUpdateCounts(sim_mob::SegmentStats* agKeeper);

	double getOutputFlowRate(const Lane* lane);
	int getOutputCounter(const Lane* lane);
	double getAcceptRate(const Lane* lane);
	double getSegmentSpeed(const RoadSegment* rdSeg, bool hasVehicle);
	void updateSupplyStats(const Lane* lane, double newOutputFlowRate);
	void restoreSupplyStats(const Lane* lane);
	std::pair<unsigned int, unsigned int> getLaneAgentCounts(const sim_mob::Lane* lane); //returns std::pair<queuingCount, movingCount>
	unsigned int getInitialQueueCount(const Lane* lane);

	//TODO: To be removed after debugging.
	std::stringstream debugMsgs;

};

} /* namespace sim_mob */

