/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * CF_Model.cpp
 *
 *  Created on: 2011-8-15
 *      Author: wangxy & Li Zhemin
 */

#include "Driver.hpp"

#include <limits>

#include "entities/vehicle/Vehicle.hpp"

using std::numeric_limits;
using namespace sim_mob;

const sim_mob::Driver::CarFollowParam sim_mob::Driver::CF_parameters[2] = {
//        alpha   beta    gama    lambda  rho     stddev
		{ 0.0400, 0.7220, 0.2420, 0.6820, 0.6000, 0.8250},
		{-0.0418, 0.0000, 0.1510, 0.6840, 0.6800, 0.8020}
};

void sim_mob::Driver::updateLeadingGapandMode(UpdateParams& p)
{
	p.currSpeed = vehicle->velocity.getRelX()/100;
	LogOut("ID:" << parent->getId() << ":1111," << minCFDistance <<"," <<tsStopDistance << "," << minPedestrianDis << "\n");

	if(minCFDistance != 5000 && minCFDistance <= tsStopDistance && minCFDistance <= minPedestrianDis)
	{
		space = minCFDistance/100;
		mode = 0;
	}
	else if(minPedestrianDis != 5000 && minPedestrianDis <= minCFDistance && minPedestrianDis <= tsStopDistance)
	{
		space = minPedestrianDis/100;
		mode = 1;
	}
	else if(tsStopDistance != 5000 && tsStopDistance <= minPedestrianDis && tsStopDistance <= minCFDistance)
	{
		space = tsStopDistance/100;
		mode = 2;
	}
	else
	{
		space = tsStopDistance/100;//which should be default value 50m
		mode = 3;
	}

}
void sim_mob::Driver::makeAcceleratingDecision(UpdateParams& p)
{
	//currently convert back to m/s
//	speed_ = perceivedXVelocity_/100;
//	//in the case that perceivedXVelocity is not defined
//	if(speed_<0||speed_>50)

	//p.currSpeed = vehicle->xVel_/100;
	if(space <= 0) {
		acc_=0;
	}
	else{

		if(mode == 3) {

			acc_ = accOfFreeFlowing(p);
			return;
		} else if(mode == 0)
		{
			LogOut("ID:" << parent->getId() << "124");
			//v_lead 		=	CFD->getVehicle()->xVel_/100;
			//v_lead 		=	CFD->getVehicle()->velocity.getRelX()/100;
			v_lead = CFD->buffer_velocity.get().getRelX() / 100;
			//a_lead		=	CFD->getVehicle()->xAcc_/100;
			//a_lead          =   CFD->getVehicle()->accel.getRelX()/100;
			a_lead = CFD->buffer_accel.get().getRelX() / 100;
		}
		else
		{
			v_lead = 0;
			a_lead = 0;
		}
		double dt	=	timeStep;
		if (p.currSpeed == 0) {
			headway = 2 * space * 100000;
		} else {
			headway = 2*space / (p.currSpeed+p.currSpeed+timeStep*getMaxAcceleration());
		}
		space_star	=	space + v_lead * dt + 0.5 * a_lead * dt * dt;

		if(headway < hBufferLower) {
			LogOut("ID:" << parent->getId() << "126");
			acc_ = accOfEmergencyDecelerating(p);
		}
		if(headway > hBufferUpper) {
			LogOut("ID:" << parent->getId() << "127");
			acc_ = accOfMixOfCFandFF(p);
		}
		if(headway <= hBufferUpper && headway >= hBufferLower) {
			LogOut("ID:" << parent->getId() << "128");
			acc_ = accOfCarFollowing(p);
		}
	}
}

double sim_mob::Driver::breakToTargetSpeed(UpdateParams& p)
{
	double v 			=	p.currSpeed;
	double dt			=	timeStep;

	//NOTE: This is the only use of epsilon(), so I just copied the value directly.
	//      See LC_Model for how to declare a private temporary variable. ~Seth
	if( space_star > numeric_limits<double>::epsilon()) {
		return  ((v_lead + a_lead * dt ) * ( v_lead + a_lead * dt) - v * v) / 2 / space_star;
	} else if ( dt <= 0 ) {
		return MAX_ACCELERATION;
	} else {
		return ( v_lead + a_lead * dt - v ) / dt;
	}
}

double sim_mob::Driver::accOfEmergencyDecelerating(UpdateParams& p)
{
	double v 			=	p.currSpeed;
	double dv			=	v-v_lead;
	double epsilon_v	=	0.001;
	double aNormalDec	=	getNormalDeceleration();

	double a;
	if( dv < epsilon_v ) {
		a=a_lead + 0.25*aNormalDec;
	} else if ( space > 0.01 ) {
		a=a_lead - dv * dv / 2 / space;
	} else {
		a= breakToTargetSpeed(p);
	}
	if(a<maxDeceleration)
		return maxDeceleration;
	else if(a>maxAcceleration)
		return maxAcceleration;
	else
		return a;
}

double sim_mob::Driver::uRandom()
{
//	srand(time(0));
//	long int seed_=rand();

	int seed_ = getOwnRandomNumber();
	const long int M = 2147483647;  // M = modulus (2^31)
	const long int A = 48271;       // A = multiplier (was 16807)
	const long int Q = M / A;
	const long int R = M % A;
	seed_ = A * (seed_ % Q) - R * (seed_ / Q);
	seed_ = (seed_ > 0) ? (seed_) : (seed_ + M);
	return (double)seed_ / (double)M;
}

double sim_mob::Driver::nRandom(double mean,double stddev)
{
	   double r1 = uRandom(), r2 = uRandom();
	   double r = - 2.0 * log(r1);
	   if (r > 0.0) return (mean + stddev * sqrt(r) * sin(2 * 3.1415926 * r2));
	   else return (mean);
}

double sim_mob::Driver::accOfCarFollowing(UpdateParams& p)
{
	const double density	=	1;		//represent the density of vehicles in front of the subject vehicle
										//now we ignore it, assuming that it is 1.
	double v				=	p.currSpeed;
	int i = (v > v_lead) ? 1 : 0;
	double dv =(v > v_lead)?(v-v_lead):(v_lead - v);
	double acc_ = CF_parameters[i].alpha * pow(v , CF_parameters[i].beta) /pow(minCFDistance/100 , CF_parameters[i].gama);
	acc_ *= pow(dv , CF_parameters[i].lambda)*pow(density,CF_parameters[i].rho);
	acc_ += feet2Unit(nRandom(0,CF_parameters[i].stddev));

	return acc_;
}

double sim_mob::Driver::accOfFreeFlowing(UpdateParams& p)
{
	double vn =	p.currSpeed;
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

double sim_mob::Driver::accOfMixOfCFandFF(UpdateParams& p)		//mix of car following and free flowing
{
	distanceToNormalStop = p.currSpeed * p.currSpeed / 2 /(-getNormalDeceleration());
	if( space > distanceToNormalStop ) {
		return accOfFreeFlowing(p);
	} else {
		return breakToTargetSpeed(p);
	}
}
