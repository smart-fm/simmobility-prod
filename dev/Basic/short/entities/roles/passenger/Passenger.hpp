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
 */

class BusDriver;
class PackageUtils;
class UnPackageUtils;
class BusStop;
class Person;
class Bus;
class Passenger;

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

    PassengerUpdateParams params;
	Passenger(Agent* parent, MutexStrategy mtxStrat,std::string roleName_ = "passenger");
	virtual ~Passenger();
	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;
	virtual void update(timeslice now);
	void setParentBufferedData();

	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual UpdateParams& make_frame_tick_params(timeslice now);

	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
	bool isAtBusStop();//to check if at busstop
	bool isDestBusStopReached();//to check if destination is reached
	Point2D getXYPosition();//to get current x,y position of passenger
	Point2D getDestPosition();//to get dest x,y position of passenger
	bool PassengerBoardBus(Bus* bus,BusDriver* busdriver,Person* p,std::vector<const BusStop*> busStops,int k);
	bool PassengerAlightBus(Bus* bus,int xpos_approachingbusstop,int ypos_approachingbusstop,BusDriver* busdriver);
	bool isBusBoarded();//to check if boarded
	void findWaitingTime(Bus* bus);//to find waiting time

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil){}
	virtual void unpack(UnPackageUtils& unpackageUtil){}
	virtual void packProxy(PackageUtils& packageUtil){}
	virtual void unpackProxy(UnPackageUtils& unpackageUtil){}


#endif
private:
	sim_mob::Shared<bool> boardedBus;//true if passenger has boarded bus
	sim_mob::Shared<bool> alightedBus;//true if passenger has alighted bus
	sim_mob::Shared<BusDriver*> busdriver;//passenger on bus should have info about the driver
	sim_mob::Shared<int> random_x;//for displaying passengers at the bus stop having the same trip chain so that they are spread out
	sim_mob::Shared<int> random_y;
	double WaitingTime;
	double TimeofReachingBusStop;
	Point2D destination;
};



}
