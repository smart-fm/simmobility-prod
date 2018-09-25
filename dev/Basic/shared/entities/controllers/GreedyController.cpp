/*
 * GreedyTaxiController.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha
 */

#include "GreedyController.hpp"
#include "geospatial/network/RoadNetwork.hpp"

using namespace sim_mob;

void GreedyController::computeSchedules()
{
#ifndef NDEBUG
    consistencyChecks("beginning");
#endif

    ControllerLog() << "Computing schedule: " << requestQueue.size() << " requests are in the queue, available drivers "
                    << availableDrivers.size() << std::endl;

    std::list<TripRequestMessage>::iterator request = requestQueue.begin();
    if (!availableDrivers.empty())
    {
        while (request != requestQueue.end())
        {
            //Assign the start node
            const Node *startNode = (*request).startNode;
            const Person *bestDriver = findClosestDriver(startNode);

            if (bestDriver)
            {
                //Retrieve the parking closest to the destination node
                const Node *endNode = (*request).destinationNode;
                const SMSVehicleParking *parking =
                        SMSVehicleParking::smsParkingRTree.searchNearestObject(endNode->getPosX(), endNode->getPosY());

                Schedule schedule;
                const ScheduleItem pickUpScheduleItem(ScheduleItemType::PICKUP, *request);
                const ScheduleItem dropOffScheduleItem(ScheduleItemType::DROPOFF, *request);

                schedule.push_back(pickUpScheduleItem);
                schedule.push_back(dropOffScheduleItem);

                if (parking)
                {
                    const ScheduleItem parkScheduleItem(ScheduleItemType::PARK, parking);
                    schedule.push_back(parkScheduleItem);
                }

                assignSchedule(bestDriver, schedule);

#ifndef NDEBUG
                if (currTick < request->timeOfRequest)
                    throw std::runtime_error("The assignment is sent before the request has been issued: impossible");
#endif

                request = requestQueue.erase(request);
            }
            else
            {
                //no available drivers
                ControllerLog() << "Requests to be scheduled " << requestQueue.size() << ", available drivers "
                                << availableDrivers.size() << std::endl;

                //Move to next request. Leave the unassigned request in the queue, we will process again this next time
                ++request;
            }
        }
    }
    else
    {
        //no available drivers
        ControllerLog() << "Requests to be scheduled " << requestQueue.size() << ", available drivers "
                        << availableDrivers.size() << std::endl;
    }

#ifndef NDEBUG
    consistencyChecks("end");
#endif

}
