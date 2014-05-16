/*
 * BusStopAgent.cpp
 *
 *  Created on: 17 Apr, 2014
 *      Author: zhang
 */

#include "BusStopAgent.hpp"
#include "entities/roles/waitBusActivity/waitBusActivity.hpp"
#include "message/MT_Message.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;
namespace sim_mob {

namespace medium {

BusStopAgent::BusStopAgentsMap BusStopAgent::allBusstopAgents;

void BusStopAgent::registerBusStopAgent(BusStopAgent* busstopAgent)
{
	allBusstopAgents[busstopAgent->getBusStop()] = busstopAgent;
}

BusStopAgent* BusStopAgent::findBusStopAgentByBusStop(const BusStop* busstop)
{
	try {
		return allBusstopAgents.at(busstop);
	}
	catch (const std::out_of_range& oor) {
		return nullptr;
	}
}

BusStopAgent::BusStopAgent(const MutexStrategy& mtxStrat, int id,
		const BusStop* stop, SegmentStats* stats) :
		Agent(mtxStrat, id), busStop(stop), parentSegmentStats(stats),
		availableLength(stop->getBusCapacityAsLength())
{}

BusStopAgent::~BusStopAgent() {
	// TODO Auto-generated destructor stub
}

void BusStopAgent::onEvent(event::EventId eventId,
		event::Context ctxId, event::EventPublisher* sender,
		const event::EventArgs& args) {

	Agent::onEvent(eventId, ctxId, sender, args);

}

void BusStopAgent::registerWaitingPerson(WaitBusActivity* waitingPerson) {
	waitingPersons.push_back(waitingPerson);
}

void BusStopAgent::removeWaitingPerson(WaitBusActivity* waitingPerson) {
	std::list<WaitBusActivity*>::iterator itPerson;
	itPerson = std::find(waitingPersons.begin(), waitingPersons.end(), waitingPerson);
	if(itPerson!=waitingPersons.end()){
		waitingPersons.erase(itPerson);
	}
}

void BusStopAgent::addAlightingPerson(Passenger* passenger) {
	alightingPersons.push_back(passenger);
}

const BusStop* BusStopAgent::getBusStop() const{
	return busStop;
}

bool BusStopAgent::frame_init(timeslice now) {
	messaging::MessageBus::RegisterHandler(this);
	return true;
}

Entity::UpdateStatus BusStopAgent::frame_tick(timeslice now) {
	return UpdateStatus::Continue;
}

void BusStopAgent::HandleMessage(messaging::Message::MessageType type,
		const messaging::Message& message) {

	switch (type) {
	case BOARD_BUS: {
		const BusDriverMessage& msg = MSG_CAST(BusDriverMessage, message);
		boardWaitingPersons(msg.busDriver);
		break;
	}
	case BUS_ARRIVAL: {
		const BusDriverMessage& msg = MSG_CAST(BusDriverMessage, message);
		bool busDriverAccepted = acceptBusDriver(msg.busDriver);
		if(!busDriverAccepted) {
			throw std::runtime_error("BusDriver could not be accepted by the bus stop");
		}
		break;
	}
	case BUS_DEPARTURE: {
		const BusDriverMessage& msg = MSG_CAST(BusDriverMessage, message);
		bool busDriverRemoved = removeBusDriver(msg.busDriver);
		if(!busDriverRemoved) {
			throw std::runtime_error("BusDriver could not be found in bus stop");
		}
		break;
	}
	default: {
		break;
	}
	}
}

void BusStopAgent::boardWaitingPersons(BusDriver* busDriver) {
	int numBoarding = 0;
	std::list<WaitBusActivity*>::iterator itPerson;
	for (itPerson = waitingPersons.begin(); itPerson != waitingPersons.end();
			itPerson++) {
		(*itPerson)->makeBoardingDecision(busDriver);
	}

	itPerson = waitingPersons.begin();
	while (itPerson != waitingPersons.end()) {
		if ((*itPerson)->canBoardBus()) {
			itPerson = waitingPersons.erase(itPerson);
			(*itPerson)->setBoardBus(false);
			numBoarding++;
		}
		else {
			itPerson++;
		}
	}

	lastBoardingRecorder[busDriver] = numBoarding;
}

bool BusStopAgent::acceptBusDriver(BusDriver* driver) {
	if(driver) {
		double vehicleLength = driver->getResource()->getLengthCm();
		if(availableLength >= vehicleLength) {
			servingDrivers.push_back(driver);
			availableLength=availableLength-vehicleLength;
			parentSegmentStats->addBusDriverToStop(driver->getParent(), busStop);
			return true;
		}
	}
	return false;
}

bool BusStopAgent::removeBusDriver(BusDriver* driver) {
	if(driver) {
		double vehicleLength = driver->getResource()->getLengthCm();
		std::list<sim_mob::medium::BusDriver*>::iterator driverIt =
				std::find(servingDrivers.begin(), servingDrivers.end(), driver);
		if(driverIt!=servingDrivers.end()) {
			servingDrivers.erase(driverIt);
			availableLength=availableLength+vehicleLength;
			parentSegmentStats->removeBusDriverFromStop(driver->getParent(), busStop);
			return true;
		}
	}
	return false;
}

bool BusStopAgent::canAccommodate(const double vehicleLength) {
	return (availableLength >= vehicleLength);
}

int BusStopAgent::getBoardingNum(BusDriver* busDriver) const {
	try {
		return lastBoardingRecorder.at(busDriver);
	}
	catch (const std::out_of_range& oor) {
		return 0;
	}
}
}
}
