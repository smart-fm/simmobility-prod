/*
 * ProximityBased.cpp
 *
 *  Created on: Aug 8, 2017
 *      Author: araldo
 */

#include "ProximityBased.hpp"

#include "geospatial/network/RoadNetwork.hpp"
#include <unordered_map>
#include "entities/Person.hpp"

using namespace std;

namespace sim_mob
{

	void ProximityBased::computeSchedules()
	{
		std::list<TripRequestMessage> requestQueueCopy(requestQueue);

		// We will put in these two datastructures the schedules that we have to send back to the drivers.
		// We will send the schedules only in case the ones we computed here are different from the
		// ones currently assigned to the drivers. We distinguish between brandNewSchedules, which are
		// the schedules of the drivers who did have nothing to do before this computation, and
		// updatedSchedules, in the case the respective driver had already a non-empty schedule (so she has
		// something to do), and we are modifying it.
		unordered_map<const Person*, Schedule> brandNewSchedules;
		unordered_map<const Person*, Schedule> updatedSchedules;

		// First, we will try to assign request to drivers trying to respect the time constraints. It may happen,
		// however, that after doing this attempt, which we call "1st round", there may be some un-associated
		// requests and drivers that have nothing to do. The reason why this may happen is that those drivers
		// may be too far away from the request pick up locations. We store those drivers in this
		// emptyDriversAfter1stRound list. Then, we will perform a "2nd round" in which we associate each
		// un-associated request to one of thes empty drivers.
		std::list<const Person*> emptyDriversAfter1stRound;

		//aa!!: In the way explained above, we are considering free only the drivers with an empty schedule.
		//			Should we also consider free the driver that have a non-empty schedule, but all the items are
		//			PARK or CRUISE?

		//{ 1st ROUND
		for ( std::vector<Person *>::const_iterator driverIt = subscribedDrivers.begin();
				// If the requestQueueCopy is empty, we have nothing to assign to the drivers and we can
				// quit the loop
				 driverIt != subscribedDrivers.end() && !requestQueueCopy.empty();
				 ++driverIt
		){
			const Person* driver = *driverIt;
			// We start with the current driverSchedule. We will try to fill in as many requests as possible
			Schedule driverSchedule = driver->exportServiceDriver()->getAssignedSchedule();
			const Node *driverNode = driver->exportServiceDriver()->getCurrentNode();
			size_t originalDriverScheduleSize = driverSchedule.size();

			for (std::list<TripRequestMessage>::iterator reqIt;
					reqIt != requestQueueCopy.end() &&
							// Note that if the vehicle is already full, we do not iterate over requests
							// because we now that we cannot fit them in
							driverSchedule.getPassengerCount() < maxAggregatedRequests;
					//  The iteration expression, for example ++reqIt, has been dealt with inside the body
					// of this for loop
			)
			{
				//aa!!: This fact that we retrieve the node itself from the node id querying a big map happens too much
				//			and everywhere in the code.
				//			I think it would be better to modify the TripRequestMessage, having the const Node* instead of
				//			(or in addition to) nodeId, both for the pickUp and dropOff location.
				const RoadNetwork *rdNetowrk = RoadNetwork::getInstance();
				const Node *newRequestDropOffNode = rdNetowrk->getById(rdNetowrk->getMapOfIdvsNodes(), reqIt->destinationNodeId);

				const Point& dropOffBarycenter = driverSchedule.getDropOffBarycenter();

				if ( 	getTT(driverNode->getLocation(), dropOffBarycenter)  <=  maxWaitingTime &&
			             // Note that if the driver is too far or already fully occupied, the driver
			             // is immediately skipped
						(driverSchedule.getPassengerCount() ==0 || getTT(newRequestDropOffNode->getLocation(), dropOffBarycenter) < toleratedExtraTime )
				)
				{
					// We can assign this request to the driver

					//{ MOFDIFY THE DRIVER SCHEDULE
					ScheduleItem newPickUp(PICKUP, *reqIt);
					ScheduleItem newDropOff(DROPOFF, *reqIt);
					Schedule::iterator it =driverSchedule.begin();
					driverSchedule.insert(it, newPickUp);

					// We put the dropoff before the first dropoff
					while (it->scheduleItemType != DROPOFF  && it != driverSchedule.end())
						++it;

					driverSchedule.insert(it, newPickUp);
					//} MOFDIFY THE DRIVER SCHEDULE

					// We remove the request from the requestQueueCopy, in order to avoid assigning it to other drivers
					requestQueueCopy.erase(reqIt);

					// If we erase the this entry of the requestQueueCopy, we do not need to increment the iterator, i.e.,
					// we do not need to do ++reqIt, since, figuratively, all the rest of the list has bees shifted back of
					//	1 position, after the erasure.
				}else{
					// We did not assign the request to the driver, and thus we need to explicitly increment the iterator
					++reqIt;
				}
			}// end of iteratior over requests

#ifndef NDEBUG
			if (driverSchedule.size() < originalDriverScheduleSize )
			{
				throw std::runtime_error("We have reduced the size of the schedule. This is not possible, as the only operations we have"
						" performed consist in adding scheduleItems");
			}

			if (brandNewSchedules.find(driver) !=  brandNewSchedules.end() ||
					updatedSchedules.find(driver) !=  updatedSchedules.end() ||
					std::find(emptyDriversAfter1stRound.begin(), emptyDriversAfter1stRound.end(), driver) != emptyDriversAfter1stRound.end()
			){
				// DO NOT DISABLE THIS EXCEPTION, PLEASE!
				throw std::runtime_error("Trying to add a driver twice in the same datastructure. ");
			}
#endif

			if ( driverSchedule.size() > originalDriverScheduleSize)
			{	// We have added some items to the driverSchedule. We have to send the new schedule to the driver
				if (originalDriverScheduleSize == 0)
				{	// Before this computation the driver had nothing to do, and thus we are sending her an entirely new
					// schedule
					brandNewSchedules.emplace(driver,driverSchedule);
				}else{ // We are modifying the current driver schedule
					updatedSchedules.emplace(driver, driverSchedule);
				}
			} else if (driverSchedule.size() == 0 ){
					emptyDriversAfter1stRound.push_back(driver);
			}
		}
		//} 1st ROUND


		//{ 2nd ROUND
		// We associate each un-served request to a driver
		for ( list<const Person*>::const_iterator driverIt = emptyDriversAfter1stRound.begin();
				driverIt != emptyDriversAfter1stRound.end() && !requestQueueCopy.empty();
				++driverIt
		){
			std::list<TripRequestMessage>::iterator reqIt = requestQueueCopy.begin();
			Schedule newSchedule;
			newSchedule.push_back( ScheduleItem(PICKUP, *reqIt)  );
			newSchedule.push_back( ScheduleItem(DROPOFF, *reqIt)  );
			brandNewSchedules.emplace(*driverIt,newSchedule);
			requestQueue.erase(reqIt);
		}
		//} 2nd ROUND

		bool areSchedulesUpdated = false;
		assignSchedules(updatedSchedules, areSchedulesUpdated);
		areSchedulesUpdated = true;
		assignSchedules(brandNewSchedules, areSchedulesUpdated);
}// end of computeSchedules()

}/* namespace sim_mob */
