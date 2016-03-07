/*
 * BusStopAgent.cpp
 *
 *  Created on: 17 Apr, 2014
 *      Author: zhang
 */

#include "BusStopAgent.hpp"
#include "message/MT_Message.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/PT_Statistics.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

namespace
{
	const unsigned int ONE_HOUR_IN_MS = 3600000;
}

namespace sim_mob
{
namespace medium
{
BusStopAgent::BusStopAgentsMap BusStopAgent::allBusstopAgents;

void BusStopAgent::registerBusStopAgent(BusStopAgent* busstopAgent)
{
	allBusstopAgents[busstopAgent->getBusStop()] = busstopAgent;
}

BusStopAgent* BusStopAgent::getBusStopAgentForStop(const BusStop* busstop)
{
	BusStopAgentsMap::const_iterator stpAgIt = allBusstopAgents.find(busstop);
	if(stpAgIt == allBusstopAgents.end()) { return nullptr; }
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


BusStopAgent::BusStopAgent(const MutexStrategy& mtxStrat, int id, const BusStop* stop, SegmentStats* stats) :
		Agent(mtxStrat, id), busStop(stop), parentSegmentStats(stats), availableLength(stop->getLength()), currentTimeMS(0)
{}

BusStopAgent::~BusStopAgent()
{
	for(std::list<sim_mob::medium::WaitBusActivity*>::iterator i= waitingPersons.begin(); i!=waitingPersons.end();i++)
	{
		(*i)->getParent()->currWorkerProvider=nullptr;
	}

	for(std::list<sim_mob::medium::Passenger*>::iterator i = alightingPersons.begin(); i!=alightingPersons.end(); i++)
	{
		(*i)->getParent()->currWorkerProvider=nullptr;
	}
}

void BusStopAgent::onEvent(event::EventId eventId, event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args)
{
	Agent::onEvent(eventId, ctxId, sender, args);
}

void BusStopAgent::registerWaitingPerson(sim_mob::medium::WaitBusActivity* waitingPerson)
{
	if(busStop->getTerminusType() == sim_mob::SINK_TERMINUS)
	{
		throw std::runtime_error("attempt to add waiting person at SINK_TERMINUS");
	}
	messaging::MessageBus::ReRegisterHandler(waitingPerson->getParent(), GetContext());
	waitingPersons.push_back(waitingPerson);
	waitingPerson->setStop(busStop);
}

void BusStopAgent::removeWaitingPerson(sim_mob::medium::WaitBusActivity* waitingPerson)
{
	std::list<sim_mob::medium::WaitBusActivity*>::iterator itPerson;
	itPerson = std::find(waitingPersons.begin(), waitingPersons.end(), waitingPerson);
	if (itPerson != waitingPersons.end())
	{
		waitingPersons.erase(itPerson);
	}
}

void BusStopAgent::addAlightingPerson(sim_mob::medium::Passenger* passenger)
{
	Person_MT* person = passenger->getParent();
	person->getRole()->collectTravelTime();
	alightingPersons.push_back(passenger);
}

const sim_mob::BusStop* BusStopAgent::getBusStop() const
{
	return busStop;
}

Entity::UpdateStatus BusStopAgent::frame_init(timeslice now)
{
	if(!GetContext()) { messaging::MessageBus::RegisterHandler(this); }
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus BusStopAgent::frame_tick(timeslice now)
{
	currentTimeMS = now.ms()+ConfigManager::GetInstance().FullConfig().simStartTime().getValue();
	std::list<sim_mob::medium::Passenger*>::iterator personIt = alightingPersons.begin();
	while (personIt != alightingPersons.end())
	{
		bool ret = false;
		sim_mob::medium::Passenger* alightedPassenger = *personIt;
		alightedPassenger->setEndPoint(WayPoint(busStop));
		Person_MT* person = alightedPassenger->getParent();
		if (person)
		{
			UpdateStatus val = person->checkTripChain(now.ms());
			person->setStartTime(now.ms());
			Role<Person_MT>* role = person->getRole();
			if (role)
			{
				if (role->roleType == Role<Person_MT>::RL_WAITBUSACTIVITY && val.status == UpdateStatus::RS_CONTINUE)
				{
					WaitBusActivity* waitActivity = dynamic_cast<WaitBusActivity*>(role);
					if (waitActivity)
					{
						//always make sure we dispatch this person only to SOURCE_TERMINUS or NOT_A_TERMINUS stops
						const sim_mob::BusStop* stop = busStop;
						if(stop->getTerminusType() == sim_mob::SINK_TERMINUS)
						{
							stop = stop->getTwinStop();
							if(stop->getTerminusType() == sim_mob::SINK_TERMINUS) //sanity check
							{
								throw std::runtime_error("both twin stops are SINKs");
							}
							BusStopAgent* twinStopAgent = BusStopAgent::getBusStopAgentForStop(stop);
							if (twinStopAgent)
							{
								messaging::MessageBus::SendMessage(twinStopAgent, MSG_WAITING_PERSON_ARRIVAL,
										messaging::MessageBus::MessagePtr(new ArrivalAtStopMessage(person)));
							}
						}
						else
						{
							registerWaitingPerson(waitActivity);
						}
						ret = true;
					}
				}
				else if (role->roleType == Role<Person_MT>::RL_PEDESTRIAN && val.status == UpdateStatus::RS_CONTINUE)
				{
					Conflux* conflux = parentSegmentStats->getParentConflux();
					messaging::MessageBus::PostMessage(conflux, sim_mob::medium::MSG_PEDESTRIAN_TRANSFER_REQUEST,
							messaging::MessageBus::MessagePtr(new PersonMessage(person)));
					ret = true;
				}
				else if (role->roleType == Role<Person_MT>::RL_PASSENGER && val.status == UpdateStatus::RS_DONE)
				{
					throw std::runtime_error("The next role of the person who just alighted at a bus stop cannot be PASSENGER");
				}
			}
		}

		if (ret)
		{
			personIt = alightingPersons.erase(personIt);
		}
		else
		{
			personIt++;
		}
	}

	std::list<sim_mob::medium::WaitBusActivity*>::iterator itWaitBusRole = waitingPersons.begin();
	while (itWaitBusRole != waitingPersons.end())
	{
		(*itWaitBusRole)->Movement()->frame_tick();
		itWaitBusRole++;
	}

//  COMMENTED FOR CALIBRATION ~Harish
//	sim_mob::WaitingCount waitingCnt;
//	waitingCnt.busStopNo = busStop->getStopCode();
//	waitingCnt.currTime = DailyTime(now.ms()).getStrRepr();
//	waitingCnt.count = waitingPersons.size();
//	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(), STORE_WAITING_PERSON_COUNT,
//											messaging::MessageBus::MessagePtr(new WaitingCountMessage(waitingCnt)));

	for(auto* busDriver : servingDrivers)
	{
		if(busDriver->getResource()->isMoving())
		{
			std::stringstream err;
			err << "bus driver " << busDriver->getParent()->getId() << "(" << busDriver->getParent()->busLine << ")"
					<< "has isMoving true while serving stop\n";
			throw std::runtime_error(err.str());
		}
	}

	return UpdateStatus::Continue;
}

void BusStopAgent::frame_output(timeslice now)
{}

bool BusStopAgent::isNonspatial()
{
	return false;
}

void BusStopAgent::load(const std::map<std::string, std::string>& configProps)
{}

void BusStopAgent::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch (type)
	{
	case BUS_ARRIVAL:
	{
		const BusDriverMessage& msg = MSG_CAST(BusDriverMessage, message);
		bool busDriverAccepted = acceptBusDriver(msg.busDriver);
		if (!busDriverAccepted)
		{
			throw std::runtime_error("BusDriver could not be accepted by the bus stop");
		}
		boardWaitingPersons(msg.busDriver);
		break;
	}
	case BUS_DEPARTURE:
	{
		const BusDriverMessage& msg = MSG_CAST(BusDriverMessage, message);
		bool busDriverRemoved = removeBusDriver(msg.busDriver);
		if (!busDriverRemoved)
		{
			throw std::runtime_error("BusDriver could not be found in bus stop");
		}
		break;
	}
	case MSG_WAITING_PERSON_ARRIVAL:
	{
		const ArrivalAtStopMessage& msg = MSG_CAST(ArrivalAtStopMessage, message);
		Person_MT* person = msg.waitingPerson;
		Role<Person_MT>* role = person->getRole();
		if (role)
		{
			WaitBusActivity* waitPerson = dynamic_cast<WaitBusActivity*>(role);
			if (waitPerson)
			{
				registerWaitingPerson(waitPerson);
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

bool BusStopAgent::handleBusArrival(BusDriver* busDriver)
{
	if (busDriver && acceptBusDriver(busDriver))
	{
		boardWaitingPersons(busDriver);
		return true;
	}
	return false;
}

bool BusStopAgent::handleBusDeparture(BusDriver* busDriver)
{
	return removeBusDriver(busDriver);
}

void BusStopAgent::storeWaitingTime(sim_mob::medium::WaitBusActivity* waitingActivity) const
{
	if(!waitingActivity) { return; }
	PersonWaitingTime personWaitInfo;
	personWaitInfo.busStopNo = busStop->getStopCode();
	personWaitInfo.personId  = waitingActivity->getParent()->getId();
	personWaitInfo.currentTime = DailyTime(currentTimeMS).getStrRepr();
	personWaitInfo.waitingTime = ((double) waitingActivity->getWaitingTime())/1000.0; //convert ms to second
	personWaitInfo.busLine = waitingActivity->getBusLines();
	personWaitInfo.deniedBoardingCount = waitingActivity->getDeniedBoardingCount();
	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(), STORE_PERSON_WAITING,
			messaging::MessageBus::MessagePtr(new PersonWaitingTimeMessage(personWaitInfo)));
	std::string busLines = waitingActivity->getBusLines();
}

void BusStopAgent::boardWaitingPersons(BusDriver* busDriver)
{
	unsigned int numBoarding = 0;
	std::list<WaitBusActivity*>::iterator itWaitingPerson;
	for (itWaitingPerson = waitingPersons.begin(); itWaitingPerson != waitingPersons.end(); itWaitingPerson++)
	{
		(*itWaitingPerson)->makeBoardingDecision(busDriver);
	}

	itWaitingPerson = waitingPersons.begin();
	while (itWaitingPerson != waitingPersons.end())
	{
		WaitBusActivity* waitingRole = *itWaitingPerson;
		Person_MT* person = waitingRole->getParent();
//		COMMENTED FOR CALIBRATION ~Harish
//		unsigned int waitingTm = waitingRole->getWaitingTime();
//		if (waitingTm > ONE_HOUR_IN_MS)
//		{
//			const sim_mob::SubTrip& subTrip = *(person->currSubTrip);
//			Warn() << "waiting_long,"
//					<< person->getDatabaseId() << ","
//					<< subTrip.getBusLineID() <<","
//					<< busStop->getStopCode() << ","
//					<< DailyTime(waitingTm).getStrRepr() << std::endl;
//		}
		if ((*itWaitingPerson)->canBoardBus())
		{
			bool ret = false;
			if (!busDriver->checkIsFull())
			{
				waitingRole->collectTravelTime();
				storeWaitingTime(waitingRole);
				DailyTime current(DailyTime(currentTimeMS).offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime()));
				person->checkTripChain(current.getValue());
				Role<Person_MT>* curRole = person->getRole();
				curRole->setArrivalTime(currentTimeMS);
				sim_mob::medium::Passenger* passenger = dynamic_cast<sim_mob::medium::Passenger*>(curRole);
				if (passenger)
				{
					busDriver->addPassenger(passenger);
					passenger->setStartPoint(WayPoint(busStop));
					passenger->Movement()->startTravelTimeMetric();
					ret = true;
				}
				else
				{
					throw std::runtime_error("next role after wait bus activity is not passenger");
				}
			}
			else
			{
				waitingRole->incrementDeniedBoardingCount();
				storeWaitingTime(waitingRole);
			}

			if (ret)
			{
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

bool BusStopAgent::acceptBusDriver(BusDriver* driver)
{
	if (driver)
	{
		double vehicleLength = driver->getResource()->getLengthInM();
		if (availableLength >= vehicleLength)
		{
			servingDrivers.push_back(driver);
			availableLength = availableLength - vehicleLength;
			parentSegmentStats->addBusDriverToStop(driver->getParent(), busStop);
			return true;
		}
	}
	return false;
}

bool BusStopAgent::removeBusDriver(BusDriver* driver)
{
	if (driver)
	{
		double vehicleLength = driver->getResource()->getLengthInM();
		std::list<sim_mob::medium::BusDriver*>::iterator driverIt = std::find(servingDrivers.begin(), servingDrivers.end(), driver);
		if (driverIt != servingDrivers.end())
		{
			servingDrivers.erase(driverIt);
			availableLength = availableLength + vehicleLength;
			parentSegmentStats->removeBusDriverFromStop(driver->getParent(), busStop);
			return true;
		}
	}
	return false;
}

bool BusStopAgent::canAccommodate(double vehicleLength) const
{
	return (availableLength >= vehicleLength);
}

unsigned int BusStopAgent::getBoardingNum(BusDriver* busDriver) const
{
	std::map<sim_mob::medium::BusDriver*, unsigned int>::const_iterator lbrIt = lastBoardingRecorder.find(busDriver);
	if(lbrIt == lastBoardingRecorder.end()) { return 0; }
	else { return lbrIt->second; }
}

const SegmentStats* BusStopAgent::getParentSegmentStats() const
{
	return parentSegmentStats;
}

unsigned int BusStopAgent::getWaitingCount() const
{
	return waitingPersons.size();
}
}
}
