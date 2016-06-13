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
#include <iostream>
#include "stdlib.h"
//#include "shared/entities/Person.hpp"
using namespace std;
namespace {
const double distanceArrvingAtPlatform = 0.001;
const double trainLengthMeter = 138;
const double convertKmPerHourToMeterPerSec = 1000.0/3600.0;
}
namespace sim_mob {
namespace medium{
bool TrainMovement::areColumnNamesAdded=false;
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

TrainMovement::TrainMovement():MovementFacet(),parentDriver(nullptr),safeDistance(0),safeHeadway(0),nextPlatform(nullptr)
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	safeDistance = config.trainController.safeDistance;
	safeHeadway = config.trainController.safeHeadway;
}

void TrainMovement::ResetSafeHeadWay(double safeHeadWay)
{
	this->safeHeadway = safeHeadWay;
}

void TrainMovement::ResetSafeDistance(double safeDistance)
{
	this->safeHeadway = safeDistance;
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
Platform* TrainMovement::getNextPlatform()
{
	facetMutex.lock();
	Platform* next = nextPlatform;
	facetMutex.unlock();
	return next;
}
TravelMetric& TrainMovement::finalizeTravelTimeMetric()
{
	return  travelMetric;
}
void TrainMovement::frame_init()
{
	Person_MT* person = parentDriver->parent;
	const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*(person->currTripChainItem));
	if (!trip)
	{
		Print()<< "train trip is null"<<std::endl;
	}
	else
	{
		trainPathMover.setPath(trip->getTrainRoute());
		trainPlatformMover.setPlatforms(trip->getTrainPlatform());
		Platform* next = trainPlatformMover.getNextPlatform();
		facetMutex.lock();
		nextPlatform = next;
		facetMutex.unlock();
	}
}

void TrainMovement::ChangeTrip()
{
	Person_MT* person = parentDriver->parent;
	std::string lineId=parentDriver->getTrainLine();
	Platform *platform=getNextPlatform();
	TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
	std::string oppLineId=trainController->GetOppositeLineId(lineId);
	TrainTrip* trip = dynamic_cast<TrainTrip*>(*(person->currTripChainItem));
	//TrainTrip* trainTrip = new TrainTrip();
	trip->setLineId(oppLineId);
	std::vector<Block*>route;
	std::vector<Platform*> platforms;
	trainController->getTrainRoute(oppLineId,route);
	trip->setTrainRoute(route);
	trainController->getTrainPlatforms(oppLineId,platforms);
	trip->setTrainPlatform(platforms);
	std::string stationName=platform->getStationNo();
	TakeUTurn(stationName);
}

bool TrainMovement::CheckIfTrainsAreApprochingOrAtPlatform(std::string platformNo,std::string lineID)
{
   //get the vector of train drivers for that line
	TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
	typename  std::vector <Role<Person_MT>*> trainDriverVector=trainController->GetActiveTrainsForALine(lineID);
	//std::vector<Role<Person_MT>*> trainDriverVector;
    std::vector<Role<Person_MT>*>::iterator it;
    for(it=trainDriverVector.begin();it!=trainDriverVector.end();it++)
    {
    	TrainDriver *tDriver=dynamic_cast<TrainDriver*>(*(it));
    	if(tDriver)
    	{
    		MovementFacet *moveFacet=tDriver->GetMovement();
    		if(moveFacet)
    		{
    			TrainMovement* trainMovement=dynamic_cast<TrainMovement*>(moveFacet);
                if(trainMovement)
                {
                   Platform *platform=trainMovement->getNextPlatform();
                   if(platform)
                   {
                	   if(boost::iequals(platform->getPlatformNo(),platformNo))
                		   return true;
                   }
                }
    		}
    	}
    }
return false;
}

bool TrainMovement::CheckSafeHeadWayBeforeTeleport(std::string platformNo,std::string lineID)
{
	TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
	typename  std::vector <Role<Person_MT>*> trainDriverVector=trainController->GetActiveTrainsForALine(lineID);
    TrainPlatform trainPlatform=trainController->GetNextPlatform(platformNo,lineID);
    Platform *platform=trainController->GetPlatformFromId(platformNo);
    typename std::vector <Role<Person_MT>*>::iterator it=trainDriverVector.begin();
    double minDis=-1;
    TrainDriver *nextDriverInOppLine;
    while(it!=trainDriverVector.end())
    {

    	MovementFacet *movementFacet=(*it)->Movement();
    	if(movementFacet)
    	{
    		TrainMovement *trainMovement=dynamic_cast<TrainMovement*>(movementFacet);
    		if(trainMovement)
    		{
                 Platform *nextPlatform=trainMovement->getNextPlatform();
                 if(nextPlatform)
                 {
                	 std::string nextPlatformNo=nextPlatform->getPlatformNo();
                	 if(boost::iequals(trainPlatform.platformNo,nextPlatformNo))
                	 {
                		     //const TrainPathMover &trainPathMover=trainMovement->getPathMover();

                			 double disCovered=getTotalCoveredDistance();

                             double disOfPlatform=GetDistanceFromStartToPlatform(lineID,platform);
                        	 if(minDis==-1||disCovered-disOfPlatform<minDis)
                        	 {
                        		 minDis=disCovered-disOfPlatform;
                        		 nextDriverInOppLine = dynamic_cast<TrainDriver*>((*it));
                        	 }

                             if(disCovered-disOfPlatform<=safeDistance)
                             {

                            	 return false;
                             }

                	 }
                 }
    		}
    	}
    }
  TrainDriver * trainDriver=this->getParentDriver();
  if(trainDriver)
  {
	trainDriver->SetTrainDriverInOpposite(nextDriverInOppLine);
  }
return true;
}

double TrainMovement::getTotalCoveredDistance()
{
	return trainPathMover.getTotalCoveredDistance();
}

double TrainMovement::GetDistanceFromStartToPlatform(std::string lineID,Platform *platform)
{
  return trainPathMover.GetDistanceFromStartToPlatform(lineID,platform);
}


void TrainMovement::TakeUTurn(std::string stationName)
{
    //std::string platformName=getNextPlatform();
    TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
    Person_MT* person = parentDriver->parent;
    TrainTrip* trip = dynamic_cast<TrainTrip*>(*(person->currTripChainItem));
    std::string lineId=trip->getLineId();
    Platform *platform=parentDriver->getNextPlatform();
    std::string stationId=platform->getStationNo();
    Station *station=trainController->GetStationFromId(stationId);
    Platform *oppPlatform=station->getPlatform(lineId);
    std::vector<Block *> blocks=trip->getTrainRoute();
    trainPathMover.setPath(trip->getTrainRoute());
    trainPathMover.TeleportToOppositeLine(stationId,lineId,oppPlatform);
    facetMutex.lock();
    nextPlatform = oppPlatform;
    facetMutex.unlock();
    TrainDriver *parentDriver=getParentDriver();
    parentDriver->setNextDriver(parentDriver->GetDriverInOppositeLine());

}

void TrainMovement::produceDwellTimeInfo()
{
	const std::string& fileName("pt_mrt_dwellTime.csv");
	sim_mob::BasicLogger& ptMRTMoveLogger  = sim_mob::Logger::log(fileName);
	if(TrainMovement::areColumnNamesAdded==false)
	{

		ptMRTMoveLogger <<"TimeOfDay"<<",";
		ptMRTMoveLogger<<"TrainId"<<",";
		ptMRTMoveLogger<<"TripId"<<",";
		ptMRTMoveLogger<<"PlatformId"<<",";
		ptMRTMoveLogger<<"Dwell Time"<< std::endl;
		TrainMovement::areColumnNamesAdded=true;
		std::cout<<"column names added";
	}
	TrainUpdateParams& params = parentDriver->getParams();
	DailyTime startTime = ConfigManager::GetInstance().FullConfig().simStartTime();
	const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*(parentDriver->getParent()->currTripChainItem));
	if (trip&&trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform())==0)
	{
		ptMRTMoveLogger << DailyTime(params.now.ms()+startTime.getValue()).getStrRepr() << ",";
		ptMRTMoveLogger << trip->getTrainId() << ",";
		ptMRTMoveLogger <<trip->getTripId()<<",";
		ptMRTMoveLogger << trainPlatformMover.getNextPlatform()->getStationNo()<<",";
		ptMRTMoveLogger << this->parentDriver->initialDwellTime << std::endl;

	}

}
void TrainMovement::produceMoveInfo()
{
	TrainUpdateParams& params = parentDriver->getParams();
	const TrainTrip* trip = dynamic_cast<const TrainTrip*>(*(parentDriver->getParent()->currTripChainItem));
    const std::string& fileName("pt_mrt_move.csv");
    sim_mob::BasicLogger& ptMRTMoveLogger  = sim_mob::Logger::log(fileName);
    DailyTime startTime = ConfigManager::GetInstance().FullConfig().simStartTime();
    ptMRTMoveLogger << DailyTime(params.now.ms()+startTime.getValue()).getStrRepr() << ",";

    if(trip) ptMRTMoveLogger << trip->getLineId() << ",";
    if(trip) ptMRTMoveLogger << trip->getTrainId() << ",";
    if(trip) ptMRTMoveLogger << trip->getTripId() << ",";
    ptMRTMoveLogger << params.currentSpeed/convertKmPerHourToMeterPerSec << ",";
    Platform* next = trainPlatformMover.getNextPlatform();
    std::string platformNo("");
    if(next)
    {
    	platformNo = next->getPlatformNo();
		ptMRTMoveLogger << platformNo << ",";
		ptMRTMoveLogger << trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform()) << ",";
    }
    ptMRTMoveLogger << trainPathMover.getTotalCoveredDistance() << ",";
    ptMRTMoveLogger << trainPathMover.getCurrentPosition().getX() << ",";
    ptMRTMoveLogger << trainPathMover.getCurrentPosition().getY() << ",";
    ptMRTMoveLogger << params.currentAcelerate << ",";
    ptMRTMoveLogger << parentDriver->getPassengers().size()<<",";
    ptMRTMoveLogger << parentDriver->getWaitingTime() << std::endl;
    produceDwellTimeInfo();
    passengerInfo();
}

 void TrainMovement::passengerInfo()
 {
	 TrainUpdateParams& params = parentDriver->getParams();
	 const std::string& fileName("pt_mrt_passengernfo.csv");
	 sim_mob::BasicLogger& ptMRTMoveLogger  = sim_mob::Logger::log(fileName);
	 std::list <Passenger*>pList=parentDriver->getPassengers();
	 DailyTime startTime = ConfigManager::GetInstance().FullConfig().simStartTime();
	 std::list<Passenger*>::const_iterator it=pList.begin();
	 while(it!=pList.end())
	 {

		 ptMRTMoveLogger << DailyTime(params.now.ms()+startTime.getValue()).getStrRepr() << ","<<(*it)->getParent()->GetId()<<","<<getParentDriver()->getTrainId()<<","<<getParentDriver()->getTripId()<<","<<(*it)->getParent()->currSubTrip->origin.platform->getPlatformNo()<<","<<(*it)->getParent()->currSubTrip->destination.platform->getPlatformNo()<<std::endl;
		 it++;
	 }


 }
