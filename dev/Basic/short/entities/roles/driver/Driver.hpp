//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <math.h>
#include <set>
#include <vector>

#include "buffering/Shared.hpp"
#include "conf/settings/DisableMPI.h"
#include "entities/roles/Role.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "entities/roles/driver/models/CarFollowModel.hpp"
#include "entities/roles/driver/models/LaneChangeModel.hpp"
#include "entities/roles/driver/models/IntersectionDrivingModel.hpp"
#include "message/Message.hpp"
#include "perception/FixedDelayed.hpp"
#include "util/DynamicVector.hpp"
#include "util/Math.hpp"

#include "DriverUpdateParams.hpp"
#include "DriverFacets.hpp"
#include "geospatial/TurningSection.hpp"

namespace sim_mob
{

//Forward declarations
class Pedestrian;
class Signal;
class Link;
class RoadSegment;
class Lane;
class Node;
class MultiNode;
class DPoint;
class UpdateParams;
class DriverBehavior;
class DriverMovement;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

/**
   * \author Wang Xinyuan
   * \author Li Zhemin
   * \author Runmin Xu
   * \author Seth N. Hetu
   * \author Luo Linbo
   * \author LIM Fung Chai
   * \author Zhang Shuai
   * \author Xu Yan
   */
  class Driver : public sim_mob::Role , public UpdateWrapper<DriverUpdateParams>
  {
  private:

    //Internal classes
    //Helper class for grouping a Node and a Point2D together.
    class NodePoint
    {
    public:
      Point2D point;
      const Node* node;

      NodePoint() : point(0, 0), node(nullptr)
      {
      }
    } ;

    //Indicates whether the driver is in a loading queue. There isn't actually any data structure to represent this
    //queue. We use the fact that at every time tick, agents are going to be processed sequentially anyway.
    //If this boolean is true, it means that there is no space for it on the road.
    bool isVehicleInLoadingQueue;

    //Indicates whether the position of the vehicle has been found.
    bool isVehiclePositionDefined;

    //Indicates whether we are yielding to another driver at a conflict in the intersection
    int yieldingToInIntersection;

    //Pointer to the vehicle this driver is controlling.
    Vehicle* vehicle;

    friend class DriverBehavior;
    friend class DriverMovement;
    
  protected:
    
    //Current position of the Driver
    DPoint currPos;

  public:
    
    //Constant distance values used for looking ahead / behind
    const static int distanceInFront = 3000;
    const static int distanceBehind = 5000;
    const static int maxVisibleDis = 5000;

    /*Buffered data
     These values are stored the double buffer because they are needed by other drivers.*/
    
    //The driver's current lane
    Shared<const Lane*> currLane_;
    
    //Indicates whether the driver is in an intersection
    Shared<bool> isInIntersection_;
    
    //Pointer to a turning object. The driver is either on the turning (if in intersection) or will be
    //soon (currently approaching an intersection)
    Shared<const TurningSection*> currTurning_;
    
    //Represents the distance covered within an intersection (in centimetre)
    Shared<double> moveDisOnTurning_;
    
    //Represents the distance to be covered in order to reach the approaching intersection (in metre)
    Shared<double> distToIntersection_;
    
    //Represents the distance to be covered in order to reach the end of the current segment (in centimetre)
    Shared<double> distToCurrSegmentEnd_;
    
    //Represents the distance covered on the current road segment (in centimetre)
    Shared<double> currLaneOffset_;
    
    //Represents the total length of the current road segment (in centimetre)
    Shared<double> currLaneLength_;
    
    //Represents the lateral movement distance of the vehicle (in centimetre)
    Shared<double> latMovement_;
    
    //Represents the forward speed of the vehicle (centimetre/second)
    Shared<double> fwdVelocity_;
    
    //Represents the lateral velocity of the vehicle (centimetre/second)
    Shared<double> latVelocity_;
    
    //Represents the acceleration of the vehicle (centimetre/second^2)
    Shared<double> fwdAccel_;
    
    //Indicates the lane changing move that the driver is going to make
    Shared<LANE_CHANGE_SIDE> turningDirection_;

    //The distance covered along the current road segment (in centimetre)
    double currDistAlongRoadSegment;

    //Pointer to the Driver object that is performing 'nosing'. (Current driver is 'yielding')
    Driver* yieldVehicle;
    
    //Rection time of the driver
    size_t reactionTime;
    
    //Perceived value of forward velocity
    FixedDelayed<double> *perceivedFwdVel;
    
    //Perceived value of the acceleration
    FixedDelayed<double> *perceivedFwdAcc;
    
    //Perceived value of the velocity of the vehicle in front
    FixedDelayed<double> *perceivedVelOfFwdCar;
    
    //Perceived value acceleration of the vehicle in front
    FixedDelayed<double> *perceivedAccOfFwdCar;
    
