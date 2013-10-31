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

using std::numeric_limits;
using namespace sim_mob;


namespace {
//Random number generator
//TODO: We need a policy on who can get a generator and why.
//boost::mt19937 gen;

//Declare MAX_NUM as a private variable here to limit its scope.
const double MAX_NUM = numeric_limits<double>::max();

///Simple struct to hold Gap Acceptance model parameters
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

///Simple struct to hold mandatory lane changing parameters
struct MandLaneChgParam {
	double feet_lowbound;
	double feet_delta;
	double lane_coeff;
	double congest_coeff;
	double lane_mintime;
};

struct AntiGap{
	double gap;
	double critial_gap;
};

const GapAcceptParam GA_parameters[4] = {
//	      scale alpha lambda beta0  beta1  beta2  beta3  beta4  stddev
		{ 1.00, 0.0, 0.000, 0.508, 0.000, 0.000,-0.420, 0.000, 0.488},	//Discretionary,lead
		{ 1.00, 0.0, 0.000, 2.020, 0.000, 0.000, 0.153, 0.188, 0.526},	//Discretionary,lag
		{ 1.00, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859},	//Mandatory,lead
		{ 1.00, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073}	//Mandatory,lag
};

const MandLaneChgParam MLC_parameters = {
		1320.0,		//feet, lower bound
	    5280.0,		//feet, delta
		   0.5,		//coef for number of lanes
		   1.0,		//coef for congestion level
		   1.0		//minimum time in lane
};

//Helper struct
template <class T>
struct LeadLag {
	T lead;
	T lag;
};

//Unit conversion
double meter2Feet(double meter) {
	return meter*3.2808399;
}
double feet2Meter(double feet) { //Note: This function is now in two locations.
	return feet*0.3048;
}



} //End anon namespace


// Kazi's LC gap model (see Kazi's MS thesis)
// from mitsim TS_Parameter.cc line 617
double sim_mob::MITSIM_LC_Model::lcCriticalGap(DriverUpdateParams& p, int type,	double dis, double spd, double dv)
{
	/*
	[Kazi LC Gap Models] = {
	% scale alpha lambda  beta0  beta1  beta2  beta3  beta4  sigma
	% Discretionary (Lead and Lag)
	  1.00  0.0   0.000   0.508  0.000  0.000 -0.420  0.000  0.488
	  1.00  0.0   0.000   2.020  0.000  0.000  0.153  0.188  0.526
	% Mandatory (Lead and Lag)
	  1.00  0.0   0.000   0.384  0.000  0.000  0.000  0.000  0.859
	  1.00  0.0   0.000   0.587  0.000  0.000  0.048  0.356  1.073
	}
	*/
//
//	double k=( type < 2 ) ? 1 : 5;
//	return k*-dv_ * p.elapsedSeconds;

	double lcGapModels_[][9] = {
  //	    scale  alpha   lambda   beta0   beta1   beta2   beta3   beta4   stddev
			{1.00 , 0.0 ,  0.000 ,  0.508 , 0.000 , 0.000 , -0.420 , 0.000 , 0.488},
			{1.00 , 0.0 ,  0.000 ,  2.020 , 0.000 , 0.000 , 0.153 ,  0.188 , 0.526},
			{1.00 , 0.0 ,  0.000 ,  0.384 , 0.000 , 0.000 , 0.000 ,  0.000 , 0.859},
			{1.00 , 0.0 ,  0.000 ,  0.587 , 0.000 , 0.000 , 0.048 ,  0.356 , 1.073},
			{0.60 , 0.0 ,  0.000 ,  0.384 , 0.000 , 0.000 , 0.000 ,  0.000 , 0.859}, //for test, courtesy merging
			{0.60 , 0.0 ,  0.000 ,  0.587 , 0.000 , 0.000 , 0.048 ,  0.356 , 1.073},//for test, courtesy merging
			{0.20 , 0.0 ,  0.000 ,  0.384 , 0.000 , 0.000 , 0.000 ,  0.000 , 0.859}, //for test,forced merging
			{0.20 , 0.0 ,  0.000 ,  0.587 , 0.000 , 0.000 , 0.048 ,  0.356 , 1.073}};//for test, forced merging

	  double *a = lcGapModels_[type] ;
	  double *b = lcGapModels_[type] + 3 ; //beta0
	  double rem_dist_impact = (type < 3) ?
		0.0 : (1.0 - 1.0 / (1 + exp(a[2] * dis)));
	  double dvNegative = (dv < 0) ? dv : 0.0;
	  double dvPositive = (dv > 0) ? dv : 0.0;
	  double gap = b[0] + b[1] * rem_dist_impact +
	              b[2] * dv + b[3] * dvNegative + b[4] *  dvPositive;

	  //boost::mt19937 gen;
	  boost::normal_distribution<> nrand(0,b[5]);

	  boost::variate_generator< boost::mt19937, boost::normal_distribution<> > normal(p.gen, nrand);

	  //std::cout<<"normal rand="<<normal()<<std::endl;

	  double u = gap + normal();

	  double cri_gap ;

	  if (u < -4.0) cri_gap = 0.0183 * a[0] ;      // exp(-4)=0.0183
	  else if (u > 6.0) cri_gap = 403.4 * a[0] ;   // exp(6)=403.4
	  else cri_gap = a[0] * exp(u) ;

	  if (cri_gap < a[1]) return a[1] ;
	  else return cri_gap ;
}


