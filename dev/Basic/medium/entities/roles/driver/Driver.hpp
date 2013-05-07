/* Copyright Singapore-MIT Alliance for Research and Technology */


#pragma once

#include "conf/settings/DisableMPI.h"

#include <vector>
#include <map>
#include "entities/roles/Role.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"

#include "entities/roles/Role.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "util/DynamicVector.hpp"
#include "entities/models/IntersectionDrivingModel.hpp"
#include "DriverUpdateParams.hpp"
#include "entities/AuraManager.hpp"

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
 * \author Melani Jayasuriya
 * \author Harish Loganathan
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
	std::stringstream ss;
	//int remainingTimeToComplete;

	//Driver(Agent* parent);
	Driver(Agent* parent, MutexStrategy mtxStrat);
	virtual ~Driver();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now) { throw std::runtime_error("frame_tick_output_mpi not implemented in Driver."); }
	virtual UpdateParams& make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	void setParentData();			///<set next data to parent buffer data

	//TODO: This may be risky, as it exposes non-buffered properties to other vehicles.
	const sim_mob::Vehicle* getVehicle() const {return vehicle;}
	double getTimeSpentInTick(DriverUpdateParams& p);
	void stepFwdInTime(DriverUpdateParams& p, double time);
	bool advance(DriverUpdateParams& p);
	bool moveToNextSegment(DriverUpdateParams& p);
	bool canGoToNextRdSeg(DriverUpdateParams& p, double t);
	void moveInQueue();
	bool moveInSegment(DriverUpdateParams& p2, double distance);
	bool advanceQueuingVehicle(DriverUpdateParams& p);
	bool advanceMovingVehicle(DriverUpdateParams& p);
	bool advanceMovingVehicleWithInitialQ(DriverUpdateParams& p2);
	void getSegSpeed();
	int getOutputCounter(const Lane* l);
	double getOutputFlowRate(const Lane* l);
	double getAcceptRate(const Lane* l);
	double getQueueLength(const Lane* l);
	double getLastAccept(const Lane* l);
	void setLastAccept(const Lane* l, double lastAccept);
	void updateFlow(const RoadSegment* rdSeg, double startPos, double endPos);

private:
	//void chooseNextLaneForNextLink(DriverUpdateParams& p);
	//bool update_movement(DriverUpdateParams& params, frame_t frameNumber);       ///<Called to move vehicles forward.
	//bool update_post_movement(DriverUpdateParams& params, frame_t frameNumber);       ///<Called to deal with the consequences of moving forwards.
	//void intersectionDriving(DriverUpdateParams& p);
	//void justLeftIntersection(DriverUpdateParams& p);
	//void syncCurrLaneCachedInfo(DriverUpdateParams& p);
	//void calculateIntersectionTrajectory(DPoint movingFrom, double overflow);
	//double speed_density_function(unsigned int numVehicles); ///<Called to compute the required speed of the driver from the density of the current road segment's traffic density
	bool isConnectedToNextSeg(const Lane* lane, const RoadSegment* nextRdSeg);

	void addToQueue(const Lane* lane);
	void removeFromQueue();
	const sim_mob::Lane* getBestTargetLane(const RoadSegment* targetRdSeg, const RoadSegment* nextRdSeg);
	double getInitialQueueLength(const Lane* l);
	void insertIncident(const RoadSegment* rdSeg, double newFlowRate);
	void removeIncident(const RoadSegment* rdSeg);

protected:
	//virtual double updatePositionOnLink(DriverUpdateParams& p);
	virtual Vehicle* initializePath(bool allocateVehicle);

	void setOrigin(DriverUpdateParams& p);
	//void chooseLaneToStart();
public:
	/*
	 * Making params public to expose information like justChangedToNewSegment,
	 * justMovedIntoIntersection etc available for density calculation. ~ Harish
	 */
	medium::DriverUpdateParams params;
	//to be moved to a DriverUpdateParam later
	const Lane* currLane;

protected:
	//const Lane* nextLaneInNextLink; //to be removed-no longer needed for mid-term
	const Lane* nextLaneInNextSegment;
	//size_t targetLaneIndex;
	//size_t currLaneIndex;
	mutable std::stringstream DebugStream;
	NodePoint origin;
	NodePoint goal;

protected:
	Vehicle* vehicle;
};


}}
