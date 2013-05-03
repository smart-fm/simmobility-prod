/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "Driver.hpp"
#include "entities/vehicle/BusRoute.hpp"
#include "entities/misc/BusTrip.hpp"
#include <vector>

namespace sim_mob {

//Forward declarations
class DriverUpdateParams;
class PackageUtils;
class UnPackageUtils;
class BusStop;
class Person;
class Bus;
class Passenger;


/**
 * This simple BusDriver class maintains a single, non-looping route with a series of
 *   stops. Most driving behavior is re-used from the Driver class. At bus stops, the
 *   BusDriver will simply pull over to the left-most lane and stop for a length of time.
 *
 * \author Seth N. Hetu
 */
class BusDriver : public sim_mob::Driver {
public:
	BusDriver(sim_mob::Person* parent, sim_mob::MutexStrategy mtxStrat);

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
	virtual sim_mob::DriverRequestParams getDriverRequestParams();

	/// Return the distance (m) to the (next) bus stop.
	/// A negative return value indicates that there is no relevant bus stop nearby.
	double distanceToNextBusStop();

	/// get distance to bus stop of particular segment (meter)
	/// A negative return value indicates that there is no relevant bus stop nearby.
	double getDistanceToBusStopOfSegment(const RoadSegment* rs);
	bool isBusFarawayBusStop();
	bool isBusApproachingBusStop();
	bool isBusArriveBusStop();
	bool isBusLeavingBusStop();
	bool isBusGngtoBreakDown();
	double busAccelerating(DriverUpdateParams& p);
	//mutable double lastTickDistanceToBusStop;


	///here passenger initially chooses which bus lines to board upon reaching bus stop
	///and board the bus when it approaches based on this initial choice
	void BoardingPassengers_Choice(Bus* bus);
	///here passenger makes decision to board bus when bus reaches bus stop
	///if the bus goes to the destination passenger decides to board
	void BoardingPassengers_Normal(Bus* bus);
	// new boarding method
	void IndividualBoardingAlighting_New(Bus* bus);// Yao Jin
	void DetermineBoardingAlightingFrame(Bus* bus);// Yao Jin
	void StartBoardingAlighting(Bus* bus);// Yao Jin
	void resetBoardingAlightingVariables();// reset after leaving the BusStop, Yao Jin

    void AlightingPassengers(Bus* bus);

    ///functions for passenger generation with distribution(version 1 implementation of passenger)
    ///for generating passenger random passenger distribution at a bus stop by bus driver and calling boarding and alighting functions
   /* double passengerGeneration(Bus* bus);
	void Board_passengerGeneration(Bus* bus);
	void Alight_passengerGeneration(Bus* bus);*/



   ///dwell time calculation module
	double dwellTimeCalculation(int A,int B,int delta_bay,int delta_full,int Pfront,int no_of_passengers); // dwell time calculation module
	std::vector<const sim_mob::BusStop*> findBusStopInPath(const std::vector<const sim_mob::RoadSegment*>& path) const;

	double getPositionX() const;
	double getPositionY() const;

	double getWaitTime_BusStop() { return BUS_STOP_WAIT_TIME; }
	void setWaitTime_BusStop(double time) { BUS_STOP_WAIT_TIME = time; }// total waiting time at the BusSTop
	Vehicle* initializePath_bus(bool allocateVehicle);
	Shared<BusStop_RealTimes>* getCurrentBusStopRealTimes() {
		return last_busStopRealTimes;
	}
	std::vector<Shared<BusStop_RealTimes>* >& getBusStop_RealTimes() {
		return busStopRealTimes_vec_bus;
	}

	double lastTickDistanceToBusStop;
	bool demo_passenger_increase;
	Shared<const BusStop*> lastVisited_BusStop; // can get some passenger count, passenger information and busStop information
	Shared<int> lastVisited_BusStopSequenceNum; // last visited busStop sequence number m, reset by BusDriver, What Time???(needed for query the last Stop m -->realStop Times)---> move to BusTrip later
	Shared<double> real_DepartureTime; // set by BusController, reset once stop at only busStop j (j belong to the small set of BusStops)
	Shared<double> real_ArrivalTime; // set by BusDriver, reset once stop at any busStop
	Shared<BusStop_RealTimes>* last_busStopRealTimes; // current BusStop real Times, convenient for reset
	Shared<double> DwellTime_ijk; // set by BusDriver, reset once stop at any busStop
	double dwellTime_record;// set by BusDriver(temporary), only needed by BusDriver
	Shared<int> busstop_sequence_no; // set by BusDriver, has 0.1sec delay
	Shared<std::string> lastVisited_Busline; //get bus line information
	Shared<int> lastVisited_BusTrip_SequenceNo;
	Shared<int> existed_Request_Mode;
	Shared<double> waiting_Time;
	double xpos_approachingbusstop,ypos_approachingbusstop;
	std::vector<Shared<BusStop_RealTimes>* > busStopRealTimes_vec_bus;// can be different for different pair<busLine_id,busTripRun_sequenceNum>
	bool first_busstop;
	bool last_busstop;
	bool passengerCountOld_display_flag;
	size_t no_passengers_boarding;
	size_t no_passengers_alighting;


	bool allow_boarding_alighting_flag;// flag to advance the frame
	std::vector<sim_mob::Person*> virtualBoarding_Persons;// a virtual queue, will be cleared after BusDriver leaves the BusStop
	std::map<int, int> BoardingNum_Pos;
	std::map<int, int> AlightingNum_Pos;
	std::vector<uint32_t> boarding_frames;// boarding_frames for possible boarding persons, cleared after leaving the BusStop
	std::vector<uint32_t> alighting_frames;// alighting_frames for possible alighting persons, cleared after leaving the BusStop
	uint32_t first_frame;// the first frame and bus will start boarding and alighting
	uint32_t last_frame;// the last frame and bus will leaves the BusStop
	int boardingframe_offset;
	int alightingframe_offset;
	double BUS_STOP_HOLDING_TIME_SEC;// holdingtime
	double BUS_STOP_WAIT_BOARDING_ALIGHTING_SEC;// dwelltime

protected:
	//Override the following behavior
	virtual double linkDriving(DriverUpdateParams& p);

//Basic data
private:
	std::vector<const BusStop*> busStops;
	double waitAtStopMS;
	double BUS_STOP_WAIT_TIME;// total waiting time

	//Serialization, not implemented
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil){};
	virtual void unpack(UnPackageUtils& unpackageUtil){};
	virtual void packProxy(PackageUtils& packageUtil){};
	virtual void unpackProxy(UnPackageUtils& unpackageUtil){};
#endif

};



}
