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
Agent(mtxStrat, id), busStop(stop)
{
}

BusStopAgent::~BusStopAgent()
{
	for(list<Person_ST *>::iterator i= waitingPersons.begin(); i!=waitingPersons.end();i++)
	{
		(*i)->currWorkerProvider=nullptr;
	}
}

void BusStopAgent::onEvent(event::EventId eventId, event::Context ctxId, event::EventPublisher *sender, const event::EventArgs &args)
{
	Agent::onEvent(eventId, ctxId, sender, args);
}

void BusStopAgent::registerWaitingPerson(Person_ST *waitingPerson)
{
	if(busStop->getTerminusType() == SINK_TERMINUS)
	{
		throw runtime_error("Attempt to add waiting person at SINK_TERMINUS");
	}
	
	messaging::MessageBus::ReRegisterHandler(waitingPerson, GetContext());
	waitingPersons.push_back(waitingPerson);
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
	WaitingCount waitingCnt;
	waitingCnt.busStopNo = busStop->getStopCode();
	waitingCnt.currTime = DailyTime(now.ms() + ConfigManager::GetInstance().FullConfig().simulation.baseGranMS).getStrRepr();
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
	{
		const ArrivalAtStopMessage &arrivalMsg = MSG_CAST(ArrivalAtStopMessage, message);
		registerWaitingPerson(arrivalMsg.waitingPerson);
		break;
	}

	case MSG_WAKEUP_WAITING_PERSON:
	{
		const BusDriverMessage &busDriverMsg = MSG_CAST(BusDriverMessage, message);
		for (Person_ST *waitingPerson : waitingPersons)
		{
			messaging::MessageBus::PostMessage(waitingPerson, MSG_WAKEUP_WAITING_PERSON,
					messaging::MessageBus::MessagePtr(new BusDriverMessage(busDriverMsg.busDriver)));
		}
		break;
	}

	case MSG_BOARD_BUS_SUCCESS:
	{
		const PersonMessage &personMsg = MSG_CAST(PersonMessage, message);
		waitingPersons.remove(personMsg.person);
		break;
	}
	}
}
