//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AMOD_Controller.hpp"

using namespace sim_mob;
using namespace std;

AMOD_Controller::AMOD_Controller(const MutexStrategy &mtx, unsigned int computationPeriod, unsigned int id, std::string tripSupportMode_,
                                 TT_EstimateType tt_estType, unsigned maxAggregatedRequests_, bool studyAreaEnabledController_,unsigned int toleratedExtraTime,unsigned int maxWaitingTime,bool parkingEnabled)
        : OnCallController(mtx, computationPeriod,SERVICE_CONTROLLER_AMOD, id, tripSupportMode_,tt_estType,maxAggregatedRequests_,studyAreaEnabledController_, toleratedExtraTime,maxWaitingTime,parkingEnabled)
{}

void AMOD_Controller::computeSchedules()
{
#ifndef NDEBUG
	consistencyChecks("Begin - AMOD_Controller::computeSchedules");
#endif

	//Separate the shared and single ride requests
	separateSharedAndSingleRequests();

	//Assign the shared trip requests

	//First try to match the requests with drivers that are already serving shared requests
	if (!driversServingSharedReq.empty())
	{
		matchDriversServingSharedReq();
	}

	//Now try to assign the available drivers to the shared ride requests
	if (availableDrivers.empty())
	{
		return;
	}

	std::unordered_map<const Person *, Schedule> schedulesComputedSoFar;

	for (const Person *driver : availableDrivers)
	{
		//The node in which the driver is currently located
		const Node *driverNode = driver->exportServiceDriver()->getCurrentNode();
		Schedule schedule;

		// Number of requests that we have aggregated so far
		unsigned aggregatedRequests = 0;

		schedule = buildSchedule(maxAggregatedRequests, maxWaitingTime, driverNode, schedule, &aggregatedRequests);

#ifndef NDEBUG
		if (schedule.size() != aggregatedRequests * 2 )
		{
			throw std::runtime_error("There should be 2 schedule items (1 pickup + 1 dropoff) per each request \
			inserted in this schedule. But it is not the case here.");
		}

		if (aggregatedRequests > maxAggregatedRequests)
		{
			throw std::runtime_error("The number of aggregated requests is incorrect");
		}

		if (schedulesComputedSoFar.find(driver) != schedulesComputedSoFar.end()  )
		{
			throw std::runtime_error("Trying to assign more than one schedule to a single driver");
		}
#endif

		if (!schedule.empty())
		{
			schedulesComputedSoFar.emplace(driver, schedule);

			//If this driver wasn't assigned a schedule with more than 1 person, we add it to the
			//set of drivers serving shared requests that can still be assigned another passenger
			if(schedule.size() <= 2 && !parkingEnabled)
			{
				driversServingSharedReq.insert(driver);
			}
            else if(schedule.size()<=3)
            {
                driversServingSharedReq.insert(driver);
            }
		}
	}

	assignSchedules(schedulesComputedSoFar);

	//Assign the single rider trip requests
	if(!availableDrivers.empty())
	{
		matchSingleRiderReq();
	}

#ifndef NDEBUG
	consistencyChecks("End - AMOD_Controller::computeSchedules");
#endif
}

void AMOD_Controller::separateSharedAndSingleRequests()
{
	//Iterate through the newly added requests and separate them into shared/single
	for(auto it : requestQueue)
	{
		if(it.requestType == RequestType::TRIP_REQUEST_SINGLE)
		{
			singleRideRequests.push_back(it);
		}
		else if(it.requestType == RequestType::TRIP_REQUEST_SHARED)
		{
			sharedRideRequests.push_back(it);
		}
		else
		{
			stringstream msg;
			msg << "Requests sent to AMOD controller must be of type TRIP_REQUEST_SINGLE or TRIP_REQUEST_SHARED."
			    << "Request sent by person " << it.userId << " does not specify type, and will be discarded."
			    << endl;

			Warn() << msg.str();
		}
	}

	requestQueue.clear();
}

