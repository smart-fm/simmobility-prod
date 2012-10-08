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

#include "MultiNode.hpp"
#include "entities/signal/Signal.hpp"
#include "util/SegmentVehicles.hpp"
#include "workers/Worker.hpp"

namespace sim_mob {

class RoadSegment;

namespace aimsun
{
//Forward declaration
class Loader;
}

class Conflux /*: sim_mob::Agent */ {

	friend class sim_mob::aimsun::Loader;
public:


private:
	// MultiNode around which this conflux is constructed
	sim_mob::MultiNode* multiNode;

	sim_mob::Signal* signal;

	/* segments in this conflux (on upstream links)
	 * All segments on half-links whose direction is flowing into the intersection */
	std::set<sim_mob::RoadSegment*> upstreamSegments;

	/* segments on downstream links
	 * These half-links conceptually belong to another conflux. */
	std::set<sim_mob::RoadSegment*> downstreamSegments;

	/* Map to store the vehicle counts of each road segment on this conflux */
	std::map<sim_mob::RoadSegment*, sim_mob::SegmentVehicles*> segmentAgents;

	/* This is a temporary storage data structure from which the agents would be moved to segmentAgents of
	 * another conflux during a flip (barrier synchronization). */
	std::map<sim_mob::RoadSegment*, sim_mob::SegmentVehicles*> segmentAgentsDownstream;

	/* Worker to which this conflux belongs to*/
	sim_mob::Worker* parentWorker;

public:
	//constructors and destructor
	Conflux() {};
	virtual ~Conflux() {};

	const sim_mob::MultiNode* getMultiNode() const {
		return multiNode;
	}

	// Getters
	const sim_mob::Signal* getSignal() const {
		return signal;
	}

	std::set<sim_mob::RoadSegment*> getDownstreamSegments() {
		return downstreamSegments;
	}

	std::set<sim_mob::RoadSegment*> getUpstreamSegments() {
		return upstreamSegments;
	}

	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentVehicles*> getSegmentAgents() const {
		return segmentAgents;
	}

	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentVehicles*> getSegmentAgentsDownstream() const {
		return segmentAgentsDownstream;
	}

	// adds the agent into this conflux (to segmentAgents list)
	void addAgent(sim_mob::Agent* ag);

	// adds this agent into segmentAgentsDownstream list
	void prepareAgentForHandover(sim_mob::Agent* ag);

	sim_mob::Worker* getParentWorker() const {
		return parentWorker;
	}

	void setParentWorker(sim_mob::Worker* parentWorker) {
		this->parentWorker = parentWorker;
	}

};

} /* namespace sim_mob */

