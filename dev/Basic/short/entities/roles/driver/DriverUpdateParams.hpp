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
#include "geospatial/simmobility_network/Lane.hpp"
#include "geospatial/simmobility_network/RoadSegment.hpp"
#include "geospatial/simmobility_network/TurningConflict.hpp"
#include "util/DynamicVector.hpp"
#include "util/LangHelpers.hpp"


namespace sim_mob
{

  //Forward declarations
  class simmobility_network::Lane;
  class Driver;
  class IncidentPerformer;

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

    StopPoint(std::string& segId, double& dis, double& dwellT) : id(-1), segmentId(segId), distance(dis), isPassed(false), dwellTime(dwellT)
    {
    }

    StopPoint(const StopPoint& source) : id(source.id), segmentId(source.segmentId), distance(source.distance), dwellTime(source.dwellTime), isPassed(source.isPassed)
    {
    }
    int id;
    std::string segmentId;
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

    explicit DriverUpdateParams(boost::mt19937 & gen) : UpdateParams(gen) , nextLaneIndex(0), isTargetLane(true),
    status(0), flags(0), yieldTime(0, 0), lcTimeTag(200), speedOnSign(0), newFwdAcc(0), cftimer(0.0), newLatVelM(0.0), utilityLeft(0),
    utilityCurrent(0), utilityRight(0), perceivedDistToTrafficSignal(500), rnd(0),
    disAlongPolyline(0), dorigPosx(0), dorigPosy(0), movementVectx(0), movementVecty(0), headway(999), currLane(NULL),
    stopPointPerDis(100), stopPointState(NO_FOUND_STOP_POINT), startStopTime(0), disToSP(999),
    currLaneIndex(0), leftLane(NULL), rightLane(NULL), leftLane2(NULL), rightLane2(NULL), currSpeed(0), desiredSpeed(0), currLaneOffset(0),
    currLaneLength(0), trafficSignalStopDistance(0), elapsedSeconds(0), perceivedFwdVelocity(0), perceivedLatVelocity(0), perceivedFwdVelocityOfFwdCar(0),
    perceivedLatVelocityOfFwdCar(0), perceivedAccelerationOfFwdCar(0), perceivedDistToFwdCar(0),
    laneChangingVelocity(0), isCrossingAhead(false), isApproachingIntersection(false), crossingFwdDistance(0), space(0), a_lead(0),
    v_lead(0), space_star(0), distanceToNormalStop(0), dis2stop(0), impatienceTimer(0.0), justChangedToNewSegment(false),
    justMovedIntoIntersection(false), overflowIntoIntersection(0), driver(NULL), emergHeadway(999), acc(0),
    density(0), initSegId(0), initDis(0), initSpeed(0), parentId(0), FFAccParamsBeta(0), nextStepSize(0), maxAcceleration(0), normalDeceleration(0),
    lcMaxNosingTime(0), maxLaneSpeed(0), maxDeceleration(0), impatienceTimerStart(0.0), hasStoppedForStopSign(false)
    {
    }

    double getNextStepSize()
    {
      return nextStepSize;
    }

    virtual void reset(timeslice now, const Driver& owner);

    bool willYield(unsigned int reason);

    const simmobility_network::RoadSegment* nextLink();

    /**
     *  /brief add one kind of status to the vh
     *  /param new state
     */
    void setStatus(unsigned int s);
    /*
     *  /brief set status to "performing lane change"
     */
    void setStatusDoingLC(LANE_CHANGE_SIDE& lcs);

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
    void addTargetLanes(set<const simmobility_network::Lane*> tl);

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
    set<const simmobility_network::Lane*> targetLanes;

    enum STOP_POINT_STATE
    {
      APPROACHING_STOP_POINT = 1,
      CLOSE_STOP_POINT = 2,
      JUST_ARRIVE_STOP_POINT = 3,
      WAITING_AT_STOP_POINT = 4,
      LEAVING_STOP_POINT = 5,
      NO_FOUND_STOP_POINT = 6
    } ;

    const simmobility_network::Lane* currLane;
    size_t currLaneIndex;
    size_t nextLaneIndex;
    const simmobility_network::Lane* leftLane;
    const simmobility_network::Lane* rightLane;
    const simmobility_network::Lane* leftLane2;
    const simmobility_network::Lane* rightLane2;

    double currSpeed;
    double desiredSpeed;

    double currLaneOffset;
    double currLaneLength;

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

    LANE_CHANGE_SIDE turningDirection;

    /// record last lane change decision
    /// both lc model and driverfacet can set this value
    LANE_CHANGE_SIDE lastDecision;

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

    std::map<TurningConflict*, std::list<NearestVehicle> > conflictVehicles;
    void insertConflictTurningDriver(simmobility_network::TurningConflict* tc, double distance, const Driver* driver);

    // used to check vh when do acceleration merging
    NearestVehicle nvLeadFreeway; // lead vh on freeway segment,used when subject vh on ramp
    NearestVehicle nvLagFreeway; // lag vh on freeway,used when subject vh on ramp

    NearestPedestrian npedFwd;

    double laneChangingVelocity;

    bool isCrossingAhead;

    //Indicates whether a vehicle is approaching an unsignalised intersection
    bool isApproachingIntersection;
    
    //Indicates how long a driver has been waiting to go ahead, after a while, the driver is more
    //likely to accept a smaller gap
    double impatienceTimer;
    
    //Indicates when the impatience timer was started
    double impatienceTimerStart;
    
    //Indicates if the driver has already stopped for the stop sign
    bool hasStoppedForStopSign;
    
    int crossingFwdDistance;

    //Related to our car following model.
    double space;
    double a_lead;
    double v_lead;
    double space_star;
    double distanceToNormalStop;

    //Related to our lane changing model.
    double dis2stop; //meter

    //Handles state information
    bool justChangedToNewSegment;
    DPoint TEMP_lastKnownPolypoint;
    bool justMovedIntoIntersection;
    double overflowIntoIntersection;

    Driver* driver;

    /// if current lane connect to target segment
    /// assign in driverfact
    bool isTargetLane;

    // perception distance to stop point
    double stopPointPerDis;

    STOP_POINT_STATE stopPointState;
    double disToSP;
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

    int initSegId;
    int initDis;
    int initSpeed;

    //debug car jump
    double disAlongPolyline; //cm
    double movementVectx;
    double movementVecty;
    DPoint lastOrigPos_;
    double dorigPosx;
    double dorigPosy;
    DynamicVector latMv_;

    std::string cfDebugStr;
    std::stringstream  lcDebugStr;

    std::string accSelect;
    std::string debugInfo;

    int parentId;

    double FFAccParamsBeta;

    double newLatVelM; //meter/sec

    SMStatusManager statusMgr;

    // key=segment aimsun id, value= stoppoint vector, one segment may has more than one stoppoint
    std::map<std::string, std::vector<StopPoint> > stopPointPool;

    /// decision timer (second)
    /// count down in DriverMovement
    double cftimer;

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
    double newFwdAcc;

    // critical gap param
    std::vector< std::vector<double> > LC_GAP_MODELS;

  public:
#ifndef SIMMOB_DISABLE_MPI
    static void pack(PackageUtils& package, const DriverUpdateParams* params);

    static void unpack(UnPackageUtils& unpackage, DriverUpdateParams* params);
#endif
  } ;


}