void AMOD_Controller::matchDriversServingSharedReq()
{
	{
		unsigned maxAggRequests = maxAggregatedRequests- 1;

		//This will contain the constructed schedule for every driver
		std::unordered_map<const Person *, Schedule> schedulesComputedSoFar;

		auto driver_Iter = driversServingSharedReq.begin();
		while (driver_Iter!=driversServingSharedReq.end())
		{
			const Person* driver = *driver_Iter;
			//The node in which the driver is currently located
			const Node *driverNode = driver->exportServiceDriver()->getCurrentNode();

#ifndef NDEBUG
			ControllerLog() << "matchDriversServingSharedReq(): driverSchedules.size() = "
                           << driverSchedules.size() << endl;
#endif
			//Get the schedule assigned to the driver
			Schedule orgSchedule = driverSchedules[driver];

#ifndef NDEBUG
			ControllerLog() << "matchDriversServingSharedReq(): driverSchedules.size() = "
                           << driverSchedules.size() << endl;
#endif

			if(!orgSchedule.empty() && orgSchedule.back().scheduleItemType == PARK)
			{
				//Remove the parking schedule item
				orgSchedule.pop_back();
			}

			if(orgSchedule.empty())
			{
				++driver_Iter;
				continue;
			}

			unsigned aggregatedRequests = 0;

			Schedule schedule;
			schedule = buildSchedule(maxAggRequests, maxWaitingTime, driverNode, orgSchedule, &aggregatedRequests);

#ifndef NDEBUG
			if (aggregatedRequests > (maxAggregatedRequests + 1))
           {
               throw std::runtime_error("The number of aggregated requests is incorrect");
           }

           if (schedulesComputedSoFar.find(driver) != schedulesComputedSoFar.end())
           {
               throw std::runtime_error("Trying to assign more than one schedule to a single driver");
           }
#endif

			if (schedule.size() != orgSchedule.size() && !schedule.empty())
			{
				schedulesComputedSoFar.emplace(driver, schedule);

				//Remove the driver from the set of drivers serving shared requests that can take
				//additional passengers, as this drivers capacity is full
				driversServingSharedReq.erase(driver_Iter++);
			}
			else
			{
				++driver_Iter;
			}
		}

#ifndef NDEBUG
		for (const std::pair<const Person *, Schedule> &s : schedulesComputedSoFar)
       {
           const Person *driver = s.first;
           if (driverSchedules.find(driver) == driverSchedules.end())
           {
               stringstream msg;
               msg << "Trying to assign a schedule to a driver that is already occupied as it "
                   << "has a non-empty schedule";
               throw std::runtime_error(msg.str());
           }
       }
#endif

		assignSchedules(schedulesComputedSoFar, true);
	}
}

void AMOD_Controller::matchSingleRiderReq()
{
	auto request = singleRideRequests.begin();

	while (request != singleRideRequests.end())
	{
		//Assign the start node
		const Node *startNode = (*request).startNode;
		const Person *feasibleDriver = findClosestDriver(startNode);
		for (const Person *driver : availableDrivers)
		{
			const Node *driverNode = driver->exportServiceDriver()->getCurrentNode(); 
			if (getTT(driverNode, startNode, ttEstimateType) <= maxWaitingTime &&
					(isCruising(driver) || isParked(driver))) {
				feasibleDriver = driver;
				break;
			}
		}

		if (feasibleDriver)
		{
            Schedule schedule;
			const ScheduleItem pickUpScheduleItem(PICKUP, *request);
			const ScheduleItem dropOffScheduleItem(DROPOFF, *request);

			schedule.push_back(pickUpScheduleItem);
			schedule.push_back(dropOffScheduleItem);

            if(parkingEnabled)
            {
                //Retrieve the parking closest to the destination node
                const Node *endNode = (*request).destinationNode;
                auto parking = SMSVehicleParking::smsParkingRTree.searchNearestObject(endNode->getPosX(),
                                                                                      endNode->getPosY());
                if (parking)
                {
                    const ScheduleItem parkScheduleItem(PARK, parking);
                    schedule.push_back(parkScheduleItem);
                }
            }

			ControllerLog()<<"SingleRideRequest: is prepared  for Driver "<<feasibleDriver->getDatabaseId()<<" at time "<<currTick<<" ."<<endl;
            assignSchedule(feasibleDriver, schedule);
#ifndef NDEBUG
			if (currTick < request->timeOfRequest)
			{
				throw std::runtime_error("The assignment is sent before the request has been issued: impossible");
			}
#endif

			request = singleRideRequests.erase(request);
		}
		else
		{
			//no available drivers
			ControllerLog() << "Requests to be scheduled " << singleRideRequests.size() << ", available drivers "
			                << availableDrivers.size() << endl;

			//Move to next request. Leave the unassigned request in the queue, we will process again this next time
			++request;
		}
	}
}

