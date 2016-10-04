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
#include "behavioral/ServiceController.hpp"
#include "entities/TrainRemoval.h"

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
				if(boost::iequals(parentDriver->getTrainLine(),"EW_1"))
				{
					int debug=1;
				}
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

		TrainPlatformMover& TrainMovement::getTrainPlatformMover()
		{
			return trainPlatformMover;
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
			trainPlatformMover_accpos.setPlatforms(platforms);
			std::string stationName=platform->getStationNo();
			Station *stn=trainController->getStationFromId(stationName);
			Platform *oppPlatform=stn->getPlatform(oppLineId);
			Platform *inroutePlaform=nullptr;
			while(oppPlatform!=inroutePlaform)
			{
				inroutePlaform = trainPlatformMover.getNextPlatform(true);
				trainPlatformMover_accpos.getNextPlatform(true);
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
			int trId=parentDriver->getTrainId();
			if(trId==21||trId==4)
			{
				int debug=1;
			}
			while(it!=trainDriverVector.end())
			{

				MovementFacet *movementFacet=(*it)->Movement();

				if(movementFacet)
				{
					TrainMovement *trainMovement=dynamic_cast<TrainMovement*>(movementFacet);
					int trainIdinLine = trainMovement->getParentDriver()->getTrainId();
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
								if((disCovered-disOfPlatform>0)&&(minDis==-1||(disCovered-disOfPlatform<minDis)))
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
		  int oppTrId=nextDriverInOppLine->getTrainId();
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
			int trainId=parentDriver->getTrainId();
			ServiceController::getInstance()->removeTrainIdAndTrainDriverInMap(trainId,prevLine,parentDriver);
			ServiceController::getInstance()->insertTrainIdAndTrainDriverInMap(trainId,lineId,parentDriver);
			typename  std::vector <Role<Person_MT>*> trainDriverVector=TrainController<Person_MT>::getInstance()->getActiveTrainsForALine(prevLine);
			std::vector<Role<Person_MT>*>::iterator it;
			for(it=trainDriverVector.begin();it!=trainDriverVector.end();it++)
			{
				TrainDriver *tDriver=dynamic_cast<TrainDriver*>(*(it));
				if(tDriver)
				{
					if(tDriver->getNextDriver()==parentDriver)
					{
						if(tDriver->getTrainId()==17)
						{
							int debug=1;
						}
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
				/*if(stPoint.getX()==(*pointItr).getX()&&stPoint.getY()==(*pointItr).getY()&&stPoint.getZ()==(*pointItr).getZ())
				{
					parentDriver->setStoppingParameters(stPoint,(*stopPointItr).duration);
					stopPointEntities.erase(stopPointItr);
					//remove stop point from list
					return true;
				}*/

				if((*stopPointItr).distance==getTotalCoveredDistance())
				{
					parentDriver->setStoppingParameters(stPoint,(*stopPointItr).duration);
					stopPointEntities.erase(stopPointItr);
					return true;
				}

				else
				{

					//if(stPoint.getX()==nextPoint.getX()&&stPoint.getY()==nextPoint.getY()&&stPoint.getZ()==nextPoint.getZ())
					//{
						TrainUpdateParams& params = parentDriver->getParams();
						//if(params.distanceToNextStopPoint-params.movingDistance<distanceArrvingAtPlatform)
						if((*stopPointItr).distance-getTotalCoveredDistance()<distanceArrvingAtPlatform)
						{
							parentDriver->setStoppingParameters(stPoint,(*stopPointItr).duration);
							stopPointEntities.erase(stopPointItr);
							//remove stop point from list
							return true;
						}
					//}
				}
				stopPointItr++;
				index++;
			}
			return false;
		 }

		bool  TrainMovement::getToMove()
		{
			return toMove;
		}

		double TrainMovement::getCurrentSpeed()
		{
			TrainUpdateParams& params = parentDriver->getParams();
			return params.currentSpeed;
		}

		double TrainMovement::getSafeHeadWay()
		{
			return safeHeadway;
		}

		void  TrainMovement::setToMove(bool toMove)
		{
			this->toMove=toMove;
		}

		void TrainMovement::setNoMoveTimeslice(int ts)
		{
			noMoveTimeSlice=ts;
		}

		void TrainMovement::setUserSpecifiedTimeToTakeUturn(double time)
		{
			userSpecifiedUturnTime = time;
		}

		void TrainMovement::frame_tick()
		{

			produceMoveInfo();
			TrainUpdateParams& params = parentDriver->getParams();
			parentDriver->updatePassengers();
			TrainDriver::TRAIN_NEXTREQUESTED requested = parentDriver->getNextRequested();
			if(!getParentDriver()->isStoppedAtPoint()&&requested==TrainDriver::REQUESTED_WAITING_LEAVING)
			{
				isStopPointPresent();
			}
			if(getParentDriver()->isStoppedAtPoint())
			{
				params.currentSpeed = 0.0;
				params.currentAcelerate = 0.0;
				double remainingTime=parentDriver->reduceStoppingTime(params.secondsInTick);
				if(remainingTime<params.secondsInTick)
				{
					parentDriver->setStoppingTime(0);
					parentDriver->setStoppingStatus(false);
					if(requested!=TrainDriver::REQUESTED_WAITING_LEAVING)
					return;
				}
			}
			if(parentDriver->getTrainId()==1)
			{
				int d=8;
			}
			if(!toMove&&params.now.ms()==noMoveTimeSlice)
			{
				toMove=true;
			}

			else
			{
			switch(requested)
			{
				case TrainDriver::REQUESTED_TO_PLATFROM:
				{
					if(!parentDriver->isStoppedAtPoint())
					{
						bool isToBeRemoved=false;
						updatePlatformsList(isToBeRemoved);
						if(isToBeRemoved)
						{
							parentDriver->setIsToBeRemoved(isToBeRemoved);
						}
						Platform *nextPlatformAccordingToPosition=trainPlatformMover_accpos.getNextPlatform(false);
						if(parentDriver->getTrainId()==2 && boost::iequals(getNextPlatform()->getPlatformNo(),"NE14_2"))
						{
							int debug=1;
						}
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
						if(parentDriver->getTrainId()==4)
						{
							int debug=1;
						}
						if(parentDriver->getSubsequentNextRequested()!=TrainDriver::NO_REQUESTED)
						{
							parentDriver->setNextRequested(parentDriver->getSubsequentNextRequested());
							isDisruptedState =false;
							isStrandedBetweenPlatforms_DisruptedState =false;
							parentDriver->setSubsequentNextRequested(TrainDriver::NO_REQUESTED);
						}

						else if(isDisruptedState&&!isStrandedBetweenPlatforms_DisruptedState&&!parentDriver->getUTurnFlag())
						{

							if(shouldStopDueToDisruption(parentDriver)&&!isUTurnPlatformOnTheWay())
							{
								bool isUturnPlatform=TrainController<Person_MT>::getInstance()->isUturnPlatform(getNextPlatform()->getPlatformNo(),parentDriver->getTrainLine());
								if(!isUturnPlatform)
								{
									if(!parentDriver->hasForceAlightedInDisruption())
									{
										parentDriver->setNextRequested(TrainDriver::REQUESTED_LEAVING_PLATFORM);
									}
								}
							}
							else if(!shouldStopDueToDisruption(parentDriver))
							{
								parentDriver->setNextRequested(TrainDriver::REQUESTED_LEAVING_PLATFORM);
								isDisruptedState =false;
							}

						}

						else if(isStrandedBetweenPlatforms_DisruptedState)
						{
							if(!shouldStopDueToDisruption(parentDriver))
							{
								isDisruptedState=false;
								isStrandedBetweenPlatforms_DisruptedState=false;
								//move train ahead
								parentDriver->setNextRequested(TrainDriver::REQUESTED_TO_PLATFROM);

							}
						}

						else if(parentDriver->getUTurnFlag())
						{
							if(shouldStopDueToDisruption(parentDriver)&&!isUTurnPlatformOnTheWay())
							{
								parentDriver->setUturnFlag(false);
								parentDriver->setNextRequested(TrainDriver::REQUESTED_TO_PLATFROM);
							}
						}

						else
						{
							bool isDisruptedPlatform = false;
							bool takeUturn=false;
							//disruption by params
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

									if(parentDriver->getUTurnFlag())
									{
										if(!parentDriver->hasForceAlightedInDisruption())
										{
											parentDriver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
										}
										else
										{
											takeUturn=true;

										}
									}
									else
									{
										//check for Uturn platform
										bool shldStop=shouldStopDueToDisruption(parentDriver);
										if(shldStop&&!isUTurnPlatformOnTheWay())
										{
											bool isUturnPlatform=TrainController<Person_MT>::getInstance()->isUturnPlatform(getNextPlatform()->getPlatformNo(),parentDriver->getTrainLine());
											if(isUturnPlatform)
											{
												if(!parentDriver->hasForceAlightedInDisruption())
												{
													parentDriver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
												}
												else
												{
													takeUturn=true;
													parentDriver->setUturnFlag(true);;
												}
											}

											else
											{
												if(!parentDriver->hasForceAlightedInDisruption())
												{
													parentDriver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
													isDisruptedState = true;
													isDisruptedPlatform = true;
												}
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
										if(parentDriver->getPassengers().size()>0)
										{
											parentDriver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
										}
										else
										{
											parentDriver->setNextRequested(TrainDriver::REQUESTED_LEAVING_PLATFORM);
											parentDriver->getParent()->setToBeRemoved();
											arrivalAtEndPlatform();
										}
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

											//for uturn
											if(takeUturn)
											{
												isDisruptedState=false;
												Platform *platform=getNextPlatform();
												std::string stationNo=platform->getStationNo();
												parentDriver->setNextRequested(TrainDriver::REQUESTED_TAKE_UTURN);
												Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
												waitingTimeRemainingBeforeUturn=userSpecifiedUturnTime;
												isWaitingForUturn =true;
												if(waitingTimeRemainingBeforeUturn<=5)
												{
													messaging::MessageBus::PostMessage(stationAgent,TRAIN_MOVE_AT_UTURN_PLATFORM,
															messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver,true)));
												}
											   //pass message for Uturn

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

									else
									{
										if(!parentDriver->isStoppedAtPoint())
										{
											if(parentDriver->getTrainId()==4)
											{
												int debug=1;
											}
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
					if(!isWaitingForUturn)
					{
						waitingTimeRemainingBeforeUturn=userSpecifiedUturnTime;
						isWaitingForUturn =true;
					}
					else
					{
						waitingTimeRemainingBeforeUturn=waitingTimeRemainingBeforeUturn-5;
					}

					if(waitingTimeRemainingBeforeUturn<=5)
					{
						;
						Platform *platform=getNextPlatform();
						std::string stationNo=platform->getStationNo();
						Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
						messaging::MessageBus::PostMessage(stationAgent,TRAIN_MOVE_AT_UTURN_PLATFORM,
										messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver,true)));
					}

					break;
				}
			}
			}
		}

		/*bool TrainMovement::shouldStopDueToDisruption()
		{
			TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
			typename  std::vector <Role<Person_MT>*> activeTrains = trainController->getActiveTrainsForALine(parentDriver->getTrainLine());
			std::vector <Role<Person_MT>*>::iterator itr= activeTrains.begin();
			for(;itr!=activeTrains.end();itr++)
			{
				TrainDriver *driver=dynamic_cast<TrainDriver*>(*itr);
				int offset =1;
				Platform *nextPlt=driver->getMovement()->getPlatformByOffset(offset);
				int noOfTrainsWithSameNextPlat=0;
				while(true)
				{
					TrainDriver *nextDriver=driver->getNextDriver();
					if(nextDriver)
					{
						if(nextDriver->getNextPlatform() == nextPlt)
						{
							//nextPlt is a uturn platform then return false

							//check if it is one platform before disruption
							noOfTrainsWithSameNextPlat++;
							continue;
						}
						else
						{
							offset++;
							Platform *nextplt=driver->getMovement()->getPlatformByOffset(offset);
							if(nextDriver->getNextPlatform() == nextPlt)
							{
								//nextPlt is a uturn platform then return false

								//check if it is one platform before disruption
								if()
								continue;
							}
							else
							{
								//check if that many number of platforms are empty before disrupted platforms
								int k =offset+1;
								int n=0;
								while()
								{
									Platform *kthPlatform = driver->getMovement()->getPlatformByOffset(k);
									Platform *k1thPlatform=trainController->getNextPlatform(kthPlatform,driver->getTrainLine());
									//if(kthPlatform) is in disrupted region
									{
										k++;
										n++;

									}
									//else
									{
										k++;
										n++;
									}
								}

								return false;
							}
						}
					}
					else
					{
						return false;
					}

				}
			}
		}*/

		bool TrainMovement::isUTurnPlatformOnTheWay()
		{
			Platform *nextplatformPlt=parentDriver->getNextPlatform();
			std::string line=parentDriver->getTrainLine();
			TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
			std::map<std::string,std::vector<std::string>> uTurnPlatforms=trainController->getUturnPlatforms();
			std::vector<std::string> uTurnPlatformList=uTurnPlatforms[line];
			while(nextplatformPlt!=nullptr)
			{

				TrainPlatform nextTrainplatformPlt=trainController->getNextPlatform(nextplatformPlt->getPlatformNo(),line);
				if(std::find(uTurnPlatformList.begin(),uTurnPlatformList.end(),nextTrainplatformPlt.platformNo)!=uTurnPlatformList.end())
				{
					return true;
				}
				nextplatformPlt=trainController->getPlatformFromId(nextTrainplatformPlt.platformNo);
			}
			return false;
		}

		bool TrainMovement::shouldStopDueToDisruption(TrainDriver *aheadDriver)
		{

			TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
			std::string trainLine=parentDriver->getTrainLine();
			std::map<std::string,std::vector<std::string>> disruptedPlatformsMap=trainController->getDisruptedPlatforms_ServiceController();
			if(disruptedPlatformsMap.size()==0||disruptedPlatformsMap.find(trainLine)!=disruptedPlatformsMap.end())
			{
				return false;
			}

			std::string maxPlatform=shouldTrainAheadStopDueToDisruption(aheadDriver);
			if(trainController->isPlatformBeforeAnother(aheadDriver->getNextPlatform()->getPlatformNo(),maxPlatform,aheadDriver->getTrainLine()))
			{
				return false;
			}
			return true;
		}


		std::string TrainMovement::shouldTrainAheadStopDueToDisruption(TrainDriver *driver)
		{
			TrainDriver *aheadDriver=driver->getNextDriver();
			TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
			std::map<std::string,std::vector<std::string>> disruptedPlatforms=trainController->getDisruptedPlatforms_ServiceController();
			std::vector<std::string> platforms=disruptedPlatforms[parentDriver->getTrainLine()];
			std::string firstDisruptedPlatfrom =platforms[0];
			if(aheadDriver)
			{

				std::map<std::string,std::vector<std::string>> disruptedPlatforms=trainController->getDisruptedPlatforms_ServiceController();
				std::vector<std::string> platforms=disruptedPlatforms[parentDriver->getTrainLine()];
				std::string firstDisruptedPlatfrom =platforms[0];
				Platform *aheadDriverPlatform=aheadDriver->getNextPlatform();
				bool isPlatformBefore=trainController->isPlatformBeforeAnother(aheadDriverPlatform->getPlatformNo(),firstDisruptedPlatfrom,parentDriver->getTrainLine());
				if(isPlatformBefore)
				{
					std::string maxPlatform=shouldTrainAheadStopDueToDisruption(aheadDriver);
					if(trainController->isPlatformBeforeAnother(maxPlatform,aheadDriver->getNextPlatform()->getPlatformNo(),parentDriver->getTrainLine()))
					{
						 Platform *prePlatform=trainController->getPrePlatform(aheadDriver->getNextPlatform()->getPlatformNo(),parentDriver->getTrainLine());
						 return prePlatform->getPlatformNo();
					}
					else
					{
						trainController->getPrePlatform(maxPlatform,parentDriver->getTrainLine());
					}

				}
				else
				{
					return firstDisruptedPlatfrom;
				}

			}
			return firstDisruptedPlatfrom;
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

		bool TrainMovement::isStationCase_1(double disToTrain, double disToPlatform,double& effectDis)
		{
			bool res = false;
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

		void TrainMovement::eraseStopPoint(std::vector<PolyPoint>::iterator pointItr)
		{
			std::vector<StopPointEntity> &stopPoints=parentDriver->getStopPoints();
			std::vector<StopPointEntity>::iterator stopPointItr=stopPoints.begin();
			while(stopPointItr!=stopPoints.end())
			{
				if(((*stopPointItr).point.getX()==(*pointItr).getX())&&((*stopPointItr).point.getY()==(*pointItr).getY())&&((*stopPointItr).point.getZ()==(*pointItr).getZ()))
				{
					stopPoints.erase(stopPointItr);
					return;
				}
			}
		}

		void TrainMovement::findNearestStopPoint(std::vector<StopPointEntity> &stopPoints,double &distance)
		{
			std::vector<StopPointEntity>::iterator stopPointItr=stopPoints.begin();
			double minDis=0;
			StopPointEntity stopPoint;
			while(stopPointItr!=stopPoints.end())
			{
				double discovered=getTotalCoveredDistance();
				distance=(*stopPointItr).distance-discovered;
				if(distance>0&&(minDis==0||(distance<minDis)))
				{
					stopPoint=(*stopPointItr);
					minDis = distance;
				}
				stopPointItr++;
			}
			distance = minDis;
		}

		double TrainMovement::getRealSpeedLimit()
		{
			TrainUpdateParams& params = parentDriver->getParams();
			if(parentDriver->getTrainId()==1&&((int)getTotalCoveredDistance())==11)
			{
				int x=0;
			}
			bool stationCaseDecisionConfirmed=false;
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
			while(!stationCaseDecisionConfirmed)
			{
				std::vector<StopPointEntity> &stopPoints=parentDriver->getStopPoints();
				std::vector<StopPointEntity>::iterator itr=stopPoints.begin();
				std::vector<PolyPoint> points;
				while(itr!=stopPoints.end())
				{
					points.push_back((*itr).point);
					itr++;
				}

				std::vector<PolyPoint>::iterator stopPointItr=points.end();
				if(stopPoints.size()!=0)
				{

					stopPointItr=trainPathMover.findNearestStopPoint(points);
					findNearestStopPoint(stopPoints,disToNextStopPoint);
					//std::vector<PolyPoint>::const_iterator currStopPointItr=points.end();;
					//currStopPointItr=trainPathMover.GetCurrentStopPoint();
					//disToNextStopPoint=trainPathMover.calcDistanceBetweenTwoPoints(currStopPointItr,stopPointItr,points);
					if(disToNextStopPoint!=0)
					{
						//disToNextStopPoint=disToNextStopPoint-trainPathMover.getDistanceMoveToNextPoint();
						int debug = 1;
					}
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
						if(distanceToNextObject==params.distanceToNextStopPoint)
						{
							if(decelerate>params.maxDecelerationToStopPoint)
							{
								decelerate = params.maxDecelerationToStopPoint;
							}
						}
						speedLimit = std::sqrt(2.0 * decelerate * distanceToNextObject);
						if (params.currentSpeed > speedLimit)
						{
							if(distanceToNextObject==params.distanceToNextStopPoint)
							{
								if((params.currentSpeed*params.currentSpeed)/(2*distanceToNextObject)<=params.maxDecelerationToStopPoint)
								{
									params.currCase = TrainUpdateParams::STATION_CASE;
									params.currentSpeedLimit = speedLimit;
									return distanceToNextObject;
								}
								else
								{
									//erase stop point
									eraseStopPoint(stopPointItr);
								}
							}
							else
							{
								params.currCase = TrainUpdateParams::STATION_CASE;
								params.currentSpeedLimit = speedLimit;
								return distanceToNextObject;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
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
				if(distanceToNextObject==params.distanceToNextStopPoint&&distanceToNextObject!=params.disToNextPlatform)
				{
					if(decelerate>params.maxDecelerationToStopPoint)
					{
						decelerate=params.maxDecelerationToStopPoint;
					}
				}
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
			//std::vector<std::string>::iterator it=std::find(disruptedPlatformNames.begin(),disruptedPlatformNames.end(),platform->getPlatformNo());
			//if(it!=disruptedPlatformNames.end())
			//{
			TrainDriver *nextDriver=parentDriver->getNextDriver();
			if(nextDriver)
			{
				if(nextDriver->getMovement()->getDisruptedState()&&(nextDriver->getNextRequested()==TrainDriver::REQUESTED_AT_PLATFORM||nextDriver->getNextRequested()==TrainDriver::REQUESTED_WAITING_LEAVING)&&nextDriver->getNextPlatform()==getNextPlatform())
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
			//}
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


		bool TrainMovement::updatePlatformsList(bool &isToBeRemoved)
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
								setToMove(false);
								TrainUpdateParams& params = parentDriver->getParams();
								setNoMoveTimeslice(params.now.ms());
								messaging::MessageBus::PostMessage(trainStationAgent,TRAIN_MOVETO_NEXT_PLATFORM,
										messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver,true)));
								Agent *stationAgentOld=TrainController<sim_mob::medium::Person_MT>::getInstance()->getAgentFromStation(oldPlatform->getStationNo());
								TrainStationAgent *trainStationAgentOld = dynamic_cast<TrainStationAgent*>(stationAgentOld);
								std::list<TrainDriver*> &trainsoldSt=trainStationAgentOld->getTrains();
								std::list<TrainDriver*>::iterator oldStTrainListItr=std::find(trainsoldSt.begin(),trainsoldSt.end(),parentDriver);
								if(oldStTrainListItr!=trainsoldSt.end())
								{
									isToBeRemoved = true;
								}
							}
						}
					}
				}
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
					parentDriver->setForceAlightStatus(false);
					if(parentDriver->getTrainId()==4)
					{
						int debug = 1;
					}
					isWaitingForUturn = false;
					waitingTimeRemainingBeforeUturn=0;
					userSpecifiedUturnTime=0;
					parentDriver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
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

			TrainRemoval *trainRemovalInstance=TrainRemoval::getInstance();
			trainRemovalInstance->addToTrainRemovalList(parentDriver);
			//std::vector<Platform*>::const_iterator it;

			/*const std::vector<Platform*>& prevs = trainPlatformMover.getPrevPlatforms();
			for (it = prevs.begin(); it != prevs.end(); it++)
			{
				Platform* prev = (*it);
				std::string stationNo = prev->getStationNo();
				Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
				//messaging::MessageBus::PostMessage(stationAgent,TRAIN_ARRIVAL_AT_ENDPOINT,
						//messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver)));
			}*/
		}
	}
} /* namespace sim_mob */
