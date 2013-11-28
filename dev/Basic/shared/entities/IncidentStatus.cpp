/*
 * IncidentResponse.cpp
 *
 *  Created on: Oct 2, 2013
 *      Author: zhang
 */

#include "IncidentStatus.hpp"
#include "util/Utils.hpp"
#include <cmath>

namespace sim_mob {


IncidentStatus::IncidentStatus() : currentStatus(INCIDENT_CLEARANCE), defaultSpeedLimit(0), distanceTo(0), laneSide(LCS_SAME), currentLaneIndex(0), nextLaneIndex(-1), lengthOfIncident(0), changedLane(false), slowdownVelocity(false){
	 randomNum = Utils::generateFloat(0, 1.0);
}

IncidentStatus::~IncidentStatus() {
	// TODO Auto-generated destructor stub
}

int IncidentStatus::checkBlockingStatus(const Incident*inc){
	int ret = -1;
	bool isFullyBlocking = true;
	for(std::vector<Incident::LaneItem>::const_iterator laneIt=inc->laneItems.begin(); laneIt!=inc->laneItems.end(); laneIt++){
		if((*laneIt).speedLimit > 0 ){
			isFullyBlocking = false;
			ret = (*laneIt).laneId;
			break;
		}
	}

	return ret;
}

float IncidentStatus::getSpeedLimit(unsigned int laneId) {
	if(currentIncidents.size()==0)
		return -1.0;

	float speedLimitFactor = -1.0;
	if(laneId < currentIncidents[0]->laneItems.size()){
		speedLimitFactor = currentIncidents[0]->laneItems[laneId].speedLimit;
	}

	return speedLimitFactor*defaultSpeedLimit;
}

double IncidentStatus::reduceIncidentLength(float forward){
	if(lengthOfIncident>0){
		lengthOfIncident -= forward;
	}
	return lengthOfIncident;
}
double IncidentStatus::getCurrentIncidentLength(){
	return lengthOfIncident;
}


bool IncidentStatus::insertIncident(const Incident* inc){

	bool ret = true;
	if( currentIncidents.count(inc->incidentId) > 0 ) {
		ret = false;
		return ret;
	}
	else {
		currentIncidents.insert(std::make_pair(inc->incidentId, inc));
	}

	int destinationLaneId = checkBlockingStatus(inc);

	currentStatus = INCIDENT_FULLYBLOCKING;
	lengthOfIncident = inc->length;

	if(destinationLaneId<0){
		slowdownVelocity = true;
		currentStatus = INCIDENT_OCCURANCE_LANE;
	}
	else if(destinationLaneId>=0){

		const float convertFactor = 100.0;
		if(getSpeedLimit(currentLaneIndex)/convertFactor<defaultSpeedLimit){
			slowdownVelocity = true;
		}

		if(destinationLaneId!=currentLaneIndex && getSpeedLimit(currentLaneIndex)==0){
			currentStatus = INCIDENT_OCCURANCE_LANE;
			nextLaneIndex = destinationLaneId;
			if( currentLaneIndex < nextLaneIndex){
				laneSide = LCS_LEFT;
			}
			else {
				laneSide = LCS_RIGHT;
			}
		}
		else {
			currentStatus = INCIDENT_ADJACENT_LANE;
		}
	}

	return ret;
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
	nextLaneIndex = -2;
	changedLane = false;
	slowdownVelocity = false;
}


} /* namespace sim_mob */