Schedule AMOD_Controller::buildSchedule(unsigned int maxAggregatedRequests, const double maxWaitingTime, const Node *driverNode,
                                        Schedule schedule, unsigned int *aggregatedRequests)
{
	auto itReq = sharedRideRequests.begin();
	while(itReq != sharedRideRequests.end() && *aggregatedRequests < maxAggregatedRequests)
	{
		// We now iterate over all the un-assigned requests and we see if we can assign them to this driver
		const TripRequestMessage request = *itReq;

		// We will set it to true if we observe that this request can be assigned to this driver
		bool isMatchingSuccessful = false;
		unsigned pickupIdx = 0;

		do
		{
			// To check if we can assign this request to this driver, we create tentative schedules (like this
			// scheduleHypothesis).
			// We will try to modify this tentative schedule. If we succeed, they will become the real schedule.
			// Otherwise, we will start again from the real schedule and try to insert in it the following requests
			Schedule scheduleHypothesis = schedule;

			// First, we have to find a feasible position in the schedule for the pickup of the current request.
			ScheduleItem newPickup(PICKUP, request);
			scheduleHypothesis.insert(scheduleHypothesis.begin() + pickupIdx, newPickup);

			double vehicleTime = evaluateSchedule(driverNode, scheduleHypothesis, toleratedExtraTime, maxWaitingTime);

			if (vehicleTime > 0)
			{
				// It is possible to insert the pick up. Now, I start from the current scheduleHypothesis,
				// in which the pick up has been successfully inserted, and I seek a feasible position for the dropoff
				unsigned dropoffIdx = pickupIdx + 1;
				do
				{
					Schedule scheduleHypothesis2 = scheduleHypothesis;
					ScheduleItem newDropoff(DROPOFF, request);
					scheduleHypothesis2.insert(scheduleHypothesis2.begin() + dropoffIdx, newDropoff);
					double vehicleTime = evaluateSchedule(driverNode, scheduleHypothesis2, toleratedExtraTime,
					                                      maxWaitingTime);
					if (vehicleTime > 0)
					{
						// I can also insert the drop-off. Perfect, I will make this successful scheduleHypothesis my schedule
						isMatchingSuccessful = true;
						schedule = scheduleHypothesis2;
						(*aggregatedRequests)++;

						// The request is matched now. I can eliminate it, so that I will not assign
						// it again
						itReq = sharedRideRequests.erase(itReq);
					}
					else
					{
						// I will try with the subsequent position
						dropoffIdx++;
					}
				}
				while (dropoffIdx <= schedule.size() && !isMatchingSuccessful);
				// Note: I am using <= and not < in the while condition, since it is possible to insert the
				// drop-off at the end of the schedule
			}

			// If we arrived here, it means we did not find a feasible position in the schedule
			// for the drop-off. We can try by changing the position of the pickup.
			// If we find another feasible position for the pickup, it is possible that we will also find a feasible position
			// for a drop-off, given the new pickup position

			++pickupIdx;
		}
		while (pickupIdx < schedule.size() && !isMatchingSuccessful);
		// Note: I am using < and not <= in the while condition, since, if I insert the pickup at the end,
		// it is like I am not really sharing this request with

		if (!isMatchingSuccessful)
		{
			// I did not remove the request from the requests list, and thus I have to advance the iterator manually
			itReq++;
		}
		// else I do nothing, as I did what was to be done, some lines before
	}

	// Now we are done with this driver.
	return schedule;
}
