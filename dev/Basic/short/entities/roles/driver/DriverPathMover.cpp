//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverPathMover.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <bits/stl_map.h>

#include "conf/CMakeConfigParams.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/settings/DisableMPI.h"
#include "exceptions/Exceptions.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "logging/Log.hpp"
#include "util/GeomHelpers.hpp"
#include "util/Utils.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"
#endif

using namespace sim_mob;

//Error message strings
const std::string DriverPathMover::ErrorDrivingPathNotSet("The driving path is not set.");
const std::string DriverPathMover::ErrorNotInIntersection("the driver is not in an intersection!");
const std::string DriverPathMover::ErrorEntireRouteDone("Entire route is done!");

DriverPathMover::DriverPathMover() :
currLane(NULL), currTurning(NULL), nextTurning(NULL), currPolyLine(NULL), inIntersection(false), distCoveredFromCurrPtToNextPt(0.0), distCoveredOnCurrWayPt(0.0)
{
}

DriverPathMover::DriverPathMover(const DriverPathMover &pathMover) :
currLane(pathMover.currLane), currTurning(pathMover.currTurning), currPolyLine(pathMover.currPolyLine), inIntersection(pathMover.inIntersection),
distCoveredFromCurrPtToNextPt(pathMover.distCoveredFromCurrPtToNextPt), distCoveredOnCurrWayPt(pathMover.distCoveredOnCurrWayPt)
{
	//Align the iterators
	currWayPointIt = drivingPath.begin() + (pathMover.currWayPointIt - pathMover.drivingPath.begin());
	
	const std::vector<PolyPoint> &polyPoints = currPolyLine->getPoints();
	currPolyPoint = polyPoints.begin() + (pathMover.currPolyPoint - pathMover.currPolyLine->getPoints().begin());
	nextPolyPoint = polyPoints.begin() + (pathMover.nextPolyPoint - pathMover.currPolyLine->getPoints().begin());
}

const Lane* DriverPathMover::getCurrLane() const
{
	return currLane;
}

const TurningPath* DriverPathMover::getCurrTurning() const
{
	return currTurning;
}

const std::vector<WayPoint>& DriverPathMover::getDrivingPath() const
{
	return drivingPath;
}

std::vector<WayPoint>::const_iterator DriverPathMover::getCurrWayPointIt() const
{
	return currWayPointIt;
}

bool DriverPathMover::isInIntersection() const
{
	return inIntersection;
}

const PolyPoint& DriverPathMover::getCurrPolyPoint() const
{
	if (!isDoneWithEntireRoute())
	{
		return *currPolyPoint;
	}
	else
	{
		throw std::runtime_error(ErrorEntireRouteDone);
	}
}

const PolyPoint& DriverPathMover::getNextPolyPoint() const
{
	if (nextPolyPoint != currPolyLine->getPoints().end())
	{
		return *nextPolyPoint;
	}
	else
	{
		throw std::runtime_error(ErrorEntireRouteDone);
	}
}

const WayPoint& DriverPathMover::getCurrWayPoint() const
{
	if (isDrivingPathSet())
	{
		if (!isDoneWithEntireRoute())
		{
			return *currWayPointIt;
		}
		else
		{
			throw std::runtime_error(ErrorEntireRouteDone);
		}
	}
	else
	{
		throw std::runtime_error(ErrorDrivingPathNotSet);
	}
}

const WayPoint* DriverPathMover::getNextWayPoint() const
{
	if (isDrivingPathSet())
	{
		if (!isDoneWithEntireRoute())
		{
			if((currWayPointIt + 1) != drivingPath.end())
			{
				return &(*(currWayPointIt + 1));
			}
			else
			{
				return NULL;
			}
		}
		else
		{
			throw std::runtime_error(ErrorEntireRouteDone);
		}
	}
	else
	{
		throw std::runtime_error(ErrorDrivingPathNotSet);
	}
}

