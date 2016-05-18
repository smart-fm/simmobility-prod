//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <boost/random.hpp>
#include <limits>

#include "Driver.hpp"
#include "IncidentPerformer.hpp"
#include "entities/roles/driver/models/LaneChangeModel.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "util/Math.hpp"
#include "util/Utils.hpp"
#include "geospatial/network/RoadNetwork.hpp"

using std::numeric_limits;
using namespace std;
using namespace sim_mob;

namespace
{
//Declare MAX_NUM as a private variable here to limit its scope.
const double MAX_NUM = numeric_limits<double>::max();

template <class T> struct LeadLag
{
	T lead;
	T lag;
};
}

MITSIM_LC_Model::MITSIM_LC_Model(DriverUpdateParams &params, DriverPathMover *pathMover) : LaneChangingModel(pathMover)
{
	modelName = "general_driver_model";
	splitDelimiter = " ,";

	readDriverParameters(params);
}

MITSIM_LC_Model::~MITSIM_LC_Model()
{
}

void MITSIM_LC_Model::readDriverParameters(DriverUpdateParams &params)
{
	std::string str;
	bool isAMOD = false;

	if (params.driver->getParent()->amodId != "-1")
	{
		isAMOD = true;
	}

	ParameterManager *parameterMgr = ParameterManager::Instance(isAMOD);

	//MLC_PARAMETERS
	parameterMgr->param(modelName, "MLC_PARAMETERS", str, string("1320.0  5280.0 0.5 1.0  1.0"));
	makeMLCParam(str);

	//LC_GAP_MODELS
	std::vector< std::string > strArray;
	parameterMgr->param(modelName, "LC_GAP_MODELS_0", str, string("1.00,   0.0,   0.000,  0.508,  0.000,  0.000,  -0.420, 0.000,   0.488"));
	strArray.push_back(str);
	parameterMgr->param(modelName, "LC_GAP_MODELS_1", str, string("1.00, 0.0, 0.000, 2.020, 0.000, 0.000, 0.153, 0.188, 0.526"));
	strArray.push_back(str);
	parameterMgr->param(modelName, "LC_GAP_MODELS_2", str, string("1.00, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859"));
	strArray.push_back(str);
	parameterMgr->param(modelName, "LC_GAP_MODELS_3", str, string("1.00, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073"));
	strArray.push_back(str);
	parameterMgr->param(modelName, "LC_GAP_MODELS_4", str, string("0.60, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859"));
	strArray.push_back(str);
	parameterMgr->param(modelName, "LC_GAP_MODELS_5", str, string("0.60, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073"));
	strArray.push_back(str);
	parameterMgr->param(modelName, "LC_GAP_MODELS_6", str, string("0.20, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859"));
	strArray.push_back(str);
	parameterMgr->param(modelName, "LC_GAP_MODELS_7", str, string("0.20, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073"));
	strArray.push_back(str);
	makeCriticalGapParams(params, strArray);

	//GAP_PARAM
	strArray.clear();
	parameterMgr->param(modelName, "GAP_PARAM_0", str, string("-1.23, -0.482, 0.224, -0.0179, 2.10, 0.239"));
	strArray.push_back(str);
	parameterMgr->param(modelName, "GAP_PARAM_1", str, string("0.00,   0.00,  0.224, -0.0179, 2.10, 0.000"));
	strArray.push_back(str);
	parameterMgr->param(modelName, "GAP_PARAM_2", str, string("-0.772, -0.482, 0.224, -0.0179, 2.10, 0.675"));
	strArray.push_back(str);
	makeTargetGapPram(strArray);

	//Minimum speed
	parameterMgr->param(modelName, "min_speed", minSpeed, 0.1);

	//Driver look ahead distance
	lookAheadDistance = mlcDistance();

	//Lane Utility parameters
	parameterMgr->param(modelName, "lane_utility_model", str, string("3.9443 -0.3213  -1.1683  -1.1683 0.0 0.0633 -1.0 0.0058 -0.2664 -0.0088 -3.3754 10 19 -2.3400 -4.5084 -2.8257 -1.2597 -0.7239 -0.3269"));
	makeLaneUtilityParams(str);

	//Critical gap parameters
	parameterMgr->param(modelName, "critical_gaps_param", str, string("0.5 -0.231  -2.700  1.112	0.5   0.000 0.2  0.742   6.0"));
	makeCriticalGapParams(str);

	//Nosing parameters
	parameterMgr->param(modelName, "nosing_param", str, string("1.0 0.5  0.6  0.1	0.2   1.0 300.0  180.0   600.0 40.0"));
	makeNosingParams(params, str);

	parameterMgr->param(modelName, "MLC_Yielding_Probabilities", str, string("0.13 0.71  0.13  0.03"));
	makeLC_YieldingProbabilities(str);

	//Kazi Nosing parameters
	parameterMgr->param(modelName, "kazi_nosing_param", str, string("-3.159  0.313  -0.027  2.050  0.028  0.6"));
	makeKaziNosingParams(str);

	//CF_CRITICAL_TIMER_RATIO
	parameterMgr->param(modelName, "CF_CRITICAL_TIMER_RATIO", CF_CRITICAL_TIMER_RATIO, 0.5);

	//LC Yielding Model
	parameterMgr->param(modelName, "LC_Yielding_Model", str, string("0.80 1.0"));

	//Minimum time in lane in same direction
	parameterMgr->param(modelName, "LC_Discretionary_Lane_Change_Model_MinTimeInLaneSameDir", minTimeInLaneSameDir, 2.0);

	//Minimum time in lane in different direction
	parameterMgr->param(modelName, "LC_Discretionary_Lane_Change_Model_MinTimeInLaneDiffDir", minTimeInLaneDiffDir, 2.0);

	//Target Gap Model
	parameterMgr->param(modelName, "Target_Gap_Model", str, string("-0.837   0.913  0.816  -1.218  -2.393  -1.662"));
	Utils::convertStringToArray(str, params.targetGapParams);

	parameterMgr->param(modelName, "check_stop_point_distance", params.stopVisibilityDistance, 100.0);
}

double MITSIM_LC_Model::calcCriticalGapKaziModel(DriverUpdateParams &params, int type, double distance, double diffInSpeed)
{
	std::vector<double> a = params.LC_GAP_MODELS[type];

	//                    beta0            beta1                  beta2                    beta3                        beta4
	double b[] = {params.LC_GAP_MODELS[type][3], params.LC_GAP_MODELS[type][4], params.LC_GAP_MODELS[type][5], params.LC_GAP_MODELS[type][6], params.LC_GAP_MODELS[type][7]};
	double rem_dist_impact = (type < 3) ? 0.0 : (1.0 - 1.0 / (1 + exp(a[2] * distance)));
	double dvNegative = (diffInSpeed < 0) ? diffInSpeed : 0.0;
	double dvPositive = (diffInSpeed > 0) ? diffInSpeed : 0.0;
	double gap = b[0] + b[1] * rem_dist_impact + b[2] * diffInSpeed + b[3] * dvNegative + b[4] * dvPositive;

	boost::normal_distribution<> nrand(0, b[4]);
	boost::variate_generator< boost::mt19937, boost::normal_distribution<> > normal(params.gen, nrand);
	double u = gap + normal();
	double criGap = 0;

	if (u < -4.0)
	{
		criGap = 0.0183 * a[0]; //exp(-4)=0.0183
	}
	else if (u > 6.0)
	{
		criGap = 403.4 * a[0]; //exp(6)=403.4  
	}
	else
	{
		criGap = a[0] * exp(u);
	}

	return (criGap < a[1]) ? a[1] : criGap;
}

