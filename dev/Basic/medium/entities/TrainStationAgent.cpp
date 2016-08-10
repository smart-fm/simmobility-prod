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
#include "behavioral/ServiceController.hpp"
using namespace sim_mob::event;

namespace
{
const double safeDistanceToAhead = 140;
}
namespace sim_mob {
namespace medium
{

TrainStationAgent::TrainStationAgent():station(nullptr),Agent(MutexStrategy::MtxStrat_Buffered, -1), parentConflux(nullptr),disruptionParam(nullptr)
{
	// TODO Auto-generated constructor stub

}

TrainStationAgent::~TrainStationAgent()
{
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

void TrainStationAgent::setLines()
{
	if(station)
	{
		std::map<std::string,Platform*> platformLineMap=station->getPlatforms();
		std::map<std::string,Platform*>::iterator it=platformLineMap.begin();
		TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
		while(it!=platformLineMap.end())
		{
			  Platform *platform=it->second;
			  std::string lineId=it->first;
			  if(trainController->IsFirstStation(lineId,platform))
			  {
				  IsStartStation[lineId]=true;
			  }
			  else
				  IsStartStation[lineId]=false;
			  it++;
		}
	}
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

	case  TRAIN_MOVE_AT_UTURN_PLATFORM:
	{
		std::list<TrainDriver*>::iterator it=(trainDriver.begin());
		std::string trainLine=(*it)->getTrainLine();
		const TrainDriverMessage& msg = MSG_CAST(TrainDriverMessage, message);
		it=find(trainDriver.begin(),trainDriver.end(),*it);
        if(it==trainDriver.end())
        {
        	msg.trainDriver->getParent()->currWorkerProvider = currWorkerProvider;
        	trainDriver.push_back(msg.trainDriver);
        }

		//msg.trainDriver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
		break;
	}
	case TRAIN_ARRIVAL_AT_STARTPOINT:
	{
		const TrainDriverMessage& msg = MSG_CAST(TrainDriverMessage, message);

		msg.trainDriver->setNextRequested(TrainDriver::REQUESTED_TO_PLATFROM);
		std::string lineId = msg.trainDriver->getTrainLine();

		if(pendingTrainDriver.find(lineId)==pendingTrainDriver.end())
		{
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
			iPending != pendingTrainDriver.end(); iPending++)
	{
		std::list<TrainDriver*>& pendingDrivers = (*iPending).second;
		std::string lineId = iPending->first;
		bool isTrainServiceTerminated = TrainController<sim_mob::medium::Person_MT>::getInstance()->IsServiceTerminated(lineId);
		if(isTrainServiceTerminated)
		{
			std::list<TrainDriver*>::iterator itr=pendingDrivers.begin();
			while(itr!=pendingDrivers.end())
			{
				(*itr)->setNextRequested(TrainDriver::REQUESTED_TO_DEPOT);
				messaging::MessageBus::PostMessage(TrainController<Person_MT>::getInstance(),
				MSG_TRAIN_BACK_DEPOT, messaging::MessageBus::MessagePtr(new TrainMessage((*itr)->getParent())));
				itr++;
			}
			pendingDrivers.clear();
			return;
		}
		if(!pendingDrivers.empty() && pendingDrivers.front()->getParent()->getStartTime()<=now.ms())
		{

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

			std::map<std::string, bool>::iterator itr = IsStartStation.find(lineId);
			TrainDriver* next = pendingDrivers.front();
			const TrainTrip *trainTrip=next->getTrainTrip();
			if(trainTrip->isUnScheduledTrain())
			{

            	   Platform *stationAgentPlatform=station->getPlatform(lineId);
            	   TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
            	   std::vector <Role<Person_MT>*> trainDriverVector=trainController->GetActiveTrainsForALine(lineId);
            	   std::vector<Role<Person_MT>*>::iterator it;

				   next->GetMovement()->teleportToPlatform(stationAgentPlatform->getPlatformNo());
            	   bool isTrainApproachingClose=false;
            	       for(it=trainDriverVector.begin();it!=trainDriverVector.end();it++)
            	       {
							TrainDriver *tDriver=dynamic_cast<TrainDriver*>(*(it));
							if(tDriver)
							{
								MovementFacet *moveFacet=tDriver->GetMovement();
								if(moveFacet)
								{
									TrainMovement* trainMovement=dynamic_cast<TrainMovement*>(moveFacet);
									   if(trainMovement)
									   {
										  Platform *platform=trainMovement->getNextPlatform();
										  if(platform)
										  {

                                               if(stationAgentPlatform==platform)
                                               {
                                            	   const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
                                            	   double safeDistance = config.trainController.safeDistance;
                                            	   if(trainMovement->getDistanceToNextPlatform(tDriver)-platform->getLength()-safeDistance-138<0)
                                            	   {
                                            		   isTrainApproachingClose=true;
                                            		   break;
                                            	   }
                                               }
										  }
									   }
								}

							}

            	       if(isTrainApproachingClose)
            	    	   continue;
            	       else
            	       {
            	    	   //advance the train route to that platform.
            	    	   next->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);

            	       }

			   }
			}
			if (!isUsed)
			{
				TrainDriver* next = pendingDrivers.front();
				TrainDriver* ahead = nullptr;
				TrainDriver *behindDriver=nullptr;
				bool success = false;
				std::map<std::string, TrainDriver*>::iterator iLastDriver;
				iLastDriver = lastTrainDriver.find(lineId);
				if (iLastDriver != lastTrainDriver.end())
				{
					ahead = iLastDriver->second;
					sim_mob::medium::TrainMovement* trainMover = dynamic_cast<sim_mob::medium::TrainMovement*>(next->Movement());
					double distanceToNextTrain = trainMover->getDistanceToNextTrain(ahead,false);
					if (distanceToNextTrain > safeDistanceToAhead)
					{
						success = true;
					}
				}

				else
				{

					TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
					std::vector <Role<Person_MT>*> trainDriverVector=trainController->GetActiveTrainsForALine(lineId);
					std::vector <Role<Person_MT>*>::iterator trainDriverItr=trainDriverVector.begin();
					double minDis=-1;
					success=true;

					double minDisBehindDriver=-1;
					while(trainDriverItr!=trainDriverVector.end())
					{
						TrainDriver *trainDriver =dynamic_cast<TrainDriver*>(*trainDriverItr);
						if(trainDriver&&trainDriver!=next)
						{
							TrainMovement *movement=trainDriver->GetMovement();
							double totalDisCoverdByOtherTrain=movement->getTotalCoveredDistance();
							if(totalDisCoverdByOtherTrain - (next->GetMovement()->getTotalCoveredDistance())<0)
							{
								if(minDisBehindDriver==-1||((next->GetMovement()->getTotalCoveredDistance())-totalDisCoverdByOtherTrain)<minDisBehindDriver)
								{
									behindDriver=trainDriver;
									minDisBehindDriver=((next->GetMovement()->getTotalCoveredDistance())-totalDisCoverdByOtherTrain);
								}
								trainDriverItr++;
								continue;
							}
							double differentDistance=totalDisCoverdByOtherTrain - (next->GetMovement()->getTotalCoveredDistance())-138-(movement->getSafeDistance());
							if(differentDistance<0)
							{
								success=false;
								ahead=trainDriver;
								break;
							}
							else
							{
								if(minDis==-1||differentDistance<minDis)
								{
									minDis=differentDistance;
									ahead=trainDriver;
								}
							}
						}
						trainDriverItr++;
					}
				}

				if (success || !ahead)
				{
					trainDriver.push_back(next);
					pendingDrivers.pop_front();
					lastUsage[lineId] = true;
					lastTrainDriver[lineId] = next;
					next->setNextDriver(ahead);
					if(behindDriver!=nullptr)
					{
						int id=behindDriver->getTrainId();
						int x=id;
						behindDriver->setNextDriver(next);
					}
					Role<Person_MT> *tDriver=dynamic_cast<Role<Person_MT>*>(next);
					TrainController<Person_MT>::getInstance()->AddToListOfActiveTrainsInLine(lineId,tDriver);
				}
			}
		}
	}
}

void TrainStationAgent::setLastDriver(std::string lineId,TrainDriver *driver)
{
	lastTrainDriver[lineId]=driver;
}
void TrainStationAgent::passengerLeaving(timeslice now)
{
	std::map<const Platform*, std::list<Passenger*>>::iterator it;
	for (it = leavingPersons.begin(); it != leavingPersons.end(); it++)
	{
		std::list<Passenger*>::iterator itPassenger = it->second.begin();
		while (itPassenger != it->second.end() && parentConflux)
		{
			messaging::MessageBus::PostMessage(parentConflux,
					PASSENGER_LEAVE_FRM_PLATFORM, messaging::MessageBus::MessagePtr(new PersonMessage((*itPassenger)->getParent())));
			itPassenger = it->second.erase(itPassenger);
		}
	}
}
void TrainStationAgent::triggerRerouting(const event::EventArgs& args, timeslice now)
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
	DailyTime current = DailyTime(ConfigManager::GetInstance().FullConfig().simStartTime().getValue()+now.ms());
	for(std::list<Person_MT*>::iterator it = colPersons.begin(); it!=colPersons.end();it++){
		messaging::MessageBus::SubscribeEvent(EVT_DISRUPTION_CHANGEROUTE, this, *it);
		messaging::MessageBus::PublishInstantaneousEvent(EVT_DISRUPTION_CHANGEROUTE, this,
				messaging::MessageBus::EventArgsPtr(new ReRouteEventArgs(stationName,current.getValue())));
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
	/*if(arePassengersreRouted == false)
	{
		if(station)
		{
			std::map<std::string, Platform*> linePlatformMap=station->getPlatforms();
			std::map<std::string, Platform*>::iterator it;
			std::map<std::string,std::vector<std::string>> platforms=TrainController<sim_mob::medium::Person_MT>::getInstance()->GetDisruptedPlatforms_ServiceController();
			for(it=linePlatformMap.begin();it!=linePlatformMap.end();it++)
			{
				std::vector<std::string> platformNames=platforms[it->first];
				std::vector<std::string>::iterator itr=platformNames.begin();
				if(itr!=platformNames.end())
				{
					Platform *disruptedPlt=dynamic_cast<Platform*>(it->second);
					if(disruptedPlt)
					{
						itr=std::find(platformNames.begin(),platformNames.end(),disruptedPlt->getPlatformNo());
						//trigger rerouting
						if(itr!=platformNames.end())
						triggerRerouting(DisruptionEventArgs(*disruptionParam),now);
						else
						{
							itr=platformNames.begin();
							Platform *platform=TrainController<sim_mob::medium::Person_MT>::getInstance()->getPrePlatform(it->first,*itr);
							if(disruptedPlt==platform)
							{
								triggerRerouting(DisruptionEventArgs(*disruptionParam),now);
							}
						}
					}
				}
			}
		}
	}*/
	double sysGran = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	std::list<TrainDriver*>::iterator it=trainDriver.begin();

 	it=trainDriver.begin();

	while (it != trainDriver.end())
	{
		double tickInSec = 0.0;
		do
		{
			callMovementFrameTick(now, *it);
			tickInSec += (*it)->getParams().secondsInTick;
			const Platform* platform = (*it)->getNextPlatform();
			if ((*it)->getNextRequested() == TrainDriver::REQUESTED_AT_PLATFORM)
			{

				DailyTime startTm = ConfigManager::GetInstance().FullConfig().simStartTime();
				DailyTime current(now.ms() + startTm.getValue());
				(*it)->setArrivalTime(current.getStrRepr());
				bool isDisruptedPlat = false;
				std::string trainLine=(*it)->getTrainLine();
				std::map<std::string,std::vector<std::string>> platformNames = TrainController<sim_mob::medium::Person_MT>::getInstance()->GetDisruptedPlatforms_ServiceController();
				std::vector<std::string> disruptedPlatformNames=platformNames[trainLine];
				std::vector<std::string>::iterator itr=std::find(disruptedPlatformNames.begin(),disruptedPlatformNames.end(),platform->getPlatformNo());
				if(itr!=disruptedPlatformNames.end())
				{
					(*it)->GetMovement()->SetDisruptedState(true);
					isDisruptedPlat=true;
				}


				if((*it)->getForceAlightFlag())
				{

					(*it)->LockUnlockRestrictPassengerEntitiesLock(true);
					(*it)->setForceAlightStatus(true); //will be unset by service controller
					int alightingNum = (*it)->AlightAllPassengers(leavingPersons[platform], now);
					(*it)->LockUnlockRestrictPassengerEntitiesLock(false);
					(*it)->calculateDwellTime(0,alightingNum,0,now);
					(*it)->setNextRequested(TrainDriver::REQUESTED_WAITING_LEAVING);
					(*it)->setForceAlightFlag(false);
					if(isDisruptedPlat)
					continue;

					else
					{
						//push the passengers to waiting persons list if this platform is not their actual alighting platform
						// so that they board the next train
						std::list<Passenger*> leavingPassengers=leavingPersons[platform];
						for (std::list<Passenger*>::iterator it=leavingPassengers.begin(); it != leavingPassengers.end(); )
						{
							Passenger *pr=dynamic_cast<Passenger*>(*it);
							const WayPoint endPoint=pr->getEndPoint();
							const Platform * alightingPlatform=endPoint.platform;
							if(alightingPlatform!=platform)
							{
								if(forceAlightedPersons.find(platform)==forceAlightedPersons.end())
								{
									forceAlightedPersons[platform]=std::list<Passenger*>();
								}
								forceAlightedPersons[platform].push_back(pr);
								leavingPersons[platform].erase(it);
							}
							else
								it++;
						}

					}
				}


				double dwellTimeInSecs = 0.0;

				if(disruptionParam.get()&&platform)
				{
					std::vector<std::string> platformNames = disruptionParam->platformNames;
					if(platformNames.size()>0&&disruptionParam->platformLineIds.size()>0)
					{
						Platform* platform = TrainController<Person_MT>::getPrePlatform(disruptionParam->platformLineIds.front(),platformNames.front());
						if(platform)
						{
							platformNames.push_back(platform->getPlatformNo());
						}
					}
					auto itPlat = std::find(platformNames.begin(), platformNames.end(), platform->getPlatformNo());
					if (itPlat != platformNames.end())
					{
						//(*it)->GetMovement()->SetDisruptedState(true);

						if(!(*it)->getForceAlightFlag())
						{
							(*it)->LockUnlockRestrictPassengerEntitiesLock(true);
							int alightingNum = (*it)->AlightAllPassengers(leavingPersons[platform], now);
							(*it)->LockUnlockRestrictPassengerEntitiesLock(false);
							(*it)->calculateDwellTime(0,alightingNum,0,now);
						}
						(*it)->setNextRequested(TrainDriver::REQUESTED_WAITING_LEAVING);
						isDisruptedPlat = true;
					}
				}

				else
				{
					std::string trainLine=(*it)->getTrainLine();
				    std::map<std::string,std::vector<std::string>> platformNames = TrainController<sim_mob::medium::Person_MT>::getInstance()->GetDisruptedPlatforms_ServiceController();
					std::vector<std::string> disruptedPlatformNames=platformNames[trainLine];
					std::vector<std::string>::iterator itr=std::find(disruptedPlatformNames.begin(),disruptedPlatformNames.end(),platform->getPlatformNo());
					if(itr!=disruptedPlatformNames.end())
					{
						(*it)->GetMovement()->SetDisruptedState(true);
						if(!(*it)->getForceAlightFlag())
						{
							(*it)->LockUnlockRestrictPassengerEntitiesLock(true);
							int alightingNum = (*it)->AlightAllPassengers(leavingPersons[platform], now);
							(*it)->LockUnlockRestrictPassengerEntitiesLock(false);
							(*it)->calculateDwellTime(0,alightingNum,0,now);
						}
						(*it)->setNextRequested(TrainDriver::REQUESTED_WAITING_LEAVING);
						isDisruptedPlat = true;
					}
				}


				if(!isDisruptedPlat)
				{
					if((*it)->GetTerminateStatus())
					{
						if(!(*it)->getForceAlightFlag())
						{
							(*it)->LockUnlockRestrictPassengerEntitiesLock(true);
							int alightingNum = (*it)->AlightAllPassengers(leavingPersons[platform],now);
							(*it)->LockUnlockRestrictPassengerEntitiesLock(false);
							(*it)->calculateDwellTime(0,alightingNum,0,now);
						}
						(*it)->setNextRequested(TrainDriver::REQUESTED_WAITING_LEAVING);

					}
					else
					{
						if(!(*it)->getForceAlightFlag())
						{
							(*it)->LockUnlockRestrictPassengerEntitiesLock(true);
							int alightingNum = (*it)->alightPassenger(leavingPersons[platform],now);
							int noOfPassengersInTrain=(*it)->getPassengers().size();
							int forcealightedboardingnum=(*it)->boardForceAlightedPassengersPassenger(forceAlightedPersons[platform],now);
							int boardingNum = (*it)->boardPassenger(waitingPersons[platform], now);
							(*it)->LockUnlockRestrictPassengerEntitiesLock(false);
							(*it)->calculateDwellTime(boardingNum+forcealightedboardingnum,alightingNum,noOfPassengersInTrain,now);
						}
						(*it)->setNextRequested(TrainDriver::REQUESTED_WAITING_LEAVING);
					}

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
				it--;
				break;
			}

			else if((*it)->getNextRequested() == TrainDriver::REQUESTED_TAKE_UTURN)
			{
				/*std::string lineId = (*it)->getTrainLine();
			    lastUsage[lineId] = false;
				it = trainDriver.erase(it);*/
			   break;
			}

			/*else if((*it)->getNextRequested() == TrainDriver::REQUESTED_TO_DEPOT)
			{
				std::string lineId = (*it)->getTrainLine();
				lastUsage[lineId] = false;
				removeAheadTrain(*it);
				messaging::MessageBus::PostMessage(TrainController<Person_MT>::getInstance(),
				MSG_TRAIN_BACK_DEPOT, messaging::MessageBus::MessagePtr(new TrainMessage((*it)->getParent())));
				it = trainDriver.erase(it);
				break;
			}*/

		}while (tickInSec < sysGran);
		it++;
	}
	passengerLeaving(now);
	return UpdateStatus::Continue;
}

void TrainStationAgent::performDisruption(timeslice now)
{
	if(!disruptionParam.get())
	{
		passengerLeaving(now);
	}
	else
	{
		DailyTime duration = disruptionParam->duration;
		unsigned int baseGran = ConfigManager::GetInstance().FullConfig().baseGranMS();
		disruptionParam->duration = DailyTime(duration.offsetMS_From(DailyTime(baseGran)));
		if(duration.getValue()>baseGran)
		{
			std::vector<std::string>::const_iterator it;
			std::vector<std::string> platformNames = disruptionParam->platformNames;
			if(platformNames.size()>0&&disruptionParam->platformLineIds.size()>0)
			{
				Platform* platform = TrainController<Person_MT>::getPrePlatform(disruptionParam->platformLineIds.front(),platformNames.front());
				if(platform)
				{
					platformNames.push_back(platform->getPlatformNo());
				}
			}

			for(it=platformNames.begin(); it!=platformNames.end(); it++)
			{
				if(TrainController<Person_MT>::checkPlatformIsExisted(this, *it))
				{
					triggerRerouting(DisruptionEventArgs(*disruptionParam),now);
					arePassengersreRouted =true;
					break;
				}
			}
			std::list<Person_MT*> colPersons;
			for(std::list<TrainDriver*>::iterator it=trainDriver.begin(); it!=trainDriver.end(); it++)
			{
				colPersons.push_back((*it)->getParent());
			}
			for(std::list<Person_MT*>::iterator it = colPersons.begin(); it!=colPersons.end();it++)
			{
				messaging::MessageBus::SubscribeEvent(EVT_DISRUPTION_STATION, this, *it);
				messaging::MessageBus::PublishInstantaneousEvent(EVT_DISRUPTION_STATION, this,
						messaging::MessageBus::EventArgsPtr(new DisruptionEventArgs(*disruptionParam)));
				messaging::MessageBus::UnSubscribeEvent(EVT_DISRUPTION_STATION, this, *it);
			}
		}
		else
		{
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
	if(!aheadDriver)
	{
		return;
	}
	std::string lineId = aheadDriver->getTrainLine();
	int trainId= aheadDriver->getTrainId();
	std::list<TrainDriver*>::iterator it=trainDriver.begin();
	while (it != trainDriver.end())
	{
		if((*it)->getNextDriver()==aheadDriver)
		{
			(*it)->setNextDriver(nullptr);
			//Role<Person_MT> *tDriver=dynamic_cast<Role<Person_MT>*>(aheadDriver);

		}
		it++;
	}


	std::map<std::string, TrainDriver*>::iterator iLastDriver = lastTrainDriver.find(lineId);
	if(iLastDriver!=lastTrainDriver.end())
	{
		if(iLastDriver->second==aheadDriver)
		{
			lastTrainDriver[lineId]=nullptr;

		}
	}

	ServiceController::getInstance()->removeTrainIdAndTrainDriverInMap(trainId,lineId,aheadDriver);
	TrainController<Person_MT>::getInstance()->RemoveFromListOfActiveTrainsInLine(lineId,aheadDriver);
}

Entity::UpdateStatus TrainStationAgent::callMovementFrameTick(timeslice now, TrainDriver* driver)
{
	if(driver)
	{
		if(driver->getTrainId()==10&&driver->getTripId()==1016)
		{
			bool dr=true;
		}
		driver->make_frame_tick_params(now);
		driver->Movement()->frame_tick();
	}
	return UpdateStatus::Continue;
}
}
} /* namespace sim_mob */
