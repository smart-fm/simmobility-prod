/*
 * CF_Model.cpp
 *
 *  Created on: 2011-8-15
 *      Author: wangxy & Li Zhemin
 */

#include "Driver.hpp"
using namespace sim_mob;

const double sim_mob::Driver::CF_parameters[2][6] = {
//        alpha   beta    gama    lambda  rho     stddev
		{ 0.0400, 0.7220, 0.2420, 0.6820, 0.6000, 0.8250},
		{-0.0418, 0.0000, 0.1510, 0.6840, 0.6800, 0.8020}
};


void sim_mob::Driver::makeAcceleratingDecision()
{
	//currently convert back to m/s
	speed_ = xVel_/100;
	double space = minCFDistance/100.0;
	if(space <= 0) {
		acc_=0;
	}
	else{
		if(!CFD) {
			acc_ = accOfFreeFlowing();
			return;
		} else {
			v_lead 		=	CFD->getRelatXacc()/100;
			a_lead		=	CFD->getRelatXacc()/100;
			double dt	=	timeStep;
			if (speed_ == 0)headway = 2 * space * 100000;
			else headway = space / speed_;
			space_star	=	space + v_lead * dt + 0.5 * a_lead * dt * dt;
		}
		if(headway < hBufferLower) {
			acc_ = accOfEmergencyDecelerating();
		}
		if(headway > hBufferUpper) {
			acc_ = accOfMixOfCFandFF();
		}
		if(headway <= hBufferUpper && headway >= hBufferLower) {
			acc_ = accOfCarFollowing();
		}
	}
}

double sim_mob::Driver::breakToTargetSpeed()
{
	double v 			=	speed_;
	double dt			=	timeStep;
	if( space_star > FLT_EPSILON) {
		return MAX_ACCELERATION;//(( v_lead + a_lead * dt ) * ( v_lead + a_lead * dt) - v * v) / 2 / space_star;
	} else if ( dt <= 0 ) {
		return MAX_ACCELERATION;
	} else {
		return ( v_lead + a_lead * dt - v ) / dt;
	}
}

double sim_mob::Driver::accOfEmergencyDecelerating()
{
	double v 			=	speed_;
	double dv			=	v-v_lead;
	double epsilon_v	=	0.001;
	double aNormalDec	=	getNormalDeceleration();

	double r;
	if( dv < epsilon_v ) {
		r=a_lead + 0.25*aNormalDec;
	} else if ( minCFDistance/100 > 0.01 ) {
		r=a_lead - dv * dv / 2 / (minCFDistance/100);
	} else {
		r= breakToTargetSpeed();
	}
	if(r<maxDeceleration)
		return maxDeceleration;
	else if(r>maxAcceleration)
		return maxAcceleration;
	else
		return r;
}

double uRandom()
{
	srand(time(0));
	long int seed_=rand();
	const long int M = 2147483647;  // M = modulus (2^31)
	const long int A = 48271;       // A = multiplier (was 16807)
	const long int Q = M / A;
	const long int R = M % A;
	seed_ = A * (seed_ % Q) - R * (seed_ / Q);
	seed_ = (seed_ > 0) ? (seed_) : (seed_ + M);
	return (double)seed_ / (double)M;
}

double nRandom(double mean,double stddev)
{
	   double r1 = uRandom(), r2 = uRandom();
	   double r = - 2.0 * log(r1);
	   if (r > 0.0) return (mean + stddev * sqrt(r) * sin(2 * 3.1415926 * r2));
	   else return (mean);
}

double sim_mob::Driver::accOfCarFollowing()
{
	const double density	=	1;		//represent the density of vehicles in front of the subject vehicle
										//now we ignore it, assuming that it is 1.
	double v				=	speed_;
	int i = (v > v_lead) ? 1 : 0;
	double dv =(v > v_lead)?(v-v_lead):(v_lead - v);
	double acc_ = CF_parameters[i][0] * pow(v , CF_parameters[i][1]) /pow(minCFDistance/100 , CF_parameters[i][2]);
	acc_ *= pow(dv , CF_parameters[i][3])*pow(density,CF_parameters[i][4]);
	acc_ += feet2Unit(nRandom(0,CF_parameters[i][5]));

	return acc_;
}

double sim_mob::Driver::accOfFreeFlowing()
{
	double vn			=	speed_;
	double acc_;

	if ( vn < getTargetSpeed()) {
		if( vn < maxLaneSpeed) {
			acc_=getMaxAcceleration();
		} else {
			acc_ = getNormalDeceleration();
		}
	}
	if ( vn > getTargetSpeed()) {
		acc_ = getNormalDeceleration();
	}
	if ( vn == getTargetSpeed()) {
		if( vn < maxLaneSpeed) {
			acc_=getMaxAcceleration();
		} else {
			acc_ = 0;
		}
	}
	return acc_;
}

double sim_mob::Driver::accOfMixOfCFandFF()		//mix of car following and free flowing
{
	distanceToNormalStop = speed_ * speed_ / 2 /(-getNormalDeceleration());
	if( minCFDistance/100 > distanceToNormalStop ) {
		return accOfFreeFlowing();
	} else {
		return breakToTargetSpeed();
	}
}
