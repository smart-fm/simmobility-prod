/*
 * DriverFacets.hpp
 *
 *  Created on: Apr 1, 2013
 *      Author: harish
 */

#pragma once
#include "conf/settings/DisableMPI.h"
#include "entities/roles/RoleFacets.hpp"
#include "DriverUpdateParams.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "Driver.hpp"

namespace sim_mob {
namespace medium {

class DriverBehavior: public sim_mob::BehaviorFacet {
public:
	explicit DriverBehavior(sim_mob::Agent* parentAgent = nullptr);
	virtual ~DriverBehavior();

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);

private:
	sim_mob::medium::Driver* parentDriver;

};

class DriverMovement: public sim_mob::MovementFacet {
public:
	explicit DriverMovement(sim_mob::Agent* parentAgent = nullptr);
	virtual ~DriverMovement();

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);

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
	bool advanceMovingVehicleWithInitialQ(DriverUpdateParams& p);
	void getSegSpeed();
	int getOutputCounter(const Lane* l);
	double getOutputFlowRate(const Lane* l);
	double getAcceptRate(const Lane* l);
	double getQueueLength(const Lane* l);
	double getLastAccept(const Lane* l);
	void setLastAccept(const Lane* l, double lastAccept);
	void updateFlow(const RoadSegment* rdSeg, double startPos, double endPos);
	Vehicle* initializePath(bool allocateVehicle);
	void setOrigin(DriverUpdateParams& p);

private:
	sim_mob::medium::Driver* parentDriver;
	Vehicle* vehicle;
	const Lane* currLane;
	const Lane* nextLaneInNextSegment;

	mutable std::stringstream DebugStream;

	bool isConnectedToNextSeg(const Lane* lane, const RoadSegment* nextRdSeg);
	void addToQueue(const Lane* lane);
	void removeFromQueue();
	const sim_mob::Lane* getBestTargetLane(const RoadSegment* targetRdSeg, const RoadSegment* nextRdSeg);
	double getInitialQueueLength(const Lane* l);
	void insertIncident(const RoadSegment* rdSeg, double newFlowRate);
	void removeIncident(const RoadSegment* rdSeg);

};

} /* namespace medium */
} /* namespace sim_mob */
