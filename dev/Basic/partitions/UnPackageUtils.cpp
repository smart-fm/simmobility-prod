#include "constants.h"

#ifndef SIMMOB_DISABLE_MPI
#include "UnPackageUtils.hpp"

#include "conf/simpleconf.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "entities/misc/TripChain.hpp"
#include "util/GeomHelpers.hpp"
#include "entities/roles/driver/GeneralPathMover.hpp"
#include "entities/roles/driver/IntersectionDrivingModel.hpp"
#include "util/DynamicVector.hpp"

namespace sim_mob {

void UnPackageUtils::initializePackage(std::string value) {
	buffer << value;
	package = new boost::archive::text_iarchive(buffer);
}

void UnPackageUtils::clearPackage() {
	buffer.clear();
	if (package) {
		delete package;
		package = NULL;
	}
}

const Node* UnPackageUtils::unpackageNode() {
	bool hasSomthing;
	(*package) & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	sim_mob::Point2D location;
	(*package) & location;

	sim_mob::RoadNetwork& rn = ConfigParams::GetInstance().getNetwork();
	return rn.locateNode(location, true);
}

const RoadSegment* UnPackageUtils::unpackageRoadSegment() {
	bool hasSomthing;
	(*package) & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	sim_mob::Point2D point_1;
	sim_mob::Point2D point_2;

	(*package) & point_1;
	(*package) & point_2;

	return sim_mob::getRoadSegmentBasedOnNodes(&point_1, &point_2);
}

const Link* UnPackageUtils::unpackageLink() {
	bool hasSomthing;
	(*package) & hasSomthing;

	if (hasSomthing == false) {
		return NULL;
	}

	sim_mob::Point2D point_1;
	sim_mob::Point2D point_2;

	(*package) & point_1;
	(*package) & point_2;

	return sim_mob::getLinkBetweenNodes(&point_1, &point_2);
}

const Lane* UnPackageUtils::unpackageLane() {
	bool hasSomthing;
	(*package) & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	sim_mob::Point2D start;
	sim_mob::Point2D end;
	int lane_id;

	(*package) & start;
	(*package) & end;
	(*package) & lane_id;

	const sim_mob::RoadSegment* roadSegment = sim_mob::getRoadSegmentBasedOnNodes(&start, &end);

	const std::vector<sim_mob::Lane*>& lanes = roadSegment->getLanes();
	std::vector<sim_mob::Lane*>::const_iterator it = lanes.begin();

	for (; it != lanes.end(); it++) {
		if ((*it)->getLaneID() == lane_id)
			return (*it);
	}

	return NULL;
}

const TripChain* UnPackageUtils::unpackageTripChain() {
	bool hasSomthing;
	(*package) & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	TripChain* tripChain = new TripChain();
	tripChain->from = *(unpackageTripActivity());
	tripChain->to = *(unpackageTripActivity());

	(*package) & (tripChain->primary);
	(*package) & (tripChain->flexible);
	(*package) & (tripChain->startTime);
	(*package) & (tripChain->mode);

	return tripChain;
}

const TripActivity* UnPackageUtils::unpackageTripActivity() {
	bool hasSomthing;
	(*package) & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	TripActivity* activity = new TripActivity();
	(*package) & (activity->description);

	activity->location = const_cast<sim_mob::Node*> (unpackageNode());
	return activity;
}

const Vehicle* UnPackageUtils::unpackageVehicle() {
	bool hasSomthing;
	(*package) & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	Vehicle* one_vehicle = new Vehicle();

	double length;
	(*package) & length;
	//not used

	double width;
	(*package) & width;
	//not used

	//segment
	unpackageGeneralPathMover(&(one_vehicle->fwdMovement));
	//one_vehicle->fwdMovement = *(unpackageGeneralPathMover());

	//After fwdMovement
	(*package) & (one_vehicle->latMovement);
	(*package) & (one_vehicle->fwdVelocity);
	(*package) & (one_vehicle->latVelocity);
	(*package) & (one_vehicle->fwdAccel);

	(*package) & (one_vehicle->posInIntersection);
	(*package) & (one_vehicle->error_state);

	return one_vehicle;
}

void UnPackageUtils::unpackageGeneralPathMover(GeneralPathMover* one_motor) {

	int path_size;
	(*package) & path_size;

	for (int i = 0; i < path_size; i++) {
		const sim_mob::RoadSegment* one_segment = unpackageRoadSegment();
		one_motor->fullPath.push_back(one_segment);
	}

	int move_size;
	(*package) & move_size;

	if (move_size >= 0) {
		one_motor->currSegmentIt = one_motor->fullPath.begin() + move_size;
	}

	bool hasPointList;
	(*package) & hasPointList;

	if (hasPointList) {
		(*package) & (one_motor->polypointsList);

		int current_poly;
		(*package) & current_poly;

		if (current_poly >= 0) {
			one_motor->currPolypoint = one_motor->polypointsList.begin() + current_poly;
		}

		int next_poly;
		(*package) & next_poly;

		if (next_poly >= 0) {
			one_motor->nextPolypoint = one_motor->polypointsList.begin() + next_poly;
		} else {
			//error
			one_motor->nextPolypoint = one_motor->polypointsList.end();
		}

		const std::vector<Point2D>& tempLaneZero =
				const_cast<RoadSegment*> (*(one_motor->currSegmentIt))->getLaneEdgePolyline(0);

		int current_zero_poly;
		(*package) & current_zero_poly;
		if (current_zero_poly >= 0) {
			one_motor->currLaneZeroPolypoint = tempLaneZero.begin() + current_zero_poly;
		}

		int next_zero_poly;
		(*package) & next_zero_poly;
		if (next_zero_poly >= 0) {
			one_motor->nextLaneZeroPolypoint = tempLaneZero.begin() + next_zero_poly;
		}
	}

	(*package) & (one_motor->distAlongPolyline);
	(*package) & (one_motor->distMovedInCurrSegment);
	(*package) & (one_motor->distOfThisSegment);
	(*package) & (one_motor->distOfRestSegments);
	(*package) & (one_motor->inIntersection);
	(*package) & (one_motor->isMovingForwardsInLink);
	(*package) & (one_motor->currLaneID);

	std::string buffer;
	(*package) & buffer;
	(one_motor->DebugStream) << buffer;

}

const Crossing* UnPackageUtils::unpackageCrossing() {
	bool hasSomthing;
	(*package) & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	sim_mob::Point2D near_1;
	sim_mob::Point2D near_2;
	sim_mob::Point2D far_1;
	sim_mob::Point2D far_2;

	(*package) & near_1;
	(*package) & near_2;
	(*package) & far_1;
	(*package) & far_2;

	return sim_mob::getCrossingBasedOnNode(&near_1, &near_2, &far_1, &far_2);
}

IntersectionDrivingModel* UnPackageUtils::unpackageIntersectionDrivingModel() {
	bool hasSomthing;
	(*package) & hasSomthing;

	if (hasSomthing == false)
		return NULL;

	SimpleIntDrivingModel* one_model = new SimpleIntDrivingModel();
	(*package) & (*one_model);
	return one_model;
}

FixedDelayed<DPoint*>& UnPackageUtils::unpackageFixedDelayedDPoint() {
	int delayMS;
	(*package) & delayMS;

	bool reclaimPtrs;
	(*package) & reclaimPtrs;

	FixedDelayed<DPoint*>* one_delay = new FixedDelayed<DPoint*> (delayMS, reclaimPtrs);

	int list_size;
	(*package) & list_size;

	for (int i = 0; i < list_size; i++) {
		DPoint value;
		int value2;

		(*package) & value;
		(*package) & value2;

		DPoint* buffer_value = new DPoint(value.x, value.y);
		uint32_t ut_value = value2;

		FixedDelayed<DPoint*>::HistItem one(buffer_value, ut_value);
		one_delay->history.push_back(one);
	}

	return (*one_delay);
}

FixedDelayed<double>& UnPackageUtils::unpackageFixedDelayedDouble() {
	int delayMS;
	(*package) & delayMS;

	bool reclaimPtrs;
	(*package) & reclaimPtrs;

	FixedDelayed<double>* one_delay = new FixedDelayed<double> (delayMS, reclaimPtrs);

	int list_size;
	(*package) & list_size;

	for (int i = 0; i < list_size; i++) {
		double value;
		int value2;

		(*package) & value;
		(*package) & value2;

		uint32_t ut_value = value2;

		FixedDelayed<double>::HistItem one(value, ut_value);
		one_delay->history.push_back(one);
	}

	return (*one_delay);
}

FixedDelayed<int>& UnPackageUtils::unpackageFixedDelayedInt() {
	int delayMS;
	(*package) & delayMS;

	bool reclaimPtrs;
	(*package) & reclaimPtrs;

	FixedDelayed<int>* one_delay = new FixedDelayed<int> (delayMS, reclaimPtrs);

	int list_size;
	(*package) & list_size;

	for (int i = 0; i < list_size; i++) {
		int value;
		int value2;

		(*package) & value;
		(*package) & value2;

		uint32_t ut_value = value2;

		FixedDelayed<int>::HistItem one(value, ut_value);
		one_delay->history.push_back(one);
	}

	return (*one_delay);
}

Point2D* UnPackageUtils::unpackagePoint2D() {
	Point2D* one_point = new Point2D(0, 0);
	(*package) & (*one_point);

	return one_point;
}
}
#endif
