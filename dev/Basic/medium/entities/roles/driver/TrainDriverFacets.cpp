/*
 * TrainDriverFacets.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */

#include "entities/roles/driver/TrainDriverFacets.hpp"
#include "entities/misc/TrainTrip.hpp"
#include "entities/Person_MT.hpp"
#include "entities/roles/driver/TrainDriver.hpp"
#include "entities/TrainController.hpp"
#include "message/MT_Message.hpp"
#include "message/MessageBus.hpp"
#include "message/MessageHandler.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include <iostream>
#include "stdlib.h"
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string.hpp>
#include "TrainPathMover.hpp"
#include "entities/TrainStationAgent.hpp"

using namespace std;

namespace
{
	const double distanceArrvingAtPlatform = 0.001;
	const double trainLengthMeter = 138;
	const double convertKmPerHourToMeterPerSec = 1000.0/3600.0;
	/**
	 * converts time from  seconds to milli-seconds
	 */
	inline unsigned int converToMilliseconds(double timeInSecs)
	{
		return (timeInSecs*1000.0);
	}
}

namespace sim_mob
{
	namespace medium
	{
		bool TrainMovement::areColumnNamesAdded=false;
		TrainBehavior::TrainBehavior():BehaviorFacet(),parentDriver(nullptr)
		{

		}
		TrainBehavior::~TrainBehavior()
		{

		}
		void TrainBehavior::frame_init()
		{
		}
		void TrainBehavior::frame_tick()
		{

		}
		std::string TrainBehavior::frame_tick_output()
		{
			return std::string();
		}
		TrainDriver* TrainBehavior::getParentDriver() const
		{
			return parentDriver;
		}

		void TrainBehavior::setParentDriver(TrainDriver* parentDriver)
		{
			if (!parentDriver)
			{
				throw std::runtime_error("parentDriver cannot be NULL");
			}
			this->parentDriver = parentDriver;
		}

		TrainMovement::TrainMovement():MovementFacet(),parentDriver(nullptr),safeDistance(0),safeHeadway(0),nextPlatform(nullptr),forceResetMovingCase(false)
		{
			const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
			safeDistanceLock.lock();
			safeDistance = config.trainController.safeDistance;
			safeDistanceLock.unlock();
			safeHeadway = config.trainController.safeHeadway;
		}

		void TrainMovement::resetSafeHeadWay(double safeHeadWay)
		{
			safeHeadwayLock.lock();
			this->safeHeadway = safeHeadWay;
			safeHeadwayLock.unlock();
		}

		void TrainMovement::resetSafeDistance(double safeDistance)
		{
			safeDistanceLock.lock();
			this->safeDistance = safeDistance;
			safeDistanceLock.unlock();
		}

		TrainMovement::~TrainMovement()
		{

		}
		TravelMetric& TrainMovement::startTravelTimeMetric()
		{
			return  travelMetric;
		}

		const TrainPathMover& TrainMovement::getPathMover() const
		{
			return trainPathMover;
		}

		Platform* TrainMovement::getNextPlatform()
		{
			facetMutex.lock();
			Platform* next = nextPlatform;
			facetMutex.unlock();
			return next;
		}

		Platform* TrainMovement::getNextPlatformFromPlatformMover()
		{
			return trainPlatformMover.getNextPlatform();
		}

		TravelMetric& TrainMovement::finalizeTravelTimeMetric()
		{
			return  travelMetric;
		}

		void TrainMovement::frame_init()
		{
			Person_MT* person = parentDriver->parent;
			const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*(person->currTripChainItem));
			if (!trip)
			{
				Print()<< "train trip is null"<<std::endl;
			}
			else
			{
				trainPathMover.setPath(trip->getTrainRoute());
				trainPlatformMover.setPlatforms(trip->getTrainPlatform());
				trainPlatformMover_accpos.setPlatforms(trip->getTrainPlatform());
				//nextPlatform=trainPlatformMover.getNextPlatform(); //first platform
				trainPathMover.SetParentMovementFacet(this);
				Platform* next = trainPlatformMover.getNextPlatform();
				facetMutex.lock();
				nextPlatform = next;
				facetMutex.unlock();

			}
		}

		void TrainMovement::changeTrip()
		{
			Person_MT* person = parentDriver->parent;
			std::string lineId=parentDriver->getTrainLine();
			Platform *platform=getNextPlatform();
			TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
			std::string oppLineId=trainController->getOppositeLineId(lineId);
			TrainTrip* trip = dynamic_cast<TrainTrip*>(*(person->currTripChainItem));
			trip->setLineId(oppLineId);
			std::vector<Block*>route;
			std::vector<Platform*> platforms;
			trainController->getTrainRoute(oppLineId,route);
			trip->setTrainRoute(route);
			trainController->getTrainPlatforms(oppLineId,platforms);
			trip->setTrainPlatform(platforms);
			trainPlatformMover.setPlatforms(platforms);
			std::string stationName=platform->getStationNo();
			Station *stn=trainController->getStationFromId(stationName);
			Platform *oppPlatform=stn->getPlatform(oppLineId);
			Platform *inroutePlaform=nullptr;
			while(oppPlatform!=inroutePlaform)
			{
				inroutePlaform=trainPlatformMover.getNextPlatform(true);
			}
			TakeUTurn(stationName);
		}

		bool TrainMovement::checkIfTrainsAreApprochingOrAtPlatform(std::string platformNo,std::string lineID)
		{
		   //get the vector of train drivers for that line
			TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
			typename  std::vector <Role<Person_MT>*> trainDriverVector=trainController->getActiveTrainsForALine(lineID);
			//std::vector<Role<Person_MT>*> trainDriverVector;
			std::vector<Role<Person_MT>*>::iterator it;
			for(it=trainDriverVector.begin();it!=trainDriverVector.end();it++)
			{
				TrainDriver *tDriver=dynamic_cast<TrainDriver*>(*(it));
				if(tDriver)
				{
					MovementFacet *moveFacet=tDriver->getMovement();
					if(moveFacet)
					{
						TrainMovement* trainMovement=dynamic_cast<TrainMovement*>(moveFacet);
						if(trainMovement)
						{
							Platform *platform=trainMovement->getNextPlatform();
							if(platform)
							{
								if(boost::iequals(platform->getPlatformNo(),platformNo))
								{
								   return true;
								}
							}
						}
					}
				}
			}
		return false;
		}

