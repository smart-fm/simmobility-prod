#include "GenConfig.h"

#include "PackageUtils.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include "entities/roles/driver/GeneralPathMover.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "entities/misc/TripChain.hpp"
#include "util/GeomHelpers.hpp"
#include "entities/roles/driver/IntersectionDrivingModel.hpp"
#include "geospatial/Point2D.hpp"
#include "util/DynamicVector.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "partitions/PartitionManager.hpp"

namespace sim_mob {
std::string PackageUtils::getPackageData() {
	return buffer.str().data();
}

void PackageUtils::initializePackage() {
	package = new boost::archive::text_oarchive(buffer);
}

void PackageUtils::clearPackage() {
	buffer.clear();
	if (package) {
		delete package;
		package = NULL;
	}
}

void PackageUtils::packageNode(const Node* one_node) {
	bool hasSomthing = true;
	if (!one_node) {
		hasSomthing = false;
		(*package) & hasSomthing;
		return;
	} else {
		(*package) & hasSomthing;
	}

	(*package) & (*(one_node->location));
}

void PackageUtils::packageRoadSegment(const RoadSegment* roadsegment) {
	bool hasSomthing = true;
	if (!roadsegment) {

		hasSomthing = false;
		(*package) & hasSomthing;
		return;
	} else {

		(*package) & hasSomthing;
	}


	(*package) & (*(roadsegment->getStart()->location));
	(*package) & (*(roadsegment->getEnd()->location));
}
void PackageUtils::packageLink(const Link* one_link) {
	bool hasSomthing = true;
	if (!one_link) {
		hasSomthing = false;
		(*package) & hasSomthing;
		return;
	} else {
		(*package) & hasSomthing;
	}

	(*package) & (*(one_link->getStart()->location));
	(*package) & (*(one_link->getEnd()->location));
}

void PackageUtils::packageLane(const Lane* one_lane) {
	bool hasSomthing = true;
	if (!one_lane) {
		hasSomthing = false;
		(*package) & hasSomthing;
		return;
	} else {
		(*package) & hasSomthing;
	}

	sim_mob::Point2D const & start = one_lane->getRoadSegment()->getStart()->location;
	sim_mob::Point2D const & end = one_lane->getRoadSegment()->getEnd()->location;
	int lane_id = one_lane->getLaneID();

	(*package) & (start);
	(*package) & (end);
	(*package) & lane_id;
}

void PackageUtils::packageTripChain(const TripChain* tripChain) {
	bool hasSomthing = true;
	if (!tripChain) {
		hasSomthing = false;
		(*package) & hasSomthing;
		return;
	} else {
		(*package) & hasSomthing;
	}

	packageTripActivity(&(tripChain->from));
	packageTripActivity(&(tripChain->to));

	(*package) & (tripChain->primary);
	(*package) & (tripChain->flexible);
	(*package) & (tripChain->startTime);
	(*package) & (tripChain->mode);
}

void PackageUtils::packageTripActivity(const TripActivity* tripActivity) {
	bool hasSomthing = true;
	if (!tripActivity) {
		hasSomthing = false;
		(*package) & hasSomthing;
		return;
	} else {
		(*package) & hasSomthing;
	}

	(*package) & (tripActivity->description);
	packageNode(tripActivity->location);
}

void PackageUtils::packageVehicle(const Vehicle* one_vehicle) {
	bool hasSomthing = true;
	if (!one_vehicle) {
		hasSomthing = false;
		(*package) & hasSomthing;
		return;
	} else {
		(*package) & hasSomthing;
	}

	(*package) & (one_vehicle->length);
	(*package) & (one_vehicle->width);

	packageGeneralPathMover(&(one_vehicle->fwdMovement));

	//After fwdMovement
	(*package) & (one_vehicle->latMovement);
	(*package) & (one_vehicle->fwdVelocity);
	(*package) & (one_vehicle->latVelocity);
	(*package) & (one_vehicle->fwdAccel);

	(*package) & (one_vehicle->posInIntersection);
	(*package) & (one_vehicle->error_state);
}

void PackageUtils::packageGeneralPathMover(const GeneralPathMover* fwdMovement) {
	//transfer vector of road segment
	//part 1

	int path_size = fwdMovement->fullPath.size();
	(*package) & (path_size);

	std::vector<const sim_mob::RoadSegment*>::const_iterator itr = fwdMovement->fullPath.begin();
	for (; itr != fwdMovement->fullPath.end(); itr++) {
		packageRoadSegment(*itr);
	}

	int current_segment = fwdMovement->currSegmentIt - fwdMovement->fullPath.begin();
	(*package) & current_segment;


	//part 2
	//polypointsList
	if (fwdMovement->polypointsList.size() > 0) {
		bool hasPointList = true;
		(*package) & hasPointList;

		(*package) & (fwdMovement->polypointsList);

		int current_poly = fwdMovement->currPolypoint - fwdMovement->polypointsList.begin();
		(*package) & current_poly;

		int next_poly = fwdMovement->nextPolypoint - fwdMovement->polypointsList.begin();
		(*package) & next_poly;

		//copy from GeneralPathMover.cpp, the design of current_zero_poly is not good.
		//I thought current_zero_poly is the iterator of currPolypoint
		const std::vector<Point2D>& tempLaneZero = const_cast<RoadSegment*>(*(fwdMovement->currSegmentIt))->getLaneEdgePolyline(0);

		int current_zero_poly = fwdMovement->currLaneZeroPolypoint - tempLaneZero.begin();
		(*package) & current_zero_poly;

		int next_zero_poly = fwdMovement->nextLaneZeroPolypoint - tempLaneZero.begin();
		(*package) & next_zero_poly;

	} else {
		bool hasPointList = false;
		(*package) & hasPointList;
	}

	//part 3
	//Others
	(*package) & (fwdMovement->distAlongPolyline);
	(*package) & (fwdMovement->distMovedInCurrSegment);
	(*package) & (fwdMovement->distOfThisSegment);
	(*package) & (fwdMovement->distOfRestSegments);
	(*package) & (fwdMovement->inIntersection);
	(*package) & (fwdMovement->isMovingForwardsInLink);
	(*package) & (fwdMovement->currLaneID);

	std::string value_ = fwdMovement->DebugStream.str();
	(*package) & (value_);

}

void PackageUtils::packageCrossing(const Crossing* one_crossing) {
	bool hasSomthing = true;
	if (!one_crossing) {
		hasSomthing = false;
		(*package) & hasSomthing;
		return;
	} else {
		(*package) & hasSomthing;
	}

	std::cout << "333.6" << std::endl;

	Point2D near1 = one_crossing->nearLine.first;
	Point2D near2 = one_crossing->nearLine.second;
	Point2D far1 = one_crossing->farLine.first;
	Point2D far2 = one_crossing->farLine.second;

	(*package) & (near1);
	(*package) & (near2);

	std::cout << "333.7" << std::endl;

	(*package) & (far1);
	(*package) & (far2);

	std::cout << "333.8" << std::endl;
}

void PackageUtils::packageIntersectionDrivingModel(SimpleIntDrivingModel* one_model) {
	bool hasSomthing = true;
	if (!one_model) {
		hasSomthing = false;
		(*package) & hasSomthing;
		return;
	} else {
		(*package) & hasSomthing;
	}

	(*package) & (*one_model);
}

void PackageUtils::packageFixedDelayedDPoint(FixedDelayed<DPoint*>& one_delay) {
	(*package) & (one_delay.delayMS);
	(*package) & (one_delay.reclaimPtrs);

	int list_size = one_delay.history.size();
	(*package) & list_size;

	std::list<FixedDelayed<DPoint*>::HistItem>::const_iterator itr = one_delay.history.begin();
	for (; itr != one_delay.history.end(); itr++) {
		FixedDelayed<DPoint*>::HistItem one = (*itr);
		DPoint* value = one.item;
		int value2 = one.observedTime;

		(*package) & (*value);
		(*package) & value2;
	}
}

void PackageUtils::packageFixedDelayedDouble(FixedDelayed<double>& one_delay) {
	(*package) & (one_delay.delayMS);
	(*package) & (one_delay.reclaimPtrs);


	int list_size = one_delay.history.size();
	(*package) & list_size;

	std::list<FixedDelayed<double>::HistItem>::const_iterator itr = one_delay.history.begin();
	for (; itr != one_delay.history.end(); itr++) {
		FixedDelayed<double>::HistItem one = (*itr);
		double value = one.item;
		int value2 = one.observedTime;

		(*package) & (value);
		(*package) & value2;
	}
}

void PackageUtils::packageFixedDelayedInt(FixedDelayed<int>& one_delay) {
	(*package) & (one_delay.delayMS);
	(*package) & (one_delay.reclaimPtrs);

	int list_size = one_delay.history.size();
	(*package) & list_size;

	std::list<FixedDelayed<int>::HistItem>::const_iterator itr = one_delay.history.begin();
	for (; itr != one_delay.history.end(); itr++) {
		FixedDelayed<int>::HistItem one = (*itr);
		int value = one.item;
		int value2 = one.observedTime;

		(*package) & (value);
		(*package) & value2;
	}
}

void PackageUtils::packagePoint2D(const Point2D& one_point) {
	(*package) & (one_point);
}

}
#endif
