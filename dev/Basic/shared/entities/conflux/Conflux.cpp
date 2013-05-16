/*
 * Conflux.cpp
 *
 *  Created on: Oct 2, 2012
 *      Author: harish
 */

#include "Conflux.hpp"
#include<map>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include "Conflux.hpp"
#include "conf/simpleconf.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "logging/Log.hpp"


using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;

namespace {
//Ensure all time ticks are valid.
void check_frame_times(unsigned int agentId, uint32_t now, unsigned int startTime, bool wasFirstFrame, bool wasRemoved) {
	//Has update() been called early?
	if (now<startTime) {
		std::stringstream msg;
		msg << "Agent(" <<agentId << ") specifies a start time of: " <<startTime
				<< " but it is currently: " << now
				<< "; this indicates an error, and should be handled automatically.";
		throw std::runtime_error(msg.str().c_str());
	}

	//Has update() been called too late?
	if (wasRemoved) {
		std::stringstream msg;
		msg << "Agent(" <<agentId << ") should have already been removed, but was instead updated at: " <<now
				<< "; this indicates an error, and should be handled automatically.";
		throw std::runtime_error(msg.str().c_str());
	}

	//Was frame_init() called at the wrong point in time?
	if (wasFirstFrame) {
		if (abs(now-startTime)>=ConfigParams::GetInstance().baseGranMS) {
			std::stringstream msg;
			msg <<"Agent was not started within one timespan of its requested start time.";
			msg <<"\nStart was: " <<startTime <<",  Curr time is: " <<now <<"\n";
			msg <<"Agent ID: " <<agentId <<"\n";
			throw std::runtime_error(msg.str().c_str());
		}
	}
}
} //End un-named namespace


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
	currFrameNumber = frameNumber;

	resetPositionOfLastUpdatedAgentOnLanes();
	//resetSegmentFlows();

	//sort the virtual queues before starting to move agents for this tick
	//for now, virtual queues are the lane infinities of the first road segment (in the driving direction) of each link
	SegmentStats* firstSegStats = nullptr;
	for(std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> >::iterator i = upstreamSegmentsMap.begin(); i!=upstreamSegmentsMap.end(); i++) {
		firstSegStats = findSegStats(*(i->second.begin()));
		firstSegStats->sortPersons_DecreasingRemTime(firstSegStats->laneInfinity);
	}
	firstSegStats = nullptr; //reset so that this isn't accidentally used elsewhere

	if (sim_mob::StreetDirectory::instance().signalAt(*multiNode) != nullptr) {
		updateUnsignalized(); //TODO: Update Signalized must be implemented
	}
	else {
		updateUnsignalized();
	}
	//updateSupplyStats(frameNumber);

	UpdateStatus retVal(UpdateStatus::RS_CONTINUE); //always return continue. Confluxes never die.
	//resetOutputBounds(); //For use by other confluxes in the next tick
	lastUpdatedFrame = frameNumber.frame();
	return retVal;
}

void sim_mob::Conflux::updateSignalized() {
	throw std::runtime_error("Conflux::updateSignalized() not implemented yet.");
}

void sim_mob::Conflux::updateUnsignalized() {
	initCandidateAgents();
	sim_mob::Person* ag = agentClosestToIntersection();
	while (ag) {
		updateAgent(ag);

		// get next Person to update
		ag = agentClosestToIntersection();
	}
}

