//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Conflux.cpp
 *
 *  Created on: Oct 2, 2012
 *      Author: harish
 */

#include "Conflux.hpp"


#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>
#include <vector>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Person.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "entities/misc/TripChain.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/PathSetManager.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "event/SystemEvents.hpp"
#include "event/args/EventArgs.hpp"
#include "message/MessageBus.hpp"
#include "event/EventPublisher.hpp"
#include "logging/Log.hpp"
#include "util/Utils.hpp"
#include "workers/Worker.hpp"

using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;

namespace{
    const double INFINITESIMAL_DOUBLE = 0.000001;
    const double PASSENGER_CAR_UNIT = 400.0; //cm; 4 m.
}

sim_mob::Conflux::Conflux(sim_mob::MultiNode* multinode, const MutexStrategy& mtxStrat, int id)
	: Agent(mtxStrat, id),
	  multiNode(multinode), signal(StreetDirectory::instance().signalAt(*multinode)),
	  parentWorker(nullptr), currFrameNumber(0,0), debugMsgs(std::stringstream::out),
	  isBoundary(false), isMultipleReceiver(false)
{
	messaging::MessageBus::RegisterHandler(this);
}


sim_mob::Conflux::~Conflux()
{
	for(SegmentStatsMap::iterator i=segmentAgents.begin(); i!=segmentAgents.end(); i++) {
		safe_delete_item(i->second);
	}
	activityPerformers.clear();
}


void sim_mob::Conflux::addAgent(sim_mob::Person* person, const sim_mob::RoadSegment* rdSeg) {
	/*
	 * Persons start at a node (for now).
	 * we will always add the Person to the road segment in "lane infinity".
	 */
	sim_mob::SegmentStats* rdSegStats = segmentAgents.find(rdSeg)->second;
	person->setCurrSegStats(rdSegStats);
	person->setCurrLane(rdSegStats->laneInfinity);
	person->distanceToEndOfSegment = rdSegStats->getLength();
	person->remainingTimeThisTick = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	rdSegStats->addAgent(rdSegStats->laneInfinity, person);
}

UpdateStatus sim_mob::Conflux::update(timeslice frameNumber) {
	currFrameNumber = frameNumber;

	resetPositionOfLastUpdatedAgentOnLanes();

	//reset the remaining times of persons in lane infinity and VQ if required.
	resetPersonRemTimes();

	if (sim_mob::StreetDirectory::instance().signalAt(*multiNode) != nullptr) {
		updateUnsignalized(); //TODO: Update Signalized must be implemented and called here
	}
	else {
		updateUnsignalized();
	}

	UpdateStatus retVal(UpdateStatus::RS_CONTINUE); //always return continue. Confluxes never die.
	setLastUpdatedFrame(frameNumber.frame());
	return retVal;
}

void sim_mob::Conflux::updateSignalized() {
	throw std::runtime_error("Conflux::updateSignalized() not implemented yet.");
}

void sim_mob::Conflux::updateUnsignalized() {

#define UsingTopCMergeInConflux
#ifdef UsingTopCMergeInConflux
	//merge vehicles on conflux
	PersonList allPersonsInReverseOrder;
	getAllPersonsUsingTopCMerge(allPersonsInReverseOrder);

	PersonList::iterator personIt = allPersonsInReverseOrder.begin();
	for (; personIt != allPersonsInReverseOrder.end(); personIt++) {
		updateAgent(*personIt);
	}
#else
	initCandidateAgents();
	sim_mob::Person* person = agentClosestToIntersection();
	while (person) {
		updateAgent(person);
		person = agentClosestToIntersection(); // get next Person to update
	}
#endif

	// We would have to add and erase persons in activityPerformers in updateAgent(). Therefore we need to iterate on a copy.
	PersonList activityPerformersCopy = activityPerformers;
	for(PersonList::iterator i = activityPerformersCopy.begin(); i != activityPerformersCopy.end(); i++) {
		updateAgent(*i);
	}

	PersonList pedestrianPerformersCopy = pedestrianPerformers;
	for(PersonList::iterator i = pedestrianPerformersCopy.begin(); i != pedestrianPerformersCopy.end(); i++) {
		updateAgent(*i);
	}
}

