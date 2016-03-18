/*
 * TrainDriver.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */

#include "entities/roles/driver/TrainDriver.hpp"
#include "TrainDriverFacets.hpp"
#include "util/Utils.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/misc/TrainTrip.hpp"
namespace sim_mob {
namespace medium{

TrainDriver::TrainDriver(Person_MT* parent,
		sim_mob::medium::TrainBehavior* behavior,
		sim_mob::medium::TrainMovement* movement,
		std::string roleName, Role<Person_MT>::Type roleType) :
	sim_mob::Role<Person_MT>::Role(parent, behavior, movement, roleName, roleType),nextDriver(nullptr),trainStatus(NO_STATUS),waitingTimeSec(0.0)
{

}

TrainDriver::~TrainDriver()
{

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
TrainDriver::TRAIN_STATUS TrainDriver::getCurrentStatus() const
{
	return trainStatus;
}

void TrainDriver::setCurrentStatus(TRAIN_STATUS status)
{
	trainStatus = status;
}
void TrainDriver::calculateDwellTime(int totalNum)
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	int time = Utils::generateFloat(config.trainController.miniDwellTime, config.trainController.maxDwellTime);
	int sysGran = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	time = (time/sysGran)*sysGran;
	waitingTimeSec = time;
}
double TrainDriver::getWaitingTime() const
{
	return waitingTimeSec;
}

void TrainDriver::reduceWaitingTime(double val)
{
	waitingTimeSec -= val;
}

std::string TrainDriver::getTrainLine() const
{
	std::string lineId;
	if(getParent())
	{
		std::vector<TripChainItem *>::iterator currTrip = getParent()->currTripChainItem;
		const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*currTrip);
		if(trip){
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
		if(trip){
			id = trip->getTripId();
		}
	}
	return id;
}
Platform* TrainDriver::getNextPlatform() const
{
	Platform* platform = nullptr;
	TrainMovement* movement = dynamic_cast<TrainMovement*>(movementFacet);
	if(movement){
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
	if(passengerList.size()<config.trainController.maxCapacity){
		return config.trainController.maxCapacity-passengerList.size();
	}
	return 0;
}
int TrainDriver::alightPassenger(std::list<Passenger*>& alightingPassenger)
{
	int num = 0;
	const Platform* platform = this->getNextPlatform();
	std::list<Passenger*>::iterator i = passengerList.begin();
	while(i!=passengerList.end()){
		const WayPoint& endPoint = (*i)->getEndPoint();
		if(endPoint.type==WayPoint::MRT_PLATFORM){
			if(endPoint.platform==platform){
				alightingPassenger.push_back(*i);
				i = passengerList.erase(i);
				num++;
				continue;
			}
		}
		i++;
	}
	return num;
}

int TrainDriver::boardPassenger(std::list<Passenger*>& boardingPassenger)
{
	int num = 0;
	int validNum = getEmptyOccupation();
	std::list<Passenger*>::iterator i = boardingPassenger.begin();
	while(i!=boardingPassenger.end()&&validNum>0){
		passengerList.push_back(*i);
		i = boardingPassenger.erase(i);
		validNum--;
		num++;
	}
	return num;
}
}
} /* namespace sim_mob */
