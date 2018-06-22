/*
 * IncrementalSharing.h
 *
 *  Created on: 14 Jul 2017
 *      Author: araldo
 */

#ifndef ENTITIES_CONTROLLERS_INCREMENTALSHARING_HPP_
#define ENTITIES_CONTROLLERS_INCREMENTALSHARING_HPP_

#include "OnCallController.hpp"


namespace sim_mob
{

class IncrementalSharing : public OnCallController
{
public:
	IncrementalSharing(const MutexStrategy &mtxStrat, unsigned int computationPeriod, unsigned id, std::string tripSupportMode_,
	                   TT_EstimateType ttEstimateType, unsigned maxAggregatedRequests_,bool studyAreaEnabledController_,
                       unsigned int toleratedExtraTime,unsigned int maxWaitingTime,bool parkingEnabled) : OnCallController(mtxStrat, computationPeriod,
	                                                                      MobilityServiceControllerType::SERVICE_CONTROLLER_INCREMENTAL,
	                                                                      id, tripSupportMode_, ttEstimateType,maxAggregatedRequests_,studyAreaEnabledController_,toleratedExtraTime,maxWaitingTime,parkingEnabled)
	{
	}

	virtual ~IncrementalSharing()
	{
	}

	//virtual void checkSequence(const std::string &sequence) const;

	// Inherits from the parent
	//virtual void sendCruiseCommand(const Person *driver, const Node *nodeToCruiseTo, const timeslice currTick) const;


#ifndef NDEBUG
	// Overrides the parent method
	//aa!!: Why is it disabled?
	//virtual void consistencyChecks(const std::string& label) const;
#endif

protected:
	//virtual const Node *getCurrentNode(Person *p);

	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	virtual void computeSchedules();

	/**
	 * This method attempts to match drivers who are ferrying a single passenger to other requests
	 * If a match is found, the driver's schedule is updated, and is removed from the partially avaliable list
	 */
	void matchPartiallyAvailableDrivers();

	/**
	 * Iterates over all the un-assigned requests and check if they can be added to schedule. If yes it
	 * returns the new schedule, with the added requests
	 */
	Schedule buildSchedule(unsigned int maxAggregatedRequests, double maxWaitingTime, const Node *driverNode,
	                       Schedule schedule, unsigned int *aggregatedRequests);
};
}

#endif /* ENTITIES_CONTROLLERS_INCREMENTALSHARING_HPP_ */
