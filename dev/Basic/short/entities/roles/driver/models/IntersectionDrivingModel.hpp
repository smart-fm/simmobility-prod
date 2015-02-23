//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include <vector>

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
class IntersectionDrivingModel {
public:
	//Allow propagation of delete
	virtual ~IntersectionDrivingModel() {}

    //Builds a straight line trajectory form the end point of the entry lane into the intersection
    //to the start point of the exit lane out of the intersection
	virtual void startDriving(const DPoint& fromLanePt, const DPoint& toLanePt, double startOffset) = 0;

    //Moves the vehicle by given amount along the trajectory
    virtual DPoint continueDriving(double amount) = 0;
    
    //Depending on the conflicting vehicles, calculates the acceleration that allows the vehicle to 
    //pass through without colliding with the other vehicles
    virtual double makeAcceleratingDecision(DriverUpdateParams& params, TurningSection *currTurning) = 0;

    //Returns the current angle of the vehicle according to the position in the trajectory
    virtual double getCurrentAngle() = 0;

    //Returns the distance covered within the intersection
    virtual double getMoveDistance() = 0;

    //Checks whether we've completed driving in the intersection
    virtual bool isDone() = 0;
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
class SimpleIntDrivingModel : public IntersectionDrivingModel {
private:
	DynamicVector intTrajectory;
	double totalMovement;

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


	virtual double getCurrentAngle() { return intTrajectory.getAngle(); }
	virtual bool isDone() { return totalMovement >= intTrajectory.getMagnitude(); }

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

    //Trajectory of the vehicle in the intersection
    DynamicVector intTrajectory;
    
    //Distance covered within the intersection
    double totalMovement;
    
    //Calculates the deceleration needed for the vehicle to come to a stop within a given distance
    double brakeToStop(double distance, DriverUpdateParams& params);

  public:
    
    MITSIM_IntDriving_Model();
    
    virtual ~MITSIM_IntDriving_Model();
    
    //Builds a straight line trajectory form the end point of the entry lane into the intersection
    //to the start point of the exit lane out of the intersection
    virtual void startDriving (const DPoint& fromLanePt, const DPoint& toLanePt, double startOffset);
    
    //Moves the vehicle by given amount along the trajectory
    virtual DPoint continueDriving (double amount);
    
    //Depending on the conflicting vehicles, calculates the acceleration that allows the vehicle to 
    //pass through without colliding with the other vehicles
    virtual double makeAcceleratingDecision(DriverUpdateParams& params, TurningSection *currTurning);

    //Returns the current angle of the vehicle according to the position in the trajectory
    virtual double getCurrentAngle ();
    
    //Returns the distance covered within the intersection
    double getMoveDistance() {return totalMovement;}
    
    //Checks whether we've completed driving in the intersection
    virtual bool isDone ();    
};
  
}
