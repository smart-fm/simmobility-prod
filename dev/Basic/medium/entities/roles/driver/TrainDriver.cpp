/*
 * TrainDriver.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */

#include "entities/roles/driver/TrainDriver.hpp"
#include "TrainDriverFacets.hpp"
#include "util/Utils.hpp"
#include "entities/PT_Statistics.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/misc/TrainTrip.hpp"
#include "behavioral/ServiceController.hpp"
#include "entities/incident/IncidentManager.hpp"
#include "event/args/ReRouteEventArgs.hpp"
#include "entities/TrainController.hpp"
namespace sim_mob
{

namespace medium
{
int TrainDriver::counter=0;
long TrainDriver::boardPassengerCount = 0;
TrainDriver::TrainDriver(Person_MT* parent,
		sim_mob::medium::TrainBehavior* behavior,
		sim_mob::medium::TrainMovement* movement,
		std::string roleName, Role<Person_MT>::Type roleType) :
	sim_mob::Role<Person_MT>::Role(parent, behavior, movement, roleName, roleType),
	nextDriver(nullptr),nextRequested(NO_REQUESTED),subsequent_nextRequested(NO_REQUESTED),waitingTimeSec(0.0),initialDwellTime(0.0),disruptionParam(nullptr),platSequenceNumber(0)
{
	int trainId = getTrainId();
	std::string lineId = getTrainLine();
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	std::map<const std::string,TrainProperties> trainLinePropertiesMap = config.trainController.trainLinePropertiesMap;
	TrainProperties trainProperties = trainLinePropertiesMap[lineId];
	maxCapacity = trainProperties.maxCapacity;
	ServiceController::getInstance()->insertTrainIdAndTrainDriverInMap(trainId,lineId,this);
}

TrainDriver::~TrainDriver()
{

}

void TrainDriver::onParentEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args)
{
	switch(eventId)
	{
		case EVT_DISRUPTION_STATION:
		{
			const event::DisruptionEventArgs& exArgs = MSG_CAST(event::DisruptionEventArgs, args);
			const DisruptionParams& disruption = exArgs.getDisruption();
			disruptionParam.reset(new DisruptionParams(disruption));
			break;
		}
	}
}

bool TrainDriver::operator< (TrainDriver * &other)
{
	TrainMovement *movement=getMovement();
	if(movement)
	{
		double totalDistance=movement->getTotalCoveredDistance();
		TrainMovement *otherMovement=other->getMovement();
		if(totalDistance<otherMovement->getTotalCoveredDistance())
		{
			return true;
		}
	}
	return false;
}

void TrainDriver::setForceAlightStatus(bool status)
{
	isForceAlighted = status;
}

bool TrainDriver::getForceAlightStatus() const
{
	return isForceAlighted;
}

Role<Person_MT>* TrainDriver::clone(Person_MT *parent) const
{
	TrainBehavior* behavior = new TrainBehavior();
	if(parent)
	{
		std::string lineId;
		std::vector<TripChainItem *>::iterator currTrip = parent->currTripChainItem;
		const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*currTrip);
		if(trip)
		{
			lineId = trip->getLineId();
		}

		TrainMovement* movement = new TrainMovement(lineId);
		TrainDriver* driver = new TrainDriver(parent, behavior, movement, "TrainDriver_");
		behavior->setParentDriver(driver);
		movement->setParentDriver(driver);
		return driver;
	}
}

void TrainDriver::setNextDriver(TrainDriver* driver)
{
	nextDriver = driver;
}

TrainDriver* TrainDriver::getNextDriver() const
{
	return nextDriver;
}

double TrainDriver::reduceStoppingTime(double secondsinTick)
{
	remainingStopTime = remainingStopTime-secondsinTick;
	return remainingStopTime;
}

void TrainDriver::setTerminateTrainService(bool terminate)
{
	terminateTrainServiceLock.lock();
	shouldTerminateService = terminate;
	terminateTrainServiceLock.unlock();
}

