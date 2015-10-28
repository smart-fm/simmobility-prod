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

  //Forward declarations
  class Lane;
  class Driver;
  class IncidentPerformer;
  class IntersectionAccessMessage;

#ifndef SIMMOB_DISABLE_MPI
  class PackageUtils;
  class UnPackageUtils;
#endif

  //Struct for holding data about the "nearest" vehicle.

  struct NearestVehicle
  {

    NearestVehicle() : driver(nullptr), distance(DBL_MAX)
    {
    }

    NearestVehicle(const NearestVehicle& src) : driver(src.driver), distance(src.distance)
    {
    }
    
    bool exists() const
    {
      return distance < DBL_MAX;
    }
    
    const Driver* driver;
    double distance;
  } ;

  struct compare_NearestVehicle
  {
    bool operator() (const  NearestVehicle& first, const  NearestVehicle& second)
    {
      return ( first.distance > second.distance );
    }
  } ;

  //Similar, but for pedestrians

  struct NearestPedestrian
  {
    NearestPedestrian() : distance(DBL_MAX)
    {
    }

    bool exists()
    {
      return distance < DBL_MAX;
    }

    double distance;
  } ;

  struct StopPoint
  {
    StopPoint()
    {
    }

    StopPoint(unsigned int segId, double dis, double dwellT) : segmentId(segId), distance(dis), isPassed(false), dwellTime(dwellT)
    {
    }

    StopPoint(const StopPoint &source) : segmentId(source.segmentId), distance(source.distance), dwellTime(source.dwellTime), isPassed(source.isPassed)
    {
    }
    unsigned int segmentId;
    double distance;
    double dwellTime; //10 sec
    bool isPassed;
  } ;


  ///Simple struct to hold parameters which only exist for a single update tick.
  /// \author Wang Xinyuan
  /// \author Li Zhemin
  /// \author Seth N. Hetu
  ///NOTE: Constructor is currently implemented in Driver.cpp. Feel free to shuffle this around if you like.
  class DriverUpdateParams : public UpdateParams
  {
  public:
    DriverUpdateParams();
    explicit DriverUpdateParams(boost::mt19937 & gen);

    double getNextStepSize()
    {
      return nextStepSize;
    }

    virtual void reset(timeslice now, const Driver& owner);

    bool willYield(unsigned int reason);

    const RoadSegment* nextLink();

    /**
     *  /brief add one kind of status to the vh
     *  /param new state
     */
    void setStatus(unsigned int s);
    /*
     *  /brief set status to "performing lane change"
     */
    void setStatusDoingLC(LaneChangeTo& lcs);

    /**
     *  /brief get status of the vh
     *  /return state
     */
    unsigned int getStatus()
    {
      return status;
    }

    unsigned int getStatus(unsigned int mask)
    {
      return (status & mask);
    }

    /**
     *  /brief remove the status from the vh
     *  /return state
     */
    void unsetStatus(unsigned int s);

    unsigned int status;	// current status indicator
    unsigned int flags;	// additional indicator for internal use

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

    /**
     *  /brief add target lanes
     */
    void addTargetLanes(set<const Lane*> tl);

    /**
     *  /brief calculate min gap
     *  /param type driver type
     *  /return gap distance
     */
    double lcMinGap(int type);

    void buildDebugInfo();

    void setStatus(string name, StatusValue v, string whoSet);

    StatusValue getStatus(string name);

    /**
     *  @brief insert stop point
     *  @param sp stop point
     */
    void insertStopPoint(StopPoint& sp);

    double speedOnSign;

    std::vector<double> targetGapParams;

    /// lanes,which are ok to change to
    set<const Lane*> targetLanes;

    enum StopPointState
    {
      STOP_POINT_FOUND = 1,
      ARRIVING_AT_STOP_POINT = 2,
      ARRIVED_AT_STOP_POINT = 3,
      WAITING_AT_STOP_POINT = 4,
      LEAVING_STOP_POINT = 5,
      STOP_POINT_NOT_FOUND = 6
    } ;

    const Lane* currLane;
    size_t currLaneIndex;
    size_t nextLaneIndex;
    const Lane* leftLane;
    const Lane* rightLane;
    const Lane* leftLane2;
    const Lane* rightLane2;

    double currSpeed;
    double desiredSpeed;
	
    double elapsedSeconds;
    double trafficSignalStopDistance;
    sim_mob::TrafficColor trafficColor;
    sim_mob::TrafficColor perceivedTrafficColor;

    double perceivedFwdVelocity;
    double perceivedLatVelocity;
    double perceivedFwdVelocityOfFwdCar;
    double perceivedLatVelocityOfFwdCar;
    double perceivedAccelerationOfFwdCar;
    double perceivedDistToFwdCar;
    double perceivedDistToTrafficSignal;

    LaneChangeTo turningDirection;

    /// record last lane change decision
    /// both lc model and driverfacet can set this value
    LaneChangeTo lastDecision;

    //Nearest vehicles in the current lane, and left/right (including fwd/back for each).
    //Nearest vehicles' distances are initialized to threshold values.

    // used to check vh opposite intersection
    NearestVehicle nvFwdNextLink;
    NearestVehicle nvFwd;
    NearestVehicle nvBack;
    NearestVehicle nvLeftFwd;
    NearestVehicle nvLeftBack;
    NearestVehicle nvRightFwd;
    NearestVehicle nvRightBack;
    NearestVehicle nvLeftFwd2; //the second adjacent lane
    NearestVehicle nvLeftBack2;
    NearestVehicle nvRightFwd2;
    NearestVehicle nvRightBack2;

    std::map<const TurningConflict*, std::list<NearestVehicle> > conflictVehicles;
    void insertConflictTurningDriver(const TurningConflict* tc, double distance, const Driver* driver);

    // used to check vh when do acceleration merging
    NearestVehicle nvLeadFreeway; // lead vh on freeway segment,used when subject vh on ramp
    NearestVehicle nvLagFreeway; // lag vh on freeway,used when subject vh on ramp

    NearestPedestrian npedFwd;

    double laneChangingVelocity;

    //Indicates whether a vehicle is approaching an unsignalised intersection
    bool isApproachingIntersection;
    
    //Indicates how long a driver has been waiting to go ahead, after a while, the driver is more
    //likely to accept a smaller gap
    double impatienceTimer;
    
    //Indicates when the impatience timer was started
    double impatienceTimerStart;
    
    //Indicates if the driver has already stopped for the stop sign
    bool hasStoppedForStopSign;

	//The access time sent by the intersection manager
    double accessTime;

    //Indicates if the access time has been received from the intersection manager
    bool isResponseReceived;

    //Indicates if the car following accelerations are to be over-ridden
    bool useIntAcc;
    
    //Related to our car following model.
    double space;
    double a_lead;
    double v_lead;
    double space_star;
    double distanceToNormalStop;
    double distToStop;

    //Handles state information
    bool justChangedToNewSegment;
    Point TEMP_lastKnownPolypoint;
    bool justMovedIntoIntersection;
    double overflowIntoIntersection;

    Driver* driver;

    /// if current lane connect to target segment
    /// assign in driverfact
    bool isTargetLane;

    // perception distance to stop point
    double stopPointPerDis;

    StopPointState stopPointState;
    double distanceToStoppingPt;
    StopPoint currentStopPoint;
    double startStopTime;

    double utilityLeft;
    double utilityRight;
    double utilityCurrent;
    double rnd;
    std::string lcd; // lc decision

    /// headway value from carFollowingRate()
    double headway;
    double emergHeadway;

    //Selected acc (for debugging)
    double acc;

    double density;

    int initialSpeed;


    std::string cfDebugStr;
    std::stringstream  lcDebugStr;

    std::string accSelect;
    std::string debugInfo;

    int parentId;

    double FFAccParamsBeta;

    double lateralVelocity;

    SMStatusManager statusMgr;

	/**
	 * Stores the stopping points for the driver
	 * Key = segment id, value= vector of stopping points (one segment may have more than one stop point)
	 */
    std::map<unsigned int, std::vector<StopPoint> > stopPointPool;

    double reactionTimeCounter;

    double nextStepSize;

    double maxAcceleration;

    double normalDeceleration;

    double maxDeceleration;

    timeslice yieldTime;	// time to start yielding

    double lcTimeTag;		// time changed lane , ms

    vector<double> nosingParams;

    double lcMaxNosingTime;

    double maxLaneSpeed;

    /// fwd acc from car follow model m/s^2
    double acceleration;

    // critical gap param
    std::vector< std::vector<double> > LC_GAP_MODELS;

  public:
#ifndef SIMMOB_DISABLE_MPI
    static void pack(PackageUtils& package, const DriverUpdateParams* params);

    static void unpack(UnPackageUtils& unpackage, DriverUpdateParams* params);
#endif
  } ;


}