void MITSIM_LC_Model::chooseTargetGap(DriverUpdateParams &params)
{
	params.lcDebugStr << "===CTG";

	// 1.0 SET/UNSET choose adjacent,forward,backward STATUS
	params.unsetStatus(STATUS_TARGET_GAP);

	// 2.0 if doing nosing ,just return
	if (params.flag(FLAG_NOSING))
	{
		params.lcDebugStr << ";NOS";
		return;
	}
	if (params.flag(FLAG_NOSING_FEASIBLE))
	{
		params.lcDebugStr << ";NOF";
		return;
	}

	if (params.flag(FLAG_STUCK_AT_END))
	{
		params.lcDebugStr << ";STK";
		return;
	}

	// 3.0 check lane change decision direction
	LaneChangeTo changeMode = LANE_CHANGE_TO_NONE;
	if (params.getStatus(STATUS_LEFT))
	{
		changeMode = LANE_CHANGE_TO_LEFT;
	}
	else if (params.getStatus(STATUS_RIGHT))
	{
		changeMode = LANE_CHANGE_TO_RIGHT;
	}
	else
	{
		return; // No request for lane change,just return
	}

	// TODO: need check incident of current lane? MITSIM has "if (isInIncidentArea(plane)) return;"

	// 4.0 get lead,lag vehicles
	// FRONT, LEAD AND LAG VEHICLES (do not have to be in same segment)
	// TODO: check aura manager function inn driver facets
	const NearestVehicle * av = NULL; // side leader
	const NearestVehicle * bv = NULL; // side follower
	const NearestVehicle * front = &params.nvFwd;
	const NearestVehicle * aav = NULL; //side forward of forward
	const NearestVehicle * bbv = NULL; //side backward of backward

	if (changeMode == LANE_CHANGE_TO_LEFT)
	{
		av = &params.nvLeftFwd;
		bv = &params.nvLeftBack;
		aav = &params.nvLeftFwd2;
		bbv = &params.nvLeftBack2;
	}
	else
	{
		av = &params.nvRightFwd;
		bv = &params.nvRightBack;
		aav = &params.nvRightFwd2;
		bbv = &params.nvRightBack2;
	}

	// 5.0 easy set
	// if no left/right ahead vehicle, just use adjacent gap
	if (!av->exists())
	{
		params.setStatus(STATUS_ADJACENT);
		params.lcDebugStr << ";ava";
		return;
	}
	if (!bv->exists())
	{
		params.setStatus(STATUS_ADJACENT);
		params.lcDebugStr << ";bva";
		return;
	}

	Driver *avDriver = const_cast<Driver*> (av->driver);
	DriverMovement *avDriverMvt = dynamic_cast<DriverMovement*> (avDriver->Movement());
	Driver *bvDriver = const_cast<Driver*> (bv->driver);
	DriverMovement *bvDriverMvt = dynamic_cast<DriverMovement*> (bvDriver->Movement());
	Driver *frontDriver = NULL;

	if (front->exists())
	{
		params.lcDebugStr << ";d1";
		frontDriver = const_cast<Driver*> (front->driver);
	}
	Driver *bbvDriver = NULL;
	if (bbv->exists())
	{
		params.lcDebugStr << ";d2";
		bbvDriver = const_cast<Driver*> (bbv->driver);
	}

	// 6.0 calculate FORWARD GAP utility value
	float dis2gap = 0.0;
	float effectiveGap = 0.0;
	float remainderGap = 0.0;
	float gapSpeed = 0.0;
	float dis2front = 0.0;

	float d1, s1, d2;

	if (aav->exists())
	{
		// if has forward of forward vh
		params.lcDebugStr << ";d3";
		d1 = aav->distance - av->distance - av->driver->getVehicleLength(); // get gap length of av and aav
		Driver *aavDriver = const_cast<Driver*> (aav->driver);
		s1 = av->driver->getFwdVelocity() - aavDriver->getFwdVelocity(); // speed diff of av and aav
	}
	else
	{
		params.lcDebugStr << ";d4";

		// get side ahead vh distance to end of link
		// tmp solution distance to next segment ,if next seg exist
		// TODO: meaning of av->distance()
		if (avDriverMvt->fwdDriverMovement.getNextSegment())
		{
			params.lcDebugStr << ";d5";
			d1 = avDriverMvt->fwdDriverMovement.getDistToEndOfCurrWayPt() + avDriverMvt->fwdDriverMovement.getNextSegment()->getLength();
		}
		else
		{
			params.lcDebugStr << ";d6";
			d1 = avDriverMvt->fwdDriverMovement.getDistToEndOfCurrWayPt();
		}

		s1 = 0;
	}

	dis2gap = av->distance;

	if (!front->exists())
	{
		params.lcDebugStr << ";d7";
		effectiveGap = d1;
		remainderGap = 0;
		gapSpeed = s1;
	}
	else
	{
		params.lcDebugStr << ";d8";
		dis2front = front->distance;

		if (dis2gap > dis2front)
		{
			params.lcDebugStr << ";d9";
			effectiveGap = (-1) * (frontDriver->gapDistance(avDriver) + av->driver->getVehicleLength() + front->driver->getVehicleLength());
			remainderGap = d1;
			gapSpeed = avDriver->getFwdVelocity() - frontDriver->getFwdVelocity();
		}
		else
		{
			params.lcDebugStr << ";d10";
			d2 = avDriver->gapDistance(frontDriver);
			if (d1 >= d2)
			{
				params.lcDebugStr << ";d11";
				effectiveGap = d2;
				remainderGap = d1 - d2;
				gapSpeed = avDriver->getFwdVelocity() - frontDriver->getFwdVelocity();
			}
			else
			{
				params.lcDebugStr << ";d22";
				effectiveGap = d1;
				remainderGap = 0;
				gapSpeed = s1;
			}
		}
	}

	double eufwd = gapExpOfUtility(params, 1, effectiveGap, dis2gap, gapSpeed, remainderGap);

	// 7.0 ADJACENT GAP

	// adjacent gap length
	d1 = av->distance + bv->distance;
	s1 = bvDriver->getFwdVelocity() - avDriver->getFwdVelocity();

	dis2gap = 0;

	if (!front->exists())
	{
		effectiveGap = d1;
		remainderGap = 0;
		gapSpeed = s1;
	}
	else
	{
		d2 = bvDriver->gapDistance(frontDriver);
		if (d1 > dis2front)
		{
			effectiveGap = d2;
			remainderGap = d1 - d2;
			gapSpeed = bvDriver->getFwdVelocity() - frontDriver->getFwdVelocity();
		}
		else
		{
			effectiveGap = d1;
			remainderGap = 0;
			gapSpeed = s1;
		}
	}


	double euadj = gapExpOfUtility(params, 3, effectiveGap, dis2gap, gapSpeed, remainderGap);

	// 8.0 BACKWARD GAP

	if (bbv->exists())
	{
		d1 = bbv->distance - bv->distance - bv->driver->getVehicleLength(); // get gap length of bv and bbv
		s1 = bbvDriver->getFwdVelocity() - bvDriver->getFwdVelocity();
	}
	else
	{

		s1 = 0;
		// get side back vh distance to start of link
		// tmp solution distance to move along segment ,if next seg exist
		// TODO: meaning of d1 = bv->lane()->length() - bv->distance();
		d1 = bvDriverMvt->fwdDriverMovement.getDistCoveredOnCurrWayPt();

	}

	dis2gap = bv->distance;

	effectiveGap = d1;
	remainderGap = 0;
	gapSpeed = s1;


	double eubck = gapExpOfUtility(params, 2, effectiveGap, dis2gap, gapSpeed, remainderGap);

	double sum = eufwd + eubck + euadj;
	double rnd = Utils::uRandom();
	if (rnd < euadj / sum) params.setStatus(STATUS_ADJACENT);
	else if (rnd < (euadj + eubck) / sum) params.setStatus(STATUS_BACKWARD);
	else params.setStatus(STATUS_FORWARD);

	return;
}

double MITSIM_LC_Model::gapExpOfUtility(DriverUpdateParams &params, int n, float effectiveGap, float distToGap, float gapSpeed, float remainderGap)
{
	std::vector<double> a = params.targetGapParams;

	double u = a[2] * effectiveGap + a[3] * distToGap + a[4] * gapSpeed;

	if (remainderGap > 0.1)
	{
		u += a[5];
	}

	if (n == 1)
	{
		u += a[0];
	}
	else if (n == 2)
	{
		u += a[1];
	}

	return exp(u);
}

void MITSIM_LC_Model::makeMLCParam(std::string &str)
{
	std::vector<double> array;
	Utils::convertStringToArray(str, array);
	MLC_PARAMETERS.lowbound = array[0];
	MLC_PARAMETERS.delta = array[1];
	MLC_PARAMETERS.lane_mintime = array[2];
}

void MITSIM_LC_Model::makeCriticalGapParams(DriverUpdateParams &params, std::vector<std::string> &strMatrix)
{
	for (int i = 0; i < strMatrix.size(); ++i)
	{
		std::vector<double> array;
		Utils::convertStringToArray(strMatrix[i], array);
		params.LC_GAP_MODELS.push_back(array);
	}
}

void MITSIM_LC_Model::makeTargetGapPram(std::vector<std::string> &strMatrix)
{
	for (int i = 0; i < strMatrix.size(); ++i)
	{
		std::vector<double> array;
		Utils::convertStringToArray(strMatrix[i], array);
		GAP_PARAM.push_back(array);
	}
}

void MITSIM_LC_Model::makeLaneUtilityParams(std::string &str)
{
	Utils::convertStringToArray(str, laneUtilityParams);
}

void MITSIM_LC_Model::makeNosingParams(DriverUpdateParams &params, string &str)
{
	Utils::convertStringToArray(str, params.nosingParams);
	lcMaxNosingDis = params.nosingParams[8];
	lcMaxStuckTime = params.nosingParams[7];
	lcNosingConstStateTime = params.nosingParams[0];
	params.lcMaxYieldingTime = params.nosingParams[6];
}

void MITSIM_LC_Model::makeKaziNosingParams(string &str)
{
	Utils::convertStringToArray(str, kaziNosingParams);
}

void MITSIM_LC_Model::makeLC_YieldingProbabilities(string &str)
{
	Utils::convertStringToArray(str, lcYieldingProb);
}

void MITSIM_LC_Model::makeCriticalGapParams(std::string &str)
{
	Utils::convertStringToArray(str, criticalGapParams);
}

