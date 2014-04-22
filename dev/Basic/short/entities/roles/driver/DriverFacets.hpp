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
#include "DriverPathMover.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "geospatial/RoadItem.hpp"
#include "entities/IncidentStatus.hpp"
#include "geospatial/Incident.hpp"
#include "util/OneTimeFlag.hpp"

namespace sim_mob {

class DriverBehavior: public sim_mob::BehaviorFacet {
public:
	explicit DriverBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverBehavior();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	Driver* getParentDriver() const {
		return parentDriver;
	}

	void setParentDriver(Driver* parentDriver) {
		if(!parentDriver) {
			throw std::runtime_error("parentDriver cannot be NULL");
		}
		this->parentDriver = parentDriver;
	}

protected:
	Driver* parentDriver;

};

class DriverMovement: public sim_mob::MovementFacet {
public:
	explicit DriverMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	Driver* getParentDriver() const {
		return parentDriver;
	}

	void setParentDriver(Driver* parentDriver) {
		if(!parentDriver) {
			throw std::runtime_error("parentDriver cannot be NULL");
		}
		this->parentDriver = parentDriver;
	}

protected:
	Driver* parentDriver;

protected:
	// Update models
	LaneChangeModel* lcModel;
	CarFollowModel* cfModel;
	IntersectionDrivingModel* intModel;

public:
	// DriverPathMover
	DriverPathMover fwdDriverMovement;
private:
	//Sample stored data which takes reaction time into account.
	int lastIndex;
	double disToFwdVehicleLastFrame; //to find whether vehicle is going to crash in current frame.

public:
	double maxLaneSpeed;
	//for coordinate transform
	void setParentBufferedData();			///<set next data to parent buffer data
	//Call once
	void initPath(std::vector<sim_mob::WayPoint> wp_path, int startLaneID);
	void resetPath(std::vector<sim_mob::WayPoint> wp_path);
	const sim_mob::RoadSegment* hasNextSegment(bool inSameLink) const;
	DPoint& getPosition() const;
    /**
      * get nearest obstacle in perceptionDis
      * @param type is obstacle type, currently only two types are BusStop and Incident.
      * @param dis is used to retrieved real distance from driver to incident obstacle.
      * @param perceptionDis is search range . default value is 200 meters
      * @return true if inserting successfully .
      */
	const sim_mob::RoadItem* getRoadItemByDistance(sim_mob::RoadItemType type,double &dis, double perceptionDis=20000,bool isInSameLink=true);

private:
	void check_and_set_min_car_dist(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other);
	static void check_and_set_min_nextlink_car_dist(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other);

	void check_and_set_min_car_dist2(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other);

	//More update methods
	bool update_sensors(timeslice now);        ///<Called to update things we _sense_, like nearby vehicles.
	bool update_movement(timeslice now);       ///<Called to move vehicles forward.
	bool update_post_movement(timeslice now);  ///<Called to deal with the consequences of moving forwards.

    double currLinkOffset;
	size_t targetLaneIndex;

	const Lane* nextLaneInNextLink;
private:
	double distanceToNextStop();
	bool sArriveStop();

public:
//	//TODO: This may be risky, as it exposes non-buffered properties to other vehicles.
//	const Vehicle* getVehicle() const {return vehicle;}

	void updateAdjacentLanes(DriverUpdateParams& p);
	void updatePositionDuringLaneChange(DriverUpdateParams& p, LANE_CHANGE_SIDE relative);

	///Reroutes around a given blacklisted set of RoadSegments. See Role for documentation.
	void rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment*>& blacklisted);

protected:
	virtual double updatePositionOnLink(DriverUpdateParams& p);
	virtual double linkDriving(DriverUpdateParams& p);
	virtual double dwellTimeCalculation(int A,int B,int delta_bay,int delta_full,int Pfront,int no_of_passengers); // dwell time calculation module

	sim_mob::Vehicle* initializePath(bool allocateVehicle);

	void setOrigin(DriverUpdateParams& p);

	void checkIncidentStatus(DriverUpdateParams& p, timeslice now);

	void responseIncidentStatus(DriverUpdateParams& p, timeslice now);

	///Set the internal rrRegions array from the current path.
	///This effectively converts a list of RoadSegments into a (much smaller) list of Regions.
	///This will trigger communication with the client.
	void setRR_RegionsFromCurrentPath();

	//Helper: for special strings
	//NOTE: I am disabling special strings. ~Seth
	NearestVehicle & nearestVehicle(DriverUpdateParams& p);
	void perceivedDataProcess(NearestVehicle & nv, DriverUpdateParams& params);
	double getAngle() const;  ///<For display purposes only.

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

	void updateNearbyAgents();
	bool updateNearbyAgent(const sim_mob::Agent* other, const sim_mob::Driver* other_driver);
	void updateNearbyAgent(const sim_mob::Agent* other, const sim_mob::Pedestrian* pedestrian);
	void updateDisToLaneEnd();

	void saveCurrTrafficSignal();

	void setTrafficSignalParams(DriverUpdateParams& p);
	void intersectionDriving(DriverUpdateParams& p);


	void findCrossing(DriverUpdateParams& p);

	double getDistanceToSegmentEnd() const;
	sim_mob::DynamicVector getCurrPolylineVector() const;
	sim_mob::DynamicVector getCurrPolylineVector2() const;
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

	//Have we sent the list of all regions at least once?
	OneTimeFlag sentAllRegions;

	//The most recently-set path, which will be sent to RoadRunner.
	std::vector<const sim_mob::RoadSegment*> rrPathToSend;
};
}
