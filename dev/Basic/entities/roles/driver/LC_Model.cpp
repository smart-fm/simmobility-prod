/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * LC_Model.cpp
 *
 *  Created on: 2011-8-15
 *      Author: mavswinwxy & Li Zhemin
 */

#include "Driver.hpp"

#include <limits>

#include "entities/vehicle/Vehicle.hpp"
#include "geospatial/Link.hpp"

using std::numeric_limits;
using namespace sim_mob;


namespace {
//Declare MAX_NUM as a private variable here to limit its scope.
const double MAX_NUM = numeric_limits<double>::max();

//Helper struct
template <class T>
struct LeadLag {
	T lead;
	T lag;
};


} //End anon namespace


const sim_mob::Driver::GapAcceptParam sim_mob::Driver::GA_parameters[4] = {
//	    scale alpha lambda beta0  beta1  beta2  beta3  beta4  stddev
		{ 1.00, 0.0, 0.000, 0.508, 0.000, 0.000,-0.420, 0.000, 0.488},	//Discretionary,lead
		{ 1.00, 0.0, 0.000, 2.020, 0.000, 0.000, 0.153, 0.188, 0.526},	//Discretionary,lag
		{ 1.00, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859},	//Mandatory,lead
		{ 1.00, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073}	//Mandatory,lag
};
const sim_mob::Driver::MandLaneChgParam sim_mob::Driver::MLC_parameters = {
		1320.0,		//feet, lower bound
	    5280.0,		//feet, delta
		   0.5,		//coef for number of lanes
		   1.0,		//coef for congestion level
		   1.0		//minimum time in lane
};

double sim_mob::Driver::lcCriticalGap(int type,	double dis_, double spd_, double dv_)
{
	double k=( type < 2 ) ? 1 : 5;
	return k*-dv_*timeStep;

	//The code below is from MITSIMLab. While the calculation result not suit for current unit.
	//So now, I just put them here.
	/*double dis=unit2Feet(dis_);
	double spd=unit2Feet(spd_);
	double dv=unit2Feet(dv_);
	double rem_dist_impact = (type < 3) ?
		0.0 : (1.0 - 1.0 / (1 + exp(GA_parameters[type][2] * dis)));
	double dvNegative = (dv < 0) ? dv : 0.0;
	double dvPositive = (dv > 0) ? dv : 0.0;
	double gap = GA_parameters[type][3] + GA_parameters[type][4] * rem_dist_impact +
			GA_parameters[type][5] * dv + GA_parameters[type][6] * dvNegative + GA_parameters[type][7] *  dvPositive;
	double u = gap + nRandom(0, GA_parameters[type][8]);
	double cri_gap ;

	if (u < -4.0) {
		cri_gap = 0.0183 * GA_parameters[type][0] ;		// exp(-4)=0.0183
	}
	else if (u > 6.0) {
		cri_gap = 403.4 * GA_parameters[type][0] ;   	// exp(6)=403.4
	}
	else cri_gap = GA_parameters[type][0] * exp(u) ;

	if (cri_gap < GA_parameters[type][1]) return feet2Unit(GA_parameters[type][1]) ;
	else return feet2Unit(cri_gap) ;*/
}


unsigned int sim_mob::Driver::gapAcceptance(int type)
{
	//[0:left,1:right]
	LeadLag<double> otherSpeed[2];		//the speed of the closest vehicle in adjacent lane
	LeadLag<double> otherDistance[2];	//the distance to the closest vehicle in adjacent lane

	const Lane* adjacentLanes[2] = {leftLane,rightLane};
	const Driver* fwd;
	const Driver* back;
	for(int i=0;i<2;i++){
		fwd = (i==0) ? LFD : RFD;
		back = (i==0) ? LBD : RBD;

		if(adjacentLanes[i]){	//the left/right side exists
			if(!fwd) {		//no vehicle ahead on current lane
				otherSpeed[i].lead=MAX_NUM;
				otherDistance[i].lead=MAX_NUM;
			} else {				//has vehicle ahead
				//otherSpeed[i].lead=fwd->getVehicle()->xVel_;
				//otherSpeed[i].lead = fwd->getVehicle()->velocity.getRelX();
				otherSpeed[i].lead = fwd->getVehicle()->velocity.getMagnitude();

				otherDistance[i].lead=(i==0)? minLFDistance:minRFDistance;
			}
			if(!back){//no vehicle behind
				otherSpeed[i].lag=-MAX_NUM;
				otherDistance[i].lag=MAX_NUM;
			}
			else{		//has vehicle behind, check the gap
				//otherSpeed[i].lag=back->getVehicle()->xVel_;
				//otherSpeed[i].lag=back->getVehicle()->velocity.getRelX();
				otherSpeed[i].lag=back->getVehicle()->velocity.getMagnitude();

				otherDistance[i].lag=(i==0)? minLBDistance:minRBDistance;
			}
		} else {			// no left/right side exists
			otherSpeed[i].lead    = -MAX_NUM;
			otherDistance[i].lead = 0;
			otherSpeed[i].lag     = MAX_NUM;
			otherDistance[i].lag  = 0;
		}
	}

	//[0:left,1:right]
	LeadLag<bool> flags[2];
	for(int i=0;i<2;i++){	//i for left / right
		for(int j=0;j<2;j++){	//j for lead / lag
			if (j==0) {
				double v      = perceivedXVelocity_;

				double dv     = otherSpeed[i].lead - perceivedXVelocity_;
				flags[i].lead = (otherDistance[i].lead > lcCriticalGap(j+type,dis2stop,v,dv));
			} else {

				double v 	 = otherSpeed[i].lag;
				double dv 	 = perceivedXVelocity_-otherSpeed[i].lag;
				flags[i].lag = (otherDistance[i].lag > lcCriticalGap(j+type,dis2stop,v,dv));
			}
		}
	}

	//Build up a return value.
	unsigned int returnVal = 0;
	if ( flags[0].lead && flags[0].lag ) {
		returnVal |= LSIDE_LEFT;
	}
	if ( flags[1].lead && flags[1].lag ) {
		returnVal |= LSIDE_RIGHT;
	}

	return returnVal;
}