    //Perceived distance to the vehicle in front
    FixedDelayed<double> *perceivedDistToFwdCar;
    
    //The perceived colour of the traffic signal
    FixedDelayed<sim_mob::TrafficColor> *perceivedTrafficColor;
    
    //The perceived distance to the traffic signal
    FixedDelayed<double> *perceivedDistToTrafficSignal;

    //The origin of the driver's trip
    NodePoint origin;
    
    //The destination of the driver's trip
    NodePoint goal;

    //For FMOD request
    Shared<std::string> stop_event_time;
    Shared<int> stop_event_type;
    Shared<int> stop_event_scheduleid;
    Shared<int> stop_event_nodeid;
    Shared<std::vector<int> > stop_event_lastBoardingPassengers;
    Shared<std::vector<int> > stop_event_lastAlightingPassengers;

    //Constructor and public member functions
    Driver(Person* parent, sim_mob::MutexStrategy mtxStrat, sim_mob::DriverBehavior* behavior = nullptr,
           sim_mob::DriverMovement* movement = nullptr, Role::type roleType_ = RL_DRIVER, std::string roleName_ = "driver");

    virtual ~Driver();

    //Initialises the reaction time of the driver and the perception delays based on the reaction time
    void initReactionTime();

    //Updates the information held by the current driver about a nearby driver
    void handleUpdateRequest(MovementFacet* mFacet);

    //Returns true if the vehicle associated with this driver is a bus, else returns false
    bool isBus();

    //Calculates and returns the gap between the current driver and the given driver. The gap calculated
    //is in terms of seconds (i.e. headway)
    //CAUTION: The driver in "front" and this driver may not be in the same lane (it could be in the left
    //or right neighbour lane), but the two have to be in either the same segment or in adjoining segment downstream
    double gapDistance(const Driver* front);

    //Getter to the vehicle object controlled by the driver. Returns a constant pointer.
    const Vehicle* getVehicle() const
    {
      return vehicle;
    }

    //Setter for the vehicle object controlled by the driver
    void setVehicle(Vehicle *vehicle)
    {
      safe_delete_item(this->vehicle);
      this->vehicle = vehicle;
    }
    
    //Getter to the vehicle object controlled by the driver.
    Vehicle* getVehicle()
    {
      return vehicle;
    }

    //Getter to the length of the vehicle being driven by this driver (in centimetre). Returns a constant value.
    const double getVehicleLengthCM() const
    {
      return vehicle->getLengthCm();
    }

    //Getter to the length of the vehicle being driven by this driver (in metre). Returns a constant value.
    const double getVehicleLengthM() const
    {
      return getVehicleLengthCM() / 100.0;
    }

    //Getter to the isVehicleInLoadingQueue flag. Returns true if the vehicle is still in the loading queue.
    bool IsVehicleInLoadingQueue()
    {
      return isVehicleInLoadingQueue;
    }

    //Returns the forward velocity of the vehicle (in metre/second)
    const double getFwdVelocityM() const;

    //Returns the current position of the driver
    const DPoint& getCurrPosition() const;

    //Sets a new path from the current segment to the destination.
    //NOTE: Used only by road-runner. The vehicle will restart from the start of the current segment
    void rerouteWithPath(const std::vector<sim_mob::WayPoint>& path);

    //Sets the current position of the driver
    void setCurrPosition(DPoint currPosition);

    //Sets the reaction time of the driver to the one provided (in milli-seconds). 
    //Also resets the perception delays accordingly.
    void resetReactionTime(double timeMS);
	
	//Setter for yieldingToInIntersection
    void setYieldingToInIntersection(int);

    //Getter for yieldingToInIntersection
    int getYieldingToInIntersection() const;

    /*Overridden functions*/
	
	//Creates and initialises the movement and behaviour objects required for the Driver role,
	//assigns them to a new driver and returns a pointer to the driver.
    virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Resets the dirver parameters object
    virtual void make_frame_tick_params(timeslice now);

	//Creates a vector of the subscription parameters and returns it
    virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	//Creates a vector of the subscription parameters specific to FMOD and returns it
    virtual std::vector<sim_mob::BufferedBase*> getDriverInternalParams();

    //Handler for the parent event from other agents
    virtual void onParentEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);

    ///Reroute around a blacklisted set of RoadSegments. See Role's comments for more information.
    virtual void rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment*>& blacklisted);
    
    //Message handler which provide a chance to handle message transfered from parent agent
    virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message);

    //Serialization
#ifndef SIMMOB_DISABLE_MPI
  public:
    virtual void pack(PackageUtils& packageUtil);
    virtual void unpack(UnPackageUtils& unpackageUtil);

    virtual void packProxy(PackageUtils& packageUtil);
    virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif

  } ;



}
