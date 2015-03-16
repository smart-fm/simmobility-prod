//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
// license.txt (http://opensource.org/licenses/MIT)

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "DriverUpdateParams.hpp"
#include "models/IntersectionDrivingModel.hpp"
#include "Driver.hpp"

#include <math.h>

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
//	makePolypoints(fromLanePt,toLanePt);
	polypoints = currTurning->getPolylinePoints();
	// calculate polyline length
	length = 0;
	for(int i=0;i<polypoints.size()-1;++i){
		double dx = polypoints.at(i+1).x-polypoints.at(i).x;
		dx=dx*dx;
		double dy = polypoints.at(i+1).y-polypoints.at(i).y;
		dy=dy*dy;
		length += sqrt(dx+dy);
	}
	polypointIter = polypoints.begin();
//	intTrajectory = DynamicVector (fromLanePt.x, fromLanePt.y, toLanePt.x, toLanePt.y);
	DPoint p1 = (*polypointIter);
	DPoint p2 = *(polypointIter+1);
	currPolyline = DynamicVector ( (*polypointIter).x, (*polypointIter).y, (*(polypointIter+1)).x, (*(polypointIter+1)).y);
	polypointIter++;
	if(polypointIter == polypoints.end()) {
		//polypoints only have one point? throw error
		std::string str = "MITSIM_IntDriving_Model polypoints only have one point.";
		throw std::runtime_error(str);
	}
	currPosition = fromLanePt;
	totalMovement = startOffset;
	polylineMovement = startOffset;
}