LaneChangeTo MITSIM_LC_Model::checkForLC_WithLookAhead(DriverUpdateParams &params)
{
	LaneChangeTo change = LANE_CHANGE_TO_NONE;

	params.lcDebugStr << ";checkDLC";

	//Check if we will be doing a mandatory lane change soon
	if (params.flag(FLAG_ESCAPE))
	{
		params.lcDebugStr << ";FLAG_ESCAPE";
		return change;
	}

	//Find lanes connect to target segment in lookahead distance
	/*std::vector<const Lane*> connectedLanes;
	getConnectedLanesInLookAheadDistance(params, lookAheadDistance, connectedLanes);
	params.lcDebugStr << ";clilad" << connectedLanes.size();*/

	int nRight = -1; // number of lane changes required for the current lane.
	int nLeft = -1; // number of lane changes required for the current lane.
	int nCurrent = -1; // number of lane changes required for the current lane.

	//Compute number of lane changes to right,left,current
	set<const Lane *>::iterator it;
	for (it = params.targetLanes.begin(); it != params.targetLanes.end(); ++it)
	{
		const Lane *lane = *it;
		size_t targetLaneIdx = lane->getLaneIndex();
		if (targetLaneIdx < params.currLaneIndex)
		{
			//Target lane is on the left of current lane
			int numberlc = params.currLaneIndex - targetLaneIdx;
			if (numberlc < nLeft || nLeft < 0)
			{
				nLeft = numberlc;
			}
		}
		else if (targetLaneIdx > params.currLaneIndex)
		{
			//Target lane on the right
			int numberlc = targetLaneIdx - params.currLaneIndex;
			if (numberlc < nRight || nRight < 0)
			{
				nRight = numberlc;
			}
		}
		else
		{
			nCurrent = 0;
		}		
	}

	params.lcDebugStr << ";nR" << nRight << ";nL" << nLeft << ";nC" << nCurrent;

	double eul = 0.0, eur = 0.0, euc = 1.0;
	double lcDistance = params.distToStop;

	int res = isReadyForNextDLC(params, 2);
	params.lcDebugStr << ";isRyL" << res;
	if (res && nLeft > 0 && params.leftLane)
	{
		eul = lcUtilityLookAheadLeft(params, nLeft, lcDistance);
		params.lcDebugStr << ";doeul" << eul;
	}

	res = isReadyForNextDLC(params, 1);
	params.lcDebugStr << ";isRyR" << res;
	if (res && nRight > 0 && params.rightLane)
	{
		eur = lcUtilityLookAheadRight(params, nRight, lcDistance);
		params.lcDebugStr << ";doeur" << eur;
	}

	double sum = eul + eur;
	params.lcDebugStr << ";sum" << sum;
	if (sum > 0)
	{
		euc = lcUtilityLookAheadCurrent(params, nCurrent, lcDistance);
		params.lcDebugStr << ";doeuc" << euc;
	}
	else
	{
		params.utilityCurrent = 1.0;
		params.lcd = "lcd-cc";
		params.lcDebugStr << ";LCS_SAME";

		return LANE_CHANGE_TO_NONE;
	}

	sum += euc;

	double rnd = Utils::generateFloat(0, 1);
	if (rnd >= 1.0) rnd = 0.99;
	params.lcDebugStr << ";rnd" << rnd;
	float probOfCurrentLane = euc / sum;
	params.lcDebugStr << ";pc" << probOfCurrentLane;
	float probOfCL_RL = probOfCurrentLane + eur / sum;
	params.lcDebugStr << ";pr" << probOfCL_RL;

	params.utilityCurrent = euc / sum;
	params.utilityLeft = eul / sum;
	params.utilityRight = eur / sum;

	if (rnd <= probOfCurrentLane)
	{
		change = LANE_CHANGE_TO_NONE;
		params.lcd = "lcd-c";
		params.lcDebugStr << ";lcd-c";
	}
	else if (rnd <= probOfCL_RL)
	{
		params.lcd = "lcd-r";
		change = LANE_CHANGE_TO_RIGHT;
		params.lcDebugStr << ";lcd-r";
	}
	else
	{
		params.lcd = "lcd-l";
		change = LANE_CHANGE_TO_LEFT;
		params.lcDebugStr << ";lcd-l";
	}

	params.lcDebugStr << ";chg" << change;
	return change;
}

int MITSIM_LC_Model::isReadyForNextDLC(DriverUpdateParams &params, int mode)
{
	float sec = timeSinceTagged(params);
	params.lcDebugStr << ";irfnd" << sec;

	switch (mode)
	{
	case 1: // request a change to the right
	{
		if (params.flag(FLAG_PREV_LC_RIGHT) && sec > minTimeInLaneSameDir)
		{
			params.lcDebugStr << ";1i0";
			return 1;
		}
		else if (sec > minTimeInLaneDiffDir)
		{
			params.lcDebugStr << ";1i1";
			return 1;
		}
		else
		{
			params.lcDebugStr << ";1i2";
			return 0;
		}
	}
	case 2: // request a change to the left
	{
		if (params.flag(FLAG_PREV_LC_LEFT) && sec > minTimeInLaneSameDir)
		{
			params.lcDebugStr << ";2i0";
			return 1;
		}
		else if (sec > minTimeInLaneDiffDir)
		{
			params.lcDebugStr << ";2i1";
			return 1;
		}
		else
		{
			params.lcDebugStr << ";2i3";
			return 0;
		}
	}
	}
	return sec > minTimeInLaneSameDir;
}

int MITSIM_LC_Model::getNumberOfLCToEndOfLink(DriverUpdateParams &params, const Lane *currLane)
{
	int noOfLaneChanges = 0;

	//1.0 Get the last segment in the link	
	const Link *currLink = currLane->getParentSegment()->getParentLink();
	const RoadSegment *lastSeg = currLink->getRoadSegment(currLink->getRoadSegments().size() - 1);

	//2.0 Get the number of way points to the turning at the end of the link
	int noOfWayPts = lastSeg->getSequenceNumber() - currLane->getParentSegment()->getSequenceNumber() + 1;

	//3.0 Access the way-point representing the turning group in the path
	std::vector<WayPoint>::const_iterator itTurningGroup = fwdDriverMovement->getCurrWayPointIt() + noOfWayPts;

	if (itTurningGroup != fwdDriverMovement->getDrivingPath().end())
	{
		if (itTurningGroup->type == WayPoint::TURNING_GROUP)
		{
			//4.0 Find the minimum number of lane changes required to access a turning path in the turning group
			std::map<unsigned int, std::map<unsigned int, TurningPath *> >::const_iterator itTurnings = itTurningGroup->turningGroup->getTurningPaths().begin();
			while (itTurnings != itTurningGroup->turningGroup->getTurningPaths().end())
			{
				//Lane index is lane id % 10, so use the "from lane id" to get the index
				
				int laneChanges = params.currLaneIndex - (itTurnings->first % 10);

				//Update only if fewer lane changes required
				if (noOfLaneChanges == 0 || abs(noOfLaneChanges) > abs(laneChanges))
				{
					noOfLaneChanges = laneChanges;
				}

				++itTurnings;
			}
		}
		else
		{
			throw std::runtime_error("Turning group expected after the link!");
		}
	}
	else
	{
		//The path ends at the end of the link. Simply ensure that we're connected to the last segment
		if (params.currLaneIndex >= lastSeg->getNoOfLanes())
		{
			noOfLaneChanges = params.currLaneIndex - (lastSeg->getNoOfLanes() - 1);
		}
	}

	return noOfLaneChanges;
}

double MITSIM_LC_Model::lcUtilityCurrent(DriverUpdateParams &params)
{
	// 1.0 lane utility parameters
	vector<double> a = laneUtilityParams;

	// 2.0 LEADING AND LAG VEHICLES
	const NearestVehicle * av = &params.nvFwd;
	const NearestVehicle * bv = &params.nvBack;

	// 3.0 count number of lane change to the end of link
	double vld, mlc, density, spacing;
	int n = abs(getNumberOfLCToEndOfLink(params, params.currLane));

	density = params.density;
	float heavy_neighbor = 0.0;

	if (av->exists())
	{
		vld = std::min<double>(params.desiredSpeed, av->driver->getFwdVelocity());
		heavy_neighbor = (av->driver->getVehicle()->getVehicleType() != VehicleBase::BUS) ? 0.0 : a[7];
		spacing = av->distance;
	}
	else
	{
		vld = params.desiredSpeed;
		spacing = params.distToStop;
	}

	if (bv->exists())
	{
		heavy_neighbor = (bv->driver->getVehicle()->getVehicleType() != VehicleBase::BUS) ? heavy_neighbor : a[7];
	}

	float left_most = 0.0;

	//Check if current lane is the left most lane
	if (params.currLaneIndex == 0)
	{
		left_most = 0.0;
	}
	else
	{
		left_most = a[2];
	}

	switch (n)
	{
	case 0:
	{
		mlc = 0;
		break;
	}
	case 1:
	{
		mlc = a[12] * pow(params.distToStop / 1000.0, a[17]) + a[15]; // why divide 1000
		break;
	}
	case 2:
	{
		mlc = a[13] * pow(params.distToStop / 1000.0, a[17]) + a[15] + a[16];
		break;
	}
	default:
	{
		mlc = (a[13] + a[14]*(n - 2)) * pow(params.distToStop / 1000, a[17]) + a[15] + a[16] * (n - 1);
		break;
	}
	}

	// MITSIM TS_LCModels.cc Dan: If vehicle ahead is a bus and there is a bus stop ahead
	// in the lane, set busAheadDummy to 1 for disincentive to be
	// applied in the utility.
	int busAheadDummy = 0;
	if (params.nvRightFwd.exists())
	{
		if (params.nvRightFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			busAheadDummy = 1;
		}
	}

	double u = a[0] + a[4] * vld + a[6] * spacing + a[8] * density + mlc + heavy_neighbor + left_most + a[5] * busAheadDummy;

	return exp(u);
}

double MITSIM_LC_Model::lcUtilityRight(DriverUpdateParams &params)
{
	// 1.0 lane utility parameters
	vector<double> a = laneUtilityParams;

	// 2.0 LEADING AND LAG VEHICLES
	const NearestVehicle * av = &params.nvRightFwd;
	const NearestVehicle * bv = &params.nvRightBack;

	// 3.0 count number of lane change to the end of link
	double vld, mlc, density, spacing;
	int n = abs(getNumberOfLCToEndOfLink(params, params.currLane));

	density = params.density;
	float heavy_neighbor = 0.0;

	if (av->exists())
	{
		vld = std::min<double>(params.desiredSpeed, av->driver->getFwdVelocity());
		heavy_neighbor = (av->driver->getVehicle()->getVehicleType() != VehicleBase::BUS) ? 0.0 : a[7];
		spacing = av->distance;
	}
	else
	{
		vld = params.desiredSpeed;
		spacing = params.distToStop;
	}

	if (bv->exists())
	{
		heavy_neighbor = (bv->driver->getVehicle()->getVehicleType() != VehicleBase::BUS) ? heavy_neighbor : a[7];
	}

	float left_most = 0.0;

	//Check if current lane is the left most lane
	if (params.currLaneIndex == 0)
	{
		left_most = 0.0;
	}
	else
	{
		left_most = a[2];
	}

	switch (n)
	{
	case 0:
	{
		mlc = 0;
		break;
	}
	case 1:
	{
		mlc = a[12] * pow(params.distToStop / 1000.0, a[17]) + a[15]; // why divide 1000
		break;
	}
	case 2:
	{
		mlc = a[13] * pow(params.distToStop / 1000.0, a[17]) + a[15] + a[16];
		break;
	}
	default:
	{
		mlc = (a[13] + a[14] * (n - 2)) * pow(params.distToStop / 1000, a[17]) + a[15] + a[16] * (n - 1);
		break;
	}
	}

	// MITSIM TS_LCModels.cc Dan: If vehicle ahead is a bus and there is a bus stop ahead
	// in the lane, set busAheadDummy to 1 for disincentive to be
	// applied in the utility.
	int busAheadDummy = 0;
	if (params.nvRightFwd.exists())
	{
		if (params.nvRightFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			busAheadDummy = 1;
		}
	}

	double u = a[1] + a[4] * vld + a[6] * spacing + a[8] * density + mlc + heavy_neighbor + left_most + a[5] * busAheadDummy;

	return exp(u);
}

