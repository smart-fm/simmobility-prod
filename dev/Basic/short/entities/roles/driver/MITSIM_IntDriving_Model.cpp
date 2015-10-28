//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
// license.txt (http://opensource.org/licenses/MIT)

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <cmath>

#include "models/IntersectionDrivingModel.hpp"
#include "DriverUpdateParams.hpp"
#include "Driver.hpp"
#include "util/Utils.hpp"

using namespace std;
using namespace sim_mob;

MITSIM_IntDriving_Model::MITSIM_IntDriving_Model() :
intersectionAttentivenessFactorMin(0), intersectionAttentivenessFactorMax(0), minimumGap(0), impatienceFactor(0), length(0), polylineMovement(0)
{
	modelType = Int_Model_MITSIM;
}

MITSIM_IntDriving_Model::MITSIM_IntDriving_Model(sim_mob::DriverUpdateParams& params) :
intersectionAttentivenessFactorMin(0), intersectionAttentivenessFactorMax(0), minimumGap(0), impatienceFactor(0), length(0), polylineMovement(0)
{
	modelType = Int_Model_MITSIM;
	initParam(params);
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

void MITSIM_IntDriving_Model::startDriving(const Point& fromLanePt, const Point& toLanePt, double startOffset)
{
	polypoints = currTurning->getPolyLine()->getPoints();
	
	// calculate polyline length
	length = currTurning->getLength();
	
	polypointIter = polypoints.begin();
	Point p1 = (*polypointIter);
	Point p2 = *(polypointIter+1);
	
	currPolyline = DynamicVector ( p1.getX(), p1.getY(), p2.getX(), p2.getY());
	polypointIter++;
	
	if(polypointIter == polypoints.end()) 
	{
		//poly-points only have one point! throw error
		std::string str = "MITSIM_IntDriving_Model poly-points only have one point.";
		throw std::runtime_error(str);
	}
	
	currPosition = fromLanePt;
	distCoveredOnTurning = startOffset;
	polylineMovement = startOffset;
}

Point MITSIM_IntDriving_Model::continueDriving(double amount,DriverUpdateParams& p)
{
	if(amount == 0)
	{
		return currPosition;
	}
	
	distCoveredOnTurning += amount;
	
	// check "amount" exceed the rest length of the DynamicVector
	DynamicVector tt(currPosition.getX(),currPosition.getY(),currPolyline.getEndX(),currPolyline.getEndY());
	double restLen = tt.getMagnitude();
	
	if (amount > restLen &&  polypointIter != polypoints.end() && polypointIter+1 != polypoints.end())
	{
		// move to next polyline, if has
		polylineMovement = amount - restLen;
		currPolyline = DynamicVector ( (*polypointIter).getX(), (*polypointIter).getY(), (*(polypointIter+1)).getX(), (*(polypointIter+1)).getY());
		polypointIter++;
		
		// current polyline length
		double l = currPolyline.getMagnitude();
		
		while(polylineMovement > l && polypointIter != polypoints.end() && polypointIter+1 != polypoints.end())
		{
			polylineMovement = polylineMovement - l;
			currPolyline = DynamicVector ( (*polypointIter).getX(), (*polypointIter).getY(), (*(polypointIter+1)).getX(), (*(polypointIter+1)).getY());
			polypointIter++;
		}
	}
	else 
	{
		polylineMovement += amount;
	}

	DynamicVector temp = currPolyline;
	temp.scaleVectTo (polylineMovement).translateVect ();

	currPosition = Point (temp.getX (), temp.getY ());

	return currPosition;
}

bool MITSIM_IntDriving_Model::isDone() 
{
	return distCoveredOnTurning >= length;
}

double MITSIM_IntDriving_Model::getCurrentAngle()
{
	return currPolyline.getAngle();
}

void MITSIM_IntDriving_Model::makePolypoints(const Point& fromLanePt, const Point& toLanePt) 
{
	/*
	// 1.0 calculate circle radius
	//http://rossum.sourceforge.net/papers/CalculationsForRobotics/CirclePath.htm

	// 2.0 calculate center point position
	// 3.0 all points position on a circle
	// filter base on from/to points x,y range

	//http://stackoverflow.com/questions/5300938/calculating-the-position-of-points-in-a-circle

	// make dummy polypoints
	double dy = -fromLanePt.y + toLanePt.y;
	double dx = -fromLanePt.x + toLanePt.x;

	double k = atan2(dy,dx);

	double kk = -k+3.14/2.0;

	double b = 500.0; // 10 meter

	// middle point position
	double xm,ym;
	if(fromLanePt.x > toLanePt.x)
	{
		xm = toLanePt.x + abs(dx)/2.0;
	}
	else 
	{
		xm = fromLanePt.x + abs(dx)/2.0;
	}

	if(fromLanePt.y > toLanePt.y)
	{
		ym = toLanePt.y + abs(dy)/2.0;
	}
	else 
	{
		ym = fromLanePt.y + abs(dy)/2.0;
	}

	double xx = b* cos(kk) + xm;
	double yy = b*sin(kk) + ym;

	Point dp(xx,yy);

	polypoints.push_back(fromLanePt);
	polypoints.push_back(dp);
	polypoints.push_back(toLanePt);

	length = sqrt((fromLanePt.x - dp.x)*(fromLanePt.x - dp.x) + (fromLanePt.y - dp.y)*(fromLanePt.y - dp.y));
	length += sqrt((toLanePt.x - dp.x)*(toLanePt.x - dp.x) + (toLanePt.y - dp.y)*(toLanePt.y - dp.y));
	*/
}

double MITSIM_IntDriving_Model::makeAcceleratingDecision(DriverUpdateParams &params)
{
	double acc = params.maxAcceleration;
	const double vehicleLength = params.driver->getVehicleLength();

	//Reduce the reaction time in intersection			
	params.reactionTimeCounter = params.reactionTimeCounter * Utils::generateFloat(intersectionAttentivenessFactorMin, intersectionAttentivenessFactorMax);
	
	/*
	//Safety margin distance in front of the vehicle (half a vehicle length seems a reasonable margin)
	const double safeDist = 1.5 * vehicleLength;
	double distToStopLine = params.driver->distToIntersection_.get() - safeDist;
	
	//Print() << "\nTime:" << params.now.frame();
	//Print() << "\nID:" << params.parentId;
	//Print() << "\nDistToStopLine:" << distToStopLine;
			
	//Check if we've stopped close enough to the stop line
	if (distToStopLine <= 1 && params.currSpeed <= 0.1)
	{
		params.hasStoppedForStopSign = true;
		//Print() << "\nStopped for Stop sign - Done";
	}
	
	//We have to stop for the stop sign
	if (currTurning->turningHasStopSign() && ! params.hasStoppedForStopSign
		&& distToStopLine <= 5)
	{
		double brakingAcc = brakeToStop(distToStopLine, params);

		if (acc > brakingAcc && params.currSpeed != 0)
		{
			acc = brakingAcc;
			params.driver->setYieldingToInIntersection(-1);			
		}				
		
		//Print() << "\nStopping for stop sign";
		
		return acc;
	}

	//Vector of conflicts for the current turning
	const vector<TurningConflict *> &conflicts = currTurning->getTurningConflicts();

	//Select the nearest conflict ahead of us (the vector is sorted according to the distance from the start of the turning,
	//we may have crossed some)
	for (vector<TurningConflict *>::const_iterator itConflicts = conflicts.begin(); itConflicts != conflicts.end(); ++itConflicts)
	{
		bool isGapRejected = false;
		//Print() << "\nConflict:" << (*itConflicts)->getDbId();
		
		//The priority of the turnings in the conflict
		//0 - equal priority, 1 - first turning has priority, 2 - second turning has priority
		int priority = (*itConflicts)->getPriority();
		
		bool isFirstTurning = ((*itConflicts)->getFirstTurning() == currTurning) ? true : false;
		
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
				(priority == 0 && ! currTurning->turningHasStopSign()
				 && (*itConflicts)->getSecondTurning()->turningHasStopSign()))
			{
				continue;
			}
			
			//Get the distance to conflict point
			cfltDistFrmTurning = (*itConflicts)->getFirst_cd();
		}
		else
		{
			//Our turning has priority, or equal priority but other turning has
			//stop sign, ignore the vehicles in this conflict
			if (priority == 2 ||
				(priority == 0 && ! currTurning->turningHasStopSign()
				 && (*itConflicts)->getFirstTurning()->turningHasStopSign()))
			{
				continue;
			}
			
			//Get the distance to conflict point
			cfltDistFrmTurning = (*itConflicts)->getSecond_cd();
		}
		
		//Print() << "\tConfDist:" << cfltDistFrmTurning;
		//Print() << "\tDistOnTurn:" << params.driver->moveDisOnTurning_ / 100;
	
		//Calculate the distance to conflict point from current position
		distToConflict = cfltDistFrmTurning - (params.driver->moveDisOnTurning_ / 100);
		
		//If the vehicle is approaching the intersection rather than already in it, add the distance to the end
		//of the segment to the distance to conflict
		if (params.isApproachingIntersection)
		{
			distToConflict += params.driver->distToIntersection_.get();
		}
		
		//Print() << "\tDistToConflict:" << distToConflict;
		
		//The distance at which the vehicle needs to come to a stand-still when yielding
		double stoppingDist = 0;
		
		//If the turning has a stop sign, we stop just before the	intersection
		if (currTurning->turningHasStopSign())
		{
			stoppingDist = distToStopLine;
		}
		else
		{
			stoppingDist = distToConflict - safeDist;
		}

		//Print() << "\tStoppingDist:" << stoppingDist;

		//If we're yet to reach the conflict, calculate the time required for us and the conflict driver
		//else go to the next conflict
		if (distToConflict > vehicleLength)
		{
			//Time taken to reach conflict by current driver
			double timeToConflict = calcArrivalTime(abs(distToConflict), params);
			
			if(timeToConflict == -1)
			{
				if(params.currSpeed > 1.0)
				{
					timeToConflict = abs(distToConflict) / params.currSpeed;
				}
				else
				{
					timeToConflict = 0;
				}
			}
			
			//Print() << "\tTimeToConflict:" << timeToConflict;

			//Calculate the critical gap
			//It is the max of: minimumGap and the difference between the 
			//criticalGap for the turning conflict and a random value
			double criticalGap = (*itConflicts)->getCriticalGap();
			
			//Add a random add-on value
			criticalGap += Utils::nRandom(criticalGapAddOn[0], criticalGapAddOn[1]);
			
			//Reduce by impatience on if equal priority
			if(priority == 0)
			{
				criticalGap -= (params.impatienceTimer * impatienceFactor);
			}
			
			criticalGap = max(criticalGap, minimumGap);

			//Print() << "\tCriticalGap:" << criticalGap;			

			//The map entry to the conflict and list of NearestVehicles on the it
			map<TurningConflict *, list<NearestVehicle> >::iterator itConflictVehicles = params.conflictVehicles.find(*itConflicts);
			
			//Check if we have found an entry in the map
			if (itConflictVehicles != params.conflictVehicles.end())
			{
				//Print() << "\n#Vehicles spotted:" << itConflictVehicles->second.size();
				
				//Iterator to the list of nearest vehicles on the list
				list<NearestVehicle>::iterator itNearestVehicles = itConflictVehicles->second.begin();

				//Look for the vehicle reaching the conflict point - the list is sorted according to the distance from the conflict
				//point - distance less than 0 means yet to reach the conflict and greater than 0 means crossed the conflict point
				for (; itNearestVehicles != itConflictVehicles->second.end(); ++itNearestVehicles)
				{
					Driver *drv = const_cast<Driver *>(itNearestVehicles->driver);
					DriverUpdateParams &paramsOtherDriver = drv->getParams();
						
					//Print() << "\nSpottedVeh:" << paramsOtherDriver.parentId;
					//Print() << "\tDistToCflt:" << itNearestVehicles->distance;
					
					//If a driver is already yielding to us, scan the next conflict
					if (itNearestVehicles->driver->getYieldingToInIntersection() == params.parentId)
					{
						//Print() << "\nThe vehicle is yielding to me";
						break;
					}
					
					//If the other vehicle is blocking the conflict
					if(itNearestVehicles->distance >= -itNearestVehicles->driver->getVehicleLengthM()
					   && itNearestVehicles->distance < 0.0)
					{
						//Print() << "\nOther vehicle has crossed stopping point.";
						isGapRejected = true;
					}					
					//Check if the vehicle is yet to arrive at the conflict point
					else if (itNearestVehicles->distance < -itNearestVehicles->driver->getVehicleLengthM())
					{																	
						//Time taken to reach conflict point by incoming driver
						double timeToConflictOtherDriver = 0;
						
						//Speed of the incoming vehicle (m/s))
						double speed = itNearestVehicles->driver->getVehicle()->getVelocity() / 100;

						if (speed > 1.0)
						{
							//Negate the distance while calculating the time
							timeToConflictOtherDriver = abs(itNearestVehicles->distance) / speed;
						}
						
						//Print() << "\tTimeToConflictOther:" << timeToConflictOtherDriver;
						
						//The gap between the drivers
						double gap = abs(timeToConflictOtherDriver - timeToConflict);
						
						//Print() << "\tGap:" << gap;
												
						//The gap was accepted, but we need to check if there's enough space after the conflict
						//point. A vehicle with higher priority might collide with us if we can't go past 
						//the conflict point
						if (gap >= criticalGap && timeToConflictOtherDriver != 0 && timeToConflict != 0)
						{
							//Print() << "\tgap >= criticalGap";

							//Distance of forward vehicle from the conflict point
							double distAheadOfConflict = DBL_MAX;

							//If the forward driver exists, reject the gap if he's not far enough
							//ahead of the conflict
							if (params.nvFwd.driver)
							{
								distAheadOfConflict = params.nvFwd.distance - distToConflict;
							}
							//If there is no forward driver, check for the one after the intersection
							//(just to be safe as this conflict point may be close to the end of the turning)
							else if (params.nvFwdNextLink.driver)
							{
								distAheadOfConflict = params.nvFwdNextLink.distance - distToConflict;
							}

							//Reject the gap if we don't have at least 5 vehicle length of space
							if (distAheadOfConflict < (5 * vehicleLength))
							{
								isGapRejected = true;
							}
						}
						//Other driver has stopped
						else if(timeToConflict != 0 && timeToConflictOtherDriver == 0)
						{
							//Print() << "\tTimeToConflict != 0 && TimeToConflictOtherDriver == 0";
							
							//Assume the vehicle starts moving with the same speed as us							
							
							//Assumed time calculated based on our speed
							double assumedTimeToConflict = abs(itNearestVehicles->distance) / params.currSpeed;
							
							//Print() << "\tAssumedTimeToConflict:" << assumedTimeToConflict;
							
							//Assumed gap
							double assumedGap = abs(assumedTimeToConflict - timeToConflict);
							
							//Check if the gap is accepted
							if(assumedGap <= criticalGap && itNearestVehicles->driver->getYieldingToInIntersection() == -1)
							{
								//Print() << "\tAssumed Gap rejected.";
								isGapRejected = true;
							}
							else
							{
								//Print() << "\tAssumed Gap accepted.";
							}
						}
						//We have stopped, crawl till we can better judge the gap
						else if(timeToConflict == 0 && timeToConflictOtherDriver != 0)
						{
							//Print() << "\tTimeToConflict == 0 && TimeToConflictOtherDriver != 0";
							
							if (distToConflict > abs(itNearestVehicles->distance)
									&& itNearestVehicles->driver->getYieldingToInIntersection() == -1)
							{
								//If we're stationary, crawl to conflict point
								double crawlAcc = crawlingAcc(stoppingDist, params);

								//Print() << "\tCrawlAcc:" << crawlAcc << "\tAcc:" << acc;

								if (acc > crawlAcc)
								{
									acc = crawlAcc;
									params.driver->setYieldingToInIntersection(paramsOtherDriver.parentId);

									//Print() << "\tYieldVeh:" << paramsOtherDriver.parentId;
									//Print() << "\tCrawl!";
								}
								else
								{
									//Print() << "\nCrawlAcc not small enough";
									isGapRejected = true;
								}
							}
						}
						//Both vehicles have stopped, break deadlock
						else if(timeToConflict == 0 && timeToConflictOtherDriver == 0)
						{
							//Print() << "\tTimeToConflict == 0 && TimeToConflictOtherDriver == 0";

							//Compare the distances, if the one nearer to the conflict can go through
							if (distToConflict > abs(itNearestVehicles->distance)
									&& itNearestVehicles->driver->getYieldingToInIntersection() == -1)
							{
								//Print() << "Other driver is closer to conflict.";
								isGapRejected = true;
							}
							else
							{
								//Print() << "I'm closer to conflict.";
							}
						}
						//All other cases, reject gap
						else
						{
							isGapRejected = true;
						}						
					}
					
					//If the gap has been rejected (slow down and stop)
					if (isGapRejected)
					{
						//Print() << "\tisGapRejected == true";
						//Calculate the deceleration required to stop before conflict
						double brakingAcc = brakeToStop(stoppingDist, params);

						//Print() << "\tBrakingAcc:" << brakingAcc << "\tAcc:" << acc;

						//If this deceleration is smaller than the one we have previously, use this
						if (acc > brakingAcc)
						{
							acc = brakingAcc;
							params.driver->setYieldingToInIntersection(paramsOtherDriver.parentId);

							//Print() << "\tYieldVeh:" << paramsOtherDriver.parentId;
							//Print() << "\tGapReject!";
						}
					}					
				}
			}
		}
		else
		{
			//Print() << "\nConflict " << (*itConflicts)->getDbId() << " crossed. Distance: " << distToConflict;
		}
	}
	*/
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
	double acc = 0;
	
	if (distance > sim_mob::Math::DOUBLE_EPSILON)
	{
		//v^2 = u^2 + 2as (Equation of motion)
		//So, a = (v^2 - u^2) / 2s
		//v = final velocity, u = initial velocity
		//a = acceleration, s = displacement
		double sqCurrVel = params.currSpeed * params.currSpeed;
		
		acc = -sqCurrVel / distance * 0.5;

		if (acc <= params.normalDeceleration)
		{
			acc = params.normalDeceleration;
		}
		else
		{
			double dt = params.nextStepSize;
			double vt = params.currSpeed * dt;
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
		acc = (dt > 0.0) ? -(params.currSpeed) / dt : params.maxDeceleration;
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

double MITSIM_IntDriving_Model::crawlingAcc(double distance, DriverUpdateParams& params)
{
	//Acceleration to slowly crawl towards the conflict point
	return 2 * ((distance / 2) - params.currSpeed * params.nextStepSize) / (params.nextStepSize * params.nextStepSize);
}

double MITSIM_IntDriving_Model::calcArrivalTime(double distance, DriverUpdateParams& params)
{
	double arrivalTime = -1, acceleration = 0, finalVel = 0;
	
	//The final velocity is limited by the turning speed, so calculate the acceleration required to
	//achieve the final velocity
	//v^2 = u^2 + 2as
	//So, a = (v^2 - u^2) / (2s)
	
	//Get the speed limit and convert it to m/s
	finalVel = 1;//currTurning->getTurningSpeed() / 3.6;
	
	//Calculate the acceleration
	acceleration = ((finalVel * finalVel) - (params.currSpeed * params.currSpeed)) / (2 * distance);
	
	//We know s = ut + (1/2)at^2
	//To find the time required, we rearrange the equation as follows:
	//(1/2)at^2 + ut - s = 0	This is a quadratic equation AX^2 + BX + C = 0 and we can solve for t
	//A = (1/2)a; B = u; C = -s
	
	if(acceleration == 0 && params.currSpeed != 0)
	{
		//Acceleration is 0, so we have a linear relation : ut - s = 0
		arrivalTime = distance / params.currSpeed;
	}
	else
	{
		//As it is a quadratic equation, we will have two solutions
		double sol1 = 0, sol2 = 0;

		//The discriminant (b^2 - 4ac)
		double discriminant = (params.currSpeed * params.currSpeed) - (2 * acceleration * (-distance));

		if(discriminant >= 0)
		{
			//Calculate the solutions
			sol1 = (-params.currSpeed - sqrt(discriminant)) / acceleration;
			sol2 = (-params.currSpeed + sqrt(discriminant)) / acceleration;

			//As time can be negative, return the solution that is a positive value	
			if (sol1 >= 0 && sol2 >= 0)
			{
				arrivalTime = min(sol1, sol2);
			}
			else if (sol1 >= 0)
			{
				arrivalTime = sol1;
			}
			else if(sol2 >= 0)
			{
				arrivalTime = sol2;
			}
		}		
	}
	
	return arrivalTime;
}
