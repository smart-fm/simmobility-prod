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

namespace sim_mob
{
namespace medium
{

class Driver;

/**
 * Behaviour Facet class for mid-term Driver role
 *
 * \author Melani Jayasuriya
 * \author Harish Loganathan
 */
class DriverBehavior : public BehaviorFacet
{
public:
	explicit DriverBehavior();
	virtual ~DriverBehavior();

	/**
	 * Virtual overrides
	 */
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

	Driver* getParentDriver();

protected:
	/**
	 * Pointer to the parent Driver role.
	 */
	Driver* parentDriver;

};

/**
 * Movement Facet class for mid-term Driver role
 *
 * \author Melani Jayasuriya
 * \author Harish Loganathan
 */
class DriverMovement : public MovementFacet
{
public:
	explicit DriverMovement();
	virtual ~DriverMovement();

	//virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();
	virtual Conflux* getStartingConflux() const;

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

	const MesoPathMover & getMesoPathMover() const
	{
		return pathMover;
	}

	MesoPathMover & getMesoPathMover()
	{
		return pathMover;
	}

	bool canOverrideLaneConnectors() const
	{
		return laneConnectorOverride;
	}

	void setLaneConnectorOverride(bool laneConnectorOverride)
	{
		this->laneConnectorOverride = laneConnectorOverride;
	}

protected:
	/// mark startTimeand origin. Called at every frame_init
	virtual TravelMetric& startTravelTimeMetric();
	/**
	 * mark the destination and end time and travel time.
	 * upon every change in the role, this method is called
	 * to collect the metrics collected during the previous
	 * role's period.
	 */
	//
	virtual TravelMetric& finalizeTravelTimeMetric();


	/**
	 * Process CBD information where a segment in the path has been traversed
	 * @param  completedRS the completed Road Segment
	 * @param  nextRS the next Road Segment to be visited next(if any)
	 * @return Travel Metrics member object
	 */

	virtual TravelMetric& processCBD_TravelMetrics(const RoadSegment* completedRS, const RoadSegment* nextRS);

	/**
	 * Pointer to the parent Driver role.
	 */
	Driver* parentDriver;

	MesoPathMover pathMover;
	const Lane* currLane;
	bool isQueuing;
	bool laneConnectorOverride;

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
	void setParentData(DriverUpdateParams& params); ///<set next data to parent buffer data

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
	 * Handle the event where a segment in the path has been traversed
	 * @param  completedRS the completed Road Segment
	 * @param  nextRS the next Road Segment to be visited next(if any)
	 */
	virtual void onSegmentCompleted(const RoadSegment* completedRS, const RoadSegment* nextRS);

	/**
	 * Handle the event where a Link in the path has been traversed
	 * @param  completedLink the completed link
	 * @param  nextLink the next link to be visited next(if any)
	 */

	virtual void onLinkCompleted(const Link * completedLink, const Link * nextLink);

	/**
	 * checks whether the driver can move into the next segment in path
	 *
	 * @param params driver update params for current tick
	 * @param nextSegStats next segment stats in path
	 * @param nextLink the immediate link downstream to nextSegStats
	 * @return true if driver can move into the next segment; false otherwise
	 */
	bool canGoToNextRdSeg(DriverUpdateParams& params, const SegmentStats* nextSegStats, const Link* nextLink = nullptr) const;

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
	int getOutputCounter(const Lane* lane, const SegmentStats* segStats);

	/**
	 * set number of vehicles that can move out of a lane in this tick
	 *
	 * @param l lane in segment
	 * @param count new value of outpur counter
	 * @param segStats segStats segment stats corresponding to lane l's segment
	 */
	void setOutputCounter(const Lane* lane, int count, const SegmentStats* segStats);

	double getOutputFlowRate(const Lane* lane);
	double getAcceptRate(const Lane* lane, const SegmentStats* segStats);
	double getQueueLength(const Lane* lane);
	double getLastAccept(const Lane* lane, const SegmentStats* segStats);
	void setLastAccept(const Lane* lane, double lastAccept, const SegmentStats* segStats);

