//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/random.hpp>

#include "conf/settings/DisableMPI.h"
#include "entities/UpdateParams.hpp"
#include "entities/models/Constants.h"
#include "entities/roles/driver/SMStatus.h"
#include "entities/roles/driver/models/LaneChangeModel.hpp"
#include "entities/signal/Signal.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/TurningConflict.hpp"
#include "util/DynamicVector.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob
{

class Lane;
class Driver;
class IncidentPerformer;
class IntersectionAccessMessage;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

/**Holds the the "nearest" vehicle.*/
struct NearestVehicle
{
	/**The nearest driver*/
	const Driver* driver;

	/**Distance to to the driver (in metre)*/
	double distance;

	NearestVehicle() : driver(NULL), distance(DBL_MAX)
	{
	}

	NearestVehicle(const NearestVehicle &src) : driver(src.driver), distance(src.distance)
	{
	}

	bool exists() const
	{
		return (driver != NULL);
	}

	void reset()
	{
		driver = NULL;
		distance = DBL_MAX;
	}
};

/**Comparator for sorting the nearest vehicles in the conflicting vehicles list*/
struct compare_NearestVehicle
{
	bool operator()(const NearestVehicle& first, const NearestVehicle& second)
	{
		return ( first.distance > second.distance);
	}
};

/**Holds the nearest pedestrian*/
struct NearestPedestrian
{
	/**Distance to the nearest pedestrian*/
	double distance;

	NearestPedestrian() : distance(DBL_MAX)
	{
	}

	bool exists()
	{
		return distance < DBL_MAX;
	}
};

/**Represents the location to stop*/
struct StopPoint
{
	/**The segment at which the stop point is located*/
	unsigned int segmentId;

	/**The offset from the start of the segment*/
	double distance;

	/**The time for which the vehicle is to stop at the stop point*/
	double dwellTime;

	/**Indicates whether the we've already crossed the stop point*/
	bool isPassed;

	StopPoint()
	{
	}

	StopPoint(unsigned int segId, double dis, double dwellT) : segmentId(segId), distance(dis), isPassed(false), dwellTime(dwellT)
	{
	}

	StopPoint(const StopPoint &source) : segmentId(source.segmentId), distance(source.distance), dwellTime(source.dwellTime), isPassed(source.isPassed)
	{
	}
};

/**
 * Holds parameters which only exist for a single update tick.
 * 
 * \author Wang Xinyuan
 * \author Li Zhemin
 * \author Seth N. Hetu
 */
class DriverUpdateParams : public UpdateParams
{
public:
	/**Represents the various states in the stopping point behaviour*/
	enum StopPointState
	{
		STOP_POINT_FOUND = 1,
		ARRIVING_AT_STOP_POINT = 2,
		ARRIVED_AT_STOP_POINT = 3,
		WAITING_AT_STOP_POINT = 4,
		LEAVING_STOP_POINT = 5,
		STOP_POINT_NOT_FOUND = 6
	};

	/**Indicates whether a vehicle is approaching an unsignalised intersection*/
	bool isApproachingIntersection;

	/**Indicates if the driver has already stopped for the stop sign*/
	bool hasStoppedForStopSign;

	/**Indicates if the access time has been received from the intersection manager*/
	bool isResponseReceived;

	/**Indicates if the car following accelerations are to be over-ridden*/
	bool useIntAcc;

	/**Indicates if the current lane is the target lane*/
	bool isTargetLane;

	/**The current lane's index*/
	unsigned int currLaneIndex;

	/**The next lane's index*/
	unsigned int nextLaneIndex;

	/**Current status indicator*/
	unsigned int status;

	/**Additional indicator for internal use*/
	unsigned int flags;

	/**The initial speed of the vehicle*/
	int initialSpeed;

	/**The id of the agent*/
	int parentId;

	/**The current speed of the vehicle (m/s)*/
	double currSpeed;

	/**The speed desired by the driver (m/s)*/
	double desiredSpeed;

	/**Time elapsed since the previous tick*/
	double elapsedSeconds;

	/**Distance to the traffic signal*/
	double trafficSignalStopDistance;

	/**The perceived forward velocity*/
	double perceivedFwdVelocity;

	/**The perceived lateral velocity*/
	double perceivedLatVelocity;

	/**The perceived forward velocity of the car in front*/
	double perceivedFwdVelocityOfFwdCar;

	/**The perceived lateral velocity of the car in front*/
	double perceivedLatVelocityOfFwdCar;

	/**The perceived acceleration of the car in front*/
	double perceivedAccelerationOfFwdCar;

	/**The perceived distance to the car in front*/
	double perceivedDistToFwdCar;

	/**The perceived distance to the traffic signal*/
	double perceivedDistToTrafficSignal;

	/**The speed limit on the road sign*/
	double speedLimit;

	/**Indicates how long a driver has been waiting to go ahead, after a while, the driver is more likely to accept a smaller gap*/
	double impatienceTimer;

	/**Indicates when the impatience timer was started*/
	double impatienceTimerStart;

	/**The access time sent by the intersection manager*/
	double accessTime;

	/**Gap between the vehicles. This is used in the car following model*/
	double gapBetnVehicles;

	/**Acceleration of the lead vehicle*/
	double accLeadVehicle;

	/**Velocity of the lead vehicle*/
	double velocityLeadVehicle;

	/***/
	double spaceStar;

	/**Distance required to stop in normal circumstances*/
	double distanceToNormalStop;

	/**Represents the distance beyond which we can't continue. This may be due to lane not being connected to the next segment, an incident ahead, etc*/
	double distToStop;

	/**The distance by which we have entered the intersection*/
	double overflowIntoIntersection;

	/**Distance at which a stopping point is visible*/
	double stopVisibilityDistance;

	/**The distance to the stopping point*/
	double distanceToStoppingPt;

	/**The time at which the vehicle stopped at the stop point. Used to compare with dwell time in order to decide when to start moving*/
	double stopTimeTimer;

	/**Utility of the left lane*/
	double utilityLeft;

	/**Utility of the right lane*/
	double utilityRight;

	/**Utility of the current lane*/
	double utilityCurrent;

	/**The headway*/
	double headway;

	/**The selected acceleration - used for debugging*/
	double acc;

	/**The density of the current lane*/
	double density;

	/**The parameters for computing the free flow acceleration*/
	double FFAccParamsBeta;

	/**The lateral velocity of the vehicle*/
	double lateralVelocity;

	/**The count down timer for the reaction time expiry*/
	double reactionTimeCounter;

	/**The next step size*/
	double nextStepSize;

	/**The maximum value of acceleration (m/s^2)*/
	double maxAcceleration;

	/**The normal deceleration value (m/s^2)*/
	double normalDeceleration;

	/**The maximum value of deceleration (m/s^2)*/
	double maxDeceleration;

	/**The time of making the most recent lane change (milli-seconds)*/
	double laneChangeTime;

	/**The maximum time for which a driver can yield (seconds)*/
	double lcMaxYieldingTime;

	/**The maximum driving speed allowed in the lane*/
	double maxLaneSpeed;

	/**The acceleration of the vehicle*/
	double acceleration;

	/**Indicates the type of acceleration that was selected*/
	std::string accSelect;

	/**The lane change decision - used for debugging*/
	std::string lcd;

	/**Debugging information for car-following model*/
	std::string cfDebugStr;

	/**Debugging information for lane-changing model*/
	std::stringstream lcDebugStr;

	/**The debugging information*/
	std::string debugInfo;

	/**The time when the driver started yielding*/
	timeslice yieldTime;

	/**The colour on the traffic signal*/
	TrafficColor trafficColor;

	/**The perceived traffic signal colour*/
	TrafficColor perceivedTrafficColor;

	/**Indicates which lane the driver is changing to*/
	LaneChangeTo turningDirection;

	/**The nearest vehicle ahead of us that is in the next link.*/
	NearestVehicle nvFwdNextLink;

	/**The nearest vehicle ahead of us*/
	NearestVehicle nvFwd;

	/**The nearest vehicle behind us*/
	NearestVehicle nvBack;

	/**The nearest vehicle ahead of us, but in the left lane*/
	NearestVehicle nvLeftFwd;

	/**The nearest vehicle behind us, but in the left lane*/
	NearestVehicle nvLeftBack;

	/**The nearest vehicle ahead of us, nut in the right lane*/
	NearestVehicle nvRightFwd;

	/**The nearest vehicle behind us, but in the right lane*/
	NearestVehicle nvRightBack;

	/**The nearest vehicle ahead of us, but on the lane left of our left lane*/
	NearestVehicle nvLeftFwd2;

	/**The nearest vehicle behind us, but on the lane left of our left lane*/
	NearestVehicle nvLeftBack2;

	/**The nearest vehicle ahead of us, but on the lane right of our right lane*/
	NearestVehicle nvRightFwd2;

	/**The nearest vehicle behind us, but on the lane right of our right lane*/
	NearestVehicle nvRightBack2;

	/**Lead vehicle on the freeway/expressway. This is used when the subject vehicle is on the ramp*/
	NearestVehicle nvLeadFreeway;

	/**Lag vehicle on freeway/expressway. This is used when the subject vehicle is on the ramp*/
	NearestVehicle nvLagFreeway;

	/**The current state in the stopping point state machine*/
	StopPointState stopPointState;

	/**The current stopping point*/
	StopPoint currentStopPoint;

	/**The status manager. Used in the lane changing model to keep track of the status*/
	SMStatusManager statusMgr;

	/**The driver object to which these parameters belong*/
	Driver* driver;

	/**The lane the driver is currently on*/
	const Lane* currLane;

	/**The lane to the left of the driver's current lane*/
	const Lane* leftLane;

	/**The lane to the right of the driver's current lane*/
	const Lane* rightLane;

	/**The lane to the left of the driver's left lane*/
	const Lane* leftLane2;

	/**The lane to the right of the driver's right lane*/
	const Lane* rightLane2;

	/**
	 * Stores the stopping points for the driver
	 * Key = segment id, value= vector of stopping points (one segment may have more than one stop point)
	 */
	std::map<unsigned int, std::vector<StopPoint> > stopPointPool;

	/**The map of conflicting vehicles in the intersection. Key=Turning conflict, Value=Sorted list of vehicles(nearest vehicle first)*/
	std::map<const TurningConflict*, std::list<NearestVehicle> > conflictVehicles;

	/**Parameters for calculating the target gap*/
	std::vector<double> targetGapParams;

	/**The parameters for the nosing model*/
	std::vector<double> nosingParams;

	/**The critical gap parameters for lane changing*/
	std::vector< std::vector<double> > LC_GAP_MODELS;

	/**The set of lanes which we can change to*/
	std::set<const Lane*> targetLanes;

	DriverUpdateParams();
	virtual ~DriverUpdateParams();

	/**
	 * Calculates the minimum gap
     *
	 * @param type type of model to be used to calculate the gap
     *
	 * @return the minimum gap (metre)
     */
	double lcMinGap(int type);

	/**
	 * Builds a string with debugging information to be written to the output file
     */
	void buildDebugInfo();

	/**
	 * Sets the given status
     *
	 * @param name the status name
     * @param v the status to be set
     * @param whoSet the method setting the status
     */
	void setStatus(string name, StatusValue v, string whoSet);

	/**
	 * Retrieves the required status
	 *
     * @param name the name of the required status
     *
	 * @return the status
     */
	StatusValue getStatus(string name);

	/**
	 * Adds the given stopping point to the vector of stopping points.
	 *
     * @param stopPt the stop point
     */
	void insertStopPoint(StopPoint &stopPt);

	/**
	 * Sets the status of the vehicle
     * @param status the status value to be set
     */
	void setStatus(unsigned int status);

	/**
	 * Clears the given status of the vehicle
	 *
     * @param status status to be cleared
     */
	void unsetStatus(unsigned int status);

	/**
	 * Sets the status to "Performing lane change"
	 *
     * @param laneChangingTo the lane changing direction
     */
	void setStatusDoingLC(LaneChangeTo &laneChangingTo);

	/**
	 * Resets the parameters
	 *
     * @param now the current time
     * @param owner the driver to which the parameters belong
     */
	virtual void reset(timeslice now, const Driver &owner);

	/**
     * @return the first road segment in the next link if the link is in the path, else NULL
     */
	const RoadSegment* nextLink();

	/**
	 * Adds the driver to the map of conflicting vehicles.
	 *
     * @param conflict the conflict towards which the driver is heading
     * @param distance the driver's distance to the conflict
     * @param driver the conflicting driver
     */
	void insertConflictTurningDriver(const TurningConflict *conflict, double distance, const Driver *driver);

	double getNextStepSize()
	{
		return nextStepSize;
	}

	unsigned int getStatus()
	{
		return status;
	}

	unsigned int getStatus(unsigned int mask)
	{
		return (status & mask);
	}

	void toggleFlag(unsigned int flag)
	{
		flags ^= flag;
	}

	unsigned int flag(unsigned int mask = 0xFFFFFFFF)
	{
		return (flags & mask);
	}

	void setFlag(unsigned int s)
	{
		flags |= s;
	}

	void unsetFlag(unsigned int s)
	{
		flags &= ~s;
	}

#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const DriverUpdateParams* params);
	static void unpack(UnPackageUtils& unpackage, DriverUpdateParams* params);
#endif
};
}
