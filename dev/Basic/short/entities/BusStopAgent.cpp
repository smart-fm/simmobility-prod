//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusStopAgent.hpp"
#include "message/ST_Message.hpp"
#include "entities/PT_Statistics.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace std;
using namespace sim_mob;

namespace
{
	const unsigned int ONE_HOUR_IN_MS = 3600000;
}

BusStopAgent::BusStopAgentsMap BusStopAgent::allBusstopAgents;

void BusStopAgent::registerBusStopAgent(BusStopAgent *busStopAgent, WorkGroup &workGroup)
{
	allBusstopAgents[busStopAgent->getBusStop()] = busStopAgent;
	workGroup.assignAWorker(busStopAgent);
}

BusStopAgent* BusStopAgent::getBusStopAgentForStop(const BusStop *busStop)
{
	BusStopAgentsMap::const_iterator stpAgIt = allBusstopAgents.find(busStop);
	
	if(stpAgIt == allBusstopAgents.end())
	{
		return nullptr;
	}
	
	return stpAgIt->second;
}

void BusStopAgent::removeAllBusStopAgents()
{
	BusStopAgent::BusStopAgentsMap::iterator busStopAgIt = allBusstopAgents.begin();
	
	while(busStopAgIt != allBusstopAgents.end())
	{
		safe_delete_item( (*busStopAgIt).second);
		busStopAgIt++;
	}
	
	allBusstopAgents.clear();
}

BusStopAgent::BusStopAgent(const MutexStrategy &mtxStrat, int id, const BusStop *stop) :
Agent(mtxStrat, id), busStop(stop), currentTimeMS(0)
{
}

BusStopAgent::~BusStopAgent()
{
	for(list<WaitBusActivity*>::iterator i= waitingPersons.begin(); i!=waitingPersons.end();i++)
	{
		(*i)->getParent()->currWorkerProvider=nullptr;
	}

	for(list<Passenger*>::iterator i = alightingPersons.begin(); i!=alightingPersons.end(); i++)
	{
		(*i)->getParent()->currWorkerProvider=nullptr;
	}
}

void BusStopAgent::onEvent(event::EventId eventId, event::Context ctxId, event::EventPublisher *sender, const event::EventArgs &args)
{
	Agent::onEvent(eventId, ctxId, sender, args);
}

void BusStopAgent::registerWaitingPerson(WaitBusActivity *waitingPerson)
{
	if(busStop->getTerminusType() == SINK_TERMINUS)
	{
		throw runtime_error("Attempt to add waiting person at SINK_TERMINUS");
	}
	
	messaging::MessageBus::ReRegisterHandler(waitingPerson->getParent(), GetContext());
	waitingPersons.push_back(waitingPerson);
	waitingPerson->setStop(busStop);
}

void BusStopAgent::removeWaitingPerson(WaitBusActivity *waitingPerson)
{
	list<WaitBusActivity*>::iterator itPerson;
	itPerson = find(waitingPersons.begin(), waitingPersons.end(), waitingPerson);
	
	if (itPerson != waitingPersons.end())
	{
		waitingPersons.erase(itPerson);
	}
}

void BusStopAgent::addAlightingPerson(Passenger *passenger)
{
	alightingPersons.push_back(passenger);
}

const BusStop* BusStopAgent::getBusStop() const
{
	return busStop;
}

Entity::UpdateStatus BusStopAgent::frame_init(timeslice now)
{
	if(!GetContext())
	{
		messaging::MessageBus::RegisterHandler(this);
	}
	
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus BusStopAgent::frame_tick(timeslice now)
{
	currentTimeMS = now.ms()+ConfigManager::GetInstance().FullConfig().simStartTime().getValue();
	list<Passenger *>::iterator personIt = alightingPersons.begin();
	
	while (personIt != alightingPersons.end())
	{
		(*personIt)->setAlightVehicle(true);
		personIt = alightingPersons.erase(personIt);
	}

	WaitingCount waitingCnt;
	waitingCnt.busStopNo = busStop->getStopCode();
	waitingCnt.currTime = DailyTime(now.ms()).getStrRepr();
	waitingCnt.count = waitingPersons.size();
	
	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(), STORE_WAITING_PERSON_COUNT,
											messaging::MessageBus::MessagePtr(new WaitingCountMessage(waitingCnt)));

	return UpdateStatus::Continue;
}

void BusStopAgent::frame_output(timeslice now)
{
}

bool BusStopAgent::isNonspatial()
{
	return false;
}

void BusStopAgent::load(const map<string, string> &configProps)
{
}

