/*
 * TaxiStandAgent.cpp
 *
 *  Created on: Nov 7, 2016
 *      Author: zhang huai peng
 */

#include "entities/TaxiStandAgent.hpp"
#include "message/MessageBus.hpp"
#include "roles/waitTaxiActivity/WaitTaxiActivity.hpp"
#include "entities/PT_Statistics.hpp"
#include "geospatial/network/TaxiStand.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

namespace sim_mob
{
namespace medium
{
std::map<const TaxiStand*, TaxiStandAgent*> TaxiStandAgent::allTaxiStandAgents;

TaxiStandAgent::TaxiStandAgent(const MutexStrategy& mtxStrat, int id, const TaxiStand* stand, SegmentStats* stats):taxiStand(stand),parentSegmentStats(stats),currentTimeMS(0),capacity(10),Agent(mtxStrat, id)
{

}

TaxiStandAgent::~TaxiStandAgent()
{

}

Entity::UpdateStatus TaxiStandAgent::frame_init(timeslice now)
{
	if(!GetContext())
	{
		messaging::MessageBus::RegisterHandler(this);
	}
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus TaxiStandAgent::frame_tick(timeslice now)
{
	currentTimeMS = now.ms()+ConfigManager::GetInstance().FullConfig().simStartTime().getValue();
	auto itWaitPerson = waitingPeople.begin();
	while (itWaitPerson != waitingPeople.end())
	{
		(*itWaitPerson)->getRole()->Movement()->frame_tick();
		itWaitPerson++;
	}
	return Entity::UpdateStatus::Continue;
}

void TaxiStandAgent::frame_output(timeslice now)
{

}

bool TaxiStandAgent::isNonspatial()
{
	return false;
}

void TaxiStandAgent::setTaxiStand(const TaxiStand* stand)
{
	taxiStand = stand;
}

const TaxiStand* TaxiStandAgent::getTaxiStand() const
{
	return taxiStand;
}

void TaxiStandAgent::addWaitingPerson(medium::Person_MT* person)
{
	waitingPeople.push_back(person);
}

void TaxiStandAgent::registerTaxiStandAgent(TaxiStandAgent* agent)
{
	if(agent && agent->getTaxiStand() )
	{
		allTaxiStandAgents[agent->getTaxiStand()] = agent;
	}
}

bool TaxiStandAgent::acceptTaxiDriver(Person_MT* driver)
{
	if (queuingDrivers.size() < capacity) {
		queuingDrivers.push_back(driver);
		return true;
	}
	return false;
}

Person_MT* TaxiStandAgent::pickupOneWaitingPerson()
{
	Person_MT* res = nullptr;
	if(waitingPeople.size()>0)
	{
		res = waitingPeople.front();
		waitingPeople.pop_front();
		storeWaitingTime(res);
	}
	return res;
}
void TaxiStandAgent::storeWaitingTime(Person_MT* waitingPerson) const
{
	if(waitingPerson){
		WaitTaxiActivity* activity = dynamic_cast<WaitTaxiActivity*>(waitingPerson->getRole());
		if(activity){
			unsigned int waitingTime = activity->getWaitingTime();
			PersonWaitingTime personWaitInfo;
			personWaitInfo.busStopNo = taxiStand->getStandId();
			personWaitInfo.personId  = waitingPerson->getId();
			personWaitInfo.personIddb = waitingPerson->getDatabaseId();
			personWaitInfo.originNode = (*(waitingPerson->currTripChainItem))->origin.node->getNodeId();
			personWaitInfo.destNode = (*(waitingPerson->currTripChainItem))->destination.node->getNodeId();
			personWaitInfo.endstop = waitingPerson->currSubTrip->endLocationId;
			personWaitInfo.currentTime = DailyTime(currentTimeMS + ConfigManager::GetInstance().FullConfig().simulation.baseGranMS).getStrRepr();
			personWaitInfo.waitingTime = waitingTime;
			messaging::MessageBus::PostMessage(PT_Statistics::getInstance(), STORE_PERSON_WAITING,
					messaging::MessageBus::MessagePtr(new PersonWaitingTimeMessage(personWaitInfo)));
		}
	}
}

}
}