bool TrainDriver::getTerminateStatus() const
{
	terminateTrainServiceLock.lock();
	bool terminateservice = shouldTerminateService;
	terminateTrainServiceLock.unlock();
	return terminateservice;
}

void TrainDriver::setStoppingTime(double secondsinTick)
{
	remainingStopTime = secondsinTick;
}

void TrainDriver::setStoppingStatus(bool status)
{
	stoppedAtPoint = status;
}

void TrainDriver::clearStopPoints()
{
	stopPointEntities.clear();
}

void TrainDriver::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
	if(disruptionParam.get())
	{
		DailyTime duration = disruptionParam->duration;
		unsigned int baseGran = ConfigManager::GetInstance().FullConfig().baseGranMS();
		if(duration.getValue()>baseGran)
		{
			disruptionParam->duration = DailyTime(duration.offsetMS_From(DailyTime(baseGran)));
		}
		else
		{
			disruptionParam.reset();
		}
	}
}

std::vector<BufferedBase*> TrainDriver::getSubscriptionParams()
{
	return std::vector<BufferedBase*>();
}
void TrainDriver::leaveFromCurrentPlatform()
{
	TrainMovement* movement = dynamic_cast<TrainMovement*>(movementFacet);
	if(movement)
	{
		movement->leaveFromPlaform();
	}
}

TrainMovement* TrainDriver::getMovement() const
{
	TrainMovement* movement = dynamic_cast<TrainMovement*>(movementFacet);
	return movement;
}

void TrainDriver::setTrainDriverInOpposite(TrainDriver *trainDriver)
{
	nextDriverInOppLine = trainDriver;
}

bool TrainDriver::isStoppedAtPoint()
{
	return stoppedAtPoint;
}
TrainDriver * TrainDriver::getDriverInOppositeLine() const
{
    return nextDriverInOppLine;
}

TrainDriver::TRAIN_NEXTREQUESTED TrainDriver::getNextRequested() const
{
	driverMutex.lock();
	TRAIN_NEXTREQUESTED next = nextRequested;
	driverMutex.unlock();
	return next;
}

void TrainDriver::setNextRequested(TRAIN_NEXTREQUESTED res)
{
	counter++;
	driverMutex.lock();
	nextRequested = res;
	driverMutex.unlock();
}

void TrainDriver::getMovementMutex()
{
	movementMutex.lock();
}

void TrainDriver::movementMutexUnlock()
{
	movementMutex.unlock();
}

int TrainDriver::getInitialNumberOfPassengers()
{
	return initialnumberofpassengers;
}

void TrainDriver::setInitialNumberOfPassengers(int initialnumberofpassengers)
{
	this->initialnumberofpassengers = initialnumberofpassengers;
}

