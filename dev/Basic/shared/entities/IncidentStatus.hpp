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
	enum INCIDENTSTATUS{INCIDENT_CLEARANCE, INCIDENT_OCCURANCE_LANE, INCIDENT_ADJACENT_LANE };
	IncidentStatus();
	virtual ~IncidentStatus();
	bool insertIncident(const Incident* inc);
	bool removeIncident(const Incident* inc);
	void checkIsCleared(timeslice* now, const RoadSegment* currentRoad);
	INCIDENTSTATUS getCurrentStatus() { return currentStatus; }
	void resetStatus();
	double urandom();

public:
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
	INCIDENTSTATUS currentStatus;
	//record the lane changing decision
	bool changedlane;
	//record the decision of slowing down velocity
	bool slowdownVelocity;

private:
	//record incident objects to help make decision
	std::map<unsigned int, const Incident*> currentIncidents;
	//initialization flag;
	static unsigned int flags;
	//initial random seed
	long seed;
};

} /* namespace sim_mob */
#endif /* INCIDENTRESPONSE_HPP_ */