	/**
	 * update flow of segment segStats
	 * @param segStats segment stats whose flow is to be updated
	 * @param startPos position of driver at the start of the tick
	 * @param endPos final position of driver
	 */
	void updateFlow(const SegmentStats* segStats, double startPos, double endPos);

	/**
	 * accepts a list of WayPoint-s and returns a list of SegmentStats* corresponding
	 * to RoadSegment* in the list of WayPoint.
	 */
	void initSegStatsPath(std::vector<WayPoint>& input,
						std::vector<const SegmentStats*>& output);
	/**
	 * overload of the above
	 */
	void initSegStatsPath(const std::vector<const RoadSegment*>& input,
						std::vector<const SegmentStats*>& output);

	/**
	 * randomly chooses the starting segment from the first link of the path
	 * @param path the path for which first segment has to be randomized
	 */
	void randomizeStartingSegment(std::vector<WayPoint>& path);

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
	virtual const Lane* getBestTargetLane(const SegmentStats* nextSegStats, const SegmentStats* nextToNextSegStats);

	/**
	 * Updates travel time for this driver for the link which he has just exited from.
	 *
	 * @param prevSeg the last segment in the link from which the driver has just exited
	 * @param linkExitTimeSec time at which the link was exited
	 */
	void updateLinkTravelTimes(const SegmentStats* prevSegStat, double linkExitTimeSec);

	/**
	 * Updates travel time for this driver for the road segment which he has just exited from.
	 *
	 * @param prevSeg the last segment in the link from which the driver has just exited
	 * @param linkExitTimeSec time at which the link was exited
	 */
	void updateRdSegTravelTimes(const SegmentStats* prevSegStat, double segEnterExitTime);

	/**
	 * get number of intersections between the agent's location and incident location
	 * \param in list of stats in the incident roadsegemnt
	 * \param intersections out list of intersections suggested as re-routing points
	 * \param remaining lists of segstats from the original path which are remaining to reach to each of the suggested re-routing point
	 * return the number of intersections
	 */
	int findReroutingPoints(const std::vector<SegmentStats*>& stats, std::map<const Node*, std::vector<const SegmentStats*> > & remaining) const;

	bool wantReRoute()
	{
		return true;
	}; //placeholder

	/**
	 * Changes the Travel Path
	 * @param currSegment current segment where the agent is
	 * @param nextSegment the next segment along the current path
	 */
	void reroute();

	/**
	 * Changes the Travel Path based on the incident information
	 * \param in msg incident information(roadsegment and flow rate)
	 * \param newFlowRate new flow rate supplied to lanes
	 */
	void reroute(const InsertIncidentMessage &msg);

	///tries to remove the uturn if any
	bool UTurnFree(std::vector<WayPoint> & oldPath, std::vector<const SegmentStats*> & newPath, SubTrip & subTrip, std::set<const RoadSegment*> & excludeRS);

	///checks to see if it is possible to join an old path to a new one
	///it even tries to create a second new path  it the check fails once
	bool canJoinPaths(std::vector<WayPoint> & oldPath, std::vector<const SegmentStats*> & newPath, SubTrip & subTrip, std::set<const RoadSegment*> & excludeRS);

	//checks if there is a uturn
	bool hasUTurn(std::vector<WayPoint> & oldPath, std::vector<const SegmentStats*> & newPath);

	/**
	 * message handler which provide a chance to handle message transfered from parent agent.
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void handleMessage(messaging::Message::MessageType type, const messaging::Message& message);

	/**
	 * gets the first link downstream to nextSegStats
	 * @param nextSegStats the next segment stats for driver
	 * @return next link for driver
	 */
	const Link* getNextLinkForLaneChoice(const SegmentStats* nextSegStats) const;
	friend class MesoReroute;

};

} /* namespace medium */
} /* namespace sim_mob */
