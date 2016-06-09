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
#include "entities/roles/driver/TrainDriverFacets.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "message/MT_Message.hpp"
#include "message/MessageBus.hpp"
#include "message/MessageHandler.hpp"
#include "event/SystemEvents.hpp"
#include "event/args/ReRouteEventArgs.hpp"
using namespace sim_mob::event;
namespace {
const double safeDistanceToAhead = 1000.0;
}
namespace sim_mob {
namespace medium
{

TrainStationAgent::TrainStationAgent():station(nullptr),Agent(MutexStrategy::MtxStrat_Buffered, -1), parentConflux(nullptr),disruptionParam(nullptr) {
	// TODO Auto-generated constructor stub

}

TrainStationAgent::~TrainStationAgent() {
	// TODO Auto-generated destructor stub
}

void TrainStationAgent::setStationName(const std::string& name)
{
	stationName = name;
}
void TrainStationAgent::setStation(const Station* station)
{
	this->station = station;
}
void TrainStationAgent::setConflux(Conflux* conflux)
{
	parentConflux = conflux;
}

void TrainStationAgent::onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args)
{
	switch(eventId)
	{
	case event::EVT_CORE_MRT_DISRUPTION:
	{
		const event::DisruptionEventArgs& exArgs = MSG_CAST(event::DisruptionEventArgs, args);
		const DisruptionParams& disruption = exArgs.getDisruption();
		disruptionParam.reset(new DisruptionParams(disruption));
		break;
	}
	}
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
		msg.trainDriver->setNextRequested(TrainDriver::REQUESTED_TO_PLATFROM);
		break;
	}
	case TRAIN_ARRIVAL_AT_STARTPOINT:
	{
		const TrainDriverMessage& msg = MSG_CAST(TrainDriverMessage, message);
		msg.trainDriver->setNextRequested(TrainDriver::REQUESTED_TO_PLATFROM);
		std::string lineId = msg.trainDriver->getTrainLine();
		if(pendingTrainDriver.find(lineId)==pendingTrainDriver.end()){
			pendingTrainDriver[lineId] = std::list<TrainDriver*>();
		}
		if(pendingTrainDriver[lineId].size()>1){
			messaging::MessageBus::PostMessage(TrainController<Person_MT>::getInstance(),
										MSG_TRAIN_BACK_DEPOT, messaging::MessageBus::MessagePtr(new TrainMessage(msg.trainDriver->getParent())));
		} else {
			pendingTrainDriver[lineId].push_back(msg.trainDriver);
		}
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
			iPending != pendingTrainDriver.end(); iPending++)
	{
		std::list<TrainDriver*>& pendingDrivers = (*iPending).second;
		std::string lineId = iPending->first;
		if(!pendingDrivers.empty() && pendingDrivers.front()->getParent()->getStartTime()<=now.ms()){
			bool isUsed = false;
			std::map<std::string, bool>::iterator iUsed = lastUsage.find(lineId);
			if(iUsed!=lastUsage.end())
			{
				isUsed = iUsed->second;
			}
			else
			{
				lastUsage[lineId] = false;
			}
			if (!isUsed)
			{
				TrainDriver* next = pendingDrivers.front();
				TrainDriver* ahead = nullptr;
				bool success = false;
				std::map<std::string, TrainDriver*>::iterator iLastDriver;
				iLastDriver = lastTrainDriver.find(lineId);
				if (iLastDriver != lastTrainDriver.end())
				{
					ahead = iLastDriver->second;
					sim_mob::medium::TrainMovement* trainMover = dynamic_cast<sim_mob::medium::TrainMovement*>(next->Movement());
					double distanceToNextTrain = trainMover->getDistanceToNextTrain(ahead);
					if (distanceToNextTrain > safeDistanceToAhead)
					{
						success = true;
					}
				}
				if (success || !ahead)
				{
					trainDriver.push_back(next);
					pendingDrivers.pop_front();
					lastUsage[lineId] = true;
					lastTrainDriver[lineId] = next;
					next->setNextDriver(ahead);
					Role<Person_MT> *tDriver=dynamic_cast<Role<Person_MT>*>(next);
					TrainController<Person_MT>::getInstance()->AddToListOfActiveTrainsInLine(lineId,tDriver);

				}
			}
		}
	}
}
void TrainStationAgent::passengerLeaving(timeslice now)
{
	std::map<const Platform*, std::list<Passenger*>>::iterator it;
	for (it = leavingPersons.begin(); it != leavingPersons.end(); it++) {
		std::list<Passenger*>::iterator itPassenger = it->second.begin();
		while (itPassenger != it->second.end() && parentConflux) {
			messaging::MessageBus::PostMessage(parentConflux,
					PASSENGER_LEAVE_FRM_PLATFORM, messaging::MessageBus::MessagePtr(new PersonMessage((*itPassenger)->getParent())));
			itPassenger = it->second.erase(itPassenger);
		}
	}
}
void TrainStationAgent::triggerRerouting(const event::EventArgs& args)
{
	std::list<Person_MT*> colPersons;
	std::map<const Platform*, std::list<WaitTrainActivity*>>::iterator itWaiting;
	for(itWaiting=waitingPersons.begin(); itWaiting!=waitingPersons.end(); itWaiting++){
		std::list<WaitTrainActivity*>& persons = itWaiting->second;
		for(std::list<WaitTrainActivity*>::iterator i=persons.begin(); i!=persons.end(); i++){
			colPersons.push_back((*i)->getParent());
		}
		persons.clear();
	}
	std::map<const Platform*, std::list<Passenger*>>::iterator itPassenger;
	for (itPassenger = leavingPersons.begin(); itPassenger != leavingPersons.end(); itPassenger++) {
		std::list<Passenger*>& persons = itPassenger->second;;
		for(std::list<Passenger*>::iterator i=persons.begin(); i!=persons.end(); i++){
			colPersons.push_back((*i)->getParent());
		}
		persons.clear();
	}
	for(std::list<Person_MT*>::iterator it = colPersons.begin(); it!=colPersons.end();it++){
		messaging::MessageBus::SubscribeEvent(EVT_DISRUPTION_CHANGEROUTE, this, *it);
		messaging::MessageBus::PublishInstantaneousEvent(EVT_DISRUPTION_CHANGEROUTE, this,
				messaging::MessageBus::EventArgsPtr(new ChangeRouteEventArgs(stationName,0)));
		messaging::MessageBus::UnSubscribeEvent(EVT_DISRUPTION_CHANGEROUTE, this, *it);
		messaging::MessageBus::PostMessage(parentConflux,
							PASSENGER_LEAVE_FRM_PLATFORM, messaging::MessageBus::MessagePtr(new PersonMessage(*it)));
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
	messaging::MessageBus::SubscribeEvent(event::EVT_CORE_MRT_DISRUPTION, this);
	return UpdateStatus::Continue;
}
Entity::UpdateStatus TrainStationAgent::frame_tick(timeslice now)
{
	dispathPendingTrains(now);
	updateWaitPersons();
	performDisruption(now);
	ConfigManager::GetInstance().FullConfig().simStartTime();
	double sysGran = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	std::list<TrainDriver*>::iterator it=trainDriver.begin();

	while (it != trainDriver.end())
	{
		double tickInSec = 0.0;
		do {
			callMovementFrameTick(now, *it);
			tickInSec += (*it)->getParams().secondsInTick;
			if ((*it)->getNextRequested() == TrainDriver::REQUESTED_AT_PLATFORM)
			{

				bool isDisruptedPlat = false;
				const Platform* platform = (*it)->getNextPlatform();
				if(disruptionParam.get()&&platform){
					std::vector<std::string> platformNames = disruptionParam->platformNames;
					if(platformNames.size()>0&&disruptionParam->platformLineIds.size()>0){
						Platform* platform = TrainController<Person_MT>::getPrePlatform(disruptionParam->platformLineIds.front(),platformNames.front());
						if(platform){
							platformNames.push_back(platform->getPlatformNo());
						}
					}
					auto itPlat = std::find(platformNames.begin(), platformNames.end(), platform->getPlatformNo());
					if (itPlat != platformNames.end()) {
						int alightingNum = (*it)->AlightAllPassengers(leavingPersons[platform], now);
						(*it)->calculateDwellTime(0,alightingNum);
						(*it)->setNextRequested(TrainDriver::REQUESTED_WAITING_LEAVING);
						isDisruptedPlat = true;
					}
				}
				if(!isDisruptedPlat){
					int alightingNum = (*it)->alightPassenger(leavingPersons[platform],now);
					int boardingNum = (*it)->boardPassenger(waitingPersons[platform], now);
					(*it)->calculateDwellTime(boardingNum,alightingNum);
					(*it)->setNextRequested(TrainDriver::REQUESTED_WAITING_LEAVING);
				}

			}

			else if ((*it)->getNextRequested() == TrainDriver::REQUESTED_LEAVING_PLATFORM)
			{
				std::string lineId = (*it)->getTrainLine();
				lastUsage[lineId] = false;
				if((*it)->getParent()->isToBeRemoved())
				{
					removeAheadTrain(*it);
					(*it)->setNextRequested(TrainDriver::REQUESTED_TO_DEPOT);
					messaging::MessageBus::PostMessage(TrainController<Person_MT>::getInstance(),
							MSG_TRAIN_BACK_DEPOT, messaging::MessageBus::MessagePtr(new TrainMessage((*it)->getParent())));
				}
				it = trainDriver.erase(it);
				break;
			}
		}while (tickInSec < sysGran);
		it++;
	}
	passengerLeaving(now);
	return UpdateStatus::Continue;
}

void TrainStationAgent::performDisruption(timeslice now)
{
	if(!disruptionParam.get()){
		passengerLeaving(now);
	} else {
		DailyTime duration = disruptionParam->duration;
		unsigned int baseGran = ConfigManager::GetInstance().FullConfig().baseGranMS();
		disruptionParam->duration = DailyTime(duration.offsetMS_From(DailyTime(baseGran)));
		if(duration.getValue()>baseGran){
			std::vector<std::string>::const_iterator it;
			std::vector<std::string> platformNames = disruptionParam->platformNames;
			if(platformNames.size()>0&&disruptionParam->platformLineIds.size()>0){
				Platform* platform = TrainController<Person_MT>::getPrePlatform(disruptionParam->platformLineIds.front(),platformNames.front());
				if(platform){
					platformNames.push_back(platform->getPlatformNo());
				}
			}
			for(it=platformNames.begin(); it!=platformNames.end(); it++){
				if(TrainController<Person_MT>::checkPlatformIsExisted(this, *it)){
					triggerRerouting(DisruptionEventArgs(*disruptionParam));
					break;
				}
			}
			std::list<Person_MT*> colPersons;
			for(std::list<TrainDriver*>::iterator it=trainDriver.begin(); it!=trainDriver.end(); it++){
				colPersons.push_back((*it)->getParent());
			}
			for(std::list<Person_MT*>::iterator it = colPersons.begin(); it!=colPersons.end();it++){
				messaging::MessageBus::SubscribeEvent(EVT_DISRUPTION_STATION, this, *it);
				messaging::MessageBus::PublishInstantaneousEvent(EVT_DISRUPTION_STATION, this,
						messaging::MessageBus::EventArgsPtr(new DisruptionEventArgs(*disruptionParam)));
				messaging::MessageBus::UnSubscribeEvent(EVT_DISRUPTION_STATION, this, *it);
			}
		} else {
			disruptionParam.reset();
		}
	}
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
	std::string lineId = aheadDriver->getTrainLine();
	std::list<TrainDriver*>::iterator it=trainDriver.begin();
	while (it != trainDriver.end()) {
		if((*it)->getNextDriver()==aheadDriver){
			(*it)->setNextDriver(nullptr);
			Role<Person_MT> *tDriver=dynamic_cast<Role<Person_MT>*>(aheadDriver);
			TrainController<Person_MT>::getInstance()->RemoveFromListOfActiveTrainsInLine(lineId,aheadDriver);
		}
		it++;
	}


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
