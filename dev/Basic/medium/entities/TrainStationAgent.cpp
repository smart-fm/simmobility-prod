/*
 * TrainStationAgent.cpp
 *
 *  Created on: Feb 24, 2016
 *      Author: zhang huai peng
 */

#include "TrainStationAgent.hpp"
#include "message/MT_Message.hpp"
#include "entities/roles/driver/TrainDriver.hpp"
#include "entities/Person_MT.hpp"
#include "entities/TrainController.hpp"
#include "entities/conflux/Conflux.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "message/MT_Message.hpp"
#include "message/MessageBus.hpp"
#include "message/MessageHandler.hpp"
namespace sim_mob {
namespace medium
{
TrainStationAgent::TrainStationAgent():station(nullptr),Agent(MutexStrategy::MtxStrat_Buffered, -1), parentConflux(nullptr) {
	// TODO Auto-generated constructor stub

}

TrainStationAgent::~TrainStationAgent() {
	// TODO Auto-generated destructor stub
}

void TrainStationAgent::setStation(const Station* station)
{
	this->station = station;
}
void TrainStationAgent::setConflux(Conflux* conflux)
{
	parentConflux = conflux;
}
void TrainStationAgent::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch (type)
	{
	case TRAIN_MOVETO_NEXT_PLATFORM:
	{
		const TrainDriverMessage& msg = MSG_CAST(TrainDriverMessage, message);
		msg.trainDriver->getParent()->currWorkerProvider = currWorkerProvider;
		trainDriver.push_back(msg.trainDriver);
		msg.trainDriver->setCurrentStatus(TrainDriver::MOVE_TO_PLATFROM);
		break;
	}
	case TRAIN_ARRIVAL_AT_STARTPOINT:
	{
		const TrainDriverMessage& msg = MSG_CAST(TrainDriverMessage, message);
		msg.trainDriver->setCurrentStatus(TrainDriver::MOVE_TO_PLATFROM);
		std::string lineId = msg.trainDriver->getTrainLine();
		if(pendingTrainDriver.find(lineId)==pendingTrainDriver.end()){
			pendingTrainDriver[lineId] = std::list<TrainDriver*>();
		}
		pendingTrainDriver[lineId].push_back(msg.trainDriver);
		break;
	}
	case TRAIN_ARRIVAL_AT_ENDPOINT:
	{
		const TrainDriverMessage& msg = MSG_CAST(TrainDriverMessage, message);
		removeAheadTrain(msg.trainDriver);
		break;
	}
	case PASSENGER_ARRIVAL_AT_PLATFORM:
	{
		const PersonMessage& msg = MSG_CAST(PersonMessage, message);
		WaitTrainActivity* waitingPerson = dynamic_cast<WaitTrainActivity*>(msg.person->getRole());
		if(waitingPerson){
			const Platform* platform = waitingPerson->getStartPlatform();
			if(platform){
				waitingPersons[platform].push_back(waitingPerson);
			} else {
				throw std::runtime_error("the waiting person don't know starting platform.");
			}
		} else {
			throw std::runtime_error("the person arriving at platform is not waiting role.");
		}
		break;
	}
	}
}

void TrainStationAgent::dispathPendingTrains(timeslice now)
{
	std::map<std::string, std::list<TrainDriver*>>::iterator iPending;
	for (iPending = pendingTrainDriver.begin();
			iPending != pendingTrainDriver.end(); iPending++) {
		std::list<TrainDriver*>& pendingDrivers = (*iPending).second;
		std::string lineId = iPending->first;
		if(!pendingDrivers.empty() && pendingDrivers.front()->getParent()->getStartTime()<=now.ms()){
			bool isUsed = false;
			std::map<std::string, bool>::iterator iUsed = lastUsage.find(lineId);
			if(iUsed!=lastUsage.end()){
				isUsed = iUsed->second;
			} else {
				lastUsage[lineId] = false;
			}
			if(!isUsed){
				TrainDriver* next = pendingDrivers.front();
				trainDriver.push_back(next);
				pendingDrivers.pop_front();
				lastUsage[lineId] = true;
				std::map<std::string, TrainDriver*>::iterator iLastDriver;
				iLastDriver = lastTrainDriver.find(lineId);
				if(iLastDriver!=lastTrainDriver.end()){
					next->setNextDriver(iLastDriver->second);
				}
				lastTrainDriver[lineId] = next;
			}
		}
	}
}
void TrainStationAgent::passengerLeaving(timeslice now)
{
	std::map<const Platform*, std::list<Passenger*>>::iterator it;
	for (it = aligtingPersons.begin(); it != aligtingPersons.end(); it++) {
		std::list<Passenger*>::iterator itPassenger = it->second.begin();
		while (itPassenger != it->second.end() && parentConflux) {
			messaging::MessageBus::PostMessage(parentConflux,
					PASSENGER_LEAVE_FRM_PLATFORM, messaging::MessageBus::MessagePtr(new PersonMessage((*itPassenger)->getParent())));
			itPassenger = it->second.erase(itPassenger);
		}
	}
}

void TrainStationAgent::updateWaitPersons()
{
	std::map<const Platform*, std::list<WaitTrainActivity*>>::iterator it;
	for(it=waitingPersons.begin(); it!=waitingPersons.end(); it++){
		std::list<WaitTrainActivity*>& persons = it->second;
		for(std::list<WaitTrainActivity*>::iterator i=persons.begin(); i!=persons.end(); i++){
			(*i)->Movement()->frame_tick();
		}
	}
}
Entity::UpdateStatus TrainStationAgent::frame_init(timeslice now)
{
	return UpdateStatus::Continue;
}
Entity::UpdateStatus TrainStationAgent::frame_tick(timeslice now)
{
	dispathPendingTrains(now);
	updateWaitPersons();
	ConfigManager::GetInstance().FullConfig().simStartTime();
	double sysGran = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	std::list<TrainDriver*>::iterator it=trainDriver.begin();
	while (it != trainDriver.end()) {
		double tickInSec = 0.0;
		do {
			callMovementFrameTick(now, *it);
			tickInSec += (*it)->getParams().secondsInTick;
			if ((*it)->getCurrentStatus() == TrainDriver::ARRIVAL_AT_PLATFORM) {
				const Platform* platform = (*it)->getNextPlatform();
				int alightingNum = (*it)->alightPassenger(aligtingPersons[platform]);
				int boardingNum = (*it)->boardPassenger(waitingPersons[platform], now);
				(*it)->calculateDwellTime(alightingNum+boardingNum);
				(*it)->setCurrentStatus(TrainDriver::WAITING_LEAVING);
			} else if ((*it)->getCurrentStatus() == TrainDriver::LEAVING_FROM_PLATFORM) {
				std::string lineId = (*it)->getTrainLine();
				lastUsage[lineId] = false;
				if((*it)->getParent()->isToBeRemoved()){
					removeAheadTrain(*it);
					(*it)->setCurrentStatus(TrainDriver::MOVE_TO_DEPOT);
					messaging::MessageBus::PostMessage(TrainController<Person_MT>::getInstance(),
							MSG_TRAIN_BACK_DEPOT, messaging::MessageBus::MessagePtr(new TrainMessage((*it)->getParent())));
				}
				it = trainDriver.erase(it);
				break;
			}
		} while (tickInSec < sysGran);
		it++;
	}
	passengerLeaving(now);
	return UpdateStatus::Continue;
}
void TrainStationAgent::frame_output(timeslice now)
{
	if (!isToBeRemoved())
	{
		std::list<TrainDriver*>::iterator it=trainDriver.begin();
		while (it != trainDriver.end()) {
			LogOut((*it)->Movement()->frame_tick_output());
			it++;
		}
	}
}

bool TrainStationAgent::isNonspatial()
{
	return false;
}
void TrainStationAgent::removeAheadTrain(TrainDriver* aheadDriver)
{
	if(!aheadDriver){
		return;
	}
	std::list<TrainDriver*>::iterator it=trainDriver.begin();
	while (it != trainDriver.end()) {
		if((*it)->getNextDriver()==aheadDriver){
			(*it)->setNextDriver(nullptr);
		}
		it++;
	}

	std::string lineId = aheadDriver->getTrainLine();
	std::map<std::string, TrainDriver*>::iterator iLastDriver = lastTrainDriver.find(lineId);
	if(iLastDriver!=lastTrainDriver.end()){
		if(iLastDriver->second==aheadDriver){
			lastTrainDriver[lineId]=nullptr;
		}
	}
}

Entity::UpdateStatus TrainStationAgent::callMovementFrameTick(timeslice now, TrainDriver* driver)
{
	if(driver){
		driver->make_frame_tick_params(now);
		driver->Movement()->frame_tick();
	}
	return UpdateStatus::Continue;
}
}
} /* namespace sim_mob */
