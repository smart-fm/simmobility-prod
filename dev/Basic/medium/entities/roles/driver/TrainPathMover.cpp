/*
 * TrainPathMover.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */

#include <algorithm>
#include <limits>
#include <stdexcept>
#include "util/GeomHelpers.hpp"
#include "entities/roles/driver/TrainDriverFacets.hpp"
#include <entities/roles/driver/TrainPathMover.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string.hpp>
#include "TrainDriver.hpp"
#include "entities/TrainController.hpp"
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

const std::vector<Platform*>& TrainPlatformMover::getPlatforms() const
{
	return platforms;
}

Platform* TrainPlatformMover::getFirstPlatform() const
{
	if(platforms.size()>0){
		return platforms.front();
	} else {
		return nullptr;
	}
}

void TrainPlatformMover::clearPrevPlatforms()
{
	prevPlatforms.clear();
}

void TrainPlatformMover::resetPlatformItr()
{
	currPlatformIt = platforms.begin();
}
Platform* TrainPlatformMover::getNextPlatform(bool updated)
{
	if(updated)
	{
		prevPlatforms.push_back(*currPlatformIt);
		currPlatformIt++;
	}
	if(currPlatformIt!=platforms.end())
	{
		return (*currPlatformIt);
	}
	else
	{
		return nullptr;
	}
}
Platform* TrainPlatformMover::getPlatformByOffset(unsigned int offset) const
{
	std::vector<Platform*>::iterator next = std::next(currPlatformIt, offset);
	if(next!=platforms.end())
	{
		return (*next);
	}
	else
	{
		return nullptr;
	}
}

void TrainPlatformMover::setPlatformIteratorToEnd()
{
	while(currPlatformIt!=platforms.end())
	{
		prevPlatforms.push_back(*currPlatformIt);
		currPlatformIt++;
	}
}

Platform *TrainPlatformMover::getLastPlatformOnRoute() const
{
	return *(platforms.end() - 1);
}
bool TrainPlatformMover::isLastPlatform()
{
	if((currPlatformIt+1)==platforms.end()){
		return true;
	} else {
		return false;
	}
}
TrainPathMover::TrainPathMover():distanceMoveToNextPoint(0),currPolyLine(nullptr),distMovedOnCurrBlock(0),distMovedOnEntirePath(0)
{

}


TrainPathMover::~TrainPathMover() {

}

void TrainPathMover::SetParentMovementFacet(MovementFacet * movementFacet)
{
	parentMovementFacet=movementFacet;
}

double TrainPathMover::calcDistanceBetweenTwoPoints() const
{
	DynamicVector vector(*currPolyPointIt, *nextPolyPointIt);
	return vector.getMagnitude();
}

double TrainPathMover::calcDistanceBetweenCurrentAndSubsequentPoint(Point a,Point b) const
{
	DynamicVector vector(a, b);
	return vector.getMagnitude();
}

