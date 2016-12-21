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
	const double distanceBehingToMakeUpToSafeDistance = 200;
}
namespace sim_mob
{
namespace medium
{

	boost::mutex TrainStationAgent::insertTrainOrUturnlock;
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
			TrainController<Person_MT> *trainController=TrainController<Person_MT>::getInstance();
			while(it!=platformLineMap.end())
			{
				Platform *platform=it->second;
				std::string lineId=it->first;
				if(trainController->isFirstStation(lineId,platform))
				{
					IsStartStation[lineId]=true;
				}
				else
				{
					IsStartStation[lineId]=false;
				}
				it++;
			}
		}
	}

	std::list<TrainDriver*>& TrainStationAgent::getTrains()
	{
		return trainDriver;
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
				std::list<TrainDriver*> &trains=this->getTrains();
				if(std::find(trains.begin(),trains.end(),msg.trainDriver)==trains.end())
				{
					trainDriver.push_back(msg.trainDriver);
				}
				msg.trainDriver->setNextRequested(TrainDriver::REQUESTED_TO_PLATFROM);
				break;
			}

			case  TRAIN_MOVE_AT_UTURN_PLATFORM:
			{
				const TrainDriverMessage& msg = MSG_CAST(TrainDriverMessage, message);
				TrainDriver *driver=msg.trainDriver;
				if(driver->getUTurnFlag())
				{
					driver->setNextRequested(TrainDriver::REQUESTED_TAKE_UTURN);
					TrainMovement *movement=driver->getMovement();
					if(movement)
					{
						insertTrainOrUturnlock.lock();
						movement->prepareForUTurn();
						insertTrainOrUturnlock.unlock();
					}
				}
				else
				{
					driver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
				}
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
				if(waitingPerson)
				{

					TrainController<sim_mob::medium::Person_MT> *trainController = TrainController<sim_mob::medium::Person_MT>::getInstance();
					double walkTime = 0.0;
					waitingPerson->setWalkTimeToPlatform(walkTime);
					const Platform* platform = waitingPerson->getStartPlatform();
					if(platform)
					{
						walkingPersons[platform].push_back(waitingPerson);
					}
					else
					{
						throw std::runtime_error("the waiting person don't know starting platform.");
					}

				}
				else
				{
					throw std::runtime_error("the person arriving at platform is not waiting role.");
				}
				break;
			}

			case INSERT_UNSCHEDULED_TRAIN:
			{
				checkAndInsertUnscheduledTrains();
			}
		}
	}

	void TrainStationAgent::checkAndInsertUnscheduledTrains()
	{
		std::map<std::string, std::list<TrainDriver*>>::iterator iPending;
		for (iPending = pendingTrainDriver.begin();
					iPending != pendingTrainDriver.end(); iPending++)
		{
			std::list<TrainDriver*>& pendingDrivers = (*iPending).second;
			std::string lineId = iPending->first;
			std::vector<std::string>::iterator itr = find(unscheduledTrainLines.begin(),unscheduledTrainLines.end(),lineId);
			if(itr!=unscheduledTrainLines.end())
			{
				const Platform *stationAgentPlatform=station->getPlatform(lineId);
				TrainController<sim_mob::medium::Person_MT> *trainController = TrainController<sim_mob::medium::Person_MT>::getInstance();
				insertTrainOrUturnlock.lock();
				std::vector <Role<Person_MT>*> trainDriverVector = trainController->getActiveTrainsForALine(lineId);
				std::vector<Role<Person_MT>*>::iterator it;
				TrainDriver* next = pendingDrivers.front();
				next->getMovement()->teleportToPlatform(stationAgentPlatform->getPlatformNo());
				bool isTrainApproachingClose=false;
				for(it = trainDriverVector.begin();it != trainDriverVector.end();it++)
				{
					TrainDriver *tDriver = dynamic_cast<TrainDriver*>(*(it));
					if(tDriver)
					{
						MovementFacet *moveFacet = tDriver->getMovement();
						if(moveFacet)
						{
							TrainMovement* trainMovement = dynamic_cast<TrainMovement*>(moveFacet);
							if(trainMovement)
							{
								Platform *platform = trainMovement->getNextPlatform();
								if(platform)
								{
									if(stationAgentPlatform == platform)
									{
										const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
										double safeDistance = config.trainController.safeDistance;
										//checking safe distance from the ned of platform
										double distanceToTrainAhead = trainMovement->getDistanceToNextPlatform(tDriver)-platform->getLength()-safeDistance;
										const std::map<const std::string,TrainProperties> &trainLinePropertiesMap = config.trainController.trainLinePropertiesMap;
										const TrainProperties &trainProperties = trainLinePropertiesMap.find(lineId)->second;										
										double minDisBehindTrain = trainProperties.minDistanceTrainBehindForUnscheduledTrain;
										if(distanceToTrainAhead-distanceBehingToMakeUpToSafeDistance-minDisBehindTrain<0)
										{
											isTrainApproachingClose = true;
											break;
										}
										else
										{
											//also check for safe headway ...
											double currentSpeed = trainMovement->getCurrentSpeed();
											double safeHeadWay = trainMovement->getSafeHeadWay();
											if((distanceToTrainAhead-safeDistance)/currentSpeed<safeHeadWay)
											{
												isTrainApproachingClose=true;
												break;
											}
										}
									}
								}
							}
						}
					}

					if(isTrainApproachingClose)
					{
						continue;
					}
				}
				std::map<std::string, bool>::iterator iUsed = lastUsage.find(lineId);
				bool isUsed=false;
				if(iUsed != lastUsage.end())
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
					TrainDriver *behindDriver = nullptr;
					bool success = false;
					std::map<std::string, TrainDriver*>::iterator iLastDriver;
					iLastDriver = lastTrainDriver.find(lineId);
					TrainController<sim_mob::medium::Person_MT> *trainController = TrainController<sim_mob::medium::Person_MT>::getInstance();
					std::vector <Role<Person_MT>*> trainDriverVector = trainController->getActiveTrainsForALine(lineId);
					std::vector <Role<Person_MT>*>::iterator trainDriverItr = trainDriverVector.begin();
					double minDis=-1;
					success=true;
					double minDisBehindDriver = -1;
					while(trainDriverItr != trainDriverVector.end())
					{
						TrainDriver *trainDriver = dynamic_cast<TrainDriver*>(*trainDriverItr);
						if(trainDriver&&trainDriver != next)
						{
							TrainMovement *movement = trainDriver->getMovement();
							double totalDisCoverdByOtherTrain = movement->getTotalCoveredDistance();
							if(totalDisCoverdByOtherTrain - (next->getMovement()->getTotalCoveredDistance())<0)
							{
								if(minDisBehindDriver == -1||((next->getMovement()->getTotalCoveredDistance())-totalDisCoverdByOtherTrain)<minDisBehindDriver)
								{
									behindDriver = trainDriver;
									minDisBehindDriver = ((next->getMovement()->getTotalCoveredDistance())-totalDisCoverdByOtherTrain);
								}
								trainDriverItr++;
								continue;
							}
							double differentDistance = totalDisCoverdByOtherTrain - (next->getMovement()->getTotalCoveredDistance())-distanceBehingToMakeUpToSafeDistance-(movement->getSafeDistance());
							if(differentDistance<0)
							{
								success=false;
								ahead=trainDriver;
								break;
							}
							else
							{
								if(minDis == -1 || differentDistance<minDis)
								{
									minDis=differentDistance;
									ahead=trainDriver;
								}
							}
						}
						trainDriverItr++;
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
							behindDriver->setNextDriver(next);
						}
						next->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
						Role<Person_MT> *tDriver=dynamic_cast<Role<Person_MT>*>(next);
						TrainController<Person_MT>::getInstance()->addToListOfActiveTrainsInLine(lineId,tDriver);
					}
				}
				insertTrainOrUturnlock.unlock();
		  }


	 }
		unscheduledTrainLines.clear();
	}

	std::map<std::string, TrainDriver*> TrainStationAgent::getLastDriver()
	{
		return lastTrainDriver;
	}

	void TrainStationAgent::dispathPendingTrains(timeslice now)
	{

		bool unscheduledToBeDispatched=false,trainAheadTooClose=false;
		std::map<std::string, std::list<TrainDriver*>>::iterator iPending;
		for (iPending = pendingTrainDriver.begin();
				iPending != pendingTrainDriver.end(); iPending++)
		{
			std::list<TrainDriver*>& pendingDrivers = (*iPending).second;
			std::string lineId = iPending->first;
			bool isTrainServiceTerminated = TrainController<sim_mob::medium::Person_MT>::getInstance()->isServiceTerminated(lineId);
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
				continue;
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
					unscheduledTrainLines.push_back(lineId);
				}

				else
				{
					if (!isUsed)
					{
						insertTrainOrUturnlock.lock();
						TrainDriver* next = pendingDrivers.front();
						TrainDriver* ahead = nullptr;
						TrainDriver *behindDriver=nullptr;
						bool success = false;
						std::map<std::string, TrainDriver*>::iterator iLastDriver;
						iLastDriver = lastTrainDriver.find(lineId);
						//check for nearest train ahead
						TrainController<Person_MT> *trainController=TrainController<Person_MT>::getInstance();
						std::vector <Role<Person_MT>*> trainDriverVector=trainController->getActiveTrainsForALine(lineId);
						std::vector <Role<Person_MT>*>::iterator trainDriverItr=trainDriverVector.begin();
						double minDis=-1;
						success=true;
						double minDisBehindDriver=-1;
						while(trainDriverItr != trainDriverVector.end())
						{
							TrainDriver *trainDriver =dynamic_cast<TrainDriver*>(*trainDriverItr);
							if(trainDriver&&trainDriver!=next)
							{
								TrainMovement *movement = trainDriver->getMovement();
								double totalDisCoverdByOtherTrain = movement->getTotalCoveredDistance();
								if(totalDisCoverdByOtherTrain - (next->getMovement()->getTotalCoveredDistance())<0)
								{
									continue;
								}
								else
								{
									double differentDistance = totalDisCoverdByOtherTrain - (next->getMovement()->getTotalCoveredDistance())-(movement->getSafeDistance());
									if(differentDistance<0)
									{
										success=false;
										ahead=trainDriver;
										break;
									}
									else
									{
										if(minDis == -1||differentDistance<minDis)
										{
											minDis = differentDistance;
											ahead = trainDriver;
										}
									}
								}
							}
							trainDriverItr++;
						}

						//end
						if (success || !ahead)
						{
							trainDriver.push_back(next);
							pendingDrivers.pop_front();
							lastUsage[lineId] = true;
							lastTrainDriver[lineId] = next;
							next->setNextDriver(ahead);
							if(behindDriver!=nullptr)
							{
								behindDriver->setNextDriver(next);
							}
							Role<Person_MT> *tDriver = dynamic_cast<Role<Person_MT>*>(next);
							TrainController<Person_MT>::getInstance()->addToListOfActiveTrainsInLine(lineId,tDriver);
						}
						insertTrainOrUturnlock.unlock();
					}
				}
			}
		}
		messaging::MessageBus::PostMessage(this,INSERT_UNSCHEDULED_TRAIN,messaging::MessageBus::MessagePtr(new TrainDriverMessage(nullptr)));
	}

	void TrainStationAgent::setLastDriver(std::string lineId,TrainDriver *driver)
	{
		lastTrainDriver[lineId] = driver;
	}

	void TrainStationAgent::addTrainDriverInToStationAgent(TrainDriver * driver)
	{
		trainDriver.push_back(driver);
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
		for(itWaiting=waitingPersons.begin(); itWaiting != waitingPersons.end(); itWaiting++)
		{
			std::list<WaitTrainActivity*>& persons = itWaiting->second;
			for(std::list<WaitTrainActivity*>::iterator i = persons.begin(); i!=persons.end(); i++)
			{
				colPersons.push_back((*i)->getParent());
			}
			persons.clear();
		}
		std::map<const Platform*, std::list<Passenger*>>::iterator itPassenger;
		for (itPassenger = leavingPersons.begin(); itPassenger != leavingPersons.end(); itPassenger++)
		{
			std::list<Passenger*>& persons = itPassenger->second;;
			for(std::list<Passenger*>::iterator i = persons.begin(); i != persons.end(); i++)
			{
				colPersons.push_back((*i)->getParent());
			}
			persons.clear();
		}
		DailyTime current = DailyTime(ConfigManager::GetInstance().FullConfig().simStartTime().getValue()+now.ms());
		for(std::list<Person_MT*>::iterator it = colPersons.begin(); it!=colPersons.end();it++)
		{
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
		for(it = waitingPersons.begin(); it != waitingPersons.end(); it++)
		{
			std::list<WaitTrainActivity*>& persons = it->second;
			for(std::list<WaitTrainActivity*>::iterator i=persons.begin(); i!=persons.end(); i++)
			{
				MovementFacet *facet = (*i)->Movement();
				facet->frame_tick();
			}
		}
		for(it = walkingPersons.begin(); it != walkingPersons.end(); it++)
		{
			std::list<WaitTrainActivity*>& persons = it->second;
			for(std::list<WaitTrainActivity*>::iterator i = persons.begin(); i != persons.end(); i++)
			{
				MovementFacet *facet = (*i)->Movement();
				facet->frame_tick();
				double walkTime = (*i)->getWalkTimeToPlatform();
				if(walkTime < 5)
				{
					const Platform *platform = (it)->first;
					if(platform)
					{
						waitingPersons[platform].push_back(*i);
						i = persons.erase(i);
					}
				}
				else
				{
					(*i)->reduceWalkingTime();
					double walkTime = (*i)->getWalkTimeToPlatform();
					if(walkTime < 5)
					{
						const Platform *platform = (it)->first;
						if(platform)
						{
							waitingPersons[platform].push_back(*i);
							i = persons.erase(i);
						}
					}
				}
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
	std::map<const Platform*, std::list<Passenger*>>::iterator forceAlightedPersonsItr= forceAlightedPersons.begin();
	while(forceAlightedPersonsItr != forceAlightedPersons.end())
	{
		const Platform *forceAlightPlatform = (forceAlightedPersonsItr)->first;
		std::list<Person_MT*>::iterator personsFAIt = personsForcedAlighted.begin();
		std::list<Passenger*> passengersForceAlighted = (forceAlightedPersonsItr)->second;
		std::list<Passenger*>::iterator passengerItr = passengersForceAlighted.begin();
		while(passengerItr != passengersForceAlighted.end())
		{

			if((*passengerItr))
			{

				(*passengerItr)->Movement()->frame_tick();
			}
			passengerItr++;
		}
		forceAlightedPersonsItr++;
	}
	double sysGran = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	std::list<TrainDriver*>::iterator it = trainDriver.begin();

	it = trainDriver.begin();
	while (it != trainDriver.end())
	{
		const Platform* platform = (*it)->getNextPlatform();
		double tickInSec = 0.0;
		do
		{

			if((*it)->getNextRequested() == TrainDriver::REQUESTED_WAITING_LEAVING && (*it)->getForceAlightFlag() == true)
			{
				(*it)->alightAllPassengers(leavingPersons[platform], now);
				int initialnumberofpassengers=(*it)->getInitialNumberOfPassengers();
				(*it)->calculateDwellTime(0,initialnumberofpassengers,initialnumberofpassengers,now,true);
				(*it)->setForceAlightFlag(false);
				if((*it)->getMovement()->getDisruptedState()== true)
				{
					(*it)->setHasForceAlightedInDisruption(true);
				}
			}
			callMovementFrameTick(now, *it);
			tickInSec += (*it)->getParams().secondsInTick;
			if((*it)->getIsToBeRemoved() && ((*it)->getMovement()->getNextPlatform() == nullptr || !boost::iequals((*it)->getMovement()->getNextPlatform()->getStationNo(),stationName)))
			{
				(*it)->setIsToBeRemoved(false);
				it = trainDriver.erase(it);
				it--;
			}
			else if ((*it)->getNextRequested() == TrainDriver::REQUESTED_AT_PLATFORM)
			{

				DailyTime startTm = ConfigManager::GetInstance().FullConfig().simStartTime();
				DailyTime current(now.ms() + startTm.getValue());
				(*it)->setArrivalTime(current.getStrRepr());
				bool isDisruptedState = false;
				std::string trainLine = (*it)->getTrainLine();
				std::map<std::string,std::vector<std::string>> platformNames = TrainController<sim_mob::medium::Person_MT>::getInstance()->getDisruptedPlatforms_ServiceController();
				std::vector<std::string> disruptedPlatformNames = platformNames[trainLine];
				std::vector<std::string>::iterator itr = std::find(disruptedPlatformNames.begin(),disruptedPlatformNames.end(),platform->getPlatformNo());
				TrainPlatformMover &platformMover = (*it)->getMovement()->getTrainPlatformMover();
				if(itr != disruptedPlatformNames.end())
				{
					(*it)->getMovement()->setDisruptedState(true);
					isDisruptedState = true;
				}


				if((*it)->getForceAlightFlag())
				{

					(*it)->lockUnlockRestrictPassengerEntitiesLock(true);
					(*it)->setForceAlightStatus(true); //will be unset by service controller
					int alightingNum = (*it)->alightAllPassengers(leavingPersons[platform], now);
					(*it)->lockUnlockRestrictPassengerEntitiesLock(false);
					(*it)->calculateDwellTime(0,alightingNum,0,now,false);
					(*it)->setNextRequested(TrainDriver::REQUESTED_WAITING_LEAVING);
					(*it)->setForceAlightFlag(false);

					if((*it)->getMovement()->getDisruptedState() || (*it)->getUTurnFlag()==true)
					{
						(*it)->setHasForceAlightedInDisruption(true);
						continue;
					}
				}


				double dwellTimeInSecs = 0.0;

				if(disruptionParam.get()&&platform)
				{
					std::vector<std::string> platformNames = disruptionParam->platformNames;
					if(platformNames.size()>0 && disruptionParam->platformLineIds.size()>0)
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
						if(!(*it)->getForceAlightStatus())
						{
							(*it)->lockUnlockRestrictPassengerEntitiesLock(true);
							int alightingNum = (*it)->alightAllPassengers(leavingPersons[platform], now);
							(*it)->lockUnlockRestrictPassengerEntitiesLock(false);
							(*it)->calculateDwellTime(0,alightingNum,0,now,false);
						}
						(*it)->setNextRequested(TrainDriver::REQUESTED_WAITING_LEAVING);
						isDisruptedState = true;
					}
				}

				else
				{
					if((!(*it)->getForceAlightFlag())&&((*it)->getMovement()->getDisruptedState()||(*it)->getUTurnFlag()))
					{
						(*it)->lockUnlockRestrictPassengerEntitiesLock(true);
						int alightingNum = (*it)->alightAllPassengers(leavingPersons[platform], now);
						(*it)->lockUnlockRestrictPassengerEntitiesLock(false);
						(*it)->calculateDwellTime(0,alightingNum,0,now,false);
						(*it)->setNextRequested(TrainDriver::REQUESTED_WAITING_LEAVING);
						isDisruptedState = true;
					}

				}


				if(!isDisruptedState)
				{
					if((*it)->getTerminateStatus())
					{
						if(!(*it)->getForceAlightStatus())
						{
							(*it)->lockUnlockRestrictPassengerEntitiesLock(true);
							int alightingNum = (*it)->alightAllPassengers(leavingPersons[platform],now);
							(*it)->lockUnlockRestrictPassengerEntitiesLock(false);
							(*it)->calculateDwellTime(0,alightingNum,0,now,false);
						}
						(*it)->setNextRequested(TrainDriver::REQUESTED_WAITING_LEAVING);
					}
					if((*it)->getUTurnFlag())
					{
						if(!(*it)->getForceAlightStatus())
						{
							(*it)->lockUnlockRestrictPassengerEntitiesLock(true);
							int alightingNum = (*it)->alightAllPassengers(leavingPersons[platform],now);
							(*it)->lockUnlockRestrictPassengerEntitiesLock(false);
							(*it)->calculateDwellTime(0,alightingNum,0,now,false);
						}
						(*it)->setNextRequested(TrainDriver::REQUESTED_WAITING_LEAVING);
					}
					else
					{
						if( !(*it)->getForceAlightStatus() )
						{
							(*it)->lockUnlockRestrictPassengerEntitiesLock(true);
							int alightingNum = (*it)->alightPassenger(leavingPersons[platform],now);
							int noOfPassengersInTrain = (*it)->getPassengers().size();
							int forcealightedboardingnum = 0;
							if(forceAlightedPersons.find(platform) != forceAlightedPersons.end())
							{
								forcealightedboardingnum = (*it)->boardForceAlightedPassengersPassenger(forceAlightedPersons[platform],now);
							}
							int boardingNum = (*it)->boardPassenger(waitingPersons[platform], now);
							(*it)->lockUnlockRestrictPassengerEntitiesLock(false);
							(*it)->calculateDwellTime(boardingNum+forcealightedboardingnum,alightingNum,noOfPassengersInTrain,now,false);
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

					messaging::MessageBus::PostMessage(this,TRAIN_ARRIVAL_AT_ENDPOINT,
											messaging::MessageBus::MessagePtr(new TrainDriverMessage((*it))));
				}
				it = trainDriver.erase(it);
				it--;
				break;
			}

			else if((*it)->getNextRequested() == TrainDriver::REQUESTED_TAKE_UTURN)
			{
			   break;
			}

		}while (tickInSec < sysGran);

		pushForceAlightedPassengersToWaitingQueue(platform);
		it++;
	}

	passengerLeaving(now);
	return UpdateStatus::Continue;
}

	void TrainStationAgent::pushForceAlightedPassengersToWaitingQueue(const Platform *platform)
	{
		std::list<Passenger*> &leavingPassengers = leavingPersons[platform];
		for (std::list<Passenger*>::iterator it = leavingPassengers.begin(); it != leavingPassengers.end(); )
		{

			Passenger *pr = dynamic_cast<Passenger*>(*it);
			const WayPoint endPoint = pr->getEndPoint();
			const Platform * alightingPlatform = endPoint.platform;
			std::string lineId = platform->getLineId();
			std::string stationNo = platform->getStationNo();
			std::vector<Platform*> platforms = TrainController<sim_mob::medium::Person_MT>::getInstance()->getPlatforms(lineId,stationNo);
			std::vector<Platform*>::iterator itr = std::find(platforms.begin(),platforms.end(),alightingPlatform);
			if(alightingPlatform == platform)
			{
				it++;
			}
			else if(itr != platforms.end())
			{

				if(forceAlightedPersons.find(platform) == forceAlightedPersons.end())
				{
					forceAlightedPersons[platform] = std::list<Passenger*>();
				}
				forceAlightedPersons[platform].push_back(pr);
				personsForcedAlighted.push_back(pr->getParent());
				it = leavingPersons[platform].erase(it);

			}
			else
			{
				std::string oppLineId=TrainController<sim_mob::medium::Person_MT>::getInstance()->getOppositeLineId(lineId);
				std::vector<Platform*> oppPlatforms = TrainController<sim_mob::medium::Person_MT>::getInstance()->getPlatforms(oppLineId,stationNo);
				std::string alightingPlatformStationNo = alightingPlatform->getStationNo();
				Station *alightingPlatformStation = TrainController<sim_mob::medium::Person_MT>::getInstance()->getStationFromId(alightingPlatformStationNo);
				Platform *oppAlightingPlatform = alightingPlatformStation->getPlatform(oppLineId);

				itr = std::find(oppPlatforms.begin(), oppPlatforms.end(), oppAlightingPlatform);
				if(itr != oppPlatforms.end())
				{
					Station *station = TrainController<sim_mob::medium::Person_MT>::getInstance()->getStationFromId(stationNo);
					Platform *oppPlatform = station->getPlatform(oppLineId);
					if(forceAlightedPersons.find(oppPlatform) == forceAlightedPersons.end())
					{
						forceAlightedPersons[oppPlatform] = std::list<Passenger*>();
					}
					forceAlightedPersons[oppPlatform].push_back(pr);
					sim_mob::WayPoint wayPoint(oppAlightingPlatform);
					pr->setEndPoint(wayPoint);
					it = leavingPersons[platform].erase(it);
				}
			}
		}
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
				if(platformNames.size() > 0 && disruptionParam->platformLineIds.size()>0)
				{
					Platform* platform = TrainController<Person_MT>::getPrePlatform(disruptionParam->platformLineIds.front(),platformNames.front());
					if(platform)
					{
						platformNames.push_back(platform->getPlatformNo());
					}
				}

				for(it = platformNames.begin(); it != platformNames.end(); it++)
				{
					if(TrainController<Person_MT>::checkPlatformIsExisted(this, *it))
					{
						triggerRerouting(DisruptionEventArgs(*disruptionParam),now);
						arePassengersreRouted =true;
						break;
					}
				}
				std::list<Person_MT*> colPersons;
				for(std::list<TrainDriver*>::iterator it = trainDriver.begin(); it != trainDriver.end(); it++)
				{
					colPersons.push_back((*it)->getParent());
				}
				for(std::list<Person_MT*>::iterator it = colPersons.begin(); it != colPersons.end(); it++)
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
			while (it != trainDriver.end())
			{
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
		int trainId  = aheadDriver->getTrainId();
		std::list<TrainDriver*>::iterator it = trainDriver.begin();
		while (it != trainDriver.end())
		{
			if((*it)->getNextDriver() == aheadDriver)
			{
				TrainDriver *nextDriver = aheadDriver->getNextDriver();
				if(nextDriver)
				{
					(*it)->setNextDriver(nextDriver);
				}
				else
				{
					(*it)->setNextDriver(nullptr);
				}
			}
			it++;
		}

		std::map<std::string, TrainDriver*>::iterator iLastDriver = lastTrainDriver.find(lineId);
		if(iLastDriver != lastTrainDriver.end())
		{
			if(iLastDriver->second == aheadDriver)
			{
				lastTrainDriver[lineId] = nullptr;

			}
		}

		ServiceController::getInstance()->removeTrainIdAndTrainDriverInMap(trainId,lineId,aheadDriver);
		TrainController<Person_MT>::getInstance()->removeFromListOfActiveTrainsInLine(lineId,aheadDriver);
	}

	Entity::UpdateStatus TrainStationAgent::callMovementFrameTick(timeslice now, TrainDriver* driver)
	{
		if(driver)
		{
			driver->make_frame_tick_params(now);
			bool isToBeRemoved=false;
			driver->Movement()->frame_tick();
		}
		return UpdateStatus::Continue;
	}
}
} /* namespace sim_mob */