void TrainDriver::calculateDwellTime(int boarding,int alighting,int noOfPassengerInTrain,timeslice now,bool forceAlightwhileWaiting)
{
	const std::string& fileName("pt_mrt_Boarding_Alighting_DwellTime.csv");
	sim_mob::BasicLogger& ptMRTMoveLogger = sim_mob::Logger::log(fileName);
	std::string tm = (DailyTime(now.ms())+DailyTime(ConfigManager::GetInstance().FullConfig().simStartTime())).getStrRepr();
	TrainController<sim_mob::medium::Person_MT> *trainController = TrainController<sim_mob::medium::Person_MT>::getInstance();
	Platform *platform = getMovement()->getNextPlatform();
	std::string stationNo = platform->getStationNo();
	Station *station = trainController->getStationFromId(stationNo);
	const std::vector<double> personCountCoefficients = trainController->getNumberOfPersonsCoefficients(station,platform);
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	const std::map<const std::string,TrainProperties>& trainLinePropertiesMap = config.trainController.trainLinePropertiesMap;
	const TrainProperties &trainProperties = trainLinePropertiesMap.find(getTrainLine())->second;
	const TrainDwellTimeInfo &dwellTimeInfo = trainProperties.dwellTimeInfo;
	double dwellTime = -1;
	if(personCountCoefficients.size() == 3)
	{
		noOfPassengerInTrain = noOfPassengerInTrain*personCountCoefficients[2];
		dwellTime = dwellTimeInfo.firstCoeff + (dwellTimeInfo.secondCoeff*personCountCoefficients[0]*boarding + dwellTimeInfo.thirdCoeff*personCountCoefficients[1]*alighting + dwellTimeInfo.fourthCoeff*personCountCoefficients[2]*noOfPassengerInTrain)/24;
	}
	else
	{
		dwellTime = dwellTimeInfo.firstCoeff + (dwellTimeInfo.secondCoeff*boarding + dwellTimeInfo.thirdCoeff*alighting + dwellTimeInfo.fourthCoeff*noOfPassengerInTrain)/24;
	}
		
	//double dwellTime = 12.22 + 2.27*boarding/24 + 1.82*alighting/24; //+ 0.00062*(noOfPassengerInTrain/24)*(noOfPassengerInTrain/24)*(noOfPassengerInTrain/24)*(boarding/24);
	Platform *currentPlatform=getMovement()->getNextPlatform();
	if(currentPlatform)
	{
		std::string trainLine=getTrainLine();
		double maxDwellTime=0.0,minDwellTime=0.0;
		bool useMaxDwellTime = false,useMinDwellTime = false;
		if(platformHoldingTimeEntities.find(platform->getPlatformNo()) != platformHoldingTimeEntities.end())
		{
			dwellTime = platformHoldingTimeEntities[platform->getPlatformNo()];
		}

		else
		{

			if(platformMaxHoldingTimeEntities.find(platform->getPlatformNo()) != platformMaxHoldingTimeEntities.end())
			{
				maxDwellTime = platformMaxHoldingTimeEntities[platform->getPlatformNo()];
				useMaxDwellTime = true;
			}
			else
			{
				maxDwellTime = trainController->getMaximumDwellTime(trainLine);
			}

			if(platformMinHoldingTimeEntities.find(platform->getPlatformNo()) != platformMinHoldingTimeEntities.end())
			{
				minDwellTime = platformMinHoldingTimeEntities[platform->getPlatformNo()];
				useMinDwellTime = true;
			}
			else
			{
				minDwellTime = trainController->getMinDwellTime(stationNo,trainLine);
			}

			if(useMaxDwellTime)
			{
				if(dwellTime>maxDwellTime)
				{
					dwellTime = maxDwellTime;
				}

				if(dwellTime < minDwellTime&&minDwellTime <= maxDwellTime)
				{
					dwellTime = minDwellTime;
				}
				else if(dwellTime < minDwellTime && minDwellTime > maxDwellTime)
				{
					dwellTime = maxDwellTime;
				}

			}

			else if (useMinDwellTime)
			{
				if(dwellTime < minDwellTime)
				{
					dwellTime = minDwellTime;
				}

				if(dwellTime > maxDwellTime && maxDwellTime >= minDwellTime)
				{
					dwellTime = maxDwellTime;
				}
				else if (dwellTime > maxDwellTime && maxDwellTime < minDwellTime)
				{
					dwellTime = minDwellTime;
				}
			}

			else
			{
				if(dwellTime < minDwellTime)
				{
					dwellTime = minDwellTime;
				}
				else if (dwellTime > maxDwellTime)
				{
					dwellTime = maxDwellTime;
				}
			}
		}
	}

	if(forceAlightwhileWaiting)
	{
		dwellTime = dwellTime - (initialDwellTime - waitingTimeSec);
		minDwellTimeRequired = dwellTime; /*minDwellTimeRequired is the min dwell time calculated so that all passengers can board and alight */
		waitingTimeSec = dwellTime;

	}
	else
	{
		initialDwellTime = dwellTime;
		minDwellTimeRequired = dwellTime; /*minDwellTimeRequired is the min dwell time calculated so that all passengers can board and alight */
		waitingTimeSec = dwellTime;
	}


	ptMRTMoveLogger<<getTrainId()<<","<<tm<<","<<boarding<<","<<alighting<<","<<noOfPassengerInTrain<<","<<initialDwellTime<<std::endl;
}

