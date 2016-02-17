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

TrainPathMover::TrainPathMover() {

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
bool TrainPathMover::advanceToNextPoint()
{
	bool ret = false;
	if(nextPolyPointIt != currPolyLine->getPoints().end())
	{
		//Advance the iterators to the points
		++currPolyPointIt;
		++nextPolyPointIt;

		if(nextPolyPointIt == currPolyLine->getPoints().end())
		{
			ret = advanceToNextPolyLine();
		}
	}
	else
	{
		ret = advanceToNextPolyLine();
	}

	return ret;
}

bool TrainPathMover::advanceToNextPolyLine()
{
	bool ret = false;

	currBlockIt++;
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
	}
}

} /* namespace sim_mob */
