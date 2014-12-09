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
#include "IncidentPerformer.hpp"
#include "FmodSchedulesPerformer.hpp"
#include "entities/amodController/AMODController.hpp"
#include "entities/roles/driver/models/CarFollowModel.hpp"

namespace sim_mob
{

class CarFollowModel;

class DriverBehavior: public sim_mob::BehaviorFacet
{
public:
	explicit DriverBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverBehavior();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	Driver* getParentDriver() const
	{
		return parentDriver;
	}

	void setParentDriver(Driver* parentDriver)
	{
		if (!parentDriver)
		{
			throw std::runtime_error("parentDriver cannot be NULL");
		}
		this->parentDriver = parentDriver;
	}

protected:
	Driver* parentDriver;

};

class DriverMovement: public sim_mob::MovementFacet
{
public:

	explicit DriverMovement(sim_mob::Person* parentAgent = nullptr,
			Driver* parentDriver = nullptr);
	virtual ~DriverMovement();

	//Virtual overrides
	virtual void init();
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	// mark startTimeand origin
	virtual TravelMetric & startTravelTimeMetric();

	//	mark the destination and end time and travel time
	virtual TravelMetric & finalizeTravelTimeMetric();

	Driver* getParentDriver() const
	{
		return parentDriver;
	}

	void setParentDriver(Driver* parentDriver)
	{
		if (!parentDriver)
		{
			throw std::runtime_error("parentDriver cannot be NULL");
		}
		this->parentDriver = parentDriver;
	}

	CarFollowModel* getCarFollowModel()
	{
		return cfModel;
	}

	//for coordinate transform
	//set next data to parent buffer data
	void setParentBufferedData();

	//Call once
	void initPath(std::vector<sim_mob::WayPoint> wp_path, int startLaneID);

	void initPathWithInitSeg(std::vector<sim_mob::WayPoint> wp_path,
			int startLaneID, int segId, int initPer, int initSpeed);

	void resetPath(std::vector<sim_mob::WayPoint> wp_path);

	const sim_mob::RoadSegment* hasNextSegment(bool inSameLink) const;

	DPoint getPosition();
	/**
	 * get nearest obstacle in perceptionDis
	 * @param type is obstacle type, currently only two types are BusStop and Incident.
	 * @param dis is used to retrieved real distance from driver to incident obstacle.
	 * @param perceptionDis is search range . default value is 200 meters
	 * @return true if inserting successfully .
	 */
	const sim_mob::RoadItem* getRoadItemByDistance(sim_mob::RoadItemType type,
			double &dis, double perceptionDis = 20000,
			bool isInSameLink = true);
	/**
	 *  @brief get distance to nearest forward stop point in the link
	 *  @param perceptionDis perception distance
	 *  @return -1 not find bus, > 0 find and distance to stop point
	 */
	double getDisToStopPoint(double perceptionDis = 20000);
	/**
	 *  /brief get lanes connect to segment at look ahead distance
	 *  /param distance look ahead distance from current position
	 *  /param lanePool store found lanes
	 */
	void getLanesConnectToLookAheadDis(double distance,
			std::vector<sim_mob::Lane*>& lanePool);

	// check lane connect to rs
	// lanes' segment shall connect ro rs
	bool laneConnectToSegment(sim_mob::Lane* lane,
			const sim_mob::RoadSegment* rs);

	void updateAdjacentLanes(DriverUpdateParams& p);

	void updateLateralMovement(DriverUpdateParams& p);

	/**
	 *   @brief sync data after lane changing movement completed
	 */
	void syncInfoLateralMove(DriverUpdateParams& p);

	void updatePosDuringLaneChange(DriverUpdateParams& p);

	///Reroutes around a given blacklisted set of RoadSegments. See Role for documentation.
	void rerouteWithBlacklist(
			const std::vector<const sim_mob::RoadSegment*>& blacklisted);

	void rerouteWithPath(const std::vector<sim_mob::WayPoint>& path);

	void intersectionVelocityUpdate();

	//This always returns the lane we are moving towards; regardless of if we've passed the
	//  halfway point or not.
	LANE_CHANGE_SIDE getCurrLaneChangeDirection() const;

