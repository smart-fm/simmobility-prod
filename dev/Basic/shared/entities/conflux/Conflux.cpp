/*
 * Conflux.cpp
 *
 *  Created on: Oct 2, 2012
 *      Author: harish
 */

#include "Conflux.hpp"

#include <map>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include "conf/simpleconf.hpp"
#include "entities/Person.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/RoadSegment.hpp"
#include "logging/Log.hpp"
#include "util/Utils.hpp"
#include "workers/Worker.hpp"


using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;


sim_mob::Conflux::Conflux(sim_mob::MultiNode* multinode, const MutexStrategy& mtxStrat, int id)
	: Agent(mtxStrat, id),
	  multiNode(multinode), signal(StreetDirectory::instance().signalAt(*multinode)),
	  parentWorker(nullptr), currFrameNumber(0,0), debugMsgs(std::stringstream::out)
{
}

sim_mob::Conflux::~Conflux()
{
	for(std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*>::iterator i=segmentAgents.begin(); i!=segmentAgents.end(); i++) {
		safe_delete_item(i->second);
	}
}


void sim_mob::Conflux::addAgent(sim_mob::Person* ag, const sim_mob::RoadSegment* rdSeg) {
	/**
	 * The agents always start at a node (for now).
	 * we will always add the Person to the road segment in "lane infinity".
	 */
	sim_mob::SegmentStats* rdSegStats = segmentAgents.find(rdSeg)->second;
	ag->setCurrSegment(rdSeg);
	ag->setCurrLane(rdSegStats->laneInfinity);
	ag->distanceToEndOfSegment = rdSeg->computeLaneZeroLength();
	ag->remainingTimeThisTick = ConfigParams::GetInstance().baseGranMS / 1000.0;
	rdSegStats->addAgent(rdSegStats->laneInfinity, ag);
}

UpdateStatus sim_mob::Conflux::update(timeslice frameNumber) {
	timeval startTime;
	gettimeofday(&startTime, nullptr);

	currFrameNumber = frameNumber;

	resetPositionOfLastUpdatedAgentOnLanes();

	//reset the remaining times of persons in lane infinity if required.
	resetPersonRemTimesInVQ();

	timeval startUpdateTime;
	gettimeofday(&startUpdateTime, nullptr);

	Print() << "Conflux: " << multiNode->getID() << "|Frame: " << frameNumber.frame()
			<< " execution time before updateUnsignalized: "<< Utils::diff_ms(startUpdateTime, startTime)
			<< " ms." << std::endl;

	if (sim_mob::StreetDirectory::instance().signalAt(*multiNode) != nullptr) {
		updateUnsignalized(); //TODO: Update Signalized must be implemented and called here
	}
	else {
		updateUnsignalized();
	}

	timeval endUpdateTime;
	gettimeofday(&endUpdateTime, nullptr);

	Print() << "Conflux: " << multiNode->getID() << "|Frame: " << frameNumber.frame()
			<< " execution time for updateUnsignalized: "<< Utils::diff_ms(endUpdateTime,startUpdateTime)
			<< " ms." << std::endl;

	UpdateStatus retVal(UpdateStatus::RS_CONTINUE); //always return continue. Confluxes never die.
	setLastUpdatedFrame(frameNumber.frame());
	return retVal;
}

void sim_mob::Conflux::updateSignalized() {
	throw std::runtime_error("Conflux::updateSignalized() not implemented yet.");
}

void sim_mob::Conflux::updateUnsignalized() {
	initCandidateAgents();
	sim_mob::Person* person = agentClosestToIntersection();
	while (person) {
		updateAgent(person);
		person = agentClosestToIntersection(); // get next Person to update
	}
}

