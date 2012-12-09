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
	sim_mob::SegmentStats* rdSegStats = segmentAgents.at(rdSeg);
	rdSegStats->laneInfinity.push(ag);
}

void sim_mob::Conflux::addAgent(sim_mob::Agent* ag) {
	segmentAgents[ag->getCurrSegment()]->laneInfinity.push(ag);
}

UpdateStatus sim_mob::Conflux::update(timeslice frameNumber) {
	currFrameNumber = frameNumber;

	if (sim_mob::StreetDirectory::instance().signalAt(*multiNode) != nullptr) {
		updateUnsignalized(frameNumber); //TODO: Update Signalized must be implemented
	}
	else {
		updateUnsignalized(frameNumber);
	}
	updateSupplyStats(frameNumber);
	UpdateStatus retVal(UpdateStatus::RS_CONTINUE); //always return continue. Confluxes never die.
	return retVal;
}

void sim_mob::Conflux::updateSignalized() {
}

void sim_mob::Conflux::updateUnsignalized(timeslice frameNumber) {
//	debugMsgs << "\nUpdate " << frameNumber << " - Multinode :" << getMultiNode()->getID() << std::endl;
	initCandidateAgents();
	sim_mob::Agent* ag = agentClosestToIntersection();
	while (ag) {
		updateAgent(ag);
		// get next agent to update
		ag = agentClosestToIntersection();
	}
	std::cout << debugMsgs.str();
	debugMsgs.str(std::string());
}

void sim_mob::Conflux::updateAgent(sim_mob::Agent* ag) {
	debugMsgs << "\ncurrFrameNumber:" << currFrameNumber.frame()
			<< "\tag:" << ag->getId()
			<< "\tseg: [" << ag->getCurrSegment()->getStart()->getID() << "->" << ag->getCurrSegment()->getEnd()->getID() << "]"
			<< "\tDist:" << ag->distanceToEndOfSegment;

	const sim_mob::RoadSegment* segBeforeUpdate = ag->getCurrSegment();
	const sim_mob::Lane* laneBeforeUpdate = ag->getCurrLane();

	UpdateStatus res = ag->update(currFrameNumber);

	const sim_mob::RoadSegment* segAfterUpdate = ag->getCurrSegment();
	const sim_mob::Lane* laneAfterUpdate = ag->getCurrLane();

	if((segBeforeUpdate != segAfterUpdate) || (!laneBeforeUpdate)) {
		segmentAgents[segBeforeUpdate]->dequeue(laneBeforeUpdate);
		if(segmentAgents.find(segAfterUpdate) != segmentAgents.end()) {
//			debugMsgs << "\nAdding agent " << ag->getId() << " to lane " << laneAfterUpdate->getLaneID()
//						<< " of Segment [" << segAfterUpdate->getStart()->getID() << "->" << segAfterUpdate->getEnd()->getID() << "]";
//			debugMsgs << "\nBefore addAgent size(" << laneAfterUpdate->getLaneID() << ") = " << segmentAgents[segAfterUpdate]->getAgents(laneAfterUpdate).size();
			segmentAgents[segAfterUpdate]->addAgent(laneAfterUpdate, ag);
//			debugMsgs << "\nAfter addAgent size(" << laneAfterUpdate->getLaneID() << ") = " << segmentAgents[segAfterUpdate]->getAgents(laneAfterUpdate).size();
		}
		else if (segmentAgentsDownstream.find(segAfterUpdate) != segmentAgentsDownstream.end()) {
			segmentAgentsDownstream[segAfterUpdate]->addAgent(laneAfterUpdate, ag);
		}
	}

	if (res.status == UpdateStatus::RS_DONE) {
		//This agent is done. Remove from simulation. TODO: Check if this is valid for trip chaining. ~ Harish
		killAgent(ag);
	} else if (res.status == UpdateStatus::RS_CONTINUE) {
		// TODO: I think there will be nothing here. Have to make sure. ~ Harish
	} else {
		throw std::runtime_error("Unknown/unexpected update() return status.");
	}
}

double sim_mob::Conflux::getSegmentSpeed(const RoadSegment* rdSeg, bool hasVehicle){
	if (hasVehicle)
		return getSegmentAgents()[rdSeg]->getSegSpeed(hasVehicle);
	else
		return getSegmentAgents()[rdSeg]->getSegSpeed(hasVehicle);
}

