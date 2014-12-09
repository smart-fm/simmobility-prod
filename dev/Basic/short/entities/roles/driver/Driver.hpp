//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <math.h>
#include <set>

#include "conf/settings/DisableMPI.h"

#include "entities/roles/Role.hpp"
#include "buffering/Shared.hpp"
#include "perception/FixedDelayed.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "util/DynamicVector.hpp"

#include "entities/roles/driver/models/CarFollowModel.hpp"
#include "entities/roles/driver/models/LaneChangeModel.hpp"
#include "entities/roles/driver/models/IntersectionDrivingModel.hpp"
#include "DriverUpdateParams.hpp"
#include "DriverFacets.hpp"
#include "util/Math.hpp"


namespace sim_mob
{

//Forward declarations
class Pedestrian;
class Signal;
class Link;
class RoadSegment;
class Lane;
class Node;
class MultiNode;
class DPoint;
class UpdateParams;
class DriverBehavior;
class DriverMovement;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif


/**
 * \author Wang Xinyuan
 * \author Li Zhemin
 * \author Runmin Xu
 * \author Seth N. Hetu
 * \author Luo Linbo
 * \author LIM Fung Chai
 * \author Zhang Shuai
 * \author Xu Yan
 */
class Driver : public sim_mob::Role , public UpdateWrapper<DriverUpdateParams>{

private:

	//Internal classes
	//Helper class for grouping a Node and a Point2D together.
	class NodePoint {
	public:
		Point2D point;
		const Node* node;
		NodePoint() : point(0,0), node(nullptr) {}
	};

	//Indicates whether the driver is in a loading queue. There isn't actually any data structure to represent this
	//queue. We use the fact that at every time tick, agents are going to be processed sequentially anyway.
	//If this boolean is true, it means that there is no space for it on the road.
	bool isVehicleInLoadingQueue;

	//Indicates whether the position of the vehicle has been found.
	bool isVehiclePositionDefined;

	friend class DriverBehavior;
	friend class DriverMovement;

public:

	const static int distanceInFront = 3000;
	const static int distanceBehind = 5000;
	const static int maxVisibleDis = 5000;

	//Buffered data
	//need to store these values in the double buffer, because it is needed by other drivers.
	Shared<const Lane*> currLane_;
	Shared<bool> isInIntersection;
	Shared<double> currLaneOffset_;
	Shared<double> currLaneLength_;
	Shared<double> latMovement;
	Shared<double> fwdVelocity;
	Shared<double> latVelocity;
	Shared<double> fwdAccel;
	Shared<LANE_CHANGE_SIDE> turningDirection;

	bool isAleadyStarted;
	double startTime;
	double currDistAlongRoadSegment;

	// me is doing yielding, and yieldVehicle is doing nosing
	Driver* yieldVehicle;

	//Pointer to the vehicle this driver is controlling.
	Vehicle* vehicle;

	// driver path-mover split purpose, we save the currPos in the Driver
	DPoint currPos;

	//Sample stored data which takes reaction time into account.
	size_t reacTime;
	FixedDelayed<double> *perceivedFwdVel;
	FixedDelayed<double> *perceivedFwdAcc;
	FixedDelayed<double> *perceivedVelOfFwdCar;
	FixedDelayed<double> *perceivedAccOfFwdCar;
	FixedDelayed<double> *perceivedDistToFwdCar;
	FixedDelayed<sim_mob::TrafficColor> *perceivedTrafficColor;
	FixedDelayed<double> *perceivedDistToTrafficSignal;

	//first, assume that each vehicle moves towards a goal
	NodePoint origin;
	NodePoint goal;


	//for fmod request
	Shared<std::string> stop_event_time;
	Shared<int> stop_event_type;
	Shared<int> stop_event_scheduleid;
	Shared<int> stop_event_nodeid;
	Shared<std::vector<int> > stop_event_lastBoardingPassengers;
	Shared<std::vector<int> > stop_event_lastAlightingPassengers;

	//Constructor and public member functions
	Driver(Person* parent, sim_mob::MutexStrategy mtxStrat, sim_mob::DriverBehavior* behavior = nullptr, sim_mob::DriverMovement* movement = nullptr, Role::type roleType_ = RL_DRIVER, std::string roleName_ = "driver");
	virtual ~Driver();

	void initReactionTime();

	void handleUpdateRequest(MovementFacet* mFacet);

	bool isBus();

	/*
	 * /brief Find the distance from front vehicle.
	 * CAUTION: TS_Vehicles "front" and this vehicle may not be in the same
	 * lane (could be in the left or right neighbor lane), but they have
	 * to be in either the same segment or in a downstream NEIGHBOR
	 * segment.
	 */
	double gapDistance(const Driver* front);

	const Vehicle* getVehicle() const
	{
		return vehicle;
	}
	const double getVehicleLengthCM() const
	{
		return vehicle->getLengthCm();
	}
	const double getVehicleLengthM() const
	{
		return getVehicleLengthCM() / 100.0;
	}

	//Getter method for isVehicleInLoadingQueue
	bool IsVehicleInLoadingQueue()
	{
		return isVehicleInLoadingQueue;
	}

	Vehicle* getVehicle()
	{
		return vehicle;
	}

	Agent* getDriverParent(const Driver *self)
	{
		return self->parent;
	}

	const double getFwdVelocityM() const;

	const DPoint& getCurrPosition() const;

	void rerouteWithPath(const std::vector<sim_mob::WayPoint>& path);

	// for path-mover splitting purpose
	void setCurrPosition(DPoint currPosition);

	/**
	 * /brief reset reaction time
	 * /param t time in ms
	 */
	void resetReacTime(double t);

	//Virtual implementations
	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;
	virtual void make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
	virtual std::vector<sim_mob::BufferedBase*> getDriverInternalParams();
	//handle parent event from other agents
	virtual void onParentEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);
	///Reroute around a blacklisted set of RoadSegments. See Role's comments for more information.
	virtual void rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment*>& blacklisted);

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif

};



}
