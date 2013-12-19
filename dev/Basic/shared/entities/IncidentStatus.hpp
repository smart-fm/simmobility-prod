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
	enum IncidentStatusType{INCIDENT_CLEARANCE, INCIDENT_FULLYBLOCKING, INCIDENT_OCCURANCE_LANE, INCIDENT_ADJACENT_LANE };
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
      * check whether or not fully blocking happen
      * @param inc is incident object which hold detail description
      * @return blocking lane id .
      */
	int checkBlockingStatus(const Incident*inc);

   /**
	  * calculate whether the incident is bypassed
	  * @param forward means car take currently movement on the road segment
	  * @return current incident length .
	  */
	double reduceIncidentLength(float forward);

   /**
	  * the getter of current incident length
	  * @return current incident length .
	  */
	double getCurrentIncidentLength();

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
	void setNextLaneIndex(int value);
	int getNextLaneIndex();

    /**
      * the setter and getter for the property 'nextLaneIndex'
      */
	void setCurrentLaneIndex(int value);
	int getCurrentLaneIndex();

 	/**
	 * the setter for the property 'defaultSpeedLimit'
	 */
	void setDefaultSpeedLimit(float value);

	/**
	 * the getter for speed limit in incident lane
	 */
	float getSpeedLimit(unsigned int laneId);

    /**
      * the setter and getter for the property 'visibilityDist'
      */
	void setVisibilityDistance(float value);
	float getVisibilityDistance();

    /**
      * the setter and getter for the property 'distanceTo'
      */
	void setDistanceToIncident(float value);
	float getDistanceToIncident();

    /**
      * the setter and getter for the property 'randomNum'
      */
	void setRandomValue(float value);
	float getRandomValue();

    /**
      * the setter and getter for the property 'laneSide'
      */
	void setLaneSide(LANE_CHANGE_SIDE value);
	LANE_CHANGE_SIDE getLaneSide();

    /**
      * the setter and getter for the property 'changedLane'
      */
	void setChangedLane(bool value);
	bool getChangedLane();

    /**
      * the setter and getter for the property 'slowdownVelocity'
      */
	void setSlowdownVelocity(bool value);
	bool getSlowdownVelocity();

private:
	int currentLaneIndex;
	//record next lane index for lane changing model
	int nextLaneIndex;
	//default speed limit in current road segment
	float defaultSpeedLimit;
	//record visibility distance within which driver can see the incident
	float visibilityDist;
	//record real distance to the incident at current time
	float distanceTo;
	//record a random number for the reference to mandatory lane changing
	float randomNum;
	//record current incident length
	float incidentLength;
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
};

} /* namespace sim_mob */
#endif /* INCIDENTRESPONSE_HPP_ */