void sim_mob::Conflux::updateAgent(sim_mob::Person* person) {
	if (person->getLastUpdatedFrame() < currFrameNumber.frame()) {
		//if the person is moved for the first time in this tick
		person->remainingTimeThisTick = ConfigParams::GetInstance().baseGranMS / 1000.0;
	}
	person->currWorkerProvider = parentWorker; // Let the person know which worker managing him... for logs to work.
	const sim_mob::RoadSegment* segBeforeUpdate = person->getCurrSegment();
	const sim_mob::Lane* laneBeforeUpdate = person->getCurrLane();
	bool isQueuingBeforeUpdate = person->isQueuing;
	sim_mob::SegmentStats* segStatsBfrUpdt = findSegStats(segBeforeUpdate);

	if(segBeforeUpdate->getParentConflux() != this) {
		Print() << "segBeforeUpdate not in the current conflux|segBeforeUpdate's conflux is " << segBeforeUpdate->getParentConflux()->getMultiNode()->getID() << std::endl;
		throw std::runtime_error("segBeforeUpdate not in the current conflux");
	}

	UpdateStatus res = perform_person_move(currFrameNumber, person);

	if (res.status == UpdateStatus::RS_DONE) {
		//This Person is done. Remove from simulation.
		killAgent(person, segBeforeUpdate, laneBeforeUpdate, isQueuingBeforeUpdate);
		Print()<<"Person "<< person->getId() << " is removed from the simulation." <<std::endl;
		return;
	} else if (res.status == UpdateStatus::RS_CONTINUE) {
		// TODO: I think there will be nothing here. Have to make sure. ~ Harish
	} else {
		throw std::runtime_error("Unknown/unexpected update() return status.");
	}

	const sim_mob::RoadSegment* segAfterUpdate = person->getCurrSegment();
	const sim_mob::Lane* laneAfterUpdate = person->getCurrLane();
	bool isQueuingAfterUpdate = person->isQueuing;
	sim_mob::SegmentStats* segStatsAftrUpdt = findSegStats(segAfterUpdate);

	if (!laneBeforeUpdate) /*If the person was in virtual queue*/ {
		if(laneAfterUpdate) /*If the person has moved to another lane in some segment*/ {
			Print()<<"laneAfterUpdate: "<<laneAfterUpdate->getLaneID()<<std::endl;
			Print()<<"segAftrUpdt: "<<segAfterUpdate->getSegmentID()<<std::endl;
			segStatsAftrUpdt->addAgent(laneAfterUpdate, person);
			Print() << "Frame:" << currFrameNumber.frame() << "|updateAgent()|Conflux:" << this->multiNode->getID() << "|Person moved out of VQ:" << person->getId() << std::endl;
		}
		else  {
			if (segStatsBfrUpdt != segStatsAftrUpdt) {
				/*the person has moved to another virtual queue - which is not possible if the virtual queues are processed at the end*/
				debugMsgs
						<< "Error: Person has moved from one virtual queue to another. "
						<< "\n Person " << person->getId()
						<< "|Frame: " << currFrameNumber.frame()
						<< "|segBeforeUpdate: " << segBeforeUpdate->getStartEnd()
						<< "|segStatsAftrUpdt: " << segAfterUpdate->getStartEnd();
				Print() << debugMsgs.str();
				throw std::runtime_error(debugMsgs.str());
			}
			else {
				/* This is typically the person who was not accepted by the next lane in the next segment.
				 * We push this person back to the same virtual queue and let him update in the next tick.
				 */
				person->distanceToEndOfSegment = segAfterUpdate->computeLaneZeroLength();
				Print() << "Person " << person->getId() << " is pushed to VQ of Conflux " << segAfterUpdate->getParentConflux()->getMultiNode()->getID() << "|link " << segAfterUpdate->getStartEnd() << std::endl;
				segAfterUpdate->getParentConflux()->pushBackOntoVirtualQueue(segAfterUpdate->getLink(), person);
			}
		}
	}
	else if((segBeforeUpdate != segAfterUpdate) /*if the person has moved to another segment*/
			|| (laneBeforeUpdate == segStatsBfrUpdt->laneInfinity && laneBeforeUpdate != laneAfterUpdate) /* or if the person has moved out of lane infinity*/)
	{
		Person* dequeuedPerson = segStatsBfrUpdt->dequeue(person, laneBeforeUpdate, isQueuingBeforeUpdate);
	//	Person* dequeuedPerson = segStatsBfrUpdt->dequeue(laneBeforeUpdate, isQueuingBeforeUpdate);
		if(dequeuedPerson != person) {
			Print()<< "Error: Dequeued Person " << dequeuedPerson->getId() << " Person " << person->getId()<<std::endl;
			segStatsBfrUpdt->printAgents();
			debugMsgs << "Error: Person " << dequeuedPerson->getId() << " dequeued instead of Person " << person->getId()
					<< "\n Person " << person->getId() << ": segment: " << segBeforeUpdate->getStartEnd()
					<< "|lane: " << laneBeforeUpdate->getLaneID()
					<< "|Frame: " << currFrameNumber.frame()
					<< "\n Person " << dequeuedPerson->getId() << ": "
					<< "segment: " << dequeuedPerson->getCurrSegment()->getStartEnd()
					<< "|lane: " << dequeuedPerson->getCurrLane()->getLaneID()
					<< "|Frame: " << dequeuedPerson->getLastUpdatedFrame();

			Print() << debugMsgs.str();
			throw std::runtime_error(debugMsgs.str());
		}

		if(laneAfterUpdate) {
			segStatsAftrUpdt->addAgent(laneAfterUpdate, person);
		}
		else {
			/* We wouldn't know which lane the person has to go to if the person wants to enter a link
			 * which belongs to a conflux that is not processed for this tick yet.
			 * We add this person to the virtual queue for that link here */
			person->distanceToEndOfSegment = segAfterUpdate->computeLaneZeroLength();
			Print() << "Person " << person->getId() << " is pushed to VQ of Conflux " << segAfterUpdate->getParentConflux()->getMultiNode()->getID() << "|link " << segAfterUpdate->getStartEnd() << std::endl;
			segAfterUpdate->getParentConflux()->pushBackOntoVirtualQueue(segAfterUpdate->getLink(), person);
		}
	}
	else if (isQueuingBeforeUpdate != isQueuingAfterUpdate) {
		segStatsAftrUpdt->updateQueueStatus(laneAfterUpdate, person);
	}

	// set the position of the last updated Person in his current lane (after update)
	segStatsAftrUpdt->setPositionOfLastUpdatedAgentInLane(person->distanceToEndOfSegment, person->getCurrLane());
}

