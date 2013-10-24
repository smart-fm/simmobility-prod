//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include "conf/settings/DisableMPI.h"
#include "Driver.hpp"
#include "DriverUpdateParams.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "entities/vehicle/Vehicle.hpp"

namespace sim_mob {
namespace medium {

class DriverBehavior: public sim_mob::BehaviorFacet {
public:
	explicit DriverBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverBehavior();

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);

	sim_mob::medium::Driver* getParentDriver() const {
		return parentDriver;
	}

	void setParentDriver(sim_mob::medium::Driver* parentDriver) {
		this->parentDriver = parentDriver;
	}

protected:
	sim_mob::medium::Driver* parentDriver;

};

class DriverMovement: public sim_mob::MovementFacet {
public:
	explicit DriverMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverMovement();

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void flowIntoNextLinkIfPossible(UpdateParams& p);

	void setParentData(DriverUpdateParams& p);			///<set next data to parent buffer data
	double getTimeSpentInTick(DriverUpdateParams& p);
	void stepFwdInTime(DriverUpdateParams& p, double time);
	bool advance(DriverUpdateParams& p);
	bool moveToNextSegment(DriverUpdateParams& p);
	bool canGoToNextRdSeg(DriverUpdateParams& p);
	void moveInQueue();
	bool moveInSegment(DriverUpdateParams& p2, double distance);
	bool advanceQueuingVehicle(DriverUpdateParams& p);
	bool advanceMovingVehicle(DriverUpdateParams& p);
	bool advanceMovingVehicleWithInitialQ(DriverUpdateParams& p);
	void getSegSpeed();
	int getOutputCounter(const Lane* l);
	void setOutputCounter(const Lane* l, int count);
	double getOutputFlowRate(const Lane* l);
	double getAcceptRate(const Lane* l);
	double getQueueLength(const Lane* l);
	double getLastAccept(const Lane* l);
	void setLastAccept(const Lane* l, double lastAccept);
	void updateFlow(const RoadSegment* rdSeg, double startPos, double endPos);
	virtual Vehicle* initializePath(bool allocateVehicle);
	void setOrigin(DriverUpdateParams& p);

	sim_mob::medium::Driver* getParentDriver() const {
		return parentDriver;
	}

	void setParentDriver(sim_mob::medium::Driver* parentDriver) {
		this->parentDriver = parentDriver;
	}

protected:
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
	void updateLinkTravelTimes(const RoadSegment* prevSeg, double linkExitTimeSec);
	void updateRdSegTravelTimes(const RoadSegment* prevSeg, double linkExitTimeSec);
};

} /* namespace medium */
} /* namespace sim_mob */
