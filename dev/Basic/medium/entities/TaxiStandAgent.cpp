/*
 * TaxiStandAgent.cpp
 *
 *  Created on: Nov 7, 2016
 *      Author: zhang huai peng
 */

#include "entities/TaxiStandAgent.hpp"
#include "message/MessageBus.hpp"
#include "entities/roles/driver/TaxiDriver.hpp"
#include "message/MT_Message.hpp"
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
	setParentConflux();
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

	auto itWaitDriver = queuingDrivers.begin();
	while(itWaitDriver != queuingDrivers.end())
	{
		parentConflux->updateQueuingTaxiDriverAgent((*itWaitDriver));
		TaxiDriver *driver = dynamic_cast<TaxiDriver*>((*itWaitDriver)->getRole());
		if(!driver || driver->getMovementFacet()->isToBeRemovedFromTaxiStand())
		{
			itWaitDriver = queuingDrivers.erase(itWaitDriver);
			continue;
		}
		itWaitDriver++;
	}
	return Entity::UpdateStatus::Continue;
}

void TaxiStandAgent::setParentConflux()
{
	const RoadSegment *roadSegment = taxiStand->getRoadSegment();
	parentConflux =Conflux::getConflux(roadSegment);
}

Conflux * TaxiStandAgent::getParentConflux()
{
	return parentConflux;
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

TaxiStandAgent* TaxiStandAgent::getTaxiStandAgent(const TaxiStand* stand)
{
	auto it = allTaxiStandAgents.find(stand);
	if(it!=allTaxiStandAgents.end()){
		return (*it).second;
	}
	return nullptr;
}

const TaxiStand* TaxiStandAgent::getTaxiStand(int standId)
{
	for(auto it = allTaxiStandAgents.begin(); it!=allTaxiStandAgents.end(); it++)
	{
		if((*it).first->getStandId()==standId)
		{
			return (*it).first;
		}
	}
	return nullptr;
}
bool TaxiStandAgent::acceptTaxiDriver(Person_MT* driver)
{
	if (queuingDrivers.size() < capacity) {
		queuingDrivers.push_back(driver);
		return true;
	}
	return false;
}

bool TaxiStandAgent::isTaxiFirstInQueue(TaxiDriver *taxiDriver)
{
	std::deque<Person_MT*>::iterator itr = std::find( queuingDrivers.begin(), queuingDrivers.end() ,taxiDriver->getParent() );
	if (itr == queuingDrivers.begin())
	{
		return true;
	}
}

Person_MT* TaxiStandAgent::pickupOneWaitingPerson()
{
	Person_MT* res = nullptr;
	if(waitingPeople.size()>0)
	{
		res = waitingPeople.front();
		waitingPeople.pop_front();
		storeWaitingTime(res);
		res->getRole()->collectTravelTime();
		UpdateStatus status = res->checkTripChain(currentTimeMS);
		if (status.status == UpdateStatus::RS_DONE)
		{
			return nullptr;
		}
		res->getRole()->setArrivalTime(currentTimeMS);
	}
	return res;
}
void TaxiStandAgent::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch (type)
	{
	case MSG_WAITING_PERSON_ARRIVAL:
	{
		const ArrivalAtStopMessage& msg = MSG_CAST(ArrivalAtStopMessage, message);
		Person_MT* person = msg.waitingPerson;
		Role<Person_MT>* role = person->getRole();
		if (role)
		{
			WaitTaxiActivity* waitPerson = dynamic_cast<WaitTaxiActivity*>(role);
			if (waitPerson)
			{
				addWaitingPerson(person);
			}
		}
		break;
	}
	default:
	{
		break;
	}
	}
}

void TaxiStandAgent::storeWaitingTime(Person_MT* waitingPerson) const
{
	if(waitingPerson){
		WaitTaxiActivity* activity = dynamic_cast<WaitTaxiActivity*>(waitingPerson->getRole());
		if(activity){
			unsigned int waitingTime = activity->getWaitingTime();
			PersonWaitingTime personWaitInfo;
			personWaitInfo.busStopNo = boost::lexical_cast<std::string>(taxiStand->getStandId());
			personWaitInfo.personId  = waitingPerson->getId();
			personWaitInfo.personIddb = waitingPerson->getDatabaseId();
			personWaitInfo.originNode = (*(waitingPerson->currTripChainItem))->origin.node->getNodeId();
			personWaitInfo.destNode = (*(waitingPerson->currTripChainItem))->destination.node->getNodeId();
			personWaitInfo.endstop = boost::lexical_cast<std::string>(personWaitInfo.destNode);
			personWaitInfo.currentTime = DailyTime(currentTimeMS + ConfigManager::GetInstance().FullConfig().simulation.baseGranMS).getStrRepr();
			personWaitInfo.busLines = "Taxi";
			personWaitInfo.busLineBoarded = "Taxi";
			personWaitInfo.deniedBoardingCount = 0;
			personWaitInfo.waitingTime = waitingTime/1000;
			messaging::MessageBus::PostMessage(PT_Statistics::getInstance(), STORE_PERSON_WAITING,
					messaging::MessageBus::MessagePtr(new PersonWaitingTimeMessage(personWaitInfo)));
		}
	}
}

}
}