void sim_mob::Conflux::processVirtualQueues() {
	int counter = 0;
	//sort the virtual queues before starting to move agents for this tick
	for(std::map<sim_mob::Link*, std::deque<sim_mob::Person*> >::iterator i = virtualQueuesMap.begin(); i!=virtualQueuesMap.end(); i++) {
		counter = i->second.size();
		sortPersons_DecreasingRemTime(i->second);
		Print() << "Frame:" << currFrameNumber.frame() << "|processVirtualQueues()|Conflux:" << this->multiNode->getID() << "|VQ size:" << i->second.size() << std::endl;
		while(counter > 0){
			sim_mob::Person* p = i->second.front();
			i->second.pop_front();
			Print() << "processVirtualQueues()|Before update|Person: "<<p->getId()
					<< "|segment: "<<p->getCurrSegment()->getStartEnd()
					<< "|segID: "<<p->getCurrSegment()->getSegmentID()<<std::endl;
			updateAgent(p);
			counter--;
		}
	}
}

double sim_mob::Conflux::getSegmentSpeed(const RoadSegment* rdSeg, bool hasVehicle){
	if (hasVehicle){
		return findSegStats(rdSeg)->getSegSpeed(hasVehicle);
	}
	//else pedestrian lanes are not handled
	return 0.0;
}

void sim_mob::Conflux::initCandidateAgents() {
	candidateAgents.clear();
	resetCurrSegsOnUpLinks();

	sim_mob::Link* lnk = nullptr;
	const sim_mob::RoadSegment* rdSeg = nullptr;
	for (std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> >::iterator i = upstreamSegmentsMap.begin(); i != upstreamSegmentsMap.end(); i++) {
		lnk = i->first;
		while (currSegsOnUpLinks.at(lnk)) {
			rdSeg = currSegsOnUpLinks.at(lnk);
			segmentAgents.at(rdSeg)->resetFrontalAgents();
			candidateAgents.insert(std::make_pair(rdSeg, segmentAgents.at(rdSeg)->agentClosestToStopLineFromFrontalAgents()));
			if(!candidateAgents.at(rdSeg)) {
				// this road segment is deserted. search the next (which is, technically, the previous).
				const std::vector<sim_mob::RoadSegment*> segments = i->second; // or upstreamSegmentsMap.at(lnk);
				std::vector<sim_mob::RoadSegment*>::const_iterator rdSegIt = std::find(segments.begin(), segments.end(), rdSeg);
				currSegsOnUpLinks.erase(lnk);
				if(rdSegIt != segments.begin()) {
					rdSegIt--;
					currSegsOnUpLinks.insert(std::make_pair(lnk, *rdSegIt));
				}
				else {
					const sim_mob::RoadSegment* nullSeg = nullptr;
					currSegsOnUpLinks.insert(std::make_pair(lnk, nullSeg)); // No agents in the entire link
				}
			}
			else { break; }
		}
	}
}

std::map<const sim_mob::Lane*, std::pair<unsigned int, unsigned int> > sim_mob::Conflux::getLanewiseAgentCounts(const sim_mob::RoadSegment* rdSeg) {
	return findSegStats(rdSeg)->getAgentCountsOnLanes();
}

unsigned int sim_mob::Conflux::numMovingInSegment(const sim_mob::RoadSegment* rdSeg, bool hasVehicle) {
	return findSegStats(rdSeg)->numMovingInSegment(hasVehicle);
}

void sim_mob::Conflux::resetCurrSegsOnUpLinks() {
	currSegsOnUpLinks.clear();
	for(std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> >::iterator i = upstreamSegmentsMap.begin(); i != upstreamSegmentsMap.end(); i++) {
		currSegsOnUpLinks.insert(std::make_pair(i->first, i->second.back()));
	}
}

/*
 * This function resets the remainingTime of persons who remain in lane infinity for more than 1 tick.
 * Note: This may include
 * 1. newly starting persons who (were supposed to, but) did not get added to the simulation
 * in the previous tick due to traffic congestion in their starting segment.
 * 2. Persons who got added to and remained virtual queue in the previous tick
 */
void sim_mob::Conflux::resetPersonRemTimesInVQ() {
	SegmentStats* segStats = nullptr;
	for(std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> >::iterator upStrmSegMapIt = upstreamSegmentsMap.begin(); upStrmSegMapIt!=upstreamSegmentsMap.end(); upStrmSegMapIt++) {
		for(std::vector<sim_mob::RoadSegment*>::const_iterator rdSegIt=upStrmSegMapIt->second.begin(); rdSegIt!=upStrmSegMapIt->second.end(); rdSegIt++) {
			segStats = findSegStats(*rdSegIt);
			std::deque<sim_mob::Person*> personsInLaneInfinity = segStats->getAgents(segStats->laneInfinity);
			for(std::deque<sim_mob::Person*>::iterator personIt=personsInLaneInfinity.begin(); personIt!=personsInLaneInfinity.end(); personIt++) {
				if ((*personIt)->getLastUpdatedFrame() < currFrameNumber.frame()) {
					//if the person is going to be moved for the first time in this tick
					(*personIt)->remainingTimeThisTick = ConfigParams::GetInstance().baseGranMS / 1000.0;
				}
			}
		}
	}

	for(std::map<sim_mob::Link*, std::deque<sim_mob::Person*> >::iterator vqIt=virtualQueuesMap.begin(); vqIt!=virtualQueuesMap.end();vqIt++) {
		for(std::deque<sim_mob::Person*>::iterator pIt= vqIt->second.begin(); pIt!=vqIt->second.end(); pIt++) {
			if ((*pIt)->getLastUpdatedFrame() < currFrameNumber.frame()) {
				//if the person is going to be moved for the first time in this tick
				(*pIt)->remainingTimeThisTick = ConfigParams::GetInstance().baseGranMS / 1000.0;
			}
		}
	}
}

