/*
 * IncidentPerformer.cpp
 *
 *  Created on: Mar 25, 2014
 *      Author: zhang
 */

#include "IncidentPerformer.hpp"
#include "Driver.hpp"

namespace sim_mob {


//the minimum speed when approaching to incident
const float APPROACHING_SPEED = 200;

IncidentPerformer::IncidentPerformer() {
	// TODO Auto-generated constructor stub

}

IncidentPerformer::~IncidentPerformer() {
	// TODO Auto-generated destructor stub
}

sim_mob::IncidentStatus& IncidentPerformer::getIncidentStatus(){
	return incidentStatus;
}

void sim_mob::IncidentPerformer::responseIncidentStatus(Driver* parentDriver, DriverUpdateParams& p, timeslice now) {
	//slow down velocity when driver views the incident within the visibility distance
	float incidentGap = parentDriver->getVehicle()->length;
	if(incidentStatus.getSlowdownVelocity()){
		//calculate the distance to the nearest front vehicle, if no front vehicle exists, the distance is given to a enough large gap as 5 kilometers
		float fwdCarDist = 5000;
		if( p.nvFwd.exists() ){
			DPoint dFwd = p.nvFwd.driver->getVehicle()->getPosition();
			DPoint dCur = parentDriver->getVehicle()->getPosition();
			DynamicVector movementVect(dFwd.x, dFwd.y, dCur.x, dCur.y);
			fwdCarDist = movementVect.getMagnitude()-parentDriver->getVehicle()->length;
			if(fwdCarDist < 0) {
				fwdCarDist = parentDriver->getVehicle()->length;
			}
		}

		//record speed limit for current vehicle
		float speedLimit = 0;
		//record current speed
		float newSpeed = 0;
		//record approaching speed when it is near to incident position
		float approachingSpeed = APPROACHING_SPEED;
		float oldDistToStop = p.perceivedDistToFwdCar;
		LANE_CHANGE_SIDE oldDirect = p.turningDirection;
		p.perceivedDistToFwdCar = std::min(incidentStatus.getDistanceToIncident(), fwdCarDist);
		p.turningDirection = LCS_LEFT;

		//retrieve speed limit decided by whether or not incident lane or adjacent lane
		speedLimit = incidentStatus.getSpeedLimit(p.currLaneIndex);
		if(speedLimit==0 && incidentStatus.getDistanceToIncident()>incidentGap)
			speedLimit = approachingSpeed;

		// recalculate acceleration and velocity when incident happen
		float newFwdAcc = 0;
		if(parentDriver->getVehicle()->getVelocity() > speedLimit){
			DriverMovement* driverMovement = dynamic_cast<DriverMovement*>(parentDriver->Movement());
			newFwdAcc = driverMovement->getCarFollowModel()->makeAcceleratingDecision(p, speedLimit, driverMovement->maxLaneSpeed);
			newSpeed = parentDriver->getVehicle()->getVelocity()+newFwdAcc*p.elapsedSeconds*100;
			if(newSpeed < speedLimit){
				newFwdAcc = 0;
				newSpeed = speedLimit;
			}
		}
		else {
			newFwdAcc = 0;
			newSpeed = speedLimit;
		}

		//update current velocity so as to response the speed limit defined in incident lane.
		parentDriver->getVehicle()->setVelocity(newSpeed);
		p.perceivedDistToFwdCar = oldDistToStop;
		p.turningDirection = oldDirect;
	}

	//stop cars when it already is near the incident location
	if(incidentStatus.getCurrentStatus() == IncidentStatus::INCIDENT_OCCURANCE_LANE ){
		if(incidentStatus.getSpeedLimit(p.currLaneIndex)==0 && incidentStatus.getDistanceToIncident()<incidentGap) {
			parentDriver->getVehicle()->setVelocity(0);
			parentDriver->getVehicle()->setAcceleration(0);
		}
	}

	if(p.nvFwd.exists() ){//avoid cars stacking together
		DPoint dFwd = p.nvFwd.driver->getVehicle()->getPosition();
		DPoint dCur = parentDriver->getVehicle()->getPosition();
		DynamicVector movementVect(dFwd.x, dFwd.y, dCur.x, dCur.y);
		double len = parentDriver->getVehicle()->length;
		double dist = movementVect.getMagnitude();
		if( dist < len){
			parentDriver->getVehicle()->setVelocity(0);
			parentDriver->getVehicle()->setAcceleration(0);
		}
	}
}

void sim_mob::IncidentPerformer::checkIncidentStatus(Driver* parentDriver, DriverUpdateParams& p, timeslice now) {

	const RoadSegment* curSegment = parentDriver->getVehicle()->getCurrSegment();
	const Lane* curLane = parentDriver->getVehicle()->getCurrLane();
	int curLaneIndex = curLane->getLaneID() - curSegment->getLanes().at(0)->getLaneID();
	if(curLaneIndex<0){
		return;
	}

	int nextLaneIndex = curLaneIndex;
	LANE_CHANGE_SIDE laneSide = LCS_SAME;
	IncidentStatus::IncidentStatusType status = IncidentStatus::INCIDENT_CLEARANCE;
	incidentStatus.setDistanceToIncident(0);
	const float convertFactor = 1000.0/3600.0;
	incidentStatus.setDefaultSpeedLimit(curSegment->maxSpeed*convertFactor);

	const std::map<centimeter_t, const RoadItem*> obstacles = curSegment->getObstacles();
	std::map<centimeter_t, const RoadItem*>::const_iterator obsIt;
	double realDist = 0;
	bool replan = false;
	DriverMovement* driverMovement = dynamic_cast<DriverMovement*>(parentDriver->Movement());
	const RoadItem* roadItem = driverMovement->getRoadItemByDistance(sim_mob::INCIDENT, realDist);
	if(roadItem) {//retrieve front incident obstacle
		const Incident* incidentObj = dynamic_cast<const Incident*>( roadItem );

		if(incidentObj){
			float visibility = incidentObj->visibilityDistance;
			incidentStatus.setVisibilityDistance(visibility);
			incidentStatus.setCurrentLaneIndex(curLaneIndex);

			if( (now.ms() >= incidentObj->startTime) && (now.ms() < incidentObj->startTime+incidentObj->duration) && realDist<visibility){
				incidentStatus.setDistanceToIncident(realDist);
				replan = incidentStatus.insertIncident(incidentObj);
				float incidentGap = parentDriver->getVehicle()->length*2;
				if(!incidentStatus.getChangedLane() && incidentStatus.getCurrentStatus()==IncidentStatus::INCIDENT_OCCURANCE_LANE){
					double prob = incidentStatus.getVisibilityDistance()>0 ? incidentStatus.getDistanceToIncident()/incidentStatus.getVisibilityDistance() : 0.0;
					if(incidentStatus.getDistanceToIncident() < 2*incidentGap){
						incidentStatus.setChangedLane(true);
					}
					else {
						if(prob < incidentStatus.getRandomValue()) {
							incidentStatus.setChangedLane(true);
						}
					}
				}
			}
			else if( now.ms()>incidentObj->startTime+incidentObj->duration ){// if incident duration is over, the incident obstacle will be removed
				replan = incidentStatus.removeIncident(incidentObj);
			}
		}
	}
	else {//if vehicle is going beyond this incident obstacle, this one will be removed
		for(obsIt=obstacles.begin(); obsIt!=obstacles.end(); obsIt++){
			const Incident* inc = dynamic_cast<const Incident*>( (*obsIt).second );
			if(inc){
				replan = incidentStatus.removeIncident(inc);
			}
		}
	}

	if(replan){//update decision status for incident.
		incidentStatus.checkIsCleared();
	}
}



} /* namespace sim_mob */
