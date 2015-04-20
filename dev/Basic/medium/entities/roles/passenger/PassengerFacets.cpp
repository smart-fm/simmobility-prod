/*
 * PassengerFacets.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#include "PassengerFacets.hpp"
#include "Passenger.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

namespace sim_mob {

namespace medium {

PassengerBehavior::PassengerBehavior(sim_mob::Person* parentAgent) :
		BehaviorFacet(parentAgent), parentPassenger(nullptr) {

}

PassengerBehavior::~PassengerBehavior() {

}

PassengerMovement::PassengerMovement(sim_mob::Person* parentAgent) :
		MovementFacet(parentAgent), parentPassenger(nullptr), totalTimeToCompleteMS(
				0) {

}

PassengerMovement::~PassengerMovement() {

}

void PassengerMovement::setParentPassenger(
		sim_mob::medium::Passenger* parentPassenger) {
	this->parentPassenger = parentPassenger;
}

void PassengerBehavior::setParentPassenger(
		sim_mob::medium::Passenger* parentPassenger) {
	this->parentPassenger = parentPassenger;
}

void PassengerMovement::frame_init() {

	totalTimeToCompleteMS = 0;
}

void PassengerMovement::frame_tick() {
	unsigned int tickMS =
			ConfigManager::GetInstance().FullConfig().baseGranMS();
	totalTimeToCompleteMS += tickMS;

	if (parentPassenger->getDriver() == nullptr) {
		getParent()->setToBeRemoved();
	}
}

void PassengerMovement::frame_tick_output() {

}


TravelMetric & PassengerMovement::startTravelTimeMetric()
{
//	travelMetric.startTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
//	const Node* startNode = (*(pathMover.getPath().begin()))->getRoadSegment()->getEnd();
//	travelMetric.origin = WayPoint(startNode);
//	travelMetric.started = true;
	return travelMetric;
}

TravelMetric & PassengerMovement::finalizeTravelTimeMetric()
{
//	const Node* endNode = (*(pathMover.getPath().begin()))->getRoadSegment()->getEnd();
//	travelMetric.destination = WayPoint(endNode);
//	travelMetric.endTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
//	travelMetric.travelTime = (travelMetric.endTime - travelMetric.startTime).getValue();
//	travelMetric.finalized = true;
	return travelMetric;
}
}//medium
} /* namespace sim_mob */
