/*
 * Conflux.cpp
 *
 *  Created on: Oct 2, 2012
 *      Author: harish
 */

#include<map>
#include "Conflux.hpp"
using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;

void sim_mob::Conflux::addStartingAgent(sim_mob::Agent* ag, sim_mob::RoadSegment* rdSeg) {
	/**
	 * The agents always start at a node (for now), i.e. the start of a road segment.
	 * So for now, we will always add the agent to the road segment in "lane infinity".
	 */
	sim_mob::AgentKeeper* rdSegVehicles = segmentAgents.at(rdSeg);
	rdSegVehicles->laneInfinity.push(ag);
}

void sim_mob::Conflux::addAgent(sim_mob::Agent* ag) {
	throw std::runtime_error("Conflux::addAgent() not implemented");
}

UpdateStatus sim_mob::Conflux::update(frame_t frameNumber) {
	UpdateStatus retVal(UpdateStatus::RS_DONE);
	/*sim_mob::StreetDirectory sd = sim_mob::StreetDirectory::instance();
	if (sd.signalAt(*multiNode) != nullptr) {
		updateSignalized();
	}
	else {
		updateUnsignalized();
	}*/
	return retVal;
}

void sim_mob::Conflux::updateSignalized() {
}

void sim_mob::Conflux::updateUnsignalized() {

}

void sim_mob::Conflux::prepareAgentForHandover(sim_mob::Agent* ag) {
	throw std::runtime_error("Conflux::prepareAgentForHandover() not implemented");
}
