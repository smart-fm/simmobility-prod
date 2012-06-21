/* Copyright Singapore-MIT Alliance for Research and Technology */
#include "GenConfig.h"
#include "util/LangHelpers.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include "entities/vehicle/Vehicle.hpp"
#include "entities/roles/driver/GeneralPathMover.hpp"
#include "entities/roles/driver/DriverUpdateParams.hpp"
#include "entities/roles/driver/IntersectionDrivingModel.hpp"
#include "entities/misc/TripChain.hpp"

#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"

#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Point2D.hpp"
#include "partitions/ParitionDebugOutput.hpp"

/*
 * \author Xu Yan
 */

namespace sim_mob {

/*
 * Serialize Class vehicles
 */
void sim_mob::Vehicle::pack(PackageUtils& package, Vehicle* one_vehicle) {

	ParitionDebugOutput debug;
//	double test_8 = 888.555;
//	package << (test_8);

	GeneralPathMover::pack(package, &(one_vehicle->fwdMovement));

//	double test_9 = 999.555;
//	package << (test_9);

	package << one_vehicle->latMovement;
	package << one_vehicle->fwdVelocity;
	package << one_vehicle->latVelocity;
	package << one_vehicle->fwdAccel;

	package << one_vehicle->posInIntersection.x;
	package << one_vehicle->posInIntersection.y;
	package << one_vehicle->error_state;
}

Vehicle* sim_mob::Vehicle::unpack(UnPackageUtils& unpackage) {
	Vehicle* one_vehicle = new Vehicle();

	ParitionDebugOutput debug;
//	double test_8 = 1;
//	unpackage >> (test_8);
//
//	debug.outputToConsole("test_8");
//	debug.outputToConsole(test_8);

	GeneralPathMover::unpack(unpackage, &(one_vehicle->fwdMovement));

//	double test_9 = 1;
//	unpackage >> (test_9);
//
//	debug.outputToConsole("test_9");
//	debug.outputToConsole(test_9);

	unpackage >> one_vehicle->latMovement;
	unpackage >> one_vehicle->fwdVelocity;
	unpackage >> one_vehicle->latVelocity;
	unpackage >> one_vehicle->fwdAccel;

	unpackage >> one_vehicle->posInIntersection.x;
	unpackage >> one_vehicle->posInIntersection.y;
	unpackage >> one_vehicle->error_state;

	return one_vehicle;
}

/**
 * Serialize Class GeneralPathMover
 */

void sim_mob::GeneralPathMover::pack(PackageUtils& package, GeneralPathMover* fwdMovement)
{
	if (fwdMovement == NULL)
	{
		bool is_NULL = true;
		package<<(is_NULL);
		return;
	}
	else
	{
		bool is_NULL = false;
		package<<(is_NULL);
	}

//	ParitionDebugOutput::outputToConsole("sssssssss");

	int path_size = fwdMovement->fullPath.size();
	package << (path_size);

	std::vector<const sim_mob::RoadSegment*>::const_iterator itr = fwdMovement->fullPath.begin();
	for (; itr != fwdMovement->fullPath.end(); itr++)
	{
		RoadSegment::pack(package, *itr);
	}

//	ParitionDebugOutput::outputToConsole("ddddddddd");

	int current_segment = fwdMovement->currSegmentIt - fwdMovement->fullPath.begin();
	package << (current_segment);
//	ParitionDebugOutput::outputToConsole(current_segment);

//	std::cout << "BBB" << std::endl;

	//part 2
	//polypointsList
	if (fwdMovement->polypointsList.size() > 0)
	{

//		ParitionDebugOutput::outputToConsole("d11111111");

		bool hasPointList = true;
		package << (hasPointList);

//		ParitionDebugOutput::outputToConsole("d22222222222");
		package << (fwdMovement->polypointsList);

		int current_poly = fwdMovement->currPolypoint - fwdMovement->polypointsList.begin();
		package << (current_poly);

//		ParitionDebugOutput::outputToConsole("d3333333333");

		int next_poly = fwdMovement->nextPolypoint - fwdMovement->polypointsList.begin();
		package << (next_poly);

//		ParitionDebugOutput::outputToConsole("d4444444444444");

		bool hasArrivedDestination = false;
		if (fwdMovement->currSegmentIt == fwdMovement->fullPath.end())
		{
			hasArrivedDestination = true;

			//			ParitionDebugOutput::outputToConsole("d43333333");
		}

		package << (hasArrivedDestination);

		if (hasArrivedDestination == false)
		{
			//copy from GeneralPathMover.cpp, the design of current_zero_poly is not good.
			//I thought current_zero_poly is the iterator of currPolypoint
			const std::vector<Point2D>& tempLaneZero = const_cast<RoadSegment*> (*(fwdMovement->currSegmentIt))->getLaneEdgePolyline(0);

//			ParitionDebugOutput::outputToConsole("d44455555555");

			int current_zero_poly = fwdMovement->currLaneZeroPolypoint - tempLaneZero.begin();
			package << (current_zero_poly);

//			ParitionDebugOutput::outputToConsole("d55555555");

			int next_zero_poly = fwdMovement->nextLaneZeroPolypoint - tempLaneZero.begin();
			package << (next_zero_poly);

//			ParitionDebugOutput::outputToConsole("d66666666666");
		}
	}
	else
	{
		bool hasPointList = false;
		package << (hasPointList);

//		ParitionDebugOutput::outputToConsole("d0000000000");
	}

//	ParitionDebugOutput::outputToConsole("fffffffffffff");

	//part 3
	//Others
	package << (fwdMovement->distAlongPolyline);
	package << (fwdMovement->distMovedInCurrSegment);
	package << (fwdMovement->distOfThisSegment);
	package << (fwdMovement->distOfRestSegments);

//	package << (fwdMovement->inIntersection);
	package << (fwdMovement->isMovingForwardsInLink);
	package << (fwdMovement->currLaneID);

	std::string value_ = fwdMovement->DebugStream.str();
	package << (value_);

//	ParitionDebugOutput::outputToConsole("ggggggggggg");
}

void sim_mob::GeneralPathMover::unpack(UnPackageUtils& unpackage, GeneralPathMover* one_motor)
{
	bool is_NULL = false;
	unpackage >> is_NULL;
	if (is_NULL)
	{
		return;
	}

	int path_size = 0;
	unpackage >> path_size;

	for (int i = 0; i < path_size; i++)
	{
		const sim_mob::RoadSegment* one_segment = sim_mob::RoadSegment::unpack(unpackage);
		one_motor->fullPath.push_back(one_segment);
	}

	int move_size = 0;
	unpackage >> move_size;

	if (move_size >= 0)
	{
		one_motor->currSegmentIt = one_motor->fullPath.begin() + move_size;
	}

	bool hasPointList = false;
	unpackage >> hasPointList;

	if (hasPointList)
	{
		unpackage >> one_motor->polypointsList;

		int current_poly = 0;
		unpackage >> current_poly;

		if (current_poly >= 0)
		{
			one_motor->currPolypoint = one_motor->polypointsList.begin() + current_poly;
		}

		int next_poly = 0;
		unpackage >> next_poly;

		if (next_poly >= 0)
		{
			one_motor->nextPolypoint = one_motor->polypointsList.begin() + next_poly;
		}

		bool hasArrivedDestination = false;
		unpackage >> hasArrivedDestination;

		if (hasArrivedDestination == false)
		{
			const std::vector<Point2D>& tempLaneZero = const_cast<RoadSegment*> (*(one_motor->currSegmentIt))->getLaneEdgePolyline(0);

			int current_zero_poly = 0;
			unpackage >> current_zero_poly;
			if (current_zero_poly >= 0)
			{
				one_motor->currLaneZeroPolypoint = tempLaneZero.begin() + current_zero_poly;
			}

			int next_zero_poly = 0;
			unpackage >> next_zero_poly;
			if (next_zero_poly >= 0)
			{
				one_motor->nextLaneZeroPolypoint = tempLaneZero.begin() + next_zero_poly;
			}
		}
	}

	unpackage >> one_motor->distAlongPolyline;
	unpackage >> one_motor->distMovedInCurrSegment;
	unpackage >> one_motor->distOfThisSegment;
	unpackage >> one_motor->distOfRestSegments;
//	unpackage >> one_motor->inIntersection;
	unpackage >> one_motor->isMovingForwardsInLink;
	unpackage >> one_motor->currLaneID;

//	one_motor->distAlongPolyline = unpackage.unpackBasicData<double> ();
//	one_motor->distMovedInCurrSegment = unpackage.unpackBasicData<double> ();
//	one_motor->distOfThisSegment = unpackage.unpackBasicData<double> ();
//	one_motor->distOfRestSegments = unpackage.unpackBasicData<double> ();
//	one_motor->inIntersection = unpackage.unpackBasicData<bool> ();
//	one_motor->isMovingForwardsInLink = unpackage.unpackBasicData<bool> ();
//	one_motor->currLaneID = unpackage.unpackBasicData<int> ();

	std::string buffer;
	unpackage >> buffer;
	(one_motor->DebugStream) << buffer;

}

/**
 * Serialize Class DriverUpdateParams
 */
void DriverUpdateParams::pack(PackageUtils& package, const DriverUpdateParams* params) {

	if (params == NULL)
	{
		bool is_NULL = true;
		package << (is_NULL);
		return;
	}
	else
	{
		bool is_NULL = false;
		package << (is_NULL);
	}

	package << (params->frameNumber);
	package << (params->currTimeMS);

	sim_mob::Lane::pack(package, params->currLane);
	package << (params->currLaneIndex);
//	package << (params->fromLaneIndex);
	sim_mob::Lane::pack(package, params->leftLane);
	sim_mob::Lane::pack(package, params->rightLane);

	package << (params->currSpeed);
	package << (params->currLaneOffset);
	package << (params->currLaneLength);
//	package << (params->isTrafficLightStop);

	package << (params->trafficSignalStopDistance);
	package << (params->elapsedSeconds);
	package << (params->perceivedFwdVelocity);
	package << (params->perceivedLatVelocity);
	package << (params->perceivedFwdVelocityOfFwdCar);
	package << (params->perceivedLatVelocityOfFwdCar);
	package << (params->perceivedAccelerationOfFwdCar);
	package << (params->perceivedDistToFwdCar);

	package << (params->laneChangingVelocity);
	package << (params->isCrossingAhead);
	package << (params->isApproachingToIntersection);
	package << (params->crossingFwdDistance);

	package << (params->space);
	package << (params->a_lead);
	package << (params->v_lead);
	package << (params->space_star);
	package << (params->distanceToNormalStop);

	package << (params->dis2stop);
	package << (params->isWaiting);

	package << (params->justChangedToNewSegment);
	package << (params->TEMP_lastKnownPolypoint);
	package << (params->justMovedIntoIntersection);
	package << (params->overflowIntoIntersection);
}

void DriverUpdateParams::unpack(UnPackageUtils& unpackage, DriverUpdateParams* params) {
	bool is_NULL = false;
	unpackage >> is_NULL;
	if (is_NULL) {
		return;
	}

	unpackage >> params->frameNumber;
	unpackage >> params->currTimeMS;

//	params->frameNumber = unpackage.unpackBasicData<double> ();
//	params->currTimeMS = unpackage.unpackBasicData<double> ();

	params->currLane = sim_mob::Lane::unpack(unpackage);
	unpackage >> params->currLaneIndex;
//	unpackage >> params->fromLaneIndex;
	params->leftLane = sim_mob::Lane::unpack(unpackage);
	params->rightLane = sim_mob::Lane::unpack(unpackage);

	unpackage >> params->currSpeed;
	unpackage >> params->currLaneOffset;
	unpackage >> params->currLaneLength;
//	unpackage >> params->isTrafficLightStop;

//	params->currSpeed = unpackage.unpackBasicData<double> ();
//	params->currLaneOffset = unpackage.unpackBasicData<double> ();
//	params->currLaneLength = unpackage.unpackBasicData<double> ();
//	params->isTrafficLightStop = unpackage.unpackBasicData<bool> ();

	unpackage >> params->trafficSignalStopDistance;
	unpackage >> params->elapsedSeconds;
	unpackage >> params->perceivedFwdVelocity;
	unpackage >> params->perceivedLatVelocity;
	unpackage >> params->perceivedFwdVelocityOfFwdCar;
	unpackage >> params->perceivedLatVelocityOfFwdCar;
	unpackage >> params->perceivedAccelerationOfFwdCar;
	unpackage >> params->perceivedDistToFwdCar;

//	params->trafficSignalStopDistance = unpackage.unpackBasicData<double> ();
//	params->elapsedSeconds = unpackage.unpackBasicData<double> ();
//	params->perceivedFwdVelocity = unpackage.unpackBasicData<double> ();
//	params->perceivedLatVelocity = unpackage.unpackBasicData<double> ();
//	params->perceivedFwdVelocityOfFwdCar = unpackage.unpackBasicData<double> ();
//	params->perceivedLatVelocityOfFwdCar = unpackage.unpackBasicData<double> ();
//	params->perceivedAccelerationOfFwdCar = unpackage.unpackBasicData<double> ();
//	params->perceivedDistToFwdCar = unpackage.unpackBasicData<double> ();
}

/**
 * Class IntersectionDrivingModel
 */

void SimpleIntDrivingModel::pack(PackageUtils& package, const SimpleIntDrivingModel* params) {
	if (params == NULL) {
		bool is_NULL = true;
		package << (is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package << (is_NULL);
	}

	ParitionDebugOutput debug;

//	std::cout << "Before:" << params->intTrajectory.getAngle() << std::endl;
//	std::cout << "Before totalMovement:" << params->totalMovement << std::endl;

	package << (params->intTrajectory);
//	debug.outputToConsole("TAC 11");

	package << (params->totalMovement);
//	debug.outputToConsole("TAC 22");
}

void SimpleIntDrivingModel::unpack(UnPackageUtils& unpackage, SimpleIntDrivingModel* params) {
	ParitionDebugOutput debug;

	bool is_NULL = false;
	unpackage >> is_NULL;
	if (is_NULL) {
		return;
	}

	unpackage >> (params->intTrajectory);
//	debug.outputToConsole("SimpleIntDrivingModel 11");
//	std::cout << "After:" << params->intTrajectory.getAngle() << std::endl;

	unpackage >> (params->totalMovement);
//	debug.outputToConsole("SimpleIntDrivingModel 22");
//	std::cout << "After totalMovement:" << params->totalMovement << std::endl;
//	params->totalMovement = unpackage.unpackBasicData<double> ();
}

/**
 * Class TripChain
 */
void TripChain::pack(PackageUtils& package, const TripChain* chain) {
	if (chain == NULL) {
		bool is_NULL = true;
		package<<(is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package<<(is_NULL);
	}

	package << (chain->from.description);
	sim_mob::Node::pack(package, chain->from.location);

	package << (chain->to.description);
	sim_mob::Node::pack(package, chain->to.location);

	package << (chain->primary);
	package << (chain->flexible);

	package << (chain->startTime);
	package << (chain->mode);
}

TripChain* TripChain::unpack(UnPackageUtils& unpackage) {
	bool is_NULL = false;
	unpackage >> is_NULL;
	if (is_NULL) {
		return NULL;
	}

	TripChain* chain = new TripChain();

	unpackage >> chain->from.description;
//
//	chain->from.description = unpackage.unpackBasicData<std::string> ();
	chain->from.location = Node::unpack(unpackage);

	unpackage >> chain->to.description;
//	chain->to.description = unpackage.unpackBasicData<std::string> ();
	chain->to.location = Node::unpack(unpackage);

	unpackage >> chain->primary;
	unpackage >> chain->flexible;
	unpackage >> chain->startTime;
	unpackage >> chain->mode;

	return chain;
}
}

#endif