void TrainDriver::insertPlatformHoldEntities(std::string platformName,double duration)
{
	platformHoldingTimeEntitiesLock.lock();
	if(platformMaxHoldingTimeEntities.find(platformName) != platformMaxHoldingTimeEntities.end())
	{
		if(duration > platformMaxHoldingTimeEntities[platformName])
		{
			return;
		}
	}
	if(platformMinHoldingTimeEntities.find(platformName) != platformMinHoldingTimeEntities.end())
	{
		if(duration < platformMinHoldingTimeEntities[platformName])
		{
			return;
		}
	}
	platformHoldingTimeEntities[platformName] = duration;
	platformHoldingTimeEntitiesLock.unlock();
}


void TrainDriver::resetHoldingTime()
{
	platformHoldingTimeEntitiesLock.lock();
	Platform *currentPlatform = getMovement()->getNextPlatform();
	if(platformHoldingTimeEntities.find(currentPlatform->getPlatformNo()) != platformHoldingTimeEntities.end())
	{
		double holdingTime = platformHoldingTimeEntities[currentPlatform->getPlatformNo()];
		waitingTimeSec = holdingTime - (initialDwellTime - waitingTimeSec);
		initialDwellTime = holdingTime;
		platformHoldingTimeEntities.erase(currentPlatform->getPlatformNo());
		isHoldingTimeReset = true;
	}
	platformHoldingTimeEntitiesLock.unlock();

}

void TrainDriver::resetMaximumHoldingTime(std::string platformName,double duration)
{
	platformMaxHoldingTimeEntities[platformName] = duration;
}

void TrainDriver::resetMinimumHoldingTime(std::string platformName,double duration)
{
	platformMinHoldingTimeEntities[platformName] = duration;
}

double TrainDriver::getWaitingTime() const
{
	return waitingTimeSec;
}

void TrainDriver::reduceWaitingTime(double val)
{
	waitingTimeSec -= val;
}
void TrainDriver::setWaitingTime(double val)
{
	waitingTimeSec = val;
}

std::vector<std::string> TrainDriver::getPlatformsToBeIgnored()
{

	platformsToBeIgnoredLock.lock();
	std::vector<std::string> platforms = platformsToBeIgnored;
	platformsToBeIgnoredLock.unlock();
	return platforms;
}

void TrainDriver::AddPlatformsToIgnore(std::vector<std::string> PlatformsToIgnore)
{
	std::vector<std::string>::iterator it = PlatformsToIgnore.begin();
	platformsToBeIgnoredLock.lock();
	while(it!=PlatformsToIgnore.end())
	{
		std::vector<std::string>::iterator findelement = std::find(platformsToBeIgnored.begin(),platformsToBeIgnored.end(),*it);
		if(findelement == platformsToBeIgnored.end())
		{
			platformsToBeIgnored.push_back((*it));
		}
		it++;
	}
	platformsToBeIgnoredLock.unlock();
}

void TrainDriver::AddPlatforms(std::vector<std::string> PlatformsToAdd)
{
	std::vector<std::string>::iterator it = PlatformsToAdd.begin();
	platformsToBeIgnoredLock.lock();
	while(it!=PlatformsToAdd.end())
	{
		std::vector<std::string>::iterator findelement=std::find(platformsToBeIgnored.begin(),platformsToBeIgnored.end(),*it);
		if(findelement != platformsToBeIgnored.end())
		{
			platformsToBeIgnored.erase(findelement);
			TrainMovement *movement = getMovement();
			if(movement)
			{
				movement->setShouldIgnoreAllPlatforms(false);
			}
		}
		it++;
	}
	platformsToBeIgnoredLock.unlock();
}
std::string TrainDriver::getTrainLine() const
{
	std::string lineId;
	if(getParent())
	{
		std::vector<TripChainItem *>::iterator currTrip = getParent()->currTripChainItem;
		const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*currTrip);
		if(trip)
		{
			lineId = trip->getLineId();
		}
	}

	return lineId;
}