const RoadSegment* DriverPathMover::getCurrSegment() const
{
	if (currLane)
	{
		return currLane->getParentSegment();
	}
	else
	{
		return NULL;
	}
}

const Link* DriverPathMover::getCurrLink() const
{
	if (currLane)
	{
		return currLane->getParentSegment()->getParentLink();
	}
	else
	{
		return NULL;
	}
}

const RoadSegment* DriverPathMover::getNextSegment() const
{
	const RoadSegment *nextSeg = NULL;
	
	//Access the next way-point
	std::vector<WayPoint>::const_iterator itWayPt = currWayPointIt + 1;
	
	if(itWayPt != drivingPath.end())
	{
		if(itWayPt->type == WayPoint::ROAD_SEGMENT)
		{
			nextSeg = itWayPt->roadSegment;
		}
	}
	
	return nextSeg;
}

const RoadSegment* DriverPathMover::getNextSegInNextLink() const
{
	const RoadSegment *nextSeg = NULL;
	const Link *nextLink = getNextLink();
	
	if(nextLink)
	{
		nextSeg = nextLink->getRoadSegment(0);
	}
	
	return nextSeg;
}


const Link* DriverPathMover::getNextLink() const
{
	const Link *nextLink = NULL;
	
	//Access the next way-point
	std::vector<WayPoint>::const_iterator itWayPt = currWayPointIt + 1;
	
	while(itWayPt != drivingPath.end())
	{
		if(itWayPt->type == WayPoint::ROAD_SEGMENT && itWayPt->roadSegment->getParentLink() != getCurrLink())
		{
			nextLink = itWayPt->roadSegment->getParentLink();
			break;
		}
		++itWayPt;
	}
	
	return nextLink;
}

const Lane* DriverPathMover::getNextLane() const
{
	const Lane *nextLane = NULL;
	
	if(currLane)
	{
		//Use the lane connectors to find the next lane
		
		//Get all the connectors that are physically connected to the next segment
		std::vector<const LaneConnector *> trueConnections;
		currLane->getPhysicalConnectors(trueConnections);
		
		if(!trueConnections.empty())
		{
			//Choose the middle connection
			unsigned int midConnection = trueConnections.size() / 2;
			nextLane = trueConnections.at(midConnection)->getToLane();
		}
	}
	else
	{
		nextLane = currTurning->getToLane();
	}
	
	return nextLane;
}

const TurningPath* DriverPathMover::getNextTurning()
{
	if(nextTurning == nullptr)
	{
		if (currLane && currWayPointIt != drivingPath.end() && (currWayPointIt + 1) != drivingPath.end())
		{
			//Get next way point. It should be a turning group
			const WayPoint nextWayPt = *(currWayPointIt + 1);
			if (nextWayPt.type == WayPoint::TURNING_GROUP)
			{
				//The turnings from the current lane
				const std::map<unsigned int, TurningPath *> *turnings = nextWayPt.turningGroup->getTurningPaths(currLane->getLaneId());

				if (turnings)
				{
					if(turnings->size() > 1)
					{
						//Select a random turning path
						unsigned int randomInt = Utils::generateInt(0, turnings->size() - 1);
						std::map<unsigned int, TurningPath *>::const_iterator it = turnings->begin();
						std::advance(it, randomInt);
						nextTurning = it->second;
					}
					else
					{
						//Only one turning path available
						nextTurning = turnings->begin()->second;
					}
				}
			}
		}
	}
	
	return nextTurning;
}

