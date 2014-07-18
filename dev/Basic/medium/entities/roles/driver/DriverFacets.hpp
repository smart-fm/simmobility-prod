//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include "conf/settings/DisableMPI.h"
#include "Driver.hpp"
#include "DriverUpdateParams.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "MesoPathMover.hpp"

namespace sim_mob {
namespace medium {

class Driver;

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
		if(!parentDriver) {
			throw std::runtime_error("parentDriver cannot be NULL");
		}
		this->parentDriver = parentDriver;
	}

	sim_mob::medium::Driver* getParentDriver();

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
	//debug
	unsigned int sectionId;
public:
	explicit DriverMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverMovement();

	//virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	sim_mob::medium::Driver* getParentDriver() const {
		return parentDriver;
	}

	void setParentDriver(sim_mob::medium::Driver* parentDriver) {
		if(!parentDriver) {
			throw std::runtime_error("parentDriver cannot be NULL");
		}
		this->parentDriver = parentDriver;
	}

	const MesoPathMover & getMesoPathMover() const{
		return pathMover;
	}

	MesoPathMover & getMesoPathMover() {
		return pathMover;
	}


protected:
	/**
	 * Pointer to the parent Driver role.
	 */
	sim_mob::medium::Driver* parentDriver;

	MesoPathMover pathMover;
	const Lane* currLane;
	const Lane* laneInNextSegment;
	bool isQueuing;

	mutable std::stringstream DebugStream;

	/**
	 * For moving into a new link after getting permission from the managing conflux
	 *
	 * @param params driver update params for current tick
	 */
	void flowIntoNextLinkIfPossible(DriverUpdateParams& params);

	/**
	 * sets fields in the parent person so that the conflux can manage the person
	 *
	 * @param params driver update params for current tick
	 */
	void setParentData(DriverUpdateParams& params);	///<set next data to parent buffer data

	/**
	 * adds to the time spent by driver in current tick
	 *
	 * @param params driver update params for current tick
	 */
	void stepFwdInTime(DriverUpdateParams& params, double time);

	/**
	 * moves the driver forward along its path
	 *
	 * @param params driver update params for current tick
	 * @return true if advance was successful; false otherwise
	 */
	bool advance(DriverUpdateParams& params);

	/**
	 * moves driver to the next segment
	 *
	 * @param params driver update params for current tick
	 * @return true if successfully moved to next segment; false otherwise
	 */
	virtual bool moveToNextSegment(DriverUpdateParams& params);

	/**
	 * checks whether the driver can move into the next segment in path
	 *
	 * @param params driver update params for current tick
	 * @param nextSegStats next segment stats in path
	 * @return true if driver can move into the next segment; false otherwise
	 */
	bool canGoToNextRdSeg(DriverUpdateParams& params, const sim_mob::SegmentStats* nextSegStats);

	/**
	 * sets position of driver in queue
	 */
	void moveInQueue();

	/**
	 * move driver forward within the seg stats
	 *
	 * @param distance distance to move forward
	 * @return status of move (success = true, failure = false)
	 */
	bool moveInSegment(double distance);

	/**
	 * move driver forward in queue
	 *
	 * @param params driver update params for current tick
	 * @return status of move (success = true, failure = false)
	 */
	bool advanceQueuingVehicle(DriverUpdateParams& params);

	/**
	 * move driver forward in the moving part of seg stats
	 *
	 * @param params driver update params for current tick
	 * @return status of move (success = true, failure = false)
	 */
	bool advanceMovingVehicle(DriverUpdateParams& params);

	/**
	 * move driver forward in seg stats which had a queue at the start of the current tick
	 * but the queue has dissipated within this tick
	 *
	 * @param params driver update params for current tick
	 * @return status of move (success = true, failure = false)
	 */
	bool advanceMovingVehicleWithInitialQ(DriverUpdateParams& params);

	/**
	 * get the number of vehicles that can move out of a lane in this tick
	 *
	 * @param l lane in segment
	 * @param segStats segment stats corresponding to lane l's segment
	 * @return num. of vehicles that can move out
	 */
	int getOutputCounter(const Lane* lane, const sim_mob::SegmentStats* segStats);

	/**
	 * set number of vehicles that can move out of a lane in this tick
	 *
	 * @param l lane in segment
	 * @param count new value of outpur counter
	 * @param segStats segStats segment stats corresponding to lane l's segment
	 */
	void setOutputCounter(const Lane* lane, int count, const sim_mob::SegmentStats* segStats);

	double getOutputFlowRate(const Lane* lane);
	double getAcceptRate(const Lane* lane, const sim_mob::SegmentStats* segStats);
	double getQueueLength(const Lane* lane);
	double getLastAccept(const Lane* lane, const sim_mob::SegmentStats* segStats);
	void setLastAccept(const Lane* lane, double lastAccept, const sim_mob::SegmentStats* segStats);

	/**
	 * update flow of segment segStats
	 * @param segStats segment stats whose flow is to be updated
	 * @param startPos position of driver at the start of the tick
	 * @param endPos final position of driver
	 */
	void updateFlow(const sim_mob::SegmentStats* segStats, double startPos, double endPos);

	/**
	 * constructs the path for this driver if required.
	 * converts the path into a list of SementStats*.
	 * sets the path in the path mover.
	 * @return true if the path has successfully been set; false otherwise.
	 */
	virtual bool initializePath();

	/**
	 * adds driver to his starting lane
	 * @param params driver update params for current tick
	 */
	void setOrigin(DriverUpdateParams& params);

	/**
	 * get the length of queue in lane at the start of current tick
	 * @param lane the lane for which queue length is required
	 * @return initial length of queue
	 */
	double getInitialQueueLength(const Lane* lane);

	/**
	 * checks if lane is connected to the next segment
	 *
	 * @param lane current lane
	 * @param nxtRdSeg next road segment
	 * @return true if lane is connected to nextSegStats; false otherwise
	 */
	bool isConnectedToNextSeg(const Lane* lane, const sim_mob::RoadSegment *nxtRdSeg) const;

	/**
	 * checks if 'any' lane is connected to the next segment
	 *
	 * @param srcRdSeg Road Segment
	 * @param nxtRdSeg next road segment
	 * @return true if lane is connected to next Segment; false otherwise
	 */
	bool isConnectedToNextSeg(const sim_mob::RoadSegment *srcRdSeg, const sim_mob::RoadSegment *nxtRdSeg) const;

	/**
	 * add driver to queue
	 *
	 * @param lane lane in which the driver is added to queue
	 */
	void addToQueue();

	/**
	 * remove driver from queue
	 */
	void removeFromQueue();

	/**
	 * get the best lane in next segment for driver to move into
	 *
	 * @param nextSegStats next segment stats
	 * @param nextToNextSegStats second segment stats ahead from the current
	 * @return best lane in nextSegStats
	 */
	virtual const sim_mob::Lane* getBestTargetLane(
			const sim_mob::SegmentStats* nextSegStats,
			const sim_mob::SegmentStats* nextToNextSegStats);

	/**
	 * Updates travel time for this driver for the link which he has just exited from.
	 *
	 * @param prevSeg the last segment in the link from which the driver has just exited
	 * @param linkExitTimeSec time at which the link was exited
	 */
	void updateLinkTravelTimes(const sim_mob::SegmentStats* prevSegStat,
			double linkExitTimeSec);

	/**
	 * Updates travel time for this driver for the road segment which he has just exited from.
	 *
	 * @param prevSeg the segment from which the driver has just exited
	 * @param linkExitTimeSec time at which the segment was exited
	 */
	void updateRdSegTravelTimes(const sim_mob::SegmentStats* prevSegStat,
			double segmentExitTimeSec);
	/**
	 * get number of intersections between the agent's location and incident location
	 * \param in list of stats in the incident roadsegemnt
	 * \param intersections out list of intersections suggested as re-routing points
	 * \param remaining lists of segstats from the original path which are remaining to reach to each of the suggested re-routing point
	 * return the number of intersections
	 */
	int findReroutingPoints(const std::vector<sim_mob::SegmentStats*>& stats,std::map<const sim_mob::Node*, std::vector<const sim_mob::SegmentStats*> > & remaining) const;
	bool wantReRoute(){return true;};//placeholder
	/**
	 * Changes the Travel Path based on the incident information
	 * \param in msg incident information(roadsegment and flow rate)
	 * \param newFlowRate new flow rate supplied to lanes
	 */
	void reroute(const InsertIncidentMessage &msg);
	///tries to remove the uturn if any
	bool UTurnFree(std::vector<WayPoint> & oldPath, std::vector<const sim_mob::SegmentStats*> & newPath, sim_mob::SubTrip & subTrip, std::set<const sim_mob::RoadSegment*> & excludeRS);
	///checks to see if it is possible to join an old path to a new one
	///it even tries to create a second new path  it the check fails once
	bool canJoinPaths(std::vector<WayPoint> & oldPath, std::vector<const sim_mob::SegmentStats*> & newPath, sim_mob::SubTrip & subTrip, std::set<const sim_mob::RoadSegment*> & excludeRS);
	//checks if there is a uturn
	bool hasUTurn(std::vector<WayPoint> & oldPath, std::vector<const sim_mob::SegmentStats*> & newPath);
	/**
	 * message handler which provide a chance to handle message transfered from parent agent.
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleMessage(messaging::Message::MessageType type,
			const messaging::Message& message);

};

} /* namespace medium */
} /* namespace sim_mob */
