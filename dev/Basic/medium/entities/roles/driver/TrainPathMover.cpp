/*
 * TrainPathMover.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: fm-simmobility
 */

#include <entities/roles/driver/TrainPathMover.hpp>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include "util/GeomHelpers.hpp"

namespace sim_mob {

TrainPathMover::TrainPathMover():distanceMoveToNextPoint(0),currPolyLine(nullptr),distMovedOnCurrBlock(0) {

}

TrainPathMover::~TrainPathMover() {

}

double TrainPathMover::calcDistanceBetweenTwoPoints() const
{
	DynamicVector vector(*currPolyPointIt, *nextPolyPointIt);
	return vector.getMagnitude();
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
	double distance = (*currBlockIt)->getLength()-getDistCoveredOnCurrBlock();
	std::vector<Block*>::const_iterator tempIt = currBlockIt+1;
	while(tempIt!=drivingPath.end()){
		if((*tempIt)->getAttachedPlatform()!=platform){
			distance += (*tempIt)->getLength();
		} else {
			distance += (platform->getOffset()+platform->getLength());
			break;
		}
	}
	return distance;
}
double TrainPathMover::getDistanceToNextTrain(const TrainPathMover& other) const
{
	std::vector<Block*>::const_iterator tempIt = currBlockIt;
	std::vector<Block*>::const_iterator tempOtherIt = other.currBlockIt;
	if(std::distance(tempIt, tempOtherIt)<0){
		return -1.0;
	}
	double distance = (*currBlockIt)->getLength()-getDistCoveredOnCurrBlock();
	while(tempIt!=tempOtherIt){
		distance += (*tempIt)->getLength();
		tempIt++;
	}
	distance += other.getDistCoveredOnCurrBlock();
	return distance;
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

	return ret;
}

bool TrainPathMover::isCompletePath() const
{
	return (currBlockIt == drivingPath.end());
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
	}
}

} /* namespace sim_mob */