void sim_mob::Conflux::buildSubscriptionList(std::vector<BufferedBase*>& subsList) {
	Agent::buildSubscriptionList(subsList);
}

void sim_mob::Conflux::resetOutputBounds() {
	vqBounds.clear();
	sim_mob::Link* lnk = nullptr;
	const sim_mob::RoadSegment* firstSeg = nullptr;
	sim_mob::SegmentStats* segStats = nullptr;
	int outputEstimate = 0;
	for(std::map<sim_mob::Link*, std::deque<sim_mob::Person*> >::iterator i = virtualQueuesMap.begin(); i != virtualQueuesMap.end(); i++) {
		lnk = i->first;
		segStats = findSegStats(lnk->getSegments().front());
		outputEstimate = segStats->computeExpectedOutputPerTick();
		outputEstimate = outputEstimate - virtualQueuesMap.at(lnk).size(); // decrement num. of agents already in virtual queue
		outputEstimate = (outputEstimate>0? outputEstimate : 0);
		vqBounds.insert(std::make_pair(lnk, (unsigned int)outputEstimate));
	}
}

bool sim_mob::Conflux::hasSpaceInVirtualQueue(sim_mob::Link* lnk) {
	return (vqBounds.at(lnk) > virtualQueuesMap.at(lnk).size());
}

void sim_mob::Conflux::pushBackOntoVirtualQueue(sim_mob::Link* lnk, sim_mob::Person* p) {
	virtualQueuesMap.at(lnk).push_back(p);
}

double sim_mob::Conflux::computeTimeToReachEndOfLink(const sim_mob::RoadSegment* seg, double distanceToEndOfSeg) {
	sim_mob::Link* link = seg->getLink();
	const std::vector<sim_mob::RoadSegment*> segments = link->getSegments();
	std::vector<sim_mob::RoadSegment*>::const_iterator rdSegIt = std::find(segments.begin(), segments.end(), seg);

	sim_mob::SegmentStats* segStats = findSegStats(seg);
	double timeToReachEndOfLink = distanceToEndOfSeg * getSegmentSpeed(seg,true);
	for(std::vector<sim_mob::RoadSegment*>::const_iterator i = rdSegIt+1; i!=segments.end(); i++) {
		timeToReachEndOfLink += (*i)->computeLaneZeroLength() * getSegmentSpeed((*i),true);
	}
	return timeToReachEndOfLink;
}

/* unused version based on remaining time at intersection
sim_mob::Person* sim_mob::Conflux::agentClosestToIntersection() {
	sim_mob::Person* ag = nullptr;
	const sim_mob::RoadSegment* agRdSeg = nullptr;
	double maxRemainingTimeAtIntersection = std::numeric_limits<double>::min();
	double tickSize = ConfigParams::GetInstance().baseGranMS / 1000.0;

	std::map<const sim_mob::RoadSegment*, sim_mob::Person*>::iterator i = candidateAgents.begin();
	while (i != candidateAgents.end()) {
		if (i->second != nullptr) {
			if (maxRemainingTimeAtIntersection == i->second->remainingTimeAtIntersection) {
				// If current ag and (*i) are at equal distance to the intersection (end of the link), we toss a coin and choose one of them
				bool coinTossResult = ((rand() / (double) RAND_MAX) < 0.5);
				if (coinTossResult) {
					agRdSeg = i->first;
					ag = i->second;
				}
			}
			else if (maxRemainingTimeAtIntersection < i->second->remainingTimeAtIntersection) {
				maxRemainingTimeAtIntersection = i->second->distanceToEndOfSegment + lengthsOfSegmentsAhead.at(i->first);
				agRdSeg = i->first;
				ag = i->second;
			}
		}
		i++;
	}
	if (ag) {
		candidateAgents.erase(agRdSeg);
		const std::vector<sim_mob::RoadSegment*> segments = agRdSeg->getLink()->getSegments();
		std::vector<sim_mob::RoadSegment*>::const_iterator rdSegIt = std::find(segments.begin(), segments.end(), agRdSeg);
		sim_mob::Person* nextAg = segmentAgents.at(*rdSegIt)->agentClosestToStopLineFromFrontalAgents();
		while (!nextAg && rdSegIt != segments.begin()) {
			currSegsOnUpLinks.erase((*rdSegIt)->getLink());
			rdSegIt--;
			currSegsOnUpLinks.insert(std::make_pair((*rdSegIt)->getLink(), *rdSegIt)); // No agents in the entire link
			segmentAgents.at(*rdSegIt)->resetFrontalAgents();
			nextAg = segmentAgents.at(*rdSegIt)->agentClosestToStopLineFromFrontalAgents();
		}
		candidateAgents.insert(std::make_pair(*rdSegIt, nextAg));
	}
	return ag;
}*/

