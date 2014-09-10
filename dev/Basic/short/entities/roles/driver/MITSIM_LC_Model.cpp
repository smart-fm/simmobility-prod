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
#include "entities/roles/driver/models/LaneChangeModel.hpp"
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
double sim_mob::MITSIM_LC_Model::calcSideLaneUtility(DriverUpdateParams& p, bool isLeft) {
    if (isLeft && !p.leftLane) {
        return -MAX_NUM; //has no left side
    } else if (!isLeft && !p.rightLane) {
        return -MAX_NUM; //has no right side
    }
    return (isLeft) ? p.nvLeftFwd.distance : p.nvRightFwd.distance;
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
                otherDistance[i].lead = fwd->distance/100.0;
                otherAcc[i].lead = fwd->driver->fwdAccel.get();
            }

            if (!back->exists()) {//no vehicle behind
                otherSpeed[i].lag = -5000;
                otherDistance[i].lag = 5000;
                otherAcc[i].lag = 5000;
            } else { //has vehicle behind, check the gap
                otherSpeed[i].lag = back->driver->fwdVelocity.get();
                otherDistance[i].lag = back->distance/100.0;
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
void sim_mob::MITSIM_LC_Model::chooseTargetGap(DriverUpdateParams& p)
{
	// 1.0 SET/UNSET choose adjacent,forward,backward STATUS
	p.unsetStatus(STATUS_TARGET_GAP);

	// 2.0 if doing nosing ,just return
	if (p.flag(FLAG_NOSING) || p.flag(FLAG_NOSING_FEASIBLE) || p.flag(FLAG_STUCK_AT_END)) return;

	// 3.0 check lane change decision direction
	LANE_CHANGE_SIDE changeMode = LCS_SAME;
	if (p.getStatus(STATUS_LEFT)) {
		changeMode = LCS_LEFT;
	} else if (p.getStatus(STATUS_RIGHT)) {
		changeMode = LCS_RIGHT;
	} else {
		return ;			// No request for lane change,just return
	}

	// TODO: need check incident of current lane? MITSIM has "if (isInIncidentArea(plane)) return;"

	// 4.0 get lead,lag vh
	// FRONT, LEAD AND LAG VEHICLES (do not have to be in same segment)
	// TODO: check aura mgr function inn driverfacets
	const NearestVehicle * av = NULL; // side leader vh
	const NearestVehicle * bv = NULL; // side follower vh
	const NearestVehicle * front = &p.nvFwd;
	const NearestVehicle * aav = NULL; //side forward of forward vh
	const NearestVehicle * bbv = NULL; //side backward of backward vh
	if(changeMode == LCS_LEFT) {
		av = &p.nvLeftFwd;
		bv = &p.nvLeftBack;
		aav = &p.nvLeftFwd2;
		bbv = &p.nvLeftBack2;
	}
	else {
		av = &p.nvRightFwd;
		bv = &p.nvRightBack;
		aav = &p.nvRightFwd2;
		bbv = &p.nvRightBack2;
	}

	// 5.0 easy set
	// if no left/right ahead vehicle, just use adjacent gap
	if (!av->exists()) {
	    p.setStatus(STATUS_ADJACENT);
	    return;
	}
	if (!bv->exists()) {
		p.setStatus(STATUS_ADJACENT);
		return;
	}

	Driver *avDriver = const_cast<Driver*>(av->driver);
	DriverMovement *avDriverMvt = (DriverMovement*)avDriver->Movement();
	Driver *bvDriver = const_cast<Driver*>(bv->driver);
	DriverMovement *bvDriverMvt = (DriverMovement*)bvDriver->Movement();
	Driver *frontDriver = NULL;
	if(front->exists())
	{
		frontDriver = const_cast<Driver*>(front->driver);
	}
	Driver *bbvDriver = NULL;
	if(bbv->exists())
	{
		bbvDriver = const_cast<Driver*>(bbv->driver);
	}

	// 6.0 calculate FORWARD GAP utility value
	float dis2gap = 0.0;
	float effectiveGap = 0.0;
	float remainderGap = 0.0;
	float gapSpeed = 0.0;
	float dis2front = 0.0;

	float d1, s1, d2, d3;
//	  if ( av->vehicleAhead() ) {
	if(aav->exists()) { // if has forward of forward vh
//	    d1 = av->gapDistance();
		d1 = aav->distance/100.0 - av->distance/100.0 - av->driver->getVehicleLengthM(); // get gap length of av and aav
//	    s1 = av->currentSpeed() - av-> vehicleAhead()->currentSpeed();
		Driver *aavDriver = const_cast<Driver*>(aav->driver);
		s1 = av->driver->fwdVelocity/100.0 - aavDriver->getFwdVelocityM(); // speed diff of av and aav
	  } else {
//		  d1 = av->distance/100.0();
		  // get side ahead vh distance to end of link
		  // tmp solution distance to next segment ,if next seg exist
		  // TODO: meaning of av->distance()
		  if(avDriverMvt->fwdDriverMovement.getNextSegment(true))
		  {
			  d1 = avDriverMvt->fwdDriverMovement.getDisToCurrSegEndM() + avDriverMvt->fwdDriverMovement.getNextSegment(true)->length/100.0;
		  }
		  else {
			  d1 = avDriverMvt->fwdDriverMovement.getDisToCurrSegEndM();
		  }

		  s1 = 0;
//	    if (av->nextLane_) {
//	      d1 = d1 + av->nextLane_->length();
//	    }
	  }// end of aav->exists()

//	  dis2gap = this->gapDistance(av)+ av->length();
	    dis2gap  = av->distance/100.0;


//	  if (!front) {
	if(!front->exists()) {
		effectiveGap = d1;
		remainderGap = 0;
		gapSpeed = s1;
	} // end if front
	else {
//	    dis2front = this->gapDistance(front) + front->length();
		dis2front = front->distance/100.0;
//		Driver *avDriver = const_cast<Driver*>(av->driver);
//		Driver *frontDriver = const_cast<Driver*>(front->driver);
	    if (dis2gap > dis2front)
	    {
//	    	effectiveGap = (-1) * (front->gapDistance(av) + av->length() + front->length());
	    	effectiveGap = (-1) * (frontDriver->gapDistance(avDriver) + av->driver->getVehicleLengthM() + front->driver->getVehicleLengthM());
	    	remainderGap = d1;
	    	gapSpeed = avDriver->getFwdVelocityM() - frontDriver->getFwdVelocityM();
	    } // end if dis2gap > dis2front
	    else {
//	    	d2 = av->gapDistance(front);
	    	d2 = avDriver->gapDistance(frontDriver);
	    	if (d1 >= d2) {
				effectiveGap =  d2;
				remainderGap = d1-d2;
//				gapSpeed = av->currentSpeed() - front->currentSpeed();
				gapSpeed = avDriver->getFwdVelocityM() - frontDriver->getFwdVelocityM();
	    	} // end if d1 >= d2
	    	else {
				effectiveGap =  d1;
				remainderGap = 0;
				gapSpeed = s1;
	    	}// end else d1 >= d2
	    }//end else dis2gap > dis2front
	  }// end else front exist

	double eufwd = gapExpOfUtility(p,1, effectiveGap, dis2gap, gapSpeed, remainderGap);

	// 7.0 ADJACENT GAP

//	d1 = bv->gapDistance();
	// adjacent gap length
	d1 = av->distance/100.0 + bv->distance/100.0;
//	s1 = bv->currentSpeed() - av->currentSpeed();
	s1 = bvDriver->getFwdVelocityM() - avDriver->getFwdVelocityM();

	dis2gap = 0;

//	if (!front) {
	if(!front->exists()) {
	  effectiveGap = d1;
	  remainderGap = 0;
	  gapSpeed = s1;
	} //end if front not exist
	else {
//		d2 = bv->gapDistance(front);
		d2 = bvDriver->gapDistance(frontDriver);
		if (d1 > dis2front)
		{
			effectiveGap = d2;
			remainderGap = d1-d2;
//			gapSpeed = bv->currentSpeed() - front->currentSpeed();
			gapSpeed = bvDriver->getFwdVelocityM() - frontDriver->getFwdVelocityM();
		}// end if d1 > dis2front
		else {
			effectiveGap =  d1;
			remainderGap = 0;
			gapSpeed = s1;
		}// end if else d1 > dis2front
	}//end else front not exist


	double euadj = gapExpOfUtility(p,3, effectiveGap, dis2gap, gapSpeed, remainderGap);

	  // 8.0 BACKWARD GAP

//	  if ( TS_Vehicle* bv2 = bv->vehicleBehind() ) {
	if(bbv->exists()) {
//	    d1 = bv2->gapDistance();
		d1 = bbv->distance/100.0 - bv->distance/100.0 - bv->driver->getVehicleLengthM(); // get gap length of bv and bbv
//	    s1 = bv2->currentSpeed() - bv->currentSpeed();
		s1 = bbvDriver->getFwdVelocityM() - bvDriver->getFwdVelocityM();
	} else {

	    s1 = 0;
//	    d1 = bv->lane()->length() - bv->distance();
	    // get side back vh distance to start of link
	    // tmp solution distance to move along segment ,if next seg exist
	    // TODO: meaning of d1 = bv->lane()->length() - bv->distance();
	    d1 = bvDriverMvt->fwdDriverMovement.getCurrDistAlongPolylineM();

	    // TODO: why use upstream lanes length
//	    int i, n = bv->lane_->nUpLanes();//pointers to upstream lanes
//	    if (n > 0) {
//	      d2 = FLT_INF;
//	      for (i = 0; i < n; i ++) {
//		if (d3 = bv->lane_->upLane(i)->length() < d2) {
//		  d2 = d3;
//		}
//	      }
//	      d1 = d1 +d2;
//	    }
	  }//end else

//	  dis2gap = bvDriver->gapDistance(this)+ this->length();
	dis2gap = bv->distance/100.0;

	effectiveGap = d1;
	remainderGap = 0;
	gapSpeed = s1;


	double eubck = gapExpOfUtility(p,2, effectiveGap, dis2gap, gapSpeed, remainderGap);

	double sum = eufwd + eubck + euadj ;
//	double rnd = theRandomizer->urandom() ;
	double rnd = Utils::uRandom();
	if (rnd < euadj / sum) p.setStatus(STATUS_ADJACENT);
	else if (rnd < (euadj + eubck) / sum) p.setStatus(STATUS_BACKWARD);
	else p.setStatus(STATUS_FORWARD);

	return;
}
double sim_mob::MITSIM_LC_Model::gapExpOfUtility(DriverUpdateParams& p,int n, float effGap, float gSpeed, float gapDis, float remDis)
{


  std::vector<double> a = p.targetGapParams;

  double u = a[2] * effGap + a[3] * gSpeed + a[4] * gapDis;

  if (remDis > 0.1) {
	u += a[5];
  }

  if (n == 1) u += a[0];
  else if (n==2) u += a[1];


  return exp(u) ;
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

	ParameterManager::Instance()->param(modelName,"MLC_Yielding_Probabilities",str,
					string("0.13 0.71  0.13  0.03"));
	makelcYieldingProb(str);

	// kazi nosing param
	ParameterManager::Instance()->param(modelName,"kazi_nosing_param",str,
					string("-3.159  0.313  -0.027  2.050  0.028  0.6"));
	makekaziNosingParams(str);

	//CF_CRITICAL_TIMER_RATIO
	ParameterManager::Instance()->param(modelName,"CF_CRITICAL_TIMER_RATIO",CF_CRITICAL_TIMER_RATIO,0.5);

	//LC Yielding Model
	ParameterManager::Instance()->param(modelName,"LC_Yielding_Model",str,string("0.80 1.0"));

	//minTimeInLaneSameDir
	ParameterManager::Instance()->param(modelName,"LC_Discretionary_Lane_Change_Model_MinTimeInLaneSameDir",minTimeInLaneSameDir,2.0);
	//minTimeInLaneDiffDir
	ParameterManager::Instance()->param(modelName,"LC_Discretionary_Lane_Change_Model_MinTimeInLaneDiffDir",minTimeInLaneDiffDir,2.0);

	//Target Gap Model
	ParameterManager::Instance()->param(modelName,"Target_Gap_Model",str,
						string("-0.837   0.913  0.816  -1.218  -2.393  -1.662"));
	sim_mob::Utils::convertStringToArray(str,p.targetGapParams);
}
void sim_mob::MITSIM_LC_Model::makeMCLParam(std::string& str)
{
	std::vector<double> array;
	sim_mob::Utils::convertStringToArray(str,array);
	MLC_PARAMETERS.lowbound = array[0];
	MLC_PARAMETERS.delta = array[1];
	MLC_PARAMETERS.lane_mintime = array[2];

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
//	if(p.parentId == 888)
//		{
//			return LCS_SAME;
//		}
	LANE_CHANGE_SIDE change = LCS_SAME;

	// get distance to end of current segment
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	float x=driverMvt->fwdDriverMovement.getDisToCurrSegEndM();
	// distance to fwd vh
	if(p.nvFwd.exists()){
		if(x>p.nvFwd.distance/100.0){
			x = p.nvFwd.distance/100.0;
		}
	}

	// if current segment has enough distance to do lc , keep current lane
//	if ( x>=lookAheadDistance )
//	{
//		return change;
//	}

	// if already in changing lane
	if ( p.flag(FLAG_ESCAPE) )
	{
		if (p.statusMgr.getStatus(STATUS_LEFT_SIDE_OK) ) {
		  change = LCS_LEFT;
		}
	    else if (p.statusMgr.getStatus(STATUS_RIGHT_SIDE_OK) ) {
		  change = LCS_RIGHT;
		}

//		if (p.flag(FLAG_ESCAPE_LEFT)) {
//		  change = LCS_LEFT;
//		}
//		if (p.flag(FLAG_ESCAPE_RIGHT)) {
//		  change = LCS_RIGHT;
//		}
		p.setStatus(STATUS_MANDATORY);
		return change;
	}

	if(p.parentId == 1 && p.now.frame()>17)
	{
		int i=0;
	}
	// find lanes connect to target segment in lookahead distance
	driverMvt->fwdDriverMovement.getNextSegment(true);
	std::vector<sim_mob::Lane*> connectedLanes;
	std::cout<<std::endl;
	std::cout<<"tick: "<<p.now.frame()<<std::endl;
	std::cout<<"carid: "<<p.parentId<<std::endl;
	driverMvt->getLanesConnectToLookAheadDis(lookAheadDistance,connectedLanes);

	int nRight = 100; // number of lane changes required for the current lane.
	int nLeft = 100; // number of lane changes required for the current lane.
	int nCurrent = 100; // number of lane changes required for the current lane.


	// computer number of lane changes to right,left,current
	for (int i = 0; i < connectedLanes.size(); i++) {

		int l1 = abs(getLaneIndex(connectedLanes[i]));
		int l2 = p.currLaneIndex;
		int numlcRight  = l1 -  (l2-1) ;
		int numlcLeft  = l1 -  (l2+1) ;
		int numlcCurrent  = l1 -  l2;

	 nRight =  std::min<int>(nRight, numlcRight);
	 nLeft =  std::min<int>(nLeft, numlcLeft);
	 nCurrent =  std::min<int>(nCurrent, numlcCurrent);

	 if(nRight<0) nRight=0;
	 if(nLeft<0) nLeft=0;
	 if(nCurrent<0) nCurrent=0;
	}

	double eul = 0.0, eur = 0.0, euc = 1.0 ;
	double lcDistance = p.dis2stop;


	int res = isReadyForNextDLC(p,2);
	if( (res || nCurrent>0) && p.leftLane ) {
		eul = lcUtilityLookAheadLeft(p, nLeft, lcDistance);
	}

	res = isReadyForNextDLC(p,1);
	if( (res || nCurrent>0) && p.rightLane ) {
		eur = lcUtilityLookAheadRight(p, nRight, lcDistance);
	}


//	for(int i=0;i<connectedLanes.size();i++)
//	{
//		size_t goodLaneIdx = getLaneIndex(connectedLanes[i]);
////		if(isReadyForNextDLC(p,2) && p.leftLane == connectedLanes[i])
//		int res = isReadyForNextDLC(p,2);
//		if(res && goodLaneIdx > p.currLaneIndex)
//		{
//			eul = lcUtilityLookAheadLeft(p, nLeft, lcDistance);
//		}
////		if(isReadyForNextDLC(p,1) && p.rightLane == connectedLanes[i])
//		res = isReadyForNextDLC(p,1);
//		if(res && goodLaneIdx < p.currLaneIndex)
//		{
//			eur = lcUtilityLookAheadRight(p, nRight, lcDistance);
//		}
//		if(p.currLane == connectedLanes[i])
//		{
//			euc = lcUtilityLookAheadCurrent(p, nRight, lcDistance);
//		}
//	}

	double sum = eul + eur ;
	if(sum > 0)
	{
		euc = lcUtilityLookAheadCurrent(p, nCurrent, lcDistance);
	}
	else
	{
		p.utilityCurrent = 1.0;
		p.lcd = "lcd-c";
		return LCS_SAME;
	}

	if(p.parentId == 1 && p.now.frame()>17)
		{
			int i=0;
		}
	sum += euc;

//	if(euc> eul && euc>eur)
//	{
//		change = LCS_SAME;
//	}
//	if(eur> euc && eur>eul)
//	{
//		change = LCS_RIGHT;
//	}
//	if(eul> euc && eul>eur)
//	{
//		change = LCS_LEFT;
//	}

//	boost::uniform_int<> zero_to_max(0, RAND_MAX);
//	double rnd = (double) (zero_to_max(p.gen) % 1000) / 1000;
	double rnd = Utils::uRandom();
	p.rnd = rnd;
//	double rnd = Utils::generateFloat(0,0.8);
	float probOfCurrentLane = euc / sum;
	float probOfCL_LL = probOfCurrentLane + eul / sum;

	p.utilityCurrent = euc / sum ;
	p.utilityLeft = eul / sum ;
	p.utilityRight = eur /sum ;

	if (rnd < probOfCurrentLane){
		change = LCS_SAME ;
		p.lcd = "lcd-c";
	}
	else if (rnd < probOfCL_LL){
		p.lcd = "lcd-l";
		change = LCS_LEFT ;
	}
	else {
		p.lcd = "lcd-r";
		change = LCS_RIGHT ;
	}



	return change;
}
int sim_mob::MITSIM_LC_Model::isReadyForNextDLC(DriverUpdateParams& p,int mode)
{
  float sec = timeSinceTagged(p);

  switch(mode) {
  case 1:			// request a change to the right
	{
	  if (p.flag(FLAG_PREV_LC_RIGHT) && // same direction
		  sec > getDlcMinTimeInLaneSameDir()) {
		return 1;
	  } else if (sec > getDlcMinTimeInLaneDiffDir()) {
		return 1;
	  } else {
		return 0;
	  }
	}
  case 2:			// request a change to the left
	{
	  if (p.flag(FLAG_PREV_LC_LEFT) && // same direction
		  sec > getDlcMinTimeInLaneSameDir()) {
		return 1;
	  } else if (sec > getDlcMinTimeInLaneDiffDir()) {
		return 1;
	  } else {
		return 0;
	  }
	}
  }
  return sec > getDlcMinTimeInLaneSameDir();
}
int sim_mob::MITSIM_LC_Model::isWrongLane(DriverUpdateParams& p,const Lane* lane)
{
	// by right shall use lane connector
	// as in the link, can use lane index

	int res = 0;

	// 1.0 get lane index
	size_t laneIdx = getLaneIndex(lane);
	// 2.0 get lane's segment's link
	RoadSegment* segment = lane->getRoadSegment();

	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	std::vector<const sim_mob::RoadSegment*>::iterator currentSegIt = driverMvt->fwdDriverMovement.currSegmentIt;
	std::vector<const sim_mob::RoadSegment*>::iterator currentSegItEnd = driverMvt->fwdDriverMovement.fullPath.end();

	for(;currentSegIt != currentSegItEnd;++currentSegIt)
	{
		const RoadSegment* rs = *currentSegIt;
		if(segment->getLink() == rs->getLink()) {
			// same link
			size_t segLaneSize = rs->getLanesSize(false);
			if(segLaneSize < laneIdx)
			{
				// need right lane change
				int n = segLaneSize - laneIdx;
				if(n<res) res = n;
			}
		}// end if segment->getLink()
	}// end for

	return res;
}
double sim_mob::MITSIM_LC_Model::LCUtilityCurrent(DriverUpdateParams& p)
{
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	// 1.0 lane utility parameters
	vector<double> a = laneUtilityParams;
	// 2.0 LEADING AND LAG VEHICLES
	const NearestVehicle * av = &p.nvFwd; // leader vh
	const NearestVehicle * bv = &p.nvBack; // follower vh
	// 3.0 count number of lane change to the end of link
	double vld, mlc, density, spacing;
	int n = abs(isWrongLane(p,p.currLane));
	// TODO calculate lane density
	density = 0;
	float heavy_neighbor = 0.0;
	if (av->exists()) {
		  vld = std::min<double>(p.desiredSpeed, av->driver->getFwdVelocityM()) ;
		  heavy_neighbor = (av->driver->getVehicle()->getVehicleType() != VehicleBase::BUS) ? 0.0 : a[7];
		  spacing = av->distance/100.0;
	}
	else {
		  vld = p.desiredSpeed;
		  spacing = p.dis2stop;
	}
	if(bv->exists())
	{
		heavy_neighbor = (bv->driver->getVehicle()->getVehicleType() != VehicleBase::BUS) ? heavy_neighbor : a[7];
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

	double u = a[0] + a[4] * vld + a[6] * spacing + a[8] * density + mlc + heavy_neighbor + right_most + a[5] * busAheadDummy;

	return exp(u) ;
}
double sim_mob::MITSIM_LC_Model::LCUtilityRight(DriverUpdateParams& p)
{
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	// 1.0 lane utility parameters
	vector<double> a = laneUtilityParams;
	// 2.0 LEADING AND LAG VEHICLES
	const NearestVehicle * av = &p.nvRightFwd; // right leader vh
	const NearestVehicle * bv = &p.nvRightBack; // right follower vh
	// 3.0 count number of lane change to the end of link
	double vld, mlc, density, spacing;
	int n = abs(isWrongLane(p,p.currLane));
	// TODO calculate lane density
	density = 0;
	float heavy_neighbor = 0.0;
	if (av->exists()) {
		  vld = std::min<double>(p.desiredSpeed, av->driver->getFwdVelocityM()) ;
		  heavy_neighbor = (av->driver->getVehicle()->getVehicleType() != VehicleBase::BUS) ? 0.0 : a[7];
		  spacing = av->distance/100.0;
	}
	else {
	      vld = p.desiredSpeed;
	      spacing = p.dis2stop;
	}

	if(bv->exists())
	{
		heavy_neighbor = (bv->driver->getVehicle()->getVehicleType() != VehicleBase::BUS) ? heavy_neighbor : a[7];
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

	double u = a[1] + a[4] * vld + a[6] * spacing + a[8] * density + mlc + heavy_neighbor + right_most + a[5] * busAheadDummy;

	return exp(u) ;
}
double sim_mob::MITSIM_LC_Model::LCUtilityLeft(DriverUpdateParams& p)
{
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	// 1.0 lane utility parameters
	vector<double> a = laneUtilityParams;
	// 2.0 LEADING AND LAG VEHICLES
	const NearestVehicle * av = &p.nvLeftFwd; // left leader vh
	const NearestVehicle * bv = &p.nvLeftBack; // left follower vh
	// 3.0 count number of lane change to the end of link
	double vld, mlc, density, spacing;
	int n = abs(isWrongLane(p,p.currLane));
	// TODO calculate lane density
	density = 0;
	float heavy_neighbor = 0.0;
	if (av->exists()) {
	      spacing = av->distance/100.0;
	      vld = std::min<double>(p.desiredSpeed, av->driver->getFwdVelocityM()) ;
	      heavy_neighbor = (av->driver->getVehicle()->getVehicleType() != VehicleBase::BUS) ? 0.0 : a[7];
	}
	else {
	      vld = p.desiredSpeed;
	      spacing = p.dis2stop;
//		if (nextLane_) {
//		  spacing += nextLane_->length();
//		}
	}//end if av->exists()

	float left_most = 0.0;

	if(bv->exists())
	{
		heavy_neighbor = (bv->driver->getVehicle()->getVehicleType() != VehicleBase::BUS) ? heavy_neighbor : a[7];
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

	int busAheadDummy = 0;
	if(p.nvLeftFwd.exists())
	{
		if(p.nvLeftFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)
		{
			busAheadDummy = 1;
		}
	}

	double u = a[4] * vld + a[6] * spacing + a[8] * density + mlc + heavy_neighbor + left_most + a[5] * busAheadDummy;

	return exp(u) ;

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
		double leftFwdVel = p.nvLeftFwd.driver->fwdVelocity.get()/100.0;
		double currentSpeed = p.perceivedFwdVelocity / 100.0;
		vld = std::min<double>(leftFwdVel,currentSpeed);

		if(p.nvLeftFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)// get vh type, heavy vh only bus now
		{
			heavy_neighbor = a[7];
		}
		spacing = p.nvLeftFwd.distance/100.0;
	}
	else
	{
		vld =  p.desiredSpeed;
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

	double u = a[4] * vld + a[6] * spacing + a[8] * density + mlc + heavy_neighbor + left_most + a[5] * busAheadDummy;
	double res = exp(u);
	return  res;
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
		double leftFwdVel = p.nvRightFwd.driver->fwdVelocity.get()/100.0;
		double currentSpeed = p.perceivedFwdVelocity / 100.0;

		vld = std::min<double>(leftFwdVel,currentSpeed);

		if(p.nvRightFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)// get vh type, heavy vh only bus now
		{
			heavy_neighbor = a[7];
		}
		spacing = p.nvRightFwd.distance/100.0;

		//
		Driver* fwd_driver_ = const_cast<Driver*>(p.nvRightFwd.driver);
		int fwdcarid = fwd_driver_->getParent()->getId();
		int i=0;
		//
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

	double u = a[1] + a[4] * vld + a[6] * spacing +a[8] * density + mlc + heavy_neighbor + right_most + a[5] * busAheadDummy;
	double res = exp(u);
	return  res;
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
		double frontVhVel = p.perceivedFwdVelocityOfFwdCar/100.0;//p.nvFwd.driver->fwdVelocity.get()/100.0;
		double currentSpeed = p.perceivedFwdVelocity / 100.0;
		vld = std::min<double>(frontVhVel,currentSpeed);

		if(p.nvFwd.driver->getVehicle()->getVehicleType() == VehicleBase::BUS)// get vh type, heavy vh only bus now
		{
			heavy_neighbor = a[7];
		}
		spacing = p.perceivedDistToFwdCar/100.0;
	}
	else
	{
		vld =  p.desiredSpeed;
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
		double gap_behind = p.nvBack.distance/100.0;
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

	double u = a[0]+ a[4] * vld + a[6] * spacing + a[8] * density + mlc + heavy_neighbor + right_most + tailgate_dummy + a[5] * busAheadDummy;

	double res = exp(u) ;
	return res;
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
	double dis = MLC_PARAMETERS.lowbound + n*(MLC_PARAMETERS.delta - MLC_PARAMETERS.lowbound);
	return dis;
}
LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::makeLaneChangingDecision(DriverUpdateParams& p)
{
	// if in the middle of lc , just pass
	if(p.getStatus(STATUS_LC_CHANGING)){
		if(p.getStatus(STATUS_LC_LEFT)) {
			return LCS_LEFT;
		}
		else if(p.getStatus(STATUS_LC_RIGHT)){
			return LCS_RIGHT;
		}
		else {
			// error, in mid of changing lane, but status is lcs_same
			throw std::runtime_error("makeLaneChangingDecision: error, in mid of changing lane, but status is lcs_same");
		}
	}//end getStatus()

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
		// reset status, TODO move to function
		p.setStatus(STATUS_LEFT_SIDE_OK,STATUS_UNKNOWN,string("makeLaneChangingDecision"));
		p.setStatus(STATUS_RIGHT_SIDE_OK,STATUS_UNKNOWN,string("makeLaneChangingDecision"));
		p.setStatus(STATUS_CURRENT_LANE_OK,STATUS_UNKNOWN,string("makeLaneChangingDecision"));
		p.utilityCurrent = 0 ;
		p.utilityLeft = 0 ;
		p.utilityRight = 0 ;
		p.rnd=0;
		// check lanes connect to next segment
		checkConnectLanes(p);
if(p.parentId == 1 && p.now.frame()>36)
{
	int i=0;
}
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
	// 1. unset FLAG_LC_FAILED, check whether can do lc
	p.unsetFlag(FLAG_LC_FAILED);

	LANE_CHANGE_SIDE changeMode = LCS_SAME;

	// 2.0 check decision
	if (p.getStatus(STATUS_LEFT)) {
		changeMode = LCS_LEFT;
	} else if (p.getStatus(STATUS_RIGHT)) {
		changeMode = LCS_RIGHT;
	} else {
		return 0.0;			// No request for lane change
	}

	// 3.0 get lead,lag vh
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

	// 4.0 get lead,lag vh distance
	// LEADING HEADWAY
	float aheadway;		// leading headway
	if (av->exists()) {
		aheadway = av->distance/100.0;
	} else {
		aheadway = Math::FLT_INF;
	}

	 // LAG HEADWAY

	float bheadway;		// lag headway
	if (bv->exists()) {
		bheadway = bv->distance/100.0;
	} else {
		bheadway = Math::FLT_INF;
	}

	// 5.0 check if lc decision from events
	// Is the lane change is triggered by special events
	int escape = p.flag(FLAG_ESCAPE);

	int lctype;
	if ( p.getStatus(STATUS_MANDATORY) &&
	   ( !p.getStatus(STATUS_CURRENT_LANE_OK) ) ) {
		lctype = 2;		// must do lane changing
	} else if (escape) {
		lctype = 2;		// special event
	} else {
		lctype = 0;		// discretionary
	}

	// 6.0 check if lead,lag gap ok
	if (bv->exists() && bheadway < lcCriticalGap(p,1, bv->driver->fwdVelocity/100.0 - p.currSpeed )) {
		p.setFlag(FLAG_LC_FAILED_LAG); // lag gap
	}
	else if (av->exists() && aheadway < lcCriticalGap(p,0, av->driver->fwdVelocity/100.0 - p.currSpeed)) {
		p.setFlag(FLAG_LC_FAILED_LEAD); // lead gap
	}

	// 7.0 if gap ok, then doing lane change
	if( !p.flag(FLAG_LC_FAILED) )
	{
		p.setStatusDoingLC(changeMode);
//		//set status to "doing lc"
//		if(changeMode==LCS_LEFT)
//		{
//			p.setStatus(STATUS_LC_LEFT);
//		}
//		else
//		{
//			p.setStatus(STATUS_LC_RIGHT);
//		}
//			return executionLC(changeMode);
	}

	// 8.0 CHECK IF THE GAPS ARE not ACCEPTABLE,then do nosing
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

			if ( p.getStatus(STATUS_CURRENT_LANE_OK) && p.nextLink() ) {
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
				//executionLC(changeMode);
				p.setStatusDoingLC(changeMode);
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


//	//TODO new status
//	if(changeMode==LCS_LEFT)
//	{
//		p.setStatus(STATUS_LC_LEFT);
//	}
//	else
//	{
//		p.setStatus(STATUS_LC_RIGHT);
//	}
//
//	return executionLC(changeMode);



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
			av->distance/100.0 < 2.0 * p.lcMinGap(2)) {

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
				   p.nvFwd.distance/100.0 > p.distanceToNormalStop) {
		  return 0;
		}//end if FLAG_LC_FAILED_LEAD
	}//end if av

	if (bv->exists()) {

		Driver *bvDriver = const_cast<Driver*>(bv->driver);
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
				bv->distance/100.0 < 2.0 * p.lcMinGap(3) )) {

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
double sim_mob::MITSIM_LC_Model::executeLaterVel(LANE_CHANGE_SIDE& change)
{
	if (change != LCS_SAME) {
		const int lane_shift_velocity = 5.50; //TODO: What is our lane changing velocity? Just entering this for now...
		return change == LCS_LEFT ? lane_shift_velocity : -lane_shift_velocity;
	}
	return 0.0;
}
double sim_mob::MITSIM_LC_Model::timeSinceTagged(DriverUpdateParams& p)
{
	double currentTime = p.now.ms();
	double t = (currentTime - p.lcTimeTag) / 1000.0;// convert ms to s
	return t;
}
bool sim_mob::MITSIM_LC_Model::path(DriverUpdateParams& p)
{
	// as current vehicle always has path
	return true;
}
int sim_mob::MITSIM_LC_Model::checkIfLookAheadEvents(DriverUpdateParams& p)
{
	// TODO: check event ,like incident

	p.unsetFlag(FLAG_ESCAPE | FLAG_AVOID);
	p.unsetStatus(STATUS_MANDATORY);
	p.dis2stop = DEFAULT_DIS_TO_STOP;//?
	// set default target lanes
	p.targetLanes.clear();
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	const std::vector<sim_mob::Lane*> lanes = driverMvt->fwdDriverMovement.getCurrSegment()->getLanes();
	for(int i=0;i<lanes.size();++i)
	{
		if(!lanes[i]->is_pedestrian_lane()) {
			p.targetLanes.insert(lanes[i]);
		}
	}

	bool needMLC = false;
	bool needDLC = false;
	// 1.0 check incident
	int res = isThereBadEventAhead(p);
	if(res == -1) needMLC = true;
	if(res == 1) needDLC = true;

	// 1.1 check lane drop
	// no incident,but need check lane drop
	set<const Lane*> laneDropTargetLanes;
	res = isThereLaneDrop(p,laneDropTargetLanes);
	if(res == -1) needMLC = true;
	if(res == 1) needDLC = true;
	p.addTargetLanes(laneDropTargetLanes);

	// 1.2 check lane connect to next segment
	set<const Lane*> laneConnectorTargetLanes;
	res = isLaneConnectToNextLink(p,laneConnectorTargetLanes);
	if(res == -1) needMLC = true;
	if(res == 1) needDLC = true;
	p.addTargetLanes(laneConnectorTargetLanes);

	// 2.0 set flag
	if (needMLC) {
		p.setFlag(FLAG_ESCAPE);
	} else if (needDLC) {
		p.setFlag(FLAG_AVOID);
	}
//	} else {
//		dis2stop_ = distanceFromDownNode();
//		vis_ = link()->length();
//	}

	// 3.0 if has mld require and not enough headway, set STATUS_MANDATORY
	 if ( needMLC  && p.dis2stop < lookAheadDistance ) {
//	    setMandatoryStatusTag();
		p.setStatus(STATUS_MANDATORY);
	 }

	 return p.getStatus(STATUS_MANDATORY);
}
int sim_mob::MITSIM_LC_Model::isThereBadEventAhead(DriverUpdateParams& p)
{
	// TODO set dis2stop

	// only mandatory lane change or no change
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	driverMvt->incidentPerformer.checkIncidentStatus(p, p.driver->getParams().now);

	if(driverMvt->incidentPerformer.getIncidentStatus().getChangedLane())
	{
		//HP: pls set p.dis2stop , it is distance to the incident.
		p.dis2stop = driverMvt->incidentPerformer.getIncidentStatus().getDistanceToIncident();
		return -1; //mandatory lane change
	}

	return 0;
}
int sim_mob::MITSIM_LC_Model::isThereLaneDrop(DriverUpdateParams& p,set<const Lane*>& targetLanes)
{
	std::string str = "isThereLaneDrop";
	// TODO use lane connector
	// but now use lane index

	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	if (!(driverMvt->hasNextSegment(true)))
	{
		// not has next segment in current link,means current on last segment of the link
		double d = driverMvt->fwdDriverMovement.getAllRestRoadSegmentsLengthCM() -
				driverMvt->fwdDriverMovement.getCurrDistAlongRoadSegmentCM() - driverMvt->parentDriver->vehicle->getLengthCm() / 2;
		d /= 100.0;
//		if (p.nvFwd.distance < p.dis2stop)
//			p.dis2stop = p.nvFwd.distance;
		if(d<p.dis2stop)
			p.dis2stop = d;

		//fill targetLanes
		const std::vector<sim_mob::Lane*> lanes = driverMvt->fwdDriverMovement.getCurrSegment()->getLanes();
		for(int i=0;i<lanes.size();++i) {
			if(!lanes[i]->is_pedestrian_lane()) {
				targetLanes.insert(lanes[i]);
			}
		}// end of for
	}
	else {
		//has next segment of current link
		// check current lane is most left lane and next segment lane size smaller than current lane index
		// means has lane drop, need lane change
		size_t currentSegmentLaneSize = driverMvt->fwdDriverMovement.getCurrSegment()->getLanes().size();
		if(driverMvt->fwdDriverMovement.getCurrSegment()->getLanes().at(currentSegmentLaneSize-1)->is_pedestrian_lane()) {
			// last lane of current segment is ped lane
			currentSegmentLaneSize--;
		}
		if(driverMvt->fwdDriverMovement.getCurrSegment()->getLanes().at(0)->is_pedestrian_lane()) {
			// first lane of current segment is ped lane
			currentSegmentLaneSize--;
		}
		// check next segment lane size
		size_t nextSegmentLaneSize = driverMvt->fwdDriverMovement.getNextSegment(true)->getLanes().size();
		if(driverMvt->fwdDriverMovement.getNextSegment(true)->getLanes().at(nextSegmentLaneSize-1)->is_pedestrian_lane())
		{
			// next segment has ped lane
			nextSegmentLaneSize--;
		}
		if(driverMvt->fwdDriverMovement.getNextSegment(true)->getLanes().at(0)->is_pedestrian_lane())
		{
			// next segment has ped lane
			nextSegmentLaneSize--;
		}
//		if(p.currLaneIndex == currentSegmentLaneSize)
		if(nextSegmentLaneSize < currentSegmentLaneSize )
		{
			// seems current segment's most left lane has lane drop
			double d = (driverMvt->fwdDriverMovement.getCurrPolylineTotalDistCM() -
					driverMvt->fwdDriverMovement.getCurrDistAlongRoadSegmentCM() )/100.0;
			if(d<p.dis2stop)
			{
				p.dis2stop = d;
			}
			// fill targetLanes
			for(int i=0;i<nextSegmentLaneSize;++i)
			{
				const Lane* l = driverMvt->fwdDriverMovement.getCurrSegment()->getLanes().at(i);
				if(!l->is_pedestrian_lane())
					targetLanes.insert(l);
			}

			if(nextSegmentLaneSize > p.currLaneIndex)//has lane drop
			{
				// we are on most left lane of current segment
				p.setStatus(STATUS_LEFT_SIDE_OK,STATUS_NO,str);
				//of course, current lane is not ok
				p.setStatus(STATUS_CURRENT_LANE_OK,STATUS_NO,str);
				return -1;
			}
		}
		else {
			//all lanes ok,fill targetLanes
			const std::vector<sim_mob::Lane*> lanes = driverMvt->fwdDriverMovement.getCurrSegment()->getLanes();
			for(int i=0;i<lanes.size();++i) {
				if(!lanes[i]->is_pedestrian_lane()) {
					targetLanes.insert(lanes[i]);
				}
			}// end of for
		}
	}// end else
	return 0;
}
int sim_mob::MITSIM_LC_Model::isLaneConnectToNextLink(DriverUpdateParams& p,set<const Lane*>& targetLanes)
{
	std::string str = "isLaneConnectToNextLink";
	int res=-1;
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	double dis = driverMvt->fwdDriverMovement.getAllRestRoadSegmentsLengthCM()
					- driverMvt->fwdDriverMovement.getCurrDistAlongRoadSegmentCM()
					- driverMvt->parentDriver->vehicle->getLengthCm() / 2 - 200;
	p.dis2stop = dis/100.0;
	const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (driverMvt->fwdDriverMovement.getCurrSegment()->getEnd());
	if(currEndNode)
	{
		// current segment's end node is multi node, so it is last segment of the link
		// get next segment(in next link)
		const RoadSegment* nextSegment = driverMvt->fwdDriverMovement.getNextSegment(false);
		if(!nextSegment){
			//seems current on last segment of the path
//			p.setStatus(STATUS_LEFT_OK); p.setStatus(STATUS_RIGHT_OK); p.setStatus(STATUS_CURRENT_OK);
			if(p.leftLane && !p.leftLane->is_pedestrian_lane()) {
				p.setStatus(STATUS_LEFT_SIDE_OK,STATUS_YES,str);
			}
			if(p.rightLane && !p.rightLane->is_pedestrian_lane()) {
				p.setStatus(STATUS_RIGHT_SIDE_OK,STATUS_YES,str);
			}
			if(p.currLane && !p.currLane->is_pedestrian_lane()) {
				p.setStatus(STATUS_CURRENT_LANE_OK,STATUS_YES,str);
			}

			const std::vector<sim_mob::Lane*> lanes = driverMvt->fwdDriverMovement.getCurrSegment()->getLanes();
			for(int i=0;i<lanes.size();++i) {
				if(!lanes[i]->is_pedestrian_lane()) {
					targetLanes.insert(lanes[i]);
				}
			}// end of for
			res = 0; // no need lane change
		}//end if(!nextSegment)
		else {
			// next segment on diff link
			// get lane connector
			const std::set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(driverMvt->fwdDriverMovement.getCurrSegment());

			// unset status
			p.setStatus(STATUS_LEFT_SIDE_OK,STATUS_NO,str); p.setStatus(STATUS_RIGHT_SIDE_OK,STATUS_NO,str);p.setStatus(STATUS_CURRENT_LANE_OK,STATUS_NO,str);
			// check left,right lanes connect to next target segment
			for (std::set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++)
			{
				if ( (*it)->getLaneTo()->getRoadSegment() == nextSegment ) // this lc connect to target segment
				{
					int laneIdx = getLaneIndex((*it)->getLaneFrom());
					if(laneIdx > p.currLaneIndex)
					{
						p.setStatus(STATUS_LEFT_SIDE_OK,STATUS_YES,str);
					}
					else if(laneIdx < p.currLaneIndex)
					{
						p.setStatus(STATUS_RIGHT_SIDE_OK,STATUS_YES,str);
					}
					else {
						p.setStatus(STATUS_CURRENT_LANE_OK,STATUS_YES,str);
					}
				}//end if
			}//end for

			if (lcs.size()>0)
			{
						//
				if(p.currLane->is_pedestrian_lane()) {
					//if can different DEBUG or RELEASE mode, that will be perfect, but now comment it out, so that does nor affect performance.
					//I remember the message is not critical
					WarnOut("drive on pedestrian lane");
					double d = driverMvt->fwdDriverMovement.getDisToCurrSegEndM();
					if(d<p.dis2stop)
						p.dis2stop = d;
					return -1;
				}
				std::map<int,vector<int> > indexes;
				std::set<int> noData;

				for (std::set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++) {
					if ((*it)->getLaneTo()->getRoadSegment() == nextSegment ) {
						// add lane to targetLanes
						const Lane* l = (*it)->getLaneFrom();
						targetLanes.insert(l);
						if( (*it)->getLaneFrom() == p.currLane )
						{
							// current lane connect to next link
							// no need lc
							res = 0;
						}
					}
				}// end for
			} // end of if (!lcs)
			else {
				throw std::runtime_error("isLaneConnectToNextLink: error, no lane connect in multi node");
			}
		}// end of else
	}//end if(currEndNode)
	else {
		// not on last segment of the link
		const std::vector<sim_mob::Lane*> lanes = driverMvt->fwdDriverMovement.getCurrSegment()->getLanes();
		for(int i=0;i<lanes.size();++i) {
			if(!lanes[i]->is_pedestrian_lane()) {
				targetLanes.insert(lanes[i]);
			}
		}// end of for
		res = 0;
	}
	return res;
}
LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::checkMandatoryEventLC(DriverUpdateParams& p)
{
	LANE_CHANGE_SIDE lcs = LCS_SAME;
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();

	// 1.0 check if has incident
	if( driverMvt->incidentPerformer.getIncidentStatus().getChangedLane() )
	{
		//p.nextLaneIndex = p.incidentPerformer.getIncidentStatus().getNextLaneIndex();
		lcs =driverMvt->incidentPerformer.getIncidentStatus().getLaneSide();
		if(lcs == LCS_LEFT) {
			p.setFlag(FLAG_ESCAPE_LEFT);
		}
		else if (lcs == LCS_RIGHT){
			p.setFlag(FLAG_ESCAPE_RIGHT);
		}
		return lcs;
	}

	// 2.0 if FLAG_ESCAPE(need do mlc) and current lane not ok, count left/right lc times to drive on good lane
	if(p.flag(FLAG_ESCAPE) )//no need check STATUS_CURRENT_OK, as if FLAG_ESCAPE ,current lane confirm not ok
	{
		// 2.1 find current lane index
		size_t currentLaneIdx = p.currLaneIndex;
		// number of lane changes to an open lane
		int nl = -1;		// left side
		int nr = -1;		// right side
		// use p.targetLanes
		set<const Lane*>::iterator it;
		for(it=p.targetLanes.begin();it!=p.targetLanes.end();++it)
		{
			//2.2 lane index
			const Lane* l = *it;
			size_t targetLaneIdx = getLaneIndex(l);
			if(targetLaneIdx>currentLaneIdx)
			{
				// target lane is in left of current lane
				int numberlc = targetLaneIdx - currentLaneIdx;
				if(numberlc < nl || nl<0) {
					nl = numberlc;
				}
			}
			else if(targetLaneIdx < currentLaneIdx)
			{
				// target lane in right
				int numberlc = currentLaneIdx - targetLaneIdx;
				if(numberlc < nr || nr <0) {
					nr = numberlc;
				}
			}
			else
			{
				// target lane == current lane,shall not happen
				WarnOut("target lane == current lane");
			}
		}//end for

		// 2.3 set FLAG_ESCAPE_LEFT or FLAG_ESCAPE_RIGHT
		// 2.3.1 There is an open left lane and no open right lane or the
		//       open left lane is closer.
		if(nl>0 && nr == -1)
		{
			if (p.flag(FLAG_ESCAPE)) {
				p.setFlag(FLAG_ESCAPE_LEFT);
			}
			else if (p.flag(FLAG_AVOID)) {
				p.setFlag(FLAG_AVOID_LEFT);
			}
			return LCS_LEFT;
		}
		// 2.3.2 There is an open right lane and no open left lane or the
		//       open right lane is closer.
		if(nr>0 && nl == -1)
		{
			if (p.flag(FLAG_ESCAPE) ){
				p.setFlag(FLAG_ESCAPE_RIGHT);
			}
			else if (p.flag(FLAG_AVOID)) {
				p.setFlag(FLAG_AVOID_RIGHT);
			}
			return LCS_RIGHT;
		}
		// 2.3.3 There is open lane on both side. Choose one randomly.
		if (Utils::brandom(0.5)) {
			if (p.flag(FLAG_ESCAPE)) {
				p.setFlag(FLAG_ESCAPE_LEFT);
			}
			else if (p.flag(FLAG_AVOID)){
				p.setFlag(FLAG_AVOID_LEFT);
			}
				return LCS_LEFT;
		} else {
				if (p.flag(FLAG_ESCAPE)) {
					p.setFlag(FLAG_ESCAPE_RIGHT);
				}
				else if (p.flag(FLAG_AVOID)) {
					p.setFlag(FLAG_AVOID_RIGHT);
				}
				return LCS_RIGHT;
		}//end if Utils::brandom(0.5)
	}//end if p.flag(FLAG_ESCAPE)

	// 3.0 discretionary lane change
	double eul = 0.0, eur = 0.0, euc = 1.0 ;
	if (isReadyForNextDLC(p,2))
	{
		eul = LCUtilityLeft(p);
	}

	if (isReadyForNextDLC(p,1))
	{
		eul = LCUtilityRight(p);
	}

	// 4.0 choose
	double sum = eul + eur ;

	if (sum > 0 ){
		euc = LCUtilityCurrent(p);
	  }

	sum += euc;

	double rnd = Utils::urandom();

	LANE_CHANGE_SIDE change;

	float probOfCurrentLane = euc / sum;
	float probOfCL_LL = probOfCurrentLane + eul / sum;
	if (rnd < probOfCurrentLane)  change = LCS_SAME;
	else if (rnd < probOfCL_LL) change = LCS_LEFT;
	else change = LCS_RIGHT;

	return change;
}
void sim_mob::MITSIM_LC_Model::checkConnectLanes(DriverUpdateParams& p)
{
	std::string str = "checkConnectLanes";
	// check current lane has connector to next link
//	if(p.dis2stop<distanceCheckToChangeLane) // <150m need check above, ready to change lane
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (driverMvt->fwdDriverMovement.getCurrSegment()->getEnd());
	if(currEndNode)
	{
		// check has next segment in path
		const RoadSegment* nextSegment = driverMvt->fwdDriverMovement.getNextSegment(false);
		if(!nextSegment){
			//seems on last segment of the path
			if(p.leftLane && !p.leftLane->is_pedestrian_lane()) {
				p.setStatus(STATUS_LEFT_SIDE_OK,STATUS_YES,str);
			}
			if(p.rightLane && !p.rightLane->is_pedestrian_lane()) {
				p.setStatus(STATUS_RIGHT_SIDE_OK,STATUS_YES,str);
			}
			if(p.currLane && !p.currLane->is_pedestrian_lane()) {
				p.setStatus(STATUS_CURRENT_LANE_OK,STATUS_YES,str);
			}
		}
		else {
			// get lane connector
			const std::set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(driverMvt->fwdDriverMovement.getCurrSegment());

			// check left,right lanes connect to next target segment
			for (std::set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++)
			{
				if ( (*it)->getLaneTo()->getRoadSegment() == nextSegment ) // this lc connect to target segment
				{
					int laneIdx = getLaneIndex((*it)->getLaneFrom());
					// lane index 0 start from most left lane of the segment
					// so lower number in the left, higher number in the right
					if(laneIdx > p.currLaneIndex) {
						p.setStatus(STATUS_LEFT_SIDE_OK,STATUS_YES,str);
					}
					else if(laneIdx < p.currLaneIndex) {
						p.setStatus(STATUS_RIGHT_SIDE_OK,STATUS_YES,str);
					}
					else if(laneIdx == p.currLaneIndex) {
						p.setStatus(STATUS_CURRENT_LANE_OK,STATUS_YES,str);
					}
				}// end if = nextsegment
			}//end for
		}// end else
	}// end if node
	else {
		// segment's end node is uninode
		// here just assume every lane can connect to next segment
		// for lane drop, it will set the status in isThereLaneDrop()
		// TODO use uninode lane connector
		if(p.leftLane && !p.leftLane->is_pedestrian_lane()) {
			p.setStatus(STATUS_LEFT_SIDE_OK,STATUS_YES,str);
		}
		if(p.rightLane && !p.rightLane->is_pedestrian_lane()) {
			p.setStatus(STATUS_RIGHT_SIDE_OK,STATUS_YES,str);
		}
		if(p.currLane && !p.currLane->is_pedestrian_lane()) {
			p.setStatus(STATUS_CURRENT_LANE_OK,STATUS_YES,str);
		}
	}

}
