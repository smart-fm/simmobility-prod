//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "waitBusActivity.hpp"
#include "waitBusActivityFacets.hpp"
#include "entities/Person.hpp"
#include "geospatial/BusStop.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "message/MT_Message.hpp"
#include "entities/PT_Statistics.hpp"
#include "boost/algorithm/string.hpp"
#include "entities/Person_MT.hpp"

using std::vector;
using namespace sim_mob;

namespace sim_mob
{

namespace medium
{

sim_mob::medium::WaitBusActivity::WaitBusActivity(Person_MT *parent,
												  sim_mob::medium::WaitBusActivityBehavior* behavior,
												  sim_mob::medium::WaitBusActivityMovement* movement,
												  std::string roleName, Role::Type roleType) :
sim_mob::Role(parent, behavior, movement, parent, roleName, roleType),
waitingTime(0), stop(nullptr), boardBus(false), failedBoardingTimes(0)
{
}

Role* sim_mob::medium::WaitBusActivity::clone(Person_MT *parent) const
{
	WaitBusActivityBehavior* behavior = new WaitBusActivityBehavior();
	WaitBusActivityMovement* movement = new WaitBusActivityMovement();
	WaitBusActivity* waitBusActivity = new WaitBusActivity(parent, behavior, movement);
	behavior->setParentWaitBusActivity(waitBusActivity);
	movement->setParentWaitBusActivity(waitBusActivity);
	return waitBusActivity;
}

void sim_mob::medium::WaitBusActivity::increaseWaitingTime(unsigned int incrementMs)
{
	waitingTime += incrementMs;
}

void sim_mob::medium::WaitBusActivity::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}

void sim_mob::medium::WaitBusActivity::collectTravelTime()
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
	mode = "WAITING_BUS";
	messaging::MessageBus::PostMessage(PT_Statistics::GetInstance(),
									STORE_PERSON_TRAVEL,
									messaging::MessageBus::MessagePtr(
																	  new PersonTravelTimeMessage(personId, tripStartPoint,
																								  tripEndPoint, subStartPoint, subEndPoint,
																								  subStartType, subEndType, mode, service,
																								  arrivaltime, travelTime)), true);

}

void sim_mob::medium::WaitBusActivity::increaseFailedBoardingTimes()
{
	failedBoardingTimes++;
}

const std::string sim_mob::medium::WaitBusActivity::getBusLines()
{
	sim_mob::SubTrip& subTrip = *(parent->currSubTrip);
	const std::string tripLineID = subTrip.getBusLineID();
	return tripLineID;
}

void sim_mob::medium::WaitBusActivity::makeBoardingDecision(BusDriver* driver)
{
	const std::vector<const sim_mob::BusStop*>* stopsVec =
			driver->getBusStopsVector();
	if (!stopsVec)
	{
		setBoardBus(false);
		return;
	}

	const std::string busLineID = driver->getBusLineID();
	sim_mob::SubTrip& subTrip = *(parent->currSubTrip);
	const std::string tripLineID = subTrip.getBusLineID();
	std::vector<std::string> lines;
	boost::split(lines, tripLineID, boost::is_any_of("/"));
	if (std::find(lines.begin(), lines.end(), busLineID) != lines.end())
	{
		setBoardBus(true);
		return;
	}
	else
	{
		setBoardBus(false);
		return;
	}

	const sim_mob::BusStop* destStop = nullptr;
	if (parent->destNode.type_ == WayPoint::BUS_STOP
			&& parent->destNode.busStop_)
	{
		destStop = parent->destNode.busStop_;
	}

	if (!destStop)
	{
		setBoardBus(false);
		return;
	}

	std::vector<const sim_mob::BusStop*>::const_iterator itStop = std::find(stopsVec->begin(), stopsVec->end(), destStop);
	
	if (itStop != stopsVec->end())
	{
		setBoardBus(true);
	}
}

void sim_mob::medium::WaitBusActivity::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
}

std::vector<BufferedBase*> sim_mob::medium::WaitBusActivity::getSubscriptionParams()
{
	return vector<BufferedBase*>();
}

}
}
