/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "DriverUpdateParams.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

#ifndef SIMMOB_DISABLE_MPI
using namespace sim_mob;

namespace {

void DriverUpdateParams::pack(PackageUtils& package, const DriverUpdateParams* params) {

	if (params == NULL) {
		bool is_NULL = true;
		package.packBasicData(is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package.packBasicData(is_NULL);
	}

	package.packBasicData<double> (params->frameNumber);
	package.packBasicData<double> (params->currTimeMS);

	sim_mob::Lane::pack(package, params->currLane);
	package.packBasicData<int> (params->currLaneIndex);
	package.packBasicData<int> (params->fromLaneIndex);
	sim_mob::Lane::pack(package, params->leftLane);
	sim_mob::Lane::pack(package, params->rightLane);

	package.packBasicData<double> (params->currSpeed);
	package.packBasicData<double> (params->currLaneOffset);
	package.packBasicData<double> (params->currLaneLength);
	package.packBasicData<bool> (params->isTrafficLightStop);

	package.packBasicData<double> (params->trafficSignalStopDistance);
	package.packBasicData<double> (params->elapsedSeconds);
	package.packBasicData<double> (params->perceivedFwdVelocity);
	package.packBasicData<double> (params->perceivedLatVelocity);
	package.packBasicData<double> (params->perceivedFwdVelocityOfFwdCar);
	package.packBasicData<double> (params->perceivedLatVelocityOfFwdCar);
	package.packBasicData<double> (params->perceivedAccelerationOfFwdCar);
	package.packBasicData<double> (params->perceivedDistToFwdCar);

	package.packBasicData<double> (params->laneChangingVelocity);
	package.packBasicData<bool> (params->isCrossingAhead);
	package.packBasicData<bool> (params->isApproachingToIntersection);
	package.packBasicData<int> (params->crossingFwdDistance);

	package.packBasicData<double> (params->space);
	package.packBasicData<double> (params->a_lead);
	package.packBasicData<double> (params->v_lead);
	package.packBasicData<double> (params->space_star);
	package.packBasicData<double> (params->distanceToNormalStop);

	package.packBasicData<double> (params->dis2stop);
	package.packBasicData<bool> (params->isWaiting);

	package.packBasicData<bool> (params->justChangedToNewSegment);
	package.packDPoint(params->TEMP_lastKnownPolypoint);
	package.packBasicData<bool> (params->justMovedIntoIntersection);
	package.packBasicData<double> (params->overflowIntoIntersection);
}

void DriverUpdateParams::unpack(UnPackageUtils& unpackage, DriverUpdateParams* params) {
	bool is_NULL = unpackage.unpackBasicData<bool> ();
	if (is_NULL) {
		return;
	}

	params->frameNumber = unpackage.unpackBasicData<double> ();
	params->currTimeMS = unpackage.unpackBasicData<double> ();

	params->currLane = sim_mob::Lane::unpack(unpackage);
	params->currLaneIndex = unpackage.unpackBasicData<int> ();
	params->fromLaneIndex = unpackage.unpackBasicData<int> ();
	params->leftLane = sim_mob::Lane::unpack(unpackage);
	params->rightLane = sim_mob::Lane::unpack(unpackage);

	params->currSpeed = unpackage.unpackBasicData<double> ();
	params->currLaneOffset = unpackage.unpackBasicData<double> ();
	params->currLaneLength = unpackage.unpackBasicData<double> ();
	params->isTrafficLightStop = unpackage.unpackBasicData<bool> ();

	params->trafficSignalStopDistance = unpackage.unpackBasicData<double> ();
	params->elapsedSeconds = unpackage.unpackBasicData<double> ();
	params->perceivedFwdVelocity = unpackage.unpackBasicData<double> ();
	params->perceivedLatVelocity = unpackage.unpackBasicData<double> ();
	params->perceivedFwdVelocityOfFwdCar = unpackage.unpackBasicData<double> ();
	params->perceivedLatVelocityOfFwdCar = unpackage.unpackBasicData<double> ();
	params->perceivedAccelerationOfFwdCar = unpackage.unpackBasicData<double> ();
	params->perceivedDistToFwdCar = unpackage.unpackBasicData<double> ();
}

}

#endif