PolyPoint TrainPathMover::GetStopPoint(double distance) const
{
	std::vector<Block*>::const_iterator curr=drivingPath.begin();
	double dis=0;
	while(curr!=drivingPath.end())
	{
	      const PolyLine *polyLine=(*curr)->getPolyLine();
	      const std::vector<PolyPoint> pointvector=polyLine->getPoints();
	      std::vector<PolyPoint>::const_iterator itr=pointvector.begin();

	      while(itr!=pointvector.end())
	      {
	    	  std::vector<PolyPoint>::const_iterator nextPolyPoint=itr+1;
	    	  if(nextPolyPoint == pointvector.end())
	    	  {
	    		  const PolyLine* nextPolyLine = (*(curr+1))->getPolyLine();
	    		  std::vector<PolyPoint>::const_iterator nextPolyPointItr = nextPolyLine->getPoints().begin();
	    		  dis=dis+calcDistanceBetweenCurrentAndSubsequentPoint(*itr,(*nextPolyPointItr));
	    	  }

	    	  else
	    	   dis=dis+calcDistanceBetweenCurrentAndSubsequentPoint(*itr,(*nextPolyPoint));

	    	  if(dis>distance)
	    		  return *itr;

	    	  itr++;

	      }

	      curr++;
	}
}
double TrainPathMover::calcDistanceBetweenTwoPoints(std::vector<PolyPoint>::const_iterator& currPointItr,std::vector<PolyPoint>::iterator& laterPoint,std::vector<PolyPoint> &points)
{

	std::vector<Block*>::const_iterator curr=currBlockIt;
	double dis=0;
	bool startPointfound=false;
	bool foundLaterPoint=false;
	if(currPointItr==points.end()||laterPoint==points.end())
		return dis;
	while(curr!=drivingPath.end())
	{
      const PolyLine *polyLine=(*curr)->getPolyLine();
      const std::vector<PolyPoint> pointvector=polyLine->getPoints();
      std::vector<PolyPoint>::const_iterator itr=pointvector.begin();
      while(itr!=pointvector.end())
      {
    	  if((*currPointItr).getX()==(*itr).getX()&&(*currPointItr).getY()==(*itr).getY()&&(*currPointItr).getZ()==(*itr).getZ())
    	  {
    		  startPointfound=true;
    	  }

    	  if((*laterPoint).getX()==(*itr).getX()&&(*laterPoint).getY()==(*itr).getY()&&(*laterPoint).getZ()==(*itr).getZ())
    	  {
    		  foundLaterPoint=true;
    		  break;
    	  }
    	  if(startPointfound==true)
    	  {
			  std::vector<PolyPoint>::const_iterator nextPointItr = itr+1;
			  if(nextPointItr == pointvector.end())
			  {
				  if((curr+1)==drivingPath.end())
				  {
					  break;
				  }
				  const PolyLine* nextPolyLine = (*(curr+1))->getPolyLine();
				  std::vector<PolyPoint>::const_iterator nextPolyPointItr = nextPolyLine->getPoints().begin();
				  dis=dis+calcDistanceBetweenCurrentAndSubsequentPoint(*itr,(*nextPolyPointItr));
			  }
			  else
			  {
			      dis=dis+calcDistanceBetweenCurrentAndSubsequentPoint(*itr,*nextPointItr);
			  }
    	  }
    	  itr++;
      }
      if(foundLaterPoint==true)
    	  break;
      curr++;

	}

	return dis;
}

std::vector<PolyPoint>::const_iterator TrainPathMover::GetCurrentStopPoint() const
{
	return currPolyPointIt;
}

int TrainPathMover::GetCurrentBlockId()
{
  return (*currBlockIt)->getBlockId();
}