LaneSide sim_mob::MITSIM_LC_Model::gapAcceptance(DriverUpdateParams& p, int type)
{
	//[0:left,1:right]
	LeadLag<double> otherSpeed[2];		//the speed of the closest vehicle in adjacent lane
	LeadLag<double> otherDistance[2];	//the distance to the closest vehicle in adjacent lane

	const Lane* adjacentLanes[2] = {p.leftLane, p.rightLane};
	const NearestVehicle * fwd;
	const NearestVehicle * back;
	for(int i=0;i<2;i++){
		fwd = (i==0) ? &p.nvLeftFwd : &p.nvRightFwd;
		back = (i==0) ? &p.nvLeftBack : &p.nvRightBack;

		if(adjacentLanes[i]){	//the left/right side exists
			if(!fwd->exists()) {		//no vehicle ahead on current lane
				otherSpeed[i].lead=5000;
				otherDistance[i].lead=5000;
			} else {				//has vehicle ahead
				otherSpeed[i].lead = fwd->driver->fwdVelocity.get();
				otherDistance[i].lead= fwd->distance;
			}
//check otherDistance[i].lead if <= 0 return

			if(!back->exists()){//no vehicle behind
				otherSpeed[i].lag=-5000;
				otherDistance[i].lag=5000;
			}
			else{		//has vehicle behind, check the gap
				otherSpeed[i].lag=back->driver->fwdVelocity.get();
				otherDistance[i].lag= back->distance;
			}
		} else {			// no left/right side exists
			otherSpeed[i].lead    = 0;
			otherDistance[i].lead = 0;
			otherSpeed[i].lag     = 0;
			otherDistance[i].lag  = 0;
		}
	}

	//[0:left,1:right]
	LeadLag<bool> flags[2];
	for(int i=0;i<2;i++){	//i for left / right
		for(int j=0;j<2;j++){	//j for lead / lag
			if (j==0) {
				double v      = p.perceivedFwdVelocity/100.0;
				double dv     = (otherSpeed[i].lead/100.0 - v);
				double dis = otherDistance[i].lead/100.0;
				double cri_gap = lcCriticalGap(p, j+type,p.dis2stop,v,dv);
				flags[i].lead = (dis > cri_gap);
				if(cri_gap<0)
					std::cout<<"find gap < 1"<<std::endl;
			} else {
				double v 	 = otherSpeed[i].lag/100.0;
//				double dv 	 = p.perceivedFwdVelocity/100.0 - otherSpeed[i].lag/100.0;
				double dv	 = otherSpeed[i].lag/100.0 - p.perceivedFwdVelocity/100.0; // fixed by Runmin
				double cri_gap = lcCriticalGap(p, j+type,p.dis2stop,v,dv);
				flags[i].lag = (otherDistance[i].lag/100.0 > cri_gap);
				if(cri_gap<0)
						std::cout<<"find gap < 1."<<std::endl;
			}
		}
	}

	//Build up a return value.
	LaneSide returnVal = {false, false};
	if ( flags[0].lead && flags[0].lag ) {
		returnVal.left = true;
	}
	if ( flags[1].lead && flags[1].lag ) {
		returnVal.right = true;
	}

	return returnVal;
}