sim_mob::Person* sim_mob::Conflux::agentClosestToIntersection() {
	sim_mob::Person* ag = nullptr;
	const sim_mob::RoadSegment* agRdSeg = nullptr;
	double minTime = std::numeric_limits<double>::max();
	double timeToReachEndOfLink = 0;
	std::map<const sim_mob::RoadSegment*, sim_mob::Person*>::iterator i = candidateAgents.begin();
	while (i != candidateAgents.end()) {
		if (i->second != nullptr) {
			timeToReachEndOfLink = computeTimeToReachEndOfLink(i->first, i->second->distanceToEndOfSegment);
			if (minTime == timeToReachEndOfLink) {
				// If current ag and (*i) are at equal distance to the intersection (end of the link), we toss a coin and choose one of them
				bool coinTossResult = ((rand() / (double) RAND_MAX) < 0.5);
				if (coinTossResult) {
					agRdSeg = i->first;
					ag = i->second;
				}
			} else if (minTime > timeToReachEndOfLink) {
				minTime = timeToReachEndOfLink;
				agRdSeg = i->first;
				ag = i->second;
			}
		}
		i++;
	}
	if (ag) {
		candidateAgents.erase(agRdSeg);
		const std::vector<sim_mob::RoadSegment*> segments = agRdSeg->getLink()->getSegments();
		std::vector<sim_mob::RoadSegment*>::const_iterator rdSegIt = std::find(segments.begin(), segments.end(), agRdSeg);
		sim_mob::Person* nextAg = segmentAgents.at(*rdSegIt)->agentClosestToStopLineFromFrontalAgents();
		while (!nextAg && rdSegIt != segments.begin()) {
			currSegsOnUpLinks.erase((*rdSegIt)->getLink());
			rdSegIt--;
			currSegsOnUpLinks.insert(std::make_pair((*rdSegIt)->getLink(), *rdSegIt)); // No agents in the entire link
			segmentAgents.at(*rdSegIt)->resetFrontalAgents();
			nextAg = segmentAgents.at(*rdSegIt)->agentClosestToStopLineFromFrontalAgents();
		}
		candidateAgents.insert(std::make_pair(*rdSegIt, nextAg));
	}
	return ag;
}

unsigned int sim_mob::Conflux::numQueueingInSegment(const sim_mob::RoadSegment* rdSeg, bool hasVehicle) {
	return findSegStats(rdSeg)->numQueueingInSegment(hasVehicle);
}

double sim_mob::Conflux::getOutputFlowRate(const Lane* lane) {
	return findSegStats(lane->getRoadSegment())->getLaneParams(lane)->getOutputFlowRate();
}

int sim_mob::Conflux::getOutputCounter(const Lane* lane) {
	return findSegStats(lane->getRoadSegment())->getLaneParams(lane)->getOutputCounter();
}

void sim_mob::Conflux::setOutputCounter(const Lane* lane, int count) {
	findSegStats(lane->getRoadSegment())->getLaneParams(lane)->setOutputCounter(count);
}

double sim_mob::Conflux::getAcceptRate(const Lane* lane) {
	return findSegStats(lane->getRoadSegment())->getLaneParams(lane)->getAcceptRate();
}

void sim_mob::Conflux::updateSupplyStats(const Lane* lane, double newOutputFlowRate) {
	findSegStats(lane->getRoadSegment())->updateLaneParams(lane, newOutputFlowRate);
}

void sim_mob::Conflux::restoreSupplyStats(const Lane* lane) {
	findSegStats(lane->getRoadSegment())->restoreLaneParams(lane);
}

void sim_mob::Conflux::updateAndReportSupplyStats(timeslice frameNumber) {
	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*>::iterator it = segmentAgents.begin();
	for( ; it != segmentAgents.end(); ++it )
	{
		(it->second)->updateLaneParams(frameNumber);
		if (ConfigParams::GetInstance().OutputEnabled()) {
			Log() <<(it->second)->reportSegmentStats(frameNumber);
		}
	}
}

std::pair<unsigned int, unsigned int> sim_mob::Conflux::getLaneAgentCounts(const sim_mob::Lane* lane) {
	return findSegStats(lane->getRoadSegment())->getLaneAgentCounts(lane);
}

unsigned int sim_mob::Conflux::getInitialQueueCount(const Lane* lane) {
	return findSegStats(lane->getRoadSegment())->getInitialQueueCount(lane);
}

void sim_mob::Conflux::killAgent(sim_mob::Person* ag, const sim_mob::RoadSegment* prevRdSeg, const sim_mob::Lane* prevLane, bool wasQueuing) {
	if (prevLane) {
		findSegStats(prevRdSeg)->removeAgent(prevLane, ag, wasQueuing);
	} /*else the person must have started from a VQ*/
	Print() << "Killing " << ag->getId() << std::endl;
	parentWorker->remEntity(ag);
	parentWorker->scheduleForRemoval(ag);
}