void DriverPathMover::setPath(const std::vector<WayPoint> &path, int startLaneIndex, int startSegmentId)
{
	//Clear the previous path, if any
	drivingPath.clear();
	currTurning = NULL;
	inIntersection = false;
	
	if(!path.empty())
	{
		//Copy the way-points from the given path
		drivingPath = path;
		
		//Check if the start segment is given, if not start at the first segment
		if(startSegmentId != 0)
		{
			std::vector<WayPoint>::const_iterator it = drivingPath.begin();
			
			//Look for the given start segment in the path
			while(it != drivingPath.end())
			{
				if(it->type == WayPoint::ROAD_SEGMENT)
				{
					if (startSegmentId == it->roadSegment->getRoadSegmentId())
					{
						currWayPointIt = it;
						break;
					}
				}
				
				++it;
			}
			
			//If the requested segment was not in the driving path, start from the first segment
			if(it == drivingPath.end())
			{
				currWayPointIt = drivingPath.begin();
			}
		}
		else
		{
			//Set the current way-point iterator to point to the first segment in the path
			currWayPointIt = drivingPath.begin();
		}
		
		//The current lane index
		unsigned int currLaneIndex = 0;
		unsigned int noOfLanes = currWayPointIt->roadSegment->getNoOfLanes();
		
		//Validate the given start lane index
		if(startLaneIndex < 0 || startLaneIndex >= noOfLanes)
		{
			//Invalid index, default to left most lane
			//currLaneIndex = 0;
			
			//Randomly select a lane
			currLaneIndex = Utils::generateInt(0, noOfLanes-1);
		}
		else
		{
			currLaneIndex = startLaneIndex;
		}
		
		//Set the current lane
		currLane = currWayPointIt->roadSegment->getLane(currLaneIndex);
		
		//Ensure that this is not a pedestrian lane
		while(currLane->isPedestrianLane())
		{
			Print() << "Starting lane " << currLane->getLaneId() << " (index = " << currLaneIndex << ")is a pedestrian lane";
			Print() << "Selecting next lane...";
			
			//Try the next lane
			++currLaneIndex;
			currLane = currWayPointIt->roadSegment->getLane(currLaneIndex);
		}
		
		//Set the starting point at a random distance within the start segment
		double laneLength = currLane->getLength();
		
		//Divide the lane into parts of 100m and select a random entry section
		int lastEntryIndex = laneLength / 100;
		int selectedIndex = Utils::generateInt(0, lastEntryIndex);		
		
		//Set the current poly-line and the set the iterators to point to the current and next points
		currPolyLine = currLane->getPolyLine();
		currPolyPoint = currPolyLine->getPoints().begin();
		nextPolyPoint = currPolyPoint + 1;
		
		//Initialise the distances covered
		distCoveredFromCurrPtToNextPt = 0;
		distCoveredOnCurrWayPt = 0;
		
		//Distance at which the the driver will start
		double distance = selectedIndex * 100;
		
		//If the starting distance is 0, no need to bother
		if(distance != 0)
		{
			//Adjust the distances covered according to the driver's starting distance
			while (distance != 0)
			{
				double distBetwCurrAndNxtPt = calcDistFromCurrToNextPt();
				if (distBetwCurrAndNxtPt <= distance)
				{
					distCoveredOnCurrWayPt += distBetwCurrAndNxtPt;
					currPolyPoint = nextPolyPoint;
					nextPolyPoint += 1;
					distance -= distBetwCurrAndNxtPt;
				}
				else
				{
					distCoveredFromCurrPtToNextPt = distance;
					distance = 0;
				}
			}
		}
	}
}

void DriverPathMover::setPathStartingWithTurningGroup(const std::vector<WayPoint> &path, const Lane *fromLane)
{
	//Clear the previous path, if any
	drivingPath.clear();
	currLane = nullptr;
	inIntersection = true;

	if (!path.empty())
	{
		//Copy the way-points from the given path
		drivingPath = path;

		//Set the current way-point iterator to point to the first segment in the path
		currWayPointIt = drivingPath.begin();
		
		//Select the turning path based on the previous lane and next lane
		const TurningGroup *tGroup = path.front().turningGroup;
		const std::map<unsigned int, TurningPath *> *turnings = tGroup->getTurningPaths(fromLane->getLaneId());
		
		if (turnings)
		{
			if (turnings->size() > 1)
			{
				//Select a random turning path
				unsigned int randomInt = Utils::generateInt(0, turnings->size() - 1);
				std::map<unsigned int, TurningPath *>::const_iterator it = turnings->begin();
				std::advance(it, randomInt);
				currTurning = it->second;
			}
			else
			{
				//Only one turning path available
				currTurning = turnings->begin()->second;
			}
		}
		else
		{
			std::stringstream msg;
			msg << "No turning path from Lane " << fromLane->getLaneId() << " in the turning group " << tGroup->getTurningGroupId();
			throw runtime_error(msg.str());
		}

		//Set the current poly-line and the set the iterators to point to the current and next points
		currPolyLine = currTurning->getPolyLine();
		currPolyPoint = currPolyLine->getPoints().begin();
		nextPolyPoint = currPolyPoint + 1;

		//Initialise the distances covered
		distCoveredFromCurrPtToNextPt = 0;
		distCoveredOnCurrWayPt = 0;		
	}
}

