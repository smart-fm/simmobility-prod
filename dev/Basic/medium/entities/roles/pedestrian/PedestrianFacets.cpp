/*
 * PedestrainFacets.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#include "PedestrianFacets.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/BusStop.hpp"

namespace sim_mob {
namespace medium {

PedestrianBehavior::PedestrianBehavior(sim_mob::Person* parentAgent) :
		BehaviorFacet(parentAgent), parentPedestrian(nullptr) {

}

PedestrianBehavior::~PedestrianBehavior() {

}

PedestrianMovement::PedestrianMovement(sim_mob::Person* parentAgent,double speed) :
		MovementFacet(parentAgent), parentPedestrian(nullptr),
		remainingTimeToComplete(0), walkSpeed(speed) {
}

PedestrianMovement::~PedestrianMovement() {}

void PedestrianMovement::setParentPedestrian(
		sim_mob::medium::Pedestrian* parentPedestrian) {
	this->parentPedestrian = parentPedestrian;
}


TravelMetric & PedestrianMovement::startTravelTimeMetric()
{
	return *travelTimeMetric;
}

TravelMetric & PedestrianMovement::finalizeTravelTimeMetric()
{
	return *travelTimeMetric;
}
void PedestrianBehavior::setParentPedestrian(
		sim_mob::medium::Pedestrian* parentPedestrian) {
	this->parentPedestrian = parentPedestrian;
}

void PedestrianMovement::frame_init() {
	std::vector<const RoadSegment*> roadSegs;
	initializePath(roadSegs);

	sim_mob::SubTrip& subTrip = *(getParent()->currSubTrip);
	const RoadSegment* segStart = nullptr;
	const RoadSegment* segEnd = nullptr;
	double actualDistanceStart = 0.0;
	double actualDistanceEnd = 0.0;

	if (subTrip.fromLocation.type_ == WayPoint::BUS_STOP) {
		segStart = subTrip.fromLocation.busStop_->getParentSegment();
		const Node* node = segStart->getEnd();
		const BusStop* stop = subTrip.fromLocation.busStop_;
		DynamicVector EstimateDist(stop->xPos, stop->yPos, node->location.getX(),
				node->location.getY());
		actualDistanceStart = EstimateDist.getMagnitude();
	}

	if (subTrip.toLocation.type_ == WayPoint::BUS_STOP) {
		segEnd = subTrip.toLocation.busStop_->getParentSegment();
		const Node* node = segEnd->getStart();
		const BusStop* stop = subTrip.toLocation.busStop_;
		DynamicVector EstimateDist(stop->xPos, stop->yPos, node->location.getX(),
				node->location.getY());
		actualDistanceEnd = EstimateDist.getMagnitude();
	}

	Link* currentLink = nullptr;
	double distanceInLink = 0.0;
	double curDistance = 0.0;
	std::vector<const RoadSegment*>::const_iterator it = roadSegs.begin();
	if (it != roadSegs.end()) {
		currentLink = (*it)->getLink();
	}

	for (; it != roadSegs.end(); it++) {
		const RoadSegment* rdSeg = *it;
		curDistance = rdSeg->getLaneZeroLength();
		if (rdSeg == segStart) {
			curDistance = actualDistanceStart;
		} else if (rdSeg == segEnd) {
			curDistance = actualDistanceEnd;
		}

		if (rdSeg->getLink() == currentLink) {
			distanceInLink += curDistance;
		} else {
			double remainingTime = distanceInLink / walkSpeed;
			trajectory.push_back((std::make_pair(currentLink, remainingTime)));
			currentLink = rdSeg->getLink();
			distanceInLink = curDistance;
		}
	}

	if (currentLink) {
		double remainingTime = distanceInLink / walkSpeed;
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
	if (subTrip.fromLocation.type_ == WayPoint::NODE) {
		source = streetDirectory.DrivingVertex(*subTrip.fromLocation.node_);
	} else if (subTrip.fromLocation.type_ == WayPoint::BUS_STOP) {
		const Node* node = subTrip.fromLocation.busStop_->getParentSegment()->getStart();
		source = streetDirectory.DrivingVertex(*node);
	}

	if (subTrip.toLocation.type_ == WayPoint::NODE) {
		destination = streetDirectory.DrivingVertex(*subTrip.toLocation.node_);
	} else if (subTrip.toLocation.type_ == WayPoint::BUS_STOP) {
		const Node* node = subTrip.toLocation.busStop_->getParentSegment()->getEnd();
		destination = streetDirectory.DrivingVertex(*node);
	}

	wayPoints = streetDirectory.SearchShortestDrivingPath(source, destination);
	for (std::vector<WayPoint>::iterator it = wayPoints.begin();
			it != wayPoints.end(); it++) {
		if (it->type_ == WayPoint::ROAD_SEGMENT) {
			path.push_back(it->roadSegment_);
		}
	}
}

void PedestrianMovement::frame_tick() {
	double tickSec = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	if (remainingTimeToComplete <= tickSec) {
		double lastRemainingTime = tickSec - remainingTimeToComplete;
		if (trajectory.size() == 0) {
			getParent()->setNextLinkRequired(nullptr);
			getParent()->setToBeRemoved();
		}
		else {
			Link* nextLink = trajectory.front().first;
			remainingTimeToComplete = trajectory.front().second - lastRemainingTime;
			trajectory.erase(trajectory.begin());
			getParent()->setNextLinkRequired(nextLink);
		}
	}
	else {
		remainingTimeToComplete -= tickSec;
	}
	getParent()->setRemainingTimeThisTick(0);
}

void PedestrianMovement::frame_tick_output() {}

}

} /* namespace sim_mob */
