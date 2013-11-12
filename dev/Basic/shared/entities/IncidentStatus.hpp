/*
 * IncidentResponse.hpp
 *
 *  Created on: Oct 2, 2013
 *      Author: zhang Huai Peng
 */

#include "geospatial/Incident.hpp"
#include "geospatial/Lane.hpp"
#include "map"
#ifndef INCIDENTRESPONSE_HPP_
#define INCIDENTRESPONSE_HPP_

class timeslice;
namespace sim_mob {

class RoadSegment;
class IncidentStatus {
public:
	enum IncidentStatusType{INCIDENT_CLEARANCE, INCIDENT_OCCURANCE_LANE, INCIDENT_ADJACENT_LANE };
	IncidentStatus();
	virtual ~IncidentStatus();

    /**
      * insert a new incident obstacle into this class
      * @param inc is incident obstacle.
      * @return true if inserting successfully .
      */
	bool insertIncident(const Incident* inc);

    /**
      * remove a old incident obstacle from this class
      * @param inc is incident obstacle.
      * @return true if removing successfully .
      */
	bool removeIncident(const Incident* inc);

    /**
      * check whether all incident obstacles are cleared already
      * @return true if removing successfully .
      */
	void checkIsCleared();

    /**
      * the setter and getter for the property 'currentStatus'
      * @return current incident status .
      */
	IncidentStatusType getCurrentStatus() {
		return currentStatus;
	}
	void setCurrentStatus(IncidentStatusType value) {
		currentStatus = value;
	}

    /**
      * reset member parameters to default value.
      * @return void .
      */
	void resetStatus();

    /**
      * the setter and getter for the property 'nextLaneIndex'
      */
	void setNextLaneIndex(int value) {
		nextLaneIndex = value;
	}
	int getNextLaneIndex() {
		return nextLaneIndex;
	}

    /**
      * the setter and getter for the property 'speedLimit'
      */
	void setSpeedLimit(float value) {
		speedLimit = value;
	}
	float getSpeedLimit() {
		return speedLimit;
	}

    /**
      * the setter and getter for the property 'speedLimitOthers'
      */
	void setSpeedLimitOthers(float value) {
		speedLimitOthers = value;
	}
	float getSpeedLimitOthers() {
		return speedLimitOthers;
	}

    /**
      * the setter and getter for the property 'visibilityDist'
      */
	void setVisibilityDistance(float value) {
		visibilityDist = value;
	}
	float getVisibilityDistance() {
		return visibilityDist;
	}

    /**
      * the setter and getter for the property 'distanceTo'
      */
	void setDistanceToIncident(float value) {
		distanceTo = value;
	}
	float getDistanceToIncident() {
		return distanceTo;
	}

    /**
      * the setter and getter for the property 'randomNum'
      */
	void setRandomValue(float value) {
		randomNum = value;
	}
	float getRandomValue() {
		return randomNum;
	}

    /**
      * the setter and getter for the property 'laneSide'
      */
	void setLaneSide(LANE_CHANGE_SIDE value) {
		laneSide = value;
	}
	LANE_CHANGE_SIDE getLaneSide() {
		return laneSide;
	}

    /**
      * the setter and getter for the property 'changedLane'
      */
	void setChangedLane(bool value) {
		changedLane = value;
	}
	bool getChangedLane() {
		return changedLane;
	}

    /**
      * the setter and getter for the property 'slowdownVelocity'
      */
	void setSlowdownVelocity(bool value) {
		slowdownVelocity = value;
	}
	bool getSlowdownVelocity() {
		return slowdownVelocity;
	}

private:
	//record next lane index for lane changing model
	int nextLaneIndex;
	//record speed limit defined in incident lane
	float speedLimit;
	//record speed limit defined in adjacent lane to incident
	float speedLimitOthers;
	//record visibility distance within which driver can see the incident
	float visibilityDist;
	//record real distance to the incident at current time
	float distanceTo;
	//record a random number for the reference to mandatory lane changing
	float randomNum;
	//record lane changing direction(left or right) for lane changing model
	LANE_CHANGE_SIDE laneSide;
	//record current detection result when incident happen
	IncidentStatusType currentStatus;
	//record the lane changing decision
	bool changedLane;
	//record the decision of slowing down velocity
	bool slowdownVelocity;
	//record incident objects to help make decision
	std::map<unsigned int, const Incident*> currentIncidents;
	//initialization flag;
	static unsigned int flags;
	//initial random seed
	long seed;
};

} /* namespace sim_mob */
#endif /* INCIDENTRESPONSE_HPP_ */
