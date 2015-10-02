//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusControllerMT.hpp"

#include <map>
#include <string>
#include <vector>
#include "buffering/Shared.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/misc/BusTrip.hpp"
#include "entities/roles/DriverRequestParams.hpp"
#include "Person_ST.hpp"

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;

BusControllerMT::BusControllerMT(int id, const MutexStrategy& mtxStrat) :
		BusController(id, mtxStrat), nextTimeTickToStage(0)
{
}

BusControllerMT::~BusControllerMT()
{
}

void BusControllerMT::processRequests()
{
	for (vector<Entity*>::iterator it = busDrivers.begin(); it != busDrivers.end(); it++)
	{
		Person_ST* person = dynamic_cast<sim_mob::Person_ST*>(*it);
		if (person)
		{
			Role<Person_ST>* role = person->getRole();
			if (role)
			{
				handleRequest(role->getDriverRequestParams());
			}
		}
	}
}

void BusControllerMT::handleRequest(DriverRequestParams rParams)
{
	//No reaction if request params is empty.
	if (rParams.asVector().empty())
	{
		return;
	}

	Shared<int>* existedRequestMode = rParams.existedRequest_Mode;
	if (existedRequestMode
			&& (existedRequestMode->get() == Role<Person_ST>::REQUEST_DECISION_TIME || existedRequestMode->get() == Role<Person_ST>::REQUEST_STORE_ARRIVING_TIME))
	{
		Shared<string>* lastVisitedBusline = rParams.lastVisited_Busline;
		Shared<int>* lastVisitedBusTripSequenceNo = rParams.lastVisited_BusTrip_SequenceNo;
		Shared<int>* busstopSequenceNo = rParams.busstop_sequence_no;
		Shared<double>* realArrivalTime = rParams.real_ArrivalTime;
		Shared<double>* dwellTime = rParams.DwellTime_ijk;
		Shared<const BusStop*>* lastVisitedBusStop = rParams.lastVisited_BusStop;
		Shared<BusStopRealTimes>* lastBusStopRealTimes = rParams.last_busStopRealTimes;
		Shared<double>* waitingTime = rParams.waiting_Time;

		if (existedRequestMode && lastVisitedBusline && lastVisitedBusTripSequenceNo && busstopSequenceNo && realArrivalTime && dwellTime && lastVisitedBusStop
				&& lastBusStopRealTimes && waitingTime)
		{
			BusStopRealTimes realTime;
			if (existedRequestMode->get() == Role<Person_ST>::REQUEST_DECISION_TIME)
			{
				double waitingtime = computeDwellTime(lastVisitedBusline->get(), lastVisitedBusTripSequenceNo->get(), busstopSequenceNo->get(),
						realArrivalTime->get(), dwellTime->get(), realTime, lastVisitedBusStop->get());
				waitingTime->set(waitingtime);
			}
			else if (existedRequestMode->get() == Role<Person_ST>::REQUEST_STORE_ARRIVING_TIME)
			{
				storeRealTimesAtEachBusStop(lastVisitedBusline->get(), lastVisitedBusTripSequenceNo->get(), busstopSequenceNo->get(), realArrivalTime->get(),
						dwellTime->get(), lastVisitedBusStop->get(), realTime);
			}
			lastBusStopRealTimes->set(realTime);
		}
	}

}

void BusControllerMT::assignBusTripChainWithPerson(std::set<Entity*>& activeAgents)
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	const map<string, BusLine*>& buslines = ptSchedule.getBusLines();
	if (0 == buslines.size())
	{
		throw std::runtime_error("Error:  No busline in the ptSchedule, please check the setPTSchedule.");
	}

	for (map<string, BusLine*>::const_iterator buslinesIt = buslines.begin(); buslinesIt != buslines.end(); buslinesIt++)
	{
		BusLine* busline = buslinesIt->second;
		const vector<BusTrip>& busTrips = busline->queryBusTrips();

		for (vector<BusTrip>::const_iterator tripIt = busTrips.begin(); tripIt != busTrips.end(); tripIt++)
		{
			if (tripIt->startTime.isAfterEqual(ConfigManager::GetInstance().FullConfig().simStartTime()))
			{
				Person_ST* person = new Person_ST("BusController", config.mutexStategy(), -1, tripIt->getPersonID());
				person->setPersonCharacteristics();
				vector<TripChainItem*> tripChain;
				tripChain.push_back(const_cast<BusTrip*>(&(*tripIt)));
				person->setTripChain(tripChain);
				person->initTripChain();
				addOrStashBuses(person, activeAgents);
			}
		}
	}

	for (std::set<Entity*>::iterator it = activeAgents.begin(); it != activeAgents.end(); it++)
	{
		(*it)->parentEntity = this;
		busDrivers.push_back(*it);
	}

}
