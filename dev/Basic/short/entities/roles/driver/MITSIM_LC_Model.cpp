//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * LC_Model.cpp
 *
 *  Created on: 2011-8-15
 *      Author: mavswinwxy & Li Zhemin & Runmin Xu
 */

#include <boost/random.hpp>

#include <limits>

#include "entities/vehicle/Vehicle.hpp"
#include "entities/models/LaneChangeModel.hpp"
#include "Driver.hpp"
#include "geospatial/LaneConnector.hpp"
#include "util/Utils.hpp"
#include "util/Math.hpp"
#include "IncidentPerformer.hpp"

using std::numeric_limits;
using namespace sim_mob;


namespace {

    //Declare MAX_NUM as a private variable here to limit its scope.
    const double MAX_NUM = numeric_limits<double>::max();

    /**
     * Simple struct to hold Gap Acceptance model parameters
     */
    struct GapAcceptParam {
        double scale;
        double alpha;
        double lambda;
        double beta0;
        double beta1;
        double beta2;
        double beta3;
        double beta4;
        double stddev;
    };

//    /**
//     * Simple struct to hold mandatory lane changing parameters
//     */
//    struct MandLaneChgParam {
//        double feet_lowbound;
//        double feet_delta;
//        double lane_coeff;
//        double congest_coeff;
//        double lane_mintime;
//    };

    struct AntiGap {
        double gap;
        double critial_gap;
    };

//    const GapAcceptParam GA_PARAMETERS[4] = {
//        // scale alpha lambda beta0  beta1  beta2  beta3  beta4  stddev
//        { 1.00, 0.0, 0.000, 0.508, 0.000, 0.000, -0.420, 0.000, 0.488}, //Discretionary,lead
//        { 1.00, 0.0, 0.000, 2.020, 0.000, 0.000, 0.153, 0.188, 0.526}, //Discretionary,lag
//        { 1.00, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859}, //Mandatory,lead
//        { 1.00, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073} //Mandatory,lag
//    };

//    const MandLaneChgParam MLC_PARAMETERS = {
//        1320.0, //feet, lower bound
//        5280.0, //feet, delta
//        0.5, //coef for number of lanes
//        1.0, //coef for congestion level
//        1.0 //minimum time in lane
//    };

//    const double LC_GAP_MODELS[][9] = {
//        //	    scale  alpha   lambda   beta0   beta1   beta2   beta3   beta4   stddev
//        {1.00, 0.0, 0.000, 0.508, 0.000, 0.000, -0.420, 0.000, 0.488},
//        {1.00, 0.0, 0.000, 2.020, 0.000, 0.000, 0.153, 0.188, 0.526},
//        {1.00, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859},
//        {1.00, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073},
//        {0.60, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859}, //for test, courtesy merging
//        {0.60, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073}, //for test, courtesy merging
//        {0.20, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859}, //for test,forced merging
//        {0.20, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073}
//    }; //for test, forced merging

//    const double GAP_PARAM[][6] = {
//        //const	   dis2gap  gap-size  gap-vel   dummy  vn
//        {-1.23, -0.482, 0.224, -0.0179, 2.10, 0.239}, //back
//        {0.00, 0.00, 0.224, -0.0179, 2.10, 0.000}, //adj
//        {-0.772, -0.482, 0.224, -0.0179, 2.10, 0.675}
//    }; //fwd

    //Helper struct

    template <class T>
    struct LeadLag {
        T lead;
        T lag;
    };
}


// Kazi's LC gap model (see Kazi's MS thesis)
// from mitsim TS_Parameter.cc line 617

double sim_mob::MITSIM_LC_Model::lcCriticalGap(DriverUpdateParams& p,
        int type, double dis, double spd, double dv) {
//    const double *a = LC_GAP_MODELS[type];
	std::vector<double> a = p.LC_GAP_MODELS[type];
//    const double *b = LC_GAP_MODELS[type] + 3; //beta0
	//                    beta0            beta1                  beta2                    beta3                        beta4
	double b[] = {p.LC_GAP_MODELS[type][3], p.LC_GAP_MODELS[type][4], p.LC_GAP_MODELS[type][5], p.LC_GAP_MODELS[type][6], p.LC_GAP_MODELS[type][7]};
    double rem_dist_impact = (type < 3) ?
            0.0 : (1.0 - 1.0 / (1 + exp(a[2] * dis)));
    double dvNegative = (dv < 0) ? dv : 0.0;
    double dvPositive = (dv > 0) ? dv : 0.0;
    double gap = b[0] + b[1] * rem_dist_impact +
            b[2] * dv + b[3] * dvNegative + b[4] * dvPositive;

    boost::normal_distribution<> nrand(0, b[5]);
    boost::variate_generator< boost::mt19937, boost::normal_distribution<> >
            normal(p.gen, nrand);
    double u = gap + normal();
    double criGap = 0;
    if (u < -4.0) {
        criGap = 0.0183 * a[0]; // exp(-4)=0.0183
    } else if (u > 6.0) {
        criGap = 403.4 * a[0]; // exp(6)=403.4  
    } else {
        criGap = a[0] * exp(u);
    }
    return (criGap < a[1]) ? a[1] : criGap;
}

LaneSide sim_mob::MITSIM_LC_Model::gapAcceptance(DriverUpdateParams& p,
        int type) {
    //[0:left,1:right]
    //the speed of the closest vehicle in adjacent lane
    LeadLag<double> otherSpeed[2];
    //the distance to the closest vehicle in adjacent lane	
    LeadLag<double> otherDistance[2];

    const Lane * adjacentLanes[2] = {p.leftLane, p.rightLane};
    const NearestVehicle * fwd;
    const NearestVehicle * back;
    // get speed of forward vehicle of left lane, store in otherSpeed[0].lead
    // speed of backward vehicle of left lane, store in otherSpeed[0].lag
    //speed of forward vehicle of right lane, store in otherSpeed[1].lead
    //speed of backward vehicle of right lane, store in otherSpeed[1].lag
    for (int i = 0; i < 2; i++) {
        fwd = (i == 0) ? &p.nvLeftFwd : &p.nvRightFwd; // when i=0, fwd is left lane forward vehcile, when i=1, fwd is right lane forward vehicle
        back = (i == 0) ? &p.nvLeftBack : &p.nvRightBack;// when i=0, back is left lane backward vehcile, when i=1, back is right lane backward vehicle

        if (adjacentLanes[i]) { //the left/right side exists
            if (!fwd->exists()) { //no vehicle ahead on target lane
                otherSpeed[i].lead = 5000;
                otherDistance[i].lead = 5000;
            } else { //has vehicle ahead
                otherSpeed[i].lead = fwd->driver->fwdVelocity.get();
                otherDistance[i].lead = fwd->distance;
            }
            //check otherDistance[i].lead if <= 0 return

            if (!back->exists()) {//no vehicle behind
                otherSpeed[i].lag = -5000;
                otherDistance[i].lag = 5000;
            } else { //has vehicle behind, check the gap
                otherSpeed[i].lag = back->driver->fwdVelocity.get();
                otherDistance[i].lag = back->distance;
            }
        } else { // no left/right side exists
            otherSpeed[i].lead = 0;
            otherDistance[i].lead = 0;
            otherSpeed[i].lag = 0;
            otherDistance[i].lag = 0;
        }
    }

    //[0:left,1:right]
    LeadLag<bool> flags[2];
    for (int i = 0; i < 2; i++) { //i for left / right
        for (int j = 0; j < 2; j++) { //j for lead / lag
            if (j == 0) {
                double v = p.perceivedFwdVelocity / 100.0;
                double dv = (otherSpeed[i].lead / 100.0 - v);
                double dis = otherDistance[i].lead / 100.0;
                double cri_gap = lcCriticalGap(p, j + type, p.dis2stop, v, dv);
                flags[i].lead = (dis > cri_gap);
                if (cri_gap < 0)
                    std::cout << "find gap < 1" << std::endl;
            } else {
                double v = otherSpeed[i].lag / 100.0;
                //				double dv 	 = p.perceivedFwdVelocity/100.0 - otherSpeed[i].lag/100.0;
                double dv = otherSpeed[i].lag / 100.0 - p.perceivedFwdVelocity / 100.0; // fixed by Runmin
                double cri_gap = lcCriticalGap(p, j + type, p.dis2stop, v, dv);
                flags[i].lag = (otherDistance[i].lag / 100.0 > cri_gap);
                if (cri_gap < 0)
                    std::cout << "find gap < 1." << std::endl;
            }
        }
    }

    //Build up a return value.
    LaneSide returnVal = {false, false};
    if (flags[0].lead && flags[0].lag) {
        returnVal.left = true;
    }
    if (flags[1].lead && flags[1].lag) {
        returnVal.right = true;
    }

    return returnVal;
}