DPoint MITSIM_IntDriving_Model::continueDriving(double amount,DriverUpdateParams& p)
{
	if(amount == 0){
		return currPosition;
	}
	totalMovement += amount;
	// check "amout" exceed the rest length of the DynamicVector
	DynamicVector tt(currPosition.x,currPosition.y,currPolyline.getEndX(),currPolyline.getEndY());
	double restLen = tt.getMagnitude();

	if (amount > restLen &&  polypointIter != polypoints.end() && polypointIter+1 != polypoints.end() ){
		// move to next polyline, if has
		polylineMovement = amount - restLen;
		currPolyline = DynamicVector ( (*polypointIter).x, (*polypointIter).y, (*(polypointIter+1)).x, (*(polypointIter+1)).y);
		polypointIter++;

//		currPolyline.scaleVectTo (totalMovement).translateVect ();
//		currPosition = DPoint (currPolyline.getX (), currPolyline.getY ());
	}
	else {
		polylineMovement += amount;
	}

	std::cout<<std::endl;
	std::cout<<"tick: "<<p.now.frame()<<" amount: "<<amount<<" restLen: "<<restLen<< " polylineMovement: "<<polylineMovement<< " totalMovement: "<<totalMovement<<std::endl;
	std::cout<<std::setprecision(10)<<"currPolyline: "<<currPolyline.getX()<<" "<<currPolyline.getY()<<" "<<currPolyline.getEndX()<<" "<<currPolyline.getEndY()<<std::endl;
	DynamicVector temp = currPolyline;
	temp.scaleVectTo (polylineMovement).translateVect ();

	currPosition = DPoint (temp.getX (), temp.getY ());

	std::cout<<"currPosition: "<<currPosition.x<<" "<<currPosition.y<<std::endl;

//	currPolyline = (currPosition.x,currPosition.y,currPolyline.getEndX(),currPolyline.getEndY());

	return currPosition;
}
bool MITSIM_IntDriving_Model::isDone() {
	return totalMovement >= length;
}
double MITSIM_IntDriving_Model::getCurrentAngle(){
	return currPolyline.getAngle();
}
void MITSIM_IntDriving_Model::makePolypoints(const DPoint& fromLanePt, const DPoint& toLanePt) {
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
	if(fromLanePt.x > toLanePt.x){
		xm = toLanePt.x + abs(dx)/2.0;
	}
	else {
		xm = fromLanePt.x + abs(dx)/2.0;
	}

	if(fromLanePt.y > toLanePt.y){
		ym = toLanePt.y + abs(dy)/2.0;
	}
	else {
		ym = fromLanePt.y + abs(dy)/2.0;
	}

	double xx = b* cos(kk) + xm;
	double yy = b*sin(kk) + ym;

	DPoint dp(xx,yy);

	polypoints.push_back(fromLanePt);
	polypoints.push_back(dp);
	polypoints.push_back(toLanePt);

	std::cout<<std::setprecision(10)<<"dp x: "<<dp.x<<" y: "<<dp.y<<std::endl;
	//
	length = sqrt( (fromLanePt.x-dp.x)*(fromLanePt.x-dp.x) + (fromLanePt.y-dp.y)*(fromLanePt.y-dp.y));
	length += sqrt( (toLanePt.x-dp.x)*(toLanePt.x-dp.x) + (toLanePt.y-dp.y)*(toLanePt.y-dp.y));
}
double MITSIM_IntDriving_Model::makeAcceleratingDecision(DriverUpdateParams& params, const TurningSection* currTurning)
{
	double acc = params.maxAcceleration;
	const double vehicleLength = params.driver->getVehicleLengthM();
	double distToStopLine = params.driver->distToIntersection_.get() - (1.5 * vehicleLength);
	
	Print() << "\nTime:" << params.now.frame();
	Print() << "\nID:" << params.parentId;
	Print() << "\nDistToStopLine:" << distToStopLine;
	
	//Check if we've stopped close enough to the stop line
	if (distToStopLine <= 1 && params.currSpeed <= 0.1)
	{
		params.hasStoppedForStopSign = true;
		Print() << "\nStopped for Stop sign - Done";
	}
	
	//We have to stop for the stop sign
	if (currTurning->turningHasStopSign() && ! params.hasStoppedForStopSign
		&& distToStopLine <= 10)
	{
		double brakingAcc = brakeToStop(distToStopLine, params);

		if (acc > brakingAcc && params.currSpeed != 0)
		{
			acc = brakingAcc;
			params.driver->setYieldingToInIntersection(-1);			
		}				
		
		Print() << "\nStopping for stop sign";
		
		return acc;
	}

	//Vector of conflicts for the current turning
	const vector<TurningConflict *> &conflicts = currTurning->getTurningConflicts();

	//Select the nearest conflict ahead of us (the vector is sorted according to the distance from the start of the turning,
	//we may have crossed some)
	for (vector<TurningConflict *>::const_iterator itConflicts = conflicts.begin(); itConflicts != conflicts.end(); ++itConflicts)
	{
		Print() << "\tConflict:" << (*itConflicts)->getDbId();
		
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
			if (priority == 1)
			{
				//Our turning has priority, ignore the vehicles in this conflict
				continue;
			}
			
			//Get the distance to conflict point
			cfltDistFrmTurning = (*itConflicts)->getFirst_cd();
		}
		else
		{
			if (priority == 2)
			{
				//Our turning has priority, ignore the vehicles in this conflict
				continue;
			}
			
			//Get the distance to conflict point
			cfltDistFrmTurning = (*itConflicts)->getSecond_cd();
		}
		
		Print() << "\tConfDist:" << cfltDistFrmTurning;
		Print() << "\tDistOnTurn:" << params.driver->moveDisOnTurning_ / 100;
	
		//Calculate the distance to conflict point from current position
		distToConflict = cfltDistFrmTurning - (params.driver->moveDisOnTurning_ / 100);
		
		//If the vehicle is approaching the intersection rather than already in it, add the distance to the end
		//of the segment to the distance to conflict
		if (params.isApproachingIntersection)
		{
			distToConflict += params.driver->distToIntersection_.get();
		}
		
		Print() << "\tDistToConflict:" << distToConflict;
		
		//The distance at which the vehicle needs to come to a stand-still when yielding
		double stoppingDist = 0;
		
		//If the turning has a stop sign, we stop just before the	intersection
		if (currTurning->turningHasStopSign())
		{
			stoppingDist = distToStopLine;
		}
		else
		{
			stoppingDist = distToConflict - (1.5 * vehicleLength);
		}

		if (stoppingDist < 0)
		{
			stoppingDist = 0;
		}

		Print() << "\tStoppingDist:" << stoppingDist;

		//If we're yet to reach the conflict, calculate the time required for us and the conflict driver
		//else go to the next conflict
		if (distToConflict > 0.0)
		{
			//Time taken to reach conflict by current driver
			double timeToConflict = 0;
			
			if (params.currSpeed > 1.0)
			{
				timeToConflict = abs(distToConflict) / params.currSpeed;
			}
			
			Print() << "\tTimeToConflict:" << timeToConflict;

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

			Print() << "\tCriticalGap:" << criticalGap;			

			//The map entry to the conflict and list of NearestVehicles on the it
			map<TurningConflict *, list<NearestVehicle> >::iterator itConflictVehicles = params.conflictVehicles.find(*itConflicts);
			
			//Check if we have found an entry in the map
			if (itConflictVehicles != params.conflictVehicles.end())
			{
				Print() << "\n#Vehicles spotted:" << itConflictVehicles->second.size();
				
				//Iterator to the list of nearest vehicles on the list
				list<NearestVehicle>::iterator itNearestVehicles = itConflictVehicles->second.begin();

				//Look for the vehicle reaching the conflict point - the list is sorted according to the distance from the conflict
				//point - distance less than 0 means yet to reach the conflict and greater than 0 means crossed the conflict point
				for (; itNearestVehicles != itConflictVehicles->second.end(); ++itNearestVehicles)
				{
					//Check if the vehicle is yet to arrive at the conflict point
					if (itNearestVehicles->distance <= itNearestVehicles->driver->getVehicleLengthM())
					{
						Driver *drv = const_cast<Driver *>(itNearestVehicles->driver);
						DriverUpdateParams &paramsOtherDriver = drv->getParams();
						
						Print() << "\nSpottedVeh:" << paramsOtherDriver.parentId;
						
						//If a driver is already yielding to us, scan the next conflict
						if (itNearestVehicles->driver->getYieldingToInIntersection() == params.parentId)
						{
							break;
						}					
						
						//Time taken to reach conflict point by incoming driver
						double timeToConflictOtherDriver = 0;
						
						//Speed of the incoming vehicle (m/s))
						double speed = itNearestVehicles->driver->getVehicle()->getVelocity() / 100;

						if (speed > 1.0)
						{
							//Negate the distance while calculating the time
							timeToConflictOtherDriver = abs(itNearestVehicles->distance) / speed;
						}
						
						Print() << "\tTimeToConflictOther:" << timeToConflictOtherDriver;
						
						//The gap between the drivers
						double gap = abs(timeToConflictOtherDriver - timeToConflict);
						
						Print() << "\tGap:" << gap;						
												
						//If the gap between current driver and the conflicting driver is less than the critical gap,
						//reject the gap (slow down)
						if (gap < criticalGap)
						{
							Print() << "\tgap < criticalGap";
							
							//If we're at stand-still or crawling continue to crawl
							if(params.currSpeed <= 1)					
							{
								//If we're stationary, crawl to conflict point
								double crawlAcc = crawlingAcc(stoppingDist, params);
								
								Print() << "\tCrawlAcc:" << crawlAcc << "\tAcc:" << acc;

								if (acc > crawlAcc)
								{
									acc = crawlAcc;
									params.driver->setYieldingToInIntersection(paramsOtherDriver.parentId);
									
									Print() << "\tYieldVeh:" << paramsOtherDriver.parentId;
									Print() << "\tCrawl!";
								}
							}
							else
							{
								//Calculate the deceleration required to stop before conflict
								double brakingAcc = brakeToStop(stoppingDist, params);
								
								Print() << "\tBrakingAcc:" << brakingAcc << "\tAcc:" << acc;

								//If this deceleration is smaller than the one we have previously, use this
								if (acc > brakingAcc)
								{
									acc = brakingAcc;
									params.driver->setYieldingToInIntersection(paramsOtherDriver.parentId);
									
									Print() << "\tYieldVeh:" << paramsOtherDriver.parentId;
									Print() << "\tGapReject!";
								}
							}
						}						
						//The gap was accepted, but we need to check if there's enough space after the conflict
						//point. A vehicle with higher priority might collide with us if we can't go past 
						//the conflict point
						else if (timeToConflictOtherDriver != 0)
						{
							Print() << "\tgap >= criticalGap";

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
								//Calculate the deceleration required to stop before conflict
								double brakingAcc = brakeToStop(stoppingDist, params);

								Print() << "\tBrakingAcc:" << brakingAcc << "\tAcc:" << acc;

								//If this deceleration is smaller than the one we have previously, use this
								if (acc > brakingAcc)
								{
									acc = brakingAcc;
									params.driver->setYieldingToInIntersection(paramsOtherDriver.parentId);

									Print() << "\tYieldVeh:" << paramsOtherDriver.parentId;
									Print() << "\tGapAccept, but Vehicle in front!";
								}
							}
						}
					}
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
	if (distance > sim_mob::Math::DOUBLE_EPSILON)
	{
		//v^2 = u^2 + 2as (Equation of motion)
		//So, a = (v^2 - u^2) / 2s
		//v = final velocity, u = initial velocity
		//a = acceleration, s = displacement
		double sqCurrVel = params.currSpeed * params.currSpeed;
		double acc = -sqCurrVel / distance * 0.5;

		if (acc >= params.normalDeceleration)
		{
			return acc;
		}

		double dt = params.nextStepSize;
		double vt = params.currSpeed * dt;
		double a = dt * dt;
		double b = 2.0 * vt + params.normalDeceleration * a;
		double c = sqCurrVel - 2.0 * params.normalDeceleration * distance - vt;
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

double MITSIM_IntDriving_Model::crawlingAcc(double distance, DriverUpdateParams& params)
{
	//Acceleration to slowly crawl towards the conflict point
	return 2 * ((distance / 2) - params.currSpeed * params.nextStepSize) / (params.nextStepSize * params.nextStepSize);
}
