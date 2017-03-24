//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
// license.txt (http://opensource.org/licenses/MIT)

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <cmath>
#include <map>

#include "models/IntersectionDrivingModel.hpp"
#include "DriverUpdateParams.hpp"
#include "Driver.hpp"
#include "util/Utils.hpp"

using namespace std;
using namespace sim_mob;

MITSIM_IntDriving_Model::MITSIM_IntDriving_Model() :
intersectionAttentivenessFactorMin(0), intersectionAttentivenessFactorMax(0), minimumGap(0), impatienceFactor(0)
{
	modelType = IntModelType::Int_Model_MITSIM;
}

MITSIM_IntDriving_Model::MITSIM_IntDriving_Model(DriverUpdateParams& params) :
intersectionAttentivenessFactorMin(0), intersectionAttentivenessFactorMax(0), minimumGap(0), impatienceFactor(0)
{
	modelType = IntModelType::Int_Model_MITSIM;
	readDriverParameters(params);
}

MITSIM_IntDriving_Model::~MITSIM_IntDriving_Model()
{
}

double MITSIM_IntDriving_Model::getIntersectionAttentivenessFactorMin() const
{
	return intersectionAttentivenessFactorMin;
}

double MITSIM_IntDriving_Model::getIntersectionAttentivenessFactorMax() const
{
	return intersectionAttentivenessFactorMax;
}

double MITSIM_IntDriving_Model::getImpatienceFactor() const
{
	return impatienceFactor;
}

void MITSIM_IntDriving_Model::readDriverParameters(DriverUpdateParams &params)
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
	parameterMgr->param(modelName, "intersection_attentiveness_factor_min", intersectionAttentivenessFactorMin, 1.0);
	parameterMgr->param(modelName, "intersection_attentiveness_factor_max", intersectionAttentivenessFactorMax, 3.0);
	parameterMgr->param(modelName, "minimum_gap", minimumGap, 0.0);
	parameterMgr->param(modelName, "critical_gap_addon", critical_gap_addon, string("0.0 2.5"));
	parameterMgr->param(modelName, "impatience_factor", impatienceFactor, 0.2);

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
		}
		catch (boost::bad_lexical_cast&)
		{
			std::stringstream str;
			str << __func__ << ": Could not covert " << *itStr << " to type double.";
			throw std::runtime_error(str.str());
		}

		criticalGapAddOn[index] = res;

		++index;
		++itStr;
	}
}

double MITSIM_IntDriving_Model::calcBrakeToStopAcc(double distance, DriverUpdateParams &params)
{
	double acc = 0;

	if (distance > Math::DOUBLE_EPSILON)
	{
		//v^2 = u^2 + 2as (Equation of motion)
		//So, a = (v^2 - u^2) / 2s
		//v = final velocity, u = initial velocity
		//a = acceleration, s = displacement
		double sqCurrVel = params.perceivedFwdVelocity * params.perceivedFwdVelocity;

		acc = -sqCurrVel / distance * 0.5;

		if (acc <= params.normalDeceleration)
		{
			acc = params.normalDeceleration;
		}
		else
		{
			double dt = params.nextStepSize;
			double vt = params.perceivedFwdVelocity * dt;
			double a = dt * dt;
			double b = 2.0 * vt - params.normalDeceleration * a;
			double c = sqCurrVel + 2.0 * params.normalDeceleration * (distance - vt);
			double d = b * b - 4.0 * a * c;

			if (!(d < 0 || a <= 0.0))
			{
				acc = (sqrt(d) - b) / a * 0.5;
			}
		}
	}
	else
	{
		double dt = params.nextStepSize;
		acc = (dt > 0.0) ? -(params.perceivedFwdVelocity) / dt : params.maxDeceleration;
	}

	//Make sure the value is bounded
	if (acc > params.maxAcceleration)
	{
		acc = params.maxAcceleration;
	}

	if (acc < params.maxDeceleration)
	{
		acc = params.maxDeceleration;
	}

	return acc;
}