int TrainDriver::getTripId() const
{
	int id = 0;
	if(getParent())
	{
		std::vector<TripChainItem *>::iterator currTrip = getParent()->currTripChainItem;
		const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*currTrip);
		if(trip)
		{
			id = trip->getTripId();
		}
	}
	return id;
}


const TrainTrip* TrainDriver::getTrainTrip() const
{
	if(getParent())
	{
		std::vector<TripChainItem *>::iterator currTrip = getParent()->currTripChainItem;
		const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*currTrip);
		return trip ;
	}
}

int TrainDriver::getTrainId() const
{
	int id = 0;
	if(getParent())
	{
		std::vector<TripChainItem *>::iterator currTrip = getParent()->currTripChainItem;
		const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*currTrip);
		if(trip)
		{
			id = trip->getTrainId();
		}
	}
	return id;
}

Platform* TrainDriver::getNextPlatform() const
{
	Platform* platform = nullptr;
	TrainMovement* movement = dynamic_cast<TrainMovement*>(movementFacet);
	if(movement)
	{
		platform = movement->getNextPlatform();
	}
	return platform;
}


std::list<Passenger*>& TrainDriver::getPassengers()
{
	return passengerList;
}
unsigned int TrainDriver::getEmptyOccupation()
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	if(passengerList.size()<maxCapacity)
	{
		return maxCapacity-passengerList.size();
	}
	return 0;
}

void TrainDriver::setUturnFlag(bool set)
{
	uTurnFlag = set;
}

bool TrainDriver::getUTurnFlag() const
{
	return uTurnFlag;
}

void TrainDriver::setForceAlightFlag(bool flag)
{
	if(!isForceAlighted||flag == false)
	{
		forceAlightPassengers_ByServiceController = flag;
	}
}

TrainDriver::TRAIN_NEXTREQUESTED TrainDriver::getSubsequentNextRequested()
{
	return subsequent_nextRequested;
}

void TrainDriver::setSubsequentNextRequested(TrainDriver::TRAIN_NEXTREQUESTED nextReq)
{
	 subsequent_nextRequested = nextReq;
}

bool TrainDriver::getIsToBeRemoved()
{
	return isToBeRemovedFromStationAgent;
}

void TrainDriver::setIsToBeRemoved(bool isToBeRemoved)
{
	isToBeRemovedFromStationAgent = isToBeRemoved;
}

bool TrainDriver::getForceAlightFlag() const
{
	return forceAlightPassengers_ByServiceController;
}

int TrainDriver::alightPassenger(std::list<Passenger*>& alightingPassenger,timeslice now)
{
	int num = 0;
	if(isAlightingRestricted())
	{
		return num;
	}
	const Platform* platform = this->getNextPlatform();
    if(platform)
    {
    	std::string ptName=platform->getPlatformNo();
    }

    std::string lineId= platform->getLineId();
	std::list<Passenger*>::iterator i = passengerList.begin();
	std::string tm=(DailyTime(now.ms())+DailyTime(ConfigManager::GetInstance().FullConfig().simStartTime())).getStrRepr();
	sim_mob::BasicLogger& ptMRTLogger  = sim_mob::Logger::log("PersonsAlighting.csv");
	while(i!=passengerList.end())
	{
		const WayPoint& endPoint = (*i)->getEndPoint();
		if(endPoint.type == WayPoint::MRT_PLATFORM)
		{
			if(endPoint.platform == platform)
			{
				alightingPassenger.push_back(*i);
				ptMRTLogger <<(*i)->getParent()->getDatabaseId()<<","<<tm<<","<<getTrainId()<<","<<getTripId()<<","<<platform->getPlatformNo()<<","<<(*i)->getParent()->currSubTrip->origin.platform->getPlatformNo()<<","<<(*i)->getParent()->currSubTrip->destination.platform->getPlatformNo()<<std::endl;
				i = passengerList.erase(i);
				num++;
				continue;
			}

			else
			{
				std::string stationNo = platform->getStationNo();
				std::vector<Platform*> platforms=TrainController<sim_mob::medium::Person_MT>::getInstance()->getPlatforms(lineId,stationNo);
				std::vector<Platform*>::iterator itr=std::find(platforms.begin(),platforms.end(),endPoint.platform);
				if(itr == platforms.end())
				{
					alightingPassenger.push_back(*i);
					ptMRTLogger <<(*i)->getParent()->getDatabaseId()<<","<<tm<<","<<getTrainId()<<","<<getTripId()<<platform->getPlatformNo()<<","<<(*i)->getParent()->currSubTrip->origin.platform->getPlatformNo()<<","<<(*i)->getParent()->currSubTrip->destination.platform->getPlatformNo()<<std::endl;
					i = passengerList.erase(i);
					num++;
					continue;
				}
			}
		}
		else
		{
			throw std::runtime_error("the passenger in train do not know ending platform");
		}
		i++;
	}
	return num;
}