double sim_mob::Conflux::getLastAccept(const Lane* lane) {
	return findSegStats(lane->getRoadSegment())->getLaneParams(lane)->getLastAccept();
}

void sim_mob::Conflux::setLastAccept(const Lane* lane, double lastAcceptTime) {
	findSegStats(lane->getRoadSegment())->getLaneParams(lane)->setLastAccept(lastAcceptTime);
}

void sim_mob::Conflux::resetPositionOfLastUpdatedAgentOnLanes() {
	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*>::iterator it = segmentAgents.begin();
	for( ; it != segmentAgents.end(); ++it )
	{
		(it->second)->resetPositionOfLastUpdatedAgentOnLanes();
	}
}

sim_mob::SegmentStats* sim_mob::Conflux::findSegStats(const sim_mob::RoadSegment* rdSeg) {
	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*>::iterator it = segmentAgents.find(rdSeg);
	if(it == segmentAgents.end()){ // if not found, search in downstreamSegments
		return rdSeg->getParentConflux()->findSegStats(rdSeg);
	}
	return it->second;
}

void sim_mob::Conflux::setTravelTimes(Person* ag, double linkExitTime) {

	std::map<double, Person::travelStats>::const_iterator it =
			ag->getTravelStatsMap().find(linkExitTime);
	if (it != ag->getTravelStatsMap().end()){
		double travelTime = (it->first) - (it->second).linkEntryTime_;
		std::map<const Link*, travelTimes>::iterator itTT = LinkTravelTimesMap.find((it->second).link_);
		if (itTT != LinkTravelTimesMap.end())
		{
			itTT->second.agentCount_ = itTT->second.agentCount_ + 1;
			itTT->second.linkTravelTime_ = itTT->second.linkTravelTime_ + travelTime;
		}
		else{
			travelTimes tTimes(travelTime, 1);
			LinkTravelTimesMap.insert(std::make_pair(ag->getCurrSegment()->getLink(), tTimes));
		}
	}
}

bool sim_mob::Conflux::call_movement_frame_init(timeslice now, Person* person) {
	//Agents may be created with a null Role and a valid trip chain
	Print()<<"calling frame_init for Person: "<<person->getId()<<std::endl;

	if (!person->getRole()) {
		//TODO: This UpdateStatus has a "prevParams" and "currParams" that should
		//      (one would expect) be dealt with. Where does this happen?
		UpdateStatus res =	person->checkTripChain(now.ms());

		//Reset the start time (to the current time tick) so our dispatcher doesn't complain.
		person->setStartTime(now.ms());

		//Nothing left to do?
		if (res.status == UpdateStatus::RS_DONE) {
			return false;
		}
	}
	//Failsafe: no Role at all?
	if (!person->getRole()) {
		std::ostringstream txt ;
		txt << "Person " << this->getId() <<  " has no Role.";
		throw std::runtime_error(txt.str());
	}

	//Get an UpdateParams instance.
	//TODO: This is quite unsafe, but it's a relic of how Person::update() used to work.
	//      We should replace this eventually (but this will require a larger code cleanup).
	person->curr_params = &(person->getRole()->make_frame_tick_params(now));

	//Now that the Role has been fully constructed, initialize it.
	if(*(person->currTripChainItem)) {
		person->getRole()->Movement()->frame_init(*person->curr_params);
	}

	return true;
}