double MITSIM_IntDriving_Model::calcCrawlingAcc(double distance, DriverUpdateParams& params)
{
	//Acceleration to slowly crawl towards the conflict point
	return 2 * ((distance / 2) - params.perceivedFwdVelocity * params.nextStepSize) / (params.nextStepSize * params.nextStepSize);
}

double MITSIM_IntDriving_Model::calcArrivalTime(double distance, DriverUpdateParams& params)
{
	double arrivalTime = -1, acceleration = 0, finalVel = 0;

	//The final velocity is limited by the turning speed, so calculate the acceleration required to
	//achieve the final velocity
	//v^2 = u^2 + 2as
	//So, a = (v^2 - u^2) / (2s)

	//Get the speed limit and convert it to m/s
	finalVel = 1; //currTurning->getTurningSpeed() / 3.6;

	//Calculate the acceleration
	acceleration = ((finalVel * finalVel) - (params.perceivedFwdVelocity * params.perceivedFwdVelocity)) / (2 * distance);

	//We know s = ut + (1/2)at^2
	//To find the time required, we rearrange the equation as follows:
	//(1/2)at^2 + ut - s = 0	This is a quadratic equation AX^2 + BX + C = 0 and we can solve for t
	//A = (1/2)a; B = u; C = -s

	if (acceleration == 0 && params.perceivedFwdVelocity != 0)
	{
		//Acceleration is 0, so we have a linear relation : ut - s = 0
		arrivalTime = distance / params.perceivedFwdVelocity;
	}
	else
	{
		//As it is a quadratic equation, we will have two solutions
		double sol1 = 0, sol2 = 0;

		//The discriminant (b^2 - 4ac)
		double discriminant = (params.perceivedFwdVelocity * params.perceivedFwdVelocity) - (2 * acceleration * (-distance));

		if (discriminant >= 0)
		{
			//Calculate the solutions
			sol1 = (-params.perceivedFwdVelocity - sqrt(discriminant)) / acceleration;
			sol2 = (-params.perceivedFwdVelocity + sqrt(discriminant)) / acceleration;

			//As time can be negative, return the solution that is a positive value	
			if (sol1 >= 0 && sol2 >= 0)
			{
				arrivalTime = min(sol1, sol2);
			}
			else if (sol1 >= 0)
			{
				arrivalTime = sol1;
			}
			else if (sol2 >= 0)
			{
				arrivalTime = sol2;
			}
		}
	}

	return arrivalTime;
}

