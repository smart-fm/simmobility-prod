/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * CF_Model.cpp
 *
 *  Created on: 2011-8-15
 *      Author: wangxy & Li Zhemin
 */

#include <limits>

#include "entities/vehicle/Vehicle.hpp"
#include "CarFollowModel.hpp"
#include "Driver.hpp"

using std::numeric_limits;
using namespace sim_mob;


namespace {
//Threshold defaults
const double hBufferUpper			=	  1.6;	 ///< upper threshold of headway
const double hBufferLower			=	  0.8;	 ///< lower threshold of headway

//Set default data for acceleration
const double maxAcceleration = 20.0;   ///< 10m/s*s, might be tunable later
const double normalDeceleration = -maxAcceleration*0.6;
const double maxDeceleration = -maxAcceleration;

//Simple conversion
double feet2Unit(double feet) {
	return feet*0.158;
}

//Simple struct to hold Car Following model parameters
struct CarFollowParam {
	double alpha;
	double beta;
	double gama;
	double lambda;
	double rho;
	double stddev;
};

//Car following parameters for this model.
const CarFollowParam CF_parameters[2] = {
//    alpha   beta    gama    lambda  rho     stddev
	{ 0.0400, 0.7220, 0.2420, 0.6820, 0.6000, 0.8250},
	{-0.0418, 0.0000, 0.1510, 0.6840, 0.6800, 0.8020}
};


//Acceleration mode
enum ACCEL_MODE {
	AM_VEHICLE = 0,
	AM_PEDESTRIAN = 1,
	AM_TRAFF_LIGHT = 2,
	AM_NONE = 3
};

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
} //End anon namespace


double sim_mob::MITSIM_CF_Model::makeAcceleratingDecision(UpdateParams& p, double targetSpeed, double maxLaneSpeed)
{
	//Set our mode.
	ACCEL_MODE mode;
	if(p.nvFwd.distance != 5000 && p.nvFwd.distance <= p.trafficSignalStopDistance && p.nvFwd.distance <= p.npedFwd.distance) {
		p.space = p.nvFwd.distance/100;
		mode = AM_VEHICLE;
	} else if(p.npedFwd.distance != 5000 && p.npedFwd.distance <= p.nvFwd.distance && p.npedFwd.distance <= p.trafficSignalStopDistance) {
		p.space = p.npedFwd.distance/100;
		mode = AM_PEDESTRIAN;
	} else if(p.trafficSignalStopDistance != 5000 && p.trafficSignalStopDistance <= p.npedFwd.distance && p.trafficSignalStopDistance <= p.nvFwd.distance) {
		p.space = p.trafficSignalStopDistance/100;
		mode = AM_TRAFF_LIGHT;
	} else {
		p.space = p.trafficSignalStopDistance/100;//which should be default value 50m
		mode = AM_NONE;
	}
	p.space -= 0.5;

	//If we have no space left to move, immediately cut off acceleration.
	double res = 0;
	if(p.space > 0) {
		if(mode == AM_NONE) {
			return accOfFreeFlowing(p, targetSpeed, maxLaneSpeed);
		}

		//Retrieve velocity/acceleration in m/s
		p.v_lead = (mode!=AM_VEHICLE) ? 0 : p.nvFwd.driver->getVehicle()->getVelocity()/100;
		p.a_lead = (mode!=AM_VEHICLE) ? 0 : p.nvFwd.driver->getVehicle()->getAcceleration()/100;

		double dt	=	p.elapsedSeconds;
		double headway = 0;  //distance/speed
		if (p.currSpeed == 0) {
			headway = 2 * p.space * 100000;
		} else {
			headway = 2 * p.space / (p.currSpeed+p.currSpeed+p.elapsedSeconds*maxAcceleration);
		}
		p.space_star	=	p.space + p.v_lead * dt + 0.5 * p.a_lead * dt * dt;

		if(headway < hBufferLower) {
			res = accOfEmergencyDecelerating(p);
		}
		if(headway > hBufferUpper) {
			res = accOfMixOfCFandFF(p, targetSpeed, maxLaneSpeed);
		}
		if(headway <= hBufferUpper && headway >= hBufferLower) {
			res = accOfCarFollowing(p);
		}
	}
	return res;
}

double sim_mob::MITSIM_CF_Model::breakToTargetSpeed(UpdateParams& p)
{
	double v 			=	p.currSpeed;
	double dt			=	p.elapsedSeconds;

	//NOTE: This is the only use of epsilon(), so I just copied the value directly.
	//      See LC_Model for how to declare a private temporary variable. ~Seth
	if(p.space_star > numeric_limits<double>::epsilon()) {
		return  ((p.v_lead + p.a_lead * dt ) * ( p.v_lead + p.a_lead * dt) - v * v) / 2 / p.space_star;
	} else if ( dt <= 0 ) {
		return maxAcceleration;
	} else {
		return ( p.v_lead + p.a_lead * dt - v ) / dt;
	}
}

double sim_mob::MITSIM_CF_Model::accOfEmergencyDecelerating(UpdateParams& p)
{
	double v 			=	p.currSpeed;
	double dv			=	v-p.v_lead;
	double epsilon_v	=	0.001;
	double aNormalDec	=	normalDeceleration;

	double a;
	if( dv < epsilon_v ) {
		a = p.a_lead + 0.25*aNormalDec;
	} else if (p.space > 0.01 ) {
		a = p.a_lead - dv * dv / 2 / p.space;
	} else {
		a= breakToTargetSpeed(p);
	}
//	if(a<maxDeceleration)
//		return maxDeceleration;
//	else if(a>maxAcceleration)
//		return maxAcceleration;
//	else
		return a;
}



double sim_mob::MITSIM_CF_Model::accOfCarFollowing(UpdateParams& p)
{
	const double density	=	1;		//represent the density of vehicles in front of the subject vehicle
										//now we ignore it, assuming that it is 1.
	double v				=	p.currSpeed;
	int i = (v > p.v_lead) ? 1 : 0;
	double dv =(v > p.v_lead)?(v-p.v_lead):(p.v_lead - v);

	double res = CF_parameters[i].alpha * pow(v , CF_parameters[i].beta) /pow(p.nvFwd.distance/100 , CF_parameters[i].gama);
	res *= pow(dv , CF_parameters[i].lambda)*pow(density,CF_parameters[i].rho);
	res += feet2Unit(nRandom(0,CF_parameters[i].stddev));

	return res;
}

double sim_mob::MITSIM_CF_Model::accOfFreeFlowing(UpdateParams& p, double targetSpeed, double maxLaneSpeed)
{
	double vn =	p.currSpeed;
	if (vn < targetSpeed) {
		return (vn<maxLaneSpeed) ? maxAcceleration : normalDeceleration;
	} else if (vn > targetSpeed) {
		return normalDeceleration;
	}

	//If equal:
	return (vn<maxLaneSpeed) ? maxAcceleration: 0;
}

double sim_mob::MITSIM_CF_Model::accOfMixOfCFandFF(UpdateParams& p, double targetSpeed, double maxLaneSpeed)
{
	p.distanceToNormalStop = p.currSpeed * p.currSpeed / 2 /(-normalDeceleration);
	if(p.space > p.distanceToNormalStop) {
		return accOfFreeFlowing(p, targetSpeed, maxLaneSpeed);
	} else {
		return breakToTargetSpeed(p);
	}
}