void sim_mob::Conflux::updateAgent(sim_mob::Person* person) {
	//local declarations

	//To preserve the state of the person before update
	const sim_mob::Role* roleBeforeUpdate = nullptr;
	const sim_mob::RoadSegment* segBeforeUpdate = nullptr;
	const sim_mob::Lane* laneBeforeUpdate = nullptr;
	bool isQueuingBeforeUpdate = false;
	sim_mob::SegmentStats* segStatsBfrUpdt = nullptr;

	//To capture the state of the person after update b
	const sim_mob::Role* roleAfterUpdate = nullptr;
	const sim_mob::RoadSegment* segAfterUpdate = nullptr;
	const sim_mob::Lane* laneAfterUpdate = nullptr;
	bool isQueuingAfterUpdate = false;
	sim_mob::SegmentStats* segStatsAftrUpdt = nullptr;

	if (person->getLastUpdatedFrame() < currFrameNumber.frame()) {
		//if the person is being moved for the first time in this tick
		person->remainingTimeThisTick = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	}

	person->currWorkerProvider = parentWorker; // Let the person know which worker is managing him... for logs to work.
	roleBeforeUpdate = person->getRole();
	segBeforeUpdate = person->getCurrSegStats()->getRoadSegment();
	segStatsBfrUpdt = findSegStats(segBeforeUpdate); //person->getCurrSegStats() cannot be used as it returns a const pointer
	laneBeforeUpdate = person->getCurrLane();
	isQueuingBeforeUpdate = person->isQueuing;


	if(segBeforeUpdate && segBeforeUpdate->getParentConflux() != this) {
		debugMsgs << "segBeforeUpdate not in the current conflux"
				<<"|segBeforeUpdate's conflux: " << segBeforeUpdate->getParentConflux()->getMultiNode()->getID()
				<<"|this conflux: "<< this->getMultiNode()->getID()
				<<"|person: "<< person->getId()
				<<"|Frame: " << currFrameNumber.frame()
				<<"|segBeforeUpdate_worker: "<< segBeforeUpdate->getParentConflux()->getParentWorker()
				<<"|this_worker: "<< this->getParentWorker()
				<<"|SegBeforeUpdate: "<< segBeforeUpdate->getStartEnd()
				<<"|laneBeforeUpdate: " << (laneBeforeUpdate->getLaneID()?laneBeforeUpdate->getLaneID():999)
				<<"|isQueuingBeforeUpdate:"<< (isQueuingBeforeUpdate? 1:0)
				<< std::endl;
		throw std::runtime_error(debugMsgs.str());
	}

	UpdateStatus res = perform_person_move(currFrameNumber, person);

	if (res.status == UpdateStatus::RS_DONE) {
		//This Person is done. Remove from simulation.
		killAgent(person, segStatsBfrUpdt, laneBeforeUpdate, isQueuingBeforeUpdate);
		return;
	} else if (res.status == UpdateStatus::RS_CONTINUE) {
		// TODO: I think there will be nothing here. Have to make sure. ~ Harish
	} else {
		throw std::runtime_error("Unknown/unexpected update() return status.");
	}

	roleAfterUpdate = person->getRole();
	if(roleAfterUpdate->roleType == sim_mob::Role::RL_ACTIVITY) { // if role is ActivityPerformer after update
		if (roleBeforeUpdate && roleBeforeUpdate->roleType == sim_mob::Role::RL_ACTIVITY) {
			// if the role was ActivityPerformer before the update as well, do nothing
			// It is also possible that the person has changed from one activity to another. We do nothing even if this is the case.
		}
		else {
			// else if the person currently in an activity and was in a Trip previously,
			// remove this person from the network and add him to the activity performers list
			if(laneBeforeUpdate) {
				// if the person was not in from a virtual queue, we dequeue him;
				segStatsBfrUpdt->dequeue(person, laneBeforeUpdate, isQueuingBeforeUpdate);
			}
			activityPerformers.push_back(person);
		}
		return;
	}
	else { // if the person is in a Trip/SubTrip after update
		if (roleBeforeUpdate && roleBeforeUpdate->roleType == sim_mob::Role::RL_ACTIVITY) {
			// if the person has changed from an Activity to the current Trip/SubTrip during this tick,
			// remove this person from the activityPerformers list
			std::deque<Person*>::iterator pIt = std::find(activityPerformers.begin(), activityPerformers.end(), person);
			activityPerformers.erase(pIt);
		}
	}

	segAfterUpdate = person->getCurrSegStats()->getRoadSegment();
	segStatsAftrUpdt = findSegStats(segAfterUpdate); //person->getCurrSegStats() cannot be used as it returns a const pointer
	laneAfterUpdate = person->getCurrLane();
	isQueuingAfterUpdate = person->isQueuing;


	if (!laneBeforeUpdate) { //If the person was in virtual queue or was performing an activity
		if(laneAfterUpdate) { //If the person has moved to another lane (possibly even to laneInfinity if he was performing activity) in some segment
			segStatsAftrUpdt->addAgent(laneAfterUpdate, person);
		}
		else  {
			if (segStatsBfrUpdt != segStatsAftrUpdt) {
				// the person may have moved to another virtual queue
				// - which is not possible if the virtual queues are processed
				// after all conflux updates
				debugMsgs
						<< "Error: Person has moved from one virtual queue to another. "
						<< "\n Person " << person->getId()
						<< "|Frame: " << currFrameNumber.frame()
						<< "|segBeforeUpdate: " << segBeforeUpdate->getStartEnd()
						<< "|segStatsAftrUpdt: " << segAfterUpdate->getStartEnd();
				throw std::runtime_error(debugMsgs.str());
			}
			else {
				/* This is typically the person who was not accepted by the next lane in the next segment.
				 * We push this person back to the same virtual queue and let him update in the next tick.
				 */
				person->distanceToEndOfSegment = segAfterUpdate->getLaneZeroLength();
				segAfterUpdate->getParentConflux()->pushBackOntoVirtualQueue(segAfterUpdate->getLink(), person);
			}
		}
	}
	else if((segBeforeUpdate != segAfterUpdate) /*if the person has moved to another segment*/
			|| (laneBeforeUpdate == segStatsBfrUpdt->laneInfinity && laneBeforeUpdate != laneAfterUpdate) /* or if the person has moved out of lane infinity*/)
	{
		Person* dequeuedPerson = segStatsBfrUpdt->dequeue(person, laneBeforeUpdate, isQueuingBeforeUpdate);
		if(dequeuedPerson != person) {
			segStatsBfrUpdt->printAgents();
			debugMsgs << "Error: Person " << dequeuedPerson->getId() << " dequeued instead of Person " << person->getId()
					<< "\n Person " << person->getId() << ": segment: " << segBeforeUpdate->getStartEnd()
					<< "|lane: " << laneBeforeUpdate->getLaneID()
					<< "|Frame: " << currFrameNumber.frame()
					<< "\n Person " << dequeuedPerson->getId() << ": "
					<< "segment: " << dequeuedPerson->getCurrSegStats()->getRoadSegment()->getStartEnd()
					<< "|lane: " << dequeuedPerson->getCurrLane()->getLaneID()
					<< "|Frame: " << dequeuedPerson->getLastUpdatedFrame();

			throw std::runtime_error(debugMsgs.str());
		}

		if(laneAfterUpdate) {
			segStatsAftrUpdt->addAgent(laneAfterUpdate, person);
		}
		else {
			/* We wouldn't know which lane the person has to go to if the person wants to enter a link
			 * which belongs to a conflux that is not processed for this tick yet.
			 * We add this person to the virtual queue for that link here */
			person->distanceToEndOfSegment = segAfterUpdate->getLaneZeroLength();
			segAfterUpdate->getParentConflux()->pushBackOntoVirtualQueue(segAfterUpdate->getLink(), person);
		}
	}
	//It's possible for some persons to start a new trip on the same segment where they ended the previous trip.
	else if(segBeforeUpdate == segAfterUpdate && laneAfterUpdate == segStatsAftrUpdt->laneInfinity){
		Person* dequeuedPerson = segStatsBfrUpdt->dequeue(person, laneBeforeUpdate, isQueuingBeforeUpdate);
		if(dequeuedPerson != person) {
			segStatsBfrUpdt->printAgents();
			debugMsgs << "Error: Person " << dequeuedPerson->getId() << " dequeued instead of Person " << person->getId()
					<< "\n Person " << person->getId()
					<< ": segment: " << segBeforeUpdate->getStartEnd()
					<< "|segStats: " << segStatsBfrUpdt
					<< "|lane: " << laneBeforeUpdate->getLaneID()
					<< "|Frame: " << currFrameNumber.frame()
					<< "\n Person " << dequeuedPerson->getId()
					<< ": segment: " << dequeuedPerson->getCurrSegStats()->getRoadSegment()->getStartEnd()
					<< "|segStats: " << dequeuedPerson->getCurrSegStats()
					<< "|lane: " << dequeuedPerson->getCurrLane()->getLaneID()
					<< "|Frame: " << dequeuedPerson->getLastUpdatedFrame();

			throw std::runtime_error(debugMsgs.str());
		}
		//adding the person to lane infinity for the new trip
		segStatsAftrUpdt->addAgent(laneAfterUpdate, person);
	}
	else if (isQueuingBeforeUpdate != isQueuingAfterUpdate) {
		segStatsAftrUpdt->updateQueueStatus(laneAfterUpdate, person);
	}

	// set the position of the last updated Person in his current lane (after update)
	if (laneAfterUpdate && laneAfterUpdate != segStatsAftrUpdt->laneInfinity) {
		//if the person did not end up in a VQ and his lane is not lane infinity of segAfterUpdate
		segStatsAftrUpdt->setPositionOfLastUpdatedAgentInLane(person->distanceToEndOfSegment, laneAfterUpdate);
	}
}