		bool TrainMovement::checkSafeHeadWayBeforeTeleport(std::string platformNo,std::string lineID)
		{
			TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
			typename  std::vector <Role<Person_MT>*> trainDriverVector=trainController->getActiveTrainsForALine(lineID);
			TrainPlatform trainPlatform=trainController->getNextPlatform(platformNo,lineID);
			Platform *platform=trainController->getPlatformFromId(platformNo);
			typename std::vector <Role<Person_MT>*>::iterator it=trainDriverVector.begin();
			double minDis=-1;
			TrainDriver *nextDriverInOppLine=nullptr;
			while(it!=trainDriverVector.end())
			{

				MovementFacet *movementFacet=(*it)->Movement();
				if(movementFacet)
				{
					TrainMovement *trainMovement=dynamic_cast<TrainMovement*>(movementFacet);
					if(trainMovement)
					{
						Platform *nextPlatform=trainMovement->getNextPlatform();
						if(nextPlatform)
						{
							const TrainPathMover &trainPathMover=trainMovement->getPathMover();
							double disCovered=trainPathMover.getTotalCoveredDistance();
							double disOfPlatform = getDistanceFromStartToPlatform(lineID,platform);
							std::string nextPlatformNo=nextPlatform->getPlatformNo();
							if(boost::iequals(trainPlatform.platformNo,nextPlatformNo))
							{
								if(minDis==-1||(disCovered-disOfPlatform<minDis))
								{
								 minDis=disCovered-disOfPlatform;
								 nextDriverInOppLine = dynamic_cast<TrainDriver*>((*it));
								}

								safeDistanceLock.lock();
								if(disCovered-disOfPlatform-trainLengthMeter<=safeDistance)
								{
								 safeDistanceLock.unlock();
								 return false;
								}
								safeDistanceLock.unlock();
							}
							else
							{
								if(minDis==-1||disCovered-disOfPlatform<minDis)
								{
								 minDis=disCovered-disOfPlatform;
								 nextDriverInOppLine = dynamic_cast<TrainDriver*>((*it));
								}
							}
						}
					}
				}
				it++;
		  }
		  TrainDriver * trainDriver=this->getParentDriver();
		  if(trainDriver)
		  {
			trainDriver->setTrainDriverInOpposite(nextDriverInOppLine);
		  }
		return true;
		}

		double TrainMovement::getTotalCoveredDistance()
		{
			return trainPathMover.getTotalCoveredDistance();
		}

		double TrainMovement::getDistanceFromStartToPlatform(std::string lineID,Platform *platform)
		{
			return trainPathMover.GetDistanceFromStartToPlatform(lineID,platform);
		}

		void TrainMovement::teleportToPlatform(std::string platformName)
		{
			if(!isRouteSet())
			{
				trainPathMover.teleportToPlatform(platformName);
				routeSet=true;
			}
		}

		double TrainMovement::getSafeDistance()
		{
			return safeDistance;
		}

		void TrainMovement::TakeUTurn(std::string stationName)
		{
			//std::string platformName=getNextPlatform();
			TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
			Person_MT* person = parentDriver->parent;
			TrainTrip* trip = dynamic_cast<TrainTrip*>(*(person->currTripChainItem));
			std::string lineId=trip->getLineId();
			Platform *platform=parentDriver->getNextPlatform();
			std::string stationId=platform->getStationNo();
			Station *station=trainController->getStationFromId(stationId);
			Platform *oppPlatform=station->getPlatform(lineId);
			std::vector<Block *> blocks=trip->getTrainRoute();
			trainPathMover.setPath(trip->getTrainRoute());
			trainPathMover.TeleportToOppositeLine(stationId,lineId,oppPlatform);
			facetMutex.lock();
			nextPlatform = oppPlatform;
			facetMutex.unlock();
			TrainDriver *parentDriver=getParentDriver();
			std::string prevLine=TrainController<Person_MT>::getInstance()->getOppositeLineId(lineId);
			TrainController<Person_MT>::getInstance()->removeFromListOfActiveTrainsInLine(prevLine,parentDriver);
			TrainController<Person_MT>::getInstance()->addToListOfActiveTrainsInLine(lineId,parentDriver);
			typename  std::vector <Role<Person_MT>*> trainDriverVector=TrainController<Person_MT>::getInstance()->getActiveTrainsForALine(prevLine);
			std::vector<Role<Person_MT>*>::iterator it;
			for(it=trainDriverVector.begin();it!=trainDriverVector.end();it++)
			{
				TrainDriver *tDriver=dynamic_cast<TrainDriver*>(*(it));
				if(tDriver)
				{
					if(tDriver->getNextDriver()==parentDriver)
					{
						tDriver->setNextDriver(nullptr);
					}
				}
			}

			parentDriver->setNextDriver(parentDriver->getDriverInOppositeLine());
			parentDriver->setUturnFlag(false);
		}

		void TrainMovement::setNextPlatform(Platform *platform)
		{
			facetMutex.lock();
			nextPlatform = platform;
			facetMutex.unlock();
		}

