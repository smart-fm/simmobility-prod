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
	if(parent && parent->currSubTrip->mode=="MRT")
	{
		roleType = Role::RL_TRAINPASSENGER;
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
	std::string personId, tripStartPoint, tripEndPoint, subStartPoint,
			subEndPoint, subStartType, subEndType, mode, service, arrivaltime,
			travelTime;

	personId = boost::lexical_cast<std::string>(parent->getId());
	tripStartPoint = (*(parent->currTripChainItem))->startLocationId;
	tripEndPoint = (*(parent->currTripChainItem))->endLocationId;
	subStartPoint = parent->currSubTrip->startLocationId;
	subEndPoint = parent->currSubTrip->endLocationId;
	subStartType = parent->currSubTrip->startLocationType;
	subEndType = parent->currSubTrip->endLocationType;
	mode = parent->currSubTrip->getMode();
	service = parent->currSubTrip->ptLineId;
	travelTime = DailyTime(parent->getRole()->getTravelTime()).getStrRepr();
	arrivaltime = DailyTime(parent->getRole()->getArrivalTime()).getStrRepr();
	if(roleType == Role::RL_TRAINPASSENGER){
		mode = "MRT_TRAVEL";
	} else {
		mode = "BUS_TRAVEL";
	}

	messaging::MessageBus::PostMessage(PT_Statistics::GetInstance(),
			STORE_PERSON_TRAVEL,
			messaging::MessageBus::MessagePtr(
					new PersonTravelTimeMessage(personId, tripStartPoint,
							tripEndPoint, subStartPoint, subEndPoint,
							subStartType, subEndType, mode, service,
							arrivaltime, travelTime)), true);
}

}
}
