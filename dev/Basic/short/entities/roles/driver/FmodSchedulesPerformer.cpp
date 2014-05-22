/*
 * FMODSchedulesPerformer.cpp
 *
 *  Created on: Mar 27, 2014
 *      Author: zhang
 */

#include "FmodSchedulesPerformer.hpp"
#include "entities/Person.hpp"
#include "entities/AuraManager.hpp"
#include "entities/UpdateParams.hpp"
#include "Driver.hpp"
#include "BusDriver.hpp"

namespace sim_mob {

//the minimum distance approaching to node
const float APPROACHING_DISTANCE = 1000;
const float SEARCH_REGION_AGNETS = 3500;

FmodSchedulesPerformer::FmodSchedulesPerformer() {
	// TODO Auto-generated constructor stub

}

FmodSchedulesPerformer::~FmodSchedulesPerformer() {
	// TODO Auto-generated destructor stub
}

bool FmodSchedulesPerformer::performFmodSchedule(Driver* parentDriver, DriverUpdateParams& p)
{
	bool ret = false;
	FMODSchedule* schedule = parentDriver->getVehicle()->schedule;

	if (schedule) {
		const RoadSegment* currSegment = parentDriver->getVehicle()->getCurrSegment();
		const Node* stop = currSegment->getEnd();
		bool isFound = false;
		static int count = 0;
		double dwellTime = 0;
		double distance = parentDriver->distanceInFront;

		//judge whether near to stopping node
		if (distance < APPROACHING_DISTANCE) {
			for (size_t i = 0; i < schedule->stopSchdules.size(); i++) {

				FMODSchedule::STOP& stopSchedule = schedule->stopSchdules[i];
				if (stopSchedule.stopId == stop->getID()) {

					isFound = true;
					dwellTime = stopSchedule.dwellTime;

					//arrive at scheduling node
					if (dwellTime == 0) {
						parentDriver->stop_event_type.set(1);
						parentDriver->stop_event_scheduleid.set(stopSchedule.scheduleId);
						parentDriver->stop_event_nodeid.set(stop->getID());
						int passengersnum =	stopSchedule.alightingPassengers.size()
										+ stopSchedule.boardingPassengers.size();
						dwellTime = stopSchedule.dwellTime;

						processTripsInSchedule(parentDriver, schedule, stopSchedule, p);
					}

					// stopping at scheduling node
					dwellTime -= p.elapsedSeconds;
					schedule->stopSchdules[i].dwellTime = dwellTime;

					//departure from this node
					if (dwellTime < 0) {
						parentDriver->stop_event_type.set(0);
					}
				}
			}
		}

		if (isFound && dwellTime > 0.0) {
			ret = true;
		}
	}
	return ret;
}

bool FmodSchedulesPerformer::processTripsInSchedule(Driver* parentDriver, FMODSchedule* schedule, FMODSchedule::STOP& stopSchedule, DriverUpdateParams& p)
{
	//boarding and alighting
	const RoadSegment* seg = parentDriver->getVehicle()->getCurrSegment();
	const Node* node = seg->getEnd();
	const Agent* parentAgent = (parentDriver ? parentDriver->getParent() : nullptr);

	vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(
			Point2D((node->getLocation().getX() - SEARCH_REGION_AGNETS),
					(node->getLocation().getY() - SEARCH_REGION_AGNETS)),
			Point2D((node->getLocation().getX() + SEARCH_REGION_AGNETS),
					(node->getLocation().getY() + SEARCH_REGION_AGNETS)), parentAgent);

	for (vector<const Agent*>::iterator it = nearby_agents.begin();	it != nearby_agents.end(); it++) {
		//passenger boarding
		vector<int>& boardingpeople = stopSchedule.boardingPassengers;
		if (std::find(boardingpeople.begin(), boardingpeople.end(),	(*it)->getId()) != boardingpeople.end()) {
			const Person* person = dynamic_cast<const Person*>((*it));
			Passenger* passenger = person ?	dynamic_cast<Passenger*>(person->getRole()) : nullptr;

			if (!passenger) {
				continue;
			}

			schedule->insidePassengers.push_back(person);
			PassengerMovement* movement = dynamic_cast<PassengerMovement*>(passenger->Movement());
			if (movement) {
				movement->PassengerBoardBus_Choice(parentDriver);
			}
		}
	}

	vector<int>& alightingpeople = stopSchedule.alightingPassengers;
	for (vector<int>::iterator it = alightingpeople.begin(); it != alightingpeople.end(); it++) {
		vector<const Person*>::iterator itPerson = schedule->insidePassengers.begin();
		while (itPerson != schedule->insidePassengers.end()) {
			if ((*it) == (int) (*itPerson)->getId()) {
				Passenger* passenger = dynamic_cast<Passenger*>((*itPerson)->getRole());
				if (!passenger) {
					continue;
				}

				PassengerMovement* movement = dynamic_cast<PassengerMovement*>(passenger->Movement());
				if (movement) {
					movement->PassengerAlightBus(parentDriver);
				}

				itPerson = schedule->insidePassengers.erase(itPerson);
			} else {
				itPerson++;
			}
		}
	}

	//update shared parameters to record boarding and alighting person
	parentDriver->stop_event_lastAlightingPassengers.set(
			stopSchedule.alightingPassengers);
	parentDriver->stop_event_lastBoardingPassengers.set(
			stopSchedule.boardingPassengers);
}

} /* namespace sim_mob */
