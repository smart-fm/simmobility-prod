/*
 * IncrementalSharing.cpp
 *
 *  Created on: 14 Jul 2017
 *      Author: araldo
 */

#include <boost/graph/max_cardinality_matching.hpp>
#include "IncrementalSharing.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "path/PathSetManager.hpp"

using namespace sim_mob;

void IncrementalSharing::computeSchedules()
{
	if (availableDrivers.empty() || requestQueue.empty())
	{
		return;
	}

	const std::map<unsigned int, Node *> &nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();
	unsigned maxAggregatedRequests = 2; // Maximum number of requests that can be assigned to a driver
	double maxWaitingTime = 300; //seconds
	std::map<const Person *, Schedule> schedulesComputedSoFar; // We will put here the schedule that we will have constructed per each driver

	for (const Person *driver : availableDrivers)
	{
		const Node *driverNode = driver->exportServiceDriver()->getCurrentNode(); // the node in which the driver is currently located
		Schedule schedule;
		unsigned aggregatedRequests = 0; // Number of requests that we have aggregated so far

		for (std::list<TripRequestMessage>::iterator itReq = requestQueue.begin();
		     itReq != requestQueue.end() && aggregatedRequests < maxAggregatedRequests;)
		{
			// We now iterate over all the un-assigned requests and we see if we can assign them to this driver
			const TripRequestMessage request = *itReq;
			const Node *pickUpNode = nodeIdMap.find(request.startNodeId)->second;
			const Node *dropOffNode = nodeIdMap.find(request.destinationNodeId)->second;
			// We will set it to true if we observe that this request can be assigned to this driver
			bool isMatchingSuccessful = false;
			unsigned pickupIdx = 0;

			do
			{
				Schedule scheduleHypothesis = schedule;
				// To check if we can assign this request to this driver, we create tentative schedules (like this
				// scheduleHypothesis).
				// We will try to modify this tentative schedule. If we succeed, they will become the real schedule.
				// Otherwise, we will start again from the real schedule and try to insert in it the following requests

				ScheduleItem newPickup(PICKUP, request);
				// First, we have to find a feasible position in the schedule for the pickup of the current request.
				scheduleHypothesis.insert(scheduleHypothesis.begin() + pickupIdx, newPickup);

				double vehicleTime = evaluateSchedule(driverNode, scheduleHypothesis,
				                                      MobilityServiceController::toleratedExtraTime, maxWaitingTime);
				if (vehicleTime > 0)
				{
					// It is possible to insert the pick up. Now, I start from the current scheduleHypothesis, in which the pick up has been successfully inserted,
					// and I seek a feasible position for the dropoff
					unsigned dropoffIdx = pickupIdx + 1;
					do
					{
						Schedule scheduleHypothesis2 = scheduleHypothesis;
						ScheduleItem newDropoff(DROPOFF, request);
						scheduleHypothesis2.insert(scheduleHypothesis2.begin() + dropoffIdx, newDropoff);
						double vehicleTime = evaluateSchedule(driverNode, scheduleHypothesis2,
						                                      MobilityServiceController::toleratedExtraTime,
						                                      maxWaitingTime);
						if (vehicleTime > 0)
						{
							// I can also insert the dropoff. Perfect, I will make this successful scheduleHypothesis my schedule
							isMatchingSuccessful = true;
							schedule = scheduleHypothesis2;
							aggregatedRequests++;
							itReq = requestQueue.erase(itReq);
							// The request is matched now. I can eliminate it, so that I will not assign
							// it again
						}
						else
						{
							dropoffIdx++;
						} // I will try with the subsequent position
					}
					while (dropoffIdx <= schedule.size() && !isMatchingSuccessful);
					// Note: I am using <= and not < in the while condition, since it is possible to insert the dropoff at the end of the schedule
				}

				++pickupIdx;
				// If we arrived here, it means we did not find a feasible position in the schedule
				// for the dropoff. We can try by changin the position of
				// the pickup. If we find another feasible position for the pickup, it is possible that we will also find a feasible position
				// for a dropoff, given the new pickup position
			}
			while (pickupIdx < schedule.size() && !isMatchingSuccessful);
			// Note: I am using < and not <= in the while condition, since, if I insert the pickup at the end, it is like I am not really sharing this request with

			if (!isMatchingSuccessful)
			{
				// I did not remove the request from the requestQueue, and thus I have to advance the iterator manually
				itReq++;
			}
			// else I do nothing, as I did what was to be done, some lines before
		}

		// Now we are done with this driver.

#ifndef NDEBUG
		if (schedule.size() != aggregatedRequests * 2 )
		{
			throw std::runtime_error("There should be 2 schedule items (1 pickup + 1 dropoff) per each request inserted in this schedule. But it is not the case here.");
		}

		if (aggregatedRequests > maxAggregatedRequests)
		{
			throw std::runtime_error("The number of aggregated requests is incorrect");
		}

		if (schedulesComputedSoFar.find(driver) != schedulesComputedSoFar.end()  )
			throw std::runtime_error("Trying to assign more than one schedule to a single driver");
#endif

		if (!schedule.empty())
		{
			schedulesComputedSoFar.emplace(driver, schedule);
		}
	}

#ifndef NDEBUG
	for (const std::pair<const Person*, Schedule>& s : schedulesComputedSoFar)
	{
		const Person* driver = s.first;
		if (driverSchedules.find(driver) == driverSchedules.end()  )
		{
			throw std::runtime_error("Trying to assign a schedule to a driver that is already occupied as she received a schedule some schedule computation period ago"+
					std::string(" and she has not finished yet") );
		}
	}
	//
#endif

	// After we decided all the schedules for all the drivers, we can send  them
	for (const std::pair<const Person *, Schedule> &p : schedulesComputedSoFar)
	{
		const Person *driver = p.first;
		Schedule schedule = p.second;

		//Find where to park after the final drop off
		unsigned int finalDropOff = schedule.back().tripRequest.destinationNodeId;
		const RoadNetwork *rdNetowrk = RoadNetwork::getInstance();
		const Node *finalDropOffNode = rdNetowrk->getById(rdNetowrk->getMapOfIdvsNodes(), finalDropOff);
		const SMSVehicleParking *parking =
				SMSVehicleParking::smsParkingRTree.searchNearestObject(finalDropOffNode->getPosX(), finalDropOffNode->getPosY());

		if(parking)
		{
			//Append the parking schedule item to the end
			const ScheduleItem parkingSchedule(ScheduleItemType::PARK, parking);
			schedule.push_back(parkingSchedule);
		}

		assignSchedule(driver, schedule);
	}
}

