//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
// license.txt (http://opensource.org/licenses/MIT)

#include "DriverUpdateParams.hpp"
#include "models/IntersectionDrivingModel.hpp"
#include "Driver.hpp"

using namespace std;
using namespace sim_mob;

MITSIM_IntDriving_Model::MITSIM_IntDriving_Model()
{
	totalMovement = 0;
}

MITSIM_IntDriving_Model::~MITSIM_IntDriving_Model()
{
	
}

void MITSIM_IntDriving_Model::startDriving(const DPoint& fromLanePt, const DPoint& toLanePt, double startOffset)
{
	intTrajectory = DynamicVector (fromLanePt.x, fromLanePt.y, toLanePt.x, toLanePt.y);
	totalMovement = startOffset;
}

DPoint MITSIM_IntDriving_Model::continueDriving(double amount)
{
	totalMovement += amount;
	DynamicVector temp (intTrajectory);
	temp.scaleVectTo (totalMovement).translateVect ();
	return DPoint (temp.getX (), temp.getY ());
}

double MITSIM_IntDriving_Model::makeAcceleratingDecision(DriverUpdateParams& params, TurningSection* currTurning)
{
	double acc = params.maxAcceleration;
	
	//Vector of conflicts for the current turning
	vector<TurningConflict *> &conflicts = currTurning->getTurningConflicts();
 
	//Select the nearest conflict ahead of us (the vector is sorted according to the distance from the start of the turning,
	//we may have crossed some)
	for(vector<TurningConflict *>::iterator itConflicts = conflicts.begin(); itConflicts != conflicts.end(); ++itConflicts)
	{
		//Get the distance to conflict point from start of turning
		double distToConflict =
				((*itConflicts)->getFirstTurning() == currTurning) ? (*itConflicts)->getFirst_cd() : (*itConflicts)->getSecond_cd();
		
		//Calculate the distance to conflict point from current position
		distToConflict = distToConflict - params.driver->moveDisOnTurning_;
		
		//If we're yet to reach the conflict, calculate the time required for us and the conflict driver
		//else go to the next conflict
		if(distToConflict > 0 && params.currSpeed > 0)
		{
			//Time taken to reach conflict by current driver
			double timeToConflict = (distToConflict + params.driver->getVehicleLengthM()) / params.currSpeed;
			
			//Time taken to reach conflict point by incoming driver
			double timeToConflictOtherDriver = DBL_MAX;
			
			//The map entry to the conflict and list of NearestVehicles on the it
			map<TurningConflict *, list<NearestVehicle> >::iterator itConflictVehicles = params.conflictVehicles.find((*itConflicts));
			
			if(itConflictVehicles != params.conflictVehicles.end())
			{
				//Iterator to the list of nearest vehicles on the list
				list<NearestVehicle>::iterator itNearestVehicles = itConflictVehicles->second.begin();
				
				//Look for the vehicle reaching the conflict point - the list is sorted according to the distance from the conflict
				//point - distance less than 0 means yet to reach the conflict and greater than 0 means crossed the conflict point
				while(itNearestVehicles != itConflictVehicles->second.end())
				{					
					//Check if the vehicle is yet to arrive at the conflict point
					if(itNearestVehicles->distance < 0)
					{
						//Speed of the incoming vehicle (m/s))
						double speed = itNearestVehicles->driver->getVehicle()->getVelocity() / 100;
						
						//Negate the distance while calculating the time
						timeToConflictOtherDriver = (- itNearestVehicles->distance) / speed;
					}
					else
					{
						break;
					}
										
					++itNearestVehicles;
				}
			}
			
			if(timeToConflict > timeToConflictOtherDriver)
			{
				acc = brakeToStop(distToConflict, params);
			}
			
			break;
		}
	}
	
	return acc;
}

double MITSIM_IntDriving_Model::getCurrentAngle()
{
	return intTrajectory.getAngle ();
}

bool MITSIM_IntDriving_Model::isDone()
{
	return totalMovement >= intTrajectory.getMagnitude ();
}

double MITSIM_IntDriving_Model::brakeToStop(double distance, DriverUpdateParams& params)
{
	if (distance > sim_mob::Math::DOUBLE_EPSILON)
	{
		double sqCurrVel = params.currSpeed * params.currSpeed;
		double acc = -sqCurrVel / distance * 0.5;

		if (acc <= params.normalDeceleration)
		{
			return acc;
		}

		double dt = params.nextStepSize;
		double vt = params.currSpeed * dt;
		double a = dt * dt;
		double b = 2.0 * vt - params.normalDeceleration * a;
		double c = sqCurrVel + 2.0 * params.normalDeceleration * (distance - vt);
		double d = b * b - 4.0 * a * c;

		if (d < 0 || a <= 0.0)
		{
			return acc;
		}

		return (sqrt(d) - b) / a * 0.5;
	}
	else
	{
		double dt = params.nextStepSize;
		return (dt > 0.0) ? -(params.currSpeed) / dt : params.maxDeceleration;
	}
}