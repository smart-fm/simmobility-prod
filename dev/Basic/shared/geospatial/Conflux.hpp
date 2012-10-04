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

namespace sim_mob {

namespace aimsun
{
//Forward declaration
class Loader;
}

class Conflux /*: sim_mob::Agent */ {

	friend class sim_mob::aimsun::Loader;
public:
	Conflux();
	virtual ~Conflux();

private:
	// MultiNode around which this conflux is constructed
	sim_mob::MultiNode* multiNode;

	sim_mob::Signal* signal;

	/* link wise segment list in this conflux
	 * All segments on half-links whose direction is flowing into the intersection */
	std::map< const sim_mob::Link*, std::vector<sim_mob::RoadSegment*> > upstreamLinkSegments;

	/* link wise segment list representing downstream links
	 * These half-links conceptually belong to another conflux. */
	std::map< const sim_mob::Link*, std::vector<sim_mob::RoadSegment*> > downstreamLinkSegments;

	/* Map to store the vehicle counts of each road segment on this conflux */
	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentVehicles*> segmentAgents;

	/* This is a temporary storage data structure from which the agents would be moved to segmentAgents of
	 * another conflux during a flip (barrier synchronization). */
	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentVehicles*> segmentAgentsDownstream;
};

} /* namespace sim_mob */