void sim_mob::Conflux::processVirtualQueues() {
	int counter = 0;
	{
		boost::unique_lock< boost::recursive_mutex > lock(mutexOfVirtualQueue);
		//sort the virtual queues before starting to move agents for this tick
		for(VirtualQueueMap::iterator i = virtualQueuesMap.begin(); i!=virtualQueuesMap.end(); i++) {
			counter = i->second.size();
			sortPersons_DecreasingRemTime(i->second);
			while(counter > 0){
				sim_mob::Person* p = i->second.front();
				i->second.pop_front();
				updateAgent(p);
				counter--;
			}
		}
	}
}

double sim_mob::Conflux::getSegmentSpeed(SegmentStats* segStats, bool hasVehicle) const{
	if (hasVehicle){
		return segStats->getSegSpeed(hasVehicle);
	}
	//else pedestrian lanes are not handled
	return 0.0;
}

void sim_mob::Conflux::initCandidateAgents() {
	candidateAgents.clear();
	resetCurrSegsOnUpLinks();

	sim_mob::Link* lnk = nullptr;
	for (UpstreamSegmentStatsMap::iterator i = upstreamSegStatsMap.begin(); i != upstreamSegStatsMap.end(); i++) {
		lnk = i->first;
		SegmentStats* currSegStatOnLnk = currSegsOnUpLinks.at(lnk);
		while (currSegStatOnLnk) {
			currSegStatOnLnk->resetFrontalAgents();
			sim_mob::Person* personClosestToIntersection = currSegStatOnLnk->agentClosestToStopLineFromFrontalAgents();
			candidateAgents.insert(std::make_pair(currSegStatOnLnk, personClosestToIntersection));
			if(!personClosestToIntersection) {
				// this road segment is deserted. search the next (which is, technically, the previous).
				const std::vector<sim_mob::SegmentStats*>& segStatsList = i->second; // or upstreamSegStatsMap.at(lnk);
				std::vector<sim_mob::SegmentStats*>::const_iterator segStatIt =
						std::find(segStatsList.begin(), segStatsList.end(), currSegStatOnLnk);
				currSegsOnUpLinks.erase(lnk);
				if(segStatIt != segStatsList.begin()) {
					segStatIt--;
					currSegsOnUpLinks.insert(std::make_pair(lnk, *segStatIt));
				}
				else {
					sim_mob::SegmentStats* nullSeg = nullptr;
					currSegsOnUpLinks.insert(std::make_pair(lnk, nullSeg)); // No agents in the entire link
				}
			}
			else { break; }
			currSegStatOnLnk = currSegsOnUpLinks.at(lnk);
		}
	}
}

void sim_mob::Conflux::resetCurrSegsOnUpLinks() {
	currSegsOnUpLinks.clear();
	for(UpstreamSegmentStatsMap::iterator i = upstreamSegStatsMap.begin(); i != upstreamSegStatsMap.end(); i++) {
		SegmentStats* lastSegStats = i->second.back();
		currSegsOnUpLinks.insert(std::make_pair(i->first, lastSegStats));
	}
}

/*
 * This function resets the remainingTime of persons who remain in lane infinity for more than 1 tick.
 * Note: This may include
 * 1. newly starting persons who (were supposed to, but) did not get added to the simulation
 * in the previous tick due to traffic congestion in their starting segment.
 * 2. Persons who got added to and remained virtual queue in the previous tick
 */
void sim_mob::Conflux::resetPersonRemTimes() {
	SegmentStats* segStats = nullptr;
	for(UpstreamSegmentStatsMap::iterator upStrmSegMapIt = upstreamSegStatsMap.begin(); upStrmSegMapIt!=upstreamSegStatsMap.end(); upStrmSegMapIt++) {
		for(std::vector<sim_mob::SegmentStats*>::const_iterator segStatsIt=upStrmSegMapIt->second.begin(); segStatsIt!=upStrmSegMapIt->second.end(); segStatsIt++) {
			segStats = *segStatsIt;
			PersonList& personsInLaneInfinity = segStats->getPersons(segStats->laneInfinity);
			for(PersonList::iterator personIt=personsInLaneInfinity.begin(); personIt!=personsInLaneInfinity.end(); personIt++) {
				if ((*personIt)->getLastUpdatedFrame() < currFrameNumber.frame()) {
					//if the person is going to be moved for the first time in this tick
					(*personIt)->remainingTimeThisTick = ConfigManager::GetInstance().FullConfig().baseGranSecond();
				}
			}
		}
	}

	{
		boost::unique_lock< boost::recursive_mutex > lock(mutexOfVirtualQueue);
		for(VirtualQueueMap::iterator vqIt=virtualQueuesMap.begin(); vqIt!=virtualQueuesMap.end();vqIt++) {
			PersonList& personsInVQ = vqIt->second;
			for(PersonList::iterator pIt= personsInVQ.begin(); pIt!=personsInVQ.end(); pIt++) {
				if ((*pIt)->getLastUpdatedFrame() < currFrameNumber.frame()) {
					//if the person is going to be moved for the first time in this tick
					(*pIt)->remainingTimeThisTick = ConfigManager::GetInstance().FullConfig().baseGranSecond();
				}
			}
		}
	}
}

void sim_mob::Conflux::buildSubscriptionList(std::vector<BufferedBase*>& subsList) {
	Agent::buildSubscriptionList(subsList);
}