double MITSIM_LC_Model::lcUtilityLeft(DriverUpdateParams &params)
{
	// 1.0 lane utility parameters
	vector<double> a = laneUtilityParams;

	// 2.0 LEADING AND LAG VEHICLES
	const NearestVehicle *leftFwdVeh = &params.nvLeftFwd;
	const NearestVehicle *leftRearVeh = &params.nvLeftBack;

	// 3.0 count number of lane change to the end of link
	double vld, mlc, density, spacing;
	int n = abs(getNumberOfLCToEndOfLink(params, params.currLane));

	density = params.density;
	float heavy_neighbor = 0.0;

	if (leftFwdVeh->exists())
	{
		spacing = leftFwdVeh->distance;
		vld = std::min<double>(params.desiredSpeed, leftFwdVeh->driver->getFwdVelocity());
		heavy_neighbor = (leftFwdVeh->driver->getVehicle()->getVehicleType() != VehicleBase::BUS) ? 0.0 : a[7];
	}
	else
	{
		vld = params.desiredSpeed;
		spacing = params.distToStop;

	}

	float left_most = 0.0;

	if (leftRearVeh->exists())
	{
		heavy_neighbor = (leftRearVeh->driver->getVehicle()->getVehicleType() != VehicleBase::BUS) ? heavy_neighbor : a[7];
	}

	switch (n)
	{
	case 0:
	{
		mlc = 0;
		break;
	}
	case 1:
	{
		mlc = a[12] * pow(params.distToStop / 1000.0, a[17]) + a[15]; // why divide 1000
		break;
	}
	case 2:
	{
		mlc = a[13] * pow(params.distToStop / 1000.0, a[17]) + a[15] + a[16];
		break;
	}
	default:
	{
		mlc = (a[13] + a[14] * (n - 2)) * pow(params.distToStop / 1000, a[17]) + a[15] + a[16] * (n - 1);
		break;
	}
	}

	int busAheadDummy = 0;
	if (params.nvLeftFwd.exists())
	{
		if (params.nvLeftFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			busAheadDummy = 1;
		}
	}

	double u = a[4] * vld + a[6] * spacing + a[8] * density + mlc + heavy_neighbor + left_most + a[5] * busAheadDummy;

	return exp(u);

}

double MITSIM_LC_Model::lcUtilityLookAheadLeft(DriverUpdateParams &params, int noOfChanges, float lcDistance)
{
	vector<double> a = laneUtilityParams;
	double vld, mlc, density, spacing;

	density = 0;
	float heavy_neighbor = 0.0;

	if (params.nvLeftFwd.exists())
	{
		double leftFwdVel = params.nvLeftFwd.driver->getFwdVelocity();
		double currentSpeed = params.perceivedFwdVelocity;
		vld = std::min<double>(leftFwdVel, currentSpeed);

		//Get vehicle type. Bus is the only heavy vehicle now
		if (params.nvLeftFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			heavy_neighbor = a[7];
		}
		spacing = params.nvLeftFwd.distance;
		density = params.nvLeftFwd.driver->getDensity();
	}
	else
	{
		vld = params.desiredSpeed;
		spacing = params.distToStop; // MITSIM distance()
	}

	if (params.nvLeftBack.exists())
	{
		//Get vehicle type. Bus is the only heavy vehicle now
		if (params.nvLeftBack.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			heavy_neighbor = a[7];
		}
		
		density = params.nvLeftBack.driver->getDensity();
	}

	float left_most = 0.0;

	switch (noOfChanges)
	{
	case 0:
	{
		mlc = 0;
		break;
	}
	case 1:
	{
		mlc = a[12] * pow(params.distToStop / 1000.0, a[17]) + a[15]; // why divide 1000
		break;
	}
	case 2:
	{
		mlc = a[13] * pow(params.distToStop / 1000.0, a[17]) + a[15] + a[16];
		break;
	}
	default:
	{
		mlc = (a[13] + a[14] * (noOfChanges - 2)) * pow(params.distToStop / 1000, a[17]) + a[15] + a[16] * (noOfChanges - 1);
		break;
	}
	}

	//MITSIM TS_LCModels.cc Dan: If vehicle ahead is a bus and there is a bus stop ahead
	//in the lane, set busAheadDummy to 1 for disincentive to be
	//applied in the utility.
	int busAheadDummy = 0;
	if (params.nvLeftFwd.exists())
	{
		if (params.nvLeftFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			busAheadDummy = 1;
		}
	}

	double u = a[4] * vld + a[6] * spacing + a[8] * density + mlc + heavy_neighbor + left_most + a[5] * busAheadDummy;
	double res = exp(u);
	return res;
}

double MITSIM_LC_Model::lcUtilityLookAheadRight(DriverUpdateParams &params, int noOfChanges, float lcDistance)
{
	vector<double> a = laneUtilityParams;
	double vld, mlc, density, spacing;

	density = 0.0;
	float heavy_neighbor = 0.0;

	if (params.nvRightFwd.exists())
	{
		double leftFwdVel = params.nvRightFwd.driver->getFwdVelocity();
		double currentSpeed = params.perceivedFwdVelocity;

		vld = std::min<double>(leftFwdVel, currentSpeed);

		//Get vehicle type. Bus is the only heavy vehicle now
		if (params.nvRightFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			heavy_neighbor = a[7];
		}
		spacing = params.nvRightFwd.distance;
		density = params.nvRightFwd.driver->getDensity();
	}
	else
	{
		vld = vld = params.desiredSpeed;
		spacing = params.distToStop; // MITSIM distance()
	}

	if (params.nvRightBack.exists())
	{
		//Get vehicle type. Bus is the only heavy vehicle now
		if (params.nvRightBack.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			heavy_neighbor = a[7];
		}
		density = params.nvRightBack.driver->getDensity();
	}

	float left_most = 0.0;

	//Check if the current lane is left most lane
	if (params.currLane->getLaneIndex() == 0)
	{
		left_most = 0.0;
	}
	else
	{
		left_most = a[2];
	}

	switch (noOfChanges)
	{
	case 0:
	{
		mlc = 0;
		break;
	}
	case 1:
	{
		mlc = a[12] * pow(params.distToStop / 1000.0, a[17]) + a[15]; // why divide 1000
		break;
	}
	case 2:
	{
		mlc = a[13] * pow(params.distToStop / 1000.0, a[17]) + a[15] + a[16];
		break;
	}
	default:
	{
		mlc = (a[13] + a[14] * (noOfChanges - 2)) * pow(params.distToStop / 1000, a[17]) + a[15] + a[16] * (noOfChanges - 1);
		break;
	}
	}

	// MITSIM TS_LCModels.cc Dan: If vehicle ahead is a bus and there is a bus stop ahead
	// in the lane, set busAheadDummy to 1 for disincentive to be
	// applied in the utility.
	int busAheadDummy = 0;
	if (params.nvRightFwd.exists())
	{
		if (params.nvRightFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			busAheadDummy = 1;
		}
	}

	double u = a[1] + a[4] * vld + a[6] * spacing + a[8] * density + mlc + heavy_neighbor + left_most + a[5] * busAheadDummy;
	double res = exp(u);
	return res;
}

double MITSIM_LC_Model::lcUtilityLookAheadCurrent(DriverUpdateParams &params, int noOfChanges, float lcDistance)
{
	vector<double> a = laneUtilityParams;
	double vld, mlc, density, spacing;

	density = params.density;
	float heavy_neighbor = 0.0;

	if (params.nvFwd.exists())
	{
		double frontVhVel = params.perceivedFwdVelocityOfFwdCar;
		double currentSpeed = params.perceivedFwdVelocity;

		vld = std::min<double>(frontVhVel, currentSpeed);

		//Get vehicle type. Bus is the only heavy vehicle now
		if (params.nvFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			heavy_neighbor = a[7];
		}
		spacing = params.perceivedDistToFwdCar;
	}
	else
	{
		vld = params.desiredSpeed;
		spacing = params.distToStop; // MITSIM distance()
	}

	if (params.nvRightBack.exists())
	{
		//Get vehicle type. Bus is the only heavy vehicle now
		if (params.nvRightBack.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			heavy_neighbor = a[7];
		}
	}

	float left_most = 0.0;

	//Check if the current lane is left most lane
	if (params.currLane->getLaneIndex() == 0)
	{
		left_most = 0.0;
	}
	else
	{
		left_most = a[2];
	}

	switch (noOfChanges)
	{
	case 0:
	{
		mlc = 0;
		break;
	}
	case 1:
	{
		mlc = a[12] * pow(params.distToStop / 1000.0, a[17]) + a[15]; // why divide 1000
		break;
	}
	case 2:
	{
		mlc = a[13] * pow(params.distToStop / 1000.0, a[17]) + a[15] + a[16];
		break;
	}
	default:
	{
		mlc = (a[13] + a[14] * (noOfChanges - 2)) * pow(params.distToStop / 1000, a[17]) + a[15] + a[16] * (noOfChanges - 1);
		break;
	}
	}

	float tailgate_dummy = 0;

	if (params.nvBack.exists())
	{
		double gap_behind = params.nvBack.distance;
		tailgate_dummy = (gap_behind <= a[10] && density <= a[11]) ? a[9] : 0;
	}

	// MITSIM TS_LCModels.cc Dan: If vehicle ahead is a bus and there is a bus stop ahead
	// in the lane, set busAheadDummy to 1 for disincentive to be
	// applied in the utility.
	int busAheadDummy = 0;
	if (params.nvRightFwd.exists())
	{
		if (params.nvRightFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			busAheadDummy = 1;
		}
	}

	double u = a[0] + a[4] * vld + a[6] * spacing + a[8] * density + mlc + heavy_neighbor + left_most + tailgate_dummy + a[5] * busAheadDummy;

	double res = exp(u);
	return res;
}

