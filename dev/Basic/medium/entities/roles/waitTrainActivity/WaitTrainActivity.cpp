//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "WaitTrainActivity.hpp"

#include <boost/algorithm/string.hpp>
#include "entities/Person_MT.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "message/MT_Message.hpp"
#include "WaitTrainActivityFacets.hpp"

using std::vector;
using namespace sim_mob;

namespace sim_mob
{

namespace medium
{

sim_mob::medium::WaitTrainActivity::WaitTrainActivity(Person_MT* parent, sim_mob::medium::WaitTrainActivityBehavior* behavior,
		sim_mob::medium::WaitTrainActivityMovement* movement, std::string roleName, Role<Person_MT>::Type roleType) :
		sim_mob::Role<Person_MT>::Role(parent, behavior, movement, roleName, roleType), waitingTime(0),failedToBoardCount(0),platform(nullptr)
{
}

sim_mob::medium::WaitTrainActivity::~WaitTrainActivity()
{
}

Role<Person_MT>* sim_mob::medium::WaitTrainActivity::clone(Person_MT* parent) const
{
	WaitTrainActivityBehavior* behavior = new WaitTrainActivityBehavior();
	WaitTrainActivityMovement* movement = new WaitTrainActivityMovement();
	WaitTrainActivity* waitTrainActivity = new WaitTrainActivity(parent, behavior, movement);
	behavior->setParent(waitTrainActivity);
	movement->setParent(waitTrainActivity);
	return waitTrainActivity;
}

void sim_mob::medium::WaitTrainActivity::increaseWaitingTime(unsigned int incrementMs)
{
	waitingTime += incrementMs;
}

void sim_mob::medium::WaitTrainActivity::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}

void sim_mob::medium::WaitTrainActivity::collectTravelTime()
{
	PersonTravelTime personTravelTime;
	std::string personId, tripStartPoint, tripEndPoint, subStartPoint, subEndPoint, subStartType, subEndType, mode, service, arrivaltime, travelTime;
	personTravelTime.personId = parent->getDatabaseId();
	personTravelTime.tripStartPoint = (*(parent->currTripChainItem))->startLocationId;
	personTravelTime.tripEndPoint = (*(parent->currTripChainItem))->endLocationId;
	personTravelTime.subStartPoint = parent->currSubTrip->startLocationId;
	personTravelTime.subEndPoint = parent->currSubTrip->endLocationId;
	personTravelTime.subStartType = parent->currSubTrip->startLocationType;
	personTravelTime.subEndType = parent->currSubTrip->endLocationType;
	personTravelTime.mode = "WAIT_MRT";
	personTravelTime.service = parent->currSubTrip->ptLineId;
	personTravelTime.travelTime = ((double) parent->getRole()->getTravelTime())/1000.0; //convert to seconds
	personTravelTime.arrivalTime = DailyTime(parent->getRole()->getArrivalTime()).getStrRepr();
	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
					STORE_PERSON_TRAVEL_TIME, messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personTravelTime)), true);
}

void sim_mob::medium::WaitTrainActivity::incrementDeniedBoardingCount()
{
	failedToBoardCount++;
}

unsigned int sim_mob::medium::WaitTrainActivity::getWaitingTime() const
{
	return waitingTime;
}

unsigned int sim_mob::medium::WaitTrainActivity::getDeniedBoardingCount() const
{
	return failedToBoardCount;
}

void sim_mob::medium::WaitTrainActivity::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message)
{}

std::vector<BufferedBase*> sim_mob::medium::WaitTrainActivity::getSubscriptionParams()
{
	return vector<BufferedBase*>();
}
const Platform* sim_mob::medium::WaitTrainActivity::getStartPlatform() const
{
	return platform;
}
void sim_mob::medium::WaitTrainActivity::setStartPlatform(const Platform* plat)
{
	platform = plat;
}
const std::string& sim_mob::medium::WaitTrainActivity::getTrainLine() const
{
	return parent->currSubTrip->serviceLine;
}
double sim_mob::medium::WaitTrainActivity::getWalkTimeToPlatform() const
{
	return walkingTimeToPlatform;
}

void sim_mob::medium::WaitTrainActivity::setWalkTimeToPlatform(double walkTime)
{
	walkingTimeToPlatform = walkTime;
}

void sim_mob::medium::WaitTrainActivity::reduceWalkingTime()
{
	walkingTimeToPlatform = walkingTimeToPlatform - 5;
}
}
}
