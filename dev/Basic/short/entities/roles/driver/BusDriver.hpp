//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Driver.hpp"
#include "entities/vehicle/BusRoute.hpp"
#include "entities/misc/BusTrip.hpp"
#include "BusDriverFacets.hpp"
#include <vector>

namespace sim_mob {

//Forward declarations
class DriverUpdateParams;
class PackageUtils;
class UnPackageUtils;
class BusStop;
class Person;
//class Bus;
class Passenger;
class BusDriverBehavior;
class BusDriverMovement;


/**
 * This simple BusDriver class maintains a single, non-looping route with a series of
 *   stops. Most driving behavior is re-used from the Driver class. At bus stops, the
 *   BusDriver will simply pull over to the left-most lane and stop for a length of time.
 *
 * \author Seth N. Hetu
 */
class BusDriver : public sim_mob::Driver {
public:
	BusDriver(sim_mob::Person* parent, sim_mob::MutexStrategy mtxStrat, sim_mob::BusDriverBehavior* behavior = nullptr, sim_mob::BusDriverMovement* movement = nullptr, Role::type roleType_ = RL_BUSDRIVER);

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Overrides
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
	virtual sim_mob::DriverRequestParams getDriverRequestParams();

	double getPositionX() const;
	double getPositionY() const;
	// get the last bus stop real times
	Shared<BusStop_RealTimes>* getLastBusStopRealTimes() {
		return last_busStopRealTimes;
	}
	// get the bus stop real times vector
	std::vector<Shared<BusStop_RealTimes>* >& getBusStopRealTimes() {
		return busStopRealTimes_vec_bus;
	}

	/**
	* set BusStop RealTimes for a particular bus stop.
	* @param busStopSeqNum the sequence number of the bus stop to be set.
	* @param busStopRealTimes the busStopRealTimes that will be set later and will be valid at the next time tick.
	*/
	void setBusStopRealTimes(const int& busStopSeqNum, const BusStop_RealTimes& busStopRealTimes);

	// can get some passenger count, passenger information and busStop information
	Shared<const BusStop*> lastVisited_BusStop;
	// last visited busStop sequence number m, reset by BusDriver, What Time???(needed for query the last Stop m -->realStop Times)---> move to BusTrip later
	Shared<int> lastVisited_BusStopSequenceNum;
	// set by BusController, reset once stop at only busStop j (j belong to the small set of BusStops)
	Shared<double> real_DepartureTime;
	// set by BusDriver, reset once stop at any busStop
	Shared<double> real_ArrivalTime;
	// current BusStop real Times, convenient for reset
	Shared<BusStop_RealTimes>* last_busStopRealTimes;
	// set by BusDriver, reset once stop at any busStop
	Shared<double> DwellTime_ijk;
	// set by BusDriver(temporary), only needed by BusDriver
	double dwellTime_record;
	// set by BusDriver, has 0.1sec delay
	Shared<int> busstop_sequence_no;
	//get bus line information
	Shared<std::string> lastVisited_Busline;
	Shared<int> lastVisited_BusTrip_SequenceNo;
	Shared<int> existed_Request_Mode;
	Shared<double> waiting_Time;
	double xpos_approachingbusstop,ypos_approachingbusstop;
	// can be different for different pair<busLine_id,busTripRun_sequenceNum>
	std::vector<Shared<BusStop_RealTimes>* > busStopRealTimes_vec_bus;

//Basic data
private:
	//Serialization, not implemented
	friend class BusDriverBehavior;
	friend class BusDriverMovement;

#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil){};
	virtual void unpack(UnPackageUtils& unpackageUtil){};
	virtual void packProxy(PackageUtils& packageUtil){};
	virtual void unpackProxy(UnPackageUtils& unpackageUtil){};
#endif

};
}