void sim_mob::Conflux::initCandidateAgents() {
	resetCurrSegsOnUpLinks();
	typedef std::map<sim_mob::Link*, const sim_mob::RoadSegment* >::iterator currSegsOnUpLinksIt;
	sim_mob::Link* lnk = nullptr;
	const sim_mob::RoadSegment* rdSeg = nullptr;
	for (currSegsOnUpLinksIt i = currSegsOnUpLinks.begin(); i != currSegsOnUpLinks.end(); i++) {
		lnk = i->first;
		while (currSegsOnUpLinks[lnk]) {
			rdSeg = currSegsOnUpLinks[lnk];
			if(rdSeg == 0){
				throw std::runtime_error("Road Segment NULL");
			}
			segmentAgents[rdSeg]->resetFrontalAgents();
			candidateAgents[rdSeg] = segmentAgents[rdSeg]->getNext();
			if(!candidateAgents[rdSeg]) {
				// this road segment is deserted. search the next (which is, technically, the previous).
				const std::vector<sim_mob::RoadSegment*> segments = upstreamSegmentsMap.at(lnk);
				std::vector<sim_mob::RoadSegment*>::const_iterator rdSegIt = std::find(segments.begin(), segments.end(), rdSeg);
				if(rdSegIt != segments.begin()) {
					rdSegIt--;
					currSegsOnUpLinks.erase(lnk);
					currSegsOnUpLinks[lnk] = *rdSegIt;
				}
				else {
					currSegsOnUpLinks.erase(lnk);
					currSegsOnUpLinks[lnk] = nullptr;
				}
			}
			else { break; }
			std::cout << debugMsgs.str() << std::endl;
			debugMsgs.clear();
		}
	}
}

std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > sim_mob::Conflux::getLanewiseAgentCounts(const sim_mob::RoadSegment* rdSeg) {
	return segmentAgents[rdSeg]->getAgentCountsOnLanes();
}

unsigned int sim_mob::Conflux::numMovingInSegment(const sim_mob::RoadSegment* rdSeg, bool hasVehicle) {
	return segmentAgents[rdSeg]->numMovingInSegment(hasVehicle);
}

void sim_mob::Conflux::resetCurrSegsOnUpLinks() {
	currSegsOnUpLinks.clear();
	for(std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> >::iterator i = upstreamSegmentsMap.begin();
			i != upstreamSegmentsMap.end(); i++) {
		currSegsOnUpLinks.insert(std::make_pair(i->first, i->second.back()));
	}
}

sim_mob::Agent* sim_mob::Conflux::agentClosestToIntersection() {
	std::map<const sim_mob::RoadSegment*, sim_mob::Agent* >::iterator i = candidateAgents.begin();
	sim_mob::Agent* ag = nullptr;
	const sim_mob::RoadSegment* agRdSeg = nullptr;
	double minDistance = std::numeric_limits<double>::max();
	while(i != candidateAgents.end()) {
		if(i->second != nullptr) {
			if(minDistance == (i->second->distanceToEndOfSegment + lengthsOfSegmentsAhead[i->first])) {
				// If current ag and (*i) are at equal distance to the stop line, we toss a coin and choose one of them
				bool coinTossResult = ((rand() / (double)RAND_MAX) < 0.5);
				if(coinTossResult) {
					agRdSeg = i->first;
					ag = i->second;
				}
			}
			else if (minDistance > (i->second->distanceToEndOfSegment + lengthsOfSegmentsAhead[i->first])) {
				minDistance = i->second->distanceToEndOfSegment + lengthsOfSegmentsAhead[i->first];
				agRdSeg = i->first;
				ag = i->second;
			}
		}
		i++;
	}
	if(ag) {
		candidateAgents.erase(agRdSeg);
		const std::vector<sim_mob::RoadSegment*> segments = agRdSeg->getLink()->getSegments();
		sim_mob::Agent* nextAg = segmentAgents[agRdSeg]->getNext();
		std::vector<sim_mob::RoadSegment*>::const_iterator rdSegIt = std::find(segments.begin(), segments.end(), agRdSeg);
		while(!nextAg && rdSegIt != segments.begin()){
			//debugMsgs << "\nConfluxMN:" << multiNode->getID() << "\t[" << (*rdSegIt)->getStart()->getID() << "->" << (*rdSegIt)->getEnd()->getID() << "] is deserted. ";
			rdSegIt--;
			//debugMsgs << "Looking in [" << (*rdSegIt)->getStart()->getID() << "->" << (*rdSegIt)->getEnd()->getID() << "]";
			nextAg = segmentAgents[*rdSegIt]->getNext();
			//std::cout << debugMsgs.str() << std::endl;
			//debugMsgs.clear();
		}
		candidateAgents[agRdSeg] = nextAg;
	}
	return ag;
}

