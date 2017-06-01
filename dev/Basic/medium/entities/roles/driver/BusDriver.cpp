//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusDriver.hpp"
#include "BusDriverFacets.hpp"
#include "entities/BusStopAgent.hpp"
#include "message/MT_Message.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "util/DwellTimeCalc.hpp"
#include "util/Utils.hpp"
#include "util/DailyTime.hpp"
#include "config/MT_Config.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;

namespace
{
//number of dwell time parameter expected.
//TODO: remove this constant and restructure the dwell time parameters elegantly
const unsigned int NUM_PARAMS_DWELLTIME = 5;

/**
 * converts time from  seconds to milli-seconds
 */
inline unsigned int converToMilliseconds(double timeInMs) {
	return (timeInMs*1000.0);
}
}

BusDriver::BusDriver(Person_MT* parent, const MutexStrategy& mtxStrat, BusDriverBehavior* behavior,
		BusDriverMovement* movement, std::string roleName, Role<Person_MT>::Type roleType) :
		Driver(parent, behavior, movement, roleName, roleType),  waitingTimeAtbusStop(0.0), busSequenceNumber(1)
{
}

BusDriver::~BusDriver(){}

Role<Person_MT>* BusDriver::clone(Person_MT* parent) const {
	BusDriverBehavior* behavior = new BusDriverBehavior();
	BusDriverMovement* movement = new BusDriverMovement();
	BusDriver* busdriver = new BusDriver(parent, parent->getMutexStrategy(), behavior, movement, "BusDriver_");
	behavior->setParentBusDriver(busdriver);
	movement->setParentBusDriver(busdriver);
	movement->setParentDriver(busdriver);
	return busdriver;
}

const std::vector<const BusStop*>* BusDriver::getBusStopsVector() const {
	const std::vector<const BusStop*>* stopsVec=nullptr;
	Person_MT* person = dynamic_cast<Person_MT*>(parent);
	if(!person){
		return stopsVec;
	}

	const BusTrip* busTrip =dynamic_cast<const BusTrip*>(*(person->currTripChainItem));
	if(!busTrip){
		return stopsVec;
	}
	const BusRouteInfo& routeInfo = busTrip->getBusRouteInfo();
	stopsVec = &(routeInfo.getBusStops());
	return stopsVec;
}
std::string lastBoarding;
bool BusDriver::addPassenger(Passenger* passenger) {
	passengerList.push_back(passenger);
	return true;
}

unsigned int BusDriver::getPassengerCount() const
{
	return passengerList.size();
}

DriverRequestParams BusDriver::getDriverRequestParams()
{
	DriverRequestParams res;
	return res;
}

bool  BusDriver::checkIsFull()
{
	if (passengerList.size() < MT_Config::getInstance().getBusCapacity()) {
		return false;
	} else {
		return true;
	}
}

unsigned int BusDriver::alightPassenger(BusStopAgent* busStopAgent){
	unsigned int numAlighting = 0;
	std::list<Passenger*>::iterator itPassenger = passengerList.begin();
	const BusStop* stop = busStopAgent->getBusStop();
	if(stop->isVirtualStop())
	{
		stop = stop->getTwinStop();
		if(stop->isVirtualStop()) { throw std::runtime_error("both of the twin stops are virtual"); }
		busStopAgent = BusStopAgent::getBusStopAgentForStop(stop);
	}

	while (itPassenger != passengerList.end()) {

		/*the passengers will be always together with bus driver, so
		 * bus driver and passengers will always in the same conflux and
		 * in the same thread. so that it is safety for bus driver directly
		 * to invoke decision method of  inside passengers
		 */
		(*itPassenger)->makeAlightingDecision(busStopAgent->getBusStop());

		if ((*itPassenger)->canAlightBus()) {
			busStopAgent->addAlightingPerson(*itPassenger);
			itPassenger = passengerList.erase(itPassenger);
			numAlighting++;
		} else {
			itPassenger++;
		}
	}
	return numAlighting;
}

