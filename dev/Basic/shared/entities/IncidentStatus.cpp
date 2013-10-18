/*
 * IncidentResponse.cpp
 *
 *  Created on: Oct 2, 2013
 *      Author: zhang
 */

#include "IncidentStatus.hpp"
#include <cmath>

namespace sim_mob {

unsigned int IncidentStatus::flags = 0;
IncidentStatus::IncidentStatus() : currentPlan(INCIDENT_CLEARANCE), startFrameTick(0), randomStep(1.0),  curFrameTick(0), speedLimit(0), speedLimitOthers(0), distanceTo(0), lastSpeed(0), laneSide(LCS_SAME), changedlane(false) {
	// TODO Auto-generated constructor stub
	static int counter = 0;
	signature = counter;
	counter ++;
	unsigned int s = 0xFF << (signature * 8);
	if (!(seed = (flags & s))) {
		seed = time(0);
	}
}

IncidentStatus::~IncidentStatus() {
	// TODO Auto-generated destructor stub
}

bool IncidentStatus::insertIncident(const Incident* inc){
	if( currentIncidents.count(inc->incidentId) > 0 ) {
		return false;
	}

	currentIncidents[inc->incidentId] = inc;
	if( speedLimit<=0 ){
		speedLimit = inc->speedlimit;
	}
	else{
		speedLimit = std::min(speedLimit, inc->speedlimit);
	}

	if( speedLimitOthers<=0 ){
		speedLimitOthers = inc->speedlimitOthers;
	}
	else{
		speedLimitOthers = std::min(speedLimitOthers, inc->speedlimitOthers);
	}

	return true;
}

bool IncidentStatus::removeIncident(const Incident* inc){

	for(std::map<unsigned int, const Incident*>::iterator incIt = currentIncidents.begin(); incIt != currentIncidents.end(); incIt++) {
		if( (*incIt).first == inc->incidentId ){
			currentIncidents.erase(inc->incidentId);
			return true;
		}
	}

	return false;
}

void IncidentStatus::checkIsCleared(timeslice* now, const RoadSegment* currentRoad){

	if(currentIncidents.size() ==0 ){
		resetStatus();
	}
}

int IncidentStatus::urandom(double prob){

	const long int M = 2147483647;  // M = modulus (2^31)
	const long int A = 48271;       // A = multiplier (was 16807)
	const long int Q = M / A;
	const long int R = M % A;

	seed = A * (seed % Q) - R * (seed / Q);
	seed = (seed > 0) ? (seed) : (seed + M);

	//double ret = (double)seed / (double)M;

	double ret = rand()/(RAND_MAX*1.0);

	if(ret < prob) return (1);
	else return 0;
}


void IncidentStatus::resetStatus(){
	currentPlan = INCIDENT_CLEARANCE;
	nextLaneIndex = -1;
	speedLimit = -1;
	randomStep = 1.0;
	changedlane = false;
	//startFrameTick = 0;
	//curFrameTick = 0;
}


} /* namespace sim_mob */
