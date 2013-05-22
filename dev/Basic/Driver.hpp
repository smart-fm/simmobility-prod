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
#include "DriverUpdateParams.hpp"
#include "DriverFacets.hpp"

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

class DriverBehavior;
class DriverMovement;
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

	Driver(Agent* parent, MutexStrategy mtxStrat, sim_mob::medium::DriverBehavior* behavior = nullptr, sim_mob::medium::DriverMovement* movement = nullptr);
	virtual ~Driver();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual overrides
	virtual UpdateParams& make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	void setParentData();			///<set next data to parent buffer data

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
	bool isConnectedToNextSeg(const Lane* lane, const RoadSegment* nextRdSeg);

	void addToQueue(const Lane* lane);
	void removeFromQueue();
	const sim_mob::Lane* getBestTargetLane(const RoadSegment* targetRdSeg, const RoadSegment* nextRdSeg);
	double getInitialQueueLength(const Lane* l);
	void insertIncident(const RoadSegment* rdSeg, double newFlowRate);
	void removeIncident(const RoadSegment* rdSeg);

protected:
	//virtual double updatePositionOnLink(DriverUpdateParams& p);
	Vehicle* initializePath(bool allocateVehicle);

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

private:
	//const Lane* nextLaneInNextLink; //to be removed-no longer needed for mid-term
	const Lane* nextLaneInNextSegment;
	Vehicle* vehicle;
	//size_t targetLaneIndex;
	//size_t currLaneIndex;
	mutable std::stringstream DebugStream;
	NodePoint origin;
	NodePoint goal;

protected:
	friend class DriverBehavior;
	friend class DriverMovement;
};


} // namespace medium
} // namespace sim_mob
