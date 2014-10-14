//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * BusDriver.cpp
 *
 *  Created on: May 6, 2013
 *      Author: zhang huai peng
 *      		melani
 */

#include "BusDriver.hpp"
#include "entities/Person.hpp"
#include "entities/BusStopAgent.hpp"
#include "message/MT_Message.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "util/DwellTimeCalc.hpp"
#include "util/Utils.hpp"
#include "config/MT_Config.hpp"

using namespace sim_mob;
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

sim_mob::medium::BusDriver::BusDriver(Person* parent, MutexStrategy mtxStrat,
		sim_mob::medium::BusDriverBehavior* behavior,
		sim_mob::medium::BusDriverMovement* movement,
		std::string roleName, Role::type roleType)
: sim_mob::medium::Driver(parent, mtxStrat, behavior, movement, roleName, roleType),
  requestMode(mtxStrat, 0), visitedBusStop(mtxStrat, nullptr),
  visitedBusStopSequenceNo(mtxStrat, -1), arrivalTime(mtxStrat, 0.0),
  dwellTime(mtxStrat, 0.0), visitedBusTripSequenceNo(mtxStrat, 0),
  visitedBusLine(mtxStrat, "0"), holdingTime(mtxStrat, 0.0),
  waitingTimeAtbusStop(0.0)
{}

sim_mob::medium::BusDriver::~BusDriver() {}

Role* sim_mob::medium::BusDriver::clone(Person* parent) const {
	BusDriverBehavior* behavior = new BusDriverBehavior(parent);
	BusDriverMovement* movement = new BusDriverMovement(parent);
	BusDriver* busdriver = new BusDriver(parent, parent->getMutexStrategy(), behavior, movement, "BusDriver_");
	behavior->setParentBusDriver(busdriver);
	movement->setParentBusDriver(busdriver);
	movement->setParentDriver(busdriver);
	return busdriver;
}

const std::vector<const sim_mob::BusStop*>* sim_mob::medium::BusDriver::getBusStopsVector() const {
	const std::vector<const sim_mob::BusStop*>* stopsVec=nullptr;
	sim_mob::Person* person = dynamic_cast<Person*>(parent);
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

bool sim_mob::medium::BusDriver::addPassenger(sim_mob::medium::Passenger* passenger) {
	passengerList.push_back(passenger);
	return true;
}

sim_mob::DriverRequestParams sim_mob::medium::BusDriver::getDriverRequestParams()
{
	sim_mob::DriverRequestParams res;
	return res;
}

bool  sim_mob::medium::BusDriver::checkIsFull()
{
	if (passengerList.size() < MT_Config::getInstance().getBusCapacity()) {
		return false;
	} else {
		return true;
	}
}

unsigned int sim_mob::medium::BusDriver::alightPassenger(sim_mob::medium::BusStopAgent* busStopAgent){
	unsigned int numAlighting = 0;
	std::list<sim_mob::medium::Passenger*>::iterator itPassenger = passengerList.begin();
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

void sim_mob::medium::BusDriver::storeArrivalTime(const std::string& current, const std::string& waitTime, const sim_mob::BusStop* stop)
{
	sim_mob::Person* person = parent;
	if (!person) {
		return;
	}

	const BusTrip* busTrip =
			dynamic_cast<const BusTrip*>(*(person->currTripChainItem));
	if (busTrip) {
		const Busline* busLine = busTrip->getBusline();
		std::string stopNo = stop->getBusstopno_();
		std::string tripId = busTrip->tripID;
		std::string busLineId = busLine->getBusLineID();
		unsigned int sequenceNo = busTrip->getBusTripRun_SequenceNum();

		messaging::MessageBus::PostMessage(PT_Statistics::GetInstance(), STORE_BUS_ARRIVAL,
				messaging::MessageBus::MessagePtr(new BusArrivalTimeMessage(stopNo, busLineId, tripId, current, waitTime, sequenceNo)));
	}
}

void sim_mob::medium::BusDriver::predictArrivalAtBusStop(double preArrivalTime,
		sim_mob::medium::BusStopAgent* busStopAgent) {
	sim_mob::Person* person = dynamic_cast<Person*>(parent);
	if (!person) {
		return;
	}

	const BusTrip* busTrip =
			dynamic_cast<const BusTrip*>(*(person->currTripChainItem));
	if (busTrip) {
		const Busline* busLine = busTrip->getBusline();
		visitedBusLine.set(busLine->getBusLineID());
		visitedBusTripSequenceNo.set(busTrip->getBusTripRun_SequenceNum());
		requestMode.set(Role::REQUEST_DECISION_TIME);
		visitedBusStop.set(busStopAgent->getBusStop());
		visitedBusStopSequenceNo.set(visitedBusStopSequenceNo.get() + 1);
		arrivalTime.set(preArrivalTime);
	}
}

void sim_mob::medium::BusDriver::openBusDoors(const std::string& current, sim_mob::medium::BusStopAgent* busStopAgent) {
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
	busStopAgent->handleBusArrival(this);
	unsigned int numAlighting = alightPassenger(busStopAgent);
	unsigned int numBoarding = busStopAgent->getBoardingNum(this);

	unsigned int totalNumber = numAlighting + numBoarding;

	const std::vector<float>& dwellTimeParams =
				MT_Config::getInstance().getDwellTimeParams();

	const float fixedTime = Utils::generateFloat(dwellTimeParams[0],dwellTimeParams[1]);
	const float individualTime = Utils::generateFloat(dwellTimeParams[2], dwellTimeParams[3]);
	int boardNum = std::max(numAlighting, numBoarding);
	waitingTimeAtbusStop = fixedTime+boardNum*individualTime;

	DailyTime dwellTime( converToMilliseconds(waitingTimeAtbusStop) );
	storeArrivalTime(current, dwellTime.toString(), busStopAgent->getBusStop());


	if (requestMode.get() == Role::REQUEST_DECISION_TIME) {
		requestMode.set(Role::REQUEST_NONE);
		//final waiting time is maximum value between dwelling time and holding time
		waitingTimeAtbusStop = std::max(waitingTimeAtbusStop, holdingTime.get());
		holdingTime.set(0.0);
	}
	currResource->setMoving(false);
}

void sim_mob::medium::BusDriver::closeBusDoors(sim_mob::medium::BusStopAgent* busStopAgent) {
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

std::vector<BufferedBase*> sim_mob::medium::BusDriver::getSubscriptionParams() {
	return vector<BufferedBase*>();
}

void sim_mob::medium::BusDriver::make_frame_tick_params(timeslice now) {
	getParams().reset(now);
}


