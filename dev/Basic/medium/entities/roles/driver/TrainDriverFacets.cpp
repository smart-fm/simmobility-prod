/*
 * TrainDriverFacets.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */

#include "entities/roles/driver/TrainDriverFacets.hpp"
#include "entities/misc/TrainTrip.hpp"
#include "entities/Person_MT.hpp"
#include "entities/roles/driver/TrainDriver.hpp"
namespace {
const double safeDistanceInMetres = 100;
const double distanceArrvingAtPlatform = 0.1;
}
namespace sim_mob {
namespace medium{
TrainBehavior::TrainBehavior():BehaviorFacet(),parentDriver(nullptr)
{

}
TrainBehavior::~TrainBehavior()
{

}
void TrainBehavior::frame_init()
{
}
void TrainBehavior::frame_tick()
{

}
std::string TrainBehavior::frame_tick_output()
{
	return std::string();
}
TrainDriver* TrainBehavior::getParentDriver() const
{
	return parentDriver;
}

void TrainBehavior::setParentDriver(TrainDriver* parentDriver)
{
	if (!parentDriver)
	{
		throw std::runtime_error("parentDriver cannot be NULL");
	}
	this->parentDriver = parentDriver;
}

TrainMovement::TrainMovement():MovementFacet(),parentDriver(nullptr)
{

}
TrainMovement::~TrainMovement()
{

}
TravelMetric& TrainMovement::startTravelTimeMetric()
{
	return  travelMetric;
}

const TrainPathMover& TrainMovement::getPathMover() const
{
	return trainPathMover;
}

TravelMetric& TrainMovement::finalizeTravelTimeMetric()
{
	return  travelMetric;
}
void TrainMovement::frame_init()
{
	Person_MT* person = parentDriver->parent;
	const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*(person->currTripChainItem));
	if (!trip) {
		Print()<< "train trip is null"<<std::endl;
	} else {
		trainPathMover.setPath(trip->getTrainRoute());
		trainPlatformMover.setPlatforms(trip->getTrainPlatform());
	}
}
void TrainMovement::frame_tick()
{
	moveForward();
}
std::string TrainMovement::frame_tick_output()
{
	return std::string();
}
TrainDriver* TrainMovement::getParentDriver() const
{
	return parentDriver;
}

void TrainMovement::setParentDriver(TrainDriver* parentDriver)
{
	if (!parentDriver)
	{
		throw std::runtime_error("parentDriver cannot be NULL");
	}
	this->parentDriver = parentDriver;
}

double TrainMovement::getRealSpeedLimit()
{
	const TrainDriver* nextDriver = parentDriver->getNextDriver();
	double distanceToNextTrain = 0.0;
	double distanceToNextPlatform = 0.0;
	double distanceToNextObject = 0.0;
	double speedLimit = 0.0;
	if(nextDriver){
		TrainMovement* nextMovement = dynamic_cast<TrainMovement*>(nextDriver->movementFacet);
		if(nextMovement){
			distanceToNextTrain = trainPathMover.getDistanceToNextTrain(nextMovement->getPathMover())+safeDistanceInMetres;
			distanceToNextPlatform = trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());
		}
	}
	if(distanceToNextTrain==0.0||distanceToNextPlatform==0.0){
		distanceToNextObject = std::max(distanceToNextTrain, distanceToNextPlatform);
	} else {
		distanceToNextObject = std::min(distanceToNextTrain,distanceToNextPlatform);
	}

	double decelerate = trainPathMover.getCurrentDecelerationRate();
	speedLimit = std::sqrt(2.0*decelerate*distanceToNextObject);
	speedLimit = std::min(speedLimit, trainPathMover.getCurrentSpeedLimit());

	return speedLimit;
}

double TrainMovement::getEffectiveAccelerate()
{
	TrainUpdateParams& params = parentDriver->getParams();
	double effectiveSpeed = params.currentSpeed;
	double realSpeedLimit = getRealSpeedLimit();
	bool isAccelerated = false;
	if(effectiveSpeed<realSpeedLimit){
		isAccelerated = true;
	}
	if(isAccelerated){
		effectiveSpeed = effectiveSpeed+trainPathMover.getCurrentAccelerationRate()*params.secondsInTick;
		effectiveSpeed = std::min(effectiveSpeed, realSpeedLimit);
	} else {
		effectiveSpeed = effectiveSpeed-trainPathMover.getCurrentDecelerationRate()*params.secondsInTick;
		effectiveSpeed = std::max(effectiveSpeed, realSpeedLimit);
	}

	double effectiveAccelerate = (effectiveSpeed-params.currentSpeed)/params.secondsInTick;
	return effectiveAccelerate;
}

double TrainMovement::getEffectiveMovingDistance()
{
	TrainUpdateParams& params = parentDriver->getParams();
	double effectiveAcc = getEffectiveAccelerate();
	double movingDist = params.currentSpeed*params.secondsInTick+0.5*effectiveAcc*params.secondsInTick*params.secondsInTick;
	params.currentSpeed = params.currentSpeed+effectiveAcc*params.secondsInTick;
	return movingDist;
}

bool TrainMovement::isStopAtPlatform()
{
	double distanceToNextPlatform = trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());
	if(distanceToNextPlatform<distanceArrvingAtPlatform){
		return true;
	} else {
		return false;
	}
}
bool TrainMovement::moveForward()
{
	if(!isStopAtPlatform()){
		double movingDistance = getEffectiveMovingDistance();
		trainPathMover.advance(movingDistance);
		return true;
	}
	return false;
}
bool TrainMovement::isAtLastPlaform()
{
	return trainPlatformMover.isLastPlatform();
}
void TrainMovement::leaveFromPlaform()
{
	if(!isAtLastPlaform()){
		trainPlatformMover.getNextPlatform(true);
		moveForward();
	}
}
}
} /* namespace sim_mob */
