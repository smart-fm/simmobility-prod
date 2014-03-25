/*
 * IncidentPerformer.hpp
 *
 *  Created on: Mar 25, 2014
 *      Author: zhang
 */

#include "entities/IncidentStatus.hpp"
#include "DriverUpdateParams.hpp"

#ifndef INCIDENTPERFORMER_HPP_
#define INCIDENTPERFORMER_HPP_

namespace sim_mob {

class Driver;
class IncidentPerformer {
public:
	IncidentPerformer();
	virtual ~IncidentPerformer();

	/**
	 * get incident status.
	 * @return incident status object.
	 */
	sim_mob::IncidentStatus& getIncidentStatus();

	/**
	 * Check current incident status.
	 * @return void .
	 */
	void checkIncidentStatus(Driver* parent, DriverUpdateParams& p, timeslice now);

	/**
	 * reponse incident.
	 * @return void .
	 */
	void responseIncidentStatus(Driver* parent, DriverUpdateParams& p, timeslice now);


private:
	//incident response plan
	sim_mob::IncidentStatus incidentStatus;

};

} /* namespace sim_mob */
#endif /* INCIDENTPERFORMER_HPP_ */