Entity::UpdateStatus sim_mob::Conflux::call_movement_frame_tick(timeslice now, Person* person) {
	Role* personRole = person->getRole();
	if (!person->curr_params) {
		person->curr_params = &personRole->make_frame_tick_params(now);
		Print() << "updated person->curr_params: " << now.frame() << "|" << person->curr_params->now.frame() << std::endl;
	}
	person->setLastUpdatedFrame(currFrameNumber.frame());

	Entity::UpdateStatus retVal(UpdateStatus::RS_CONTINUE);

	/*
	 * The following loop guides the movement of the person by invoking the movement facet of the person's role one or more times
	 * until the remainingTimeThisTick of the person is expired.
	 * The frame tick of the movement facet returns when one of the following conditions are true. These are handled by case distinction.
	 *
	 * 1. frame_tick has displaced the person to the maximum distance that the person can move in the full tick duration. This case identified by
	 * checking if the remainingTimeThisTick of the person is 0. This case terminates the loop. The person's location is updated in the conflux
	 * that it belongs to. If the person has to be removed from the simulation, he is.
	 *
	 * 2. The person has reached the end of a link. This case is identified by checking requestedNextSegment for not null which indicates that the role has
	 * requested permission to move to the next link in its path. The conflux immdeitely grants permission by setting the flag canMoveToNextSegment.
	 * If the next link is not processed for the current tick, the person is added to lane infinity of the next conflux's segment stats and the
	 * loop is broken. If the next link is processed, the loop continues until any of the termination conditions are met. The movement role facet (driver)
	 * checks canMoveToNextSegment flag before it advances in its frame_tick.
	 *
	 * 3. The person has reached the end of the current subtrip. The loop will catch this and update the current trip chain item and change roles.
	 * This loop sets the current segment, set the lane as lane infinity and call the movement facet of the person's role again.
	 */

	while(person->remainingTimeThisTick > 0.0) {
		Print() << "Person: " << person->getId() << "|remainingTimeThisTick: " << person->remainingTimeThisTick << std::endl;

		if (!person->isToBeRemoved()) {
			personRole->Movement()->frame_tick(*person->curr_params);
		}

		if (person->isToBeRemoved()) {
			retVal = person->checkTripChain(now.ms());
			personRole = person->getRole();

			//Reset the start time (to the NEXT time tick) so our dispatcher doesn't complain.
			person->setStartTime(now.ms());

			if(person->currTripChainItem != person->tripChain.end()) {
				if((*person->currTripChainItem)->itemType == sim_mob::TripChainItem::IT_ACTIVITY) {
					//IT_ACTIVITY as of now is just a matter of waiting for a period of time(between its start and end time)
					//since start time of the activity is usually later than what is configured initially,
					//we have to make adjustments so that it waits for exact amount of time
					sim_mob::ActivityPerformer *ap = dynamic_cast<sim_mob::ActivityPerformer*>(personRole);
					ap->setActivityStartTime(sim_mob::DailyTime((*person->currTripChainItem)->startTime.getValue() + now.ms() + ConfigParams::GetInstance().baseGranMS));
					ap->setActivityEndTime(sim_mob::DailyTime(now.ms() + ConfigParams::GetInstance().baseGranMS + (*person->currTripChainItem)->endTime.getValue()));
					ap->initializeRemainingTime();
				}
				else if((*person->currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP) {
					//person->getRole()->Movement()->frame_init(*person->curr_params);
					call_movement_frame_init(now, person);
					person->setCallFrameInit(false);
					const RoadSegment* curSeg = person->getRole()->getResource()->getCurrSegment();
					person->setCurrSegment(curSeg);
					person->setCurrLane(curSeg->getParentConflux()->findSegStats(curSeg)->laneInfinity);
					person->distanceToEndOfSegment = curSeg->computeLaneZeroLength();
				}
			}
		}

		if(person->requestedNextSegment){
			Conflux* nxtConflux = person->requestedNextSegment->getParentConflux();
			SegmentStats* nxtSegStats = findSegStats(person->requestedNextSegment);

			Print() << "nxtConflux:" << nxtConflux->getMultiNode()->getID()
					<< "|lastUpdatedFrame:" << nxtConflux->getLastUpdatedFrame()
					<< "|currFrame:" << now.frame()
					<< "|requestedNextSegment: " << person->requestedNextSegment->getStartEnd()
					<< std::endl;

			person->canMoveToNextSegment = Person::GRANTED; // grant permission. But check whether the subsequent frame_tick can be called now.
			if(now.frame() > nxtConflux->getLastUpdatedFrame()) {
				//this is a hack to count the number of agents trying to loop back to the same
				//conflux again in the same frame tick
				if(nxtConflux == this){
					person->setRemainingTimeThisTick(0.0);
					Print()<<"Person "<<person->getId()<<" loops back to conflux "<< this->multiNode->getID()<<std::endl;
					break;
				}

				// nxtConflux is not processed for the current tick yet
				if(nxtConflux->hasSpaceInVirtualQueue(person->requestedNextSegment->getLink())) {
					person->setCurrSegment(person->requestedNextSegment);
					person->setCurrLane(nullptr); // so that the updateAgent function will add this agent to the lane infinity of nxtSegStats
					person->requestedNextSegment = nullptr;
					break; //break off from loop
				}
				else {
					person->canMoveToNextSegment = Person::DENIED;
					person->requestedNextSegment = nullptr;
				}

			}
			else if(now.frame() == nxtConflux->getLastUpdatedFrame()) {
				// nxtConflux is processed for the current tick. Can move to the next link.
				// handled by setting person->canMoveToNextSegment = GRANTED
				person->requestedNextSegment = nullptr;
			}
			else {
				throw std::runtime_error("lastUpdatedFrame of confluxes are managed incorrectly");
			}
		}
	}
	return retVal;
}

void sim_mob::Conflux::call_movement_frame_output(timeslice now, Person* person) {
	//Save the output
	if (!isToBeRemoved()) {
		person->currRole->Movement()->frame_tick_output(*person->curr_params);
	}

	//TODO: Still risky.
	person->curr_params = nullptr; //WARNING: Do *not* delete curr_params; it is only used to point to the result of get_params.
}

void sim_mob::Conflux::reportLinkTravelTimes(timeslice frameNumber) {
	if (ConfigParams::GetInstance().OutputEnabled()) {
		std::map<const Link*, travelTimes>::const_iterator it = LinkTravelTimesMap.begin();
		for( ; it != LinkTravelTimesMap.end(); ++it ) {
			LogOut("(\"linkTravelTime\""
				<<","<<frameNumber.frame()
				<<","<<it->first->getLinkId()
				<<",{"
				<<"\"travelTime\":\""<< (it->second.linkTravelTime_)/(it->second.agentCount_)
				<<"\"})"<<std::endl);
		}
	}
}

void sim_mob::Conflux::resetLinkTravelTimes(timeslice frameNumber) {
	LinkTravelTimesMap.clear();
}

void sim_mob::Conflux::updateLaneParams(const Lane* lane, double newOutFlowRate) {
	findSegStats(lane->getRoadSegment())->updateLaneParams(lane, newOutFlowRate);
}

void sim_mob::Conflux::restoreLaneParams(const Lane* lane) {
	findSegStats(lane->getRoadSegment())->restoreLaneParams(lane);
}

double sim_mob::Conflux::getSegmentFlow(const RoadSegment* rdSeg){
	return findSegStats(rdSeg)->getSegFlow();
}

void sim_mob::Conflux::incrementSegmentFlow(const RoadSegment* rdSeg) {
	findSegStats(rdSeg)->incrementSegFlow();
}

void sim_mob::Conflux::resetSegmentFlows() {
	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*>::iterator it = segmentAgents.begin();
	for( ; it != segmentAgents.end(); ++it )
	{
		(it->second)->resetSegFlow();
	}
}

UpdateStatus sim_mob::Conflux::perform_person_move(timeslice now, Person* person)
{
	// We give the Agent the benefit of the doubt here and simply call frame_init().
	// This allows them to override the start_time if it seems appropriate (e.g., if they
	// are swapping trip chains). If frame_init() returns false, immediately exit.
	bool calledFrameInit = false;
	if (person->isCallFrameInit()) {
		//Call frame_init() and exit early if requested to.
		if (!call_movement_frame_init(now, person)) {
			return UpdateStatus::Done;
		}

		//Set call_frame_init to false here; you can only reset frame_init() in frame_tick()
		person->setCallFrameInit(false); //Only initialize once.
		calledFrameInit = true;
	}

	//Now that frame_init has been called, ensure that it was done so for the correct time tick.
	CheckFrameTimes(person->getId(), now.ms(), person->getStartTime(), calledFrameInit, person->isToBeRemoved());

	//Perform the main update tick
	UpdateStatus retVal = call_movement_frame_tick(now, person);

	//Save the output
	if (retVal.status != UpdateStatus::RS_DONE && person->remainingTimeThisTick<=0) {
		call_movement_frame_output(now, person);
	}

	return retVal;
}

double sim_mob::Conflux::getPositionOfLastUpdatedAgentInLane(const Lane* lane) {
	return findSegStats(lane->getRoadSegment())->getPositionOfLastUpdatedAgentInLane(lane);
}

const Lane* sim_mob::Conflux::getLaneInfinity(const RoadSegment* rdSeg) {
	return findSegStats(rdSeg)->laneInfinity;
}

bool sim_mob::cmp_person_remainingTimeThisTick::operator ()(const Person* x, const Person* y) const {
	if ((!x) || (!y)) {
		std::stringstream debugMsgs;
		debugMsgs
				<< "cmp_person_remainingTimeThisTick: Comparison failed because at least one of the arguments is null"
				<< "|x: " << (x ? x->getId() : 0) << "|y: "
				<< (y ? y->getId() : 0);
		throw std::runtime_error(debugMsgs.str());
	}
	//We want greater remaining time in this tick to translate into a higher priority.
	return (x->getRemainingTimeThisTick() > y->getRemainingTimeThisTick());
}

//Sort all agents in lane (based on remaining time this tick)
void sim_mob::sortPersons_DecreasingRemTime(std::deque<Person*> personList) {
	cmp_person_remainingTimeThisTick cmp_person_remainingTimeThisTick_obj;
	//ordering is required only if we have more than 1 person in the deque
	if(personList.size() > 1) {
		std::sort(personList.begin(), personList.end(), cmp_person_remainingTimeThisTick_obj);
	}
}

std::deque<sim_mob::Person*> sim_mob::Conflux::getAllPersons() {
	std::deque<sim_mob::Person*> allPersonsInCfx, tmpAgents;
	sim_mob::SegmentStats* segStats = nullptr;
	for(std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> >::iterator upStrmSegMapIt = upstreamSegmentsMap.begin(); upStrmSegMapIt!=upstreamSegmentsMap.end(); upStrmSegMapIt++) {
		for(std::vector<sim_mob::RoadSegment*>::const_iterator rdSegIt=upStrmSegMapIt->second.begin(); rdSegIt!=upStrmSegMapIt->second.end(); rdSegIt++) {
			segStats = findSegStats(*rdSegIt);
			tmpAgents = segStats->getAgents();
			allPersonsInCfx.insert(allPersonsInCfx.end(), tmpAgents.begin(), tmpAgents.end());
		}
	}
	for(std::map<sim_mob::Link*, std::deque<sim_mob::Person*> >::iterator vqMapIt = virtualQueuesMap.begin(); vqMapIt != virtualQueuesMap.end(); vqMapIt++) {
		tmpAgents = vqMapIt->second;
		allPersonsInCfx.insert(allPersonsInCfx.end(), tmpAgents.begin(), tmpAgents.end());
	}
	return allPersonsInCfx;
}