double MITSIM_LC_Model::lcCriticalGap(DriverUpdateParams &params, int type, double dv)
{
	vector<double> a = criticalGapParams;

	float dvNegative = (dv < 0) ? dv : 0.0;
	float dvPositive = (dv > 0) ? dv : 0.0;

	float gap = 0.0;

	float maxdiff = a[8];

	switch (type)
	{
	case 0:
	{
		// lead gap
		gap = a[0] + a[1] * dvNegative + a[2] * dvPositive + Utils::generateFloat(0, a[3]);
		break;
	}
	case 1:
	{
		// lag gap
		gap = a[4] + a[5] * dvNegative + a[6] * std::min<double>(dvPositive, maxdiff) + Utils::generateFloat(0, a[7]);
		break;
	}
	}

	float cri_gap = exp(gap);

	return cri_gap;
}

double MITSIM_LC_Model::mlcDistance()
{
	double n = Utils::generateFloat(0, 1.0);
	double dis = MLC_PARAMETERS.lowbound + n * (MLC_PARAMETERS.delta - MLC_PARAMETERS.lowbound);
	return dis;
}

LaneChangeTo MITSIM_LC_Model::makeLaneChangingDecision(DriverUpdateParams &params)
{
	static const std::string makeLCD("makeLaneChangingDecision");
	params.lcDebugStr << "makeD" << params.now.frame();
	params.noOfLC = 0;
	
	if(params.desiredSpeed < minSpeed)
	{
		params.lcDebugStr << ";samesm";
		return LANE_CHANGE_TO_NONE;
	}
	
	//Reset status
	params.setStatus(STATUS_LEFT_SIDE_OK, STATUS_UNKNOWN, makeLCD);
	params.setStatus(STATUS_RIGHT_SIDE_OK, STATUS_UNKNOWN, makeLCD);
	params.setStatus(STATUS_CURRENT_LANE_OK, STATUS_UNKNOWN, makeLCD);
	
	//Reset utilities
	params.utilityCurrent = 0;
	params.utilityLeft = 0;
	params.utilityRight = 0;

	//Update the lane connectivity status
	setLaneConnectionStatus(params);

	LaneChangeTo change = LANE_CHANGE_TO_NONE; // direction to change	

	if (checkForEventsAhead(params))
	{
		params.lcDebugStr << ";hevent";
		change = checkMandatoryEventLC(params);
	}
	else
	{
		params.lcDebugStr << ";noEvent";
		change = checkForLC_WithLookAhead(params);
	}

	if (change == LANE_CHANGE_TO_LEFT)
	{
		//To left
		params.unsetStatus(STATUS_CHANGING);
		params.setStatus(STATUS_LEFT);
	}
	else if (change == LANE_CHANGE_TO_RIGHT)
	{
		//To right
		params.unsetStatus(STATUS_CHANGING);
		params.setStatus(STATUS_RIGHT);
	}
	else
	{
		params.unsetStatus(STATUS_CHANGING);
	}

	return change;
}

double MITSIM_LC_Model::executeLaneChanging(DriverUpdateParams &params)
{
	params.lcDebugStr << ";,,,ELC";

	// 1. unset FLAG_LC_FAILED, check whether we can do a lane change
	params.unsetFlag(FLAG_LC_FAILED);

	LaneChangeTo changeMode = LANE_CHANGE_TO_NONE;

	// 2.0 check decision
	if (params.getStatus(STATUS_LEFT))
	{
		params.lcDebugStr << ";ELC-L";
		changeMode = LANE_CHANGE_TO_LEFT;
	}
	else if (params.getStatus(STATUS_RIGHT))
	{
		params.lcDebugStr << ";ELC-R";
		changeMode = LANE_CHANGE_TO_RIGHT;
	}
	else
	{
		params.lcDebugStr << ";ELC-S";
		return 0.0; // No request for lane change
	}

	// 3.0 get lead, lag vehicles
	const NearestVehicle *fwdVeh;
	const NearestVehicle *rearVeh;

	if (changeMode == LANE_CHANGE_TO_LEFT)
	{
		fwdVeh = &params.nvLeftFwd;
		rearVeh = &params.nvLeftBack;
	}
	else
	{
		fwdVeh = &params.nvRightFwd;
		rearVeh = &params.nvRightBack;
	}

	// 4.0 get lead, lag vehicle distance
	// LEADING HEADWAY
	float aheadway; // leading headway
	if (fwdVeh->exists())
	{
		aheadway = fwdVeh->distance;
		params.lcDebugStr << ";ah" << aheadway;
	}
	else
	{
		aheadway = Math::FLT_INF;
	}

	// LAG HEADWAY
	float bheadway; // lag headway
	if (rearVeh->exists())
	{
		bheadway = rearVeh->distance;
		params.lcDebugStr << ";bh" << bheadway;
	}
	else
	{
		bheadway = Math::FLT_INF;
	}

	// 5.0 check if lane change decision triggered due to special events
	int escape = params.flag(FLAG_ESCAPE);

	int lctype;
	if (params.getStatus(STATUS_MANDATORY))
	{
		params.lcDebugStr << ";M";
		lctype = 2; // must do lane changing
	}
	else if (escape)
	{
		params.lcDebugStr << ";e";
		lctype = 2; // special event
	}
	else
	{
		params.lcDebugStr << ";0";
		lctype = 0; // discretionary
	}
	
	double cGapRear = -1, cGapFwd = -1;

	// 6.0 check if lead, lag gap is OK
	if (rearVeh->exists())
	{
		cGapRear = lcCriticalGap(params, 1, rearVeh->driver->getFwdVelocity() - params.currSpeed);
		
		if(bheadway < cGapRear)
		{
			params.lcDebugStr << ";FLG" << "cgapR" << cGapRear;
			params.setFlag(FLAG_LC_FAILED_LAG); // lag gap
		}
	}
	if (fwdVeh->exists())
	{
		cGapFwd = lcCriticalGap(params, 0, fwdVeh->driver->getFwdVelocity() - params.currSpeed);
		
		if(aheadway < cGapFwd)
		{
			params.lcDebugStr << ";FLD" << "cgapF" << cGapFwd;
			params.setFlag(FLAG_LC_FAILED_LEAD); // lead gap
		}
	}

	// 7.0 if gap is OK, then set status as doing lane change
	if (!params.flag(FLAG_LC_FAILED))
	{
		params.lcDebugStr << ";SLC";
		params.setStatusDoingLC(changeMode);
	}

	// 8.0 CHECK IF THE GAPS ARE not ACCEPTABLE,then do nosing
	if (params.flag(FLAG_LC_FAILED))
	{
		// The gaps are not acceptable and this guy is in mandatory
		// state.  Check if this vehicle is nosing or becomes aggressive
		// and decide to nose in.

		int nosing = params.flag(FLAG_NOSING);
		params.lcDebugStr << ";LCF" << nosing << ";ds" << params.distToStop;
		if (!nosing && params.distToStop < lcMaxNosingDis)
		{
			params.lcDebugStr << ";NDis";
			int nlanes = params.noOfLC;
			float gap = aheadway + bheadway;
			float dv = fwdVeh->exists() ? fwdVeh->driver->getFwdVelocity() - params.currSpeed : 0;

			float pmf;

			if (params.flag(FLAG_ESCAPE) == 0 && fwdDriverMovement->getNextSegInNextLink())
			{
				// current lane connects to next link on path
				params.lcDebugStr << ";CN";
				float longer_dis = fwdDriverMovement->getDistToEndOfCurrLink() + fwdDriverMovement->getNextSegInNextLink()->getLength();
				pmf = lcNosingProbability(longer_dis, dv, gap, nlanes);
				// remaining distance for forced merging is the length remaining
				// in the current link plus at least the length of the first
				// segment of the next link
			}
			else
			{
				// current lane does not connect to next link on path
				params.lcDebugStr << ";else";
				pmf = lcNosingProbability(fwdDriverMovement->getDistToEndOfCurrWayPt(), dv, gap, nlanes);
				// forced merging must be performed in current link
			}

			nosing = Utils::brandom(pmf);
		}

		params.unsetFlag(FLAG_NOSING); // reset the flag

		if (nosing)
		{
			params.lcDebugStr << ";nig";
			
			// Since I am nosing, updating of acceleration rate sooner
			params.reactionTimeCounter = CF_CRITICAL_TIMER_RATIO * params.nextStepSize;
			
			// Now I am going to nose in provided it is feasible and the
			// lag vehicle is willing to yield
			
			int isNosingFeasible = checkNosingFeasibility(params, fwdVeh, rearVeh, params.distToStop);
			params.lcDebugStr << ";fi" << isNosingFeasible;
			
			if (isNosingFeasible)
			{				
				params.lcDebugStr << ";nf";
				params.setFlag(FLAG_NOSING_FEASIBLE);

				// Nosing is feasible
				if (rearVeh->exists() && params.flag(FLAG_LC_FAILED_LAG) && rearVeh->distance > 0)
				{					
					// There is a lag vehicle in the target lane with which we don't have enough gap
					//Also, make sure that the lag vehicle isn't stuck (yielding to it would get us stuck as well,
					//the stuck vehicle will change lanes once we go through)
					Driver *bvd = const_cast<Driver *> (rearVeh->driver);
					DriverUpdateParams &bvp = bvd->getParams();
				
					bvd->setYieldingToDriver(params.driver);
					if (!(bvd->IsBusDriver() && bvp.getStatus(STATUS_STOPPED)))
					{
						bvp.reactionTimeCounter = std::min<double>(params.reactionTimeCounter, bvp.reactionTimeCounter);
					}

					if (!bvp.flag(FLAG_YIELDING))
					{
						bvp.yieldTime = params.now;
					}
					if (params.getStatus(STATUS_LEFT))
					{
						params.setFlag(FLAG_NOSING_LEFT);
						bvp.setFlag(FLAG_YIELDING_RIGHT);
					}
					else
					{
						params.setFlag(FLAG_NOSING_RIGHT);
						bvp.setFlag(FLAG_YIELDING_LEFT);
					}
				}
				else
				{
					// No lag vehicle in the target lane
					if (params.getStatus(STATUS_LEFT))
					{
						params.setFlag(FLAG_NOSING_LEFT);
					}
					else
					{
						params.setFlag(FLAG_NOSING_RIGHT);
					}
				}
				
				// Check if the minimum gaps are available.				
				if (abs(bheadway) > cGapRear && aheadway > cGapFwd)
				{
					params.setStatusDoingLC(changeMode);
				}				
			}
			else
			{
				params.lcDebugStr << ";NTFea";
				params.unsetFlag(FLAG_NOSING_FEASIBLE);

				// Nosing is not feasible, but maintain the nosing state
				if (params.getStatus(STATUS_LEFT))
				{
					params.setFlag(FLAG_NOSING_LEFT);
				}
				else
				{
					params.setFlag(FLAG_NOSING_RIGHT);
				}
			}
		}
		return 0.0;
	}
}