double sim_mob::MITSIM_LC_Model::calcSideLaneUtility(DriverUpdateParams& p, bool isLeft) {
    if (isLeft && !p.leftLane) {
        return -MAX_NUM; //has no left side
    } else if (!isLeft && !p.rightLane) {
        return -MAX_NUM; //has no right side
    }
    return (isLeft) ? p.nvLeftFwd.distance : p.nvRightFwd.distance;
}

LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::makeDiscretionaryLaneChangingDecision(DriverUpdateParams& p) {
    // for available gaps(including current gap between leading vehicle and itself), vehicle will choose the longest
    //const LaneSide freeLanes = gapAcceptance(p, DLC);
    LaneSide freeLanes = gapAcceptance(p, DLC);
    if (!freeLanes.left && !freeLanes.right) {
        return LCS_SAME; //neither gap is available, stay in current lane
    }
    double s = p.nvFwd.distance;
    const double satisfiedDistance = 2000;
    if (s > satisfiedDistance) {
        return LCS_SAME; // space ahead is satisfying, stay in current lane
    }

    //choose target gap, for both left and right
    std::vector<TARGET_GAP> tg;
    tg.push_back(TG_Same);
    tg.push_back(TG_Same);
    chooseTargetGap(p, tg);

    //calculate the utility of both sides
    double leftUtility = calcSideLaneUtility(p, true);
    double rightUtility = calcSideLaneUtility(p, false);

    //to check if their utilities are greater than current lane
    bool left = (s < leftUtility);
    bool right = (s < rightUtility);

    //decide
    if (freeLanes.rightOnly() && right) {
        return LCS_RIGHT;
    }
    if (freeLanes.leftOnly() && left) {
        return LCS_LEFT;
    }
    if (freeLanes.both()) {
        // avoid ossilation
        return p.lastDecision;
    }

    if (left || right) {
        p.targetGap = (leftUtility > rightUtility) ? tg[0] : tg[1];
    }

    return LCS_SAME;
}


double sim_mob::MITSIM_LC_Model::checkIfMandatory(DriverUpdateParams& p) {
    if (p.nextLaneIndex == p.currLaneIndex)
        p.dis2stop = 5000; //defalut 5000m
    //The code below is MITSIMLab model
    double num = 1; //now we just assume that MLC only need to change to the adjacent lane
    double y = 0.5; //segment density/jam density, now assume that it is 0.5
    //double delta0	=	feet2Unit(MLC_parameters.feet_lowbound);
    double dis2stop_feet = Utils::toFeet(p.dis2stop);
    double dis = dis2stop_feet - MLC_PARAMETERS.feet_lowbound;
    double delta = 1.0 + MLC_PARAMETERS.lane_coeff * num + MLC_PARAMETERS.congest_coeff * y;
    delta *= MLC_PARAMETERS.feet_delta;
    return (delta == 0) ? 1 : exp(-dis * dis / (delta * delta));
}

LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::makeMandatoryLaneChangingDecision(DriverUpdateParams& p) {
    LaneSide freeLanes = gapAcceptance(p, MLC);
    //find which lane it should get to and choose which side to change
    //now manually set to 1, it should be replaced by target lane index
    //i am going to fix it.
    int direction = p.nextLaneIndex - p.currLaneIndex;
    //current lane is target lane
    if (direction == 0) {
        return LCS_SAME;
    }

    //current lane isn't target lane
    if (freeLanes.right && direction < 0) { 
        //target lane on the right and is accessible
        p.isWaiting = false;
        return LCS_RIGHT;
    } else if (freeLanes.left && direction > 0) { 
        //target lane on the left and is accessible
        p.isWaiting = false;
        return LCS_LEFT;
    } else { 
        //when target side isn't available,vehicle will decelerate to wait a proper gap.
        p.isWaiting = true;
        return LCS_SAME;
    }
}

//LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::executeNGSIMModel(DriverUpdateParams& p) {
//    bool isCourtesy = false; //if courtesy merging
//    bool isForced = false; // if forced merging
//    int direction = p.nextLaneIndex - p.currLaneIndex;
//    LANE_CHANGE_SIDE lcs = direction > 0 ? LCS_LEFT : LCS_RIGHT;
//
//    //check if courtesy merging
//    isCourtesy = ifCourtesyMerging(p);
//    if (isCourtesy) {
//        lcs = makeCourtesyMerging(p);
//        return lcs;
//    } else {
//        //check if forced merging
//        isForced = ifForcedMerging(p);
//        if (isForced) {
//            lcs = makeForcedMerging(p);
//            return lcs;
//        }
//    }
//    return LCS_SAME;
//}

bool sim_mob::MITSIM_LC_Model::ifCourtesyMerging(DriverUpdateParams& p) {
    //[0:left,1:right]
    LeadLag<double> otherSpeed[2]; //the speed of the closest vehicle in adjacent lane
    LeadLag<double> otherDistance[2]; //the distance to the closest vehicle in adjacent lane
    LeadLag<double> otherAcc[2]; //the acceleration of the closet vehicles in the adjacent lane

    const Lane * adjacentLanes[2] = {p.leftLane, p.rightLane};
    const NearestVehicle * fwd;
    const NearestVehicle * back;
    for (int i = 0; i < 2; i++) {
        fwd = (i == 0) ? &p.nvLeftFwd : &p.nvRightFwd;
        back = (i == 0) ? &p.nvLeftBack : &p.nvRightBack;

        if (adjacentLanes[i]) { //the left/right side exists
            if (!fwd->exists()) { //no vehicle ahead on current lane
                otherSpeed[i].lead = 5000;
                otherDistance[i].lead = 5000;
                otherAcc[i].lead = 5000;
            } else { //has vehicle ahead
                otherSpeed[i].lead = fwd->driver->fwdVelocity.get();
                otherDistance[i].lead = fwd->distance;
                otherAcc[i].lead = fwd->driver->fwdAccel.get();
            }

            if (!back->exists()) {//no vehicle behind
                otherSpeed[i].lag = -5000;
                otherDistance[i].lag = 5000;
                otherAcc[i].lag = 5000;
            } else { //has vehicle behind, check the gap
                otherSpeed[i].lag = back->driver->fwdVelocity.get();
                otherDistance[i].lag = back->distance;
                otherAcc[i].lag = back->driver->fwdAccel.get();
            }
        } else { // no left/right side exists
            otherSpeed[i].lead = 0;
            otherDistance[i].lead = 0;
            otherAcc[i].lead = 0;
            otherSpeed[i].lag = 0;
            otherDistance[i].lag = 0;
            otherAcc[i].lag = 0;
        }
    }

    int direction = p.nextLaneIndex - p.currLaneIndex;
    //[0:left,1:right]
    int i = direction > 0 ? 0 : 1;
    //if (direction > 0) direction = 0;
    //else direction = 1;

    double dis_lead = otherDistance[i].lead / 100.0;
    double dis_lag = otherDistance[i].lag / 100.0;

    double v_lead = otherSpeed[i].lead / 100.0;
    double v_lag = otherSpeed[i].lag / 100.0;

    double acc_lead = otherAcc[i].lead / 100.0;
    double acc_lag = otherAcc[i].lag / 100.0;


    double gap = dis_lead + dis_lag + (v_lead - v_lag) * p.elapsedSeconds + 0.5 * (acc_lead - acc_lag) * p.elapsedSeconds * p.elapsedSeconds;

    double para[4] = {1.82, 1.81, -0.153, 0.0951};

    double v = v_lag - p.perceivedFwdVelocity / 100;
    double dv = v > 0 ? v : 0;

    //calculate critical gap for courtesy merging
    double critical_gap = exp(para[0] + para[1] * dv + para[3] * p.dis2stop / 100);
    bool courtesy = gap - critical_gap > 0 ? true : false;
    return courtesy;
}

