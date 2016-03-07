//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Pedestrian.hpp"
#include "config/MT_Config.hpp"
#include "entities/Person.hpp"
#include "entities/Person_MT.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/activityRole/ActivityFacets.hpp"
#include "message/MT_Message.hpp"
#include "PedestrianFacets.hpp"

using std::vector;
using namespace sim_mob;
using namespace sim_mob::medium;

sim_mob::medium::Pedestrian::Pedestrian(Person_MT *parent,
										sim_mob::medium::PedestrianBehavior* behavior,
										sim_mob::medium::PedestrianMovement* movement,
										std::string roleName, Role<Person_MT>::Type roleType) :
		sim_mob::Role<Person_MT>::Role(parent, behavior, movement, roleName, roleType)
{
}

sim_mob::medium::Pedestrian::~Pedestrian()
{
}

Role<Person_MT>* sim_mob::medium::Pedestrian::clone(Person_MT *parent) const
{
	double walkSpeed = MT_Config::getInstance().getPedestrianWalkSpeed();
	PedestrianBehavior* behavior = new PedestrianBehavior();
	PedestrianMovement* movement = new PedestrianMovement(walkSpeed);
	Pedestrian* pedestrian = new Pedestrian(parent, behavior, movement);
	behavior->setParentPedestrian(pedestrian);
	movement->setParentPedestrian(pedestrian);
	return pedestrian;
}

std::vector<BufferedBase*> sim_mob::medium::Pedestrian::getSubscriptionParams()
{
	return vector<BufferedBase*>();
}

void sim_mob::medium::Pedestrian::make_frame_tick_params(timeslice now)
{
}

void sim_mob::medium::Pedestrian::collectTravelTime()
{
//  COMMENTED FOR CALIBRATION ~Harish
//	PersonTravelTime personTravelTime;
//	personTravelTime.personId = parent->getId();
//	if(parent->getPrevRole() && parent->getPrevRole()->roleType==Role<Person_MT>::RL_ACTIVITY)
//	{
//		ActivityPerformer<Person_MT>* activity = dynamic_cast<ActivityPerformer<Person_MT>* >(parent->getPrevRole());
//		std::string activityLocNodeIdStr = boost::lexical_cast<std::string>(activity->getLocation()->getNodeId());
//		personTravelTime.tripStartPoint = activityLocNodeIdStr;
//		personTravelTime.tripEndPoint = activityLocNodeIdStr;
//		personTravelTime.subStartPoint = activityLocNodeIdStr;
//		personTravelTime.subEndPoint = activityLocNodeIdStr;
//		personTravelTime.subStartType = "NODE";
//		personTravelTime.subEndType = "NODE";
//		personTravelTime.mode = "ACTIVITY";
//		personTravelTime.service = parent->currSubTrip->ptLineId;
//		personTravelTime.travelTime = ((double) activity->getTravelTime())/1000.0;
//		personTravelTime.arrivalTime = DailyTime(activity->getArrivalTime()).getStrRepr();
//		messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
//				STORE_PERSON_TRAVEL_TIME, messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personTravelTime)), true);
//	}
//	personTravelTime.tripStartPoint = (*(parent->currTripChainItem))->startLocationId;
//	personTravelTime.tripEndPoint = (*(parent->currTripChainItem))->endLocationId;
//	personTravelTime.subStartPoint = parent->currSubTrip->startLocationId;
//	personTravelTime.subEndPoint = parent->currSubTrip->endLocationId;
//	personTravelTime.subStartType = parent->currSubTrip->startLocationType;
//	personTravelTime.subEndType = parent->currSubTrip->endLocationType;
//	personTravelTime.mode = "WALK";
//	personTravelTime.service = parent->currSubTrip->ptLineId;
//	personTravelTime.travelTime = totalTravelTimeMS/1000.0;
//	personTravelTime.arrivalTime = DailyTime(parent->getRole()->getArrivalTime()).getStrRepr();
//	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
//			STORE_PERSON_TRAVEL_TIME, messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personTravelTime)), true);
}