void sim_mob::Conflux::updateAgent(sim_mob::Person* person) {
	if (person->lastUpdatedFrame < currFrameNumber.frame()) {
		//if the person is moved for the first time in this tick
		person->remainingTimeThisTick = ConfigParams::GetInstance().baseGranMS / 1000.0;
	}

	const sim_mob::RoadSegment* segBeforeUpdate = person->getCurrSegment();
	const sim_mob::Lane* laneBeforeUpdate = person->getCurrLane();
	bool isQueuingBeforeUpdate = person->isQueuing;
	sim_mob::SegmentStats* segStatsBfrUpdt = findSegStats(segBeforeUpdate);

	UpdateStatus res = perform_person_move(currFrameNumber, person);

	if (res.status == UpdateStatus::RS_DONE) {
		//This Person is done. Remove from simulation.
		killAgent(person, segBeforeUpdate, laneBeforeUpdate);
		Print()<<"kill agent: "<< person->getId()<<std::endl;
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

	if((segBeforeUpdate != segAfterUpdate) || (laneBeforeUpdate == segStatsBfrUpdt->laneInfinity && laneBeforeUpdate != laneAfterUpdate))
	{
		Person* dequeuedAgent = segStatsBfrUpdt->dequeue(laneBeforeUpdate);
		if(dequeuedAgent != person) {
			segStatsBfrUpdt->printAgents();
			segStatsAftrUpdt->printAgents();
			debugMsgs << "Error: Person " << dequeuedAgent->getId() << " dequeued instead of Person " << person->getId()
					<< "|segment: [" << segBeforeUpdate->getStart()->getID() <<","<< segBeforeUpdate->getEnd()->getID() << "]"
					<< "|lane: " << laneBeforeUpdate->getLaneID()
					<< "|Frame " << currFrameNumber.frame();
			throw std::runtime_error(debugMsgs.str());
		}
		if(laneAfterUpdate) {
			segStatsAftrUpdt->addAgent(laneAfterUpdate, person);
		}
		else {
			// If we don't know which lane the Person has to go to, we add him to lane infinity.
			// NOTE: One possible scenario for this is a Person who is starting on a new trip chain item.
			person->setCurrLane(segStatsAftrUpdt->laneInfinity);
			person->distanceToEndOfSegment = segAfterUpdate->computeLaneZeroLength();
			segStatsAftrUpdt->addAgent(segStatsAftrUpdt->laneInfinity, person);
			laneAfterUpdate = segStatsAftrUpdt->laneInfinity;
		}
	}
	else if (isQueuingBeforeUpdate != isQueuingAfterUpdate)
	{
		segStatsAftrUpdt->updateQueueStatus(laneAfterUpdate, person);
	}

	// set the position of the last updated Person in his current lane (after update)
	segStatsAftrUpdt->setPositionOfLastUpdatedAgentInLane(person->distanceToEndOfSegment, person->getCurrLane());

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

void sim_mob::Conflux::resetOutputBounds() {
	outputBounds.clear();
	const sim_mob::RoadSegment* firstSeg = nullptr;
	sim_mob::SegmentStats* segStats = nullptr;
	unsigned int outputEstimate = 0;
	for(std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> >::iterator i = upstreamSegmentsMap.begin(); i != upstreamSegmentsMap.end(); i++) {
		firstSeg = *(i->second.begin());
		segStats = findSegStats(firstSeg);
		outputEstimate = segStats->computeExpectedOutputPerTick();
		outputEstimate = outputEstimate - segStats->numAgentsInLane(segStats->laneInfinity);
		outputBounds.insert(std::make_pair(firstSeg, outputEstimate));
		Print()<<"outputBounds rdSeg (insert): ["<<firstSeg->getStart()->getID()<<","<<firstSeg->getEnd()->getID()
					<<"]"<<" conflux mnode: "<<multiNode->getID()<<std::endl;
	}
}

bool sim_mob::Conflux::hasSpaceInVirtualQueue(const sim_mob::RoadSegment* rdSeg) {
	Print()<<"outputBounds rdSeg (hasSpace): ["<<rdSeg->getStart()->getID()<<","<<rdSeg->getEnd()->getID()
			<<"]"<<" conflux mnode: "<<multiNode->getID()<<std::endl;
	return (outputBounds.at(rdSeg) > 0);
}

void sim_mob::Conflux::decrementBound(const sim_mob::RoadSegment* rdSeg) {
	try {
		if(outputBounds.at(rdSeg)) {
			unsigned int bound = outputBounds.at(rdSeg);
			bound = (bound > 0)? bound-1 : 0;
			outputBounds[rdSeg] = bound;
		}
	}
	catch(std::exception& e) {
		debugMsgs << "decrementBound() called for segment " << rdSeg->getStartEnd() << " from invalid conflux " << multiNode->getID();
		throw std::runtime_error(debugMsgs.str());
	}
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
	double minDistance = std::numeric_limits<double>::max();
	std::map<const sim_mob::RoadSegment*, sim_mob::Person*>::iterator i = candidateAgents.begin();
	while (i != candidateAgents.end()) {
		if (i->second != nullptr) {
			if (minDistance == (i->second->distanceToEndOfSegment + lengthsOfSegmentsAhead.at(i->first))) {
				// If current ag and (*i) are at equal distance to the intersection (end of the link), we toss a coin and choose one of them
				bool coinTossResult = ((rand() / (double) RAND_MAX) < 0.5);
				if (coinTossResult) {
					agRdSeg = i->first;
					ag = i->second;
				}
			} else if (minDistance > (i->second->distanceToEndOfSegment + lengthsOfSegmentsAhead.at(i->first))) {
				minDistance = i->second->distanceToEndOfSegment + lengthsOfSegmentsAhead.at(i->first);
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

void sim_mob::Conflux::prepareLengthsOfSegmentsAhead() {
	for(std::map<sim_mob::Link*, const std::vector<sim_mob::RoadSegment*> >::iterator i = upstreamSegmentsMap.begin(); i != upstreamSegmentsMap.end(); i++)
	{
		double lengthAhead = 0.0;
		for(std::vector<sim_mob::RoadSegment*>::const_reverse_iterator j = i->second.rbegin(); j != i->second.rend(); j++)
		{
			lengthsOfSegmentsAhead.insert(std::make_pair(*j, lengthAhead));
			lengthAhead = lengthAhead + (*j)->computeLaneZeroLength();
		}
	}
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
		(it->second)->reportSegmentStats(frameNumber);
	}
}

std::pair<unsigned int, unsigned int> sim_mob::Conflux::getLaneAgentCounts(const sim_mob::Lane* lane) {
	return findSegStats(lane->getRoadSegment())->getLaneAgentCounts(lane);
}

unsigned int sim_mob::Conflux::getInitialQueueCount(const Lane* lane) {
	return findSegStats(lane->getRoadSegment())->getInitialQueueCount(lane);
}

void sim_mob::Conflux::killAgent(sim_mob::Person* ag, const sim_mob::RoadSegment* prevRdSeg, const sim_mob::Lane* prevLane) {
	findSegStats(prevRdSeg)->removeAgent(prevLane, ag);
	ag->currWorker = parentWorker;
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
	if (person->getId()==167){
		Print()<<"Person "<<person->getId()<<" found in call_movement_frame_init"<<std::endl;
	}
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
	}
	person->lastUpdatedFrame = currFrameNumber.frame();

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
		debugMsgs << "Frame: " << now.frame() << "|ms: " << now.ms()
				<< "|Person: " << person->getId() << "|remainingTimeThisTick: " << person->remainingTimeThisTick
				<< "|currSegment: [" << person->currSegment->getStart()->getID() <<","<< person->currSegment->getEnd()->getID() << "]"
				<< "|distanceToEndOfSegment: " << person->distanceToEndOfSegment
				<< "|isQueuing: " << person->isQueuing
				<< "|segLength: "<< person->getCurrSegment()->computeLaneZeroLength()
				<< "|Conflux: " << multiNode->getID() <<std::endl;
		std::cout << debugMsgs.str();
		debugMsgs.str("");

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

			debugMsgs << "nxtConflux:" << nxtConflux->getMultiNode()->getID()
					<< "|lastUpdatedFrame:" << nxtConflux->lastUpdatedFrame
					<< "|currFrame:" << now.frame()
					<< "|requestedNextSegment: [" << person->requestedNextSegment->getStart()->getID() <<","<< person->requestedNextSegment->getEnd()->getID() << "]"
					<< std::endl;
			std::cout << debugMsgs.str();
			debugMsgs.str("");

			person->canMoveToNextSegment = Person::GRANTED; // grant permission. But check whether the subsequent frame_tick can be called now.
			if(now.frame() > nxtConflux->lastUpdatedFrame) {
				// nxtConflux is not processed for the current tick yet
				if(nxtConflux->hasSpaceInVirtualQueue(person->requestedNextSegment)) {
					person->setCurrSegment(person->requestedNextSegment);
					person->setCurrLane(nullptr); // so that the updateAgent function will add this agent to the lane infinity of nxtSegStats
					person->requestedNextSegment = nullptr;
					break; //break off from loop
				}
				else {
					person->canMoveToNextSegment = Person::DENIED;
				}
			}
			else if(now.frame() == nxtConflux->lastUpdatedFrame) {
				// nxtConflux is processed for the current tick. Can move to the next link.
				// handled by setting person->canMoveToNextSegment = true
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
	//We give the Agent the benefit of the doubt here and simply call frame_init().
	//This allows them to override the start_time if it seems appropriate (e.g., if they
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
	check_frame_times(person->getId(), now.ms(), person->getStartTime(), calledFrameInit, person->isToBeRemoved());

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
