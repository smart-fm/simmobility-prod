/*
 * Conflux.cpp
 *
 *  Created on: Oct 2, 2012
 *      Author: harish
 */

#include<map>
#include "Conflux.hpp"

void sim_mob::Conflux::addStartingAgent(sim_mob::Agent* ag, sim_mob::RoadSegment* rdSeg) {
	/**
	 * For now, the agents always start at a node, i.e. the start of the road segment.
	 * So for now, we will always add the agent to the road segment in "lane infinity".
	 */
	sim_mob::SegmentVehicles* rdSegVehicles = segmentAgents.at(rdSeg);
	rdSegVehicles->laneInfinity.push(ag);
}

void sim_mob::Conflux::addAgent(sim_mob::Agent* ag) {
	throw std::runtime_error("Conflux::addAgent() not implemented");
}

void sim_mob::Conflux::prepareAgentForHandover(sim_mob::Agent* ag) {
	throw std::runtime_error("Conflux::prepareAgentForHandover() not implemented");
}
 /* namespace sim_mob */