double MITSIM_IntDriving_Model::makeAcceleratingDecision(DriverUpdateParams &params)
{
	params.intDebugStr.clear();
	std::stringstream debugStr;
	double acc = params.maxAcceleration;
	const double vehicleLength = params.driver->getVehicleLength();
	const TurningGroup *currTurningGroup = currTurning->getTurningGroup();

	//Reduce the reaction time in intersection			
	params.reactionTimeCounter = params.reactionTimeCounter * Utils::generateFloat(intersectionAttentivenessFactorMin, intersectionAttentivenessFactorMax);

	//Safety margin distance in front of the vehicle (half a vehicle length seems a reasonable margin)
	const double safeDist = 1.5 * vehicleLength;
	double distToStopLine = params.driver->getDistToIntersection() - safeDist;

	debugStr << ";t:" << params.now.ms() << ";dSL:" << distToStopLine;

	//Check if we've stopped close enough to the stop line
	if (distToStopLine <= 1 && params.perceivedFwdVelocity <= 0.1)
	{
		params.hasStoppedForStopSign = true;
		debugStr << ";StpSgn-Dne";
	}

	//We have to stop for the stop sign
	if (currTurningGroup->getRule() == TurningGroupRule::TURNING_GROUP_RULE_STOP_SIGN && !params.hasStoppedForStopSign && distToStopLine <= 5)
	{
		double brakingAcc = calcBrakeToStopAcc(distToStopLine, params);

		if (acc > brakingAcc && params.perceivedFwdVelocity != 0)
		{
			acc = brakingAcc;
			params.driver->setYieldingToInIntersection(-1);
		}

		debugStr << ";StpSgn";
		return acc;
	}

	//The turning conflicts for the current turning
	const vector<TurningConflict *> &conflicts = currTurning->getConflictsOnPath();

	//Select the nearest conflict ahead of us (the vector is sorted according to the distance from the start of the turning,
	//we may have crossed some)
	for (vector<TurningConflict *>::const_iterator itConflicts = conflicts.begin(); itConflicts != conflicts.end(); ++itConflicts)
	{
		const TurningConflict *conflict = *itConflicts;
		bool isGapRejected = false;

		debugStr << ";Cfl:" << conflict->getConflictId();

		//The priority of the turnings in the conflict
		//0 - equal priority, 1 - first turning has priority, 2 - second turning has priority
		int priority = conflict->getPriority();

		bool isFirstTurning = (conflict->getFirstTurning() == currTurning) ? true : false;

		//The distance to conflict point from the start of the turning
		double cfltDistFrmTurning = 0;

		//The distance to conflict point from our position
		double distToConflict = 0;

		//Check which turning in the conflict has the higher priority. If our turning has higher priority, 
		//ignore all vehicles in the conflict as they will slow down for us
		if (isFirstTurning)
		{
			//Our turning has priority, or equal priority but other turning has
			//stop sign, ignore the vehicles in this conflict
			if (priority == 1 ||
					(priority == 0 && currTurningGroup->getRule() == TurningGroupRule::TURNING_GROUP_RULE_NO_STOP_SIGN &&
					conflict->getSecondTurning()->getTurningGroup()->getRule() == TurningGroupRule::TURNING_GROUP_RULE_STOP_SIGN))
			{
				continue;
			}

			//Get the distance to conflict point
			cfltDistFrmTurning = conflict->getFirstConflictDistance();
		}
		else
		{
			//Our turning has priority, or equal priority but other turning has
			//stop sign, ignore the vehicles in this conflict
			if (priority == 2 ||
					(priority == 0 && currTurningGroup->getRule() == TurningGroupRule::TURNING_GROUP_RULE_NO_STOP_SIGN &&
					conflict->getFirstTurning()->getTurningGroup()->getRule() == TurningGroupRule::TURNING_GROUP_RULE_STOP_SIGN))
			{
				continue;
			}

			//Get the distance to conflict point
			cfltDistFrmTurning = conflict->getSecondConflictDistance();
		}

		//Calculate the distance to conflict point from current position

		//If the vehicle is approaching the intersection rather than already in it, add the distance to the end
		//of the segment to the distance to conflict
		if (params.isApproachingIntersection)
		{
			distToConflict = cfltDistFrmTurning + params.driver->getDistToIntersection();
		}
		else
		{
			//Vehicle is in the intersection, so subtract the distance covered on the turning
			distToConflict = cfltDistFrmTurning - params.driver->getDistCoveredOnCurrWayPt();
		}

		debugStr << ";DstCfl:" << distToConflict;

		//The distance at which the vehicle needs to come to a stand-still when yielding
		double stoppingDist = 0;

		//If the turning has a stop sign, we stop just before the intersection
		if (currTurningGroup->getRule() == TurningGroupRule::TURNING_GROUP_RULE_STOP_SIGN)
		{
			stoppingDist = distToStopLine;
		}
		else
		{
			stoppingDist = distToConflict - safeDist;
		}

		debugStr << ";StpDis:" << stoppingDist;

		//If we're yet to reach the conflict, calculate the time required for us and the conflict driver
		//else go to the next conflict
		if (distToConflict > vehicleLength)
		{
			//Time taken to reach conflict by current driver
			double timeToConflict = calcArrivalTime(abs(distToConflict), params);

			if (timeToConflict == -1)
			{
				if (params.perceivedFwdVelocity > 1.0)
				{
					timeToConflict = abs(distToConflict) / params.perceivedFwdVelocity;
				}
				else
				{
					timeToConflict = 0;
				}
			}

			debugStr << ";Time2Cfl:" << timeToConflict;

			//Calculate the critical gap
			//It is the max of: minimumGap and the difference between the 
			//criticalGap for the turning conflict and a random value
			double criticalGap = conflict->getCriticalGap();

			//Add a random add-on value
			criticalGap += Utils::nRandom(criticalGapAddOn[0], criticalGapAddOn[1]);

			//Reduce by impatience on if equal priority
			if (priority == 0)
			{
				criticalGap -= (params.impatienceTimer * impatienceFactor);
			}

			criticalGap = max(criticalGap, minimumGap);

			debugStr << ";CGap:" << criticalGap;			

			//The map entry to the conflict and list of NearestVehicles on the it
			map<const TurningConflict *, std::set<NearestVehicle, compare_NearestVehicle> >::iterator itConflictVehicles = params.conflictVehicles.find(conflict);

			//Check if we have found an entry in the map
			if (itConflictVehicles != params.conflictVehicles.end())
			{
				debugStr << ";#veh:" << itConflictVehicles->second.size();

				//Iterator to the list of nearest vehicles on the list
				std::set<NearestVehicle, compare_NearestVehicle>::iterator itNearestVehicles = itConflictVehicles->second.begin();

				//Look for the vehicle reaching the conflict point - the list is sorted according to the distance from the conflict
				//point - distance less than 0 means yet to reach the conflict and greater than 0 means crossed the conflict point
				for (; itNearestVehicles != itConflictVehicles->second.end(); ++itNearestVehicles)
				{
					Driver *drv = const_cast<Driver *> (itNearestVehicles->driver);
					DriverUpdateParams &paramsOtherDriver = drv->getParams();

					debugStr << ";Veh:" << paramsOtherDriver.parentId;					
					debugStr << ";DisCfl:" << itNearestVehicles->distance;

					//If a driver is already yielding to us, scan the next conflict
					if (itNearestVehicles->driver->getYieldingToInIntersection() == params.parentId)
					{
						debugStr << ";Yldng:";						
						break;
					}

					if (itNearestVehicles->distance >= -itNearestVehicles->driver->getVehicleLength() / 2 && itNearestVehicles->distance < 0.0)
					{
						//Other vehicle is blocking the conflict
						debugStr << ";StpPtCrsd:";
						isGapRejected = true;
					}
					else if (itNearestVehicles->distance < -itNearestVehicles->driver->getVehicleLength())
					{
						//The vehicle is yet to arrive at the conflict point
						
						//Time taken to reach conflict point by incoming driver
						double timeToConflictOtherDriver = 0;

						//Speed of the incoming vehicle (m/s))
						double speed = itNearestVehicles->driver->getVehicle()->getVelocity();

						if (speed > 1.0)
						{
							//Negate the distance while calculating the time
							timeToConflictOtherDriver = abs(itNearestVehicles->distance) / speed;
						}						

						//The gap between the drivers
						double gap = abs(timeToConflictOtherDriver - timeToConflict);

						debugStr << ";T2Cfl:" << timeToConflictOtherDriver << ";gap:" << gap;

						//The gap was accepted, but we need to check if there's enough space after the conflict
						//point. A vehicle with higher priority might collide with us if we can't go past 
						//the conflict point
						if (gap >= criticalGap && timeToConflictOtherDriver != 0 && timeToConflict != 0)
						{
							debugStr << ";gp>=cgap";							

							//Distance of forward vehicle from the conflict point
							double distAheadOfConflict = DBL_MAX;
							
							if (params.nvFwd.driver)
							{
								//The forward driver exists, reject the gap if he's not far enough ahead of the conflict
								distAheadOfConflict = params.nvFwd.distance - distToConflict;
							}								
							else if (params.nvFwdNextLink.driver)
							{
								//There is no forward driver, but there is one after the intersection
								distAheadOfConflict = params.nvFwdNextLink.distance - distToConflict;
							}

							//Reject the gap if we don't have at least 5 vehicle length of space
							if (distAheadOfConflict < (5 * vehicleLength))
							{
								isGapRejected = true;
							}
						}
						else if (timeToConflict != 0 && timeToConflictOtherDriver == 0)
						{
							//Other driver has stopped
							debugStr << ";Time2Cfl!=0&&T2Cfl!=0";							

							//Assume the vehicle starts moving with the same speed as us							

							//Assumed time calculated based on our speed
							double assumedTimeToConflict = abs(itNearestVehicles->distance) / params.currSpeed;

							debugStr << ";AsmdT2Cfl:" << assumedTimeToConflict;						

							//Assumed gap
							double assumedGap = abs(assumedTimeToConflict - timeToConflict);

							//Check if the gap is accepted
							if (assumedGap <= criticalGap && itNearestVehicles->driver->getYieldingToInIntersection() == -1)
							{
								debugStr << ";AsmdGap-Rjct";								
								isGapRejected = true;
							}
							else
							{
								debugStr << ";AsmdGap-Accpt";
							}
						}							
						else if (timeToConflict == 0 && timeToConflictOtherDriver != 0)
						{
							//We have stopped, crawl till we can better judge the gap
							debugStr << ";Time2Cfl==0&&T2Cfl!=0";

							if (distToConflict > abs(itNearestVehicles->distance) && itNearestVehicles->driver->getYieldingToInIntersection() == -1)
							{
								//If we're stationary, crawl to conflict point
								double crawlAcc = calcCrawlingAcc(stoppingDist, params);								

								if (acc > crawlAcc)
								{
									acc = crawlAcc;
									params.driver->setYieldingToInIntersection(paramsOtherDriver.parentId);

									debugStr << ";Crawl:" << crawlAcc << ";YldVeh:" << paramsOtherDriver.parentId;									
								}
								else
								{
									debugStr << ";XCrawl:" << crawlAcc;
									isGapRejected = true;
								}
							}
						}							
						else if (timeToConflict == 0 && timeToConflictOtherDriver == 0)
						{
							//Both vehicles have stopped, break deadlock
							debugStr << ";Time2Cfl==0&&T2Cfl==0";

							//Compare the distances, if the one nearer to the conflict can go through
							if (distToConflict > abs(itNearestVehicles->distance) && itNearestVehicles->driver->getYieldingToInIntersection() == -1)
							{
								debugStr << ";OthClser";								
								isGapRejected = true;
							}
							else
							{
								debugStr << ";IClsr";
							}
						}							
						else
						{
							//All other cases, reject gap
							isGapRejected = true;
						}
					}

					//If the gap has been rejected (slow down and stop)
					if (isGapRejected)
					{
						//Calculate the deceleration required to stop before conflict
						double brakingAcc = calcBrakeToStopAcc(stoppingDist, params);

						debugStr << ";Gap-Rjct" << ";brake:" << brakingAcc;

						//If this deceleration is smaller than the one we have previously, use this
						if (acc > brakingAcc)
						{
							acc = brakingAcc;
							params.driver->setYieldingToInIntersection(paramsOtherDriver.parentId);

							debugStr << ";YldVeh:" << paramsOtherDriver.parentId << "Gap-Rjct";
						}
					}
				}
			}
		}
		else
		{
			debugStr << ";XdCflt" << conflict->getConflictId() << ";dist:" << distToConflict;
		}
	}

	params.intDebugStr = debugStr.str();
	return acc;
}
