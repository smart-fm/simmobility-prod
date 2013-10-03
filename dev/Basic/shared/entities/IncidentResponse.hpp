/*
 * IncidentResponse.hpp
 *
 *  Created on: Oct 2, 2013
 *      Author: zhang Huai Peng
 */

#include "geospatial/Incident.hpp"
#include "map"
#ifndef INCIDENTRESPONSE_HPP_
#define INCIDENTRESPONSE_HPP_

class timeslice;
namespace sim_mob {

class RoadSegment;
class IncidentResponse {
public:
	enum INCIDENTPLAN{INCIDENT_CLEARANCE, INCIDENT_SLOWDOWN_OR_STOP, INCIDENT_MANDATORYLANECHANGING };
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

private:
	std::map<unsigned int, const Incident*> currentIncidents;
	INCIDENTPLAN currentPlan;
};

} /* namespace sim_mob */
#endif /* INCIDENTRESPONSE_HPP_ */
