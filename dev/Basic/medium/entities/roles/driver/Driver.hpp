/* Copyright Singapore-MIT Alliance for Research and Technology */


#pragma once

#include <vector>
#include "entities/roles/Role.hpp"
#include "geospatial/StreetDirectory.hpp"
#include "GenConfig.h"

#include "entities/roles/Role.hpp"
#include "geospatial/StreetDirectory.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "util/DynamicVector.hpp"
#include "../short/entities/roles/driver/IntersectionDrivingModel.hpp"
#include "DriverUpdateParams.hpp"

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

namespace sim_mob
{

class Agent;
class Person;
class Link;
class RoadSegment;
class Lane;
class Node;
class MultiNode;
class DPoint;
class UpdateParams;

namespace medium
{


/**
 * A medium-term Driver.
 * \author Seth N. Hetu
 */
class Driver : public sim_mob::Role {
private:
	//Helper class for grouping a Node and a Point2D together.
	class NodePoint {
	public:
		Point2D point;
		const Node* node;
		NodePoint() : point(0,0), node(nullptr) {}
	};

public:
	int remainingTimeToComplete;

	//Driver(Agent* parent);
	Driver(Person* parent, MutexStrategy mtxStrat);
	virtual ~Driver();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(frame_t frameNumber) { throw std::runtime_error("Not implemented."); }
	virtual UpdateParams& make_frame_tick_params(frame_t frameNumber, unsigned int currTimeMS);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	void setParentBufferedData();			///<set next data to parent buffer data

	//TODO: This may be risky, as it exposes non-buffered properties to other vehicles.
	const Vehicle* getVehicle() const {return vehicle;}

	void intersectionVelocityUpdate();

private:
	void chooseNextLaneForNextLink(DriverUpdateParams& p);
	bool update_movement(DriverUpdateParams& params, frame_t frameNumber);       ///<Called to move vehicles forward.
	bool update_post_movement(DriverUpdateParams& params, frame_t frameNumber);       ///<Called to deal with the consequences of moving forwards.
	void intersectionDriving(DriverUpdateParams& p);
	void justLeftIntersection(DriverUpdateParams& p);
	void syncCurrLaneCachedInfo(DriverUpdateParams& p);
	void calculateIntersectionTrajectory(DPoint movingFrom, double overflow);

protected:
	virtual double updatePositionOnLink(DriverUpdateParams& p);
	sim_mob::Vehicle* initializePath(bool allocateVehicle);
	//Helper: for special strings
	void initLoopSpecialString(std::vector<WayPoint>& path, const std::string& value);
	void initTripChainSpecialString(const std::string& value);

	void setOrigin(DriverUpdateParams& p);

public:
	//Buffered data
	Shared<const Lane*> currLane_;
	Shared<double> currLaneOffset_;
	Shared<double> currLaneLength_;
	Shared<bool> isInIntersection;
	Shared<double> fwdVelocity;

	/*
	 * Making params public to expose information like justChangedToNewSegment,
	 * justMovedIntoIntersection etc available for density calculation. ~ Harish
	 */
	medium::DriverUpdateParams params;
	//to be moved to a DriverUpdateParam later
	//const Lane* currLane_;
	//double currLaneOffset_;
	//double currLaneLength_;
	//bool isInIntersection;
	//double elapsedSeconds;
	//double fwdVelocity;

	//Handles state information
	//bool justChangedToNewSegment;
	//DPoint TEMP_lastKnownPolypoint;
	//bool justMovedIntoIntersection;
	//double overflowIntoIntersection;

private:
	const Lane* nextLaneInNextLink;
	size_t targetLaneIndex;
	//size_t currLaneIndex;
	mutable std::stringstream DebugStream;
	NodePoint origin;
	NodePoint goal;

protected:
	Vehicle* vehicle;
	IntersectionDrivingModel* intModel;

};


}}