bool DriverPathMover::isDrivingPathSet() const
{
	return (!drivingPath.empty());
}

bool DriverPathMover::isDoneWithEntireRoute() const
{
	return (currWayPointIt == drivingPath.end());
}

double DriverPathMover::advance(double distance)
{
	if(!isDrivingPathSet())
	{
		throw std::runtime_error(ErrorDrivingPathNotSet);
	}
	
	if(isDoneWithEntireRoute())
	{
		throw std::runtime_error(ErrorEntireRouteDone);
	}
	
	//The distance by which we've overflowed into the intersection
	double overflowAmount = 0;
	
	//Increment the distance covered towards the next poly-point by the given amount
	distCoveredFromCurrPtToNextPt += distance;
	
	double distBetwCurrAndNxtPt = calcDistFromCurrToNextPt();
	
	//Check if we've crossed the next point, if so advance to the next point
	while(distCoveredFromCurrPtToNextPt >= distBetwCurrAndNxtPt)
	{
		distCoveredFromCurrPtToNextPt -= distBetwCurrAndNxtPt;
		overflowAmount = advanceToNextPoint();
		distBetwCurrAndNxtPt = calcDistFromCurrToNextPt();
	}
	
	return overflowAmount;
}

double DriverPathMover::calcDistFromCurrToNextPt()
{
	DynamicVector vector(*currPolyPoint, *nextPolyPoint);
	return vector.getMagnitude();
}

double DriverPathMover::advanceToNextPoint()
{
	double overflow = 0;
	
	//If we're at the end of the poly-line, move to the next poly-line. If not we update the iterators to the current and the next points
	if(nextPolyPoint != currPolyLine->getPoints().end())
	{
		//Update the distance covered in the current segment
		distCoveredOnCurrWayPt += calcDistFromCurrToNextPt();

		//Advance the iterators to the points
		++currPolyPoint;
		++nextPolyPoint;
		
		if(nextPolyPoint == currPolyLine->getPoints().end())
		{
			overflow = advanceToNextPolyLine();
		}
	}
	else
	{
		overflow = advanceToNextPolyLine();
	}
	
	return overflow;
}

