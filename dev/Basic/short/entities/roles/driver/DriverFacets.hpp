//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * DriverFacets.hpp
 *
 *  Created on: May 15th, 2013
 *      Author: Yao Jin
 */

#pragma once
#include "conf/settings/DisableMPI.h"
#include "entities/roles/RoleFacets.hpp"
#include "entities/roles/Role.hpp"
#include "DriverUpdateParams.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "Driver.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "geospatial/RoadItem.hpp"
#include "entities/IncidentStatus.hpp"
#include "geospatial/Incident.hpp"

namespace sim_mob {

class DriverBehavior: public sim_mob::BehaviorFacet {
public:
	explicit DriverBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverBehavior();

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);

	Driver* getParentDriver() const {
		return parentDriver;
	}

	void setParentDriver(Driver* parentDriver) {
		this->parentDriver = parentDriver;
	}

protected:
	Driver* parentDriver;

};

class DriverMovement: public sim_mob::MovementFacet {
//public:
//	const static int distanceInFront = 3000;
//	const static int distanceBehind = 500;
//	const static int maxVisibleDis = 5000;

public:
	explicit DriverMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverMovement();

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void flowIntoNextLinkIfPossible(UpdateParams& p);

	Driver* getParentDriver() const {
		return parentDriver;
	}

	void setParentDriver(Driver* parentDriver) {
		this->parentDriver = parentDriver;
	}

protected:
	Driver* parentDriver;

protected:
	//Update models
	LaneChangeModel* lcModel;
	CarFollowModel* cfModel;
	IntersectionDrivingModel* intModel;

private:
	//Sample stored data which takes reaction time into account.

	int lastIndex;
	double disToFwdVehicleLastFrame; //to find whether vehicle is going to crash in current frame.
	                                     //so distance in last frame need to be remembered.

public:
	double maxLaneSpeed;
	//for coordinate transform
	void setParentBufferedData();			///<set next data to parent buffer data

	/// get nearest obstacle in perceptionDis
	const sim_mob::RoadItem* getRoadItemByDistance(sim_mob::RoadItemType type,double perceptionDis,double &dis,bool isInSameLink=true);

private:
	void check_and_set_min_car_dist(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other);
	static void check_and_set_min_nextlink_car_dist(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other);

	//More update methods
	bool update_sensors(DriverUpdateParams& params, timeslice now);        ///<Called to update things we _sense_, like nearby vehicles.
	bool update_movement(DriverUpdateParams& params, timeslice now);       ///<Called to move vehicles forward.
	bool update_post_movement(DriverUpdateParams& params, timeslice now);  ///<Called to deal with the consequences of moving forwards.

    double currLinkOffset;
	size_t targetLaneIndex;

	const Lane* nextLaneInNextLink;
private:
	double distanceToNextStop();
	bool sArriveStop();

public:
//	//TODO: This may be risky, as it exposes non-buffered properties to other vehicles.
//	const Vehicle* getVehicle() const {return vehicle;}
//
//	//This is probably ok.
//	const double getVehicleLength() const { return vehicle->length; }

	void updateAdjacentLanes(DriverUpdateParams& p);
	void updatePositionDuringLaneChange(DriverUpdateParams& p, LANE_CHANGE_SIDE relative);

protected:
	virtual double updatePositionOnLink(DriverUpdateParams& p);
	virtual double linkDriving(DriverUpdateParams& p);
	virtual double dwellTimeCalculation(int A,int B,int delta_bay,int delta_full,int Pfront,int no_of_passengers); // dwell time calculation module

	sim_mob::Vehicle* initializePath(bool allocateVehicle);

	void resetPath(DriverUpdateParams& p);
	void setOrigin(DriverUpdateParams& p);

	int checkIncidentStatus(DriverUpdateParams& p, timeslice now);

	//Helper: for special strings
	void initLoopSpecialString(std::vector<WayPoint>& path, const std::string& value);
	void initTripChainSpecialString(const std::string& value);
	NearestVehicle & nearestVehicle(DriverUpdateParams& p);

	void perceivedDataProcess(NearestVehicle & nv, DriverUpdateParams& params);

private:
	bool AvoidCrashWhenLaneChanging(DriverUpdateParams& p);
	bool isCloseToLinkEnd(DriverUpdateParams& p) const;
	bool isPedestrianOnTargetCrossing() const;
	void chooseNextLaneForNextLink(DriverUpdateParams& p);
	void calculateIntersectionTrajectory(DPoint movingFrom, double overflow);

	//A bit verbose, but only used in 1 or 2 places.
	void syncCurrLaneCachedInfo(DriverUpdateParams& p);
	void justLeftIntersection(DriverUpdateParams& p);

	void updateVelocity();
	void setBackToOrigin();

	void updateNearbyAgents(DriverUpdateParams& params);
	void updateNearbyDriver(DriverUpdateParams& params, const sim_mob::Person* other, const sim_mob::Driver* other_driver);
	void updateNearbyPedestrian(DriverUpdateParams& params, const sim_mob::Person* other, const sim_mob::Pedestrian* pedestrian);

	//void updateCurrLaneLength(DriverUpdateParams& p);
	void updateDisToLaneEnd();

	void saveCurrTrafficSignal();

	void setTrafficSignalParams(DriverUpdateParams& p);
	void intersectionDriving(DriverUpdateParams& p);


	void findCrossing(DriverUpdateParams& p);

	bool processFMODSchedule(FMODSchedule* schedule, DriverUpdateParams& p);

public:
	double targetSpeed;			//the speed which the vehicle is going to achieve

	void intersectionVelocityUpdate();

	//This always returns the lane we are moving towards; regardless of if we've passed the
	//  halfway point or not.
	LANE_CHANGE_SIDE getCurrLaneChangeDirection() const;

	//This, however, returns where we are relative to the center of our own lane.
	// I'm sure we can do this in a less confusion fashion later.
	LANE_CHANGE_SIDE getCurrLaneSideRelativeToCenter() const;

private:
	//The current traffic signal in our Segment. May be null.
	const Signal* trafficSignal;

	//For generating a debugging trace
	mutable std::stringstream DebugStream;

	//incident response plan
	sim_mob::IncidentStatus incidentStatus;
};
}
