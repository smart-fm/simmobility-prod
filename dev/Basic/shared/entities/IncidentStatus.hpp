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
	enum INCIDENTPLAN{INCIDENT_CLEARANCE, INCIDENT_CHANGELANE, INCIDENT_SLOWDOWN };
	IncidentStatus();
	virtual ~IncidentStatus();
	bool insertIncident(const Incident* inc);
	bool removeIncident(const Incident* inc);
	void checkIsCleared(timeslice* now, const RoadSegment* currentRoad);
	INCIDENTPLAN getCurrentStatus() { return currentPlan; }
	void resetStatus();
	int urandom(double prob);

public:
	int nextLaneIndex;
	float speedLimit;
	float speedLimitOthers;
	float lastSpeed;
	float lastAccel;
	float visibilityDist;
	float distanceTo;
	unsigned int startFrameTick;
	unsigned int curFrameTick;
	LANE_CHANGE_SIDE laneSide;
	INCIDENTPLAN currentPlan;
	bool changedlane;
private:
	std::map<unsigned int, const Incident*> currentIncidents;
	long seed;
	static unsigned int flags;
	unsigned int signature;
};

} /* namespace sim_mob */
#endif /* INCIDENTRESPONSE_HPP_ */
