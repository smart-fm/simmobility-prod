/*
 * BusStopAgent.cpp
 *
 *  Created on: 17 Apr, 2014
 *      Author: zhang
 */

#include "BusStopAgent.hpp"
#include "entities/roles/waitBusActivity/waitBusActivity.hpp"

namespace sim_mob {

namespace medium {

BusStopAgent::BusStopAgent(const MutexStrategy& mtxStrat, int id) :
		Agent(mtxStrat, id), busStop(nullptr) {
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

void BusStopAgent::registerNewWaitingPerson(sim_mob::Person* person) {
	waitingPersons.push_back(person);
}

void BusStopAgent::removeWaitingPerson(sim_mob::Person* person) {
	std::vector<sim_mob::Person*>::iterator itPerson;
	itPerson = std::find(waitingPersons.begin(), waitingPersons.end(), person);
	if(itPerson!=waitingPersons.end()){
		waitingPersons.erase(itPerson);
	}
}

void BusStopAgent::alightingPassengerToStop(sim_mob::Person* person) {
	alightingPersons.push_back(person);
}

void BusStopAgent::setAssociateBusStop(sim_mob::BusStop* stop) {
	busStop = stop;
}

sim_mob::BusStop* BusStopAgent::getAssociateBusStop() {
	return busStop;
}

void BusStopAgent::processWaitingPersonBoarding(
		sim_mob::medium::BusDriver* busDriver) {
	std::vector<sim_mob::Person*>::iterator itPerson;
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
		if(waitingActivity && waitingActivity->getDecisionResult()==MAKE_DECISION_BOARDING){
			itPerson = waitingPersons.erase(itPerson);
			//busDriver->assignNewPassenger(*itPerson);
		}
		else {
			itPerson++;
		}
	}
}
}
}
