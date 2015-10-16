/*
 * BusStopAgent.cpp
 *
 *  Created on: 17 Apr, 2014
 *      Author: zhang
 */

#include "BusStopAgent.hpp"
#include "entities/roles/waitBusActivity/waitBusActivity.hpp"
#include "message/MT_Message.hpp"
#include "entities/PT_Statistics.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;
namespace sim_mob
{

namespace medium
{

BusStopAgent::BusStopAgentsMap BusStopAgent::allBusstopAgents;

void BusStopAgent::registerBusStopAgent(BusStopAgent* busstopAgent)
{
	allBusstopAgents[busstopAgent->getBusStop()] = busstopAgent;
}

BusStopAgent* BusStopAgent::findBusStopAgentByBusStop(const BusStop* busstop)
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
		Agent(mtxStrat, id), busStop(stop), parentSegmentStats(stats), availableLength(stop->getBusCapacityAsLength()), currentTimeMS(0)
{
}

BusStopAgent::~BusStopAgent()
{
	for(std::list<sim_mob::medium::WaitBusActivity*>::iterator i= waitingPersons.begin(); i!=waitingPersons.end();i++){
		(*i)->getParent()->currWorkerProvider=nullptr;
	}

	for(std::list<sim_mob::medium::Passenger*>::iterator i = alightingPersons.begin(); i!=alightingPersons.end(); i++){
		(*i)->getParent()->currWorkerProvider=nullptr;
	}
}

void BusStopAgent::onEvent(event::EventId eventId, event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args)
{
	Agent::onEvent(eventId, ctxId, sender, args);
}

void BusStopAgent::registerWaitingPerson(sim_mob::medium::WaitBusActivity* waitingPerson)
{
	const sim_mob::BusStop* stop = this->getBusStop();
	if(stop->terminusType == sim_mob::BusStop::SINK_TERMINUS)
	{
		throw std::runtime_error("attempt to add waiting person at SINK_TERMINUS");
	}
	messaging::MessageBus::ReRegisterHandler(waitingPerson->getParent(), GetContext());
	waitingPersons.push_back(waitingPerson);
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
	Person* person = passenger->getParent();
	person->getRole()->collectTravelTime();
	alightingPersons.push_back(passenger);
}

const sim_mob::BusStop* BusStopAgent::getBusStop() const
{
	return busStop;
}

bool BusStopAgent::frame_init(timeslice now)
{
	if(!GetContext()) { messaging::MessageBus::RegisterHandler(this); }
	return true;
}

Entity::UpdateStatus BusStopAgent::frame_tick(timeslice now)
{
	currentTimeMS = now.ms()+ConfigManager::GetInstance().FullConfig().simStartTime().getValue();
	std::list<sim_mob::medium::Passenger*>::iterator itPerson = alightingPersons.begin();
	while (itPerson != alightingPersons.end())
	{
		bool ret = false;
		sim_mob::medium::Passenger* alightedPassenger = *itPerson;
		alightedPassenger->setEndNode(busStop->getParentSegment()->getEnd());
		Person* person = alightedPassenger->getParent();
		if (person)
		{
			UpdateStatus val = person->checkTripChain();
			person->setStartTime(now.ms());
			Role* role = person->getRole();
			if (role)
			{
				if (role->roleType == Role::RL_WAITBUSACTITITY && val.status == UpdateStatus::RS_CONTINUE)
				{
					WaitBusActivity* waitActivity = dynamic_cast<WaitBusActivity*>(role);
					if (waitActivity)
					{
						//always make sure we dispatch this person only to SOURCE_TERMINUS or NOT_A_TERMINUS stops
						const sim_mob::BusStop* stop = this->getBusStop();
						if(stop->terminusType == sim_mob::BusStop::SINK_TERMINUS)
						{
							stop = stop->getTwinStop();
							if(stop->terminusType == sim_mob::BusStop::SINK_TERMINUS) { throw std::runtime_error("both twin stops are SINKs"); } //sanity check
							const StreetDirectory& strDirectory = StreetDirectory::Instance();
							Agent* twinStopAgent = strDirectory.findBusStopAgentByBusStop(stop);
							if (twinStopAgent)
							{
								messaging::MessageBus::SendMessage(twinStopAgent, MSG_WAITING_PERSON_ARRIVAL_AT_BUSSTOP,
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
				else if (role->roleType == Role::RL_PEDESTRIAN && val.status == UpdateStatus::RS_CONTINUE)
				{
					Conflux* conflux = parentSegmentStats->getRoadSegment()->getParentConflux();
					messaging::MessageBus::PostMessage(conflux, MSG_PEDESTRIAN_TRANSFER_REQUEST,
							messaging::MessageBus::MessagePtr(new PersonMessage(person)));
					ret = true;
				}
				else if (role->roleType == Role::RL_PASSENGER && val.status == UpdateStatus::RS_DONE)
				{
					throw std::runtime_error("The next role of the person who just alighted at a bus stop cannot be PASSENGER");
				}
			}
		}

		if (ret)
		{
			itPerson = alightingPersons.erase(itPerson);
		}
		else
		{
			itPerson++;
		}
	}

	std::list<sim_mob::medium::WaitBusActivity*>::iterator itWaitingPeople = waitingPersons.begin();
	while (itWaitingPeople != waitingPersons.end())
	{
		Person* person = (*itWaitingPeople)->getParent();
		person->update(now);
		itWaitingPeople++;
	}

	messaging::MessageBus::PostMessage(PT_Statistics::GetInstance(),
			STORE_WAITING_AMOUNT,
			messaging::MessageBus::MessagePtr(
					new WaitingAmountMessage(busStop->getRoadItemId(),
							DailyTime(now.ms()).getStrRepr(),
							waitingPersons.size())));

	return UpdateStatus::Continue;
}

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
	case MSG_WAITING_PERSON_ARRIVAL_AT_BUSSTOP:
	{
		const ArrivalAtStopMessage& msg = MSG_CAST(ArrivalAtStopMessage, message);
		Person* person = msg.waitingPerson;
		Role* role = person->getRole();
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

void BusStopAgent::storeWaitingTime(sim_mob::medium::WaitBusActivity* waitingActivity){

	if(!waitingActivity){
		return;
	}

	Person* person = waitingActivity->getParent();
	unsigned int waitingTime = waitingActivity->getWaitingTime();
	DailyTime currDailyTime(currentTimeMS);
	DailyTime waitingDailyTime(waitingTime);
	std::string stopId = busStop->getRoadItemId();
	std::string personId = boost::lexical_cast<std::string>((person->getId()));
	std::string busLines = waitingActivity->getBusLines();
	unsigned int failedBoardingTime = waitingActivity->getFailedBoardingTimes();
	messaging::MessageBus::PostMessage(PT_Statistics::GetInstance(), STORE_PERSON_WAITING,
			messaging::MessageBus::MessagePtr(new PersonWaitingTimeMessage(stopId, personId, currDailyTime.getStrRepr(), waitingDailyTime.getStrRepr(), busLines, failedBoardingTime)));
}

void BusStopAgent::boardWaitingPersons(BusDriver* busDriver)
{
	unsigned int numBoarding = 0;
	std::list<WaitBusActivity*>::iterator itPerson;
	for (itPerson = waitingPersons.begin(); itPerson != waitingPersons.end();
			itPerson++) {
		(*itPerson)->makeBoardingDecision(busDriver);
	}

	itPerson = waitingPersons.begin();
	while (itPerson != waitingPersons.end()) {
		WaitBusActivity* waitingPeople = *itPerson;
		Person* person = waitingPeople->getParent();
		unsigned int waitingTm = waitingPeople->getWaitingTime();
		const unsigned int hourInMilliSecs = 3600000;
		if (waitingTm > hourInMilliSecs) {
			sim_mob::SubTrip& subTrip = *(person->currSubTrip);
			const std::string tripLineID = subTrip.getBusLineID();
			Warn() << "[waiting long]Person[" << person->getId()
					<< "] waiting [" << tripLineID << " ] at ["
					<< this->getBusStop()->getRoadItemId() << "] for ["
					<< DailyTime(waitingTm).getStrRepr() << "]" << std::endl;
		}
		if ((*itPerson)->canBoardBus()) {
			bool ret = false;
			if (!busDriver->checkIsFull()) {
				if (person) {
					waitingPeople->collectTravelTime();
					storeWaitingTime(waitingPeople);
					person->checkTripChain();
					Role* curRole = person->getRole();
					curRole->setArrivalTime(currentTimeMS);
					sim_mob::medium::Passenger* passenger = dynamic_cast<sim_mob::medium::Passenger*>(curRole);
					if (passenger && busDriver->addPassenger(passenger))
					{
						passenger->setStartNode(busStop->getParentSegment()->getEnd());
						passenger->Movement()->startTravelTimeMetric();
						ret = true;
					}
				}
			} else {
				waitingPeople->increaseFailedBoardingTimes();
				storeWaitingTime(waitingPeople);
			}

			if (ret) {
				itPerson = waitingPersons.erase(itPerson);
				numBoarding++;
			} else {
				(*itPerson)->setBoardBus(false);
				itPerson++;
			}
		} else {
			itPerson++;
		}
	}

	lastBoardingRecorder[busDriver] = numBoarding;
}

bool BusStopAgent::acceptBusDriver(BusDriver* driver)
{
	if (driver)
	{
		double vehicleLength = driver->getResource()->getLengthCm();
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
		double vehicleLength = driver->getResource()->getLengthCm();
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

bool BusStopAgent::canAccommodate(const double vehicleLength)
{
	return (availableLength >= vehicleLength);
}

unsigned int BusStopAgent::getBoardingNum(BusDriver* busDriver) const
{
	try
	{
		return lastBoardingRecorder.at(busDriver);
	}
	catch (const std::out_of_range& oor)
	{
		return 0;
	}
}
}
}
