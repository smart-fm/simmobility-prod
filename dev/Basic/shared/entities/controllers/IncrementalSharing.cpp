/*
 * IncrementalSharing.cpp
 *
 *  Created on: 14 Jul 2017
 *      Author: araldo
 */

#include "IncrementalSharing.h"

namespace sim_mob {

void IncrementalSharing::computeSchedules()
{
	const std::map<unsigned int, Node*>& nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();
	unsigned vehicleCapacity = 2;
	double maxWaitingTime = 300; //seconds
	std::map<const Person*, Schedule> passengerSchedules;
	for (const Person* driver : availableDrivers)
	{
		const Node* driverNode = driver->exportServiceDriver()->getCurrentNode();
		std::map<const std::string, double> passengerMinimumTT ;
		Schedule schedule;
		unsigned passengers = 0;

		for ( std::list<TripRequestMessage>::iterator itReq =  requestQueue.begin();
				itReq != requestQueue.end() && passengers <= vehicleCapacity;
		){
			const TripRequestMessage request= *itReq;
			const Node* pickUpNode = nodeIdMap.find(request.startNodeId);
			const Node* dropOffNode = nodeIdMap.find(request.destinationNodeId);
			bool hasChancesToBeValid = true;

			Schedule::iterator scheduleIt = schedule.begin();
			do
			{
				Schedule scheduleHypothesis = schedule;
				scheduleHypothesis.insert();

				++scheduleIt;
			} while (scheduleIt != schedule.end() )


			if ( getTT( driverNode, pickUpNode, ttEstimateType  ) <= maxWaitingTime )
			{
				double currentPassengerMinimumTT = getTT(pickUpNode, dropOffNode, ttEstimateType);
				if (schedule.empty() )
				{
					// This is the first passenger I am matching to this driver
					passengerSchedules.emplace(request.userId, currentPassengerMinimumTT );
					const ScheduleItem pickUpScheduleItem(ScheduleItemType::PICKUP, *request);
					const ScheduleItem dropOffScheduleItem(ScheduleItemType::DROPOFF, *request);

					schedule.push_back(pickUpScheduleItem);
					schedule.push_back(dropOffScheduleItem);
					requestQueue.erase(itReq);
				}else if (schedule.size() == 1)
				{
					for

				}
			}// If the pickup point is far from the driver, I do not consider matching them
		}
	}
}

IncrementalSharing::~IncrementalSharing() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
