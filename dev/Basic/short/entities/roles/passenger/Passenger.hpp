/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "conf/settings/DisableMPI.h"
#include "../driver/BusDriver.hpp"
#include "buffering/BufferedDataManager.hpp"

namespace sim_mob
{

/**
 * A Person in the Passenger role is likely just waiting for his or her bus stop.
 * \author Meenu
 */
//class BusDriver;
class BusStop;
class Person;
class Bus;
class Passenger;
class PackageUtils;
class UnPackageUtils;

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
	Passenger(Agent* parent, MutexStrategy mtxStrat,std::string roleName_ = "passenger");
	virtual ~Passenger() {}

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;
	//virtual void update(timeslice now);
	void setParentBufferedData();
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual UpdateParams& make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	bool isAtBusStop();
	bool isBusBoarded();
	bool isDestBusStopReached();
	Point2D getXYPosition();
	Point2D getDestPosition();
	const BusStop* getOriginBusStop() { return OriginBusStop; }
	const BusStop* getDestBusStop() { return DestBusStop; }

	///NOTE: These boarding/alighting functions are called from BusDriver and used to transfer data.

	///passenger boards the approaching bus if it goes to the destination
	///.Decision to board is made when the bus approaches the busstop.So the first
	///bus which would take to the destination would be boarded
	bool PassengerBoardBus_Normal(BusDriver* busdriver,std::vector<const BusStop*> busStops);

	bool PassengerAlightBus(BusDriver* busdriver);

	///passenger has initially chosen which bus lines to board and passenger boards
	///the bus based on this pre-decision.Passenger makes the decision to board a bussline
	///based on the path of the bus if the bus goes to the destination and chooses the busline based on shortest distance
	bool PassengerBoardBus_Choice(BusDriver* busdriver);

	///to find waiting time for passengers who have boarded bus,time difference between
	/// time of reaching busstop and time bus reaches busstop
	void findWaitingTime(Bus* bus);

	///finds the nearest busstop for the given node,As passenger origin and destination is given in terms of nodes
	BusStop* setBusStopXY(const Node* node);

	///finds which bus lines the passenger should take after reaching the busstop
	///based on bussline info at the busstop
	void FindBusLines();

	std::vector<Busline*> ReturnBusLines();

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil){}
	virtual void unpack(UnPackageUtils& unpackageUtil){}
	virtual void packProxy(PackageUtils& packageUtil){}
	virtual void unpackProxy(UnPackageUtils& unpackageUtil){}
#endif

public:
	int alighting_Frame;// to record the alighting_frame for each individual person
	sim_mob::Shared<BusDriver*> busdriver;///passenger should have info about the driver
	sim_mob::Shared<bool> BoardedBus;
	sim_mob::Shared<bool> AlightedBus;
private:
	PassengerUpdateParams params;
	BusStop* OriginBusStop;///busstop passenger is starting the trip from
    BusStop* DestBusStop;///busstop passenger is ending the trip

	std::vector<Busline*> BuslinesToTake;///buslines passenger can take;decided by passenger upon reaching busstop
	double WaitingTime;
	double TimeOfReachingBusStop;
	///For display purposes: offset this Passenger by a given +x, +y
	Point2D DisplayOffset;

	///for display purpose of alighting passengers
	int displayX;
	int displayY;

	///to display alighted passenger for certain no of frame ticks before removal
	int skip;
};



}