void BusDriver::storeArrivalTime(const std::string& current, const std::string& waitTime, const BusStop* stop)
{
	Person_MT* person = parent;
	if (!person) {
		return;
	}

	const BusTrip* busTrip = dynamic_cast<const BusTrip*>(*(person->currTripChainItem));
	if (busTrip) {
		std::string busStopNo;
		if(stop->isVirtualStop())
		{
			busStopNo = stop->getTwinStop()->getStopCode();
		}
		else
		{
			busStopNo = stop->getStopCode();
		}
		PT_ArrivalTime arrivalInfo;
		arrivalInfo.serviceLine = busTrip->getBusLine()->getBusLineID();
		arrivalInfo.tripId = busTrip->tripID;
		arrivalInfo.sequenceNo = busSequenceNumber;
		arrivalInfo.arrivalTime = current;
		arrivalInfo.dwellTime = waitTime;
		arrivalInfo.dwellTimeSecs = (DailyTime(waitTime)).getValue() / 1000.0;
		arrivalInfo.pctOccupancy = (((double)passengerList.size())/MT_Config::getInstance().getBusCapacity()) * 100.0;
		arrivalInfo.stopNo = busStopNo;
		messaging::MessageBus::PostMessage(PT_Statistics::getInstance(), STORE_BUS_ARRIVAL, messaging::MessageBus::MessagePtr(new PT_ArrivalTimeMessage(arrivalInfo)));
		this->busSequenceNumber++;
	}
}

void BusDriver::updatePassengers()
{
	for (std::list<Passenger*>::iterator it = passengerList.begin(); it != passengerList.end(); it++)
	{
		(*it)->Movement()->frame_tick();
	}
}

const std::string BusDriver::getBusLineID() const
{
	if (!parent) {
		return std::string();
	}

	const BusTrip* busTrip =
			dynamic_cast<const BusTrip*>(*(parent->currTripChainItem));
	if (busTrip) {
		return busTrip->getBusLine()->getBusLineID();
	}
	else {
		return std::string();
	}
}


void BusDriver::openBusDoors(const std::string& current, BusStopAgent* busStopAgent)
{
	if(!busStopAgent)
	{
		throw std::runtime_error("openBusDoors(): NusStopAgent is NULL");
	}

	/* handling bus arrival should ideally take place by sending an instantaneous
	 * message, but it is not guaranteed that the bus driver and the bus stop agent
	 * will be in the same thread context. If the bus driver happens to be in the
	 * virtual queue, he would be processed from the main thread while the bus stop
	 * agent's thread context will point to the thread corresponding to the worker of
	 * its conflux. We therefore call the handleBusArrival() function directly.
	 * This is thread safe because if the bus driver is processed from the workers,
	 * all bus drivers who will call this function will be in the same worker (thread context)
	 * and hence no race condition will occur. If the bus driver is processed from the
	 * main thread, exactly one bus driver can be processed at a time and no other
	 * agent in the simulation will be updating at the same time.
	 */

	unsigned int numAlighting = alightPassenger(busStopAgent);
	busStopAgent->handleBusArrival(this);
	unsigned int numBoarding = busStopAgent->getBoardingNum(this);
	unsigned int totalNumber = numAlighting + numBoarding;

	int boardNum = std::max(numAlighting, numBoarding);
	const std::vector<float>& dwellTimeParams = MT_Config::getInstance().getDwellTimeParams();
	const float fixedTime = Utils::generateFloat(dwellTimeParams[0],dwellTimeParams[1]);
	if(boardNum==0)
	{
		waitingTimeAtbusStop=fixedTime;
	}
	else
	{
		const float individualTime = Utils::generateFloat(dwellTimeParams[2], dwellTimeParams[3]);
		waitingTimeAtbusStop = fixedTime+boardNum*individualTime;
	}

	DailyTime dwellTime( converToMilliseconds(waitingTimeAtbusStop) );
	storeArrivalTime(current, dwellTime.getStrRepr(), busStopAgent->getBusStop());

	currResource->setMoving(false);
}

void BusDriver::closeBusDoors(BusStopAgent* busStopAgent)
{
	if(!busStopAgent)
	{
		throw std::runtime_error("openBusDoors(): NusStopAgent is NULL");
	}

	/* handling bus departure should ideally take place by sending an instantaneous
	 * message, but it is not guaranteed that the bus driver and the bus stop agent
	 * will be in the same thread context. If the bus driver happens to be in a
	 * virtual queue, he would be processed from the main thread while the bus stop
	 * agent's thread context will point to the thread corresponding to the worker of
	 * its conflux. We therefore call the handleBusDeparture() function directly.
	 * This is thread safe because if the bus driver is processed from the workers,
	 * all bus drivers who will call this function will be in the same worker (thread context)
	 * and hence no race condition will occur. If the bus driver is processed from the
	 * main thread, exactly one bus driver can be processed at a time and no other
	 * agent in the simulation will be updating at the same time.
	 */
	busStopAgent->handleBusDeparture(this);
	currResource->setMoving(true);
}

std::vector<BufferedBase*> BusDriver::getSubscriptionParams() {
	return vector<BufferedBase*>();
}

void BusDriver::make_frame_tick_params(timeslice now) {
	getParams().reset(now);
}


