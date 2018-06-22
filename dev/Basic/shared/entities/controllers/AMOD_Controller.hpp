//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "OnCallController.hpp"

namespace sim_mob
{

class AMOD_Controller : public OnCallController
{
private:
	/**Stores the single ride requests*/
	std::list<TripRequestMessage> singleRideRequests;

	/**Stores the shared ride requests*/
	std::list<TripRequestMessage> sharedRideRequests;


	/**
	 * The set of drivers which are serving the shared requests and have capacity to serve
	 * additional requests
	 */
	// Taking this variable to onCallController
	//std::set<const Person *> driversServingSharedReq;


protected:
	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	virtual void computeSchedules();

	/**
	 * Separates the trip requests based on the type and adds them to the respective lists
	 */
	void separateSharedAndSingleRequests();

	/**
	 * Tries to match unassigned requests (for shared rides) to drivers that are currently
	 * serving other shared requests but have capacity to accommodate additional passengers
	 */
	void matchDriversServingSharedReq();

	/**
	 * Matches the single rider requests to the nearest available driver
	 */
	void matchSingleRiderReq();

	/**
	 * Iterates over all the un-assigned requests and check if they can be added to schedule. If yes it
	 * returns the new schedule, with the added requests
	 */
	Schedule buildSchedule(unsigned int maxAggregatedRequests, const double maxWaitingTime, const Node *driverNode,
	                       Schedule schedule, unsigned int *aggregatedRequests);

public:
	AMOD_Controller(const MutexStrategy &mtx, unsigned int computationPeriod, unsigned int id, std::string tripSupportMode_,
	                TT_EstimateType tt_estType,unsigned maxAggregatedRequests_,bool studyAreaEnabledController_,unsigned int toleratedExtraTime,unsigned int maxWaitingTime,bool parkingEnabled);
};

}