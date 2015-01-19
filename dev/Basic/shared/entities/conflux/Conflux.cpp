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
#include <stdint.h>
#include <vector>
#include "boost/lexical_cast.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Person.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/vehicle/VehicleBase.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/MultiNode.hpp"
#include "path/PathSetManager.hpp"
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
using namespace std;
using namespace boost;
typedef Entity::UpdateStatus UpdateStatus;

namespace{
sim_mob::BasicLogger & pathsetLogger = sim_mob::Logger::log("path_set");
}
namespace{
    const double INFINITESIMAL_DOUBLE = 0.000001;
    const double PASSENGER_CAR_UNIT = 400.0; //cm; 4 m.
}

sim_mob::Conflux::Conflux(sim_mob::MultiNode* multinode, const MutexStrategy& mtxStrat, int id)
: Agent(mtxStrat, id), multiNode(multinode), signal(StreetDirectory::instance().signalAt(*multinode)),
parentWorker(nullptr), currFrame(0,0), debugMsgs(std::stringstream::out), isBoundary(false), isMultipleReceiver(false)
{}

sim_mob::Conflux::~Conflux()
{
	//delete all SegmentStats in this conflux
	for(UpstreamSegmentStatsMap::iterator upstreamIt = upstreamSegStatsMap.begin();
			upstreamIt != upstreamSegStatsMap.end(); upstreamIt++) {
		const SegmentStatsList& linkSegments = upstreamIt->second;
		for(SegmentStatsList::const_iterator segIt = linkSegments.begin();
				segIt != linkSegments.end(); segIt++) {
			safe_delete_item(*segIt);
		}
	}
	// clear activity performers
	activityPerformers.clear();
	//clear pedestrian list
	pedestrianList.clear();
}

sim_mob::Conflux::PersonProps::PersonProps(const sim_mob::Person* person) {
	sim_mob::Role* role = person->getRole();
	isMoving = true;
	roleType = 0;
	if(role ){
		if(role->getResource()){
			isMoving = role->getResource()->isMoving();
		}
		roleType = role->roleType;
	}

	lane = person->getCurrLane();
	isQueuing = person->isQueuing;
	const sim_mob::SegmentStats* currSegStats = person->getCurrSegStats();
	if(currSegStats) {
		segment = currSegStats->getRoadSegment();
		segStats = segment->getParentConflux()->findSegStats(segment, currSegStats->getStatsNumberInSegment()); //person->getCurrSegStats() cannot be used as it returns a const pointer
	}
	else {
		segment = nullptr;
		segStats = nullptr;
	}
}

void sim_mob::Conflux::addAgent(sim_mob::Person* person, const sim_mob::RoadSegment* rdSeg) {

	Role* role = person->getRole();
	if(!role){
		UpdateStatus res = person->checkTripChain();
		if(res.status==UpdateStatus::RS_DONE){
			return;
		}
		role = person->getRole();
	}

	switch(role->roleType){
	case Role::RL_DRIVER:
	case Role::RL_BUSDRIVER:
	{
		/*
		 * Persons start at a node (for now).
		 * we will always add the Person to the corresponding segment stats in "lane infinity".
		 */
		SegmentStatsMap::iterator it = segmentAgents.find(rdSeg);
		if(it!=segmentAgents.end()){
			SegmentStatsList& statsList = it->second;
			sim_mob::SegmentStats* rdSegStats = statsList.front(); // we will start the person at the first segment stats of the segment
			person->setCurrSegStats(rdSegStats);
			person->setCurrLane(rdSegStats->laneInfinity);
			person->distanceToEndOfSegment = rdSegStats->getLength();
			person->remainingTimeThisTick = ConfigManager::GetInstance().FullConfig().baseGranSecond();
			rdSegStats->addAgent(rdSegStats->laneInfinity, person);
		}
		break;
	}
	case Role::RL_PEDESTRIAN:
	{
		pedestrianList.push_back(person);
		break;
	}
	case Role::RL_WAITBUSACTITITY:
	{
		assignPersonToBusStopAgent(person);
		break;
	}
	}
}

bool sim_mob::Conflux::frame_init(timeslice now)
{
	messaging::MessageBus::RegisterHandler(this);
	for(UpstreamSegmentStatsMap::iterator upstreamIt=upstreamSegStatsMap.begin(); upstreamIt!=upstreamSegStatsMap.end(); upstreamIt++)
	{
		const SegmentStatsList& linkSegments = upstreamIt->second;
		for(SegmentStatsList::const_iterator segIt=linkSegments.begin(); segIt!=linkSegments.end(); segIt++)
		{
			(*segIt)->initializeBusStops();
		}
	}
	/**************test code insert incident *********************/

	/*************************************************************/
	return true;
}

