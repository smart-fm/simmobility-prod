//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include "conf/settings/DisableMPI.h"
#include "Driver.hpp"
#include "DriverUpdateParams.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "entities/conflux/SegmentStats.hpp"

namespace sim_mob {
namespace medium {

/**
 * Behaviour Facet class for mid-term Driver role
 *
 * \author Melani Jayasuriya
 * \author Harish Loganathan
 */
class DriverBehavior: public sim_mob::BehaviorFacet {
public:
	explicit DriverBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverBehavior();

	/**
	 * Virtual overrides
	 */
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	sim_mob::medium::Driver* getParentDriver() const {
		return parentDriver;
	}

	void setParentDriver(sim_mob::medium::Driver* parentDriver) {
		this->parentDriver = parentDriver;
	}

protected:
	/**
	 * Pointer to the parent Driver role.
	 */
	sim_mob::medium::Driver* parentDriver;

};

/**
 * Movement Facet class for mid-term Driver role
 *
 * \author Melani Jayasuriya
 * \author Harish Loganathan
 */
class DriverMovement: public sim_mob::MovementFacet {
public:
	explicit DriverMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverMovement();

	/**
	 * Virtual overrides
	 */
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	/**
	 * For moving into a new link after getting permission from the managing conflux
	 */
	virtual void flowIntoNextLinkIfPossible(UpdateParams& p);

	void setParentData(DriverUpdateParams& p);	///<set next data to parent buffer data
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

	double getPositionInSegment() const {
		return this->distToSegmentEnd;
	}
	void setPositionInSegment(double dist2end){
		this->distToSegmentEnd = dist2end;
	}

protected:
	typedef std::vector<sim_mob::SegmentStats*> Path;

	/**
	 * Pointer to the parent Driver role.
	 */
	sim_mob::medium::Driver* parentDriver;

	/**
	 * Path of this driver
	 */
	Path path;

	Path::iterator currSegStatIt;

	const Lane* currLane;
	const Lane* nextLaneInNextSegment;
	bool isQueuing;
	double vehicleLength;
	double velocity;

	//distance to the end of the current segment.
	double distToSegmentEnd;

	mutable std::stringstream DebugStream;

	const sim_mob::RoadSegment* getCurrSegment() const;
	const sim_mob::RoadSegment* getNextSegment(bool inSameLink=true) const;
	const sim_mob::RoadSegment* getSecondSegmentAhead() const;
	const sim_mob::RoadSegment* getPrevSegment(bool inSameLink=true) const;
	bool hasNextSegment(bool inSameLink) const;
	void advanceInPath();
	bool isPathCompleted() const;
	void moveFwdInSegment(double fwdDisplacement);
	bool isConnectedToNextSeg(const Lane* lane, const RoadSegment* nextRdSeg);
	void addToQueue(const Lane* lane);
	void removeFromQueue();
	const sim_mob::Lane* getBestTargetLane(const RoadSegment* targetRdSeg, const RoadSegment* nextRdSeg);
	double getInitialQueueLength(const Lane* l);

	//Note: insert and remove incident functions should probably be in Confluxes. To be updated when actual incident functionality is implemented.
	/**
	 * Inserts an Incident by updating the flow rate for all lanes of a road segment to a new value.
	 *
	 * @param rdSeg roadSegment to insert incident
	 * @param newFlowRate new flow rate to be updated
	 */
	void insertIncident(const RoadSegment* rdSeg, double newFlowRate);

	/**
	 * Removes a previously inserted incident by restoring the flow rate of each lane of a road segment to normal values
	 *
	 * @param rdSeg road segment to remove incident
	 */
	void removeIncident(const RoadSegment* rdSeg);

	/**
	 * Updates travel time for this driver for the link which he has just exited from.
	 *
	 * @param prevSeg the last segment in the link from which the driver has just exited
	 * @param linkExitTimeSec time at which the link was exited
	 */
	void updateLinkTravelTimes(const RoadSegment* prevSeg, double linkExitTimeSec);

	/**
	 * Updates travel time for this driver for the road segment which he has just exited from.
	 *
	 * @param prevSeg the segment from which the driver has just exited
	 * @param linkExitTimeSec time at which the segment was exited
	 */
	void updateRdSegTravelTimes(const RoadSegment* prevSeg, double linkExitTimeSec);
};

} /* namespace medium */
} /* namespace sim_mob */