unsigned int sim_mob::Conflux::resetOutputBounds() {
	vqBounds.clear();
	sim_mob::Link* lnk = nullptr;
	sim_mob::SegmentStats* segStats = nullptr;
	int outputEstimate = 0;
	unsigned int vqCount = 0;
	{
		boost::unique_lock< boost::recursive_mutex > lock(mutexOfVirtualQueue);
		for(VirtualQueueMap::iterator i = virtualQueuesMap.begin(); i != virtualQueuesMap.end(); i++) {
			lnk = i->first;
			segStats = findSegStats(lnk->getSegments().front());
			/** In DynaMIT, the upper bound to the space in virtual queue was set based on the number of empty spaces
				the first segment of the downstream link (the one the vq is attached with) is going to create in this tick according to the outputFlowRate*tick_size.
				This would ideally underestimate the space avaiable in the next segment, as it doesn't account for the empty spaces the segment already has.
				Therefore the virtual queues are most likely to be cleared by the end of that tick.
				[1] But with short segments, we noticed that this over estimated the space and left a considerably large amount of vehicles remaining in vq.
				Therefore, as per Yang Lu's suggestion, we are replacing computeExpectedOutputPerTick() calculation with existing number of empty spaces on the segment.
				[2] Another reason for vehicles to remain in vq is that in mid-term, we currently process the new vehicles (i.e.trying to get added to the network from lane infiinity),
				before we process the virtual queues. Therefore the space that we computed to be for vehicles in virtual queues, would have been already occupied by the new vehicles
				by the time the vehicles in virtual queues try to get added.
				 **/
			//outputEstimate = segStats->computeExpectedOutputPerTick();
			/** using ceil here, just to avoid short segments returning 0 as the total number of vehicles the road segment can hold i.e. when segment is shorter than a car**/
			int num_emptySpaces = std::ceil(segStats->getRoadSegment()->getLaneZeroLength()*segStats->getRoadSegment()->getLanes().size()/PASSENGER_CAR_UNIT)
					- segStats->numMovingInSegment(true) - segStats->numQueueingInSegment(true);
			outputEstimate = (num_emptySpaces>=0)? num_emptySpaces:0;
			/** we are decrementing the number of agents in lane infinity (of the first segment) to overcome problem [2] above**/
			outputEstimate = outputEstimate - virtualQueuesMap.at(lnk).size() - segStats->numAgentsInLane(segStats->laneInfinity); // decrement num. of agents already in virtual queue
			outputEstimate = (outputEstimate>0? outputEstimate : 0);
			vqBounds.insert(std::make_pair(lnk, (unsigned int)outputEstimate));
			vqCount += virtualQueuesMap.at(lnk).size();
		}
	}
	return vqCount;
}

bool sim_mob::Conflux::hasSpaceInVirtualQueue(sim_mob::Link* lnk) {
	bool res = false;
	{
		boost::unique_lock< boost::recursive_mutex > lock(mutexOfVirtualQueue);
		try {
			res = (vqBounds.at(lnk) > virtualQueuesMap.at(lnk).size());
		}
		catch(std::out_of_range& ex){
			debugMsgs << "out_of_range exception occured in hasSpaceInVirtualQueue()"
					<< "|Conflux: " << this->multiNode->getID()
					<< "|lnk:[" << lnk->getStart()->getID() << "," << lnk->getEnd()->getID() << "]"
					<< "|lnk:" << lnk
					<< "|virtualQueuesMap.size():" << virtualQueuesMap.size()
					<< "|elements:";
			for(std::map<sim_mob::Link*, std::deque<sim_mob::Person*> >::iterator i = virtualQueuesMap.begin(); i!= virtualQueuesMap.end(); i++) {
				debugMsgs << " ([" << i->first->getStart()->getID() << "," << i->first->getEnd()->getID() << "]:" << i->first << "," << i->second.size() << "),";
			}
			throw std::runtime_error(debugMsgs.str());
		}
	}
	return res;
}

void sim_mob::Conflux::pushBackOntoVirtualQueue(sim_mob::Link* lnk, sim_mob::Person* p) {
	{
		boost::unique_lock< boost::recursive_mutex > lock(mutexOfVirtualQueue);
		virtualQueuesMap.at(lnk).push_back(p);
	}
}

double sim_mob::Conflux::computeTimeToReachEndOfLink(sim_mob::SegmentStats* segStats, double distanceToEndOfSeg) const{
	if(!segStats) { return 0.0; }
	sim_mob::Link* link = segStats->getRoadSegment()->getLink();
	const SegmentStatsList& segmentStatsList = upstreamSegStatsMap.at(link);
	double timeToReachEndOfLink = distanceToEndOfSeg / getSegmentSpeed(segStats,true);
	SegmentStatsList::const_iterator segStatIt = std::find(segmentStatsList.begin(), segmentStatsList.end(), segStats);
	std::advance(segStatIt,1); // increment the iterator to the next
	for(; segStatIt!=segmentStatsList.end(); segStatIt++) {
		timeToReachEndOfLink += (*segStatIt)->getLength() / getSegmentSpeed((*segStatIt),true);
	}
	return timeToReachEndOfLink;
}