//bool sim_mob::MITSIM_LC_Model::ifForcedMerging(DriverUpdateParams& p) {
//    boost::uniform_int<> zero_to_max(0, RAND_MAX);
//    double randNum = (double) (zero_to_max(p.gen) % 1000) / 1000;
//    if (randNum < 1 / (1 + exp(4.27 + 1.25 - 5.43))) {
//        return true;
//    }
//    return false;
//}

LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::makeCourtesyMerging(DriverUpdateParams& p) {
    LaneSide freeLanes = gapAcceptance(p, MLC_C);
    int direction = p.nextLaneIndex - p.currLaneIndex;
    //current lane is target lane
    if (direction == 0) {
        return LCS_SAME;
    }

    //current lane isn't target lane
    if (freeLanes.right && direction < 0) {
        //target lane on the right and is accessible
        p.isWaiting = false;
        return LCS_RIGHT;
    } else if (freeLanes.left && direction > 0) {
        //target lane on the left and is accessible
        p.isWaiting = false;
        return LCS_LEFT;
    } else {
        //when target side isn't available,vehicle will decelerate to wait a proper gap.
        p.isWaiting = true;
        return LCS_SAME;
    }
}

LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::makeForcedMerging(DriverUpdateParams& p) {
    LaneSide freeLanes = gapAcceptance(p, MLC_F);

    int direction = p.nextLaneIndex - p.currLaneIndex;
    if (direction == 0) {
        return LCS_SAME;
    }

    //current lane isn't target lane
    if (freeLanes.right && direction < 0) { 
        //target lane on the right and is accessible
        p.isWaiting = false;
        return LCS_RIGHT;
    } else if (freeLanes.left && direction > 0) { 
        //target lane on the left and is accessible
        p.isWaiting = false;
        return LCS_LEFT;
    } else {
        p.isWaiting = true;
        return LCS_SAME;
    }
}


//TODO:I think lane index should be a data member in the lane class

size_t getLaneIndex(const Lane* l) {
    if (l) {
        const RoadSegment* r = l->getRoadSegment();
        for (size_t i = 0; i < r->getLanes().size(); i++) {
            if (r->getLanes().at(i) == l) {
                return i;
            }
        }
    }
    return -1; //NOTE: This might not do what you expect! ~Seth
}

