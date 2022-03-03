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
namespace
{
    const double distanceMinimal = 0.001;
}
namespace sim_mob
{
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
    if(platforms.size()>0)
    {
        return platforms.front();
    }
    else
    {
        return nullptr;
    }
}

void TrainPlatformMover::clearPrevPlatforms()
{
    prevPlatforms.clear();
}

void TrainPlatformMover::resetPlatformItr()
{
    //resets the iterator to beginning position
    currPlatformIt = platforms.begin();
}
Platform* TrainPlatformMover::getNextPlatform(bool updated)
{
    //return the next platform on the route.
    //If updated value passed is true then the iterator is moved forward
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
Platform* TrainPlatformMover::getPlatformByOffset(int offset) const
{
    //This function returns the platform by offset .The offset can also be negative in which case it will
    //return the platform behind it
    if(offset < 0)
    {
        std::vector<Platform*>::iterator prev = currPlatformIt;
        int offsetneg = -offset;
        while(offsetneg > 0)
        {
            prev--;
            offsetneg--;
        }
        if(prev != platforms.end())
        {
            return (*prev);
        }
        return nullptr;
    }
    std::vector<Platform*>::iterator next = std::next(currPlatformIt, offset);
    if(next != platforms.end())
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
    //setting the iterator to the end of the list
    while(currPlatformIt != platforms.end())
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
    if((currPlatformIt+1) == platforms.end())
    {
        return true;
    } 
    else
    {
        return false;
    }
}
TrainPathMover::TrainPathMover():distanceMoveToNextPoint(0),currPolyLine(nullptr),distMovedOnCurrBlock(0),distMovedOnEntirePath(0)
{

}

TrainPathMover::~TrainPathMover()
{

}

void TrainPathMover::setParentMovementFacet(MovementFacet * movementFacet)
{
    parentMovementFacet = movementFacet;
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
    //this function gets the stop point at a particular distance from the current position of the train
    std::vector<Block*>::const_iterator curr = drivingPath.begin();
    double dis=0;
    while(curr != drivingPath.end())
    {
        const PolyLine *polyLine = (*curr)->getPolyLine();
        const std::vector<PolyPoint> pointvector = polyLine->getPoints();
        std::vector<PolyPoint>::const_iterator itr = pointvector.begin();

        while(itr != pointvector.end())
        {
            std::vector<PolyPoint>::const_iterator nextPolyPoint = itr + 1;
            if( nextPolyPoint == pointvector.end() )
            {
                //move on to next block
                if((curr+1) == drivingPath.end())
                {
                    break;
                }
                const PolyLine* nextPolyLine = (*(curr+1))->getPolyLine();
                nextPolyPoint = nextPolyLine->getPoints().begin();
                dis = dis + calcDistanceBetweenCurrentAndSubsequentPoint(*itr,(*nextPolyPoint));
            }

            else
            {
                // calculate the distance between current and subsequent stop point
                dis = dis + calcDistanceBetweenCurrentAndSubsequentPoint(*itr,(*nextPolyPoint));
            }
            if(dis > distance)
            {
                //if the distance ends up between two points then take the before point as the stop point
                return *itr;
            }
            else if(dis == distance)
            {
                return *nextPolyPoint;
            }
            itr++;
        }
        curr++;
    }
    throw std::runtime_error("Error. Unable to find stop point in TrainPathMover::GetStopPoint.");
}
double TrainPathMover::calcDistanceBetweenTwoPoints(std::vector<PolyPoint>::const_iterator& currPointItr,std::vector<PolyPoint>::const_iterator& laterPoint,const std::vector<PolyPoint> &points) const
{
    std::vector<Block*>::const_iterator curr = drivingPath.begin();
    double dis=0;
    bool startPointfound = false;
    bool foundLaterPoint = false;
    if(currPointItr == points.end() || laterPoint == points.end())
    {
        return dis;
    }
    while(curr != drivingPath.end())
    {
        const PolyLine *polyLine = (*curr)->getPolyLine();
        const std::vector<PolyPoint> pointvector = polyLine->getPoints();
        std::vector<PolyPoint>::const_iterator itr = pointvector.begin();
        while(itr != pointvector.end())
        {
            if((*currPointItr).getX() == (*itr).getX()&&(*currPointItr).getY() == (*itr).getY()&&(*currPointItr).getZ() == (*itr).getZ())
            {
                //if all x ,y ,z coordinates of the point are compared and equal then that is the start point
                startPointfound = true;
            }

            if((*laterPoint).getX() == (*itr).getX()&&(*laterPoint).getY() == (*itr).getY()&&(*laterPoint).getZ()==(*itr).getZ())
            {
                //same for the later point
                foundLaterPoint = true;
                break;
            }
            if(startPointfound == true)
            {
                std::vector<PolyPoint>::const_iterator nextPointItr = itr + 1;
                if(nextPointItr == pointvector.end())
                {
                    //if the end of current block and move on to next block
                    if((curr+1) == drivingPath.end())
                    {
                        break;
                    }
                    const PolyLine* nextPolyLine = (*(curr+1))->getPolyLine();
                    //pick up the first point from next block
                    std::vector<PolyPoint>::const_iterator nextPolyPointItr = nextPolyLine->getPoints().begin();
                    //calculating the distance between current and subsequent point
                    dis = dis + calcDistanceBetweenCurrentAndSubsequentPoint(*itr,(*nextPolyPointItr));
                }
                else
                {
                    dis = dis+calcDistanceBetweenCurrentAndSubsequentPoint(*itr,*nextPointItr);
                }
            }
            itr++;
        }
        if(foundLaterPoint == true)
        {
            break;
        }
        curr++;
    }
    if(foundLaterPoint == false)
    {
        return 0;
    }

    return dis;
}

std::vector<PolyPoint>::const_iterator TrainPathMover::GetCurrentStopPoint() const
{
    return currPolyPointIt;
}

int TrainPathMover::getCurrentBlockId() const
{
    return (*currBlockIt)->getBlockId();
}


MovementFacet *TrainPathMover::GetParentMovementFacet() const
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

    //Also add the previous saved distance to next point ,when its between 2 points and then calculate the distance required to move
    //so the total distance to move will be the ()distance required + the prev saved distance to next point ) from the current point 
    //as if the train is between 2 points then the before point is taken as current point   
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
        //calculates the distance between two points next to each other on straight line
        distBetwCurrAndNxtPt = calcDistanceBetweenTwoPoints();
    }

    return distanceMoveToNextPoint;
}
double TrainPathMover::getDistanceToNextPlatform(Platform* platform) const
{
    bool res = false;
    double distance = 0.0;
    if(platform == nullptr)
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
    if(!res || distance < distanceMinimal)
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
    else if(tempOtherIt == other.drivingPath.end())
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
    double speedLimit = 0.0;
    if(drivingPath.size() != 0&&currBlockIt != drivingPath.end())
    {
        speedLimit = (*currBlockIt)->getSpeedLimit();
    }
    return speedLimit;
}
double TrainPathMover::getCurrentDecelerationRate()
{
    double decelerate = 0.0;
    if(drivingPath.size() !=0 && currBlockIt != drivingPath.end()){
        decelerate = (*currBlockIt)->getDecelerateRate();
    }
    return decelerate;
}
double TrainPathMover::getCurrentAccelerationRate()
{
    double accelerate = 0.0;
    if(drivingPath.size() !=0 && currBlockIt != drivingPath.end())
    {
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

std::vector<PolyPoint>::const_iterator TrainPathMover::findNearestStopPoint(const std::vector<PolyPoint>& pointVector) const
{

    std::vector<PolyPoint>::const_iterator currPointItr = currPolyPointIt;
    std::vector<PolyPoint>::const_iterator itr = pointVector.begin();
    std::vector<PolyPoint>::const_iterator nearestPoint = pointVector.end();
    double minDis=-1;
    for(;itr != pointVector.end();itr++)
    {
        double dis = calcDistanceBetweenTwoPoints(currPointItr,itr,pointVector);

        if((minDis == -1 || dis < minDis) && dis != 0 )
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
        ret = true;
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

void TrainPathMover::teleportToOppositeLine(std::string station,std::string lineId,Platform *platform)
{
    std::vector<Block*>::const_iterator tempIt = currBlockIt;
    double distance = 0;
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
            distanceToBlock = distance;
            distance += platform->getOffset() + platform->getLength();
            break;
        }

    }

    distMovedOnEntirePath = distance;
    distanceMoveToNextPoint = distance-distanceToBlock;
    currPolyLine = (*currBlockIt)->getPolyLine();
    currPolyPointIt = currPolyLine->getPoints().begin();
    nextPolyPointIt = currPolyPointIt + 1;
    double distBetwCurrAndNxtPt=calcDistanceBetweenTwoPoints();
    while(distanceMoveToNextPoint >= distBetwCurrAndNxtPt)
    {
        distanceMoveToNextPoint -= distBetwCurrAndNxtPt;
        if( !advanceToNextPoint() )
        {
            break;
        }
        distBetwCurrAndNxtPt = calcDistanceBetweenTwoPoints();
    }
}

double TrainPathMover::getDistanceFromStartToPlatform(std::string lineId,Platform *platform) const
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
          if((*route.begin())->getAttachedPlatform() == (*(route.end()-1))->getAttachedPlatform() && (tempIt == route.begin()|| tempIt == route.end()-1)) //Loop route
			{
				if(getTotalCoveredDistance() <=  platform->getOffset() + platform->getLength() || tempIt == route.end()-1)
				{
					distance += platform->getOffset() + platform->getLength();
					break;
				}
				else
				{
					distance += (*tempIt)->getLength();
					tempIt++;
					continue;

				}
			}
			
		else
	 	{	
	    	distance += platform->getOffset() + platform->getLength();
            	break;
        	}	
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
    //teleports the train to a particular platform ahead of it from the start of the route
    //used in case of train U-turn
    std::vector<Block*>::const_iterator tempIt = currBlockIt;
    double distance = 0;
    double distanceToBlock;
    while (tempIt != drivingPath.end())
    {
        Platform *platform = (*tempIt)->getAttachedPlatform();
        if( !boost::iequals(platform->getPlatformNo(), platformName) )
        {
            //jump to next block and add the length of the block to distance travelled  
            distance += (*tempIt)->getLength();
            tempIt++;
            currBlockIt++;
        }

        else
        {
            //if it the block of the platform then ,add the platform offset distance in the block and the length of the platform to distance travelled
            distanceToBlock = distance;
            distance += platform->getOffset() + platform->getLength();
            break;
        }

    }

    distMovedOnEntirePath = distance;
    distanceMoveToNextPoint = distance - distanceToBlock;
    currPolyLine = (*currBlockIt)->getPolyLine();
    currPolyPointIt = currPolyLine->getPoints().begin();
    nextPolyPointIt = currPolyPointIt + 1;
    double distBetwCurrAndNxtPt = calcDistanceBetweenTwoPoints();
    while(distanceMoveToNextPoint >= distBetwCurrAndNxtPt)
    {
        //keep iterating the points in current block till  the respective distance to the platform is reached 
        //after crossing every point reduce the remaining distance 
        //if the distance to subsequent point is less than the distance remaining keep iterating
        distanceMoveToNextPoint -= distBetwCurrAndNxtPt;
        if(!advanceToNextPoint())
        {
            //when distance to next point is more than distance remaining.That tell its between the two points 
            //so the first one is taken as the relevant point.
            //and the remaining distance is saved for future use
            break;
        }
        distBetwCurrAndNxtPt = calcDistanceBetweenTwoPoints();
    }
}
} /* namespace sim_mob */
