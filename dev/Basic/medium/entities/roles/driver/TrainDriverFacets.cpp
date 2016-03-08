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
#include "entities/TrainController.hpp"
#include "message/MT_Message.hpp"
#include "message/MessageBus.hpp"
#include "message/MessageHandler.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

namespace {
const double distanceArrvingAtPlatform = 0.1;
const double trainLengthMeter = 124;
const double convertKmPerHourToMeterPerSec = 1000.0/3600.0;
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

TrainMovement::TrainMovement():MovementFacet(),parentDriver(nullptr),safeDistance(0),safeHeadway(0)
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	safeDistance = config.trainController.safeDistance;
	safeHeadway = config.trainController.safeHeadway;
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
	TrainUpdateParams& params = parentDriver->getParams();

	const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*(parentDriver->getParent()->currTripChainItem));
    const std::string& fileName("pt_mrt_move.csv");
    sim_mob::BasicLogger& ptMRTMoveLogger  = sim_mob::Logger::log(fileName);
    DailyTime startTime = ConfigManager::GetInstance().FullConfig().simStartTime();
    ptMRTMoveLogger << DailyTime(params.now.ms()+startTime.getValue()).getStrRepr() << ",";
    if(trip) ptMRTMoveLogger << trip->getLineId() << ",";
    if(trip) ptMRTMoveLogger << trip->getTripId() << ",";
    ptMRTMoveLogger << params.currentSpeed/convertKmPerHourToMeterPerSec << ",";
     Platform* next = trainPlatformMover.getNextPlatform();
    std::string platformNo("");
    if(next){
    	platformNo = next->getPlatformNo();
		ptMRTMoveLogger << platformNo << ",";
		ptMRTMoveLogger << trainPathMover.getDistanceToNextPlatform(next) << ",";
    }
    ptMRTMoveLogger << trainPathMover.getCurrentPosition().getX() << ",";
    ptMRTMoveLogger << trainPathMover.getCurrentPosition().getY() << ",";
    ptMRTMoveLogger << params.currentAcelerate << ",";
    ptMRTMoveLogger << params.currCase << ",";
    ptMRTMoveLogger << this->parentDriver->waitingTimeSec << std::endl;
    //sim_mob::Logger::log(fileName).flush();

	TrainDriver::TRAIN_STATUS status = parentDriver->getCurrentStatus();
	switch(status){
	case TrainDriver::MOVE_TO_PLATFROM:
	{
		moveForward();
		if(isStopAtPlatform()){
			parentDriver->setCurrentStatus(TrainDriver::ARRIVAL_AT_PLATFORM);
		}
		break;
	}
	case TrainDriver::WAITING_LEAVING:
	{
		parentDriver->reduceWaitingTime(params.secondsInTick);
		double waitingTime = parentDriver->getWaitingTime();
		params.currentSpeed = 0.0;
		params.currentAcelerate = 0.0;
		if(waitingTime<params.secondsInTick){
			parentDriver->setCurrentStatus(TrainDriver::LEAVING_FROM_PLATFORM);
			if(!isAtLastPlaform()){
				leaveFromPlaform();
				params.elapsedSeconds = waitingTime;
			} else {
				parentDriver->getParent()->setToBeRemoved();
			}
		}
		break;
	}
	}


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
	TrainUpdateParams& params = parentDriver->getParams();
	const TrainDriver* nextDriver = parentDriver->getNextDriver();
	double distanceToNextTrain = 0.0;
	double distanceToNextPlatform = 0.0;
	double distanceToNextObject = 0.0;
	double speedLimit = 0.0;
	double speedLimit2 = 0.0;
	distanceToNextPlatform = trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());
	if(nextDriver){
		TrainMovement* nextMovement = dynamic_cast<TrainMovement*>(nextDriver->movementFacet);
		if(nextMovement){
			double dis = trainPathMover.getDistanceToNextTrain(nextMovement->getPathMover());
			if(dis>0){
				distanceToNextTrain = dis-safeDistance-trainLengthMeter;
			}
		}
	}

	/*if ((distanceToNextPlatform != 0.0 && distanceToNextTrain == 0.0)
			|| (distanceToNextTrain != 0.0 && distanceToNextPlatform != 0.0
					&& distanceToNextPlatform < distanceToNextTrain)) {
		double decelerate = trainPathMover.getCurrentDecelerationRate();
		speedLimit = std::sqrt(2.0 * decelerate * distanceToNextPlatform);
		if (params.currentSpeed > speedLimit) {
			params.currCase = TrainUpdateParams::STATION_CASE;
			params.currentSpeedLimit = speedLimit;
			params.disToNextPlatform = distanceToNextPlatform;
			return speedLimit;
		}
	}*/

	params.currCase = TrainUpdateParams::NORMAL_CASE;
	if(distanceToNextTrain==0.0||distanceToNextPlatform==0.0){
		distanceToNextObject = std::max(distanceToNextTrain, distanceToNextPlatform);
	} else {
		distanceToNextObject = std::min(distanceToNextTrain,distanceToNextPlatform);
	}

	if(distanceToNextObject<0){
		speedLimit = 0.0;
	} else {
		double decelerate = trainPathMover.getCurrentDecelerationRate();
		speedLimit = std::sqrt(2.0*decelerate*distanceToNextObject);
	}
	speedLimit = std::min(speedLimit, trainPathMover.getCurrentSpeedLimit()*convertKmPerHourToMeterPerSec);

	if (distanceToNextTrain > 0 && safeHeadway > 0) {
		speedLimit2 = distanceToNextTrain / safeHeadway;
	}
	if (speedLimit2 > 0) {
		speedLimit = std::min(speedLimit, speedLimit2);
	}
	params.currentSpeedLimit = speedLimit;
	return speedLimit;
}

double TrainMovement::getEffectiveAccelerate()
{
	TrainUpdateParams& params = parentDriver->getParams();

	double effectiveSpeed = params.currentSpeed;
	double realSpeedLimit = getRealSpeedLimit();
	if(params.currCase==TrainUpdateParams::STATION_CASE){
		double effectiveAccelerate = -effectiveSpeed*effectiveSpeed/(2.0*params.disToNextPlatform);
		params.currentAcelerate = effectiveAccelerate;
		return effectiveAccelerate;
	}

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
	params.currentAcelerate = effectiveAccelerate;
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
		double distanceToNextPlat = trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());
		TrainUpdateParams& params = parentDriver->getParams();
		if(movingDistance>distanceToNextPlat){
			trainPathMover.advance(distanceToNextPlat);
			params.disToNextPlatform = 0.0;
		} else {
			trainPathMover.advance(movingDistance);
			params.disToNextPlatform = trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());
		}
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
	if (!isAtLastPlaform()) {
		moveForward();
		Platform* next = trainPlatformMover.getNextPlatform(true);
		std::string stationNo = next->getStationNo();
		Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
		messaging::MessageBus::PostMessage(stationAgent,TRAIN_MOVETO_NEXT_PLATFORM,
				messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver)));
	}
}
void TrainMovement::arrivalAtStartPlaform() const
{
	Platform* next = trainPlatformMover.getFirstPlatform();
	std::string stationNo = next->getStationNo();
	Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
	messaging::MessageBus::PostMessage(stationAgent, TRAIN_ARRIVAL_AT_TERMINAL,
			messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver)));
}

}
} /* namespace sim_mob */
