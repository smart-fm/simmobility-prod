/*
 * TrainPathMover.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */

#include <entities/roles/driver/TrainPathMover.hpp>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include "util/GeomHelpers.hpp"
namespace{
const double distanceMinimal = 0.001;
}
namespace sim_mob {
TrainPlatformMover::TrainPlatformMover()
{

}



TrainPlatformMover::~TrainPlatformMover()
{

}
void TrainPlatformMover::setPlatforms(const std::vector<Platform*>& plats)
{
	platforms = plats;
	currPlatformIt = platforms.begin();
}
const std::vector<Platform*>& TrainPlatformMover::getPrevPlatforms() const
{
	return prevPlatforms;
}

Platform* TrainPlatformMover::getFirstPlatform() const
{
	if(platforms.size()>0){
		return platforms.front();
	} else {
		return nullptr;
	}
}

Platform* TrainPlatformMover::getNextPlatform(bool updated)
{
	if(updated){
		prevPlatforms.push_back(*currPlatformIt);
		currPlatformIt++;
	}
	if(currPlatformIt!=platforms.end()){
		return (*currPlatformIt);
	} else {
		return nullptr;
	}
}
bool TrainPlatformMover::isLastPlatform()
{
	if((currPlatformIt+1)==platforms.end()){
		return true;
	} else {
		return false;
	}
}
TrainPathMover::TrainPathMover():distanceMoveToNextPoint(0),currPolyLine(nullptr),distMovedOnCurrBlock(0),distMovedOnEntirePath(0) {

}


TrainPathMover::~TrainPathMover() {

}

double TrainPathMover::calcDistanceBetweenTwoPoints() const
{
	DynamicVector vector(*currPolyPointIt, *nextPolyPointIt);
	return vector.getMagnitude();
}

int TrainPathMover::GetCurrentBlockId()
{
  return (*currBlockIt)->getBlockId();
}

double TrainPathMover::advance(double distance)
{
	if(drivingPath.empty())
	{
		throw std::runtime_error("path is empty in the train");
	}
	if(isCompletePath())
	{
		throw std::runtime_error("path already completed in the train");
	}

	distanceMoveToNextPoint += distance;
	moverMutex.lock();
	double temp = distMovedOnEntirePath;
	distMovedOnEntirePath = (temp+distance);
	moverMutex.unlock();

	double distBetwCurrAndNxtPt = calcDistanceBetweenTwoPoints();

	//Check if we've crossed the next point, if so advance to the next point
	while(distanceMoveToNextPoint >= distBetwCurrAndNxtPt)
	{
		distanceMoveToNextPoint -= distBetwCurrAndNxtPt;
		if(!advanceToNextPoint()){
			break;
		}
		distBetwCurrAndNxtPt = calcDistanceBetweenTwoPoints();
	}

	return distanceMoveToNextPoint;
}
double TrainPathMover::getDistanceToNextPlatform(Platform* platform) const
{
	bool res = false;
	double distance = 0.0;
	if (currBlockIt != drivingPath.end()) {
		if ((*currBlockIt)->getAttachedPlatform() == platform) {
			distance = platform->getOffset() + platform->getLength()
					- getDistCoveredOnCurrBlock();
			res = true;
		}
		else
		{
			distance = (*currBlockIt)->getLength()
					- getDistCoveredOnCurrBlock();
			std::vector<Block*>::const_iterator tempIt = currBlockIt + 1;

			while (tempIt != drivingPath.end()) {
				if ((*tempIt)->getAttachedPlatform() != platform) {
					distance += (*tempIt)->getLength();
					tempIt++;
				} else {
					distance += platform->getOffset() + platform->getLength();
					res = true;
					break;
				}
			}
		}
	}
	if(!res || distance<distanceMinimal){
		distance = 0.0;
	}
	return distance;
}
double TrainPathMover::getDistanceToNextTrain(const TrainPathMover& other) const
{
	double distance = 0.0;
	std::vector<Block*>::const_iterator tempIt = currBlockIt;
	std::vector<Block*>::const_iterator tempOtherIt = other.currBlockIt;
	if ((*tempIt) == (*tempOtherIt)) {
		distance = other.getDistCoveredOnCurrBlock()-getDistCoveredOnCurrBlock();
	} else if(tempOtherIt==other.drivingPath.end()) {
		distance = 0.0;
	} else {
		distance = (*tempIt)->getLength() - getDistCoveredOnCurrBlock();
		tempIt++;
		while (tempIt!=drivingPath.end()&&(*tempIt) != (*tempOtherIt)) {
			distance += (*tempIt)->getLength();
			tempIt++;
		}
		distance += other.getDistCoveredOnCurrBlock();
	}
	return distance;
}
double TrainPathMover::getDifferentDistance(const TrainPathMover& other) const
{
	double otherCoveredDis = other.getTotalCoveredDistance();
	moverMutex.lock();
	double res = otherCoveredDis-distMovedOnEntirePath;
	moverMutex.unlock();
	return res;
}
double TrainPathMover::getCurrentSpeedLimit()
{
	double speedLimit =0.0;
	if(drivingPath.size()!=0&&currBlockIt!=drivingPath.end()){
		speedLimit = (*currBlockIt)->getSpeedLimit();
	}
	return speedLimit;
}
double TrainPathMover::getCurrentDecelerationRate()
{
	double decelerate = 0.0;
	if(drivingPath.size()!=0&&currBlockIt!=drivingPath.end()){
		decelerate = (*currBlockIt)->getDecelerateRate();
	}
	return decelerate;
}
double TrainPathMover::getCurrentAccelerationRate()
{
	double accelerate = 0.0;
	if(drivingPath.size()!=0&&currBlockIt!=drivingPath.end()){
		accelerate = (*currBlockIt)->getAccelerateRate();
	}
	return accelerate;
}
Point TrainPathMover::getCurrentPosition() const
{
	if(isDrivingPath())
	{
		if(!isCompletePath())
		{
			DynamicVector movementVector(*currPolyPointIt, *nextPolyPointIt);
			movementVector.scaleVectTo(distanceMoveToNextPoint).translateVect();
			return Point(movementVector.getX(), movementVector.getY());
		}
		else
		{
			return *currPolyPointIt;
		}
	}
	else
	{
		throw std::runtime_error("no path is set in train");
	}
}
double TrainPathMover::getDistCoveredOnCurrBlock() const
{
	return distanceMoveToNextPoint+distMovedOnCurrBlock;
}

bool TrainPathMover::advanceToNextPoint()
{
	bool ret = false;
	if(nextPolyPointIt != currPolyLine->getPoints().end())
	{
		distMovedOnCurrBlock += calcDistanceBetweenTwoPoints();
		++currPolyPointIt;
		++nextPolyPointIt;
		ret=true;
		if(nextPolyPointIt == currPolyLine->getPoints().end())
		{
			ret = advanceToNextBlock();
		}
	}
	else
	{
		ret = advanceToNextBlock();
	}

	return ret;
}

void TrainPathMover::TeleportToOppositeLine(std::string station,std::string lineId,Platform *platform)
{
	std::vector<Block*>::const_iterator tempIt = currBlockIt;
	double distance=0;
	double distanceToBlock;
	while (tempIt != drivingPath.end())
	{
		if ((*tempIt)->getAttachedPlatform() != platform)
		{
			distance += (*tempIt)->getLength();
			tempIt++;
		}
		else
		{
			distanceToBlock=distance;
			distance += platform->getOffset() + platform->getLength();
			break;
		}

	}

	distanceMoveToNextPoint=distance-distanceToBlock;
	currPolyLine = (*currBlockIt)->getPolyLine();
	currPolyPointIt = currPolyLine->getPoints().begin();
	nextPolyPointIt = currPolyPointIt + 1;
	double distBetwCurrAndNxtPt=calcDistanceBetweenTwoPoints();
	while(distanceMoveToNextPoint >= distBetwCurrAndNxtPt)
	{
		distanceMoveToNextPoint -= distBetwCurrAndNxtPt;
		if(!advanceToNextPoint())
		{
			break;
		}
		distBetwCurrAndNxtPt = calcDistanceBetweenTwoPoints();
	}

}

double TrainPathMover::GetDistanceFromStartToPlatform(std::string lineId,Platform *platform)
{
	std::vector<Block*>::const_iterator tempIt = currBlockIt;
		double distance=0;
		double distanceToBlock;
		while (tempIt != drivingPath.end())
		{
			if ((*tempIt)->getAttachedPlatform() != platform)
			{
				distance += (*tempIt)->getLength();
				tempIt++;
			}
			else
			{
				distanceToBlock=distance;
				distance += platform->getOffset() + platform->getLength();
				break;
			}

		}
 return distance;
}



bool TrainPathMover::advanceToNextBlock()
{
	bool ret = false;
	currBlockIt++;
	distMovedOnCurrBlock = 0;
	if(currBlockIt != drivingPath.end())
	{
		currPolyLine = (*currBlockIt)->getPolyLine();
		currPolyPointIt = currPolyLine->getPoints().begin();
		nextPolyPointIt = currPolyPointIt + 1;
		ret = true;
	}
	else
	{
		return false;
	}

	return ret;
}
double TrainPathMover::getTotalCoveredDistance() const
{
	moverMutex.lock();
	double res = distMovedOnEntirePath;
	moverMutex.unlock();
	return res;
}

bool TrainPathMover::isCompletePath() const
{
	return (currBlockIt == drivingPath.end());
}
bool TrainPathMover::isDrivingPath() const
{
	return !drivingPath.empty();
}

void TrainPathMover::setPath(const std::vector<Block*> &path)
{
	drivingPath.clear();
	if(!path.empty())
	{
		drivingPath = path;
		currBlockIt = drivingPath.begin();
		currPolyLine = (*currBlockIt)->getPolyLine();
		currPolyPointIt = currPolyLine->getPoints().begin();
		nextPolyPointIt = currPolyPointIt + 1;
		distanceMoveToNextPoint = 0;
		distMovedOnCurrBlock = 0;
		distMovedOnEntirePath = 0;
	}
}

} /* namespace sim_mob */
