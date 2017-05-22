//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "WaitBusActivity.hpp"

#include <boost/algorithm/string.hpp>

#include "entities/BusStopAgent.hpp"
#include "entities/Person_ST.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "message/ST_Message.hpp"
#include "WaitBusActivityFacets.hpp"

using namespace std;
using namespace sim_mob;

WaitBusActivity::WaitBusActivity(Person_ST *parent, WaitBusActivityBehavior *behavior,
		WaitBusActivityMovement *movement, string roleName, Role<Person_ST>::Type roleType) :
Role<Person_ST>::Role(parent, behavior, movement, roleName, roleType), waitingTime(0), activityState(WAITBUS_STATE_WAITING),
failedToBoardCount(0), busDriver(nullptr)
{
}

WaitBusActivity::~WaitBusActivity()
{
}

Role<Person_ST>* WaitBusActivity::clone(Person_ST *parent) const
{
	WaitBusActivityBehavior *behavior = new WaitBusActivityBehavior();
	WaitBusActivityMovement *movement = new WaitBusActivityMovement();
	WaitBusActivity *waitBusActivity = new WaitBusActivity(parent, behavior, movement);
	behavior->setParentWaitBusActivity(waitBusActivity);
	movement->setParentWaitBusActivity(waitBusActivity);
	return waitBusActivity;
}

void WaitBusActivity::increaseWaitingTime(unsigned int incrementMs)
{
	waitingTime += incrementMs;
}

void WaitBusActivity::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}

void WaitBusActivity::collectTravelTime()
{
	PersonTravelTime personTravelTime;
	personTravelTime.personId = parent->getDatabaseId();
	personTravelTime.tripStartPoint = (*(parent->currTripChainItem))->startLocationId;
	personTravelTime.tripEndPoint = (*(parent->currTripChainItem))->endLocationId;
	personTravelTime.subStartPoint = parent->currSubTrip->startLocationId;
	personTravelTime.subEndPoint = parent->currSubTrip->endLocationId;
	personTravelTime.subStartType = parent->currSubTrip->startLocationType;
	personTravelTime.subEndType = parent->currSubTrip->endLocationType;
	personTravelTime.mode = "WAITING_BUS";
	personTravelTime.service = parent->currSubTrip->ptLineId;
	personTravelTime.travelTime = ((double) parent->getRole()->getTravelTime())/1000.0; //convert to seconds
	personTravelTime.arrivalTime = DailyTime(parent->getRole()->getArrivalTime()).getStrRepr();
	
	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
					STORE_PERSON_TRAVEL_TIME, messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personTravelTime)), true);
}

void WaitBusActivity::incrementDeniedBoardingCount()
{
	failedToBoardCount++;
}

void WaitBusActivity::makeBoardingDecision(const BusDriver *driver)
{
	TripChainItem *tripChainItem = *(driver->getParent()->currTripChainItem);
	BusTrip *busTrip = dynamic_cast<BusTrip *> (tripChainItem);
	const vector<const BusStop *> &stopsVec = busTrip->getBusRouteInfo().getBusStops();
	
	if (stopsVec.empty())
	{		
		return;
	}

	const string &busLineID = driver->getBusLineId();
	SubTrip &subTrip = *(parent->currSubTrip);
	const string &tripLineID = subTrip.getBusLineID();
	
	vector<string> lines;
	boost::split(lines, tripLineID, boost::is_any_of("/"));
	
	if (find(lines.begin(), lines.end(), busLineID) != lines.end())
	{
		activityState = WAITBUS_STATE_DECIDED_BOARD_BUS;	
		busDriver = driver;
		return;
	}

	const BusStop *destStop = nullptr;
	
	if (parent->destNode.type == WayPoint::BUS_STOP && parent->destNode.busStop)
	{
		destStop = parent->destNode.busStop;
	}
	else
	{
		return;
	}

	vector<const BusStop *>::const_iterator itStop = find(stopsVec.begin(), stopsVec.end(), destStop);
	
	if (itStop != stopsVec.end())
	{
		activityState = WAITBUS_STATE_DECIDED_BOARD_BUS;
		busDriver = driver;
	}
}

void WaitBusActivity::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch(type)
	{
	case MSG_WAKEUP_WAITING_PERSON:
	{
		const BusDriverMessage &busDriverMsg = MSG_CAST(BusDriverMessage, message);
		makeBoardingDecision(busDriverMsg.busDriver);
		break;
	}

	case MSG_BOARD_BUS_SUCCESS:
	{
		activityState = WAITBUS_STATE_WAIT_COMPLETE;
		storeWaitingTime(busDriver->getBusLineId());
		messaging::MessageBus::PostMessage(busDriver->getCurrBusStopAgent(), MSG_BOARD_BUS_SUCCESS,
				messaging::MessageBus::MessagePtr(new PersonMessage(parent)));
		break;
	}

	case MSG_BOARD_BUS_FAIL:
	{
		activityState = WAITBUS_STATE_WAITING;
		incrementDeniedBoardingCount();
		break;
	}
	}
}

void WaitBusActivity::storeWaitingTime(const std::string &busLine)
{
	const BusStop *busStop = busDriver->getCurrBusStopAgent()->getBusStop();
	const unsigned int currMS = getParams().now.ms();
	
	PersonWaitingTime personWaitInfo;
	personWaitInfo.busStopNo = (busStop->isVirtualStop()? busStop->getTwinStop()->getStopCode() : busStop->getStopCode());
	personWaitInfo.personId  = parent->getId();
	personWaitInfo.personIddb = parent->getDatabaseId();
	personWaitInfo.originNode = (*(parent->currTripChainItem))->origin.node->getNodeId();
	personWaitInfo.destNode = (*(parent->currTripChainItem))->destination.node->getNodeId();
	personWaitInfo.endstop = parent->currSubTrip->endLocationId;
	personWaitInfo.waitingTime = ((double) this->getWaitingTime())/1000.0; //convert ms to second
	personWaitInfo.busLineBoarded = busLine;
	personWaitInfo.busLines = this->getBusLines();
	personWaitInfo.deniedBoardingCount = this->getDeniedBoardingCount();
	personWaitInfo.currentTime = DailyTime(currMS + ConfigManager::GetInstance().FullConfig().simStartTime().getValue()).getStrRepr();
	
	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(), STORE_PERSON_WAITING,
			messaging::MessageBus::MessagePtr(new PersonWaitingTimeMessage(personWaitInfo)));
}

vector<BufferedBase *> WaitBusActivity::getSubscriptionParams()
{
	return vector<BufferedBase *>();
}