UpdateStatus sim_mob::Conflux::update(timeslice frameNumber) {
	if(!isInitialized()) {
		frame_init(frameNumber);
		setInitialized(true);
	}


	currFrame = frameNumber;
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

void sim_mob::Conflux::updateUnsignalized()
{
	//merge vehicles on conflux
	PersonList orderedPersonsInLanes;
	getAllPersonsUsingTopCMerge(orderedPersonsInLanes);

	PersonList::iterator personIt = orderedPersonsInLanes.begin();
	for (; personIt != orderedPersonsInLanes.end(); personIt++)
	{
		updateAgent(*personIt);
	}

	// We may have to erase persons in activityPerformers & pedestrianList in
	// updateAgent(). Therefore we need to iterate on a copy.
	PersonList activityPerformersCopy = activityPerformers;
	for (PersonList::iterator i = activityPerformersCopy.begin(); i != activityPerformersCopy.end(); i++)
	{
		updateAgent(*i);
	}

	PersonList pedestrianListCopy = pedestrianList;
	for (PersonList::iterator i = pedestrianListCopy.begin(); i != pedestrianListCopy.end(); i++)
	{
		updateAgent(*i);
	}

	updateBusStopAgents();
}

void sim_mob::Conflux::updateAgent(sim_mob::Person* person)
{
	if (person->getLastUpdatedFrame() < currFrame.frame())
	{	//if the person is being moved for the first time in this tick, reset person's remaining time to full tick size
		person->remainingTimeThisTick = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	}

	//let the person know which worker is (indirectly) managing him
	person->currWorkerProvider = parentWorker;

	//capture person info before update
	PersonProps beforeUpdate(person);

	//let the person move
	UpdateStatus res = movePerson(currFrame, person);

	//kill person if he's DONE
	if (res.status == UpdateStatus::RS_DONE) { killAgent(person, beforeUpdate); return;	}

	//capture person info after update
	PersonProps afterUpdate(person);

	//perform house keeping
	housekeep(beforeUpdate, afterUpdate, person);
}

void sim_mob::Conflux::housekeep(PersonProps& beforeUpdate, PersonProps& afterUpdate, Person* person)
{
	//if the person was in an activity and is in a Trip/SubTrip after update
	if (beforeUpdate.roleType == sim_mob::Role::RL_ACTIVITY && afterUpdate.roleType != sim_mob::Role::RL_ACTIVITY)
	{
		// if the person has changed from an Activity to the current Trip/SubTrip during this tick,
		// remove this person from the activityPerformers list
		std::deque<Person*>::iterator pIt = std::find(activityPerformers.begin(), activityPerformers.end(), person);
		if(pIt!=activityPerformers.end()) { activityPerformers.erase(pIt); }

		//if the person has switched to any role which is tracked in special lists in the conflux, put the person in that list
		if (afterUpdate.roleType == sim_mob::Role::RL_PEDESTRIAN)
		{
			pedestrianList.push_back(person);
			return; //we are done here.
		}
	}

	//perform person's role related handling
	//we first handle roles which are off the road
	switch(afterUpdate.roleType)
	{
	case sim_mob::Role::RL_WAITBUSACTITITY:
	{
		return;
	}
	case sim_mob::Role::RL_ACTIVITY:
	{	//activity role specific handling
		// if role is ActivityPerformer after update
		if (beforeUpdate.roleType == sim_mob::Role::RL_ACTIVITY)
		{
			// if the role was ActivityPerformer before the update as well, do
			// nothing. It is also possible that the person has changed from
			// one activity to another. Do nothing even in this case.
		}
		else
		{
			if(beforeUpdate.roleType == sim_mob::Role::RL_PEDESTRIAN)
			{
				PersonList::iterator pIt = std::find(pedestrianList.begin(), pedestrianList.end(), person);
				if(pIt!=pedestrianList.end()){ pedestrianList.erase(pIt); }
			}
			else if (beforeUpdate.lane)
			{
				// the person currently in an activity, was in a Trip
				// before this tick and was not in a virtual queue (because beforeUpdate.lane is not null)
				// Remove this person from the network and add him to the activity performers list.
				beforeUpdate.segStats->dequeue(person, beforeUpdate.lane, beforeUpdate.isQueuing);
			}
			activityPerformers.push_back(person);
		}
		return;
	}
	case sim_mob::Role::RL_PEDESTRIAN:
	{
		if(beforeUpdate.roleType == sim_mob::Role::RL_PEDESTRIAN) { return;	}
		break;
	}
	case sim_mob::Role::RL_BUSDRIVER:
	{
		if (beforeUpdate.isMoving && !afterUpdate.isMoving)
		{
			//if the vehicle stopped moving during the latest update (which
			//indicates that the bus has started serving a stop) we remove the bus from
			//segment stats
			//NOTE: the bus driver we remove here would have already been added
			//to the BusStopAgent corresponding to the stop currently served by
			//the bus driver.
			if (beforeUpdate.lane)
			{
				beforeUpdate.segStats->dequeue(person, beforeUpdate.lane, beforeUpdate.isQueuing);
			}
			//if the bus driver started moving from a virtual queue, his beforeUpdate.lane will be null.
			//However, since he is already into a bus stop (afterUpdate.isMoving is false) we need not
			// add this bus driver to the new seg stats. So we must return from here in any case.
			return;
		}
		else if (!beforeUpdate.isMoving && afterUpdate.isMoving)
		{
			//if the vehicle has started moving during the latest update (which
			//indicates that the bus has finished serving a stop and is getting
			//back into the road network) we add the bus driver to the new segment
			//stats
			//NOTE: the bus driver we add here would have already been removed
			//from the BusStopAgent corresponding to the stop served by the
			//bus driver.
			if(afterUpdate.lane)
			{
				afterUpdate.segStats->addAgent(afterUpdate.lane, person);
				return;
			}
		}
		else if (!beforeUpdate.isMoving && !afterUpdate.isMoving && beforeUpdate.segStats != afterUpdate.segStats)
		{
			//The bus driver has moved out of one stop and entered another within the same tick
			//we should not add the bus driver into the new segstats because he is already at the bus stop of that stats
			//we simply return in this case
			return;
		}
		break;
	}
	}

	//now we consider roles on the road
	//note: A person is in the virtual queue or performing and activity if beforeUpdate.lane is null
	if (!beforeUpdate.lane)
	{ 	//if the person was in virtual queue or was performing an activity
		if (afterUpdate.lane)
		{ 	//if the person has moved to another lane (possibly even to laneInfinity if he was performing activity) in some segment
			afterUpdate.segStats->addAgent(afterUpdate.lane, person);
		}
		else
		{
			if (beforeUpdate.segStats != afterUpdate.segStats)
			{
				// the person must've have moved to another virtual queue - which is not possible if the virtual queues are processed
				// after all conflux updates
				debugMsgs << "Error: Person has moved from one virtual queue to another. "
						<< "\n Person " << person->getId()
						<< "|Frame: " << currFrame.frame()
						<< "|Conflux: " << this->multiNode->getID()
						<< "|segBeforeUpdate: " << beforeUpdate.segment->getStartEnd() << "|segAfterUpdate: " << afterUpdate.segment->getStartEnd();
				throw std::runtime_error(debugMsgs.str());
			}
			else
			{
				// this is typically the person who was not accepted by the next lane in the next segment.
				// we push this person back to the same virtual queue and let him update in the next tick.
				person->distanceToEndOfSegment = afterUpdate.segStats->getLength();
				afterUpdate.segment->getParentConflux()->pushBackOntoVirtualQueue(afterUpdate.segment->getLink(), person);
			}
		}
	}
	else if ((beforeUpdate.segStats != afterUpdate.segStats) /*if the person has moved to another segment*/
				|| (beforeUpdate.lane == beforeUpdate.segStats->laneInfinity
						&& beforeUpdate.lane != afterUpdate.lane) /* or if the person has moved out of lane infinity*/)
	{
		if(beforeUpdate.roleType!=sim_mob::Role::RL_ACTIVITY)
		{
			// the person could have been an activity performer in which case beforeUpdate.segStats would be just null
			beforeUpdate.segStats->dequeue(person, beforeUpdate.lane, beforeUpdate.isQueuing);
		}
		if (afterUpdate.lane)
		{
			afterUpdate.segStats->addAgent(afterUpdate.lane, person);
		}
		else
		{
			// we wouldn't know which lane the person has to go to if the person wants to enter a link which belongs to
			// a conflux that is not yet processed for this tick. We add this person to the virtual queue for that link here
			person->distanceToEndOfSegment = afterUpdate.segStats->getLength();
			afterUpdate.segment->getParentConflux()->pushBackOntoVirtualQueue(afterUpdate.segment->getLink(), person);
		}
	}
	else if (beforeUpdate.segStats == afterUpdate.segStats && afterUpdate.lane == afterUpdate.segStats->laneInfinity)
	{
		//it's possible for some persons to start a new trip on the same segment where they ended the previous trip.
		beforeUpdate.segStats->dequeue(person, beforeUpdate.lane, beforeUpdate.isQueuing);
		//adding the person to lane infinity for the new trip
		afterUpdate.segStats->addAgent(afterUpdate.lane, person);
	}
	else if (beforeUpdate.isQueuing != afterUpdate.isQueuing)
	{
		//the person has joined the queuing part of the same segment stats
		afterUpdate.segStats->updateQueueStatus(afterUpdate.lane, person);
	}

	// set the position of the last updated Person in his current lane (after update)
	if (afterUpdate.lane && afterUpdate.lane != afterUpdate.segStats->laneInfinity)
	{
		//if the person did not end up in a VQ and his lane is not lane infinity of segAfterUpdate
		afterUpdate.segStats->setPositionOfLastUpdatedAgentInLane(person->distanceToEndOfSegment, afterUpdate.lane);
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
			sim_mob::Person* personClosestToIntersection = currSegStatOnLnk->personClosestToSegmentEnd();
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
				if ((*personIt)->getLastUpdatedFrame() < currFrame.frame()) {
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
				if ((*pIt)->getLastUpdatedFrame() < currFrame.frame()) {
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
		boost::unique_lock< boost::recursive_mutex > lock(mutexOfVirtualQueue);
		unsigned int vqCount = 0;
		vqBounds.clear();
		sim_mob::Link* lnk = nullptr;
		sim_mob::SegmentStats* segStats = nullptr;
		int outputEstimate = 0;
		for(VirtualQueueMap::iterator i = virtualQueuesMap.begin(); i != virtualQueuesMap.end(); i++) {
			lnk = i->first;
			segStats = upstreamSegStatsMap.at(lnk).front();
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
					- segStats->numMovingInSegment(true) - segStats->numQueuingInSegment(true);
			outputEstimate = (num_emptySpaces>=0)? num_emptySpaces:0;
			/** we are decrementing the number of agents in lane infinity (of the first segment) to overcome problem [2] above**/
			outputEstimate = outputEstimate - virtualQueuesMap.at(lnk).size() - segStats->numAgentsInLane(segStats->laneInfinity); // decrement num. of agents already in virtual queue
			outputEstimate = (outputEstimate>0? outputEstimate : 0);
			vqBounds.insert(std::make_pair(lnk, (unsigned int)outputEstimate));
			vqCount += virtualQueuesMap.at(lnk).size();
		}//loop

		if(vqBounds.empty() && !virtualQueuesMap.empty()){
			Print() << boost::this_thread::get_id() << "," << this->multiNode->getID() << " vqBounds.empty()" << std::endl;
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
			debugMsgs  << boost::this_thread::get_id() << " out_of_range exception occured in hasSpaceInVirtualQueue()"
					<< "|Conflux: " << this->multiNode->getID()
					<< "|lnk:[" << lnk->getStart()->getID() << "," << lnk->getEnd()->getID() << "]"
					<< "|lnk:" << lnk
					<< "|virtualQueuesMap.size():" << virtualQueuesMap.size()
					<< "|elements:";
			for(std::map<sim_mob::Link*, std::deque<sim_mob::Person*> >::iterator i = virtualQueuesMap.begin(); i!= virtualQueuesMap.end(); i++) {
				debugMsgs << " ([" << i->first->getStart()->getID() << "," << i->first->getEnd()->getID() << "]:" << i->first << "," << i->second.size() << "),";
			}
			debugMsgs << "|\nvqBounds.size(): " << vqBounds.size() << std::endl;
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
		sim_mob::Person* nextPerson = (*segStatsListIt)->personClosestToSegmentEnd();
		while (!nextPerson && segStatsListIt != segStatsList.begin()) {
			currSegsOnUpLinks.erase((*segStatsListIt)->getRoadSegment()->getLink());
			segStatsListIt--;
			currSegsOnUpLinks.insert(std::make_pair((*segStatsListIt)->getRoadSegment()->getLink(), *segStatsListIt)); // No agents in the entire link
			(*segStatsListIt)->resetFrontalAgents();
			nextPerson = (*segStatsListIt)->personClosestToSegmentEnd();
		}
		candidateAgents.insert(std::make_pair(*segStatsListIt, nextPerson));
	}
	return person;
}

void sim_mob::Conflux::updateAndReportSupplyStats(timeslice frameNumber) {
	const ConfigManager& cfg = ConfigManager::GetInstance();
	bool outputEnabled = cfg.CMakeConfig().OutputEnabled();
	uint32_t updtInterval = boost::lexical_cast<uint32_t>(cfg.FullConfig().system.genericProps.at("update_interval"));
	bool updateThisTick = ((frameNumber.frame() % updtInterval)==0);
	for(UpstreamSegmentStatsMap::iterator upstreamIt = upstreamSegStatsMap.begin(); upstreamIt != upstreamSegStatsMap.end(); upstreamIt++)
	{
		const SegmentStatsList& linkSegments = upstreamIt->second;
		for(SegmentStatsList::const_iterator segIt = linkSegments.begin(); segIt != linkSegments.end(); segIt++)
		{
			(*segIt)->updateLaneParams(frameNumber);
			if (updateThisTick && outputEnabled)
			{
				Log() << (*segIt)->reportSegmentStats(frameNumber.frame()/updtInterval);
			}
		}
	}
}

void sim_mob::Conflux::killAgent(sim_mob::Person* person, PersonProps& beforeUpdate)
{
	sim_mob::SegmentStats* prevSegStats = beforeUpdate.segStats;
	const sim_mob::Lane* prevLane = beforeUpdate.lane;
	bool wasQueuing = beforeUpdate.isQueuing;
	sim_mob::Role::type personRoleType = sim_mob::Role::RL_UNKNOWN;
	if(person->getRole()) { personRoleType = person->getRole()->roleType; }
	switch(personRoleType)
	{
	case sim_mob::Role::RL_ACTIVITY:
	{
		PersonList::iterator pIt = std::find(activityPerformers.begin(), activityPerformers.end(), person);
		if(pIt!=activityPerformers.end()){
			activityPerformers.erase(pIt);
		}
		break;
	}
	case sim_mob::Role::RL_PEDESTRIAN:
	{
		PersonList::iterator pIt = std::find(pedestrianList.begin(), pedestrianList.end(), person);
		if (pIt != pedestrianList.end())
		{
			pedestrianList.erase(pIt);
		}
		if (person->getNextLinkRequired()) { return; }
		break;
	}
	case sim_mob::Role::RL_DRIVER:
	{
		//It is possible that a driver is getting removed silently because
		//a path could not be established for his current sub trip.
		//In this case, the role will be Driver but the prevLane and prevSegStats will be NULL
		//if the person's previous trip chain item is an Activity.
		//TODO: There might be other weird scenarios like this, to be taken care of.
		PersonList::iterator pIt = std::find(activityPerformers.begin(), activityPerformers.end(), person);
		if(pIt!=activityPerformers.end()) {	activityPerformers.erase(pIt); } //Check if he was indeed an activity performer and erase him
		else if (prevLane)
		{
			bool removed = prevSegStats->removeAgent(prevLane, person, wasQueuing);
			if(!removed) { throw std::runtime_error("Conflux::killAgent(): Attempt to remove non-existent person in Lane");	}
		}
		break;
	}
	default: //applies for any other vehicle in a lane (Biker, Busdriver etc.)
	{
		if (prevLane)
		{
			bool removed = prevSegStats->removeAgent(prevLane, person, wasQueuing);
			//removed can be false only in the case of BusDrivers at the moment.
			//This is because a BusDriver could have been dequeued from prevLane in the previous tick and be added to his
			//last bus stop. When he has finished serving the stop, the BusDriver is done. He will be killed here. However,
			//since he was already dequeued, we can't find him in prevLane now.
			//It is an error only if removed is false and the role is not BusDriver.
			if(!removed && personRoleType != sim_mob::Role::RL_BUSDRIVER)
			{
				throw std::runtime_error("Conflux::killAgent(): Attempt to remove non-existent person in Lane");
			}
		}
		break;
	}
	}
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

sim_mob::SegmentStats* sim_mob::Conflux::findSegStats(const sim_mob::RoadSegment* rdSeg, uint16_t statsNum) {
	if(!rdSeg || statsNum == 0) {
		return nullptr;
	}
	SegmentStatsList& statsList = segmentAgents.find(rdSeg)->second;
	if(statsList.size() < statsNum) {
		return nullptr;
	}
	SegmentStatsList::iterator statsIt = statsList.begin();
	if(statsNum == 1) {
		return (*statsIt);
	}
	std::advance(statsIt, (statsNum-1));
	return (*statsIt);
}

const std::vector<sim_mob::SegmentStats*>& sim_mob::Conflux::findSegStats(const sim_mob::RoadSegment* rdSeg){
	return (segmentAgents.find(rdSeg)->second);
}

void sim_mob::Conflux::setLinkTravelTimes(Person* person, double linkExitTime) {

	std::map<double, Person::linkTravelStats>::const_iterator it =
			person->getLinkTravelStatsMap().find(linkExitTime);
	if (it != person->getLinkTravelStatsMap().end()){
		double travelTime = (it->first) - (it->second).entryTime;
		std::map<const Link*, LinkTravelTimes>::iterator itTT = LinkTravelTimesMap.find((it->second).link_);
		if (itTT != LinkTravelTimesMap.end())
		{
			itTT->second.agCnt = itTT->second.agCnt + 1;
			itTT->second.linkTravelTime_ = itTT->second.linkTravelTime_ + travelTime;
		}
		else{
			LinkTravelTimes tTimes(travelTime, 1);
			LinkTravelTimesMap.insert(std::make_pair(person->getCurrSegStats()->getRoadSegment()->getLink(), tTimes));
		}
	}
}

bool sim_mob::Conflux::callMovementFrameInit(timeslice now, Person* person) {
	//Agents may be created with a null Role and a valid trip chain
	if (!person->getRole()) {
		//TODO: This UpdateStatus has a "prevParams" and "currParams" that should
		//      (one would expect) be dealt with. Where does this happen?
		UpdateStatus res =	person->checkTripChain();

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
	person->getRole()->make_frame_tick_params(now);

	//Now that the Role has been fully constructed, initialize it.
	if(person->getRole()) {
		person->getRole()->Movement()->frame_init();

		if(person->getRole()->roleType == sim_mob::Role::RL_DRIVER && person->getCurrPath().empty()){
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
	switch(type) {
	case MSG_PEDESTRIAN_TRANSFER_REQUEST:
	{
		const PedestrianTransferRequestMessage& msg = MSG_CAST(PedestrianTransferRequestMessage, message);
		msg.pedestrian->currWorkerProvider = parentWorker;
		pedestrianList.push_back(msg.pedestrian);
		break;
	}
	case MSG_INSERT_INCIDENT:
	{
		pathsetLogger << "Conflux received MSG_INSERT_INCIDENT" << std::endl;
		const InsertIncidentMessage & msg = MSG_CAST(InsertIncidentMessage, message);
		//change the flow rate of the segment
		sim_mob::Conflux::insertIncident(msg.stats,msg.newFlowRate);
		//tell
	}
	default:
		break;
	}
}


Entity::UpdateStatus sim_mob::Conflux::callMovementFameTick(timeslice now, Person* person) {
	Role* personRole = person->getRole();
	if (person->isResetParamsRequired()) {
		personRole->make_frame_tick_params(now);
		person->setResetParamsRequired(false);
	}
	person->setLastUpdatedFrame(currFrame.frame());

	Entity::UpdateStatus retVal = UpdateStatus::Continue;

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
	unsigned i=0;
	while(person->remainingTimeThisTick > 0.0) {
		if (!person->isToBeRemoved()) {
			personRole->Movement()->frame_tick();
		}

		if (person->isToBeRemoved()) {
			retVal = person->checkTripChain();
			personRole = person->getRole();
			if (retVal.status == UpdateStatus::RS_DONE) {
				return retVal;
			}
			else if(personRole && retVal.status==UpdateStatus::RS_CONTINUE && personRole->roleType==Role::RL_WAITBUSACTITITY) {
				assignPersonToBusStopAgent(person);
				PersonList::iterator pIt = std::find(pedestrianList.begin(), pedestrianList.end(), person);
				if(pIt!=pedestrianList.end()){
					pedestrianList.erase(pIt);
				}
				return retVal;
			}

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
					if (callMovementFrameInit(now, person)){
						person->setInitialized(true);
					}
					else{
						return UpdateStatus::Done;
					}
				}
				else if((*person->currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP) {
					if (callMovementFrameInit(now, person)) {
						person->setInitialized(true);
					}
					else{
						return UpdateStatus::Done;
					}
				}
			}
		}

		if(person->getNextLinkRequired()){
			Conflux* nextConflux = person->getNextLinkRequired()->getSegments().front()->getParentConflux();
			messaging::MessageBus::PostMessage(nextConflux, MSG_PEDESTRIAN_TRANSFER_REQUEST,
					messaging::MessageBus::MessagePtr(new PedestrianTransferRequestMessage(person)));
			PersonList::iterator pIt = std::find(pedestrianList.begin(), pedestrianList.end(), person);
			if(pIt!=pedestrianList.end()){
				pedestrianList.erase(pIt);
				person->currWorkerProvider = nullptr;
			}
			return UpdateStatus::Continue;
		}

		if(person->requestedNextSegStats){
			const sim_mob::RoadSegment* nxtSegment = person->requestedNextSegStats->getRoadSegment();
			Conflux* nxtConflux = nxtSegment->getParentConflux();

			// grant permission. But check whether the subsequent frame_tick can be called now.
			person->canMoveToNextSegment = Person::GRANTED;
			long currentFrame = now.frame(); //frame will not be outside the range of long data type
			if(currentFrame > nxtConflux->getLastUpdatedFrame()) {
				// nxtConflux is not processed for the current tick yet
				if(nxtConflux->hasSpaceInVirtualQueue(nxtSegment->getLink())) {
					person->setCurrSegStats(person->requestedNextSegStats);
					person->setCurrLane(nullptr); // so that the updateAgent function will add this agent to the virtual queue
//					Print() << "Conflux: " << this->multiNode->getID()
//							<< "|Person: " << person->getId()
//							<< " setting currLane to NULL to add to VQ"
//							<< "|requestedNextSeg: " << nxtSegment->getSegmentAimsunId()
//							<< "|statsNum: " << person->requestedNextSegStats->getStatsNumberInSegment()
//							<< std::endl;
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

void sim_mob::Conflux::callMovementFrameOutput(timeslice now, Person* person) {
	//Save the output
	if (!isToBeRemoved()) {
		person->currRole->Movement()->frame_tick_output();
	}
}

void sim_mob::Conflux::reportLinkTravelTimes(timeslice frameNumber) {
	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
		std::map<const Link*, LinkTravelTimes>::const_iterator it = LinkTravelTimesMap.begin();
		for( ; it != LinkTravelTimesMap.end(); ++it ) {
			LogOut("(\"linkTravelTime\""
				<<","<<frameNumber.frame()
				<<","<<it->first->getLinkId()
				<<",{"
				<<"\"travelTime\":\""<< (it->second.linkTravelTime_)/(it->second.agCnt)
				<<"\"})"<<std::endl);
		}
	}
}

void sim_mob::Conflux::resetLinkTravelTimes(timeslice frameNumber) {
	LinkTravelTimesMap.clear();
}

void sim_mob::Conflux::incrementSegmentFlow(const RoadSegment* rdSeg, uint16_t statsNum) {
	sim_mob::SegmentStats* segStats = findSegStats(rdSeg, statsNum);
	segStats->incrementSegFlow();
}

void sim_mob::Conflux::resetSegmentFlows() {
	for(UpstreamSegmentStatsMap::iterator upstreamIt = upstreamSegStatsMap.begin();
			upstreamIt != upstreamSegStatsMap.end(); upstreamIt++) {
		const SegmentStatsList& linkSegments = upstreamIt->second;
		for(SegmentStatsList::const_iterator segIt = linkSegments.begin();
				segIt != linkSegments.end(); segIt++) {
			(*segIt)->resetSegFlow();
		}
	}
}

void sim_mob::Conflux::updateBusStopAgents()
{
	for (UpstreamSegmentStatsMap::iterator upStrmSegMapIt = upstreamSegStatsMap.begin(); upStrmSegMapIt != upstreamSegStatsMap.end(); upStrmSegMapIt++)
	{
		for (std::vector<sim_mob::SegmentStats*>::const_iterator segStatsIt = upStrmSegMapIt->second.begin(); segStatsIt != upStrmSegMapIt->second.end();
				segStatsIt++)
		{
			(*segStatsIt)->updateBusStopAgents(currFrame);
		}
	}
}

void sim_mob::Conflux::assignPersonToBusStopAgent(Person* person)
{
	Role* role = person->getRole();
	if (role && role->roleType == Role::RL_WAITBUSACTITITY) {
		const BusStop* stop = nullptr;
		if (person->originNode.type_ == WayPoint::BUS_STOP) {
			stop = person->originNode.busStop_;
		}

		if(!stop){
			if(person->currSubTrip->fromLocation.type_==WayPoint::BUS_STOP) {
				stop = person->currSubTrip->fromLocation.busStop_;
			}
		}

		if (!stop) {
			return;
		}

		const StreetDirectory& strDirectory = StreetDirectory::instance();
		Agent* busStopAgent = strDirectory.findBusStopAgentByBusStop(stop);
		if (busStopAgent) {
			messaging::MessageBus::SendInstantaneousMessage(busStopAgent,
					MSG_WAITINGPERSON_ARRIVALAT_BUSSTOP,
					messaging::MessageBus::MessagePtr(
							new ArriavalAtStopMessage(person)));
		}
	}
}

UpdateStatus sim_mob::Conflux::movePerson(timeslice now, Person* person)
{
	// We give the Agent the benefit of the doubt here and simply call frame_init().
	// This allows them to override the start_time if it seems appropriate (e.g., if they
	// are swapping trip chains). If frame_init() returns false, immediately exit.
	if (!person->isInitialized()) {
		//Call frame_init() and exit early if required.
		if (!callMovementFrameInit(now, person)) {
			return UpdateStatus::Done;
		}

		//Set call_frame_init to false here; you can only reset frame_init() in frame_tick()
		person->setInitialized(true); //Only initialize once.
	}

	//Perform the main update tick
	UpdateStatus retVal = callMovementFameTick(now, person);

	//This persons next movement will be in the next tick
	if (retVal.status != UpdateStatus::RS_DONE && person->remainingTimeThisTick<=0) {
		//now is the right time to ask for resetting of updateParams
		person->setResetParamsRequired(true);
	}

	return retVal;
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
			segStats->getPersons(tmpAgents);
			allPersonsInCfx.insert(allPersonsInCfx.end(), tmpAgents.begin(), tmpAgents.end());
		}
	}

	for(VirtualQueueMap::iterator vqMapIt = virtualQueuesMap.begin();
			vqMapIt != virtualQueuesMap.end(); vqMapIt++) {
		tmpAgents = vqMapIt->second;
		allPersonsInCfx.insert(allPersonsInCfx.end(), tmpAgents.begin(), tmpAgents.end());
	}
	allPersonsInCfx.insert(allPersonsInCfx.end(), activityPerformers.begin(), activityPerformers.end());
	allPersonsInCfx.insert(allPersonsInCfx.end(), pedestrianList.begin(), pedestrianList.end());
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

void sim_mob::Conflux::getAllPersonsUsingTopCMerge(std::deque<sim_mob::Person*>& mergedPersonDeque)
{
	sim_mob::SegmentStats* segStats = nullptr;
	std::vector<PersonList> allPersonLists;
	int sumCapacity = 0;

	//need to calculate the time to intersection for each vehicle.
	//basic test-case shows that this calculation is kind of costly.
	for (UpstreamSegmentStatsMap::iterator upStrmSegMapIt = upstreamSegStatsMap.begin(); upStrmSegMapIt != upstreamSegStatsMap.end(); upStrmSegMapIt++)
	{
		const SegmentStatsList& upstreamSegments = upStrmSegMapIt->second;
		sumCapacity += (int)(ceil((*upstreamSegments.rbegin())->getRoadSegment()->getCapacityPerInterval()));
		double totalTimeToSegEnd = 0;
		std::deque<sim_mob::Person*> oneDeque;
		for (SegmentStatsList::const_reverse_iterator rdSegIt = upstreamSegments.rbegin(); rdSegIt != upstreamSegments.rend(); rdSegIt++)
		{
			segStats = (*rdSegIt);
			double speed = segStats->getSegSpeed(true);
			//If speed is 0, treat it as a very small value
			if (speed < INFINITESIMAL_DOUBLE)
			{
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
//
//void sim_mob::Conflux::addSegTT(Agent::RdSegTravelStat & stats, Person* person) {
//
//	TravelTimeManager::TR &timeRange = TravelTimeManager::getTimeInterval(stats.entryTime);
//	std::map<TravelTimeManager::TR,TravelTimeManager::TT>::iterator itTT = rdSegTravelTimesMap.find(timeRange);
//	TravelTimeManager::TT & travelTimeInfo = (itTT == rdSegTravelTimesMap.end() ? rdSegTravelTimesMap[timeRange] : itTT->second);
//	//initialization just in case
//	if(itTT == rdSegTravelTimesMap.end()){
//		travelTimeInfo[stats.rs].first = 0.0;
//		travelTimeInfo[stats.rs].second = 0;
//	}
//	travelTimeInfo[stats.rs].first += stats.travelTime; //add to total travel time
//	rdSegTravelTimesMap[timeRange][stats.rs].second ++; //increment the total contribution
//}
//
//void sim_mob::Conflux::resetRdSegTravelTimes() {
//	rdSegTravelTimesMap.clear();
//}
//
//void sim_mob::Conflux::reportRdSegTravelTimes(timeslice frameNumber) {
//	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
//		std::map<const RoadSegment*, RdSegTravelTimes>::const_iterator it = rdSegTravelTimesMap.begin();
//		for( ; it != rdSegTravelTimesMap.end(); ++it ) {
//			LogOut("(\"rdSegTravelTime\""
//				<<","<<frameNumber.frame()
//				<<","<<it->first
//				<<",{"
//				<<"\"travelTime\":\""<< (it->second.travelTimeSum)/(it->second.agCnt)
//				<<"\"})"<<std::endl);
//		}
//	}
////	if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
////		insertTravelTime2TmpTable(frameNumber, rdSegTravelTimesMap);
////	}
//}
//
//bool sim_mob::Conflux::insertTravelTime2TmpTable(timeslice frameNumber, std::map<const RoadSegment*, sim_mob::Conflux::RdSegTravelTimes>& rdSegTravelTimesMap)
//{
////	bool res=false;
////	//sim_mob::Link_travel_time& data
////	std::map<const RoadSegment*, sim_mob::Conflux::RdSegTravelTimes>::const_iterator it = rdSegTravelTimesMap.begin();
////	for (; it != rdSegTravelTimesMap.end(); it++){
////		LinkTravelTime tt;
////		const DailyTime &simStart = ConfigManager::GetInstance().FullConfig().simStartTime();
////		tt.linkId = (*it).first->getId();
////		tt.recordTime_DT = simStart + sim_mob::DailyTime(frameNumber.ms());
////		tt.travelTime = (*it).second.travelTimeSum/(*it).second.agCnt;
////		PathSetManager::getInstance()->insertTravelTime2TmpTable(tt);
////	}
////	return res;
//}

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
	const sim_mob::RoadSegment* rdSeg = nullptr;
	const std::vector<sim_mob::TripChainItem*> & agTripChain = p->getTripChain();
	sim_mob::TripChainItem *tci;
	BOOST_FOREACH(tci,agTripChain){
		if(tci->itemType == sim_mob::TripChainItem::IT_TRIP){
			break;
		}
	}
	//sanity check
	sim_mob::Trip *firstTrip = dynamic_cast<sim_mob::Trip*>(tci);
	if(!firstTrip){ return nullptr; }

	std::vector<WayPoint> path;
	
	const RoleFactory& rf = ConfigManager::GetInstance().FullConfig().getRoleFactory();

	bool pathSetRole = false;
	if(firstTrip)
	{
		std::string mode = firstTrip->getMode();
		pathSetRole = (mode == "Car" || mode == "Taxi" || mode == "Motorcycle") ;
	}
	if (firstTrip && ConfigManager::GetInstance().FullConfig().PathSetMode() && pathSetRole) {
		path = PathSetManager::getInstance()->getPath(p,firstTrip->getSubTrips().front());
	}
	else{
		const sim_mob::TripChainItem* firstItem = agTripChain.front();

		std::string role = rf.GetRoleName(firstItem->getMode()); //getMode is a virtual function. see its documentation
		StreetDirectory& streetDirectory = StreetDirectory::instance();

		if (role=="driver" || role=="biker")
		{
			const sim_mob::SubTrip firstSubTrip = dynamic_cast<const sim_mob::Trip*>(firstItem)->getSubTrips().front();
			path = streetDirectory.SearchShortestDrivingPath(streetDirectory.DrivingVertex(*firstSubTrip.fromLocation.node_), streetDirectory.DrivingVertex(*firstSubTrip.toLocation.node_));
		}
		else if (role == "pedestrian")
		{
			StreetDirectory::VertexDesc source, destination;
			const sim_mob::SubTrip firstSubTrip = firstTrip->getSubTrips().front();
			if (firstSubTrip.fromLocation.type_ == WayPoint::NODE)
			{
				source = streetDirectory.DrivingVertex(*firstSubTrip.fromLocation.node_);
			}
			else if (firstSubTrip.fromLocation.type_ == WayPoint::BUS_STOP)
			{
				const Node* node = firstSubTrip.fromLocation.busStop_->getParentSegment()->getEnd();
				source = streetDirectory.DrivingVertex(*node);
			}

			if (firstSubTrip.toLocation.type_ == WayPoint::NODE)
			{
				destination = streetDirectory.DrivingVertex(*firstSubTrip.toLocation.node_);
			}
			else if (firstSubTrip.toLocation.type_ == WayPoint::BUS_STOP)
			{
				const Node* node = firstSubTrip.toLocation.busStop_->getParentSegment()->getEnd();
				destination = streetDirectory.DrivingVertex(*node);
			}
			path = streetDirectory.SearchShortestDrivingPath(source, destination);
		}
		else if (role == "busdriver")
		{
			//throw std::runtime_error("Not implemented. BusTrip is not in master branch yet");
			const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(p->currTripChainItem));
			std::vector<const RoadSegment*> pathRoadSeg = bustrip->getBusRouteInfo().getRoadSegments();
			std::cout << "BusTrip path size = " << pathRoadSeg.size() << std::endl;
			std::vector<const RoadSegment*>::iterator itor;
			for(itor=pathRoadSeg.begin(); itor!=pathRoadSeg.end(); itor++)
			{
				path.push_back(WayPoint(*itor));
			}
		}
		else if( role == "waitBusActivity" )
		{
			const sim_mob::SubTrip firstSubTrip = dynamic_cast<const sim_mob::Trip*>(firstTrip)->getSubTrips().front();
			const BusStop* stop = firstSubTrip.fromLocation.busStop_;
			rdSeg = stop->getParentSegment();
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


void sim_mob::Conflux::insertIncident(sim_mob::SegmentStats* segStats, const double & newFlowRate) {
	const std::vector<Lane*>& lanes = segStats->getRoadSegment()->getLanes();
	for (std::vector<Lane*>::const_iterator it = lanes.begin(); it != lanes.end(); it++) {
		segStats->updateLaneParams((*it), newFlowRate);
	}
}


void sim_mob::Conflux::insertIncident(const std::vector<sim_mob::SegmentStats*> &segStats, const double & newFlowRate) {
	BOOST_FOREACH(sim_mob::SegmentStats* stat,segStats){
		insertIncident(stat,newFlowRate);
	}
}

void sim_mob::Conflux::removeIncident(sim_mob::SegmentStats* segStats) {
	const std::vector<Lane*>& lanes = segStats->getRoadSegment()->getLanes();
	for (std::vector<Lane*>::const_iterator it = lanes.begin(); it != lanes.end(); it++){
		segStats->restoreLaneParams(*it);
	}
}

sim_mob::InsertIncidentMessage::InsertIncidentMessage(const std::vector<sim_mob::SegmentStats*>& stats, double newFlowRate):stats(stats), newFlowRate(newFlowRate){;}
sim_mob::InsertIncidentMessage::~InsertIncidentMessage() {}


