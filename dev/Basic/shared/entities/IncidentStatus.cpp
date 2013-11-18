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
IncidentStatus::IncidentStatus() : currentStatus(INCIDENT_CLEARANCE), defaultSpeedLimit(0), speedLimitFactor(0), speedLimitFactorOthers(0), distanceTo(0), laneSide(LCS_SAME), changedLane(false), slowdownVelocity(false){
	// TODO Auto-generated constructor stub
	if(flags == 0) {
		flags = 1;
		srand (time(0));
		unsigned int signature = time(0);
		unsigned int s = 0xFF << (signature * 8);
		if (!(seed = s)) {
			seed = time(0);
		}
	}

	 randomNum = rand()/(RAND_MAX*1.0);
}

IncidentStatus::~IncidentStatus() {
	// TODO Auto-generated destructor stub
}

bool IncidentStatus::insertIncident(const Incident* inc){
	if( currentIncidents.count(inc->incidentId) > 0 ) {
		return false;
	}

	currentIncidents[inc->incidentId] = inc;
	if( speedLimitFactor<=0 ){
		speedLimitFactor = inc->speedlimit;
	}
	else{
		speedLimitFactor = std::min(speedLimitFactor, inc->speedlimit);
	}

	if( speedLimitFactorOthers<=0 ){
		speedLimitFactorOthers = inc->speedlimitOthers;
	}
	else{
		speedLimitFactorOthers = std::min(speedLimitFactorOthers, inc->speedlimitOthers);
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

void IncidentStatus::checkIsCleared(){

	if(currentIncidents.size() ==0 ){
		resetStatus();
	}
}


void IncidentStatus::resetStatus(){
	currentStatus = INCIDENT_CLEARANCE;
	nextLaneIndex = -1;
	speedLimitFactor = -1;
	speedLimitFactorOthers = -1;
	changedLane = false;
	slowdownVelocity = false;
}


} /* namespace sim_mob */