int TrainDriver::alightAllPassengers(std::list<Passenger*>& alightingPassenger,timeslice now)
{
	int num = 0;
	std::list<Passenger*>::iterator i = passengerList.begin();
	while(i!=passengerList.end())
	{
		alightingPassenger.push_back(*i);
		i = passengerList.erase(i);
		num++;
	}

	return num;
}

void TrainDriver::updatePassengers()
{
	for (std::list<Passenger*>::iterator it = passengerList.begin(); it != passengerList.end(); it++)
	{
		(*it)->Movement()->frame_tick();
	}
}


void TrainDriver::insertStopPoint(PolyPoint point,double duration,double maxDecerationRate,double distance)
{
	StopPointEntity stopPointEntity;
	stopPointEntity.point=point;
	stopPointEntity.duration=duration;
	stopPointEntity.maxDecerationRate=maxDecerationRate;
	stopPointEntity.distance=distance;
	stopPointEntitiesLock.lock();
	stopPointEntities.push_back(stopPointEntity);
	stopPointEntitiesLock.unlock();

}


void TrainDriver::lockUnlockRestrictPassengerEntitiesLock(bool lock)
{
	if(lock)
	{
		restrictEntitiesLock.lock();
	}

	else
	{
		restrictEntitiesLock.unlock();
	}
}

void TrainDriver::insertRestrictPassengerEntity(std::string platformName,int movType)
{

	restrictEntitiesLock.lock();
	std::map<std::string,passengerMovement>::iterator itr = restrictPassengersEntities.find(platformName);
	if(itr != restrictPassengersEntities.end())
	{
		restrictPassengersEntities.erase(itr);
	}
	restrictPassengersEntities[platformName]=passengerMovement(movType);
	restrictEntitiesLock.unlock();
}

std::vector<StopPointEntity>& TrainDriver::getStopPoints()
{
	return stopPointEntities;
}

void TrainDriver::removeAllRestrictPassengersEnties()
{
	restrictPassengersEntities.erase(restrictPassengersEntities.begin(),restrictPassengersEntities.end());
}

void TrainDriver::removeAllMaximumHoldingTimeEntities()
{
	platformMaxHoldingTimeEntities.erase(platformMaxHoldingTimeEntities.begin(),platformMaxHoldingTimeEntities.end());
}

void TrainDriver::removeAllMinimumHoldingTimeEntities()
{
	platformMinHoldingTimeEntities.erase(platformMinHoldingTimeEntities.begin(),platformMinHoldingTimeEntities.end());
}

void TrainDriver::removeAllPlatformHoldingTimeEntities()
{
	platformHoldingTimeEntities.erase(platformHoldingTimeEntities.begin(),platformHoldingTimeEntities.end());
}

void TrainDriver::clearAllPlatformsToIgnore()
{
	platformsToBeIgnored.erase(platformsToBeIgnored.begin(),platformsToBeIgnored.end());
}

