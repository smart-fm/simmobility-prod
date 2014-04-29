/*
 * PedestrainFacets.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#include "PedestrianFacets.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

namespace sim_mob {

namespace medium {

PedestrianBehavior::PedestrianBehavior(sim_mob::Person* parentAgent) :
		BehaviorFacet(parentAgent), parentPedestrian(nullptr) {

}

PedestrianBehavior::~PedestrianBehavior() {

}

PedestrianMovement::PedestrianMovement(sim_mob::Person* parentAgent) :
		MovementFacet(parentAgent), parentPedestrian(nullptr), remainingTimeToComplete(
				0), walkSpeed(200), lastRemainingTime(0), isMoveToNextLink(
				false), nextLink(nullptr) {

}

PedestrianMovement::~PedestrianMovement() {

}

void PedestrianMovement::setParentPedestrian(
		sim_mob::medium::Pedestrian* parentPedestrian) {
	this->parentPedestrian = parentPedestrian;
}

void PedestrianBehavior::setParentPedestrian(
		sim_mob::medium::Pedestrian* parentPedestrian) {
	this->parentPedestrian = parentPedestrian;
}

void PedestrianMovement::frame_init() {

	std::vector<const RoadSegment*> roadSegs;
	initializePath(roadSegs);

	Link* currentLink = nullptr;
	float currentTotalDistance = 0;
	std::vector<const RoadSegment*>::iterator it = roadSegs.begin();
	if (it != roadSegs.end()) {
		currentLink = (*it)->getLink();
		currentTotalDistance = (*it)->getLengthOfSegment();
		it++;
	}

	for (; it != roadSegs.end(); it++) {
		if ((*it)->getLink() == currentLink) {
			currentTotalDistance += (*it)->getLengthOfSegment();
		} else {
			float remainingTime = currentTotalDistance / walkSpeed;
			trajectory.push_back((std::make_pair(currentLink, remainingTime)));
			currentLink = (*it)->getLink();
			currentTotalDistance = (*it)->getLengthOfSegment();
		}
	}

	if (!currentLink) {
		float remainingTime = currentTotalDistance / walkSpeed;
		trajectory.push_back((std::make_pair(currentLink, remainingTime)));
	}

	if (trajectory.size() > 0) {
		remainingTimeToComplete = trajectory.front().second;
		trajectory.erase(trajectory.begin());
	}

}

void PedestrianMovement::initializePath(std::vector<const RoadSegment*>& path) {
	sim_mob::SubTrip& subTrip = *(getParent()->currSubTrip);
	const StreetDirectory& streetDirectory = StreetDirectory::instance();

	StreetDirectory::VertexDesc source, destination;
	std::vector<WayPoint> wayPoints;
	float distance = 0;
	if (subTrip.fromLocation.type_ == WayPoint::NODE) {
		source = streetDirectory.WalkingVertex(*subTrip.fromLocation.node_);
	} else if (subTrip.fromLocation.type_ == WayPoint::BUS_STOP) {
		source = streetDirectory.WalkingVertex(*subTrip.fromLocation.busStop_);
	}

	if (subTrip.toLocation.type_ == WayPoint::NODE) {
		destination = streetDirectory.WalkingVertex(*subTrip.toLocation.node_);
	} else if (subTrip.toLocation.type_ == WayPoint::BUS_STOP) {
		destination = streetDirectory.WalkingVertex(
				*subTrip.toLocation.busStop_);
	}

	wayPoints = streetDirectory.SearchShortestWalkingPath(source, destination);
	for (std::vector<WayPoint>::iterator it = wayPoints.begin();
			it != wayPoints.end(); it++) {
		if (it->type_ == WayPoint::ROAD_SEGMENT) {
			path.push_back(it->roadSegment_);
		}
	}
}

void PedestrianMovement::frame_tick() {
	unsigned int tickMS =
			ConfigManager::GetInstance().FullConfig().baseGranMS();

	if (remainingTimeToComplete <= tickMS) {
		lastRemainingTime = tickMS - remainingTimeToComplete;
		if (trajectory.size() == 0) {
			getParent()->setToBeRemoved();
		} else {
			isMoveToNextLink = true;
			nextLink = trajectory.front().first;
			remainingTimeToComplete = trajectory.front().second
					+ lastRemainingTime;
			trajectory.erase(trajectory.begin());
			lastRemainingTime = 0;
			getParent()->setNextLinkRequired(nextLink);
		}
	} else {
		remainingTimeToComplete -= tickMS;
	}
}

void PedestrianMovement::resetStatus() {
	unsigned int tickMS =
			ConfigManager::GetInstance().FullConfig().baseGranMS();
	remainingTimeToComplete -= tickMS;
	isMoveToNextLink = false;
	nextLink = nullptr;
	getParent()->setNextLinkRequired(nextLink);
}

void PedestrianMovement::frame_tick_output() {

}

}

} /* namespace sim_mob */
