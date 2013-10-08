/*
 * Incident.h
 *
 *  Created on: Sep 30, 2013
 *      Author: zhang huai peng
 */
#include "geospatial/RoadItem.hpp"

#ifndef INCIDENT_H_
#define INCIDENT_H_

namespace sim_mob {

class Incident : public sim_mob::RoadItem{

public:
	unsigned int incidentId;
	float visibilityDistance;
	unsigned int segmentId;
	float position;
	unsigned int severity;
	float capFactor;
	unsigned int startTime;
	unsigned int duration;
	float speedlimit;
	float speedlimitOthers;
	unsigned int laneId;
	float compliance;
	float accessibility;

public:

	explicit Incident() : RoadItem(), incidentId(0), visibilityDistance(0.0),
			segmentId(0), position(0.0), severity(0.0), capFactor(0.0), startTime(0), duration(0), speedlimit(0.0), laneId(0), compliance(0), accessibility(0)
	{;}

	virtual ~Incident();
};

} /* namespace sim_mob */
#endif /* INCIDENT_H_ */
