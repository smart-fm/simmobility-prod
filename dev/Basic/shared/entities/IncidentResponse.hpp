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
class IncidentResponse {
public:
	enum INCIDENTPLAN{INCIDENT_CLEARANCE, INCIDENT_CHANGELANE, INCIDENT_SLOWDOWN };
	IncidentResponse();
	virtual ~IncidentResponse();
	bool insertIncident(const Incident* inc);
	bool removeIncident(const Incident* inc);
	void makeResponsePlan(timeslice* now, const RoadSegment* currentRoad);
	INCIDENTPLAN getCurrentPlan() { return currentPlan; }
	void resetStatus();

public:
	int nextLaneIndex;
	float speedLimit;
	float speedLimitOthers;
	float distanceTo;
	unsigned int startFrameTick;
	unsigned int curFrameTick;
	LANE_CHANGE_SIDE laneSide;
	INCIDENTPLAN currentPlan;

private:
	std::map<unsigned int, const Incident*> currentIncidents;

};

} /* namespace sim_mob */
#endif /* INCIDENTRESPONSE_HPP_ */
