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
namespace sim_mob {

namespace medium{
int TrainDriver::counter=0;
TrainDriver::TrainDriver(Person_MT* parent,
		sim_mob::medium::TrainBehavior* behavior,
		sim_mob::medium::TrainMovement* movement,
		std::string roleName, Role<Person_MT>::Type roleType) :
	sim_mob::Role<Person_MT>::Role(parent, behavior, movement, roleName, roleType),
	nextDriver(nullptr),nextRequested(NO_REQUESTED),waitingTimeSec(0.0),initialDwellTime(0.0),disruptionParam(nullptr),platSequenceNumber(0)
{
	int trainId=getTrainId();
	std::string lineId=getTrainLine();
	Role<Person_MT> *trainDriver=dynamic_cast<Role<Person_MT>*>(this);
	//ServiceController::getInstance()->InsertTrainIdAndTrainDriverInMap(trainId,lineId,trainDriver);
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

Role<Person_MT>* TrainDriver::clone(Person_MT *parent) const
{
	TrainBehavior* behavior = new TrainBehavior();
	TrainMovement* movement = new TrainMovement();
	TrainDriver* driver = new TrainDriver(parent, behavior, movement, "TrainDriver_");
	behavior->setParentDriver(driver);
	movement->setParentDriver(driver);
	return driver;
}

void TrainDriver::setNextDriver(const TrainDriver* driver)
{
	nextDriver = driver;
}
const TrainDriver* TrainDriver::getNextDriver() const
{
	return nextDriver;
}

void TrainDriver::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
	if(disruptionParam.get()){
		DailyTime duration = disruptionParam->duration;
		unsigned int baseGran = ConfigManager::GetInstance().FullConfig().baseGranMS();
		if(duration.getValue()>baseGran){
			disruptionParam->duration = DailyTime(duration.offsetMS_From(DailyTime(baseGran)));
		} else {
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

TrainMovement* TrainDriver::GetMovement()
{
	TrainMovement* movement = dynamic_cast<TrainMovement*>(movementFacet);
	return movement;
}

void TrainDriver::SetTrainDriverInOpposite(TrainDriver *trainDriver)
{
	nextDriverInOppLine = trainDriver;
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
double TrainDriver::calculateDwellTime(int boarding,int alighting)
{
	int noOfPassengerInTrain=this->getPassengers().size();
	/*const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	int time = Utils::generateFloat(config.trainController.miniDwellTime, config.trainController.maxDwellTime);
	int sysGran = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	time = (time/sysGran)*sysGran;*/

	double time = 12.22 + 2.27*boarding/24 + 1.82*alighting/24 + 0.00062*(noOfPassengerInTrain/24)*(noOfPassengerInTrain/24)*(noOfPassengerInTrain/24)*(boarding/24);
	waitingTimeSec = time;
	initialDwellTime=time;
	return waitingTimeSec;
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
int TrainDriver::alightPassenger(std::list<Passenger*>& alightingPassenger,timeslice now)
{
	int num = 0;
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
	if (trip && platform) {
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
}
} /* namespace sim_mob */