sim_mob::Person* sim_mob::Conflux::agentClosestToIntersection() {
	sim_mob::Person* person = nullptr;
	sim_mob::Person* candidatePerson = nullptr;
	sim_mob::SegmentStats* personSegStat = nullptr;
	sim_mob::SegmentStats* candidateStats = nullptr;
	double minTime = std::numeric_limits<double>::max();
	double timeToReachEndOfLink = 0;
	std::map<sim_mob::SegmentStats*, sim_mob::Person*>::iterator candidateAgentsIt = candidateAgents.begin();
	while (candidateAgentsIt != candidateAgents.end()) {
		candidatePerson = candidateAgentsIt->second;
		if (candidatePerson) {
			candidateStats = candidateAgentsIt->first;
			timeToReachEndOfLink = computeTimeToReachEndOfLink(candidateStats, candidatePerson->distanceToEndOfSegment);
			if (minTime == timeToReachEndOfLink) {
				// If current ag and (*i) are at equal distance to the intersection (end of the link), we toss a coin and choose one of them
				bool coinTossResult = ((rand() / (double) RAND_MAX) < 0.5);
				if (coinTossResult) {
					personSegStat = candidateStats;
					person = candidatePerson;
				}
			} else if (minTime > timeToReachEndOfLink) {
				minTime = timeToReachEndOfLink;
				personSegStat = candidateStats;
				person = candidatePerson;
			}
		}
		candidateAgentsIt++;
	}
	if (person) {
		candidateAgents.erase(personSegStat);
		const SegmentStatsList& segStatsList = upstreamSegStatsMap.at(personSegStat->getRoadSegment()->getLink());
		SegmentStatsList::const_iterator segStatsListIt = std::find(segStatsList.begin(), segStatsList.end(), personSegStat);
		sim_mob::Person* nextPerson = (*segStatsListIt)->agentClosestToStopLineFromFrontalAgents();
		while (!nextPerson && segStatsListIt != segStatsList.begin()) {
			currSegsOnUpLinks.erase((*segStatsListIt)->getRoadSegment()->getLink());
			segStatsListIt--;
			currSegsOnUpLinks.insert(std::make_pair((*segStatsListIt)->getRoadSegment()->getLink(), *segStatsListIt)); // No agents in the entire link
			(*segStatsListIt)->resetFrontalAgents();
			nextPerson = (*segStatsListIt)->agentClosestToStopLineFromFrontalAgents();
		}
		candidateAgents.insert(std::make_pair(*segStatsListIt, nextPerson));
	}
	return person;
}

void sim_mob::Conflux::updateAndReportSupplyStats(timeslice frameNumber) {
	for(UpstreamSegmentStatsMap::iterator upstreamIt = upstreamSegStatsMap.begin();
			upstreamIt != upstreamSegStatsMap.end(); upstreamIt++) {
		const SegmentStatsList& linkSegments = upstreamIt->second;
		for(SegmentStatsList::const_iterator segIt = linkSegments.begin();
				segIt != linkSegments.end(); segIt++) {
			(*segIt)->updateLaneParams(frameNumber);
			if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
				Log() << (*segIt)->reportSegmentStats(frameNumber);
			}
		}
	}
}

void sim_mob::Conflux::killAgent(sim_mob::Person* person,
		sim_mob::SegmentStats* prevSegStats, const sim_mob::Lane* prevLane,
		bool wasQueuing) {
	if (person->getRole() && person->getRole()->roleType==sim_mob::Role::RL_ACTIVITY) {
		PersonList::iterator pIt = std::find(activityPerformers.begin(), activityPerformers.end(), person);
		activityPerformers.erase(pIt);
	}
	else if (person->getRole() && person->getRole()->roleType==sim_mob::Role::RL_PEDESTRIAN) {
		PersonList::iterator pIt = std::find(pedestrianPerformers.begin(), pedestrianPerformers.end(), person);
		pedestrianPerformers.erase(pIt);
		if(person->getNextLinkRequired()){
			return;
		}
	}
	else if (prevLane) {
		prevSegStats->removeAgent(prevLane, person, wasQueuing);
	} /*else the person must have started from a VQ*/
	parentWorker->remEntity(person);
	parentWorker->scheduleForRemoval(person);
}

void sim_mob::Conflux::resetPositionOfLastUpdatedAgentOnLanes() {
	for(UpstreamSegmentStatsMap::iterator upstreamIt = upstreamSegStatsMap.begin();
			upstreamIt != upstreamSegStatsMap.end(); upstreamIt++) {
		const SegmentStatsList& linkSegments = upstreamIt->second;
		for(SegmentStatsList::const_iterator segIt = linkSegments.begin();
				segIt != linkSegments.end(); segIt++) {
			(*segIt)->resetPositionOfLastUpdatedAgentOnLanes();
		}
	}
}

sim_mob::SegmentStats* sim_mob::Conflux::findSegStats(const sim_mob::RoadSegment* rdSeg) {
	if(!rdSeg) {
		return nullptr;
	}
	std::map<const sim_mob::RoadSegment*, sim_mob::SegmentStats*>::iterator it;
	if(rdSeg->getParentConflux() == this){
		it = segmentAgents.find(rdSeg);
		return it->second;
	}
	else{ // if not found, search in downstreamSegments
		return rdSeg->getParentConflux()->findSegStats(rdSeg);
	}
}

void sim_mob::Conflux::setLinkTravelTimes(Person* person, double linkExitTime) {

	std::map<double, Person::linkTravelStats>::const_iterator it =
			person->getLinkTravelStatsMap().find(linkExitTime);
	if (it != person->getLinkTravelStatsMap().end()){
		double travelTime = (it->first) - (it->second).linkEntryTime_;
		std::map<const Link*, linkTravelTimes>::iterator itTT = LinkTravelTimesMap.find((it->second).link_);
		if (itTT != LinkTravelTimesMap.end())
		{
			itTT->second.agentCount_ = itTT->second.agentCount_ + 1;
			itTT->second.linkTravelTime_ = itTT->second.linkTravelTime_ + travelTime;
		}
		else{
			linkTravelTimes tTimes(travelTime, 1);
			LinkTravelTimesMap.insert(std::make_pair(person->getCurrSegStats()->getRoadSegment()->getLink(), tTimes));
		}
	}
}

bool sim_mob::Conflux::call_movement_frame_init(timeslice now, Person* person) {
	//Agents may be created with a null Role and a valid trip chain
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
		debugMsgs << "Person " << this->getId() <<  " has no Role.";
		throw std::runtime_error(debugMsgs.str());
	}

	//Get an UpdateParams instance.
	//TODO: This is quite unsafe, but it's a relic of how Person::update() used to work.
	//      We should replace this eventually (but this will require a larger code cleanup).
	(person->getRole()->make_frame_tick_params(now));

	//Now that the Role has been fully constructed, initialize it.
	if(*(person->currTripChainItem)) {
		person->getRole()->Movement()->frame_init();
		if(person->getCurrPath().empty()){
			return false;
		}
	}

	person->clearCurrPath();	//this will be set again for the next sub-trip
	return true;
}

void sim_mob::Conflux::onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args)
{
	sim_mob::Agent::onEvent(eventId, ctxId, sender, args);
}

void sim_mob::Conflux::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	if(type == MSG_PEDESTRIAN_TRANSFER_REQUEST){
		const Pedestrian_RequestMessageArgs& msg = MSG_CAST(Pedestrian_RequestMessageArgs, message);
		pedestrianPerformers.push_back(msg.pedestrian);
	}
}


