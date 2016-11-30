//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <vector>

#include "Pedestrian.hpp"
#include "entities/Person_ST.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"

using namespace std;
using namespace sim_mob;

Pedestrian::Pedestrian(Person_ST *parent, PedestrianBehaviour *behaviour, PedestrianMovement *movement, Role<Person_ST>::Type roleType_, std::string roleName) :
Role<Person_ST>::Role(parent, behaviour, movement, roleName, roleType_)
{
}

Pedestrian::~Pedestrian()
{
}

Role<Person_ST>* Pedestrian::clone(Person_ST *parent) const
{
	PedestrianBehaviour *behaviour = new PedestrianBehaviour();
	PedestrianMovement *movement = new PedestrianMovement();
	Pedestrian *pedestrian = new Pedestrian(parent, behaviour, movement);
	behaviour->setParentPedestrian(pedestrian);
	movement->setParentPedestrian(pedestrian);
	return pedestrian;
}

vector<BufferedBase *> Pedestrian::getSubscriptionParams()
{
	return vector<BufferedBase *>();
}

void Pedestrian::make_frame_tick_params(timeslice now)
{
}

void Pedestrian::collectTravelTime()
{
	PersonTravelTime personTravelTime;
	personTravelTime.personId = parent->getDatabaseId();
	
	if (parent->getPrevRole() && parent->getPrevRole()->roleType == Role<Person_ST>::RL_ACTIVITY)
	{
		ActivityPerformer<Person_ST> *activity = dynamic_cast<ActivityPerformer<Person_ST> *> (parent->getPrevRole());
		std::string activityLocNodeIdStr = boost::lexical_cast<std::string>(activity->getLocation()->getNodeId());
		
		personTravelTime.tripStartPoint = activityLocNodeIdStr;
		personTravelTime.tripEndPoint = activityLocNodeIdStr;
		personTravelTime.subStartPoint = activityLocNodeIdStr;
		personTravelTime.subEndPoint = activityLocNodeIdStr;
		personTravelTime.subStartType = "NODE";
		personTravelTime.subEndType = "NODE";
		personTravelTime.mode = "ACTIVITY";
		personTravelTime.service = parent->currSubTrip->ptLineId;
		personTravelTime.travelTime = ((double) activity->getTravelTime()) / 1000.0;
		personTravelTime.arrivalTime = DailyTime(activity->getArrivalTime()).getStrRepr();
		
		messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
				STORE_PERSON_TRAVEL_TIME, messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personTravelTime)), true);
	}
	
	personTravelTime.tripStartPoint = (*(parent->currTripChainItem))->startLocationId;
	personTravelTime.tripEndPoint = (*(parent->currTripChainItem))->endLocationId;
	personTravelTime.subStartPoint = parent->currSubTrip->startLocationId;
	personTravelTime.subEndPoint = parent->currSubTrip->endLocationId;
	personTravelTime.subStartType = parent->currSubTrip->startLocationType;
	personTravelTime.subEndType = parent->currSubTrip->endLocationType;
	personTravelTime.mode = "WALK";
	personTravelTime.service = parent->currSubTrip->ptLineId;
	personTravelTime.travelTime = totalTravelTimeMS / 1000.0;
	personTravelTime.arrivalTime = DailyTime(parent->getRole()->getArrivalTime()).getStrRepr();
	
	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
			STORE_PERSON_TRAVEL_TIME, messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personTravelTime)), true);
}