void TrainDriver::storeWaitingTime(WaitTrainActivity* waitingActivity, timeslice now) const
{
	if(!waitingActivity) { return; }
	PersonWaitingTime personWaitInfo;
	personWaitInfo.busStopNo = waitingActivity->getStartPlatform()->getPlatformNo();
	personWaitInfo.personId  = waitingActivity->getParent()->getId();
	personWaitInfo.personIddb = waitingActivity->getParent()->getDatabaseId();
	personWaitInfo.originNode = (*(waitingActivity->getParent()->currTripChainItem))->origin.node->getNodeId();
	personWaitInfo.destNode = (*(waitingActivity->getParent()->currTripChainItem))->destination.node->getNodeId();
	personWaitInfo.endstop = waitingActivity->getParent()->currSubTrip->endLocationId;
	personWaitInfo.currentTime = (DailyTime(now.ms())+DailyTime(ConfigManager::GetInstance().FullConfig().simStartTime())).getStrRepr();
	personWaitInfo.waitingTime = ((double) waitingActivity->getWaitingTime())/1000.0; //convert ms to second
	personWaitInfo.busLines = waitingActivity->getTrainLine();
	personWaitInfo.busLineBoarded = waitingActivity->getTrainLine();
	personWaitInfo.deniedBoardingCount = waitingActivity->getDeniedBoardingCount();
	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(), STORE_PERSON_WAITING,
			messaging::MessageBus::MessagePtr(new PersonWaitingTimeMessage(personWaitInfo)));
}
void TrainDriver::setArrivalTime(const std::string& currentTime)
{
	arrivalTimeAtPlatform = currentTime;
}

void TrainDriver::storeArrivalTime(const std::string& currentTime, const std::string& waitTime)
{
	Person_MT* person = parent;
	if (!person)
	{
		return;
	}

	Platform* platform = nullptr;
	TrainMovement* movement = dynamic_cast<TrainMovement*>(movementFacet);
	if(movement)
	{
		platform = movement->getNextPlatform();
	}
	std::vector<TripChainItem *>::iterator currTrip = getParent()->currTripChainItem;
	const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*currTrip);
	if (trip && platform)
	{
		PT_ArrivalTime arrivalInfo;
		arrivalInfo.serviceLine = trip->getLineId();
		arrivalInfo.tripId = boost::lexical_cast<std::string>(trip->getTripId());
		arrivalInfo.sequenceNo = platSequenceNumber++;
		arrivalInfo.arrivalTime = currentTime;
		arrivalInfo.dwellTime = waitTime;
		arrivalInfo.dwellTimeSecs = (DailyTime(waitTime)).getValue() / 1000.0;
		const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
		arrivalInfo.pctOccupancy = (((double)passengerList.size())/maxCapacity) * 100.0;
		arrivalInfo.stopNo = platform->getPlatformNo();
		messaging::MessageBus::PostMessage(PT_Statistics::getInstance(), STORE_BUS_ARRIVAL, messaging::MessageBus::MessagePtr(new PT_ArrivalTimeMessage(arrivalInfo)));
	}
}
int TrainDriver::boardPassenger(std::list<WaitTrainActivity*>& boardingPassenger,timeslice now)
{
	int num = 0;
    if(isBoardingRestricted())
    {
    	return num;
    }

	int validNum = getEmptyOccupation();
	sim_mob::BasicLogger& ptMRTLogger  = sim_mob::Logger::log("PersonsBoarding.csv");
	std::list<WaitTrainActivity*>::iterator i = boardingPassenger.begin();
	while(i!=boardingPassenger.end()&&validNum>0)
	{
		(*i)->collectTravelTime();
		storeWaitingTime((*i), now);
		Person_MT* person = (*i)->getParent();
		person->checkTripChain(now.ms());
		Role<Person_MT>* curRole = person->getRole();
		curRole->setArrivalTime(now.ms()+(ConfigManager::GetInstance().FullConfig().simStartTime()).getValue());
		std::string tm=(DailyTime(now.ms())+DailyTime(ConfigManager::GetInstance().FullConfig().simStartTime())).getStrRepr();
		sim_mob::medium::Passenger* passenger = dynamic_cast<sim_mob::medium::Passenger*>(curRole);
		if(passenger)
		{
			passenger->setArrivalTime(now.ms()+(ConfigManager::GetInstance().FullConfig().simStartTime()).getValue());
			passenger->setStartPoint(person->currSubTrip->origin);
			passenger->setEndPoint(person->currSubTrip->destination);
			passenger->Movement()->startTravelTimeMetric();
			boardPassengerLock.lock();
			boardPassengerCount=boardPassengerCount+1;
			boardPassengerLock.unlock();
			ptMRTLogger<<person->getDatabaseId()<<","<<tm<<","<<getTrainId()<<","<<getTripId()<<",";
			if(getNextPlatform())
			{
				ptMRTLogger<<getNextPlatform()->getPlatformNo();
			}
			else
			{
				ptMRTLogger<<" ";
			}

			ptMRTLogger<<","<<person->currSubTrip->origin.platform->getPlatformNo()<<","<<person->currSubTrip->destination.platform->getPlatformNo()<<std::endl;
			passengerList.push_back(passenger);
			i = boardingPassenger.erase(i);
			validNum--;
			num++;
		}
		else
		{
			throw std::runtime_error("next trip is not passenger in train boarding");
		}
	}
	i = boardingPassenger.begin();
	while(i != boardingPassenger.end())
	{
		(*i)->incrementDeniedBoardingCount();
		i++;
	}
	return num;
}

