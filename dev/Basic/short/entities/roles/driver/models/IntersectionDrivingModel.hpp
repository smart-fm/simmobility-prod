//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include <vector>

#include "conf/params/ParameterManager.hpp"
#include "conf/settings/DisableMPI.h"
#include "geospatial/TurningSection.hpp"
#include "util/DynamicVector.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif

namespace sim_mob {
  
  class TurningSection;
  class DriverUpdateParams;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

/**
 * \author Seth N. Hetu
 */
class IntersectionDrivingModel 
{
protected:

  //Trajectory of the vehicle in the intersection
  DynamicVector intTrajectory;

  //Distance covered within the intersection
  double totalMovement;

  //Distance at which an intersection and vehicles approaching it from other links
  //is visible to the driver
  double intersectionVisbility;

  //The minimum value of the attentiveness factor within an intersection. The factor is a random value between the 
  //given min and max
  double intersectionAttentivenessFactorMin;

  //The maximum value of the attentiveness factor within an intersection. The factor is a random value between the 
  //given min and max
  double intersectionAttentivenessFactorMax;

  //The value of minimum gap
  double minimumGap;

  //Contains the mean and std dev values for generating a random add-on to the critical gap
  double criticalGapAddOn[2];

  //The multiplier for the impatience timer. 
  double impatienceFactor;

public:

  //Allow propagation of delete
  virtual ~IntersectionDrivingModel() {}

  //Builds a straight line trajectory form the end point of the entry lane into the intersection
  //to the start point of the exit lane out of the intersection
  virtual void startDriving(const DPoint& fromLanePt, const DPoint& toLanePt, double startOffset) = 0;

  //Moves the vehicle by given amount along the trajectory
    virtual DPoint continueDriving(double amount,DriverUpdateParams& p) = 0;

  //Depending on the conflicting vehicles, calculates the acceleration that allows the vehicle to 
  //pass through without colliding with the other vehicles
  virtual double makeAcceleratingDecision(DriverUpdateParams& params, const TurningSection *currTurning) = 0;

  //Returns the current angle of the vehicle according to the position in the trajectory
    virtual double getCurrentAngle() { return intTrajectory.getAngle(); }

  //Returns the distance covered within the intersection

  double getMoveDistance()
  {
    return totalMovement;
  }

    //Checks whether we've completed driving in the intersection
    virtual bool isDone() { return totalMovement >= intTrajectory.getMagnitude(); }
    
    //Getter for the intersection visibility distance
    int getIntersectionVisbility() const { return intersectionVisbility; }

  //Getter for intersectionAttentivenessFactorMin
  double getIntersectionAttentivenessFactorMin() const 
  {
      return intersectionAttentivenessFactorMin;
  }

  //Getter for intersectionAttentivenessFactorMax
  double getIntersectionAttentivenessFactorMax() const 
  {
      return intersectionAttentivenessFactorMax;
  }

    const TurningSection *	currTurning;
  double getImpatienceFactor() const
  {
    return impatienceFactor;
  }
};

/**
 *
 * Simple version of the intersection driving model
 * The purpose of this model is to demonstrate a very simple (yet reasonably accurate) model
 * which generates somewhat plausible visuals. This model should NOT be considered valid, but
 * it can be used for demonstrations and for learning how to write your own *Model subclasses.
 *
 * \author Seth N. Hetu
 */
class SimpleIntDrivingModel : public IntersectionDrivingModel 
{
public:
  
  virtual void startDriving(const DPoint& fromLanePt, const DPoint& toLanePt, double startOffset) {
    intTrajectory = DynamicVector(fromLanePt.x, fromLanePt.y, toLanePt.x, toLanePt.y);
    totalMovement = startOffset;
  }


  virtual DPoint continueDriving(double amount) {
    totalMovement += amount;
    DynamicVector temp(intTrajectory);
    temp.scaleVectTo(totalMovement).translateVect();
    return DPoint(temp.getX(), temp.getY());
  }

  //add by xuyan
#ifndef SIMMOB_DISABLE_MPI
  static void pack(PackageUtils& package, const SimpleIntDrivingModel* params);

  static void unpack(UnPackageUtils& unpackage, SimpleIntDrivingModel* params);
#endif
};

/*
 * MITSIMLab version of the intersection driving model
 */
class MITSIM_IntDriving_Model : public IntersectionDrivingModel
{
private:

  //Reads and stores the parameters related to intersection driving from the driver parameter xml file
  //(data/driver_param.xml)
  void initParam(DriverUpdateParams& params);

  //Calculates the deceleration needed for the vehicle to come to a stop within a given distance
  double brakeToStop(double distance, DriverUpdateParams& params);
  
  //Calculate the acceleration needed to crawl
  double crawlingAcc(double distance, DriverUpdateParams& params);

public:

  MITSIM_IntDriving_Model(DriverUpdateParams& params);

  virtual ~MITSIM_IntDriving_Model();

  //Builds a straight line trajectory form the end point of the entry lane into the intersection
  //to the start point of the exit lane out of the intersection
  virtual void startDriving (const DPoint& fromLanePt, const DPoint& toLanePt, double startOffset);

  //Moves the vehicle by given amount along the trajectory
  virtual DPoint continueDriving (double amount,DriverUpdateParams& p);

  virtual double getCurrentAngle();

  virtual bool isDone();

  void makePolypoints(const DPoint& fromLanePt, const DPoint& toLanePt);

  //Depending on the conflicting vehicles, calculates the acceleration that allows the vehicle to 
  //pass through without colliding with the other vehicles
  virtual double makeAcceleratingDecision(DriverUpdateParams& params, const TurningSection *currTurning);  

private:
  /// store polypoints of the turning path

  std::vector<DPoint> polypoints;
  std::vector<DPoint>::iterator polypointIter;

  DPoint currPosition;
  DynamicVector currPolyline;

  /// length of the turning
  double length;

  double polylineMovement;

};
  
}
