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
	enum IncidentStatusType {
		INCIDENT_CLEARANCE,
		INCIDENT_FULLYBLOCKING,
		INCIDENT_OCCURANCE_LANE,
		INCIDENT_ADJACENT_LANE
	};
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
	 * the getter for the property 'currentStatus'
	 * @return current incident status .
	 */
	IncidentStatusType getCurrentStatus();

	/**
	 * the setter for the property 'currentStatus'
	 * @return void .
	 */
	void setCurrentStatus(IncidentStatusType value);

	/**
	 * reset member parameters to default value.
	 * @return void .
	 */
	void resetStatus();

	/**
	 * the getter for the property 'nextLaneIndex'
	 * return the index of next adjacent lane to incident
	 */
	int getNextLaneIndex();

	/**
	 * the setter for the property 'nextLaneIndex'
	 * return void
	 */
	void setNextLaneIndex(int value);

	/**
	 * the setter and getter for the property 'nextLaneIndex'
	 * return void
	 */
	void setCurrentLaneIndex(int value);

	/**
	 * the getter for the property 'nextLaneIndex'
	 * return current lane index
	 */
	int getCurrentLaneIndex();

	/**
	 * the setter for the property 'defaultSpeedLimit'
	 * return void
	 */
	void setDefaultSpeedLimit(float value);

	/**
	 * the getter for speed limit in incident lane
	 * return -1.0 when laneId is invalid or speed limit is not defined in a given lane, otherwise return correct speed limit
	 */
	float getSpeedLimit(unsigned int laneId);

	/**
	 * the setter for the property 'visibilityDist'
	 * return void
	 */
	void setVisibilityDistance(float value);

	/**
	 * the getter for the property 'visibilityDist'
	 * return visibility defined in incident object
	 */
	float getVisibilityDistance();

	/**
	 * the setter for the property 'distanceTo'
	 */
	void setDistanceToIncident(float value);

	/**
	 * the getter for the property 'distanceTo'
	 * return the distance to incident object
	 */
	float getDistanceToIncident();

	/**
	 * the setter for the property 'randomNum'
	 */
	void setRandomValue(float value);

	/**
	 * the getter for the property 'randomNum'
	 * return a random number for the lane changing decision
	 */
	float getRandomValue();

	/**
	 * the setter for the property 'laneSide'
	 * return void
	 */
	void setLaneSide(LANE_CHANGE_SIDE value);

	/**
	 * the  getter for the property 'laneSide'
	 * return lane side which is left or right
	 */
	LANE_CHANGE_SIDE getLaneSide();

	/**
	 * the setter for the property 'changedLane'
	 * return void
	 */
	void setChangedLane(bool value);

	/**
	 * the getter for the property 'changedLane'
	 * return lane changing decision
	 */
	bool getChangedLane();

	/**
	 * the setter for the property 'slowdownVelocity'
	 * return void
	 */
	void setSlowdownVelocity(bool value);

	/**
	 * the getter for the property 'slowdownVelocity'
	 * return slowing down velocity decision
	 */
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