void sim_mob::Conflux::prepareLengthsOfSegmentsAhead() {
	for(std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> >::iterator i = upstreamSegmentsMap.begin();
				i != upstreamSegmentsMap.end(); i++)
	{
		for(std::vector<sim_mob::RoadSegment*>::const_iterator j = i->second.begin();
				j != i->second.end(); j++)
		{
			double lengthAhead = 0.0;
			for(std::vector<sim_mob::RoadSegment*>::const_iterator k = j+1;
					k != i->second.end(); k++)
			{
				lengthAhead = lengthAhead + (*k)->computeLaneZeroLength();
			}
			lengthsOfSegmentsAhead.insert(std::make_pair(*j, lengthAhead));
		}
	}
}

unsigned int sim_mob::Conflux::numQueueingInSegment(const sim_mob::RoadSegment* rdSeg,
		bool hasVehicle) {
	std::cout << "rdSeg in Conflux: "<<rdSeg <<std::endl;
	return segmentAgents[rdSeg]->numQueueingInSegment(hasVehicle);
}

double sim_mob::Conflux::getOutputFlowRate(const Lane* lane) {
	return segmentAgents[lane->getRoadSegment()]->getLaneParams(lane)->getOutputFlowRate();
}

int sim_mob::Conflux::getOutputCounter(const Lane* lane) {
	return segmentAgents[lane->getRoadSegment()]->getLaneParams(lane)->getOutputCounter();
}

void sim_mob::Conflux::absorbAgentsAndUpdateCounts(sim_mob::SegmentStats* sourceSegStats) {
	segmentAgents[sourceSegStats->getRoadSegment()]->absorbAgents(sourceSegStats);
	std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > laneCounts = segmentAgents[sourceSegStats->getRoadSegment()]->getAgentCountsOnLanes();
	sourceSegStats->setPrevTickLaneCountsFromOriginal(laneCounts);
	sourceSegStats->clear();
}

double sim_mob::Conflux::getAcceptRate(const Lane* lane) {
	return segmentAgents[lane->getRoadSegment()]->getLaneParams(lane)->getAcceptRate();
}

void sim_mob::Conflux::updateSupplyStats(const Lane* lane, double newOutputFlowRate) {
	segmentAgents[lane->getRoadSegment()]->updateLaneParams(lane, newOutputFlowRate);
}

void sim_mob::Conflux::restoreSupplyStats(const Lane* lane) {
	segmentAgents[lane->getRoadSegment()]->restoreLaneParams(lane);
}

void sim_mob::Conflux::updateSupplyStats(timeslice frameNumber) {
	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*>::iterator it = segmentAgents.begin();
	for( ; it != segmentAgents.end(); ++it )
	{
		(it->second)->updateLaneParams(frameNumber);
		(it->second)->reportSegmentStats(frameNumber);
	}
}

std::pair<unsigned int, unsigned int> sim_mob::Conflux::getLaneAgentCounts(
		const sim_mob::Lane* lane) {
	return segmentAgents[lane->getRoadSegment()]->getLaneAgentCounts(lane);
}

unsigned int sim_mob::Conflux::getInitialQueueCount(const Lane* lane) {
	return segmentAgents[lane->getRoadSegment()]->getInitialQueueCount(lane);
}

void sim_mob::Conflux::killAgent(sim_mob::Agent* ag) {
	segmentAgents[ag->getCurrSegment()]->removeAgent(ag->getCurrLane(), ag);
}

void sim_mob::Conflux::handoverDownstreamAgents() {
	for(std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*>::iterator i = segmentAgentsDownstream.begin();
			i != segmentAgentsDownstream.end(); i++)
	{
		i->second->getRoadSegment()->getParentConflux()->absorbAgentsAndUpdateCounts(i->second);
	}
}