double DriverPathMover::advanceToNextPolyLine()
{
	double overflow = 0;
	
	if(currWayPointIt != drivingPath.end())
	{
		if((currWayPointIt + 1) != drivingPath.end())
		{
			//Check if we are in a segment or a turning group
			if(currWayPointIt->type == WayPoint::ROAD_SEGMENT)
			{
				//Check if we're heading into a segment or a turning group
				if((currWayPointIt + 1)->type == WayPoint::ROAD_SEGMENT)
				{
					inIntersection = false;
					
					//Get the next lane					
					currLane = getNextLane();
					
					if(currLane)
					{
						currPolyLine = currLane->getPolyLine();
					}
					else
					{
						std::stringstream msg;
						msg << "Reached end of segment from lane " << currLane->getLaneId() 
							<< ". It is not physically connected to any lane in the next segment ";
						throw std::runtime_error(msg.str());
					}
				}
				else
				{
					inIntersection = true;
					overflow = distCoveredFromCurrPtToNextPt;
					
					//We're entering an intersection. Use the turning group and the current lane to get the turning path			
					currTurning = getNextTurning();
					nextTurning = nullptr;
					
					if(currTurning)
					{
						currPolyLine = currTurning->getPolyLine();
						currLane = NULL;
					}
					else
					{
						std::stringstream msg;
						msg << "Reached intersection from lane " << currLane->getLaneId() << ". It is not connected to a turning path!";
						throw no_turning_path_exception(msg.str(), currLane, getNextSegInNextLink());
					}
				}
			}
			else
			{
				//Since we are in a turning group, we will be heading into a segment				
				inIntersection = false;
				
				//Use the current turning to get the next lane
				currLane = getNextLane();
				currPolyLine = currLane->getPolyLine();
				currTurning = NULL;
			}
			
			//Set the iterators to point to the current and next points
			currPolyPoint = currPolyLine->getPoints().begin();
			nextPolyPoint = currPolyPoint + 1;
		}
		
		//Advance the iterator to point to the next way-point
		++currWayPointIt;
		
		//Reset the distance covered on the current way-point
		distCoveredOnCurrWayPt = 0;
	}
	else
	{
		distCoveredFromCurrPtToNextPt = 0;
		overflow = 0;
	}
	
	return overflow;
}

void DriverPathMover::updateLateralMovement(const Lane* lane)
{
	if(lane)
	{
		//Points covered on the previous poly-line
		unsigned int pointsCovered = currPolyPoint - currPolyLine->getPoints().begin();

		//Update the current lane, poly-line and points
		currLane = lane;
		currPolyLine = currLane->getPolyLine();
		currPolyPoint = currPolyLine->getPoints().begin();
		nextPolyPoint = currPolyPoint + 1;
		
		//Reset next turning
		nextTurning = nullptr;
		
		//Map progress to current poly-line
		if(pointsCovered > 0)
		{
			currPolyPoint += pointsCovered;
			nextPolyPoint += pointsCovered;
		}
	}
	else
	{
		throw std::runtime_error("DriverPathMover::updateLateralMovement(): Invalid lane!");
	}
}

double DriverPathMover::getDistCoveredOnCurrWayPt() const
{
	return distCoveredOnCurrWayPt + distCoveredFromCurrPtToNextPt;
}

double DriverPathMover::getDistToEndOfCurrWayPt() const
{
	double length = (currLane != NULL) ? currLane->getLength() : currTurning->getLength();
	return (length - getDistCoveredOnCurrWayPt());
}

double DriverPathMover::getDistToEndOfCurrLink() const
{
	//Length of the current link
	double linkLength = 0;

	//Distance covered so far
	double distCovered = 0;
	
	if(currLane)
	{
		//Current link
		const Link *link = currLane->getParentSegment()->getParentLink();

		linkLength = link->getLength();
		distCovered = getDistCoveredOnCurrWayPt();

		//Add the lengths of the previous road-segments
		for (std::vector<RoadSegment *>::const_iterator itSegments = link->getRoadSegments().begin(); itSegments != link->getRoadSegments().end(); ++itSegments)
		{
			if ((*itSegments)->getRoadSegmentId() != currLane->getParentSegment()->getRoadSegmentId())
			{
				distCovered += (*itSegments)->getLength();
			}
			else
			{
				break;
			}
		}		
	}
	
	return (linkLength - distCovered);
}

const Point DriverPathMover::getPosition()
{
	if(isDrivingPathSet())
	{
		if(!isDoneWithEntireRoute())
		{
			DynamicVector movementVector(*currPolyPoint, *nextPolyPoint);
			movementVector.scaleVectTo(distCoveredFromCurrPtToNextPt).translateVect();
			return Point(movementVector.getX(), movementVector.getY());
		}
		else
		{
			return *currPolyPoint;
		}
	}
	else
	{
		throw std::runtime_error(ErrorDrivingPathNotSet);
	}
}
