//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
// license.txt (http://opensource.org/licenses/MIT)

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "DriverUpdateParams.hpp"
#include "models/IntersectionDrivingModel.hpp"
#include "Driver.hpp"

using namespace std;
using namespace sim_mob;

MITSIM_IntDriving_Model::MITSIM_IntDriving_Model(sim_mob::DriverUpdateParams& params)
{
	totalMovement = 0;
	initParam(params);
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

double MITSIM_IntDriving_Model::makeAcceleratingDecision(DriverUpdateParams& params, const TurningSection* currTurning)
{
	double acc = params.maxAcceleration;
	const double vehicleLength = params.driver->getVehicleLengthM();

	//Vector of conflicts for the current turning
	const vector<TurningConflict *> &conflicts = currTurning->getTurningConflicts();

	//Select the nearest conflict ahead of us (the vector is sorted according to the distance from the start of the turning,
	//we may have crossed some)
	for (vector<TurningConflict *>::const_iterator itConflicts = conflicts.begin(); itConflicts != conflicts.end(); ++itConflicts)
	{
		//Get the distance to conflict point from start of turning
		double distToConflict =
				((*itConflicts)->getFirstTurning() == currTurning) ? (*itConflicts)->getFirst_cd() : (*itConflicts)->getSecond_cd();

		//Calculate the distance to conflict point from current position
		distToConflict = distToConflict - (params.driver->moveDisOnTurning_ / 100) - vehicleLength;

		//If we're yet to reach the conflict, calculate the time required for us and the conflict driver
		//else go to the next conflict
		if (distToConflict > 0)
		{
			//Time taken to reach conflict by current driver
			double timeToConflict = abs(distToConflict) / (params.currSpeed + Math::DOUBLE_EPSILON);

			//Time taken to reach conflict point by incoming driver
			double timeToConflictOtherDriver = DBL_MAX;

			//The map entry to the conflict and list of NearestVehicles on the it
			map<TurningConflict *, list<NearestVehicle> >::iterator itConflictVehicles = params.conflictVehicles.find(*itConflicts);

			//Check if we have found an entry in the map
			if (itConflictVehicles != params.conflictVehicles.end())
			{
				//Iterator to the list of nearest vehicles on the list
				list<NearestVehicle>::iterator itNearestVehicles = itConflictVehicles->second.begin();

				//Look for the vehicle reaching the conflict point - the list is sorted according to the distance from the conflict
				//point - distance less than 0 means yet to reach the conflict and greater than 0 means crossed the conflict point
				while (itNearestVehicles != itConflictVehicles->second.end())
				{
					//Check if the vehicle is yet to arrive at the conflict point and is not slowing down
					if (itNearestVehicles->distance < 0
						&& itNearestVehicles->driver->IsYieldingInIntersection() == false)
					{
						//Speed of the incoming vehicle (m/s))
						double speed = itNearestVehicles->driver->getVehicle()->getVelocity() / 100;

						//Negate the distance while calculating the time
						timeToConflictOtherDriver = abs(itNearestVehicles->distance) / (speed + Math::DOUBLE_EPSILON);

						//Calculate the critical gap
						//It is the max of: minimumGap and the difference between the 
						//criticalGap for the turning conflict and a random value
						double criticalGap = (*itConflicts)->getCriticalGap();
						criticalGap = max(criticalGap + Utils::nRandom(criticalGapAddOn[0], criticalGapAddOn[1]), minimumGap) ;

						double gap = timeToConflict - timeToConflictOtherDriver;

						//If the gap between current driver and the conflicting driver is less than the critical gap,
						//reject the gap (slow down)
						if (abs(gap) < criticalGap)
						{
							//Calculate the deceleration required to stop before conflict
							double brakingAcc = brakeToStop(distToConflict, params);

							//If this deceleration is smaller than the one we have previously, use this
							if (acc > brakingAcc)
							{
								acc = brakingAcc;
								params.driver->setYieldingInIntersection(true);
							}
						}
					}

					++itNearestVehicles;
				}
			}
		}
	}

	return acc;
}

void MITSIM_IntDriving_Model::initParam(DriverUpdateParams& params)
{
	string modelName = "general_driver_model";
	string critical_gap_addon;
	bool isAMOD = false;

	//Check if the vehicle is autonomous
	if (params.driver->getParent()->amodId != "-1")
	{
		isAMOD = true;
	}

	//Get the parameter manager instance for the respective type of vehicle (normal or AMOD)
	ParameterManager *parameterMgr = ParameterManager::Instance(isAMOD);

	//Read the parameter values
	parameterMgr->param(modelName, "intersection_visibility", intersectionVisbility, 50.0);
	parameterMgr->param(modelName, "intersection_attentiveness_factor_min", intersectionAttentivenessFactorMin, 1.0);
	parameterMgr->param(modelName, "intersection_attentiveness_factor_max", intersectionAttentivenessFactorMax, 3.0);
	parameterMgr->param(modelName, "minimum_gap", minimumGap, 0.0);
	parameterMgr->param(modelName, "critical_gap_addon", critical_gap_addon, string("0.0 2.5"));

	//Vector to store the tokenized parameters
	std::vector<string> gapAddonParams;

	//Index
	int index = 0;

	//Tokenize the gap add-on parameters
	boost::trim(critical_gap_addon);
	boost::split(gapAddonParams, critical_gap_addon, boost::is_any_of(" "), boost::token_compress_on);

	//Convert into numeric form
	vector<string>::iterator itStr = gapAddonParams.begin();
	while (itStr != gapAddonParams.end())
	{
		double res = 0;

		try
		{
			res = boost::lexical_cast<double>(itStr->c_str());
		}		catch (boost::bad_lexical_cast&)
		{
			std::string str = "Could not covert <" + *itStr + "> to double.";
			throw std::runtime_error(str);
		}

		criticalGapAddOn[index] = res;

		++index;
		++itStr;
	}
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
	} else
	{
		double dt = params.nextStepSize;
		return (dt > 0.0) ? -(params.currSpeed) / dt : params.maxDeceleration;
	}
}