MovementFacet *TrainPathMover::GetParentMovementFacet()
{
	return parentMovementFacet;
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
		if(!advanceToNextPoint())
		{
			break;
		}

		/*PolyPoint currPoint=(*currPolyPointIt);
        MovementFacet *movementFacet=GetParentMovementFacet();
        if(movementFacet)
        {
           medium::TrainMovement *trainMovFacet=dynamic_cast<medium::TrainMovement*>((movementFacet));
           medium::TrainDriver *driver=trainMovFacet->getParentDriver();
           std::vector<medium::StopPointEntity> stopPointEntities=driver->GetStopPoints();
           std::vector<medium::StopPointEntity>::iterator it;
           for(it=stopPointEntities.begin();it!=stopPointEntities.end();it++)
           {
        	  PolyPoint point = (*it).point;
        	  double duration = (*it).duration;
        	  int polyLineId=currPoint.getPolyLineId();
        	  int seqNo=currPoint.getSequenceNumber();
        	  if(polyLineId==point.getPolyLineId()&&seqNo==point.getSequenceNumber())
        	  {

        		  distMovedOnEntirePath=distMovedOnEntirePath-distanceMoveToNextPoint;
        		  distanceMoveToNextPoint=0;
        		  TrainUpdateParams& params=driver->getParams();
        		  params.currentSpeed=0;
        		  driver->SetStoppingParameters(point,duration);
        		  break;
        	  }
           }
        }*/


		distBetwCurrAndNxtPt = calcDistanceBetweenTwoPoints();
	}

	return distanceMoveToNextPoint;
}
double TrainPathMover::getDistanceToNextPlatform(Platform* platform) const
{
	bool res = false;
	double distance = 0.0;
	if(platform==nullptr)
	{
		return -1;
	}
	if (currBlockIt != drivingPath.end())
	{
		if ((*currBlockIt)->getAttachedPlatform() == platform)
		{
			distance = platform->getOffset() + platform->getLength()
					- getDistCoveredOnCurrBlock();
			res = true;
		}
		else
		{
			distance = (*currBlockIt)->getLength()
					- getDistCoveredOnCurrBlock();
			std::vector<Block*>::const_iterator tempIt = currBlockIt + 1;

			while (tempIt != drivingPath.end())
			{
				if ((*tempIt)->getAttachedPlatform() != platform)
				{
					distance += (*tempIt)->getLength();
					tempIt++;
				}
				else
				{
					distance += platform->getOffset() + platform->getLength();
					res = true;
					break;
				}
			}
		}
	}
	if(!res || distance<distanceMinimal)
	{
		distance = 0.0;
	}
	return distance;
}
double TrainPathMover::getDistanceToNextTrain(const TrainPathMover& other) const
{
	double distance = 0.0;
	std::vector<Block*>::const_iterator tempIt = currBlockIt;
	std::vector<Block*>::const_iterator tempOtherIt = other.currBlockIt;
	if ((*tempIt) == (*tempOtherIt))
	{
		distance = other.getDistCoveredOnCurrBlock()-getDistCoveredOnCurrBlock();
	}
	else if(tempOtherIt==other.drivingPath.end())
	{
		distance = 0.0;
	}
	else
	{
		distance = (*tempIt)->getLength() - getDistCoveredOnCurrBlock();
		tempIt++;
		while (tempIt!=drivingPath.end()&&(*tempIt) != (*tempOtherIt))
		{
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

double TrainPathMover::getDistanceMoveToNextPoint()
{
	return distanceMoveToNextPoint;
}

std::vector<PolyPoint>::iterator TrainPathMover::findNearestStopPoint(std::vector<PolyPoint>& pointVector)
{

	std::vector<PolyPoint>::const_iterator currPointItr=currPolyPointIt;
	std::vector<PolyPoint>::iterator itr=pointVector.begin();
	std::vector<PolyPoint>::iterator nearestPoint=pointVector.end();
	double minDis=-1;
	for(;itr!=pointVector.end();itr++)
	{
		double dis=calcDistanceBetweenTwoPoints(currPointItr,itr,pointVector);

		if((minDis==-1||dis<minDis)&&dis!=0)
		{
			nearestPoint=itr;
			minDis=dis;
		}
	}
	return nearestPoint;
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
			currBlockIt++;
		}
		else
		{
			distanceToBlock=distance;
			distance += platform->getOffset() + platform->getLength();
			break;
		}

	}

	distMovedOnEntirePath = distance;
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
	std::vector<Block*> route;
	TrainController<sim_mob::medium::Person_MT>::getInstance()->getTrainRoute(lineId,route);
	std::vector<Block*>::const_iterator tempIt = route.begin();
		double distance=0;
		double distanceToBlock;
		while (tempIt != route.end())
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

void TrainPathMover::teleportToPlatform(std::string platformName)
{
	std::vector<Block*>::const_iterator tempIt = currBlockIt;
	double distance=0;
	double distanceToBlock;
	while (tempIt != drivingPath.end())
	{
		Platform *platform =(*tempIt)->getAttachedPlatform();
		if (!boost::iequals(platform->getPlatformNo(), platformName))
		{
			distance += (*tempIt)->getLength();
			tempIt++;
			currBlockIt++;
		}

		else
		{
			distanceToBlock=distance;
			distance += platform->getOffset() + platform->getLength();
			break;
		}

	}

	distMovedOnEntirePath = distance;
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

} /* namespace sim_mob */