int MITSIM_LC_Model::checkNosingFeasibility(DriverUpdateParams &params, const NearestVehicle *fwdVehicle, const NearestVehicle *rearVehicle, double distToStop)
{
	params.lcDebugStr << "^^^CKFIS";
	if (params.flag(FLAG_STUCK_AT_END))
	{
		params.lcDebugStr << ";stuck";
		if (timeSinceTagged(params) > lcMaxStuckTime)
		{
			//If stuck for a very long time, skip the feasibility check
			params.lcDebugStr << ";max";
			return 1;
		}
	}
	else
	{
		params.lcDebugStr << ";CF0";
		double length = params.driver->getVehicleLength();

		if (distToStop <= length && params.currSpeed < Math::DOUBLE_EPSILON)
		{
			params.lcDebugStr << ";CF1";
			params.setFlag(FLAG_STUCK_AT_END);
		}
	}

	// Constraints of acceleration rate in response to lead and lag
	// vehicles
	float lower = -Math::FLT_INF;
	float upper = Math::FLT_INF;

	params.lcDebugStr << ";CF2";
	if (fwdVehicle->exists())
	{
		params.lcDebugStr << ";CF3";
		Driver *avDriver = const_cast<Driver*> (fwdVehicle->driver);
		DriverUpdateParams& avp = avDriver->getParams();

		if ((avp.flag(FLAG_NOSING) || avp.flag(FLAG_YIELDING)) && fwdVehicle->distance < 2.0 * params.lcMinGap(2))
		{
			params.lcDebugStr << ";CF4";
			//The lead vehicle is yielding or nosing
			return 0; // To avoid dead lock
		}
		else if (params.flag(FLAG_LC_FAILED_LEAD))
		{
			params.lcDebugStr << ";CF5";

			// Acceleration rate in order to be slower than the leader
			upper = (fwdVehicle->driver->getFwdVelocity() - params.currSpeed) / lcNosingConstStateTime + fwdVehicle->driver->getFwdAcceleration();
			params.lcDebugStr << ";up" << upper;

			if (upper < params.maxDeceleration)
			{
				params.lcDebugStr << ";CF6";
				return 0; // This vehicle is too fast
			}
		}
		else if (fwdVehicle->driver->getFwdVelocity() < Math::DOUBLE_EPSILON &&
				params.distToStop > params.distanceToNormalStop && params.nvFwd.distance > params.distanceToNormalStop)
		{
			params.lcDebugStr << ";CF7";
			return 0;
		}
	}

	if (rearVehicle->exists())
	{
		params.lcDebugStr << ";CF8";
		Driver *bvDriver = const_cast<Driver*> (rearVehicle->driver);
		DriverUpdateParams& bvp = bvDriver->getParams();

		if (params.driver->getVehicle()->getVehicleType() == VehicleBase::BUS && params.flag(FLAG_NOSING))
		{

			params.lcDebugStr << ";CF9";

			// MITSIM: Dan: bus must force it's way along the route, and
			// other vehicles will generally yield;
			// Acceleration rate in order to be faster than the lag
			// vehicle and do not cause the lag vehicle to decelerate
			// harder than its normal deceleration rate
			lower = (rearVehicle->driver->getFwdVelocity() - params.currSpeed) / lcNosingConstStateTime + params.normalDeceleration;

			if (lower > params.maxAcceleration)
			{
				// I am willing to accelerate hard
				// This vehicle is too slow or close to the lag vehicle
				params.lcDebugStr << ";CF10" << ";lower" << lower;
				return 0;
			}

		}
		else if (bvp.flag(FLAG_NOSING) || (bvp.flag(FLAG_YIELDING) &&
				rearVehicle->driver->getYieldingToDriver() != params.driver &&
				abs(rearVehicle->distance) < 2.0 * params.lcMinGap(3)))
		{
			params.lcDebugStr << ";CF11";

			// The lag vehicle is nosing or yielding to another vehicle or
			// not willing to yield
			return 0; // To avoid dead lock
		}
		else if (params.flag(FLAG_LC_FAILED_LAG))
		{
			params.lcDebugStr << ";CF13";

			// Acceleration rate in order to be faster than the lag
			// vehicle and do not cause the lag vehicle to decelerate
			// harder than its normal deceleration rate
			lower = (rearVehicle->driver->getFwdVelocity() - params.currSpeed) / lcNosingConstStateTime + params.normalDeceleration;
			params.lcDebugStr << ";low" << lower;

			if (lower > params.maxAcceleration)
			{
				// I am willing to accelerate hard
				// This vehicle is too slow or close to the lag vehicle
				params.lcDebugStr << ";CF14";
				return 0;
			}
		}
	}

	// Check if there exists a feasible acceleration rate for this
	// vehicle
	params.lcDebugStr << ";CF15";
	params.lcDebugStr << ";low" << lower;
	params.lcDebugStr << ";upper" << upper;

	if (lower > upper)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

float MITSIM_LC_Model::lcNosingProbability(float distance, float diffInSpeed, float gap, int num)
{
	if (num < 0)
	{
		num = -num;
	}
	else if (num == 0)
	{
		num = 1;
	}

	std::vector<double> b = kaziNosingParams;
	float rel_spd = (diffInSpeed > 0) ? 0 : diffInSpeed;
	float rm_dist_impact = 10 - 10 / (1.0 + exp(b[2] * distance));

	if (gap > 100.0)
	{
		// 100 meters
		gap = 100.0;
	}

	float u = b[0] + b[1] * rel_spd + b[3] * rm_dist_impact + b[4] * gap + b[5] * (num - 1);
	float p = 1.0 / (1 + exp(-u));

	return p;
}

double MITSIM_LC_Model::calculateLateralVelocity(LaneChangeTo &change)
{
	if (change != LANE_CHANGE_TO_NONE)
	{
		const int lane_shift_velocity = 5.50;
		return change == LANE_CHANGE_TO_LEFT ? lane_shift_velocity : -lane_shift_velocity;
	}
	return 0.0;
}

double MITSIM_LC_Model::timeSinceTagged(DriverUpdateParams &params)
{
	double currentTime = params.now.ms();
	double t = (currentTime - params.laneChangeTime) / 1000.0; //convert ms to s
	return t;
}

int MITSIM_LC_Model::checkForEventsAhead(DriverUpdateParams& params)
{
	//Check events, like incidents

	params.unsetFlag(FLAG_ESCAPE | FLAG_AVOID);
	params.unsetStatus(STATUS_MANDATORY);

	// set default target lanes
	params.targetLanes.clear();

	bool needMLC = false;
	bool needDLC = false;

	// 1.0 check incident
	int res = isIncidentAhead(params);
	
	if (res == -1)
	{
		needMLC = true;
		params.lcDebugStr << ";inc";
	}
	if (res == 1)
	{
		needDLC = true;
	}
	
	set<const Lane*> connectedLanes;	
	
	// 1.1 check connections to the next way-point
	//No incident, but check for lane connectivity	
	res = isLaneConnectedToNextWayPt(params, connectedLanes);

	if (res == -1)
	{
		needMLC = true;
		params.lcDebugStr << ";Xcnwp";
	}
	if (res == 1)
	{
		needDLC = true;
	}

	params.targetLanes = connectedLanes;
	connectedLanes.clear();

	// 1.2 check connections to the next link
	//This is equivalent to the previous check only when we're on the last segment of the link		
	res = isLaneConnectedToNextLink(params, connectedLanes);

	if (res == -1)
	{
		needMLC = true;
		params.lcDebugStr << ";Xcnl" << fwdDriverMovement->getNextLink()->getLinkId();
	}
	if (res == 1)
	{
		needDLC = true;
	}

	params.addTargetLanes(connectedLanes);	
	connectedLanes.clear();
	
	// 1.3 Check connections in a lookahead distance
	vector<const Lane *> targetLanes;
	res = getConnectedLanesInLookAheadDistance(params, lookAheadDistance, targetLanes);	

	if(!targetLanes.empty())
	{
		if (res == -1)
		{
			needMLC = true;
			params.lcDebugStr << ";Xclad";
		}
		if (res == 1)
		{
			needDLC = true;
		}
		
		connectedLanes.insert(targetLanes.begin(), targetLanes.end());
		params.addTargetLanes(connectedLanes);
		connectedLanes.clear();
	}

	// 1.4 check stop point, like bus stop	
	res = isLaneConnectedToStopPoint(params, connectedLanes);
	
	if (res == -1)
	{
		needMLC = true;
		params.lcDebugStr << ";Xcsp";
	}
	if (res == 1)
	{
		needDLC = true;
	}
	
	if (connectedLanes.size() > 0)
	{
		//stop point lane maybe not in targetLanes, so just assign to targetLanes
		params.targetLanes = connectedLanes;
	}
	if (params.stopPointState == DriverUpdateParams::STOP_POINT_FOUND || params.stopPointState == DriverUpdateParams::ARRIVING_AT_STOP_POINT ||
			params.stopPointState == DriverUpdateParams::ARRIVED_AT_STOP_POINT || params.stopPointState == DriverUpdateParams::WAITING_AT_STOP_POINT)
	{
		if (res == 0)
		{
			//means already in most left lane
			needMLC = false;
		}
	}

	// 2.0 set flag
	if (needMLC)
	{
		params.setFlag(FLAG_ESCAPE);
	}
	else if (needDLC)
	{
		params.setFlag(FLAG_AVOID);
	}

	// 3.0 MLC is required and not enough headway, set STATUS_MANDATORY
	if (needMLC && params.distToStop < lookAheadDistance && timeSinceTagged(params) >= MLC_PARAMETERS.lane_mintime)
	{
		params.setStatus(STATUS_MANDATORY);
	}

	return params.getStatus(STATUS_MANDATORY);
}

int MITSIM_LC_Model::isIncidentAhead(DriverUpdateParams &params)
{
	// only mandatory lane change or no change
	DriverMovement *driverMvt = dynamic_cast<DriverMovement*> (params.driver->Movement());
	driverMvt->incidentPerformer.checkIncidentStatus(params, params.driver->getParams().now);

	if (driverMvt->incidentPerformer.getIncidentStatus().getChangedLane())
	{
		params.distToStop = driverMvt->incidentPerformer.getIncidentStatus().getDistanceToIncident() - params.driver->getVehicleLength();
		return -1; //mandatory lane change
	}

	return 0;
}

int MITSIM_LC_Model::isLaneConnectedToNextWayPt(DriverUpdateParams &params, set<const Lane *> &targetLanes)
{
	std::string str = "isLaneConnectedToNextWayPt";
	const Lane *currLane = fwdDriverMovement->getCurrLane();

	if (fwdDriverMovement->getNextSegment())
	{
		//Fill targetLanes - select only lanes that are connected to the next segment
		std::vector<Lane *>::const_iterator itLanes = currLane->getParentSegment()->getLanes().begin();
		while (itLanes != currLane->getParentSegment()->getLanes().end())
		{
			//Check the physical lane connectors
			std::vector<const LaneConnector *> connectors;
			(*itLanes)->getPhysicalConnectors(connectors);
			
			//If at least one lane connector is physically connected to the next segment, 
			//add the lane to the target lanes
			if(!connectors.empty())
			{
				targetLanes.insert(*itLanes);
			}			
			++itLanes;
		}

		//Check if we have a connector to the next segment		
		std::vector<const LaneConnector *> connectors;
		currLane->getPhysicalConnectors(connectors);
		
		if (connectors.empty())
		{
			//No lane connector for this lane, so we need to change lane
			double distToStop = fwdDriverMovement->getDistToEndOfCurrWayPt() - params.driver->getVehicleLength();
			
			if (distToStop < params.distToStop)
			{
				params.distToStop = distToStop;
			}

			return -1;
		}
	}
	else
	{
		//Last segment on the link

		//Fill targetLanes - select only lanes that are connected to the next turning
		const WayPoint *nextWayPt = fwdDriverMovement->getNextWayPoint();

		if (nextWayPt)
		{
			const TurningGroup *turningGroup = nextWayPt->turningGroup;
			std::map<unsigned int, std::map<unsigned int, TurningPath *> >::const_iterator itTurnings = turningGroup->getTurningPaths().begin();

			//Iterate through the turnings, and add the from lanes of the turnings
			while (itTurnings != turningGroup->getTurningPaths().end())
			{
				//Get the pointer to the from lane from the road network
				const RoadNetwork *network = RoadNetwork::getInstance();
				const Lane *fromLane = network->getById(network->getMapOfIdVsLanes(), itTurnings->first);
				
				targetLanes.insert(fromLane);
				++itTurnings;
			}

			if (!turningGroup->getTurningPaths(currLane->getLaneId()))
			{
				//No turning path from current lane
				double distToStop = fwdDriverMovement->getDistToEndOfCurrLink() - params.driver->getVehicleLength();
				
				if (distToStop < params.distToStop)
				{
					params.distToStop = distToStop;
				}
				
				return -1;
			}
		}
	}

	return 0;
}

int MITSIM_LC_Model::isLaneConnectedToNextLink(DriverUpdateParams &params, set<const Lane *> &targetLanes)
{
	int result = 0;
	bool isLaneConnected = false;
	
	const RoadSegment *currSeg = params.currLane->getParentSegment();
	const Link *currLink = currSeg->getParentLink();
	const Link *nextLink = fwdDriverMovement->getNextLink();
	
	//Check if next link exists (This may be last link in the path)
	if(nextLink)
	{
		//The turning group connecting the current and next links.
		const TurningGroup *turningGroup = currLink->getToNode()->getTurningGroup(currLink->getLinkId(), nextLink->getLinkId());

		//The turning paths belonging to the turning group
		const std::map<unsigned int, std::map<unsigned int, TurningPath *> > &turningPaths = turningGroup->getTurningPaths();

		std::map<unsigned int, std::map<unsigned int, TurningPath *> >::const_iterator itPaths = turningPaths.begin();

		//Check the connectivity to the "from lane" of each of the turning paths
		while (itPaths != turningPaths.end())
		{
			//The lane index of the "from lane" of the turning path
			unsigned int connectedLaneIdx = itPaths->first % 10;

			//Make sure the current segment has a lane with the same index
			if (connectedLaneIdx <= currSeg->getNoOfLanes() - 1)
			{
				targetLanes.insert(currSeg->getLane(connectedLaneIdx));
			}

			if (connectedLaneIdx == params.currLaneIndex)
			{
				isLaneConnected = true;
			}

			++itPaths;
		}
	}
	else
	{
		//Last link in the path, any lane is OK
		isLaneConnected = true;
		targetLanes.insert(currSeg->getLanes().begin(), currSeg->getLanes().end());
	}
	
	if(!isLaneConnected)
	{
		double distToStop = fwdDriverMovement->getDistToEndOfCurrLink() - params.driver->getVehicleLength();

		if (distToStop < params.distToStop)
		{
			params.distToStop = distToStop;
		}

		result = -1;
	}
	
	return result;
}

int MITSIM_LC_Model::isLaneConnectedToStopPoint(DriverUpdateParams &params, set<const Lane *> &targetLanes)
{
	int result = 0;

	// 1.0 find nearest forward stop point

	//Get distance to stop point of current link
	double distance = params.distanceToStoppingPt;

	if ((distance > -10 && params.stopPointState != DriverUpdateParams::STOP_POINT_NOT_FOUND)
			|| params.stopPointState == DriverUpdateParams::ARRIVED_AT_STOP_POINT)
	{
		//In case car stop just bit ahead of the stop point
		if (params.stopPointState == DriverUpdateParams::LEAVING_STOP_POINT)
		{
			return result;
		}

		//Only most left lane is target lane
		if (params.currLaneIndex != 0)
		{
			const Lane *lane = fwdDriverMovement->getCurrSegment()->getLane(0);
			targetLanes.insert(lane);
			result = -1;
		}

		return result;
	}

	return result;
}

int MITSIM_LC_Model::getConnectedLanesInLookAheadDistance(DriverUpdateParams &params, double lookAheadDist, std::vector<const Lane *> &lanePool)
{
	int res = -1;
	double scannedDist = 0;
	std::vector<WayPoint> wayPtsInLookAheadDist;
			
	std::vector<WayPoint>::const_iterator itWayPts = fwdDriverMovement->getCurrWayPointIt();
	std::vector<WayPoint>::const_iterator end = fwdDriverMovement->getDrivingPath().end();
	
	//Add the remaining part of the current way-point to the scanned distance
	scannedDist = fwdDriverMovement->getDistToEndOfCurrWayPt();
	wayPtsInLookAheadDist.push_back(*itWayPts);
	++itWayPts;
	
	//Scan till look ahead distance or end or path, whichever is smaller and make a list of the way-points
	while(scannedDist < lookAheadDist && itWayPts != end)
	{
		wayPtsInLookAheadDist.push_back(*itWayPts);
		
		if(itWayPts->type == WayPoint::ROAD_SEGMENT)
		{			
			scannedDist += itWayPts->roadSegment->getLength();
		}
		else
		{
			scannedDist += itWayPts->turningGroup->getLength();
		}
		
		++itWayPts;
	}
	
	//Lanes in the current segment
	const std::vector<Lane *> &lanes = fwdDriverMovement->getCurrSegment()->getLanes();	
	end = wayPtsInLookAheadDist.end();
	
	//Check the connectivity of each of the lanes to the way points in the look ahead distance
	for(std::vector<Lane *>::const_iterator itLanes = lanes.begin(); itLanes != lanes.end(); ++itLanes)
	{
		//The way points in the look ahead distance
		itWayPts = wayPtsInLookAheadDist.begin() + 1;
		double distance = 0;
		
		if(isLaneConnected(*itLanes, itWayPts, end, &distance))
		{
			params.lcDebugStr << ";cl" << (*itLanes)->getLaneId();
			lanePool.push_back(*itLanes);

			if (params.currLane == *itLanes)
			{
				res = 0;
			}
		}
		else
		{
			if(params.distToStop > distance)
			{
				params.distToStop = distance;
			}
		}
	}
	
	return res;
}

bool MITSIM_LC_Model::isLaneConnected(const Lane* lane, vector<WayPoint>::const_iterator itWayPts, vector<WayPoint>::const_iterator end, double *distance)
{
	if(itWayPts == end)
	{
		return true;
	}
	else
	{
		if (itWayPts->type == WayPoint::ROAD_SEGMENT)
		{
			//Update the distance
			(*distance) += lane->getLength();
			
			//The next way point is a road segment. If we have a lane connector, then we're connected to it

			std::vector<const LaneConnector *> connectors;
			lane->getPhysicalConnectors(connectors);

			if (!connectors.empty())
			{
				bool isConnected = false;
				
				for(vector<const LaneConnector *>::const_iterator itConnectors = connectors.begin(); itConnectors != connectors.end(); ++itConnectors)
				{
					isConnected = isConnected || isLaneConnected((*itConnectors)->getToLane(), (itWayPts + 1), end, distance);
				}
				
				return isConnected;
			}
			else
			{
				//We're not connected to the next segment
				return false;
			}
		}
		else
		{
			//Update the distance
			(*distance) += lane->getLength();
			
			//The next way point is a turning group. If we have a turning path, then we're connected to it

			const std::map<unsigned int, TurningPath *> *turnings = itWayPts->turningGroup->getTurningPaths(lane->getLaneId());

			if (turnings)
			{
				if ((itWayPts + 1) == end)
				{
					return true;
				}
				
				bool isConnected = false;
				
				for(map<unsigned int, TurningPath *>::const_iterator itTurnings = turnings->begin(); itTurnings != turnings->end(); ++itTurnings)
				{									
					//Additional increment for itWayPts as we are going from lane->turning path->lane directly
					isConnected = isConnected || isLaneConnected(itTurnings->second->getToLane() , (itWayPts + 2), end, distance);
				}

				return isConnected;
			}
			else
			{
				//We're not connected to the next link
				return false;
			}
		}
	}
}

LaneChangeTo MITSIM_LC_Model::checkMandatoryEventLC(DriverUpdateParams &params)
{
	LaneChangeTo lcs = LANE_CHANGE_TO_NONE;
	DriverMovement *driverMvt = dynamic_cast<DriverMovement*> (params.driver->Movement());

	params.lcDebugStr << ";checkMLC";

	// 1.0 Check for incidents
	if (driverMvt->incidentPerformer.getIncidentStatus().getChangedLane())
	{
		lcs = driverMvt->incidentPerformer.getIncidentStatus().getLaneSide();

		if (lcs == LANE_CHANGE_TO_LEFT)
		{
			params.setFlag(FLAG_ESCAPE_LEFT);
		}
		else if (lcs == LANE_CHANGE_TO_RIGHT)
		{
			params.setFlag(FLAG_ESCAPE_RIGHT);
		}

		params.lcDebugStr << ";icd";

		return lcs;
	}

	// 2.0 if FLAG_ESCAPE (need to do a MLC) and current lane is not OK, count the number of left/right lane change required to drive on the correct lane
	//No need check STATUS_CURRENT_OK, as FLAG_ESCAPE indicates current lane is not OK
	if (params.flag(FLAG_ESCAPE))
	{
		//2.1 Find the current lane index
		size_t currentLaneIdx = params.currLaneIndex;

		//Number of lane changes to an open lane
		int nl = -1; // left side
		int nr = -1; // right side

		set<const Lane *>::iterator it;
		for (it = params.targetLanes.begin(); it != params.targetLanes.end(); ++it)
		{
			//2.2 lane index
			const Lane *lane = *it;
			size_t targetLaneIdx = lane->getLaneIndex();
			if (targetLaneIdx < currentLaneIdx)
			{
				//Target lane is on the left of current lane
				int numberlc = currentLaneIdx - targetLaneIdx;
				if (numberlc < nl || nl < 0)
				{
					nl = numberlc;
				}
			}
			else if (targetLaneIdx > currentLaneIdx)
			{
				//Target lane on the right
				int numberlc = targetLaneIdx - currentLaneIdx;
				if (numberlc < nr || nr < 0)
				{
					nr = numberlc;
				}
			}
			else
			{
				//Target lane == current lane Should not happen
				WarnOut("Mandatory Lane change required, but Target lane == Current lane!");
			}
		}

		// 2.3 set FLAG_ESCAPE_LEFT or FLAG_ESCAPE_RIGHT
		// 2.3.1 There is an open left lane and no open right lane or the
		//       open left lane is closer.
		if (nl > 0 && nr == -1)
		{
			if (params.flag(FLAG_ESCAPE))
			{
				params.setFlag(FLAG_ESCAPE_LEFT);
			}
			else if (params.flag(FLAG_AVOID))
			{
				params.setFlag(FLAG_AVOID_LEFT);
			}

			params.noOfLC = nl;
			params.lcDebugStr << ";LEFT";

			return LANE_CHANGE_TO_LEFT;
		}
		// 2.3.2 There is an open right lane and no open left lane or the
		//       open right lane is closer.
		if (nr > 0 && nl == -1)
		{
			if (params.flag(FLAG_ESCAPE))
			{
				params.setFlag(FLAG_ESCAPE_RIGHT);
			}
			else if (params.flag(FLAG_AVOID))
			{
				params.setFlag(FLAG_AVOID_RIGHT);
			}

			params.noOfLC = nr;
			params.lcDebugStr << ";RIGHT";

			return LANE_CHANGE_TO_RIGHT;
		}		
	}

	/*-------We should not be doing discretionary lane change when we have mandatory event------*/
	/*// 3.0 discretionary lane change
	double eul = 0.0, eur = 0.0, euc = 1.0;
	if (isReadyForNextDLC(params, 2))
	{
		eul = lcUtilityLeft(params);
		params.lcDebugStr << ";eul" << eul;
	}

	if (isReadyForNextDLC(params, 1))
	{
		eur = lcUtilityRight(params);
		params.lcDebugStr << ";eur" << eur;
	}

	// 4.0 choose
	double sum = eul + eur;

	if (sum > 0)
	{
		euc = lcUtilityCurrent(params);
		params.lcDebugStr << ";euc" << euc;
	}

	sum += euc;

	double rnd = Utils::urandom();	
	float probOfCurrentLane = euc / sum;
	float probOfCL_LL = probOfCurrentLane + eul / sum;

	if (rnd <= probOfCurrentLane)
	{
		lcs = LANE_CHANGE_TO_NONE;
	}
	else if (rnd <= probOfCL_LL)
	{
		lcs = LANE_CHANGE_TO_LEFT;
	}
	else
	{
		lcs = LANE_CHANGE_TO_RIGHT;
	}*/

	params.lcDebugStr << ";change" << lcs;

	return lcs;
}

void MITSIM_LC_Model::setLaneConnectionStatus(DriverUpdateParams &params)
{
	std::string str = "setLaneConnectionStatus";
	const Lane *currLane = fwdDriverMovement->getCurrLane();

	if (fwdDriverMovement->getNextSegment())
	{
		//Check if we have a connector to the next segment from the current, left and right lanes

		std::vector<const LaneConnector *> connectors;
		currLane->getPhysicalConnectors(connectors);

		if (!connectors.empty())
		{
			params.setStatus(STATUS_CURRENT_LANE_OK, STATUS_YES, str);
		}
		else
		{
			params.setStatus(STATUS_CURRENT_LANE_OK, STATUS_NO, str);
		}

		if (params.leftLane)
		{
			connectors.clear();
			params.leftLane->getPhysicalConnectors(connectors);

			if (!connectors.empty())
			{
				params.setStatus(STATUS_LEFT_SIDE_OK, STATUS_YES, str);
			}
			else
			{
				params.setStatus(STATUS_LEFT_SIDE_OK, STATUS_NO, str);
			}
		}

		if (params.rightLane)
		{
			connectors.clear();
			params.rightLane->getPhysicalConnectors(connectors);

			if (!connectors.empty())
			{
				params.setStatus(STATUS_RIGHT_SIDE_OK, STATUS_YES, str);
			}
			else
			{
				params.setStatus(STATUS_RIGHT_SIDE_OK, STATUS_NO, str);
			}
		}
	}
	else
	{
		//Last segment on the link, so if it is not the end of the path, there is a turning group ahead

		const WayPoint *nextWayPt = fwdDriverMovement->getNextWayPoint();

		if (nextWayPt)
		{
			//Check if we have a turning path to the next link from the current, left and right lanes

			const std::map<unsigned int, TurningPath *> *turnings = nextWayPt->turningGroup->getTurningPaths(currLane->getLaneId());
			const Link *nextLink = fwdDriverMovement->getNextLink();
			
			//Clear status
			params.setStatus(STATUS_CURRENT_LANE_OK, STATUS_NO, str);

			if (turnings)
			{
				//Set status OK if a turning path connects to the next link
				for(std::map<unsigned int, TurningPath *>::const_iterator itTurnings = turnings->begin(); itTurnings != turnings->end(); ++itTurnings)
				{
					if(itTurnings->second->getToLane()->getParentSegment()->getParentLink() == nextLink)
					{
						params.setStatus(STATUS_CURRENT_LANE_OK, STATUS_YES, str);
						break;
					}
				}
			}

			if (params.leftLane)
			{
				turnings = nextWayPt->turningGroup->getTurningPaths(params.leftLane->getLaneId());
				
				//Clear status
				params.setStatus(STATUS_LEFT_SIDE_OK, STATUS_NO, str);

				if (turnings)
				{
					//Set status OK if a turning path connects to the next link
					for (std::map<unsigned int, TurningPath *>::const_iterator itTurnings = turnings->begin(); itTurnings != turnings->end(); ++itTurnings)
					{
						if (itTurnings->second->getToLane()->getParentSegment()->getParentLink() == nextLink)
						{
							params.setStatus(STATUS_LEFT_SIDE_OK, STATUS_YES, str);
							break;
						}
					}
				}
			}

			if (params.rightLane)
			{
				turnings = nextWayPt->turningGroup->getTurningPaths(params.rightLane->getLaneId());

				//Clear status
				params.setStatus(STATUS_RIGHT_SIDE_OK, STATUS_NO, str);
				
				if (turnings)
				{
					//Set status OK if a turning path connects to the next link
					for (std::map<unsigned int, TurningPath *>::const_iterator itTurnings = turnings->begin(); itTurnings != turnings->end(); ++itTurnings)
					{
						if (itTurnings->second->getToLane()->getParentSegment()->getParentLink() == nextLink)
						{
							params.setStatus(STATUS_RIGHT_SIDE_OK, STATUS_YES, str);
							break;
						}
					}					
				}
			}
		}
		else
		{
			//End of the path, all connections are OK
			params.setStatus(STATUS_CURRENT_LANE_OK, STATUS_YES, str);

			if (params.leftLane)
			{
				params.setStatus(STATUS_LEFT_SIDE_OK, STATUS_YES, str);
			}

			if (params.rightLane)
			{
				params.setStatus(STATUS_RIGHT_SIDE_OK, STATUS_YES, str);
			}
		}
	}
}
