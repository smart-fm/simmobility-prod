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
TrainDriver::TrainDriver(Person_MT* parent,
		sim_mob::medium::TrainBehavior* behavior,
		sim_mob::medium::TrainMovement* movement,
		std::string roleName, Role<Person_MT>::Type roleType) :
	sim_mob::Role<Person_MT>::Role(parent, behavior, movement, roleName, roleType),
	nextDriver(nullptr),nextRequested(NO_REQUESTED),subsequent_nextRequested(NO_REQUESTED),waitingTimeSec(0.0),initialDwellTime(0.0),disruptionParam(nullptr),platSequenceNumber(0)
{
	int trainId=getTrainId();
	std::string lineId=getTrainLine();
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
	isForceAlighted=status;
}

bool TrainDriver::getForceAlightStatus()
{
	return isForceAlighted;
}

Role<Person_MT>* TrainDriver::clone(Person_MT *parent) const
{
	TrainBehavior* behavior = new TrainBehavior();
	TrainMovement* movement = new TrainMovement();
	TrainDriver* driver = new TrainDriver(parent, behavior, movement, "TrainDriver_");
	behavior->setParentDriver(driver);
	movement->setParentDriver(driver);
	return driver;
}

void TrainDriver::setNextDriver(TrainDriver* driver)
{
	nextDriver = driver;
}
 TrainDriver* TrainDriver::getNextDriver()
{
	return nextDriver;
}

double TrainDriver::ReduceStoppingTime(double secondsinTick)
{
	remainingStopTime=remainingStopTime-secondsinTick;
	return remainingStopTime;
}

void TrainDriver::SetTerminateTrainService(bool terminate)
{
	terminateTrainServiceLock.lock();
	shouldTerminateService=terminate;
	terminateTrainServiceLock.unlock();
}

bool TrainDriver::GetTerminateStatus()
{
	terminateTrainServiceLock.lock();
	bool terminateservice=shouldTerminateService;
	terminateTrainServiceLock.unlock();
	return terminateservice;
}

void TrainDriver::SetStoppingTime(double secondsinTick)
{
	remainingStopTime=secondsinTick;
}

void TrainDriver::SetStoppingStatus(bool status)
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

std::vector<BufferedBase*> TrainDriver::getSubscriptionParams() {
	return std::vector<BufferedBase*>();
}
void TrainDriver::leaveFromCurrentPlatform()
{
	TrainMovement* movement = dynamic_cast<TrainMovement*>(movementFacet);
	if(movement){
		movement->leaveFromPlaform();
	}
}

TrainMovement* TrainDriver::getMovement()
{
	TrainMovement* movement = dynamic_cast<TrainMovement*>(movementFacet);
	return movement;
}

void TrainDriver::SetTrainDriverInOpposite(TrainDriver *trainDriver)
{
	nextDriverInOppLine = trainDriver;
}

bool TrainDriver::IsStoppedAtPoint()
{
 return stoppedAtPoint;
}
TrainDriver * TrainDriver::GetDriverInOppositeLine()
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

void TrainDriver::calculateDwellTime(int boarding,int alighting,int noOfPassengerInTrain,timeslice now)
{
	const std::string& fileName("pt_mrt_Boarding_Alighting_DwellTime.csv");
	sim_mob::BasicLogger& ptMRTMoveLogger  = sim_mob::Logger::log(fileName);
	std::string tm=(DailyTime(now.ms())+DailyTime(ConfigManager::GetInstance().FullConfig().simStartTime())).getStrRepr();
	double dwellTime = 12.22 + 2.27*boarding/24 + 1.82*alighting/24; //+ 0.00062*(noOfPassengerInTrain/24)*(noOfPassengerInTrain/24)*(noOfPassengerInTrain/24)*(boarding/24);
	Platform *currentPlatform=getMovement()->getNextPlatform();
	if(currentPlatform)
	{
      std::string stationNo=currentPlatform->getStationNo();
      std::string trainLine=getTrainLine();
      TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
      double minDwellTime=trainController->GetMinDwellTime(stationNo,trainLine);
      if(dwellTime<minDwellTime)
      {
    	  dwellTime=minDwellTime;
      }
      else if(dwellTime>120)
      {
    	  dwellTime =120;
      }
	}
	minDwellTimeRequired =dwellTime; /*minDwellTimeRequired is the min dwell time calculated so that all passengers can board and alight */
	waitingTimeSec = dwellTime;
	initialDwellTime=dwellTime;

	ptMRTMoveLogger<<getTrainId()<<","<<tm<<","<<boarding<<","<<alighting<<","<<noOfPassengerInTrain<<","<<initialDwellTime<<std::endl;
}

