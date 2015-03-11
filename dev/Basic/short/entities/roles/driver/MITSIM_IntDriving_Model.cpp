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
	makePolypoints(fromLanePt,toLanePt);
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
