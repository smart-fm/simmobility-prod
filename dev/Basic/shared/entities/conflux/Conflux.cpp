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
	if (sim_mob::StreetDirectory::instance().signalAt(*multiNode) != nullptr) {
		updateSignalized();
	}
	else {
		updateUnsignalized(frameNumber);
	}
	return retVal;
}

void sim_mob::Conflux::updateSignalized() {
}

void sim_mob::Conflux::updateUnsignalized(frame_t frameNumber) {
	if(frameNumber != currFrameNumber) { // todo: think if this condition check is even necessary
		// this marks the start of next tick
		currFrameNumber = frameNumber;
		initCandidateAgents();
	}
	sim_mob::Agent* ag = nullptr;
	do {
		//ag = agentClosestToIntersection();
		//ag->update(); // and do follow up of return status from update
	} while (ag != nullptr);
}

void sim_mob::Conflux::initCandidateAgents() {
	resetCurrSegsOnUpLinks();
	typedef std::map<sim_mob::Link*, std::vector<sim_mob::RoadSegment*>::const_reverse_iterator >::iterator currSegsOnUpLinksIt;
	sim_mob::Link* lnk = nullptr;
	const sim_mob::RoadSegment* rdSeg = nullptr;
	for (currSegsOnUpLinksIt i = currSegsOnUpLinks.begin(); i != currSegsOnUpLinks.end(); i++) {
		lnk = (*i).first;
		do {
			rdSeg = *((*i).second);
			segmentAgents[rdSeg]->resetFrontalAgents();
			candidateAgents[rdSeg] = segmentAgents[rdSeg]->getNext();
			if(candidateAgents[rdSeg] == nullptr) {
				// this road segment is deserted. search the next (which is technically the previous).
				currSegsOnUpLinks[lnk]++;
			}
		} while (currSegsOnUpLinks[lnk] != upstreamSegmentsMap[lnk].rend() && candidateAgents[rdSeg] == nullptr);
	}
}

void sim_mob::Conflux::prepareAgentForHandover(sim_mob::Agent* ag) {
	throw std::runtime_error("Conflux::prepareAgentForHandover() not implemented");
}

std::map<sim_mob::Lane*, std::pair<int, int> > sim_mob::Conflux::getLanewiseAgentCounts(const sim_mob::RoadSegment* rdSeg) {
	return segmentAgents[rdSeg]->getAgentCountsOnLanes();
}

unsigned int sim_mob::Conflux::numMovingInSegment(
		const sim_mob::RoadSegment* rdSeg) {
	return segmentAgents[rdSeg]->numMovingInSegment();
}

void sim_mob::Conflux::resetCurrSegsOnUpLinks() {
	typedef std::vector<sim_mob::RoadSegment*>::const_reverse_iterator rdSegIt;
	for(std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> >::iterator i = upstreamSegmentsMap.begin();
			i != upstreamSegmentsMap.end(); i++) {
			currSegsOnUpLinks[(*i).first] = (*i).second.rbegin();
	}
}

unsigned int sim_mob::Conflux::numQueueingInSegment(const sim_mob::RoadSegment* rdSeg) {
	return segmentAgents[rdSeg]->numQueueingInSegment();
}

double sim_mob::Conflux::getOutputFlowRate(const Lane* lane) {
	return segmentAgents[lane->getRoadSegment()]->getSupplyStats(lane).outputFlowRate;
}

int sim_mob::Conflux::getOutputCounter(const Lane* lane) {
	return segmentAgents[lane->getRoadSegment()]->getSupplyStats(lane).outputCounter;
}

double sim_mob::Conflux::getAcceptRate(const Lane* lane) {
	return segmentAgents[lane->getRoadSegment()]->getSupplyStats(lane).acceptRate;
}