void TrainMovement::frame_tick()
{
	produceMoveInfo();
	TrainUpdateParams& params = parentDriver->getParams();
    parentDriver->updatePassengers();
	TrainDriver::TRAIN_NEXTREQUESTED requested = parentDriver->getNextRequested();
	switch(requested){
	case TrainDriver::REQUESTED_TO_PLATFROM:
	{
		moveForward();
		if(isStopAtPlatform()){
			parentDriver->setNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
		}
		break;
	}
	case TrainDriver::REQUESTED_WAITING_LEAVING:
	{
		parentDriver->reduceWaitingTime(params.secondsInTick);
		double waitingTime = parentDriver->getWaitingTime();
		params.currentSpeed = 0.0;
		params.currentAcelerate = 0.0;
		if(waitingTime<0){
			parentDriver->setWaitingTime(0.0);
		}
		if(waitingTime<params.secondsInTick){
			bool isDisruptedPlatform = false;
			if(parentDriver->disruptionParam.get()){
				const std::vector<std::string>& platformNames = parentDriver->disruptionParam->platformNames;
				Platform* cur = trainPlatformMover.getPlatformByOffset(0);
				Platform* next = trainPlatformMover.getPlatformByOffset(1);
				if(cur){
					auto it = std::find(platformNames.begin(), platformNames.end(), cur->getPlatformNo());
					if(it!=platformNames.end()){
						isDisruptedPlatform = true;
					}
				}
				if(next && !isDisruptedPlatform){
					auto it = std::find(platformNames.begin(), platformNames.end(), next->getPlatformNo());
					if(it!=platformNames.end()){
						parentDriver->setNextRequested(TrainDriver::REQUESTED_LEAVING_PLATFORM);
						parentDriver->getParent()->setToBeRemoved();
						arrivalAtEndPlatform();
						isDisruptedPlatform = true;
					}
				}
			}
			if(!isDisruptedPlatform){
				parentDriver->setNextRequested(TrainDriver::REQUESTED_LEAVING_PLATFORM);
				if(!isAtLastPlaform()){
					params.elapsedSeconds = waitingTime;
					parentDriver->setWaitingTime(0.0);
					leaveFromPlaform();
				} else {
					parentDriver->getParent()->setToBeRemoved();
					arrivalAtEndPlatform();
				}
			}
		}
		break;
	}
	}
}
std::string TrainMovement::frame_tick_output()
{
	TrainUpdateParams& params = parentDriver->getParams();
	const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
	if (!configParams.trainController.outputEnabled) {
		return std::string();
	}

	std::stringstream logout;
	logout << std::setprecision(8);
	logout << "(\"TrainDriver\"" << ","
			<< params.now.frame() << "," << parentDriver->getParent()->getId()
			<< ",{" << "\"xPos\":\"" << trainPathMover.getCurrentPosition().getX()
			<< "\",\"yPos\":\"" << trainPathMover.getCurrentPosition().getY()
			<< "\",\"length\":\"" << trainLengthMeter
			<< "\",\"passengerNum\":\"" << parentDriver->getPassengers().size()
			<< "\",\"next\":\"" << trainPlatformMover.getNextPlatform()->getPlatformNo()
			<< "\",\"lineID\":\"" << parentDriver->getTrainLine();
	logout << "\"})" << std::endl;
	return logout.str();
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
bool TrainMovement::isStationCase(double disToTrain, double disToPlatform, double& effectDis)
{
	bool res = false;
	effectDis = disToTrain;
	if (disToPlatform > 0.0 && disToTrain == 0.0) {
		effectDis = disToPlatform;
		res = true;
	} else if (disToTrain != 0.0 && disToPlatform != 0.0
			&& disToPlatform < disToTrain) {
		effectDis = disToPlatform;
		res = true;
	} else {
		const TrainDriver* next = parentDriver->getNextDriver();
		if (next) {
			if (disToPlatform > 0.0
					&& next->getNextPlatform() != parentDriver->getNextPlatform()) {
				effectDis = disToPlatform;
				res = true;
			} else if (disToTrain > 0.0 && disToTrain < disToPlatform
					&& next->getNextRequested() == TrainDriver::REQUESTED_WAITING_LEAVING
					&& next->getNextPlatform() == parentDriver->getNextPlatform()) {
				effectDis = disToTrain;
				res = true;
			}
		}
	}
	return res;
}
double TrainMovement::getDistanceToNextTrain(const TrainDriver* nextDriver) const
{
	double distanceToNextTrain = 0.0;
	if(nextDriver){
		if (nextDriver->getNextRequested() == TrainDriver::REQUESTED_TO_DEPOT) {
			parentDriver->setNextDriver(nullptr);
		} else {
			TrainMovement* nextMovement = dynamic_cast<TrainMovement*>(nextDriver->movementFacet);
			if (nextMovement) {
				double dis = trainPathMover.getDifferentDistance(nextMovement->getPathMover());
				if (dis > 0) {
					distanceToNextTrain = dis - safeDistance - trainLengthMeter;
				}
			}
		}
	}
	return distanceToNextTrain;
}

double TrainMovement::getDistanceToNextPlatform(const TrainDriver* nextDriver)
{
	return trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());

}

