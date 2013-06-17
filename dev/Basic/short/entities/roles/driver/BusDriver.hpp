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
//	virtual void frame_init(UpdateParams& p);
//	virtual void frame_tick(UpdateParams& p);
//	virtual void frame_tick_output(const UpdateParams& p);
//	virtual void frame_tick_output_mpi(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
	virtual sim_mob::DriverRequestParams getDriverRequestParams();

//	// new boarding method
//	// individual boarding and alighting(Yao Jin)
//	void IndividualBoardingAlighting_New(Bus* bus);
//	// determine boarding and alighting MS for possible boarding and alighting persons(Yao Jin)
//	void DetermineBoardingAlightingMS(Bus* bus);
//	// start boarding and alighting based on the boarding and alighting MS
//	// change passenger amount on the Bus(Yao Jin)
//	void StartBoardingAlighting(Bus* bus);
//	// reset some boarding and alighting variables after leaving the BusStop(Yao Jin)
//	void resetBoardingAlightingVariables();
//
//    void AlightingPassengers(Bus* bus);
//
//
//   ///dwell time calculation module
//	double dwellTimeCalculation(int A,int B,int delta_bay,int delta_full,int Pfront,int no_of_passengers); // dwell time calculation module
//	std::vector<const sim_mob::BusStop*> findBusStopInPath(const std::vector<const sim_mob::RoadSegment*>& path) const;

	double getPositionX() const;
	double getPositionY() const;
	Shared<BusStop_RealTimes>* getCurrentBusStopRealTimes() {
		return last_busStopRealTimes;
	}
	std::vector<Shared<BusStop_RealTimes>* >& getBusStop_RealTimes() {
		return busStopRealTimes_vec_bus;
	}

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
//	bool first_busstop;
//	bool last_busstop;
//	bool passengerCountOld_display_flag;
//	size_t no_passengers_boarding;
//	size_t no_passengers_alighting;

//	// flag to indicate whether boarding and alighting is allowed, if it is false, boarding alighting frame is not determined(reset after BusDriver leaves the BusStop)
//	bool allow_boarding_alighting_flag;
//	// a tempoary boarding queue, will be cleared after BusDriver leaves the BusStop
//	std::vector<sim_mob::Person*> virtualBoarding_Persons;
//	// record the BoardingNum_Pos map based on the boarding queue in the BusStopAgent, cleared after BusDriver leaves the BusStop
//	std::map<int, int> BoardingNum_Pos;
//	// record the AlightingNum_Pos map based on the passenger queue in the Bus, cleared after BusDriver leaves the BusStop
//	std::map<int, int> AlightingNum_Pos;
//	// boarding_MSs for possible boarding persons, cleared after leaving the BusStop
//	std::vector<uint32_t> boarding_MSs;
//	// alighting_MSs for possible alighting persons, cleared after leaving the BusStop
//	std::vector<uint32_t> alighting_MSs;
//	// the first boarding and alighting MS where bus will start boarding and alighting, reset after leaving the BusStop
//	uint32_t first_boarding_alighting_ms;
//	// the last boarding and alighting MS and bus will leaves the BusStop, reset after leaving the BusStop
//	uint32_t last_boarding_alighting_ms;
//	// temporary boardingMS offset for boarding queue erase purpose, reset after leaving the BusStop
//	int boardingMS_offset;
//	// temporary alightingMS offset for passenger queue erase purpose, reset after leaving the BusStop
//	int alightingMS_offset;
//	// holdingtime SECS
//	double BUS_STOP_HOLDING_TIME_SEC;
//	// dwelltime(boarding and alighting time SECS)
//	double BUS_STOP_WAIT_BOARDING_ALIGHTING_SEC;

//protected:
//	//Override the following behavior
//	virtual double linkDriving(DriverUpdateParams& p);

//Basic data
private:
//	// total busStop list
//	std::vector<const BusStop*> busStops;
//	// waiting MS adding at the BusStop
//	double waitAtStopMS;
//	// total waiting time (can be holding time or dwelltime)
//	double BUS_STOP_WAIT_TIME;

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
