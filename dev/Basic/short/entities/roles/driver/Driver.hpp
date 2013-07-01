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
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "perception/FixedDelayed.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "util/DynamicVector.hpp"

#include "entities/models/CarFollowModel.hpp"
#include "entities/models/LaneChangeModel.hpp"
#include "entities/models/IntersectionDrivingModel.hpp"
#include "DriverUpdateParams.hpp"
#include "DriverFacets.hpp"


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
class Driver : public sim_mob::Role {
//Internal classes
private:
	//Helper class for grouping a Node and a Point2D together.
	class NodePoint {
	public:
		Point2D point;
		const Node* node;
		NodePoint() : point(0,0), node(nullptr) {}
	};


//Constructor and overridden methods.
public:
	const static int distanceInFront = 3000;
	const static int distanceBehind = 500;
	const static int maxVisibleDis = 5000;

	Driver(Person* parent, sim_mob::MutexStrategy mtxStrat, sim_mob::DriverBehavior* behavior = nullptr, sim_mob::DriverMovement* movement = nullptr, Role::type roleType_ = RL_DRIVER, std::string roleName_ = "driver");
	virtual ~Driver();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual implementations
	virtual UpdateParams& make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
	virtual std::vector<sim_mob::BufferedBase*> getDriverInternalParams();


//Buffered data
public:
	Shared<const Lane*> currLane_;
	Shared<double> currLaneOffset_;
	Shared<double> currLaneLength_;
	Shared<bool> isInIntersection;

	//need to store these values in the double buffer, because it is needed by other drivers.
	Shared<double> latMovement;
	Shared<double> fwdVelocity;
	Shared<double> latVelocity;
	Shared<double> fwdAccel;
	Shared<LANE_CHANGE_SIDE> turningDirection;

	//for fmod request
	Shared<std::string> stop_event_time;
	Shared<int> stop_event_type;
	Shared<int> stop_event_scheduleid;
	Shared<int> stop_event_nodeid;
	Shared< std::vector<int> > stop_event_lastBoardingPassengers;
	Shared< std::vector<int> > stop_event_lastAlightingPassengers;

	Vehicle* getVehicle() { return vehicle; }
//
public:
	double startTime;
	bool isAleadyStarted;
//Basic data
protected:
	//unsigned int currTimeMS;
	//Pointer to the vehicle this driver is controlling.
	Vehicle* vehicle;
	//This should be done through the Role class itself; for now, I'm just forcing
	//  it so that we can get the mid-term working. ~Seth
	virtual Vehicle* getResource() { return vehicle; }


protected:
	//Temporary variable which will be flushed each time tick. We save it
	// here to avoid constantly allocating and clearing memory each time tick.
	DriverUpdateParams params;

private:
//	//Sample stored data which takes reaction time into account.
//
//	int lastIndex;
	size_t reacTime;
	FixedDelayed<double> *perceivedFwdVel;
	FixedDelayed<double> *perceivedFwdAcc;
	FixedDelayed<double> *perceivedVelOfFwdCar;
	FixedDelayed<double> *perceivedAccOfFwdCar;
	FixedDelayed<double> *perceivedDistToFwdCar;
	FixedDelayed<sim_mob::TrafficColor> *perceivedTrafficColor;
	FixedDelayed<double> *perceivedDistToTrafficSignal;

	NodePoint origin;
	NodePoint goal;    //first, assume that each vehicle moves towards a goal

public:
//	double maxLaneSpeed;
//	//for coordinate transform
//	void setParentBufferedData();			///<set next data to parent buffer data

	Agent* getDriverParent(const Driver *self) { return self->parent; }

public:
	//TODO: This may be risky, as it exposes non-buffered properties to other vehicles.
	const Vehicle* getVehicle() const {return vehicle;}

	//This is probably ok.
	const double getVehicleLength() const { return vehicle->length; }

private:
	//Serialization
	friend class DriverBehavior;
	friend class DriverMovement;

#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif

};



}