double sim_mob::Driver::calcSideLaneUtility(bool isLeft){
	if(isLeft && !leftLane) {
		return -MAX_NUM;	//has no left side
	}
	else if(!isLeft && !rightLane){
		return -MAX_NUM;    //has no right side
	}
	else
		return (isLeft)?minLFDistance : minRFDistance;
}

LANE_CHANGE_SIDE sim_mob::Driver::makeDiscretionaryLaneChangingDecision()
{
	// for available gaps(including current gap between leading vehicle and itself), vehicle will choose the longest
	unsigned int freeLanes = gapAcceptance(DLC);
	bool freeLeft = ((freeLanes&LSIDE_LEFT)!=0);
	bool freeRight = ((freeLanes&LSIDE_RIGHT)!=0);

	if(!freeLeft && !freeRight) {
		return LCS_SAME;		//neither gap is available, stay in current lane
	}

	double s=minCFDistance;
	satisfiedDistance = 1000;
	if(s>satisfiedDistance) {
		return LCS_SAME;	// space ahead is satisfying, stay in current lane
	}

	//calculate the utility of both sides
	double leftUtility = calcSideLaneUtility(true);
	double rightUtility = calcSideLaneUtility(false);

	//to check if their utilities are greater than current lane
	bool left = ( s < leftUtility );
	bool right = ( s < rightUtility );

	//decide
	if(freeRight && !freeLeft && right) {
		return LCS_RIGHT;
	}
	if(freeLeft && !freeRight && left) {
		return LCS_LEFT;
	}
	if(freeLeft && freeRight){
		if(right && left){
			return (leftUtility < rightUtility) ? LCS_LEFT : LCS_RIGHT;	//both side is available, choose the better one
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
	return LCS_SAME;
}

double sim_mob::Driver::checkIfMandatory() const {
	//The code below is MITSIMLab model
	dis2stop = currLink->getLength(isForward) - currLinkOffset - vehicle->length/2 - 300;
	dis2stop /= 100;

	double num		=	1;		//now we just assume that MLC only need to change to the adjacent lane
	double y		=	0.5;	//segment density/jam density, now assume that it is 0.5
	double delta0	=	feet2Unit(MLC_parameters.feet_lowbound);
	double dis		=	dis2stop - delta0;
	double delta	=	1.0 + MLC_parameters.lane_coeff * num + MLC_parameters.congest_coeff * y;
	delta *= MLC_parameters.feet_delta;
	return exp(-dis * dis / (delta * delta));
}

LANE_CHANGE_SIDE sim_mob::Driver::makeMandatoryLaneChangingDecision()
{
	unsigned int freeLanes = gapAcceptance(MLC);
	bool freeLeft = ((freeLanes&LSIDE_LEFT)!=0);
	bool freeRight = ((freeLanes&LSIDE_RIGHT)!=0);

	//find which lane it should get to and choose which side to change
	//now manually set to 1, it should be replaced by target lane index
	//i am going to fix it.
	int direction=1-currLaneIndex;

	//current lane is target lane
	if(direction==0) {
		return LCS_SAME;
	}

	//current lane isn't target lane
	if(freeRight && direction<0) {		//target lane on the right and is accessable
		isWaiting=false;
		return LCS_RIGHT;
	} else if(freeLeft && direction>0) {	//target lane on the left and is accessable
		isWaiting=false;
		return LCS_LEFT;
	} else {			//when target side isn't available,vehicle will decelerate to wait a proper gap.
		isWaiting=true;
		return LCS_SAME;
	}
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
void sim_mob::Driver::excuteLaneChanging(UpdateParams& p)
{
	//Behavior changes depending on whether or not we're actually changing lanes.
	if(isLaneChanging){
		if(changeDecision != LCS_SAME) {
			//Set the lateral velocity of the vehicle; move it.
			int lcsSign = (changeDecision==LCS_RIGHT) ? -1 : 1;
			vehicle->velocity_lat.scaleVectTo(lcsSign*p.laneChangingVelocity);
		}
	} else {
		//If too close to node, don't do lane changing, distance should be larger than 3m
		if(p.currLaneLength - p.currLaneOffset - vehicle->length/2 <= 300) {
			return;
		}

		//Get a random number, use it to determine if we're making a discretionary or a mandatory lane change
		double randNum = (double)(rand()%1000)/1000;
		if(randNum<checkIfMandatory()){
			changeMode = MLC;
		} else {
			changeMode = DLC;
			dis2stop = MAX_NUM;		//no crucial point ahead
		}

		//make decision depending on current lane changing mode
		if(changeMode==DLC) {
			changeDecision = makeDiscretionaryLaneChangingDecision();
		} else {
			changeDecision = makeMandatoryLaneChangingDecision();
		}

		//Finally, if we've decided to change lanes, set our intention.
		if(changeDecision!=LCS_SAME) {
			isLaneChanging = true;
			lcEnterNewLane = false;
		}
	}
}
