/*
 * IncrementalSharing.cpp
 *
 *  Created on: 14 Jul 2017
 *      Author: araldo
 */

#include "IncrementalSharing.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/max_cardinality_matching.hpp>

#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "path/PathSetManager.hpp"


namespace sim_mob {

void IncrementalSharing::computeSchedules()
{
	const std::map<unsigned int, Node*>& nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();
	unsigned maxPassengers = 3;
	double maxWaitingTime = 300; //seconds
	std::map<const Person*, Schedule> driverSchedules;
	for (const Person* driver : availableDrivers)
	{
		const Node* driverNode = driver->exportServiceDriver()->getCurrentNode();
		std::map<const std::string, double> passengerMinimumTT ;
		Schedule schedule;
		unsigned passengers = 0;

		for ( std::list<TripRequestMessage>::iterator itReq =  requestQueue.begin();
				itReq != requestQueue.end() && passengers <= maxPassengers;
		){
			const TripRequestMessage request= *itReq;
			const Node* pickUpNode = *nodeIdMap.find(request.startNodeId);
			const Node* dropOffNode = *nodeIdMap.find(request.destinationNodeId);
			bool found = false;

			unsigned pickupIdx = 0;
			do
			{
				Schedule scheduleHypothesis = schedule;
				ScheduleItem newPickup(PICKUP, request);
				scheduleHypothesis.insert(scheduleHypothesis.begin()+pickupIdx, newPickup);
				double vehicleTime = evaluateSchedule(driverNode, scheduleHypothesis, MobilityServiceController::toleratedExtraTime, maxWaitingTime);
				if (vehicleTime>0)
				{
					// It is possible at least to insert the pick up. Now I check if I can also insert the dropoff
					unsigned dropoffIdx = pickupIdx+1;
					do{
						Schedule scheduleHypothesis2 = scheduleHypothesis;
						ScheduleItem newDropoff(DROPOFF, request);
						scheduleHypothesis2.insert(schedule.begin() + dropoffIdx, newDropoff);
						double vehicleTime = evaluateSchedule(driverNode, scheduleHypothesis, MobilityServiceController::toleratedExtraTime, maxWaitingTime);
						if (vehicleTime > 0){
							// I can also insert the dropoff. Perfect, I will definitely insert the request in the schedule
							found = true;
							schedule = scheduleHypothesis2;
						}
					} while ( dropoffIdx <= schedule.size() && !found  );
				}
				++pickupIdx;
			} while ( pickupIdx <= schedule.size() &&  !found );

		}

		driverSchedules.emplace(driver, schedule);
	}

	for (const std::pair<const Person*, Schedule>& p : driverSchedules)
	{
			const Person* driver = p.first; const Schedule schedule = p.second;
			assignSchedule(driver, schedule);
	}
}

IncrementalSharing::~IncrementalSharing() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