void sim_mob::MITSIM_LC_Model::chooseTargetGap(DriverUpdateParams& p, 
        std::vector<TARGET_GAP>& tg) {
    const Lane * lane[2] = {p.leftLane, p.rightLane};

    //nearest vehicles
    NearestVehicle * nv[2][4];
    nv[0][0] = &p.nvLeftBack2;
    nv[0][1] = &p.nvLeftBack;
    nv[0][2] = &p.nvLeftFwd;
    nv[0][3] = &p.nvLeftFwd2;
    nv[1][0] = &p.nvRightBack2;
    nv[1][1] = &p.nvRightBack;
    nv[1][2] = &p.nvRightFwd;
    nv[1][3] = &p.nvRightFwd2;

    double dis[2][4];
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            dis[i][j] = (nv[i][j]->exists()) ? nv[i][j]->distance / 100 : 50;
        }
    }

    double vel[2][4];
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            vel[i][j] = (nv[i][j]->exists()) ? nv[i][j]->driver->fwdVelocity / 100 : 0;
        }

    }

    boost::uniform_int<> zero_to_max(0, RAND_MAX);
    double randNum = (double) (zero_to_max(p.gen) % 1000) / 1000;

    //calculate the utilities of nearby gaps
    double U[2][3];
    for (int i = 0; i < 2; i++) {
        U[i][0] = GAP_PARAM[0][0] + GAP_PARAM[0][1] * dis[i][1] + GAP_PARAM[0][2]*(dis[i][0] - dis[i][1]) + GAP_PARAM[0][3]*(vel[i][0] - vel[i][1]) + ((!nv[i][0]->exists()) ? GAP_PARAM[0][4] : 0) + GAP_PARAM[0][5] * randNum;
        U[i][1] = GAP_PARAM[1][0] + GAP_PARAM[1][1]*0 + GAP_PARAM[1][2]*(dis[i][1] + dis[i][2]) + GAP_PARAM[1][3]*(vel[i][1] - vel[i][2]) + ((!nv[i][1]->exists() || !nv[i][2]->exists()) ? GAP_PARAM[1][4] : 0) + GAP_PARAM[1][5] * randNum;
        U[i][2] = GAP_PARAM[2][0] + GAP_PARAM[2][1] * dis[i][2] + GAP_PARAM[2][2]*(dis[i][3] - dis[i][2]) + GAP_PARAM[2][3]*(vel[i][2] - vel[i][3]) + ((!nv[i][3]->exists()) ? GAP_PARAM[2][4] : 0) + GAP_PARAM[2][5] * randNum;
        if (!lane[i]) U[i][0] = U[i][1] = U[i][2] = -MAX_NUM;
    }

    //calculte probabilities of choosing each gap
    double logsum[2] = {0.0, 0.0};
    double cdf[2][3];
    double prob[2][3];
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            logsum[i] += exp(U[i][j]);
            cdf[i][j] = logsum[i];
        }
    }
    double rnd = (double) (zero_to_max(p.gen) % 1000) / 1000;

    if (rnd >= 0 && rnd < cdf[0][0]) {
        tg[0] = TG_Left_Back;
    } else if (rnd >= cdf[0][0] && rnd < cdf[0][1]) {
        tg[0] = TG_Left_Adj;
    } else if (rnd >= cdf[0][1] && rnd < cdf[0][2]) {
        tg[0] = TG_Left_Fwd;
    }

    if (rnd >= 0 && rnd < cdf[1][0]) {
        tg[1] = TG_Right_Back;
    } else if (rnd >= cdf[1][0] && rnd < cdf[1][1]) {
        tg[1] = TG_Right_Adj;
    } else if (rnd >= cdf[1][1] && rnd < cdf[1][2]) {
        tg[1] = TG_Right_Fwd;
    }
}
sim_mob::MITSIM_LC_Model::MITSIM_LC_Model(DriverUpdateParams& p)
{
	modelName = "general_driver_model";
	splitDelimiter = " ,";

	initParam(p);
}
void sim_mob::MITSIM_LC_Model::initParam(DriverUpdateParams& p)
{
	std::string str;
	// MLC_PARAMETERS
	ParameterManager::Instance()->param(modelName,"MLC_PARAMETERS",str,string("1320.0  5280.0 0.5 1.0  1.0"));
	makeMCLParam(str);
	// LC_GAP_MODELS
	std::vector< std::string > strArray;
	ParameterManager::Instance()->param(modelName,"LC_GAP_MODELS_0",str,string("1.00,   0.0,   0.000,  0.508,  0.000,  0.000,  -0.420, 0.000,   0.488"));
	strArray.push_back(str);
	ParameterManager::Instance()->param(modelName,"LC_GAP_MODELS_1",str,string("1.00, 0.0, 0.000, 2.020, 0.000, 0.000, 0.153, 0.188, 0.526"));
	strArray.push_back(str);
	ParameterManager::Instance()->param(modelName,"LC_GAP_MODELS_2",str,string("1.00, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859"));
	strArray.push_back(str);
	ParameterManager::Instance()->param(modelName,"LC_GAP_MODELS_3",str,string("1.00, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073"));
	strArray.push_back(str);
	ParameterManager::Instance()->param(modelName,"LC_GAP_MODELS_4",str,string("0.60, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859"));
	strArray.push_back(str);
	ParameterManager::Instance()->param(modelName,"LC_GAP_MODELS_5",str,string("0.60, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073"));
	strArray.push_back(str);
	ParameterManager::Instance()->param(modelName,"LC_GAP_MODELS_6",str,string("0.20, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859"));
	strArray.push_back(str);
	ParameterManager::Instance()->param(modelName,"LC_GAP_MODELS_7",str,string("0.20, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073"));
	strArray.push_back(str);
	makeCtriticalGapParam(p,strArray);
	// GAP_PARAM
	strArray.clear();
	ParameterManager::Instance()->param(modelName,"GAP_PARAM_0",str,string("-1.23, -0.482, 0.224, -0.0179, 2.10, 0.239"));
	strArray.push_back(str);
	ParameterManager::Instance()->param(modelName,"GAP_PARAM_1",str,string("0.00,   0.00,  0.224, -0.0179, 2.10, 0.000"));
	strArray.push_back(str);
	ParameterManager::Instance()->param(modelName,"GAP_PARAM_2",str,string("-0.772, -0.482, 0.224, -0.0179, 2.10, 0.675"));
	strArray.push_back(str);
	makeTargetGapPram(strArray);
	// min speed
	ParameterManager::Instance()->param(modelName,"min_speed",minSpeed,0.1);
	// LC Mandatory Probability Model
	ParameterManager::Instance()->param(modelName,"mlc_params",str,string("132.0  528.0   0.5  1.0 1.0"));

	// driver look ahead distancd
	lookAheadDistance = mlcDistance();

	// lane Utility Params
	ParameterManager::Instance()->param(modelName,"lane_utility_model",str,
			string("3.9443 -0.3213  -1.1683  -1.1683 0.0 0.0633 -1.0 0.0058 -0.2664 -0.0088 -3.3754 10 19 -2.3400 -4.5084 -2.8257 -1.2597 -0.7239 -0.3269"));
	makeLanetilityParams(str);

	// critical gap param
	ParameterManager::Instance()->param(modelName,"critical_gaps_param",str,
				string("0.5 -0.231  -2.700  1.112	  0.5   0.000 0.2  0.742   6.0"));
	makeCriticalGapParams(str);

	// nosing param
	ParameterManager::Instance()->param(modelName,"nosing_param",str,
				string("1.0 0.5  0.6  0.1	  0.2   1.0 300.0  180.0   600.0 40.0"));
	makeNosingParams(p,str);

	// kazi nosing param
	ParameterManager::Instance()->param(modelName,"kazi_nosing_param",str,
					string("-3.159  0.313  -0.027  2.050  0.028  0.6"));
	makekaziNosingParams(str);

	//CF_CRITICAL_TIMER_RATIO
	ParameterManager::Instance()->param(modelName,"CF_CRITICAL_TIMER_RATIO",CF_CRITICAL_TIMER_RATIO,0.5);

	//LC Yielding Model
	ParameterManager::Instance()->param(modelName,"LC_Yielding_Model",str,string("0.80 1.0"));
}
void sim_mob::MITSIM_LC_Model::makeMCLParam(std::string& str)
{
	std::vector<double> array;
	sim_mob::Utils::convertStringToArray(str,array);
	MLC_PARAMETERS.feet_lowbound = array[0];
	MLC_PARAMETERS.feet_delta = array[1];
	MLC_PARAMETERS.lane_coeff = array[2];
	MLC_PARAMETERS.congest_coeff = array[3];
	MLC_PARAMETERS.lane_mintime = array[4];

}
void sim_mob::MITSIM_LC_Model::makeCtriticalGapParam(DriverUpdateParams& p,std::vector< std::string >& strMatrix)
{
	for(int i=0;i<strMatrix.size();++i)
	{
		std::vector<double> array;
		sim_mob::Utils::convertStringToArray(strMatrix[i],array);
		p.LC_GAP_MODELS.push_back(array);
	}
}
void sim_mob::MITSIM_LC_Model::makeTargetGapPram(std::vector< std::string >& strMatrix)
{
	for(int i=0;i<strMatrix.size();++i)
	{
		std::vector<double> array;
		sim_mob::Utils::convertStringToArray(strMatrix[i],array);
		GAP_PARAM.push_back(array);
	}
}
void sim_mob::MITSIM_LC_Model::makeLanetilityParams(std::string& str)
{
	sim_mob::Utils::convertStringToArray(str,laneUtilityParams);
}
void sim_mob::MITSIM_LC_Model::makeNosingParams(DriverUpdateParams& p,string& str)
{
	sim_mob::Utils::convertStringToArray(str,p.nosingParams);
	lcMaxNosingDis = p.nosingParams[8];
	lcMaxStuckTime = p.nosingParams[7];
	lcNosingConstStateTime = p.nosingParams[0];
	p.lcMaxNosingTime = p.nosingParams[6];
}
void sim_mob::MITSIM_LC_Model::makekaziNosingParams(string& str)
{
	sim_mob::Utils::convertStringToArray(str,kaziNosingParams);
}
void sim_mob::MITSIM_LC_Model::makelcYieldingProb(string& str)
{
	sim_mob::Utils::convertStringToArray(str,lcYieldingProb);
}
void sim_mob::MITSIM_LC_Model::makeCriticalGapParams(std::string& str)
{
	sim_mob::Utils::convertStringToArray(str,criticalGapParams);
}
LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::checkForLookAheadLC(DriverUpdateParams& p)
{
	LANE_CHANGE_SIDE change = LCS_SAME;

	// if already in changing lane
	if ( p.flag(FLAG_ESCAPE) )
	{
		if (p.flag(FLAG_ESCAPE_LEFT)) {
		  change = LCS_LEFT;
		}
		if (p.flag(FLAG_ESCAPE_RIGHT)) {
		  change = LCS_RIGHT;
		}
		p.setStatus(STATUS_MANDATORY);
		return change;
	}

	// get distance to end of current segment
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	float x=driverMvt->fwdDriverMovement.getDisToCurrSegEnd();

	// if current segment has enough distance to do lc , keep current lane
	if ( x>=lookAheadDistance )
	{
	    return change;
	}

	// find lanes connect to target segment in lookahead distance
	driverMvt->fwdDriverMovement.getNextSegment(true);
	std::vector<sim_mob::Lane*> connectedLanes;
	driverMvt->getLanesConnectToLookAheadDis(lookAheadDistance,connectedLanes);

	int nRight = 100; // number of lane changes required for the current lane.
	int nLeft = 100; // number of lane changes required for the current lane.
	int nCurrent = 100; // number of lane changes required for the current lane.


	for (int i = 0; i < connectedLanes.size(); i++) {

	 int numlcRight  = abs (getLaneIndex(connectedLanes[i]) -  (p.currLaneIndex+1)) ;
	 int numlcLeft  = abs (getLaneIndex(connectedLanes[i]) -  (p.currLaneIndex-1)) ;
	 int numlcCurrent  = abs (getLaneIndex(connectedLanes[i]) -  p.currLaneIndex) ;

	 nRight =  std::min<int>(nRight, numlcRight);
	 nLeft =  std::min<int>(nLeft, numlcLeft);
	 nCurrent =  std::min<int>(nCurrent, numlcCurrent);

	}

	double eul = 0.0, eur = 0.0, euc = 1.0 ;
	double lcDistance = p.dis2stop;

	for(int i=0;i<connectedLanes.size();i++)
	{
		if(p.leftLane == connectedLanes[i])
		{
			eul = lcUtilityLookAheadLeft(p, nLeft, lcDistance);
		}
		if(p.rightLane == connectedLanes[i])
		{
			eur = lcUtilityLookAheadRight(p, nRight, lcDistance);
		}
		if(p.currLane == connectedLanes[i])
		{
			euc = lcUtilityLookAheadCurrent(p, nRight, lcDistance);
		}
	}

	double sum = eul + eur ;
	if(sum > 0)
	{

	}
	else
	{
		return LCS_SAME;
	}

	sum += euc;

	boost::uniform_int<> zero_to_max(0, RAND_MAX);
	double rnd = (double) (zero_to_max(p.gen) % 1000) / 1000;

	float probOfCurrentLane = euc / sum;
	float probOfCL_LL = probOfCurrentLane + eul / sum;
	if (rnd < probOfCurrentLane) change = LCS_SAME ;
	else if (rnd < probOfCL_LL) change = LCS_LEFT ;
	else change = LCS_RIGHT ;



	return change;
}
double sim_mob::MITSIM_LC_Model::lcUtilityLookAheadLeft(DriverUpdateParams& p,int n, float LCdistance)
{
	vector<double> a = laneUtilityParams;

	double vld, mlc, density, spacing;

	//density = plane->density();
	// TODO calculate lane density
	density = 0;
	float heavy_neighbor = 0.0;
	if(p.nvLeftFwd.exists()) // front left bumper leader
	{
		double leftFwdVel = p.nvLeftFwd.driver->fwdVelocity.get();
		double currentSpeed = p.perceivedFwdVelocity / 100.0;
		vld = std::min<double>(leftFwdVel,currentSpeed);

		if(p.nvLeftFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)// get vh type, heavy vh only bus now
		{
			heavy_neighbor = a[7];
			spacing = p.nvLeftFwd.distance;
		}
	}
	else
	{
		vld = vld = p.desiredSpeed;
		spacing = p.dis2stop; // MITSIM distance()
	}

	if(p.nvLeftBack.exists())// back left bumper leader
	{
		if(p.nvLeftBack.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)// get vh type, heavy vh only bus now
		{
			heavy_neighbor = a[7];
		}
	}

	float left_most = 0.0;

	// right hand driving,so left_most = 0

	switch (n) {
	  case 0:
	    {
	      mlc = 0;
	      break;
	    }
	  case 1:
	    {
	      mlc = a[12] * pow(p.dis2stop/1000.0, a[17]) + a[15];  // why divide 1000
	      break;
	    }
	  case 2:
	    {
	      mlc = a[13] * pow(p.dis2stop/1000.0, a[17]) + a[15] + a[16];
	      break;
	    }
	  default:
	    {
	      mlc = (a[13]+a[14]*(n-2)) * pow(p.dis2stop/1000, a[17]) +a[15] + a[16] * (n-1);
	    }
	    break;
	  }

	//TODO: check bus stop ahead
	// MITSIM TS_LCModels.cc Dan: If vehicle ahead is a bus and there is a bus stop ahead
	  // in the lane, set busAheadDummy to 1 for disincentive to be
	  // applied in the utility.
	int busAheadDummy = 0;
	if(p.nvLeftFwd.exists())
	{
		if(p.nvLeftFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			busAheadDummy = 1;
		}
	}

	double u = a[4] * vld + a[8] * spacing + a[6] * density + mlc + heavy_neighbor + left_most + a[5] * busAheadDummy;

	return exp(u) ;
}
double sim_mob::MITSIM_LC_Model::lcUtilityLookAheadRight(DriverUpdateParams& p,int n, float LCdistance)
{
	vector<double> a = laneUtilityParams;

	double vld, mlc, density, spacing;

	//density = plane->density();
	// TODO calculate lane density
	density = 0;
	float heavy_neighbor = 0.0;
	if(p.nvRightFwd.exists()) // front left bumper leader
	{
		double leftFwdVel = p.nvRightFwd.driver->fwdVelocity.get();
		double currentSpeed = p.perceivedFwdVelocity / 100.0;
		vld = std::min<double>(leftFwdVel,currentSpeed);

		if(p.nvRightFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)// get vh type, heavy vh only bus now
		{
			heavy_neighbor = a[7];
			spacing = p.nvRightFwd.distance;
		}
	}
	else
	{
		vld = vld = p.desiredSpeed;
		spacing = p.dis2stop; // MITSIM distance()
	}

	if(p.nvRightBack.exists())// back left bumper leader
	{
		if(p.nvRightBack.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)// get vh type, heavy vh only bus now
		{
			heavy_neighbor = a[7];
		}
	}

	float right_most = 0.0;

	// right hand driving
	// check if current lane is most right lane, which lane idx is 0
	if(getLaneIndex(p.currLane) == 0  || (getLaneIndex(p.currLane) == 1 && p.currLane->getRoadSegment()->getLanes().at(0)->is_pedestrian_lane()) )
	{
		right_most = 0.0;
	}
	else
	{
		right_most = a[2];
	}

	switch (n) {
	  case 0:
		{
		  mlc = 0;
		  break;
		}
	  case 1:
		{
		  mlc = a[12] * pow(p.dis2stop/1000.0, a[17]) + a[15];  // why divide 1000
		  break;
		}
	  case 2:
		{
		  mlc = a[13] * pow(p.dis2stop/1000.0, a[17]) + a[15] + a[16];
		  break;
		}
	  default:
		{
		  mlc = (a[13]+a[14]*(n-2)) * pow(p.dis2stop/1000, a[17]) +a[15] + a[16] * (n-1);
		}
		break;
	  }

	//TODO: check bus stop ahead
	// MITSIM TS_LCModels.cc Dan: If vehicle ahead is a bus and there is a bus stop ahead
	  // in the lane, set busAheadDummy to 1 for disincentive to be
	  // applied in the utility.
	int busAheadDummy = 0;
	if(p.nvRightFwd.exists())
	{
		if(p.nvRightFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			busAheadDummy = 1;
		}
	}

	double u = a[1] + a[4] * vld + a[8] * spacing +a[6] * density + mlc + heavy_neighbor + right_most + a[5] * busAheadDummy;

	return exp(u) ;
}
double sim_mob::MITSIM_LC_Model::lcUtilityLookAheadCurrent(DriverUpdateParams& p,int n, float LCdistance)
{
	vector<double> a = laneUtilityParams;

	double vld, mlc, density, spacing;

	//density = plane->density();
	// TODO calculate lane density
	density = 0;
	float heavy_neighbor = 0.0;
	if(p.nvFwd.exists()) // front left bumper leader
	{
		double leftFwdVel = p.nvFwd.driver->fwdVelocity.get();
		double currentSpeed = p.perceivedFwdVelocity / 100.0;
		vld = std::min<double>(leftFwdVel,currentSpeed);

		if(p.nvRightFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)// get vh type, heavy vh only bus now
		{
			heavy_neighbor = a[7];
			spacing = p.nvRightFwd.distance;
		}
	}
	else
	{
		vld = vld = p.desiredSpeed;
		spacing = p.dis2stop; // MITSIM distance()
	}

	if(p.nvRightBack.exists())// back left bumper leader
	{
		if(p.nvRightBack.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)// get vh type, heavy vh only bus now
		{
			heavy_neighbor = a[7];
		}
	}

	float right_most = 0.0;

	// right hand driving
	// check if current lane is most right lane, which lane idx is 0
	if(getLaneIndex(p.currLane) == 0  || (getLaneIndex(p.currLane) == 1 && p.currLane->getRoadSegment()->getLanes().at(0)->is_pedestrian_lane()) )
	{
		right_most = 0.0;
	}
	else
	{
		right_most = a[2];
	}

	switch (n) {
	  case 0:
		{
		  mlc = 0;
		  break;
		}
	  case 1:
		{
		  mlc = a[12] * pow(p.dis2stop/1000.0, a[17]) + a[15];  // why divide 1000
		  break;
		}
	  case 2:
		{
		  mlc = a[13] * pow(p.dis2stop/1000.0, a[17]) + a[15] + a[16];
		  break;
		}
	  default:
		{
		  mlc = (a[13]+a[14]*(n-2)) * pow(p.dis2stop/1000, a[17]) +a[15] + a[16] * (n-1);
		}
		break;
	  }

	float tailgate_dummy = 0;
//	TS_Vehicle* behind = this->vehicleBehind() ;
	if (p.nvBack.exists()) {
		double gap_behind = p.nvBack.distance;
		//TODO: calculate segment density
	    float dens = 0.0;//tsSegment()->density();
	    tailgate_dummy = (gap_behind <= a[10] && dens <= a[11])? a[9] : 0;
	   }

	//TODO: check bus stop ahead
	// MITSIM TS_LCModels.cc Dan: If vehicle ahead is a bus and there is a bus stop ahead
	  // in the lane, set busAheadDummy to 1 for disincentive to be
	  // applied in the utility.
	int busAheadDummy = 0;
	if(p.nvRightFwd.exists())
	{
		if(p.nvRightFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			busAheadDummy = 1;
		}
	}

	double u = a[0]+ a[4] * vld + a[8] * spacing + a[6] * density + mlc + heavy_neighbor + right_most + tailgate_dummy + a[5] * busAheadDummy;

	return exp(u) ;
}
double sim_mob::MITSIM_LC_Model::lcCriticalGap(sim_mob::DriverUpdateParams& p, int type,double dv)
{
	vector<double> a = criticalGapParams;

	float dvNegative = (dv < 0) ? dv : 0.0;
	float dvPositive = (dv > 0) ? dv : 0.0;

	float gap =0.0;

	float maxdiff = a[8];

	switch (type){
	case 0: {     // lead gap
		gap = a[0] + a[1] * dvNegative +a[2] * dvPositive + Utils::generateFloat(0, a[3]);
		break;
		}
	case 1: {     // lag gap
		gap = a[4] + a[5]  * dvNegative + a[6] * std::min<double>(dvPositive, maxdiff) + Utils::generateFloat(0, a[7]);
		break;
		}
	}

	float cri_gap  = exp(gap);

	return cri_gap ;
}
double sim_mob::MITSIM_LC_Model::mlcDistance()
{
	double n = Utils::generateFloat(0,1.0);
//	float dis = mlcParams[0] + n*(mlcParams_[1] - mlcParams_[0]);
	double dis = MLC_PARAMETERS.feet_lowbound + n*(MLC_PARAMETERS.feet_delta - MLC_PARAMETERS.feet_lowbound);
	return dis;
}
LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::makeLaneChangingDecision(DriverUpdateParams& p)
{
	// if in the middle of lc , shall not reach here
	if(p.cftimer > sim_mob::Math::DOUBLE_EPSILON)
	{
		return LCS_SAME;
	}

	if(p.perceivedFwdVelocity/100 < minSpeed)
	{
		return LCS_SAME;
	}

	if (timeSinceTagged(p) < MLC_PARAMETERS.lane_mintime)
	{
		return LCS_SAME;
	}

	LANE_CHANGE_SIDE change = LCS_SAME;		// direction to change

	if (!path(p)) {
		//TODO handle re-route
	}
	else
	{
		// check lanes connect to next segment
		checkConnectLanes(p);

		if (checkIfLookAheadEvents(p))
		{
			change = checkMandatoryEventLC(p);
		}
		else
		{
			change = checkForLookAheadLC(p);
		}
	}

	if (change == LCS_LEFT) {		// to left
		p.unsetStatus(STATUS_CHANGING);
		p.setStatus(STATUS_LEFT);
	} else if (change == LCS_RIGHT) {	// to right
		p.unsetStatus(STATUS_CHANGING);
		p.setStatus(STATUS_RIGHT);
	} else {
		p.unsetStatus(STATUS_CHANGING);
	}
	return change;
}
double sim_mob::MITSIM_LC_Model::executeLaneChanging(DriverUpdateParams& p)
{
	LANE_CHANGE_SIDE changeMode = LCS_SAME;

	if (p.getStatus(STATUS_LEFT)) {
		changeMode = LCS_LEFT;
	} else if (p.getStatus(STATUS_RIGHT)) {
		changeMode = LCS_RIGHT;
	} else {
		return 0.0;			// No request for lane change
	}

	const NearestVehicle * av; // leader vh
	const NearestVehicle * bv; // follower vh

	if(changeMode == LCS_LEFT)
	{
		av = &p.nvLeftFwd;
		bv = &p.nvLeftBack;
	}
	else
	{
		av = &p.nvRightFwd;
		bv = &p.nvRightBack;
	}

//	//LEADING VEHICLE IN TARGET LANE (must be in same segment).
//	if(av->exists())
//	{
//		// get fwd vh segment
//		DriverMovement *fwdDriverMvt = (DriverMovement*)av->driver->Movement();
//		const RoadSegment* fwdRs = fwdDriverMvt->fwdDriverMovement.getCurrSegment();
//		// get current segment
//		DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
//		const RoadSegment* rs = driverMvt->fwdDriverMovement.getCurrSegment();
//
//		if(fwdRs != rs)
//		{
//
//		}
//	}

	// LEADING HEADWAY
	float aheadway;		// leading headway
	if (av->exists()) {
		aheadway = av->distance;
	} else {
		aheadway = Math::FLT_INF;
	}

	 // LAG HEADWAY

	float bheadway;		// lag headway
	if (bv->exists()) {
		bheadway = bv->distance;
	} else {
		bheadway = Math::FLT_INF;
	}

	// Is the lane change is triggered by special events
	int escape = p.flag(FLAG_ESCAPE);

	int lctype;
	if ( p.getStatus(STATUS_MANDATORY) &&
	   ( !p.getStatus(STATUS_CURRENT_OK) ) ) {
		lctype = 2;		// wrong lane and concerned
	} else if (escape) {
		lctype = 2;		// special event
	} else {
		lctype = 0;		// discretionary
	}

	if (bv->exists() && bheadway < lcCriticalGap(p,1, bv->driver->fwdVelocity/100.0 - p.currSpeed )) {
		p.setFlag(FLAG_LC_FAILED_LAG); // lag gap
	}
	else if (av && aheadway < lcCriticalGap(p,0, av->driver->fwdVelocity/100.0 - p.currSpeed)) {
		p.setFlag(FLAG_LC_FAILED_LEAD); // lead gap
	}

	// CHECK IF THE GAPS ARE ACCEPTABLE
	if (p.flag(FLAG_LC_FAILED)) {

		//TODO special cases
		//wrong lane?
//		int nlanes = lane_->isWrongLane(this);
		int nlanes = 0;

		// The gaps are not acceptable and this guy is in mandatory
		// state.  Check if this vehicle is nosing or becomes aggressive
		// and decide to nose in.

		int nosing = p.flag(FLAG_NOSING);

		if (!nosing && p.dis2stop < lcMaxNosingDis) {
			float gap = aheadway + bheadway ;
			float dv = av->exists() ? av->driver->fwdVelocity/100.0 - p.currSpeed : 0 ;

			float pmf;

			if ( p.getStatus(STATUS_CURRENT_OK) && p.nextLink() ) {
				    // current lane connects to next link on path

				float longer_dis = p.dis2stop + p.nextLink()->length/100.0;
				pmf = lcNosingProb(longer_dis, dv, gap, nlanes) ;
				// remaining distance for forced merging is the length remaining
				// in the current link plus at least the length of the first
				// segment of the next link
				}
				else {
				// current lane does not connect to next link on path

				pmf = lcNosingProb(p.dis2stop, dv, gap, nlanes) ;
				// forced merging must be performed in current link
				}
				//  end Revised model

				nosing = Utils::brandom(pmf);

		}// if maxNosingDis

		p.unsetFlag(FLAG_NOSING);	// reset the flag

		if (nosing) {
			lcProdNoseRejProb = 1.0;
			// Since I am nosing, updating of acceleration rate sooner
			p.cftimer = CF_CRITICAL_TIMER_RATIO * p.nextStepSize;
			// Now I am going to nose in provided it is feasible and the
		    // lag vehicle is willing to yield
			bool bve = !bv->exists();
			Driver* bvd = const_cast<Driver*>(bv->driver);
			DriverUpdateParams& bvp = bvd->getParams();
			bool bvy = bvp.willYield(escape?YIELD_TYPE_ESCAPE:YIELD_TYPE_CONNECTION);
			if (checkNosingFeasibility(p,av, bv, p.dis2stop) &&
					(bve|| bvy) ) {

			p.setFlag(FLAG_NOSING_FEASIBLE);

			// Nosing is feasible

			if (bv->exists()) {

			  // There is a lag vehicle in the target lane

//			  bv->yieldVehicle_ = this;
			  bvd->yieldVehicle = p.driver;
			  if (!(bvd->isBus() && bvp.getStatus(STATUS_STOPPED))) {
				bvp.cftimer = std::min<double>(p.cftimer, bvp.cftimer);
			  }

			  if (!bvp.flag(FLAG_YIELDING)) {
				bvp.yieldTime = p.now;
			  }
			  if (p.getStatus(STATUS_LEFT)) {
				p.setFlag(FLAG_NOSING_LEFT);
				bvp.setFlag(FLAG_YIELDING_RIGHT);
			  } else {
				p.setFlag(FLAG_NOSING_RIGHT);
				bvp.setFlag(FLAG_YIELDING_LEFT);
			  }//end of STATUS_LEFT

			}//end if  bv->exists()
			else {

			  // No lag vehicle in the target lane

			  if (p.getStatus(STATUS_LEFT)) {
				p.setFlag(FLAG_NOSING_LEFT);
			  } else {
				p.setFlag(FLAG_NOSING_RIGHT);
			  }
			}//end of else bv->exists()

			// Check if the minimum gaps are available.

			if (bheadway > p.lcMinGap(lctype + 1) &&
				aheadway > p.lcMinGap(lctype)) {
//			  goto execution;
				executionLC(changeMode);
			}

		  }//end of if checkNosingFeasibility
		  else
		  {
			  p.unsetFlag(FLAG_NOSING_FEASIBLE);

				// Nosing is not feasible, but maintain the nosing state

				if (p.getStatus(STATUS_LEFT)) {
				  p.setFlag(FLAG_NOSING_LEFT);
				} else {
				  p.setFlag(FLAG_NOSING_RIGHT);
				}
		  }// end else if checkNosingFeasibility
		}//if nosing
		return 0.0;

	}//end of p.flag(FLAG_LC_FAILED)



	return executionLC(changeMode);
//	// execution:
//	if (changeMode != LCS_SAME) {
//		const int lane_shift_velocity = 350; //TODO: What is our lane changing velocity? Just entering this for now...
//		return changeMode == LCS_LEFT ? lane_shift_velocity : -lane_shift_velocity;
//	}
}
int MITSIM_LC_Model::checkNosingFeasibility(DriverUpdateParams& p,const NearestVehicle * av,const NearestVehicle * bv,double dis2stop)
{
	if (p.flag(FLAG_STUCK_AT_END)) {
		if ( timeSinceTagged(p) > lcMaxStuckTime )
		{
			// If one stuck for a very long time, skip the feasibility check
		    return 1;
		}
	}
	else
	{
		double length = p.driver->getVehicle()->getLengthCm()/100.0;// vh length
		if (dis2stop < length && p.currSpeed < Math::DOUBLE_EPSILON)
		{
			p.setFlag(FLAG_STUCK_AT_END);
		}
	}

	// Constraints of acceleration rate in response to lead and lag
	// vehicles

	float lower = -Math::FLT_INF;
	float upper = Math::FLT_INF;

	if (av->exists()) {

		Driver *avDriver = const_cast<Driver*>(av->driver);
		DriverUpdateParams& avp = avDriver->getParams();
		if ((avp.flag(FLAG_NOSING) || avp.flag(FLAG_YIELDING)) &&
			av->distance < 2.0 * p.lcMinGap(2)) {

		  // The lead vehicle is yeilding or nosing
		  return 0;		// To avoid dead lock

		}
		else if (p.flag(FLAG_LC_FAILED_LEAD)) {

		  // Acceleration rate in order to be slower than the leader

		  upper = (av->driver->fwdVelocity.get()/100.0 - p.currSpeed) /
			lcNosingConstStateTime +
			av->driver->fwdAccel.get()/100.0;

		  if (upper < p.maxDeceleration) {
			return 0;		// This vehicle is too fast
		  }
		} else if (av->driver->fwdVelocity/100.0 < Math::DOUBLE_EPSILON &&
				   p.dis2stop > p.distanceToNormalStop &&
				   p.nvFwd.distance > p.distanceToNormalStop) {
		  return 0;
		}//end if FLAG_LC_FAILED_LEAD
	}//end if av

	if (bv->exists()) {

		Driver *bvDriver = const_cast<Driver*>(av->driver);
		DriverUpdateParams& bvp = bvDriver->getParams();

		if (p.driver->getVehicle()->getVehicleType() == VehicleBase::BUS && p.flag(FLAG_NOSING)) {

			// MITSIM: Dan: bus must force it's way along the route, and
			// other vehicles will generally yield;
			// Acceleration rate in order to be faster than the lag
			// vehicle and do not cause the lag vehicle to decelerate
			// harder than its normal deceleration rate

			lower = (bv->driver->fwdVelocity/100.0- p.currSpeed) /
					lcNosingConstStateTime +
					p.normalDeceleration;

			if (lower > p.maxAcceleration) {	// I am will to acc hard

				// This vehicle is too slow or close to the lag vehicle

				return 0;
			}

		} //end of bus
		else if (bvp.flag(FLAG_NOSING) ||
				(bvp.flag(FLAG_YIELDING) &&
				bv->driver->yieldVehicle != p.driver &&
				bv->distance < 2.0 * p.lcMinGap(3) )) {

			// The lag vehicle is nosing or yielding to another vehicle or
			// not willing to yield

			return 0;		// To avoid dead lock
		} else if (!Utils::brandom(lcYieldingProb[p.flag(FLAG_YIELDING) ? 1 : 0]) ) {

			// The lag vehicle is not willing to yield

			return 0;		// Skip in this iteration

		} else if (p.flag(FLAG_LC_FAILED_LAG)) {

			// Acceleration rate in order to be faster than the lag
			// vehicle and do not cause the lag vehicle to decelerate
			// harder than its normal deceleration rate

			lower = (bv->driver->fwdVelocity/100.0 - p.currSpeed) /
					lcNosingConstStateTime +
					p.normalDeceleration;

			if (lower > p.maxAcceleration) {	// I am will to acc hard

				// This vehicle is too slow or close to the lag vehicle

				return 0;
			}
		}//end if FLAG_LC_FAILED_LAG
	}//end of if bv

	// Check if there exists a feasible acceleration rate for this
	// vehicle

	if (lower > upper) return 0;
		else return 1;
}
//double MITSIM_LC_Model::lcMinGap(int type)
//{
//	std::vector<double> b = LC_GAP_MODELS[type];
//	return b[2] * b[0];
//}
float MITSIM_LC_Model::lcNosingProb(float dis, float lead_rel_spd, float gap,int num)
{
  if (num < 0) num = - num;
  else if (num == 0) num = 1;

  std::vector<double> b = kaziNosingParams;
  float rel_spd = (lead_rel_spd > 0) ? 0 : lead_rel_spd ;
  float rm_dist_impact = 10 - 10 / (1.0 + exp(b[2] * dis)) ;
  if (gap > 100.0) {			// 100 meters
	gap = 100.0 ;
  }
  float u = b[0] + b[1] * rel_spd + b[3] * rm_dist_impact + b[4] * gap +
	b[5] * (num - 1);
  float p = 1.0 / (1 + exp(-u)) ;
  return p ;
}
double sim_mob::MITSIM_LC_Model::executionLC(LANE_CHANGE_SIDE& change)
{
	if (change != LCS_SAME) {
		const int lane_shift_velocity = 350; //TODO: What is our lane changing velocity? Just entering this for now...
		return change == LCS_LEFT ? lane_shift_velocity : -lane_shift_velocity;
	}
	return 0.0;
}
double sim_mob::MITSIM_LC_Model::timeSinceTagged(DriverUpdateParams& p)
{
	double currentTime = p.now.ms();
	double t = currentTime = p.lcTimeTag;
	return t;
}
bool sim_mob::MITSIM_LC_Model::path(DriverUpdateParams& p)
{
	// as current vehicle always has path
	return true;
}
bool sim_mob::MITSIM_LC_Model::checkIfLookAheadEvents(DriverUpdateParams& p)
{
	// TODO: check event ,like incident

	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	driverMvt->incidentPerformer.checkIncidentStatus(p, p.driver->getParams().now);

	if(driverMvt->incidentPerformer.getIncidentStatus().getChangedLane())
	{
		return true;
	}

	return false;
}
LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::checkMandatoryEventLC(DriverUpdateParams& p)
{
	LANE_CHANGE_SIDE lcs = LCS_SAME;
	// TODO: handle event ,like incident
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	if(driverMvt->incidentPerformer.getIncidentStatus().getChangedLane() &&
			driverMvt->incidentPerformer.getIncidentStatus().getNextLaneIndex()>=0){
				//p.nextLaneIndex = p.incidentPerformer.getIncidentStatus().getNextLaneIndex();
				lcs =driverMvt->incidentPerformer.getIncidentStatus().getLaneSide();
			}
//			else if( (p.incidentPerformer.getIncidentStatus().getCurrentStatus()==IncidentStatus::INCIDENT_ADJACENT_LANE && p.lastChangeMode==MLC )
//					|| (p.incidentPerformer.getIncidentStatus().getCurrentStatus()==IncidentStatus::INCIDENT_CLEARANCE && p.incidentPerformer.getIncidentStatus().getCurrentIncidentLength()>0)) {
//				p.nextLaneIndex = p.currLaneIndex;
////				parentDriver->vehicle->setTurningDirection(LCS_SAME);
//				lcs
//			}
	return lcs;
}
void sim_mob::MITSIM_LC_Model::checkConnectLanes(DriverUpdateParams& p)
{
	// reset status
	p.unsetStatus(STATUS_LEFT_OK); p.unsetStatus(STATUS_RIGHT_OK); p.unsetStatus(STATUS_CURRENT_OK);
	// check current lane has connector to next link
//	if(p.dis2stop<distanceCheckToChangeLane) // <150m need check above, ready to change lane
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	const RoadSegment* nextSegment = driverMvt->fwdDriverMovement.getNextSegment(false);
	const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (driverMvt->fwdDriverMovement.getCurrSegment()->getEnd());
	if(currEndNode)
	{
		// get lane connector
		const std::set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(driverMvt->fwdDriverMovement.getCurrSegment());

		// check lef,right lanes connect to next target segment
		for (std::set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++)
		{
			if ( (*it)->getLaneTo()->getRoadSegment() == nextSegment ) // this lc connect to target segment
			{
				int laneIdx = getLaneIndex((*it)->getLaneFrom());
				// lane index 0 start from most left lane of the segment
				// so lower number in the left, higher number in the right
				if(laneIdx > p.currLaneIndex)
				{
					p.setStatus(STATUS_LEFT_OK);
				}
				else if(laneIdx < p.currLaneIndex)
				{
					p.setStatus(STATUS_RIGHT_OK);
				}
				else if(laneIdx == p.currLaneIndex)
				{
					p.setStatus(STATUS_CURRENT_OK);
				}
			}// end if = nextsegment
		}//end for
	}// end if node

}
/*
 * In MITSIMLab, vehicle change lane in 1 time step.
 * While in sim mobility, vehicle approach the target lane in limited speed.
 * So the behavior when changing lane is quite different from MITSIMLab.
 * New behavior and model need to be discussed.
 * Here is just a simple model, which now can function.
 *
 * -wangxy
 * */
