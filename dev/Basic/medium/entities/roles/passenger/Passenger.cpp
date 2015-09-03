//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Passenger.hpp"
#include "PassengerFacets.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "message/MT_Message.hpp"
#include "message/MT_Message.hpp"
#include "entities/PT_Statistics.hpp"
using std::vector;
using namespace sim_mob;

namespace sim_mob {

namespace medium {

sim_mob::medium::Passenger::Passenger(Person* parent, MutexStrategy mtxStrat,
		sim_mob::medium::PassengerBehavior* behavior,
		sim_mob::medium::PassengerMovement* movement,
		std::string roleName, Role::type roleType) :
		sim_mob::Role(behavior, movement, parent, roleName, roleType),
		driver(nullptr), alightBus(false), startNode(nullptr), endNode(nullptr)
{}

Role* sim_mob::medium::Passenger::clone(Person* parent) const {
	PassengerBehavior* behavior = new PassengerBehavior(parent);
	PassengerMovement* movement = new PassengerMovement(parent);
	Role::type roleType=Role::RL_PASSENGER;
	if (parent->currSubTrip->mode == "MRT") {
		roleType = Role::RL_TRAINPASSENGER;
	} else if (parent->currSubTrip->mode == "Sharing") {
		roleType = Role::RL_CARPASSENGER;
	}
	Passenger* passenger = new Passenger(parent, parent->getMutexStrategy(), behavior, movement, "Passenger_", roleType);
	behavior->setParentPassenger(passenger);
	movement->setParentPassenger(passenger);
	return passenger;
}

std::vector<BufferedBase*> sim_mob::medium::Passenger::getSubscriptionParams() {
	return vector<BufferedBase*>();
}

void sim_mob::medium::Passenger::makeAlightingDecision(const sim_mob::BusStop* nextStop) {
	if (getParent()->destNode.type_ == WayPoint::BUS_STOP
			&& getParent()->destNode.busStop_ == nextStop) {
		setAlightBus(true);
		setDriver(nullptr);
	}
}

void sim_mob::medium::Passenger::HandleParentMessage(messaging::Message::MessageType type,
		const messaging::Message& message)
{
	switch (type) {
	case ALIGHT_BUS: {
		const BusStopMessage& msg = MSG_CAST(BusStopMessage, message);
		makeAlightingDecision(msg.nextStop);
		break;
	}
	default: {
		break;
	}
	}
}

void sim_mob::medium::Passenger::collectTravelTime()
{
	PersonTravelTime personTravelTime;
	personTravelTime.personId = boost::lexical_cast<std::string>(parent->getId());
	personTravelTime.tripStartPoint = (*(parent->currTripChainItem))->startLocationId;
	personTravelTime.tripEndPoint = (*(parent->currTripChainItem))->endLocationId;
	personTravelTime.subStartPoint = parent->currSubTrip->startLocationId;
	personTravelTime.subEndPoint = parent->currSubTrip->endLocationId;
	personTravelTime.subStartType = parent->currSubTrip->startLocationType;
	personTravelTime.subEndType = parent->currSubTrip->endLocationType;
	personTravelTime.mode = parent->currSubTrip->getMode();
	personTravelTime.service = parent->currSubTrip->ptLineId;
	personTravelTime.travelTime = DailyTime(parent->getRole()->getTravelTime()).getStrRepr();
	personTravelTime.arrivalTime = DailyTime(parent->getRole()->getArrivalTime()).getStrRepr();

	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
			STORE_PERSON_TRAVEL_TIME, messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personTravelTime)), true);
}

}
}