int TrainDriver::boardForceAlightedPassengersPassenger(std::list<Passenger*>& forcealightedPassengers,timeslice now)
{
	int num = 0;
	if(isBoardingRestricted())
	{
		return num;
	}

	int validNum = getEmptyOccupation();
	std::list<Passenger*>::iterator i = forcealightedPassengers.begin();
	while(i != forcealightedPassengers.end()&&validNum>0)
	{
		Passenger *pr = dynamic_cast<Passenger*>(*i);
		passengerList.push_back(pr);
		i = forcealightedPassengers.erase(i);
		validNum--;
		num++;
	}

    return num;
}


bool TrainDriver::isBoardingRestricted()
{
	std::map<std::string,passengerMovement>::iterator it = restrictPassengersEntities.begin();
	const Platform* platform = this->getNextPlatform();
	if(platform)
	{
		it=restrictPassengersEntities.find(platform->getPlatformNo());
		if(it!= restrictPassengersEntities.end())
		{
			passengerMovement movType=restrictPassengersEntities[platform->getPlatformNo()];
			if(movType == BOARDING || movType == BOTH)
			{
			   restrictPassengersEntities.erase(it);
			   return true;
			}
		}
	}
}

void TrainDriver::setHasForceAlightedInDisruption(bool hasForceAlighted)
{
	hasforceAlightedInDisruption = hasForceAlighted;
}

bool TrainDriver::hasForceAlightedInDisruption()
{
	return hasforceAlightedInDisruption;
}

bool TrainDriver::isAlightingRestricted()
{
	std::map<std::string,passengerMovement>::iterator it=restrictPassengersEntities.begin();
	const Platform* platform = this->getNextPlatform();
	if(platform)
	{
		it=restrictPassengersEntities.find(platform->getPlatformNo());
		if(it!=restrictPassengersEntities.end())
		{
			passengerMovement movType=restrictPassengersEntities[platform->getPlatformNo()];
			restrictPassengersEntitiesLock.unlock();
			if(movType == ALIGHTING || movType == BOTH)
			{
				if(movType == ALIGHTING)
				{
					restrictPassengersEntities.erase(it);
				}
				return true;
			}
		}
	}
}


void TrainDriver::setStoppingParameters(PolyPoint point,double duration)
{
	remainingStopTime = duration;
	currStopPoint = point;
	stoppedAtPoint = true;
}
}
} /* namespace sim_mob */