Entity::UpdateStatus sim_mob::Conflux::call_movement_frame_tick(timeslice now, Person* person) {
	Role* personRole = person->getRole();
	if (person->isResetParamsRequired()) {
		personRole->make_frame_tick_params(now);
		person->setResetParamsRequired(false);
	}
	person->setLastUpdatedFrame(currFrameNumber.frame());

	Entity::UpdateStatus retVal(UpdateStatus::RS_CONTINUE);

	/*
	 * The following loop guides the movement of the person by invoking the movement facet of the person's role one or more times
	 * until the remainingTimeThisTick of the person is expired.
	 * The frame tick of the movement facet returns when one of the following conditions are true. These are handled by case distinction.
	 *
	 * 1. Driver's frame_tick() has displaced the person to the maximum distance that the person can move in the full tick duration.
	 * This case identified by checking if the remainingTimeThisTick of the person is 0.
	 * If remainingTimeThisTick == 0 we break off from the while loop.
	 * The person's location is updated in the conflux that it belongs to. If the person has to be removed from the simulation, he is.
	 *
	 * 2. The person has reached the end of a link.
	 * This case is identified by checking requestedNextSegment which indicates that the role has requested permission to move to the next segment in a new link in its path.
	 * The requested next segment will be assigned a segment by the mid-term driver iff the driver is moving into a new link.
	 * The conflux immediately grants permission by setting canMoveToNextSegment to GRANTED.
	 * If the next link is not processed for the current tick, the person is added to the virtual queue of the next conflux and the loop is broken.
	 * If the next link is processed, the loop continues. The movement role facet (driver) checks canMoveToNextSegment flag before it advances in its frame_tick.
	 *
	 * 3. The person has reached the end of the current subtrip. The loop will catch this by checking person->isToBeRemoved() flag.
	 * If the driver has reached the end of the current subtrip, the loop updates the current trip chain item of the person and change roles by calling person->checkTripChain().
	 * We also set the current segment, set the lane as lane infinity and call the movement facet of the person's role again.
	 */

	while(person->remainingTimeThisTick > 0.0) {
		if (!person->isToBeRemoved()) {
			personRole->Movement()->frame_tick();
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
					ap->setActivityStartTime(sim_mob::DailyTime(now.ms() + ConfigManager::GetInstance().FullConfig().baseGranMS()));
					ap->setActivityEndTime(sim_mob::DailyTime(now.ms() + ConfigManager::GetInstance().FullConfig().baseGranMS() + ((*person->currTripChainItem)->endTime.getValue() - (*person->currTripChainItem)->startTime.getValue())));
					call_movement_frame_init(now, person);
				}
				else if((*person->currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP) {
					if (call_movement_frame_init(now, person)){
						person->setCallFrameInit(false);
					}
					else{
						return UpdateStatus::Done;
					}
				}
			}
		}

		if(person->getNextLinkRequired()){
			Conflux* nextConflux = person->getNextLinkRequired()->getSegments().front()->getParentConflux();
			messaging::MessageBus::PostMessage(nextConflux, MSG_PEDESTRIAN_TRANSFER_REQUEST, messaging::MessageBus::MessagePtr(new Pedestrian_RequestMessageArgs(person)));
			person->setResetParamsRequired(true);
			return UpdateStatus::Done;
		}

		if(person->requestedNextSegStats){
			const sim_mob::RoadSegment* nxtSegment = person->requestedNextSegStats->getRoadSegment();
			Conflux* nxtConflux = nxtSegment->getParentConflux();

			// grant permission. But check whether the subsequent frame_tick can be called now.
			person->canMoveToNextSegment = Person::GRANTED;

			if(now.frame() > nxtConflux->getLastUpdatedFrame()) {
				// nxtConflux is not processed for the current tick yet
				if(nxtConflux->hasSpaceInVirtualQueue(nxtSegment->getLink())) {
					person->setCurrSegStats(person->requestedNextSegStats);
					person->setCurrLane(nullptr); // so that the updateAgent function will add this agent to the virtual queue
					person->requestedNextSegStats = nullptr;
					break; //break off from loop
				}
				else {
					person->canMoveToNextSegment = Person::DENIED;
					person->requestedNextSegStats = nullptr;
				}
			}
			else if(now.frame() == nxtConflux->getLastUpdatedFrame()) {
				// nxtConflux is processed for the current tick. Can move to the next link.
				// handled by setting person->canMoveToNextSegment = GRANTED
				person->requestedNextSegStats = nullptr;
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
		person->currRole->Movement()->frame_tick_output();
	}
}

void sim_mob::Conflux::reportLinkTravelTimes(timeslice frameNumber) {
	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
		std::map<const Link*, linkTravelTimes>::const_iterator it = LinkTravelTimesMap.begin();
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
	if (person->isCallFrameInit()) {
		//Call frame_init() and exit early if requested to.
		if (!call_movement_frame_init(now, person)) {
			return UpdateStatus::Done;
		}

		//Set call_frame_init to false here; you can only reset frame_init() in frame_tick()
		person->setCallFrameInit(false); //Only initialize once.
	}

	//Perform the main update tick
	UpdateStatus retVal = call_movement_frame_tick(now, person);

	//This persons next movement will be in the next tick
	if (retVal.status != UpdateStatus::RS_DONE && person->remainingTimeThisTick<=0) {
		//now is the right time to ask for resetting of updateParams
		person->setResetParamsRequired(true);
	}

	return retVal;
}

double sim_mob::Conflux::getPositionOfLastUpdatedAgentInLane(const Lane* lane) {
	return findSegStats(lane->getRoadSegment())->getPositionOfLastUpdatedAgentInLane(lane);
}

bool sim_mob::cmp_person_remainingTimeThisTick::operator ()
			(const Person* x, const Person* y) const {
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


void sim_mob::sortPersons_DecreasingRemTime(std::deque<Person*>& personList) {
	cmp_person_remainingTimeThisTick cmp_person_remainingTimeThisTick_obj;

	if(personList.size() > 1) { //ordering is required only if we have more than 1 person in the deque
		std::sort(personList.begin(), personList.end(), cmp_person_remainingTimeThisTick_obj);
	}
}

std::deque<sim_mob::Person*> sim_mob::Conflux::getAllPersons() {
	PersonList allPersonsInCfx, tmpAgents;
	sim_mob::SegmentStats* segStats = nullptr;
	for(UpstreamSegmentStatsMap::iterator upStrmSegMapIt = upstreamSegStatsMap.begin();
			upStrmSegMapIt!=upstreamSegStatsMap.end(); upStrmSegMapIt++) {
		const SegmentStatsList& upstreamSegments = upStrmSegMapIt->second;
		for(SegmentStatsList::const_iterator rdSegIt=upstreamSegments.begin();
				rdSegIt!=upstreamSegments.end(); rdSegIt++) {
			segStats = (*rdSegIt);
			tmpAgents = segStats->getPersons();
			allPersonsInCfx.insert(allPersonsInCfx.end(), tmpAgents.begin(), tmpAgents.end());
		}
	}

	for(VirtualQueueMap::iterator vqMapIt = virtualQueuesMap.begin();
			vqMapIt != virtualQueuesMap.end(); vqMapIt++) {
		tmpAgents = vqMapIt->second;
		allPersonsInCfx.insert(allPersonsInCfx.end(), tmpAgents.begin(), tmpAgents.end());
	}
	allPersonsInCfx.insert(allPersonsInCfx.end(), activityPerformers.begin(), activityPerformers.end());
	return allPersonsInCfx;
}

unsigned int sim_mob::Conflux::countPersons() {
	unsigned int numPersons = 0;
	for(UpstreamSegmentStatsMap::iterator upStrmSegMapIt = upstreamSegStatsMap.begin();
				upStrmSegMapIt!=upstreamSegStatsMap.end(); upStrmSegMapIt++) {
		const SegmentStatsList& upstreamSegments = upStrmSegMapIt->second;
		for(SegmentStatsList::const_iterator statsIt=upstreamSegments.begin();
				statsIt!=upstreamSegments.end(); statsIt++) {
			numPersons = numPersons + (*statsIt)->getNumPersons();
		}
	}
	return numPersons;
}

void sim_mob::Conflux::getAllPersonsUsingTopCMerge(std::deque<sim_mob::Person*>& mergedPersonDeque) {
	sim_mob::SegmentStats* segStats = nullptr;
	std::vector<PersonList> allPersonLists;
	int sumCapacity = 0;

	//need to calculate the time to intersection for each vehicle.
	//basic test-case shows that this calculation is kind of costly.
	for (UpstreamSegmentStatsMap::iterator upStrmSegMapIt = upstreamSegStatsMap.begin();
			upStrmSegMapIt != upstreamSegStatsMap.end(); upStrmSegMapIt++) {
		const SegmentStatsList& upstreamSegments = upStrmSegMapIt->second;
		double totalTimeToSegEnd = 0;
		std::deque<sim_mob::Person*> oneDeque;
		for (SegmentStatsList::const_reverse_iterator rdSegIt = upstreamSegments.rbegin();
				rdSegIt != upstreamSegments.rend(); rdSegIt++) {
			segStats = (*rdSegIt);
			double speed = segStats->getSegSpeed(true);
			//If speed is 0, treat it as a very small value
			if(speed < INFINITESIMAL_DOUBLE) {
				speed = INFINITESIMAL_DOUBLE;
			}
			segStats->updateLinkDrivingTimes(totalTimeToSegEnd);
			PersonList tmpAgents;
			segStats->topCMergeLanesInSegment(tmpAgents);
			totalTimeToSegEnd += segStats->getLength() / speed;
			oneDeque.insert(oneDeque.end(), tmpAgents.begin(), tmpAgents.end());
		}
		allPersonLists.push_back(oneDeque);
	}

	topCMergeDifferentLinksInConflux(mergedPersonDeque, allPersonLists, sumCapacity);
}

void sim_mob::Conflux::topCMergeDifferentLinksInConflux(std::deque<sim_mob::Person*>& mergedPersonDeque,
		std::vector< std::deque<sim_mob::Person*> >& allPersonLists, int capacity) {
	std::vector<std::deque<sim_mob::Person*>::iterator> iteratorLists;

	//init location
	size_t dequeSize = allPersonLists.size();
	for (std::vector<std::deque<sim_mob::Person*> >::iterator it = allPersonLists.begin(); it != allPersonLists.end(); ++it) {
		iteratorLists.push_back(((*it)).begin());
	}

	//pick the Top C
	for (size_t c = 0; c < capacity; c++) {
		int whichDeque = -1;
		double minDistance = std::numeric_limits<double>::max();
		sim_mob::Person* whichPerson = NULL;

		for (size_t i = 0; i < dequeSize; i++) {
			if (iteratorLists[i] != (allPersonLists[i]).end() && (*iteratorLists[i])->drivingTimeToEndOfLink < minDistance) {
				whichDeque = i;
				minDistance = (*iteratorLists[i])->drivingTimeToEndOfLink;
				whichPerson = (*iteratorLists[i]);
			}
		}

		if (whichDeque < 0) {
			return; //no more vehicles
		} else {
			iteratorLists[whichDeque]++;
			mergedPersonDeque.push_back(whichPerson);
		}
	}

	//After pick the Top C, there are still some vehicles left in the deque
	for (size_t i = 0; i < dequeSize; i++) {
		if (iteratorLists[i] != (allPersonLists[i]).end()) {
			mergedPersonDeque.insert(mergedPersonDeque.end(), iteratorLists[i], (allPersonLists[i]).end());
		}
	}
}

void sim_mob::Conflux::setRdSegTravelTimes(Person* person, double rdSegExitTime) {

	std::map<double, Person::rdSegTravelStats>::const_iterator it =
			person->getRdSegTravelStatsMap().find(rdSegExitTime);
	if (it != person->getRdSegTravelStatsMap().end()){
		double travelTime = (it->first) - (it->second).rdSegEntryTime_;
		std::map<const RoadSegment*, rdSegTravelTimes>::iterator itTT = RdSegTravelTimesMap.find((it->second).rdSeg_);
		if (itTT != RdSegTravelTimesMap.end())
		{
			itTT->second.agentCount_ = itTT->second.agentCount_ + 1;
			itTT->second.rdSegTravelTime_ = itTT->second.rdSegTravelTime_ + travelTime;
		}
		else{
			rdSegTravelTimes tTimes(travelTime, 1);
			RdSegTravelTimesMap.insert(std::make_pair(person->getCurrSegStats()->getRoadSegment(), tTimes));
		}
	}
}

void sim_mob::Conflux::resetRdSegTravelTimes(timeslice frameNumber) {
	RdSegTravelTimesMap.clear();
}

void sim_mob::Conflux::reportRdSegTravelTimes(timeslice frameNumber) {
	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
		std::map<const RoadSegment*, rdSegTravelTimes>::const_iterator it = RdSegTravelTimesMap.begin();
		for( ; it != RdSegTravelTimesMap.end(); ++it ) {
			LogOut("(\"rdSegTravelTime\""
				<<","<<frameNumber.frame()
				<<","<<it->first
				<<",{"
				<<"\"travelTime\":\""<< (it->second.rdSegTravelTime_)/(it->second.agentCount_)
				<<"\"})"<<std::endl);
		}
	}
	insertTravelTime2TmpTable(frameNumber, RdSegTravelTimesMap);
}

bool sim_mob::Conflux::insertTravelTime2TmpTable(timeslice frameNumber, std::map<const RoadSegment*, sim_mob::Conflux::rdSegTravelTimes>& rdSegTravelTimesMap)
{
	bool res=false;
	if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
		//sim_mob::Link_travel_time& data
		std::map<const RoadSegment*, sim_mob::Conflux::rdSegTravelTimes>::const_iterator it = rdSegTravelTimesMap.begin();
		for (; it != rdSegTravelTimesMap.end(); it++){
			Link_travel_time tt;
			DailyTime simStart = ConfigManager::GetInstance().FullConfig().simStartTime();
			std::string aimsun_id = (*it).first->originalDB_ID.getLogItem();
			std::string seg_id = getNumberFromAimsunId(aimsun_id);
			try {
				tt.link_id = boost::lexical_cast<int>(seg_id);
			} catch( boost::bad_lexical_cast const& ) {
				Print() << "Error: seg_id string was not valid" << std::endl;
				tt.link_id = -1;
			}

			tt.start_time = (simStart + sim_mob::DailyTime(frameNumber.ms())).toString();
			double frameLength = ConfigManager::GetInstance().FullConfig().baseGranMS();
			tt.end_time = (simStart + sim_mob::DailyTime(frameNumber.ms() + frameLength)).toString();
			tt.travel_time = (*it).second.rdSegTravelTime_/(*it).second.agentCount_;

			PathSetManager::getInstance()->insertTravelTime2TmpTable(tt);
		}
	}
	return res;
}