double sim_mob::MITSIM_LC_Model::executeLaneChanging(DriverUpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir, LANE_CHANGE_MODE mode) {
    //Behavior changes depending on whether or not we're actually changing lanes.
    {
        //1.If too close to node, don't do lane changing, distance should be larger than 3m
        if (p.dis2stop <= 3) {
            return 0.0;
        }

        //2.Get a random number, use it to determine if we're making a discretionary or a mandatory lane change
        boost::uniform_int<> zero_to_max(0, RAND_MAX);
        double randNum = (double) (zero_to_max(p.gen) % 1000) / 1000;
        double mandCheck = checkIfMandatory(p);
        LANE_CHANGE_MODE changeMode = mode; //DLC or MLC

        if (changeMode == DLC) {
            if (randNum < mandCheck) {
                changeMode = MLC;
            } else {
                changeMode = DLC;
                p.dis2stop = 1000; //MAX_NUM;		//no crucial point ahead
            }

            if (p.isMLC == true) {
                changeMode = MLC;
            }
        }


        //3.make decision depending on current lane changing mode
        LANE_CHANGE_SIDE decision = LCS_SAME;
        if (changeMode == DLC) {
            decision = makeDiscretionaryLaneChangingDecision(p);
        } else {
            decision = makeMandatoryLaneChangingDecision(p);
        }

        //4.Finally, if we've decided to change lanes, set our intention.
        if (decision != LCS_SAME) {
            const int lane_shift_velocity = 350; //TODO: What is our lane changing velocity? Just entering this for now...
            return decision == LCS_LEFT ? lane_shift_velocity : -lane_shift_velocity;
        }

        // remember last tick change mode
        p.lastChangeMode = changeMode;
        p.lastDecision = decision;
        return 0.0;
    }
}