	//This, however, returns where we are relative to the center of our own lane.
	// I'm sure we can do this in a less confusion fashion later.
	LANE_CHANGE_SIDE getCurrLaneSideRelativeToCenter() const;

	// DriverPathMover
	DriverPathMover fwdDriverMovement;

	//perform incident response
	IncidentPerformer incidentPerformer;

protected:

	virtual double updatePositionOnLink(DriverUpdateParams& p);

	/*
	 *  /brief do lane change and car follow
	 */
	void calcVehicleStates(DriverUpdateParams& p);

	/**
	 *  @brief calculate distance to stop point
	 */
	void calcDistanceToSP(DriverUpdateParams& p);

	/*
	 *  /brief Calculate new location and speed after an iteration based on its
	 * 	       current location, speed and acceleration.
	 */
	double move(DriverUpdateParams& p);

	virtual double dwellTimeCalculation(int A, int B, int delta_bay,
			int delta_full, int Pfront, int no_of_passengers); // dwell time calculation module

	sim_mob::Vehicle* initializePath(bool allocateVehicle);

	void setOrigin(DriverUpdateParams& p);

	///Set the internal rrRegions array from the current path.
	///This effectively converts a list of RoadSegments into a (much smaller) list of Regions.
	///This will trigger communication with the client.
	void setRR_RegionsFromCurrentPath();

	NearestVehicle & nearestVehicle(DriverUpdateParams& p);

	void perceivedDataProcess(NearestVehicle & nv, DriverUpdateParams& params);

	//For display purposes only.
	double getAngle() const;

	// Update models
	LaneChangeModel* lcModel;

	CarFollowModel* cfModel;

	IntersectionDrivingModel* intModel;

	//the speed which the vehicle is going to achieve
	double targetSpeed;

private:

	void check_and_set_min_car_dist(NearestVehicle& res, double distance,
			const Vehicle* veh, const Driver* other);

	static void check_and_set_min_nextlink_car_dist(NearestVehicle& res,
			double distance, const Vehicle* veh, const Driver* other);

	void check_and_set_min_car_dist2(NearestVehicle& res, double distance,
			const Vehicle* veh, const Driver* other);

	//More update methods
	bool update_sensors(timeslice now); ///<Called to update things we _sense_, like nearby vehicles.

	bool update_movement(timeslice now);    ///<Called to move vehicles forward.

	bool update_post_movement(timeslice now); ///<Called to deal with the consequences of moving forwards.

	double distanceToNextStop();

	bool sArriveStop();

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

	bool updateNearbyAgent(const sim_mob::Agent* other,
			const sim_mob::Driver* other_driver);

	void updateNearbyAgent(const sim_mob::Agent* other,
			const sim_mob::Pedestrian* pedestrian);

	void updateDisToLaneEnd();

	void saveCurrTrafficSignal();

	void setTrafficSignalParams(DriverUpdateParams& p);

	void intersectionDriving(DriverUpdateParams& p);

	void findCrossing(DriverUpdateParams& p);

	double getDistanceToSegmentEnd() const;

	sim_mob::DynamicVector getCurrPolylineVector() const;

	sim_mob::DynamicVector getCurrPolylineVector2() const;

	//This method is used to check if there is enough space on the lane where a vehicle from the
	//loading queue wants to start its journey
	bool findEmptySpaceAhead();

	Driver* parentDriver;

	//The current traffic signal in our Segment. May be null.
	const Signal* trafficSignal;

	//Sample stored data which takes reaction time into account.
	int lastIndex;

	//to find whether vehicle is going to crash in current frame.
	double disToFwdVehicleLastFrame;

	double currLinkOffset;

	size_t targetLaneIndex;

	const Lane* nextLaneInNextLink;

	//For generating a debugging trace
	mutable std::stringstream DebugStream;

	//Have we sent the list of all regions at least once?
	OneTimeFlag sentAllRegions;

	//The most recently-set path, which will be sent to RoadRunner.
	std::vector<const sim_mob::RoadSegment*> rrPathToSend;
};
}