void sim_mob::Conflux::findBoundaryConfluxes() {

	sim_mob::Worker* firstUpstreamWorker = nullptr;
	std::map<const sim_mob::MultiNode*, sim_mob::Conflux*>& multinode_confluxes = ConfigManager::GetInstanceRW().FullConfig().getConfluxNodes();

	for (UpstreamSegmentStatsMap::iterator i = upstreamSegStatsMap.begin(); i != upstreamSegStatsMap.end(); i++) {
		const MultiNode* upnode = dynamic_cast<const MultiNode*> (i->first->getStart());

		if(upnode){
			std::map<const sim_mob::MultiNode*, sim_mob::Conflux*>::iterator confIt = multinode_confluxes.find(upnode);
			if (confIt != multinode_confluxes.end()){
				//check if upstream conflux belongs to another worker
				if (confIt->second->getParentWorker() != this->getParentWorker()){
					if( !isBoundary){
						isBoundary = true;
						firstUpstreamWorker = confIt->second->getParentWorker();
					}
					else if(confIt->second->getParentWorker() != firstUpstreamWorker && firstUpstreamWorker){
							isMultipleReceiver = true;
							return;
					}
				}
			}
		}
	}
}

unsigned int sim_mob::Conflux::getNumRemainingInLaneInfinity() {
	unsigned int count = 0;
	sim_mob::SegmentStats* segStats = nullptr;
	for(UpstreamSegmentStatsMap::iterator upStrmSegMapIt = upstreamSegStatsMap.begin();
			upStrmSegMapIt!=upstreamSegStatsMap.end(); upStrmSegMapIt++) {
		const SegmentStatsList& segStatsList = upStrmSegMapIt->second;
		for(SegmentStatsList::const_iterator statsIt=segStatsList.begin();
				statsIt!=segStatsList.end(); statsIt++) {
			segStats = (*statsIt);
			count += segStats->numAgentsInLane(segStats->laneInfinity);
		}
	}
	return count;
}