double sim_mob::MITSIM_LC_Model::calcSideLaneUtility(DriverUpdateParams& p, bool isLeft){
	if(isLeft && !p.leftLane) {
		return -MAX_NUM;	//has no left side
	} else if(!isLeft && !p.rightLane){
		return -MAX_NUM;    //has no right side
	}
	return (isLeft) ? p.nvLeftFwd.distance : p.nvRightFwd.distance;
}

LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::makeDiscretionaryLaneChangingDecision(DriverUpdateParams& p)
{
	// for available gaps(including current gap between leading vehicle and itself), vehicle will choose the longest
	//const LaneSide freeLanes = gapAcceptance(p, DLC);
	LaneSide freeLanes = gapAcceptance(p, DLC);
	if(!freeLanes.left && !freeLanes.right) {
		return LCS_SAME;		//neither gap is available, stay in current lane
	}
	double s = p.nvFwd.distance;
	const double satisfiedDistance = 2000;
	const double minDistance = 10;
	if(s>satisfiedDistance) {
		return LCS_SAME;	// space ahead is satisfying, stay in current lane
	}

	//choose target gap, for both left and right
	TARGET_GAP* tg = chooseTargetGap(p);

	//calculate the utility of both sides
	double leftUtility = calcSideLaneUtility(p, true);
	double rightUtility = calcSideLaneUtility(p, false);

	//to check if their utilities are greater than current lane
	bool left = ( s < leftUtility );
	bool right = ( s < rightUtility );

	//decide
	if(freeLanes.rightOnly() && right) {
		return LCS_RIGHT;
	}
	if(freeLanes.leftOnly() && left) {
		return LCS_LEFT;
	}
	if(freeLanes.both()){
		if(right && left){
			return (leftUtility > rightUtility) ? LCS_LEFT : LCS_RIGHT;	//both side is available, choose the better one
		}
		if(right && !left) {
			return LCS_LEFT;
		}
		if(!right && left) {
			return LCS_RIGHT;
		}
		if(!right && !left) {
			return LCS_SAME;
		}
	}

	if(left || right){
		p.targetGap = (leftUtility > rightUtility)? tg[0] : tg[1];
	}

	return LCS_SAME;
}
int sim_mob::MITSIM_LC_Model::loadMLCKaziNosingModel(DriverUpdateParams &g)
{
//  int n = gv.nElements() ;
//  if (n != 6) return error(gv.name());
//  kaziNosingParams_ = new float[n] ; // changed from "(n)" to "[n]" by Angus
//  for (int i = 0; i < n; i ++) {
//	kaziNosingParams_[i] = gv.element(i) ;
//  }
  return 0 ;
}
double sim_mob::MITSIM_LC_Model::checkIfMandatory(DriverUpdateParams& p)
{
	if(p.nextLaneIndex == p.currLaneIndex)
		p.dis2stop = 5000;//defalut 5000m
	//The code below is MITSIMLab model
	double num		=	1;		//now we just assume that MLC only need to change to the adjacent lane
	double y		=	0.5;	//segment density/jam density, now assume that it is 0.5
	//double delta0	=	feet2Unit(MLC_parameters.feet_lowbound);
	double dis2stop_feet = meter2Feet(p.dis2stop);
	double dis		=	dis2stop_feet - MLC_parameters.feet_lowbound;
	double delta	=	1.0 + MLC_parameters.lane_coeff * num + MLC_parameters.congest_coeff * y;
	delta *= MLC_parameters.feet_delta;
	return exp(-dis * dis / (delta * delta));
}

LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::makeMandatoryLaneChangingDecision(DriverUpdateParams& p)
{
	LaneSide freeLanes = gapAcceptance(p, MLC);

	//find which lane it should get to and choose which side to change
	//now manually set to 1, it should be replaced by target lane index
	//i am going to fix it.
	int direction = p.nextLaneIndex - p.currLaneIndex;
	//direction = 0; //Otherwise drivers always merge.
	//direction = 1;
	//current lane is target lane
	//std::cout<<"Already MLC,"<<"Direction:"<<direction<<std::endl;
	if(direction==0) {
		return LCS_SAME;
	}

	//current lane isn't target lane
	if(freeLanes.right && direction<0) {		//target lane on the right and is accessable
		p.isWaiting=false;
		return LCS_RIGHT;
	} else if(freeLanes.left && direction>0) {	//target lane on the left and is accessable
		p.isWaiting=false;
		return LCS_LEFT;
	} else {			//when target side isn't available,vehicle will decelerate to wait a proper gap.
		p.isWaiting=true;
		return LCS_SAME;
	}
}


LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::executeNGSIMModel(DriverUpdateParams& p)
{
	bool isCourtesy; //if courtesy merging
	bool isForced;	// if forced merging
	double anti_gap; //anticipated gap
	double critical_anti_gap; //critical anticipated gap

	isCourtesy = false;
	isForced = false;
	anti_gap = 0;
	critical_anti_gap = 0;

	int direction = p.nextLaneIndex - p.currLaneIndex;

	LANE_CHANGE_SIDE lcs = direction>0? LCS_LEFT : LCS_RIGHT;

	//check if courtesy merging
	isCourtesy = ifCourtesyMerging(p);

	if(isCourtesy) {
		lcs = makeCourtesyMerging(p);
		//std::cout<<"courtesy change:"<<lcs<<std::endl;
		return lcs;
	}else{
		//check if forced merging
		isForced = ifForcedMerging(p);
		if(isForced){
			lcs = makeForcedMerging(p);
			//std::cout<<"forced change:"<<lcs<<std::endl;
			return lcs;
		}
	}

	return LCS_SAME;
}



bool sim_mob::MITSIM_LC_Model::ifCourtesyMerging(DriverUpdateParams& p)
{
	//[0:left,1:right]
	LeadLag<double> otherSpeed[2];		//the speed of the closest vehicle in adjacent lane
	LeadLag<double> otherDistance[2];	//the distance to the closest vehicle in adjacent lane
	LeadLag<double> otherAcc[2]; 		//the acceleration of the closet vehicles in the adjant lane

	const Lane* adjacentLanes[2] = {p.leftLane, p.rightLane};
	const NearestVehicle * fwd;
	const NearestVehicle * back;
	for(int i=0;i<2;i++){
		fwd = (i==0) ? &p.nvLeftFwd : &p.nvRightFwd;
		back = (i==0) ? &p.nvLeftBack : &p.nvRightBack;

		if(adjacentLanes[i]){	//the left/right side exists
			if(!fwd->exists()) {		//no vehicle ahead on current lane
				otherSpeed[i].lead=5000;
				otherDistance[i].lead=5000;
				otherAcc[i].lead = 5000;
			} else {				//has vehicle ahead
				otherSpeed[i].lead = fwd->driver->fwdVelocity.get();
				otherDistance[i].lead= fwd->distance;
				otherAcc[i].lead = fwd->driver->fwdAccel.get();
			}

			if(!back->exists()){//no vehicle behind
				otherSpeed[i].lag=-5000;
				otherDistance[i].lag=5000;
				otherAcc[i].lag = 5000;
			}
			else{		//has vehicle behind, check the gap
				otherSpeed[i].lag=back->driver->fwdVelocity.get();
				otherDistance[i].lag= back->distance;
				otherAcc[i].lag = back->driver->fwdAccel.get();
			}
		} else {			// no left/right side exists
			otherSpeed[i].lead    = 0;
			otherDistance[i].lead = 0;
			otherAcc[i].lead = 0;
			otherSpeed[i].lag     = 0;
			otherDistance[i].lag  = 0;
			otherAcc[i].lag = 0;
		}
	}

	int direction = p.nextLaneIndex - p.currLaneIndex;
	//[0:left,1:right]
	int i = direction>0? 0:1;
	//if (direction > 0) direction = 0;
	//else direction = 1;

	double dis_lead = otherDistance[i].lead/100.0;
	double dis_lag = otherDistance[i].lag/100.0;

	double v_lead = otherSpeed[i].lead/100.0;
	double v_lag =  otherSpeed[i].lag/100.0;

	double acc_lead = otherAcc[i].lead/100.0;
	double acc_lag = otherAcc[i].lag/100.0;


	double gap = dis_lead + dis_lag + (v_lead - v_lag)*p.elapsedSeconds + 0.5*(acc_lead - acc_lag) * p.elapsedSeconds * p.elapsedSeconds;

	double para[4]={1.82, 1.81, -0.153, 0.0951};

	double v = v_lag - p.perceivedFwdVelocity/100;
	double dv = v>0? v:0;

	//calculate critical gap for courtesy merging
	double critical_gap = exp(para[0] + para[1]*dv + para[3]* p.dis2stop/100);

	bool courtesy = gap - critical_gap > 0? true:false;


	return courtesy;

}