		void TrainMovement::produceDwellTimeInfo()
		{
			const std::string& fileName("pt_mrt_dwellTime.csv");
			sim_mob::BasicLogger& ptMRTMoveLogger  = sim_mob::Logger::log(fileName);
			if(TrainMovement::areColumnNamesAdded==false)
			{

				ptMRTMoveLogger <<"TimeOfDay"<<",";
				ptMRTMoveLogger<<"TrainId"<<",";
				ptMRTMoveLogger<<"TripId"<<",";
				ptMRTMoveLogger<<"PlatformId"<<",";
				ptMRTMoveLogger<<"Dwell Time"<< std::endl;
				TrainMovement::areColumnNamesAdded=true;
				std::cout<<"column names added";
			}
			TrainUpdateParams& params = parentDriver->getParams();
			DailyTime startTime = ConfigManager::GetInstance().FullConfig().simStartTime();
			const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*(parentDriver->getParent()->currTripChainItem));
			if (trip&&trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform())==0)
			{
				ptMRTMoveLogger << DailyTime(params.now.ms()+startTime.getValue()).getStrRepr() << ",";
				ptMRTMoveLogger << trip->getTrainId() << ",";
				ptMRTMoveLogger <<trip->getTripId()<<",";
				ptMRTMoveLogger << trainPlatformMover.getNextPlatform()->getStationNo()<<",";
				ptMRTMoveLogger << this->parentDriver->initialDwellTime << std::endl;

			}

		}
		void TrainMovement::produceMoveInfo()
		{
			TrainUpdateParams& params = parentDriver->getParams();
			const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*(parentDriver->getParent()->currTripChainItem));
			const std::string& fileName("pt_mrt_move.csv");
			sim_mob::BasicLogger& ptMRTMoveLogger  = sim_mob::Logger::log(fileName);
			DailyTime startTime = ConfigManager::GetInstance().FullConfig().simStartTime();
			ptMRTMoveLogger << DailyTime(params.now.ms()+startTime.getValue()).getStrRepr() << ",";
			if((boost::iequals(DailyTime(params.now.ms()+startTime.getValue()).getStrRepr(),"07:47:00")>=0)&&trip->getTrainId()==10&&trip->getTripId()==1016)
			{
				int a=6;
			}
			if(trip) ptMRTMoveLogger << trip->getLineId() << ",";
			if(trip) ptMRTMoveLogger << trip->getTrainId() << ",";
			if(trip) ptMRTMoveLogger << trip->getTripId() << ",";
			ptMRTMoveLogger << params.currentSpeed/convertKmPerHourToMeterPerSec << ",";
			Platform* next = trainPlatformMover.getNextPlatform();
			std::string platformNo("");
			if(next)
			{
				platformNo = next->getPlatformNo();
				ptMRTMoveLogger << platformNo << ",";
				ptMRTMoveLogger << trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform()) << ",";
			}
			ptMRTMoveLogger << trainPathMover.getTotalCoveredDistance() << ",";
			ptMRTMoveLogger << trainPathMover.getCurrentPosition().getX() << ",";
			ptMRTMoveLogger << trainPathMover.getCurrentPosition().getY() << ",";
			ptMRTMoveLogger << params.currentAcelerate << ",";
			ptMRTMoveLogger << parentDriver->getPassengers().size()<<",";
			ptMRTMoveLogger << parentDriver->getWaitingTime() << std::endl;
			produceDwellTimeInfo();
			passengerInfo();
		}

		 void TrainMovement::passengerInfo()
		 {
			 TrainUpdateParams& params = parentDriver->getParams();
			 const std::string& fileName("pt_mrt_passengernfo.csv");
			 sim_mob::BasicLogger& ptMRTMoveLogger  = sim_mob::Logger::log(fileName);
			 std::list <Passenger*>pList=parentDriver->getPassengers();
			 DailyTime startTime = ConfigManager::GetInstance().FullConfig().simStartTime();
			 std::list<Passenger*>::const_iterator it=pList.begin();
			 while(it!=pList.end())
			 {

				 ptMRTMoveLogger << DailyTime(params.now.ms()+startTime.getValue()).getStrRepr() << ","<<(*it)->getParent()->GetId()<<","<<getParentDriver()->getTrainId()<<","<<getParentDriver()->getTripId()<<","<<(*it)->getParent()->currSubTrip->origin.platform->getPlatformNo()<<","<<(*it)->getParent()->currSubTrip->destination.platform->getPlatformNo()<<std::endl;
				 it++;
			 }
		 }

		 bool TrainMovement::isRouteSet()
		 {
			 return routeSet;
		 }

		 void TrainMovement::setRouteStatus(bool status)
		 {
			 routeSet=status;
		 }

		 void TrainMovement::setIgnoreSafeDistanceByServiceController(bool ignore)
		 {
			 ignoreSafeDistance_RequestServiceController = ignore;
		 }

		 void TrainMovement::setIgnoreSafeHeadwayByServiceController(bool ignore)
		 {
			 ignoreSafeHeadway_RequestServiceController = ignore;
		 }

		bool TrainMovement::isStopPointPresent()
		 {
			const TrainPathMover &pathMover=getPathMover();
			std::vector<PolyPoint>::const_iterator pointItr=pathMover.GetCurrentStopPoint();
			PolyPoint stopPoint = *pointItr;
			PolyPoint nextPoint= *(pointItr+1);
			std::vector<StopPointEntity> &stopPointEntities=parentDriver->getStopPoints();
			int index=0;
			std::vector<StopPointEntity>::iterator stopPointItr=stopPointEntities.begin();
			while(stopPointItr!=stopPointEntities.end())
			{
				StopPointEntity stEntity=*stopPointItr;
				PolyPoint stPoint=stEntity.point;
				if(stPoint.getX()==(*pointItr).getX()&&stPoint.getY()==(*pointItr).getY()&&stPoint.getZ()==(*pointItr).getZ())
				{
					parentDriver->setStoppingParameters(stPoint,(*stopPointItr).duration);
					stopPointEntities.erase(stopPointItr);
					//remove stop point from list
					return true;
				}

				else
				{

					if(stPoint.getX()==nextPoint.getX()&&stPoint.getY()==nextPoint.getY()&&stPoint.getZ()==nextPoint.getZ())
					{
						TrainUpdateParams& params = parentDriver->getParams();
						if(params.distanceToNextStopPoint-params.movingDistance<distanceArrvingAtPlatform)
						{
							parentDriver->setStoppingParameters(stPoint,(*stopPointItr).duration);
							stopPointEntities.erase(stopPointItr);
							//remove stop point from list
							return true;
						}
					}
				}
				stopPointItr++;
				index++;
			}
			return false;
		 }

		void TrainMovement::frame_tick()
		{

			produceMoveInfo();
			TrainUpdateParams& params = parentDriver->getParams();
			parentDriver->updatePassengers();
			if(getParentDriver()->isStoppedAtPoint())
			{
				double remainingTime=parentDriver->reduceStoppingTime(params.secondsInTick);
				if(remainingTime<params.secondsInTick)
				{
					parentDriver->setStoppingTime(0);
					parentDriver->setStoppingStatus(false);
				}
			}
			TrainDriver::TRAIN_NEXTREQUESTED requested = parentDriver->getNextRequested();

			switch(requested)
			{
				case TrainDriver::REQUESTED_TO_PLATFROM:
				{
					if(!parentDriver->isStoppedAtPoint())
					{
						updatePlatformsList();
						Platform *nextPlatformAccordingToPosition=trainPlatformMover_accpos.getNextPlatform(false);
						moveForward();
						double distance=trainPathMover.GetDistanceFromStartToPlatform(parentDriver->getTrainLine(),nextPlatformAccordingToPosition);
						if(isStopAtPlatform())
						{
							if(boost::iequals(parentDriver->getNextPlatform()->getPlatformNo(),"NE3_1")||boost::iequals(parentDriver->getNextPlatform()->getPlatformNo(),"NE4_1"))
							{
								bool stophere=true;
							}
							forceResetMovingCase=false;
							parentDriver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
						}
						else
						{
							if(distance<getTotalCoveredDistance())
							{
								trainPlatformMover_accpos.getNextPlatform(true);
							}
						}
						isStopPointPresent();
					}

					break;
				}

				case TrainDriver::REQUESTED_WAITING_LEAVING:
				{
					//need to handle the case when the is stopped at platform and stop point is given
					parentDriver->reduceWaitingTime(params.secondsInTick);
					parentDriver->resetHoldingTime();
					double waitingTime = parentDriver->getWaitingTime();
					params.currentSpeed = 0.0;
					params.currentAcelerate = 0.0;
					if(waitingTime<0)
					{
						parentDriver->setWaitingTime(0.0);
					}

					if(waitingTime<params.secondsInTick)
					{
						if(parentDriver->getSubsequentNextRequested()!=TrainDriver::NO_REQUESTED)
						{
							parentDriver->setNextRequested(parentDriver->getSubsequentNextRequested());
							isDisruptedState =false;
							isStrandedBetweenPlatforms_DisruptedState =false;
							parentDriver->setSubsequentNextRequested(TrainDriver::NO_REQUESTED);
						}

						else if(isDisruptedState&&!isStrandedBetweenPlatforms_DisruptedState&&!parentDriver->getUTurnFlag())
						{
							std::string trainLine=parentDriver->getTrainLine();
							std::map<std::string,std::vector<std::string>> platformNames = TrainController<sim_mob::medium::Person_MT>::getInstance()->getDisruptedPlatforms_ServiceController();
							std::vector<std::string> disruptedPlatformNames=platformNames[trainLine];
							Platform *platform=trainPlatformMover.getPlatformByOffset(0);
							std::vector<std::string>::iterator it=std::find(disruptedPlatformNames.begin(),disruptedPlatformNames.end(),platform->getPlatformNo());
							if(it==disruptedPlatformNames.end())
							{
								isDisruptedState=false;
								//set to requested at platform to board new passengers after disruption is over.
								parentDriver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
							}

						}

						else if(isStrandedBetweenPlatforms_DisruptedState)
						{
							std::string trainLine=parentDriver->getTrainLine();
							std::map<std::string,std::vector<std::string>> platformNames = TrainController<sim_mob::medium::Person_MT>::getInstance()->getDisruptedPlatforms_ServiceController();
							std::vector<std::string> disruptedPlatformNames=platformNames[trainLine];
							Platform *platform=trainPlatformMover.getPlatformByOffset(0);
							std::vector<std::string>::iterator it=std::find(disruptedPlatformNames.begin(),disruptedPlatformNames.end(),platform->getPlatformNo());
							if(it==disruptedPlatformNames.end())
							{
								isDisruptedState=false;
								isStrandedBetweenPlatforms_DisruptedState=false;
								//move train ahead
								parentDriver->setNextRequested(TrainDriver::REQUESTED_TO_PLATFROM);

							}
						}

						else
						{
							bool isDisruptedPlatform = false;
							if(parentDriver->disruptionParam.get())
							{
								const std::vector<std::string>& platformNames = parentDriver->disruptionParam->platformNames;
								Platform* cur = trainPlatformMover.getPlatformByOffset(0);
								Platform* next = trainPlatformMover.getPlatformByOffset(1);
								if(cur)
								{
									auto it = std::find(platformNames.begin(), platformNames.end(), cur->getPlatformNo());
									if(it!=platformNames.end())
									{
										isDisruptedPlatform = true;
									}


								}
								if(next && !isDisruptedPlatform)
								{
									auto it = std::find(platformNames.begin(), platformNames.end(), next->getPlatformNo());
									if(it!=platformNames.end())
									{
										parentDriver->setNextRequested(TrainDriver::REQUESTED_LEAVING_PLATFORM);
										parentDriver->getParent()->setToBeRemoved();
										arrivalAtEndPlatform();
										isDisruptedPlatform = true;
										double dwellTimeInSecs = parentDriver->initialDwellTime;
										DailyTime dwellTime(converToMilliseconds(dwellTimeInSecs));
										parentDriver->storeArrivalTime(parentDriver->arrivalTimeAtPlatform, dwellTime.getStrRepr());
									}
								}
							}
							else
							{
								if(!isDisruptedState)
								{
									std::string trainLine=parentDriver->getTrainLine();
									std::map<std::string,std::vector<std::string>> platformNames = TrainController<sim_mob::medium::Person_MT>::getInstance()->getDisruptedPlatforms_ServiceController();
									std::vector<std::string> disruptedPlatformNames=platformNames[trainLine];
									Platform *platform=trainPlatformMover.getPlatformByOffset(0);
									std::vector<std::string>::iterator it=std::find(disruptedPlatformNames.begin(),disruptedPlatformNames.end(),platform->getPlatformNo());
									if(it!=disruptedPlatformNames.end())
									{
										isDisruptedState=true;
										isDisruptedPlatform =true;
										//compute new dwell time
										//set to requested at platform to board new passengers after disruption is over.
										parentDriver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
									}
									else
									{
										Platform *platform=trainPlatformMover.getPlatformByOffset(1);
										if(platform)
										{
											std::vector<std::string>::iterator it=std::find(disruptedPlatformNames.begin(),disruptedPlatformNames.end(),platform->getPlatformNo());
											if(it!=disruptedPlatformNames.end())
											{
												//for uturn force alight compute dwell time
												//set to requested at platform to board new passengers after disruption is over.
												parentDriver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
												isDisruptedState=true;
												isDisruptedPlatform =true;
											}
										}
									}
								}
							}


							if(!isDisruptedPlatform)
							{

								double dwellTimeInSecs=0.0;

								if(waitingTime<0.0)
								{
									dwellTimeInSecs = parentDriver->initialDwellTime-waitingTime;
								}
								else
								{
									dwellTimeInSecs = parentDriver->initialDwellTime;
								}
								DailyTime dwellTime(converToMilliseconds(dwellTimeInSecs));
								parentDriver->storeArrivalTime(parentDriver->arrivalTimeAtPlatform, dwellTime.getStrRepr());
								if(parentDriver->getTerminateStatus())
								{
									if(parentDriver->isStoppedAtPoint()==false)
									{
										parentDriver->setNextRequested(TrainDriver::REQUESTED_LEAVING_PLATFORM);
										parentDriver->getParent()->setToBeRemoved();
										arrivalAtEndPlatform();
									}
								}

								else
								{

									if(!parentDriver->isStoppedAtPoint())
									{
										parentDriver->setNextRequested(TrainDriver::REQUESTED_LEAVING_PLATFORM);
									}
									if(!isAtLastPlaform())
									{
										params.elapsedSeconds = waitingTime;
										parentDriver->setWaitingTime(0.0);
										if(!parentDriver->isStoppedAtPoint())
										{

											if(parentDriver->getUTurnFlag())
											{
											   isDisruptedState=false;
											   //parentDriver->setNextRequested(TrainDriver::REQUESTED_TAKE_UTURN);
											   Platform *platform=getNextPlatform();
												std::string stationNo=platform->getStationNo();
												Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
											   messaging::MessageBus::PostMessage(stationAgent,TRAIN_MOVE_AT_UTURN_PLATFORM,
														messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver,true)));
											   //pass message fo Uturn
											}

											else
											{
												Platform *nextPlatform=trainPlatformMover.getPlatformByOffset(1);
												std::string trainLine=parentDriver->getTrainLine();
												std::map<std::string,std::vector<std::string>> platformNames = TrainController<sim_mob::medium::Person_MT>::getInstance()->getDisruptedPlatforms_ServiceController();
												std::vector<std::string> disruptedPlatformNames=platformNames[trainLine];
												std::vector<std::string>::iterator it=std::find(disruptedPlatformNames.begin(),disruptedPlatformNames.end(),nextPlatform->getPlatformNo());
												bool nextplatformDisrupted=false;
												if(it!=disruptedPlatformNames.end())
												{
													nextplatformDisrupted=true;
												}
												if(nextplatformDisrupted)
												{
													parentDriver->setNextRequested(TrainDriver::REQUESTED_TAKE_UTURN);
													prepareForUTurn();
												}
												else
												{
													if(!leaveFromPlaform())
													{
													   parentDriver->getParent()->setToBeRemoved();
													   arrivalAtEndPlatform();
													}
												}
											}
										}
									}

									else
									{
										if(!parentDriver->isStoppedAtPoint())
										{
											parentDriver->getParent()->setToBeRemoved();
											arrivalAtEndPlatform();
										}
									}
								}
							}
						}
					}
					break;
				}

				case TrainDriver::REQUESTED_TAKE_UTURN:
				{

					//pass message to Uturn
					Platform *platform=getNextPlatform();
					std::string stationNo=platform->getStationNo();
					Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
					messaging::MessageBus::PostMessage(stationAgent,TRAIN_MOVE_AT_UTURN_PLATFORM,
									messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver,true)));

					break;
				}
			}
		}
		std::string TrainMovement::frame_tick_output()
		{
			TrainUpdateParams& params = parentDriver->getParams();
			const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
			if (!configParams.trainController.outputEnabled)
			{
				return std::string();
			}

			std::stringstream logout;
			logout << std::setprecision(8);
			logout << "(\"TrainDriver\"" << ","
					<< params.now.frame() << "," << parentDriver->getParent()->getId()
					<< ",{" << "\"xPos\":\"" << trainPathMover.getCurrentPosition().getX()
					<< "\",\"yPos\":\"" << trainPathMover.getCurrentPosition().getY()
					<< "\",\"length\":\"" << trainLengthMeter
					<< "\",\"passengerNum\":\"" << parentDriver->getPassengers().size()
					<< "\",\"next\":\"" << trainPlatformMover.getNextPlatform()->getPlatformNo()
					<< "\",\"lineID\":\"" << parentDriver->getTrainLine();
			logout << "\"})" << std::endl;
			return logout.str();
		}
		TrainDriver* TrainMovement::getParentDriver() const
		{
			return parentDriver;
		}

		void TrainMovement::setParentDriver(TrainDriver* parentDriver)
		{
			if (!parentDriver)
			{
				throw std::runtime_error("parentDriver cannot be NULL");
			}
			this->parentDriver = parentDriver;
		}
		bool TrainMovement::isStationCase(double disToTrain, double disToPlatform,double disToStopPoint, double& effectDis)
		{
			bool res = false;  //needs to be checked once again

			if(disToStopPoint>0)
			{
				if(disToPlatform>0&&disToTrain>0&&disToStopPoint<disToPlatform&&disToStopPoint<disToTrain)
				{
					effectDis = disToStopPoint;
					return true;
				}
				else if(disToTrain==0 &&disToPlatform>0&&disToStopPoint<disToPlatform)
				{
					effectDis = disToStopPoint;
					return true;
				}
			}
			effectDis = disToTrain;
			if (disToPlatform > 0.0 && disToTrain == 0.0)
			{
				effectDis = disToPlatform;
				res = true;
			}
			else if (disToTrain != 0.0 && disToPlatform != 0.0
					&& disToPlatform < disToTrain)
			{
				effectDis = disToPlatform;
				res = true;
			}
			else
			{
				const TrainDriver* next = parentDriver->getNextDriver();
				if (next)
				{
					if (disToPlatform > 0.0
							&& next->getNextPlatform() != parentDriver->getNextPlatform())
					{
						effectDis = disToPlatform;
						res = true;
					}
					else if (disToTrain > 0.0 && disToTrain < disToPlatform
							&& next->getNextRequested() == TrainDriver::REQUESTED_WAITING_LEAVING
							&& next->getNextPlatform() == parentDriver->getNextPlatform())
					{
						effectDis = disToTrain;
						res = true;
					}
				}
			}
			return res;
		}

		double TrainMovement::getDistanceToNextTrain(const TrainDriver* nextDriver,bool isSafed)
		{
			double distanceToNextTrain = 0.0;
			if(nextDriver)
			{
				if (nextDriver->getNextRequested() == TrainDriver::REQUESTED_TO_DEPOT)
				{
					parentDriver->setNextDriver(nullptr);
				}
				else
				{
					TrainMovement* nextMovement = dynamic_cast<TrainMovement*>(nextDriver->movementFacet);
					if (nextMovement)
					{
						double dis = trainPathMover.getDifferentDistance(nextMovement->getPathMover());
						if(ignoreSafeDistance_RequestServiceController)
						{	//handing different cases...ignore by service controller,ignore by normal simulation
							return dis - trainLengthMeter;
						}

						if (dis > 0)
						{
							std::map<std::string,std::vector<std::string>> platformNames=TrainController<sim_mob::medium::Person_MT>::getInstance()->getDisruptedPlatforms_ServiceController();
							std::string trainLine=parentDriver->getTrainLine();
							std::vector<std::string> disruptedPlatformNames=platformNames[trainLine];
							Platform *platform=trainPlatformMover.getPlatformByOffset(0);
							std::vector<std::string>::iterator it=std::find(disruptedPlatformNames.begin(),disruptedPlatformNames.end(),platform->getPlatformNo());
							if(it!=disruptedPlatformNames.end())
							{
								if((nextDriver->getNextRequested()==TrainDriver::REQUESTED_AT_PLATFORM||nextDriver->getNextRequested()==TrainDriver::REQUESTED_WAITING_LEAVING)&&nextDriver->getNextPlatform()==this->getNextPlatform())
								{
									distanceToNextTrain = dis - trainLengthMeter;
									return distanceToNextTrain;
								}
							}

							safeDistanceLock.lock();
							distanceToNextTrain = dis - safeDistance - trainLengthMeter;
							safeDistanceLock.unlock();

						}
					}
				}
			}
			return distanceToNextTrain;
		}

		double TrainMovement::getDistanceToNextPlatform(const TrainDriver* nextDriver)
		{
			return trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());

		}

		double TrainMovement::getRealSpeedLimit()
		{
			TrainUpdateParams& params = parentDriver->getParams();
			if(parentDriver->getTrainId()==1&&((int)getTotalCoveredDistance())==11)
			{
				int x=0;
			}
			double distanceToNextTrain = 0.0;
			double distanceToNextPlatform = 0.0;
			double distanceToNextObject = 0.0;
			double disToNextStopPoint=0;
			double speedLimit = 0.0;
			double speedLimit2 = 0.0;
			Point stopPoint;
			distanceToNextPlatform = trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());
			const TrainDriver* nextDriver = parentDriver->getNextDriver();
			distanceToNextTrain = getDistanceToNextTrain(nextDriver);
			std::vector<StopPointEntity> stopPoints=parentDriver->getStopPoints();
			std::vector<StopPointEntity>::iterator itr=stopPoints.begin();
			std::vector<PolyPoint> points;
			while(itr!=stopPoints.end())
			{
				points.push_back((*itr).point);
				itr++;
			}

			if(stopPoints.size()!=0)
			{
				std::vector<PolyPoint>::iterator stopPointItr=points.end();
				stopPointItr=trainPathMover.findNearestStopPoint(points);
				std::vector<PolyPoint>::const_iterator currStopPointItr=points.end();;
				currStopPointItr=trainPathMover.GetCurrentStopPoint();
				disToNextStopPoint=trainPathMover.calcDistanceBetweenTwoPoints(currStopPointItr,stopPointItr,points);
				if(disToNextStopPoint!=0)
					disToNextStopPoint=disToNextStopPoint-trainPathMover.getDistanceMoveToNextPoint();
			}

			if(disToNextStopPoint==0) /*means no stop point*/
			{
				if(distanceToNextTrain==0.0||distanceToNextPlatform==0.0)
				{
					distanceToNextObject = std::max(distanceToNextTrain, distanceToNextPlatform);
				}
				else
				{
					distanceToNextObject = std::min(distanceToNextTrain,distanceToNextPlatform);
				}
			}

			else
			{

				if(distanceToNextTrain==0.0&&distanceToNextPlatform==0.0)
				{
					distanceToNextObject = std::max(distanceToNextTrain, distanceToNextPlatform);
					distanceToNextObject=std::max(distanceToNextObject,disToNextStopPoint);
				}
				else
				{
					if(distanceToNextTrain==0)
					{
						distanceToNextObject = std::min(disToNextStopPoint,distanceToNextPlatform);
					}

					else if(distanceToNextPlatform==0)
					{
						distanceToNextObject = std::min(disToNextStopPoint,distanceToNextTrain);
					}

					else
					{
						distanceToNextObject = std::min(distanceToNextTrain, distanceToNextPlatform);
						distanceToNextObject=std::min(distanceToNextObject,disToNextStopPoint);
					}
				}
			}
			params.disToNextPlatform = distanceToNextPlatform;
			params.disToNextTrain = distanceToNextTrain;
			params.distanceToNextStopPoint=disToNextStopPoint;

			if(forceResetMovingCase==true && forceResetedCase==TRAINCASE::NORMAL_CASE)
			{
				params.currCase = TrainUpdateParams::NORMAL_CASE;
			}

			else
			{
				if (isStationCase(distanceToNextTrain,distanceToNextPlatform,disToNextStopPoint, distanceToNextObject))
				{
					double decelerate = trainPathMover.getCurrentDecelerationRate();
					speedLimit = std::sqrt(2.0 * decelerate * distanceToNextObject);
					if (params.currentSpeed > speedLimit)
					{
						params.currCase = TrainUpdateParams::STATION_CASE;
						params.currentSpeedLimit = speedLimit;
						return distanceToNextObject;
					}
				}
			}

			params.currCase = TrainUpdateParams::NORMAL_CASE;
			if(distanceToNextObject<0)
			{
				speedLimit = 0.0;
			}
			else
			{
				double decelerate = trainPathMover.getCurrentDecelerationRate();
				speedLimit = std::sqrt(2.0*decelerate*distanceToNextObject);
			}
			speedLimit = std::min(speedLimit, trainPathMover.getCurrentSpeedLimit()*convertKmPerHourToMeterPerSec);

			if(!ignoreSafeHeadway_RequestServiceController)
			{
				if (distanceToNextTrain > 0 && safeHeadway > 0)
				{
					speedLimit2 = distanceToNextTrain / safeHeadway;
				}

				if (speedLimit2 > 0)
				{
					speedLimit = std::min(speedLimit, speedLimit2);
				}
			}
			params.currentSpeedLimit = speedLimit;
			return speedLimit;
		}

		double TrainMovement::getEffectiveAccelerate()
		{
			TrainUpdateParams& params = parentDriver->getParams();
			double effectiveSpeed = params.currentSpeed;
			double realSpeedLimitordistanceToNextObj = getRealSpeedLimit();
			if(params.currCase==TrainUpdateParams::STATION_CASE)
			{
				double effectiveAccelerate = -(effectiveSpeed*effectiveSpeed)/(2.0*realSpeedLimitordistanceToNextObj);
				params.currentAcelerate = effectiveAccelerate;
				return effectiveAccelerate;
			}

			bool isAccelerated = false;
			if(effectiveSpeed<realSpeedLimitordistanceToNextObj)
			{
				isAccelerated = true;
			}

			if(isAccelerated)
			{
				effectiveSpeed = effectiveSpeed+trainPathMover.getCurrentAccelerationRate()*params.secondsInTick;
				effectiveSpeed = std::min(effectiveSpeed, realSpeedLimitordistanceToNextObj);
			}
			else
			{
				effectiveSpeed = effectiveSpeed-trainPathMover.getCurrentDecelerationRate()*params.secondsInTick;
				effectiveSpeed = std::max(effectiveSpeed, realSpeedLimitordistanceToNextObj);
			}

			double effectiveAccelerate = (effectiveSpeed-params.currentSpeed)/params.secondsInTick;
			params.currentAcelerate = effectiveAccelerate;
			return effectiveAccelerate;
		}

		double TrainMovement::getEffectiveMovingDistance()
		{
			double movingDist = 0.0;
			TrainUpdateParams& params = parentDriver->getParams();
			double effectiveAcc = getEffectiveAccelerate();
			if(params.currCase==TrainUpdateParams::STATION_CASE)
			{
				double nextSpeed = params.currentSpeed+effectiveAcc*params.secondsInTick;
				nextSpeed = std::max(0.0, nextSpeed);
				movingDist = (nextSpeed*nextSpeed-params.currentSpeed*params.currentSpeed)/(2.0*effectiveAcc);
				params.currentSpeed = nextSpeed;
			}
			else
			{
				movingDist = params.currentSpeed*params.secondsInTick+0.5*effectiveAcc*params.secondsInTick*params.secondsInTick;
				params.currentSpeed = params.currentSpeed+effectiveAcc*params.secondsInTick;
			}
			return movingDist;
		}

		std::string TrainMovement::getPlatformByOffset(int offset)
		{
			std::string platformName="";
			Platform *platform=trainPlatformMover.getPlatformByOffset(offset);
			if(platform)
			{
				platformName=platform->getPlatformNo();
			}
			return platformName;
		}

		bool TrainMovement::isUturnDueToDisruption()
		{
			return uTurnDueToDisruption;
		}

		bool TrainMovement::isStopAtPlatform()
		{

			std::map<std::string,std::vector<std::string>> platformNames=TrainController<sim_mob::medium::Person_MT>::getInstance()->getDisruptedPlatforms_ServiceController();
			std::string trainLine=parentDriver->getTrainLine();
			std::vector<std::string> disruptedPlatformNames=platformNames[trainLine];
			Platform *platform=trainPlatformMover.getPlatformByOffset(0);
			std::vector<std::string>::iterator it=std::find(disruptedPlatformNames.begin(),disruptedPlatformNames.end(),platform->getPlatformNo());
			if(it!=disruptedPlatformNames.end())
			{
				TrainDriver *nextDriver=parentDriver->getNextDriver();
				if(nextDriver)
				{
					if((nextDriver->getNextRequested()==TrainDriver::REQUESTED_AT_PLATFORM||nextDriver->getNextRequested()==TrainDriver::REQUESTED_WAITING_LEAVING)&&nextDriver->getNextPlatform()==getNextPlatform())
					{
						double distanceToNextTrain = getDistanceToNextTrain(nextDriver);
						if(distanceToNextTrain<distanceArrvingAtPlatform)
						{
							isStrandedBetweenPlatforms_DisruptedState=true;
							isDisruptedState=true;
							return true;
						}
					}
				}
			}
			double distanceToNextPlatform = trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());
			if(distanceToNextPlatform<distanceArrvingAtPlatform)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		bool TrainMovement::moveForward()
		{
			if(!isStopAtPlatform())
			{
				double movingDistance = getEffectiveMovingDistance();
				double distanceToNextPlat = trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());
				TrainUpdateParams& params = parentDriver->getParams();
				params.movingDistance=movingDistance;
				if(movingDistance>distanceToNextPlat&&movingDistance>params.distanceToNextStopPoint&&params.distanceToNextStopPoint!=0)
				{
					trainPathMover.advance(std::min(distanceToNextPlat,params.distanceToNextStopPoint));
					params.disToNextPlatform = trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());
				}

				else if(movingDistance>distanceToNextPlat)
				{
					trainPathMover.advance(distanceToNextPlat);
					params.disToNextPlatform = 0.0;
				}

				else if (movingDistance>params.distanceToNextStopPoint&&params.distanceToNextStopPoint!=0)
				{
					trainPathMover.advance(params.distanceToNextStopPoint);
					params.disToNextPlatform = trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());
				}

				else
				{
					trainPathMover.advance(movingDistance);
					params.disToNextPlatform = trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());
				}
				return true;
			}
			return false;
		}

		bool TrainMovement::isAtLastPlaform()
		{
			return trainPlatformMover.isLastPlatform();
		}
		bool TrainMovement::leaveFromPlaform()
		{
			if (!isAtLastPlaform())
			{
				moveForward();
				Platform *next=trainPlatformMover.getNextPlatform(true);
				Platform *u=trainPlatformMover_accpos.getNextPlatform(true);
				facetMutex.lock();
				nextPlatform = next;
				facetMutex.unlock();
				std::string stationNo = next->getStationNo();
				Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
				messaging::MessageBus::PostMessage(stationAgent,TRAIN_MOVETO_NEXT_PLATFORM,
						messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver,true)));
				return true;
			}
		}


		bool TrainMovement::updatePlatformsList()
		{

			if(parentDriver->getTrainId()==1)
			{
				int a=9;
			}
			std::vector<std::string> platformsToBeIgnored=parentDriver->getPlatformsToBeIgnored();
			Platform *nextPlt=trainPlatformMover.getPlatformByOffset(0);
			Platform *oldPlatform=nextPlt;
			Platform *nextPltAccPos=trainPlatformMover_accpos.getPlatformByOffset(0);
			int offset=0;
			while(nextPltAccPos!=nullptr)
			{
				std::vector<std::string>::iterator it= platformsToBeIgnored.begin();
				bool flag=false;
				while(it!=platformsToBeIgnored.end())
				{
					if(boost::iequals(nextPltAccPos->getPlatformNo(),*it))
					{
						offset++;
						nextPltAccPos=trainPlatformMover_accpos.getPlatformByOffset(offset);
						flag=true;
						break;
					}
					it++;
				}
				if(flag==false)
					break;
			}

			if(nextPltAccPos==nullptr)
			{
			   return false;
			}
			else
			{
				const std::vector<Platform*> &platforms=trainPlatformMover.getPlatforms();
				trainPlatformMover.resetPlatformItr();
				trainPlatformMover.clearPrevPlatforms();
				std::vector<Platform*>::const_iterator itr=std::find(platforms.begin(),platforms.end(),nextPltAccPos);
				if(itr!=platforms.end())
				{
					Platform* next = *(platforms.begin()) ;
					while(next!=nextPltAccPos)
					{
						next=trainPlatformMover.getNextPlatform(true);
					}

					if(next!=nullptr)
					{
						facetMutex.lock();
						nextPlatform = next;
						facetMutex.unlock();
						//remove train from station agent and add to appropriate station agent
						Agent *stationAgent=TrainController<sim_mob::medium::Person_MT>::getInstance()->getAgentFromStation(next->getStationNo());
						TrainStationAgent *trainStationAgent = dynamic_cast<TrainStationAgent*>(stationAgent);
						if(trainStationAgent)
						{
							std::list<TrainDriver*> &trains=trainStationAgent->getTrains();
							if(std::find(trains.begin(),trains.end(),parentDriver)==trains.end())
							{
								trainStationAgent->addTrainDriverInToStationAgent(parentDriver);
								Agent *stationAgentOld=TrainController<sim_mob::medium::Person_MT>::getInstance()->getAgentFromStation(oldPlatform->getStationNo());
								TrainStationAgent *trainStationAgentOld = dynamic_cast<TrainStationAgent*>(stationAgentOld);
								std::list<TrainDriver*> &trainsoldSt=trainStationAgent->getTrains();
								std::list<TrainDriver*>::iterator oldStTrainListItr=std::find(trainsoldSt.begin(),trainsoldSt.end(),parentDriver);
								if(oldStTrainListItr!=trainsoldSt.end())
								{
									trainsoldSt.erase(oldStTrainListItr);
								}
							}
						}
					}
				}
			}
			if(parentDriver->getTrainId()==1)
			{
				int a=9;
			}
		}

		void TrainMovement::resetMovingCase(TRAINCASE  trainCase)
		{
			forceResetMovingCase =true;
			forceResetedCase=trainCase;
		}
		void TrainMovement::prepareForUTurn()
		{
			Platform *pltform=getNextPlatform();
			std::string trainLine=parentDriver->getTrainLine();
			TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
			std::string oppLineId=trainController->getOppositeLineId(trainLine);
			std::string stationId=pltform->getStationNo();
			Station *station=trainController->getStationFromId(stationId);
			const Platform *oppPlatform=station->getPlatform(oppLineId);
			if(!checkIfTrainsAreApprochingOrAtPlatform(oppPlatform->getPlatformNo(),oppLineId))
			{
				if(checkSafeHeadWayBeforeTeleport(oppPlatform->getPlatformNo(),oppLineId))
				{

					changeTrip();
					parentDriver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
					Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationId);
					messaging::MessageBus::PostMessage(stationAgent,TRAIN_MOVE_AT_UTURN_PLATFORM,
					messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver,true)));
				}
			}
		}

		void TrainMovement::setDisruptedState(bool disruptedState)
		{
			isDisruptedState=true;
		}

		bool TrainMovement::getDisruptedState()
		{
			return isDisruptedState;
		}

		bool TrainMovement::isStrandedBetweenPlatform()
		{
			return isStrandedBetweenPlatforms_DisruptedState;
		}

		void TrainMovement::arrivalAtStartPlaform() const
		{
			Platform* next = trainPlatformMover.getFirstPlatform();
			std::string stationNo = next->getStationNo();
			Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
			messaging::MessageBus::PostMessage(stationAgent, TRAIN_ARRIVAL_AT_STARTPOINT,
					messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver)));
		}

		void TrainMovement::arrivalAtEndPlatform() const
		{
			std::vector<Platform*>::const_iterator it;
			const std::vector<Platform*>& prevs = trainPlatformMover.getPrevPlatforms();
			for (it = prevs.begin(); it != prevs.end(); it++)
			{
				Platform* prev = (*it);
				std::string stationNo = prev->getStationNo();
				Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
				messaging::MessageBus::PostMessage(stationAgent,TRAIN_ARRIVAL_AT_ENDPOINT,
						messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver)));
			}
		}
	}
} /* namespace sim_mob */