const sim_mob::RoadSegment* sim_mob::Conflux::constructPath(Person* p) {
	std::vector<sim_mob::TripChainItem*> agTripChain = p->getTripChain();
	const sim_mob::TripChainItem* firstItem = agTripChain.front();

	const RoleFactory& rf = ConfigManager::GetInstance().FullConfig().getRoleFactory();
	std::string role = rf.GetTripChainMode(firstItem);

	StreetDirectory& stdir = StreetDirectory::instance();

	std::vector<WayPoint> path;
	const sim_mob::RoadSegment* rdSeg = nullptr;

	if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
		path = PathSetManager::getInstance()->getPathByPerson(p);
	}
	else{
		if (role == "driver") {
			const sim_mob::SubTrip firstSubTrip = dynamic_cast<const sim_mob::Trip*>(firstItem)->getSubTrips().front();
			path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*firstSubTrip.fromLocation.node_), stdir.DrivingVertex(*firstSubTrip.toLocation.node_));
		}
		else if (role == "pedestrian") {
			const sim_mob::SubTrip firstSubTrip = dynamic_cast<const sim_mob::Trip*>(firstItem)->getSubTrips().front();
			path = stdir.SearchShortestWalkingPath(stdir.WalkingVertex(*firstSubTrip.fromLocation.node_), stdir.WalkingVertex(*firstSubTrip.toLocation.node_));
		}
		else if (role == "busdriver") {
			//throw std::runtime_error("Not implemented. BusTrip is not in master branch yet");
			const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(p->currTripChainItem));
			std::vector<const RoadSegment*> pathRoadSeg = bustrip->getBusRouteInfo().getRoadSegments();
			std::cout << "BusTrip path size = " << pathRoadSeg.size() << std::endl;
			std::vector<const RoadSegment*>::iterator itor;
			for(itor=pathRoadSeg.begin(); itor!=pathRoadSeg.end(); itor++){
				path.push_back(WayPoint(*itor));
			}
		}
	}

	if(path.size() > 0) {
		/* Drivers generated through xml input file, gives path as: O-Node, segment-list, D-node.
		 * BusDriver code, and pathSet code, generates only segment-list. Therefore we traverse through
		 * the path until we find the first road segment.
		 */
		p->setCurrPath(path);
		for (std::vector<WayPoint>::iterator it = path.begin(); it != path.end(); it++) {
			if (it->type_ == WayPoint::ROAD_SEGMENT) {
					rdSeg = it->roadSegment_;
					break;
			}
		}
	}
	return rdSeg;
}