void TrainDriver::InsertPlatformHoldEntities(std::string platformName,double duration)
{

	std::vector<PlatformHoldingTimeEntity>::iterator it= platformHoldingTimeEntities.begin();
	PlatformHoldingTimeEntity platformHoldEntity;
	platformHoldEntity.pltaformName=platformName;
	platformHoldEntity.holdingTime=duration;
	platformHoldingTimeEntitiesLock.lock();
	it=std::find(it,platformHoldingTimeEntities.end(),platformHoldEntity);
	if(it!=platformHoldingTimeEntities.end())
	{
		platformHoldingTimeEntities.erase(it);

	}
	platformHoldingTimeEntities.push_back(platformHoldEntity);
	platformHoldingTimeEntitiesLock.unlock();

}


void TrainDriver::ResetHoldingTime()
{
	platformHoldingTimeEntitiesLock.lock();
	std::vector<PlatformHoldingTimeEntity>::iterator it=platformHoldingTimeEntities.begin();
	Platform *currentPlatform=getMovement()->getNextPlatform();
	while(it!=platformHoldingTimeEntities.end())
	{
		if(boost::iequals(currentPlatform->getPlatformNo(),(*it).pltaformName))
		{
			double holdingTime=(*it).holdingTime;
			if(holdingTime>minDwellTimeRequired)
			{
				waitingTimeSec=holdingTime-(initialDwellTime-waitingTimeSec);
				initialDwellTime=holdingTime;
			}
			platformHoldingTimeEntities.erase(it);
			break;
		}
		it++;
	}
	platformHoldingTimeEntitiesLock.unlock();

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

std::vector<std::string> TrainDriver::GetPlatformsToBeIgnored()
{

	platformsToBeIgnoredLock.lock();
	std::vector<std::string> platforms=platformsToBeIgnored;
	platformsToBeIgnoredLock.unlock();
	return platforms;
}

void TrainDriver::AddPlatformsToIgnore(std::vector<std::string> PlatformsToIgnore)
{
   std::vector<std::string>::iterator it=PlatformsToIgnore.begin();
   platformsToBeIgnoredLock.lock();
   while(it!=PlatformsToIgnore.end())
   {
	   std::vector<std::string>::iterator findelement=std::find(platformsToBeIgnored.begin(),platformsToBeIgnored.end(),*it);
	   if(findelement==platformsToBeIgnored.end())
		   platformsToBeIgnored.push_back((*it));
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
	if(passengerList.size()<config.trainController.maxCapacity)
	{
		return config.trainController.maxCapacity-passengerList.size();
	}
	return 0;
}

void TrainDriver::SetUnsetUturnFlag(bool set)
{
	uTurnFlag=set;
}

bool TrainDriver::GetUTurnFlag()
{
	return uTurnFlag;
}

void TrainDriver::setForceAlightFlag(bool flag)
{
	if(!isForceAlighted||flag==false)
		forceAlightPassengers_ByServiceController = flag;
}

TrainDriver::TRAIN_NEXTREQUESTED TrainDriver::getSubsequentNextRequested()
{
	return subsequent_nextRequested;
}

void TrainDriver::setSubsequentNextRequested(TrainDriver::TRAIN_NEXTREQUESTED nextReq)
{
	 subsequent_nextRequested = nextReq;
}

bool TrainDriver::getForceAlightFlag()
{
	return forceAlightPassengers_ByServiceController;
}

int TrainDriver::alightPassenger(std::list<Passenger*>& alightingPassenger,timeslice now)
{
	int num = 0;
	if(IsAlightingRestricted())
	 return num;
	const Platform* platform = this->getNextPlatform();
    if(platform)
    {
    	std::string ptName=platform->getPlatformNo();
    }
	std::list<Passenger*>::iterator i = passengerList.begin();
	std::string tm=(DailyTime(now.ms())+DailyTime(ConfigManager::GetInstance().FullConfig().simStartTime())).getStrRepr();
	sim_mob::BasicLogger& ptMRTLogger  = sim_mob::Logger::log("PersonsAlighting.csv");
	while(i!=passengerList.end())
	{
		const WayPoint& endPoint = (*i)->getEndPoint();
		if(endPoint.type==WayPoint::MRT_PLATFORM)
		{
			if(endPoint.platform==platform)
			{
				alightingPassenger.push_back(*i);
				ptMRTLogger <<(*i)->getParent()->getDatabaseId()<<","<<tm<<","<<getTrainId()<<","<<getTripId()<<","<<(*i)->getParent()->currSubTrip->origin.platform->getPlatformNo()<<","<<(*i)->getParent()->currSubTrip->destination.platform->getPlatformNo()<<std::endl;
				i = passengerList.erase(i);
				num++;
				continue;
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

int TrainDriver::AlightAllPassengers(std::list<Passenger*>& alightingPassenger,timeslice now)
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


void TrainDriver::InsertStopPoint(PolyPoint point,double duration)
{

	StopPointEntity stopPointEntity;
	stopPointEntity.point=point;
	stopPointEntity.duration=duration;
	stopPointEntitiesLock.lock();
	stopPointEntities.push_back(stopPointEntity);
	stopPointEntitiesLock.unlock();
}


void TrainDriver::LockUnlockRestrictPassengerEntitiesLock(bool lock)
{
	if(lock)
	restrictEntitiesLock.lock();

	else
		restrictEntitiesLock.unlock();
}

void TrainDriver::InsertRestrictPassengerEntity(std::string platformName,int movType)
{

	restrictEntitiesLock.lock();
	std::map<std::string,passengerMovement>::iterator itr = restrictPassengersEntities.find(platformName);
	if(itr!=restrictPassengersEntities.end())
	{
		restrictPassengersEntities.erase(itr);
	}
	restrictPassengersEntities[platformName]=passengerMovement(movType);
	restrictEntitiesLock.unlock();
}

std::vector<StopPointEntity>& TrainDriver::GetStopPoints()
{
	return stopPointEntities;
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
	if (!person) {
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
		arrivalInfo.pctOccupancy = (((double)passengerList.size())/config.trainController.maxCapacity) * 100.0;
		arrivalInfo.stopNo = platform->getPlatformNo();
		messaging::MessageBus::PostMessage(PT_Statistics::getInstance(), STORE_BUS_ARRIVAL, messaging::MessageBus::MessagePtr(new PT_ArrivalTimeMessage(arrivalInfo)));
	}
}
int TrainDriver::boardPassenger(std::list<WaitTrainActivity*>& boardingPassenger,timeslice now)
{
	int num = 0;
    if(IsBoardingRestricted())
       return num;

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

			ptMRTLogger<<person->getDatabaseId()<<","<<tm<<","<<getTrainId()<<","<<getTripId()<<","<<person->currSubTrip->origin.platform->getPlatformNo()<<","<<person->currSubTrip->destination.platform->getPlatformNo()<<std::endl;
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
	while(i!=boardingPassenger.end())
	{
		(*i)->incrementDeniedBoardingCount();
		i++;
	}
	return num;
  }

int TrainDriver::boardForceAlightedPassengersPassenger(std::list<Passenger*>& forcealightedPassengers,timeslice now)
{
	int num = 0;
	if(IsBoardingRestricted())
	   return num;

	int validNum = getEmptyOccupation();
	std::list<Passenger*>::iterator i = forcealightedPassengers.begin();
	while(i!=forcealightedPassengers.end()&&validNum>0)
	{
		passengerList.push_back(*i);
		i = forcealightedPassengers.erase(i);
		validNum--;
		num++;
	}

    return num;
}


bool TrainDriver::IsBoardingRestricted()
{
	std::map<std::string,passengerMovement>::iterator it=restrictPassengersEntities.begin();
	const Platform* platform = this->getNextPlatform();
	if(platform)
	{

	   it=restrictPassengersEntities.find(platform->getPlatformNo());
	   if(it!=restrictPassengersEntities.end())
	   {

		   passengerMovement movType=restrictPassengersEntities[platform->getPlatformNo()];

		   if(movType==BOARDING||movType==BOTH)
		   {
			   restrictPassengersEntities.erase(it);
			   //restrictPassengersEntitiesLock.unlock();
			   return true;

		   }
	   }

	}
}

bool TrainDriver::IsAlightingRestricted()
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
    	   if(movType==ALIGHTING||movType==BOTH)
    	   {
    		   restrictPassengersEntities.erase(it);
    		   //restrictPassengersEntitiesLock.unlock();
    		   return true;
    	   }

       }

	}
}


void TrainDriver::SetStoppingParameters(PolyPoint point,double duration)
{
	remainingStopTime=duration;
	currStopPoint =point;
	stoppedAtPoint=true;
}
}
} /* namespace sim_mob */