void BusStopAgent::HandleMessage(messaging::Message::MessageType type, const messaging::Message &message)
{
	switch (type)
	{			
	case MSG_WAITING_PERSON_ARRIVAL:
		const ArrivalAtStopMessage &msg = MSG_CAST(ArrivalAtStopMessage, message);
		Person_ST *person = msg.waitingPerson;
		Role<Person_ST> *role = person->getRole();
		
		if (role)
		{
			WaitBusActivity *waitPerson = dynamic_cast<WaitBusActivity *>(role);
			
			if (waitPerson)
			{
				registerWaitingPerson(waitPerson);
			}
		}
		
		break;
	}
}

void BusStopAgent::storeWaitingTime(WaitBusActivity *waitingActivity, const string &busLine) const
{
	if(!waitingActivity)
	{
		return;
	}
	
	Person_ST *person = waitingActivity->getParent();
	PersonWaitingTime personWaitInfo;
	personWaitInfo.busStopNo = (busStop->isVirtualStop()? busStop->getTwinStop()->getStopCode() : busStop->getStopCode());
	personWaitInfo.personId  = person->getId();
	personWaitInfo.personIddb = person->getDatabaseId();
	personWaitInfo.originNode = (*(person->currTripChainItem))->origin.node->getNodeId();
	personWaitInfo.destNode = (*(person->currTripChainItem))->destination.node->getNodeId();
	personWaitInfo.endstop = person->currSubTrip->endLocationId;
	personWaitInfo.currentTime = DailyTime(currentTimeMS + ConfigManager::GetInstance().FullConfig().simulation.baseGranMS).getStrRepr(); //person is boarded at the end of tick
	personWaitInfo.waitingTime = ((double) waitingActivity->getWaitingTime())/1000.0; //convert ms to second
	personWaitInfo.busLineBoarded = busLine;
	personWaitInfo.busLines = waitingActivity->getBusLines();
	personWaitInfo.deniedBoardingCount = waitingActivity->getDeniedBoardingCount();
	
	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(), STORE_PERSON_WAITING,
			messaging::MessageBus::MessagePtr(new PersonWaitingTimeMessage(personWaitInfo)));
}

double BusStopAgent::boardWaitingPersons(BusDriver *busDriver)
{
	double boardingTime = 0;
	unsigned int numBoarding = 0;
	list<WaitBusActivity *>::iterator itWaitingPerson = waitingPersons.begin();
	
	while (itWaitingPerson != waitingPersons.end())
	{
		//Make boarding decision
		(*itWaitingPerson)->makeBoardingDecision(busDriver);
		
		WaitBusActivity *waitingRole = *itWaitingPerson;
		Person_ST *person = waitingRole->getParent();

		//Check if the person can board the bus
		if ((*itWaitingPerson)->canBoardBus())
		{
			bool ret = false;
			
			if (!busDriver->isBusFull())
			{
				waitingRole->collectTravelTime();
				storeWaitingTime(waitingRole, busDriver->getBusLineId());
				
				DailyTime current(DailyTime(currentTimeMS).offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime()));
				person->checkTripChain(current.getValue());
				
				Role<Person_ST> *curRole = person->getRole();
				
				curRole->setArrivalTime(currentTimeMS);
				
				Passenger *passenger = dynamic_cast<Passenger *>(curRole);
				
				if (passenger)
				{
					busDriver->addPassenger(passenger);
					ret = true;
				}
				else
				{
					stringstream msg;
					msg << "Role after wait bus activity is not passenger for Person: " << person->getDatabaseId();
					throw runtime_error(msg.str());
				}
			}
			else
			{
				waitingRole->incrementDeniedBoardingCount();
			}

			if (ret)
			{
				boardingTime += (*itWaitingPerson)->getParent()->getBoardingCharacteristics();
				itWaitingPerson = waitingPersons.erase(itWaitingPerson);
				numBoarding++;
			}
			else
			{
				(*itWaitingPerson)->setBoardBus(false);
				itWaitingPerson++;
			}
		}
		else
		{
			itWaitingPerson++;
		}
	}

	lastBoardingRecorder[busDriver] = numBoarding;
}

unsigned int BusStopAgent::getBoardingNum(BusDriver* busDriver) const
{
	map<BusDriver*, unsigned int>::const_iterator lbrIt = lastBoardingRecorder.find(busDriver);
	if(lbrIt == lastBoardingRecorder.end()) { return 0; }
	else { return lbrIt->second; }
}

unsigned int BusStopAgent::getWaitingCount() const
{
	return waitingPersons.size();
}