bool sim_mob::MITSIM_LC_Model::ifForcedMerging(DriverUpdateParams& p)
{
	boost::uniform_int<> zero_to_max(0, RAND_MAX);
	double randNum = (double)(zero_to_max(p.gen)%1000)/1000;
	if(randNum < 1/(1+exp(4.27+1.25-5.43))){
		return true;
	}
	return false;
}


LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::makeCourtesyMerging(DriverUpdateParams& p)
{
	LaneSide freeLanes = gapAcceptance(p, MLC_C);

	int direction = p.nextLaneIndex - p.currLaneIndex;
	//direction = 0; //Otherwise drivers always merge.
	//direction = 1;
	//current lane is target lane
	if(direction==0) {
		return LCS_SAME;
	}

	//current lane isn't target lane
	if(freeLanes.right && direction<0) {		//target lane on the right and is accessable
		p.isWaiting=false;
		return LCS_RIGHT;
	} else if(freeLanes.left && direction>0) {	//target lane on the left and is accessable
		p.isWaiting=false;
		return LCS_LEFT;
	} else {			//when target side isn't available,vehicle will decelerate to wait a proper gap.
		p.isWaiting=true;
		return LCS_SAME;
	}
}

LANE_CHANGE_SIDE sim_mob::MITSIM_LC_Model::makeForcedMerging(DriverUpdateParams& p)
{
	LaneSide freeLanes = gapAcceptance(p, MLC_F);

	int direction = p.nextLaneIndex - p.currLaneIndex;
	//std::cout<<"Already MLC,"<<"Direction:"<<direction<<std::endl;
	if(direction==0) {
		return LCS_SAME;
	}

	//current lane isn't target lane
	if(freeLanes.right && direction<0) {		//target lane on the right and is accessable
		p.isWaiting=false;
		return LCS_RIGHT;
	} else if(freeLanes.left && direction>0) {	//target lane on the left and is accessable
		p.isWaiting=false;
		return LCS_LEFT;
	} else {
		p.isWaiting=true;
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


TARGET_GAP* sim_mob::MITSIM_LC_Model::chooseTargetGap(DriverUpdateParams& p)
{
	double GapParam[][6] = {
		  //const	   dis2gap  gap-size  gap-vel   dummy  vn
			{-1.23  , -0.482 ,  0.224 ,  -0.0179 , 2.10 , 0.239 },	//back
			{0.00   ,  0.00  ,  0.224 ,  -0.0179 , 2.10 , 0.000 },  //adj
			{-0.772 , -0.482 ,  0.224 ,  -0.0179 , 2.10 , 0.675 }};	//fwd

	double U_Left_Fwd;
	double U_Left_Back;
	double U_Left_Adj;
	double U_Right_Fwd;
	double U_Right_Back;
	double U_Right_Adj;

	const Lane* lane[2] = {p.leftLane, p.rightLane};

	const Lane* rightLane	= p.rightLane;

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
	for(int i=0;i<2;i++){
		for(int j=0; j<4; j++){
			dis[i][j] = (nv[i][j]->exists())? nv[i][j]->distance/100 : 50;
		}
	}

	double vel[2][4];
	for(int i=0;i<2;i++){
		for(int j=0; j<4; j++){
			vel[i][j] = (nv[i][j]->exists())? nv[i][j]->driver->fwdVelocity/100 : 0;
		}

	}

	boost::uniform_int<> zero_to_max(0, RAND_MAX);
	double randNum = (double)(zero_to_max(p.gen)%1000)/1000;

	//calculate the utilities of nearby gaps
	double U[2][3];
	for(int i=0;i<2;i++){
			U[i][0] = GapParam[0][0] + GapParam[0][1]*dis[i][1] + GapParam[0][2]*(dis[i][0] - dis[i][1]) + GapParam[0][3]*(vel[i][0]-vel[i][1]) + ((!nv[i][0]->exists())? GapParam[0][4]:0) + GapParam[0][5] * randNum;
			U[i][1] = GapParam[1][0] + GapParam[1][1]*0 + GapParam[1][2]*(dis[i][1] + dis[i][2]) + GapParam[1][3]*(vel[i][1]-vel[i][2]) + ((!nv[i][1]->exists() || !nv[i][2]->exists())? GapParam[1][4]:0) + GapParam[1][5] * randNum;
			U[i][2] = GapParam[2][0] + GapParam[2][1]*dis[i][2] + GapParam[2][2]*(dis[i][3] - dis[i][2]) + GapParam[2][3]*(vel[i][2]-vel[i][3]) + ((!nv[i][3]->exists())? GapParam[2][4]:0) + GapParam[2][5] * randNum;
			if(!lane[i]) U[i][0] = U[i][1] = U[i][2] = -MAX_NUM;
	}

	/*std::cout<<"Utility=";
	for(int i=0; i<2; i++){
		for(int j=0;j<3;j++){
			std::cout<<U[i][j]<<",";
		}
	}
	std::cout<<std::endl;*/

	//calculte probabilities of choosing each gap
	double logsum[2] = {0.0, 0.0};
	double cdf[2][3];
	double prob[2][3];
	for(int i=0; i<2; i++){
		for(int j=0;j<3;j++){
			logsum[i] += exp(U[i][j]);
			cdf[i][j] = logsum[i];
		}
	}

	/*std::cout<<"prob=";
	for(int i=0; i<2; i++){
		for(int j=0;j<3;j++){
			if(logsum[i]==0){
				prob[i][j] = 0;
				cdf[i][j] = 0;
			}else{
				prob[i][j]= exp(U[i][j])/logsum[i];
				cdf[i][j] = cdf[i][j]/logsum[i];
			}

			std::cout<<prob[i][j]<<",";
		}
	}
	std::cout<<std::endl;

	std::cout<<"cdf=";
	for(int i=0; i<2; i++){
		for(int j=0;j<3;j++){
			std::cout<<cdf[i][j]<<",";
		}
	}
	std::cout<<std::endl;*/

	TARGET_GAP tg[] = {TG_Same,TG_Same};
	double rnd = (double)(zero_to_max(p.gen)%1000)/1000;

	if(rnd >= 0 && rnd < cdf[0][0]){
		tg[0] = TG_Left_Back;
	}else if(rnd >= cdf[0][0] && rnd < cdf[0][1]){
		tg[0] = TG_Left_Adj;
	}else if(rnd >= cdf[0][1] && rnd < cdf[0][2]){
		tg[0] = TG_Left_Fwd;
	}

	if(rnd >= 0 && rnd < cdf[1][0]){
		tg[1] = TG_Right_Back;
	}else if(rnd >= cdf[1][0] && rnd < cdf[1][1]){
		tg[1] = TG_Right_Adj;
	}else if(rnd >= cdf[1][1] && rnd < cdf[1][2]){
		tg[1] = TG_Right_Fwd;
	}

	//std::cout<<"target_gap="<<tg[0]<<" , "<<tg[1]<<std::endl;

	return tg;
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
double sim_mob::MITSIM_LC_Model::executeLaneChanging(DriverUpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir, LANE_CHANGE_MODE mode)
{
	if(p.now.frame() > 1300.0)
	{
		int aa=0;
	}
	//Behavior changes depending on whether or not we're actually changing lanes.
	//if(currLaneChangeDir==LCS_SAME)
	//{
	//	return 0.0;
	//}
	//else
	{
		//1.If too close to node, don't do lane changing, distance should be larger than 3m
		if(p.dis2stop <= 3) {
			return 0.0;
		}

		//2.Get a random number, use it to determine if we're making a discretionary or a mandatory lane change
		boost::uniform_int<> zero_to_max(0, RAND_MAX);
		double randNum = (double)(zero_to_max(p.gen)%1000)/1000;
		double mandCheck = checkIfMandatory(p);
		LANE_CHANGE_MODE changeMode = mode;  //DLC or MLC

		if(changeMode==DLC){
			if(randNum<mandCheck){
				changeMode = MLC;
			} else {
				changeMode = DLC;
				p.dis2stop = 1000;//MAX_NUM;		//no crucial point ahead
			}

			if(p.isMLC == true){
				changeMode = MLC;
			}
		}


		//3.make decision depending on current lane changing mode
		LANE_CHANGE_SIDE decision = LCS_SAME;
		if(changeMode==DLC) {
			decision = makeDiscretionaryLaneChangingDecision(p);
		} else {
			decision = makeMandatoryLaneChangingDecision(p);
		}

		//4.Finally, if we've decided to change lanes, set our intention.
		if(decision!=LCS_SAME) {
			const int lane_shift_velocity = 350;  //TODO: What is our lane changing velocity? Just entering this for now...

			return decision==LCS_LEFT?lane_shift_velocity:-lane_shift_velocity;
		}

		return 0.0;
	}
//	{ //Performing a lane change.
//		//Set the lateral velocity of the vehicle; move it.
//		int lcsSign = (currLaneChangeDir==LCS_RIGHT) ? -1 : 1;
//		return lcsSign*150;//p.laneChangingVelocity;
//	}


//		else {
//		//If too close to node, don't do lane changing, distance should be larger than 3m
////		if(p.nvFwd.distance <= 2000) {
////			return 0;
////		}
//
//		//Get a random number, use it to determine if we're making a discretionary or a mandatory lane change
//		boost::uniform_int<> zero_to_max(0, RAND_MAX);
//		double randNum = (double)(zero_to_max(p.gen)%1000)/1000;
//		double mandCheck = checkIfMandatory(p);
//		LANE_CHANGE_MODE changeMode;  //DLC or MLC
//
//		if(randNum<mandCheck){
//			changeMode = MLC;
//		} else {
//			changeMode = DLC;
//			p.dis2stop = 1000;//MAX_NUM;		//no crucial point ahead
//		}
//
//		//make decision depending on current lane changing mode
//		LANE_CHANGE_SIDE decision = LCS_SAME;
//		if(changeMode==DLC) {
//
//			decision = makeDiscretionaryLaneChangingDecision(p);
//		} else {
//
//			decision = makeMandatoryLaneChangingDecision(p);
//
//		}
//
//
//		//Finally, if we've decided to change lanes, set our intention.
//		if(decision!=LCS_SAME) {
//			const int lane_shift_velocity = 150;  //TODO: What is our lane changing velocity? Just entering this for now...
//
//			return decision==LCS_LEFT?lane_shift_velocity:-lane_shift_velocity;
//		}
//	}
//	return 0;
}