double TrainMovement::getRealSpeedLimit()
{
	TrainUpdateParams& params = parentDriver->getParams();
	double distanceToNextTrain = 0.0;
	double distanceToNextPlatform = 0.0;
	double distanceToNextObject = 0.0;
	double speedLimit = 0.0;
	double speedLimit2 = 0.0;
	distanceToNextPlatform = trainPathMover.getDistanceToNextPlatform(trainPlatformMover.getNextPlatform());
	const TrainDriver* nextDriver = parentDriver->getNextDriver();
	distanceToNextTrain = getDistanceToNextTrain(nextDriver);

	if(distanceToNextTrain==0.0||distanceToNextPlatform==0.0)
	{
		distanceToNextObject = std::max(distanceToNextTrain, distanceToNextPlatform);
	}
	else
	{
		distanceToNextObject = std::min(distanceToNextTrain,distanceToNextPlatform);
	}

	params.disToNextPlatform = distanceToNextPlatform;
	params.disToNextTrain = distanceToNextTrain;
	if (isStationCase(distanceToNextTrain,distanceToNextPlatform, distanceToNextObject)) {
		double decelerate = trainPathMover.getCurrentDecelerationRate();
		speedLimit = std::sqrt(2.0 * decelerate * distanceToNextObject);
		if (params.currentSpeed > speedLimit) {
			params.currCase = TrainUpdateParams::STATION_CASE;
			params.currentSpeedLimit = speedLimit;
			return speedLimit;
		}
	}

	params.currCase = TrainUpdateParams::NORMAL_CASE;
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
		double effectiveAccelerate = -(effectiveSpeed*effectiveSpeed)/(2.0*params.disToNextPlatform);
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
	double movingDist = 0.0;
	TrainUpdateParams& params = parentDriver->getParams();
	double effectiveAcc = getEffectiveAccelerate();
	if(params.currCase==TrainUpdateParams::STATION_CASE){
		double nextSpeed = params.currentSpeed+effectiveAcc*params.secondsInTick;
		nextSpeed = std::max(0.0, nextSpeed);
		movingDist = (nextSpeed*nextSpeed-params.currentSpeed*params.currentSpeed)/(2.0*effectiveAcc);
		params.currentSpeed = nextSpeed;
	} else {
		movingDist = params.currentSpeed*params.secondsInTick+0.5*effectiveAcc*params.secondsInTick*params.secondsInTick;
		params.currentSpeed = params.currentSpeed+effectiveAcc*params.secondsInTick;
	}
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
		facetMutex.lock();
		nextPlatform = next;
		facetMutex.unlock();
		std::string stationNo = next->getStationNo();
		Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
		messaging::MessageBus::PostMessage(stationAgent,TRAIN_MOVETO_NEXT_PLATFORM,
				messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver,true)));
	}
}
void TrainMovement::arrivalAtStartPlaform() const
{
	Platform* next = trainPlatformMover.getFirstPlatform();
	std::string stationNo = next->getStationNo();
	Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
	messaging::MessageBus::PostMessage(stationAgent, TRAIN_ARRIVAL_AT_STARTPOINT,
			messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver)));
}

void TrainMovement::arrivalAtEndPlatform() const
{
	std::vector<Platform*>::const_iterator it;
	const std::vector<Platform*>& prevs = trainPlatformMover.getPrevPlatforms();
	for (it = prevs.begin(); it != prevs.end(); it++) {
		Platform* prev = (*it);
		std::string stationNo = prev->getStationNo();
		Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
		messaging::MessageBus::PostMessage(stationAgent,TRAIN_ARRIVAL_AT_ENDPOINT,
				messaging::MessageBus::MessagePtr(new TrainDriverMessage(parentDriver)));
	}
}


}
} /* namespace sim_mob */
