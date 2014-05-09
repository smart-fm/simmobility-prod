/*
 * BusStopAgent.cpp
 *
 *  Created on: 17 Apr, 2014
 *      Author: zhang
 */

#include "BusStopAgent.hpp"
#include "entities/roles/waitBusActivity/waitBusActivity.hpp"
#include "message/MT_Message.hpp"

namespace sim_mob {

namespace medium {

BusStopAgent::BusStopAgent(const MutexStrategy& mtxStrat, int id, const sim_mob::BusStop* stop) :
		Agent(mtxStrat, id), busStop(stop) {
	// TODO Auto-generated constructor stub

}

BusStopAgent::~BusStopAgent() {
	// TODO Auto-generated destructor stub
}

void BusStopAgent::onEvent(event::EventId eventId,
		sim_mob::event::Context ctxId, event::EventPublisher* sender,
		const event::EventArgs& args) {

	Agent::onEvent(eventId, ctxId, sender, args);

}

void BusStopAgent::registerWaitingPerson(sim_mob::Person* person) {
	waitingPersons.push_back(person);
}

void BusStopAgent::removeWaitingPerson(sim_mob::Person* person) {
	std::list<sim_mob::Person*>::iterator itPerson;
	itPerson = std::find(waitingPersons.begin(), waitingPersons.end(), person);
	if(itPerson!=waitingPersons.end()){
		waitingPersons.erase(itPerson);
	}
}

void BusStopAgent::addAlightingPerson(sim_mob::Person* person) {
	alightingPersons.push_back(person);
}

const sim_mob::BusStop* BusStopAgent::getBusStop() const{
	return busStop;
}

bool BusStopAgent::frame_init(timeslice now) {
	try {
		messaging::MessageBus::RegisterHandler(this);
	} catch (const std::runtime_error& error) {
		Print() << error.what() << std::endl;
		return false;
	}
	return true;
}

Entity::UpdateStatus BusStopAgent::frame_tick(timeslice now) {
	return UpdateStatus::Continue;
}

void BusStopAgent::HandleMessage(messaging::Message::MessageType type,
		const messaging::Message& message) {

	Agent::HandleMessage(type, message);

	switch (type) {
	case MSG_DECISION_WAITINGPERSON_BOARDING:
		const WaitingPeopleBoardingDecisionMessageArgs& msg = MSG_CAST(
				WaitingPeopleBoardingDecisionMessageArgs, message);
		boardWaitingPersons(msg.busDriver);
		break;
	}
}

void BusStopAgent::boardWaitingPersons(
		sim_mob::medium::BusDriver* busDriver) {
	int numBoarding = 0;
	std::list<sim_mob::Person*>::iterator itPerson;
	for (itPerson = waitingPersons.begin(); itPerson != waitingPersons.end();
			itPerson++) {
		sim_mob::Role* curRole = (*itPerson)->getRole();
		sim_mob::medium::WaitBusActivity* waitingActivity =
				dynamic_cast<sim_mob::medium::WaitBusActivity*>(curRole);
		if (waitingActivity) {
			waitingActivity->makeBoardingDecision(busDriver);
		}
	}

	itPerson = waitingPersons.begin();
	while (itPerson != waitingPersons.end()) {
		sim_mob::Role* curRole = (*itPerson)->getRole();
		sim_mob::medium::WaitBusActivity* waitingActivity =
				dynamic_cast<sim_mob::medium::WaitBusActivity*>(curRole);
		if(waitingActivity && waitingActivity->getDecision()==BOARD_BUS){
			if(busDriver->insertPassenger(*itPerson)){
				itPerson = waitingPersons.erase(itPerson);
				numBoarding++;
			}
			else {
				waitingActivity->setDecision(NO_DECISION);
				itPerson++;
			}
		}
		else {
			itPerson++;
		}
	}

	std::map<sim_mob::medium::BusDriver*, int>::iterator it =
			lastBoardingRecorder.find(busDriver);
	if (it != lastBoardingRecorder.end()) {
		it->second = numBoarding;
	} else {
		lastBoardingRecorder.insert(std::make_pair(busDriver, numBoarding));
	}
}

int BusStopAgent::getBoardingNum(sim_mob::medium::BusDriver* busDriver) {
	int numBoarding = 0;
	std::map<sim_mob::medium::BusDriver*, int>::iterator it =
			lastBoardingRecorder.find(busDriver);
	if (it != lastBoardingRecorder.end()) {
		numBoarding = it->second;
	}
	return numBoarding;
}
}
}
