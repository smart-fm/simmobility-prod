/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include "entities/roles/Role.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "entities/roles/driver/BusDriver.hpp"


namespace sim_mob
{

/**
 * A Person in the Passenger role is likely just waiting for his or her bus stop.
 *
 * \author Meenu

 *  \note
 *  This is a skeleton class. All functions are defined in this header file.
 *  When this class's full functionality is added, these header-defined functions should
 *  be moved into a separate cpp file.
 */
//Helper struct
class BusDriver;

class PackageUtils;
class UnPackageUtils;
class BusStop;
class Person;
class Bus;
class Passenger;
/*enum PassengerStage {
	WaitingAtBusStop= 0,
	BoardedBus = 1,
	AlightedBus=2,
	DestReached=3,
};*/

struct PassengerUpdateParams : public sim_mob::UpdateParams {
	explicit PassengerUpdateParams(boost::mt19937& gen) : UpdateParams(gen) {}
	virtual ~PassengerUpdateParams() {}

	virtual void reset(timeslice now)
	{
		sim_mob::UpdateParams::reset(now);

	}

#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const PassengerUpdateParams* params);

	static void unpack(UnPackageUtils& unpackage, PassengerUpdateParams* params);
#endif
};
class Passenger : public sim_mob::Role {
public:
	//added by meenu
	//static Shared<int> estimated_boarding_passengers_no;
    PassengerUpdateParams params;
	Passenger(Agent* parent, MutexStrategy mtxStrat,std::string roleName_ = "passenger");
	virtual ~Passenger();
	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;
	virtual void update(timeslice now);
	void setParentBufferedData();
	//todo
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual UpdateParams& make_frame_tick_params(timeslice now);

   virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
   bool isAtBusStop();
   bool isDestBusStopReached();
   Point2D getXYPosition();
   Point2D getDestPosition();
   bool PassengerBoardBus(Bus* bus,BusDriver* busdriver,Person* p,std::vector<const BusStop*> busStops,int k);
   bool PassengerAlightBus(Bus* bus,int xpos_approachingbusstop,int ypos_approachingbusstop,BusDriver* busdriver);
   bool isBusBoarded();
   double findWaitingTime(Bus* bus);
   void EstimateBoardingAlightingPassengers(Bus* bus);

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil){}
	virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil){}
	virtual void unpackProxy(UnPackageUtils& unpackageUtil){}


#endif
private:
	    sim_mob::Shared<bool> WaitingAtBusStop;
	   	sim_mob::Shared<bool> boardedBus;
	   	sim_mob::Shared<bool> alightedBus;
	   	sim_mob::Shared<bool> DestReached;
	   	sim_mob::Shared<BusDriver*> busdriver;//passenger should have info about the driver
        sim_mob::Shared<int> random_x;
        sim_mob::Shared<int> random_y;
        double WaitingTime;
        double TimeofReachingBusStop;
};



}
