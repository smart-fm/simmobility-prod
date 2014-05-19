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

BusStopAgent::BusStopAgent(const MutexStrategy& mtxStrat, int id,
		const sim_mob::BusStop* stop, const sim_mob::SegmentStats* stat) :
		Agent(mtxStrat, id), busStop(stop), segmentStat(stat) {
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

void BusStopAgent::registerWaitingPerson(sim_mob::medium::WaitBusActivity* waitingPerson) {
	waitingPersons.push_back(waitingPerson);
}

void BusStopAgent::removeWaitingPerson(sim_mob::medium::WaitBusActivity* waitingPerson) {
	std::list<sim_mob::medium::WaitBusActivity*>::iterator itPerson;
	itPerson = std::find(waitingPersons.begin(), waitingPersons.end(), waitingPerson);
	if(itPerson!=waitingPersons.end()){
		waitingPersons.erase(itPerson);
	}
}

void BusStopAgent::addAlightingPerson(sim_mob::medium::Passenger* passenger) {
	alightingPersons.push_back(passenger);
}

const sim_mob::BusStop* BusStopAgent::getBusStop() const{
	return busStop;
}

bool BusStopAgent::frame_init(timeslice now) {
	messaging::MessageBus::RegisterHandler(this);
	return true;
}

Entity::UpdateStatus BusStopAgent::frame_tick(timeslice now) {
	std::list<sim_mob::medium::Passenger*>::iterator itPerson =
			alightingPersons.begin();
	while (itPerson != alightingPersons.end()) {
		bool ret = false;
		sim_mob::medium::Passenger* waitingPeople = *itPerson;
		Agent* parent = waitingPeople->getParent();
		Person* person = dynamic_cast<Person*>(parent);
		if (person) {
			person->checkTripChain();
			Role* role = person->getRole();
			if (role) {
				if (role->roleType == Role::RL_WAITBUSACTITITY) {
					WaitBusActivity* waitPerson =
							dynamic_cast<WaitBusActivity*>(role);
					if (waitPerson) {
						registerWaitingPerson(waitPerson);
						ret = true;
					}
				} else if (role->roleType == Role::RL_PEDESTRIAN) {
					Conflux* conflux =
							segmentStat->getRoadSegment()->getParentConflux();
					messaging::MessageBus::PostMessage(conflux,
							MSG_PEDESTRIAN_TRANSFER_REQUEST,
							messaging::MessageBus::MessagePtr(
									new PedestrianRequestMessageArgs(person)));
					ret = true;
				}
			}
		}

		if (ret) {
			itPerson = alightingPersons.erase(itPerson);
		} else {
			itPerson++;
		}
	}
	return UpdateStatus::Continue;
}

void BusStopAgent::HandleMessage(messaging::Message::MessageType type,
		const messaging::Message& message) {

	switch (type) {
	case MSG_DECISION_WAITINGPERSON_BOARDING: {
		const BoardingMessage& msg = MSG_CAST(BoardingMessage, message);
		boardWaitingPersons(msg.busDriver);
		break;
	}
	case MSG_WAITINGPERSON_ARRIVALAT_BUSSTOP: {
		const ArriavalAtStopMessage& msg = MSG_CAST(ArriavalAtStopMessage,
				message);
		Person* person = msg.waitingPerson;
		Role* role = person->getRole();
		if (role) {
			WaitBusActivity* waitPerson = dynamic_cast<WaitBusActivity*>(role);
			if (waitPerson) {
				registerWaitingPerson(waitPerson);
			}
		}
		break;
	}
	default: {
		break;
	}
	}
}

void BusStopAgent::boardWaitingPersons(sim_mob::medium::BusDriver* busDriver) {
	int numBoarding = 0;
	std::list<sim_mob::medium::WaitBusActivity*>::iterator itPerson;
	for (itPerson = waitingPersons.begin(); itPerson != waitingPersons.end();
			itPerson++) {
		(*itPerson)->makeBoardingDecision(busDriver);
	}

	itPerson = waitingPersons.begin();
	while (itPerson != waitingPersons.end()) {
		if ((*itPerson)->getDecision() == BOARD_BUS) {
			bool ret = false;
			WaitBusActivity* waitingPeople = *itPerson;
			Agent* parent = waitingPeople->getParent();
			Person* person = dynamic_cast<Person*>(parent);
			if (person) {
				person->checkTripChain();
				Role* curRole = person->getRole();
				sim_mob::medium::Passenger* passenger =
						dynamic_cast<sim_mob::medium::Passenger*>(curRole);
				if (passenger && busDriver->insertPassenger(passenger)) {
					ret = true;
				}
			}
			if (ret) {
				itPerson = waitingPersons.erase(itPerson);
				numBoarding++;
			} else {
				itPerson++;
			}
		} else {
			itPerson++;
		}
	}

	lastBoardingRecorder[busDriver] = numBoarding;
}

int BusStopAgent::getBoardingNum(sim_mob::medium::BusDriver* busDriver) const {
	try {
		return lastBoardingRecorder.at(busDriver);
	} catch (const std::out_of_range& oor) {
		return 0;
	}
}
}
}
