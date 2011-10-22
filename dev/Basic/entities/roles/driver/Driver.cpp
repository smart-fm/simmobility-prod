/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Driver.cpp
 *
 *  Created on: 2011-7-5
 *      Author: wangxy & Li Zhemin
 */


#include "Driver.hpp"


#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/Person.hpp"
#include "entities/Signal.hpp"
#include "entities/AuraManager.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Crossing.hpp"
#include "util/OutputUtil.hpp"


using namespace sim_mob;
using std::numeric_limits;
using std::max;
using std::vector;
using std::set;

//Some static properties require initialization in the CPP file. ~Seth
const double sim_mob::Driver::MAX_NUM = numeric_limits<double>::max();

//for unit conversion
double sim_mob::Driver::feet2Unit(double feet)
{
	return feet*0.158;
}
double sim_mob::Driver::unit2Feet(double unit)
{
	return unit/0.158;
}

//initiate
sim_mob::Driver::Driver(Agent* parent) : Role(parent), perceivedVelocity(reactTime, true),
	perceivedVelocityOfFwdCar(reactTime, true), perceivedDistToFwdCar(reactTime, false)
{
	//Set random seed
	//NOTE: This will reset the sequence returned by rand(); it's not a good idea.
	//      I moved srand() initialization into main.cpp; we'll need to make our own
	//      random data management classes later.
	//srand(parent->getId());

	//Set default speed in the range of 10m/s to 19m/s
	speed = 1000*(1+((double)(rand()%10))/10);

	//Set default data for acceleration
	acc_ = 0;
	maxAcceleration = MAX_ACCELERATION;
	normalDeceleration = -maxAcceleration*0.6;
	maxDeceleration = -maxAcceleration;

	//basic absolute parameters
	xPos = 0;
	yPos = 0;
	xVel = 0;
	yVel = 0;
	xAcc = 0;
	yAcc = 0;

	//basic relative parameters
	xPos_ = 0;
	yPos_ = 0;
	xVel_ = 0;
	yVel_ = 0;
	xAcc_ = 0;
	yAcc_ = 0;

	currLaneLength = 0;
	currLaneOffset = 0;

	//assume that all the car has the same size
	length=400;
	width=200;

	timeStep=0.1;			//assume that time step is constant
	isGoalSet = false;
	isOriginSet = false;
	inIntersection=false;
	isLaneChanging = false;
	angle = 0;
}

//Main update functionality
void sim_mob::Driver::update(frame_t frameNumber)
{
	//Retrieve the current time in ms
	unsigned int currTimeMS = frameNumber * ConfigParams::GetInstance().baseGranMS;

	//Update your perceptions.
	//NOTE: This should be done as perceptions arrive, but the following code kind of "mixes"
	//      input and decision-making. ~Seth
	perceivedVelocity.delay(new Point2D(xVel, yVel), currTimeMS);
	//perceivedVelocityOfFwdCar.delay(new Point2D(otherCarXVel, otherCarYVel), currTimeMS);
	//perceivedDistToFwdCar.delay(distToOtherCar, currTimeMS);

	//Now, retrieve your sensed velocity, distance, etc.
	double perceivedXVelocity = 0;  //Choose sensible defaults.
	double perceivedYVelocity = 0;
	if (perceivedVelocity.can_sense(currTimeMS)) {
		perceivedXVelocity = perceivedVelocity.sense(currTimeMS)->getX();
		perceivedYVelocity = perceivedVelocity.sense(currTimeMS)->getY();
	}

	//Here, you can use the "perceived" velocity to perform decision-making. Just be
	// careful about how you're saving the velocity values. ~Seth
	if (parent->getId()==0) {
		/*
		LogOut("At time " <<currTimeMS <<"ms, with a perception delay of " <<reactTime
				  <<"ms, my actual velocity is (" <<xVel <<"," <<yVel <<"), and my perceived velocity is ("
				  <<perceivedXVelocity <<"," <<perceivedYVelocity <<")\n");*/
	}


	//Also, in case you're wondering, the Point2D that you "new"'d in the FixedDelayed objects is
	// automatically reclaimed. This behavior can be turned off, if the object you are storing is shared.
	//~Seth


	traveledDis = 0;
	if(!isOriginSet){
		makeLinkPath();
		if(linkPath.size()==0)
			return;
		setOrigin();
		isOriginSet=true;
	}
	//still need to be modified.
	//if reach the goal, get back to the origin
	if(isGoalReached()){
		//TODO:reach destination
		setBackToOrigin();
		return;
	}

	updateNearbyAgents();

	//inside intersection
	if(inIntersection)
		intersectionDriving();
	else
	{
		//the first time vehicle pass the end of current link and enter intersection
		//if the vehicle reaches the end of the last road segment on current link
		//I assume this vehicle enters the intersection
		if(isReachLinkEnd())
		{
			inIntersection = true;
			chooseNextLaneIntersection();
			IntersectionVelocityUpdate();
			intersectionDriving();
		}
		else
		{
			//the relative coordinate system is based on each polyline segment
			//so when the polyline segment has been updated, the coordinate system should also be updated
			if(isReachPolyLineSegEnd())
			{
				xPos_ -= polyLineSegLength;
				if(!isReachLastPolyLineSeg())
					updatePolyLineSeg();
				else
				{
					updateRSInCurrLink();
					//if can't find available lane in new road segment
					//this indicates an error
					if(!currLane)
						return;
				}
			}
			if(isCloseToLinkEnd())
				trafficSignalDriving();
			else
				linkDriving();
		}
	}
	setToParent();
	updateAngle();
	output(frameNumber);
}

void sim_mob::Driver::output(frame_t frameNumber)
{
	LogOut("(\"Driver\""
			<<","<<frameNumber
			<<","<<parent->getId()
			<<",{"
			<<"\"xPos\":\""<<xPos
			<<"\",\"yPos\":\""<<yPos
			<<"\",\"angle\":\""<<angle
			<<"\"})"<<std::endl);
}

//responsible for vehicle behaviour inside intersection
//the movement is based on absolute position
void sim_mob::Driver::intersectionDriving()
{
	if(isLeaveIntersection())
	{
		inIntersection = false;
		enterNextLink();
	}
	else
	{
		xPos += xVel*timeStep;
		yPos += yVel*timeStep;
		abs2relat();
	}
}

//vehicle movement on link, perform acceleration, lane changing if necessary
//the movement is based on relative position
void sim_mob::Driver::linkDriving()
{
	if(isLaneChanging)
		updatePosLC();
	excuteLaneChanging();
	makeAcceleratingDecision();
	updateAcceleration();
	updatePositionOnLink();
	xVel_ = xVel_ + xAcc_ * timeStep;
	yVel_ = 0;
	relat2abs();
}

//TODO:this function seems no use
void sim_mob::Driver::getFromParent()
{
	xPos=parent->xPos.get();
	yPos=parent->yPos.get();
	xVel=parent->xVel.get();
	yVel=parent->yVel.get();
	xAcc=parent->xAcc.get();
	yAcc=parent->yAcc.get();
}

//for buffer data, maybe need more parameters to be stored
//TODO: need to discuss
void sim_mob::Driver::setToParent()
{
	parent->xPos.set(xPos);
	parent->yPos.set(yPos);
	parent->xVel.set(xVel);
	parent->yVel.set(yVel);
	parent->xAcc.set(xAcc);
	parent->yAcc.set(yAcc);
}


//calculate absolute-relative transform vectors
void sim_mob::Driver::abs_relat()
{
	double xDir = currPolyLineSegEnd.getX()-currPolyLineSegStart.getX();
	double yDir = currPolyLineSegEnd.getY()-currPolyLineSegStart.getY();
	polyLineSegLength = sqrt(xDir*xDir+yDir*yDir);
	xDirection = xDir/polyLineSegLength;
	yDirection = yDir/polyLineSegLength;
}

//convert absolute coordinate system to relative one
void sim_mob::Driver::abs2relat()
{
	double xOffset=xPos-currPolyLineSegStart.getX();
	double yOffset=yPos-currPolyLineSegStart.getY();
	xPos_= xOffset*xDirection+yOffset*yDirection;
	yPos_=-xOffset*yDirection+yOffset*xDirection;
	xVel_= xVel*xDirection+yVel*yDirection;
	yVel_=-xVel*yDirection+yVel*xDirection;
	xAcc_= xAcc*xDirection+yAcc*yDirection;
	yAcc_=-xAcc*yDirection+yAcc*xDirection;
}

//convert relative coordinate system to absolute one
void sim_mob::Driver::relat2abs()
{
	xPos=xPos_*xDirection-yPos_*yDirection+currPolyLineSegStart.getX();
	yPos=xPos_*yDirection+yPos_*xDirection+currPolyLineSegStart.getY();
	xVel=xVel_*xDirection-yVel_*yDirection;
	yVel=xVel_*yDirection+yVel_*xDirection;
	xAcc=xAcc_*xDirection-yAcc_*yDirection;
	yAcc=xAcc_*yDirection+yAcc_*xDirection;
}

bool sim_mob::Driver::isReachPolyLineSegEnd()
{
	if(xPos_>=polyLineSegLength)
		return true;
	else
		return false;
}

bool sim_mob::Driver::isReachLastRS()
{
	if(RSIndex==roadSegments->size()-1)
		return true;
	else
		return false;
}

bool sim_mob::Driver::isReachLastPolyLineSeg()
{
	return (polylineSegIndex >= currLanePolyLine->size()-2);
}

//TODO
bool sim_mob::Driver::isCloseToCrossing()
{
	return (isReachLastRS()&&isCrossingAhead && xPosCrossing_-xPos_-length/2 <= 1000);
}


//when the vehicle reaches the end of last link in link path
bool sim_mob::Driver::isGoalReached()
{
	return (linkIndex == linkPath.size()-1 && isReachLinkEnd());
}


//TODO:
bool sim_mob::Driver::isReachSignal()
{
	if(closeToCrossing)
	{
		return true;
	}
	else
		return false;
}

bool sim_mob::Driver::isReachLinkEnd()
{
	return (isReachLastRS()&&isReachLastPolyLineSeg()&&isReachPolyLineSegEnd());
}

bool sim_mob::Driver::isLeaveIntersection()
{
	double currXoffset = xPos - xTurningStart;
	double currYoffset = yPos - yTurningStart;
	int currDisToEntrypoint = sqrt(currXoffset*currXoffset + currYoffset*currYoffset);
	return currDisToEntrypoint >= disToEntryPoint;
}

void sim_mob::Driver::updateRSInCurrLink()
{
	targetLane.erase(targetLane.begin(),targetLane.end());
	currNode = currRoadSegment->getEnd();
	const MultiNode* mNode=dynamic_cast<const MultiNode*>(currNode);
	if(mNode){
		set<LaneConnector*>::const_iterator i;
		set<LaneConnector*> lcs=mNode->getOutgoingLanes(*currRoadSegment);
		for(i=lcs.begin();i!=lcs.end();i++){
			if((*i)->getLaneTo()->getRoadSegment()->getLink()==currLink
					&& (*i)->getLaneFrom()==currLane){
				targetLane.push_back((*i)->getLaneTo());
			}
		}
		if(targetLane.size()>0)
		{
			currLane = targetLane.at(0);
			updateCurrInfo(1);
		}
		//if can't find next lane, set current lane to null
		else
			currLane = nullptr;
		return;
	}

	//when end node of current road segment is a uninode
	const UniNode* uNode=dynamic_cast<const UniNode*>(currNode);
	if(uNode){
		currLane = uNode->getOutgoingLane(*currLane);
		if(currLane)
			updateCurrInfo(1);
	}
}


//calculate current lane length
void sim_mob::Driver::updateCurrLaneLength()
{
	currLaneLength = 0;
	for(size_t i=0;i<currLanePolyLine->size()-1;i++)
	{
		double xOffset = currLanePolyLine->at(i+1).getX() - currLanePolyLine->at(i).getX();
		double yOffset = currLanePolyLine->at(i+1).getY() - currLanePolyLine->at(i).getY();
		currLaneLength += sqrt(xOffset*xOffset + yOffset*yOffset);
	}
}

//TODO:I think lane index should be a data member in the lane class
size_t sim_mob::Driver::getLaneIndex(const Lane* l, const RoadSegment* r)
{
	for(size_t i=0;i<r->getLanes().size();i++)
	{
		if(r->getLanes().at(i)==l)
			return i;
	}
	return -1;
}


//update left and right lanes of the current lane
//if there is no left or right lane, it will be null
void sim_mob::Driver::updateAdjacentLanes()
{
	leftLane = nullptr;
	rightLane = nullptr;
	if(currLaneIndex>0)
		rightLane = currRoadSegment->getLanes().at(currLaneIndex-1);
	if(currLaneIndex < currRoadSegment->getLanes().size()-1)
		leftLane = currRoadSegment->getLanes().at(currLaneIndex+1);
}


//used in lane changing, find the start index and end index of polyline in the target lane
void sim_mob::Driver::updateStartEndIndex()
{
	double offset = 0;
	for(size_t i=0;i<currLanePolyLine->size()-1;i++)
	{
		double xOffset = currLanePolyLine->at(i+1).getX() - currLanePolyLine->at(i).getX();
		double yOffset = currLanePolyLine->at(i+1).getY() - currLanePolyLine->at(i).getY();
		offset += sqrt(xOffset*xOffset + yOffset*yOffset);
		if(offset>=currLaneOffset)
		{
			polylineSegIndex = i;
			break;
		}
	}
}

//when current lane has been changed, update current information.
//mode 0: within same RS, for lane changing model
//mode 1: during RS changing, but in the same link
//mode 2: during crossing intersection
void sim_mob::Driver::updateCurrInfo(unsigned int mode)
{
	currRoadSegment = currLane->getRoadSegment();
	currLaneIndex = getLaneIndex(currLane,currRoadSegment);
	updateAdjacentLanes();
	currLanePolyLine = &(currLane->getPolyline());
	updateCurrLaneLength();
	maxLaneSpeed = currRoadSegment->maxSpeed/3.6;
	targetSpeed = maxLaneSpeed;

	switch(mode)
	{
	case 0:
		updateStartEndIndex();
		currPolyLineSegStart = currLanePolyLine->at(polylineSegIndex);
		currPolyLineSegEnd = currLanePolyLine->at(polylineSegIndex+1);
		break;
	case 1:
		currLaneOffset = 0;
		polylineSegIndex = 0;
		currPolyLineSegStart = currLanePolyLine->at(polylineSegIndex);
		currPolyLineSegEnd = currLanePolyLine->at(polylineSegIndex+1);
		RSIndex ++;
		if(isReachLastRS()&&linkIndex<linkPath.size()-1)
		{
			updateTrafficSignal();
			chooseNextLaneIntersection();

		}
		break;
	case 2:
		currLaneOffset = 0;
		polylineSegIndex = 0;
		currPolyLineSegStart = currLanePolyLine->at(polylineSegIndex);
		currPolyLineSegEnd = currLanePolyLine->at(polylineSegIndex+1);
		currLink = currRoadSegment->getLink();
		currLinkOffset = 0;
		linkIndex ++;
		RSIndex = 0;
		roadSegments = &(currLink->getPath(isForward));
		break;
	default:
		break;
	}
	abs_relat();
}

//update polyline segment in the current RS
void sim_mob::Driver::updatePolyLineSeg()
{
	polylineSegIndex++;
	currPolyLineSegStart = currLanePolyLine->at(polylineSegIndex);
	currPolyLineSegEnd = currLanePolyLine->at(polylineSegIndex+1);
	abs_relat();
}


//currently it just chooses the first lane from the targetLane
void sim_mob::Driver::chooseNextLaneIntersection()
{
	if(linkPath[linkIndex+1]->getStart()!=linkPath[linkIndex]->getEnd())
		isForward = !isForward;

	nextLane = linkPath[linkIndex+1]->getPath(isForward).at(0)->getLanes().at(currLaneIndex);
	//if there is no lane with same index in the new road segment
	//set the next lane as lane 0
	if(!nextLane)
		nextLane = linkPath[linkIndex+1]->getPath(isForward).at(0)->getLanes().at(0);

	entryPoint = nextLane->getPolyline().at(0);
	double xDir = entryPoint.getX() - xPos;
	double yDir = entryPoint.getY() - yPos;
	disToEntryPoint = sqrt(xDir*xDir+yDir*yDir);
	xDirection_entryPoint = xDir/disToEntryPoint;
	yDirection_entryPoint = yDir/disToEntryPoint;
	xTurningStart = xPos;
	yTurningStart = yPos;
}

//TODO:
const Link* sim_mob::Driver::findLink(const MultiNode* start, const MultiNode* end)
{
	const Link* link;
	std::set<sim_mob::RoadSegment*>::const_iterator i;
	const std::set<sim_mob::RoadSegment*>& roadsegments=start->getRoadSegments();
	for(i=roadsegments.begin();i!=roadsegments.end();i++){
		link = (*i)->getLink();
		if(link->getStart()==end)
			return link;
		else if(link->getEnd()==end)
			return link;
	}
	return nullptr;
}

//TODO
void sim_mob::Driver::setBackToOrigin()
{
	xPos = parent->originNode->location->getX();
	yPos = parent->originNode->location->getY();
	xVel = 0;
	yVel = 0;
	xAcc = 0;
	yAcc = 0;
	setToParent();
}


//link path should be retrieved from other class
//for now, it serves as this purpose
void sim_mob::Driver::makeLinkPath()
{
	originNode = parent->originNode;
	destNode = parent->destNode;
	const MultiNode* multiOriginNode=dynamic_cast<const MultiNode*>(originNode);
	const MultiNode* multiDestNode=dynamic_cast<const MultiNode*>(destNode);
	const MultiNode* end = dynamic_cast<const MultiNode*>(ConfigParams::GetInstance().getNetwork().locateNode(Point2D(37250760,14355120), true));
	if(!multiOriginNode||!multiDestNode||!end)
		return;
	const Link* l;
	l = findLink(multiOriginNode,end);
	if(l)
		linkPath.push_back(l);
	l = findLink(multiDestNode,end);
	if(l)
		linkPath.push_back(l);
}

void sim_mob::Driver::setOrigin()
{
	//Retrieve the first link in the link path vector
	linkIndex = 0;
	currLink = linkPath.at(linkIndex);

	//decide the direction
	if(currLink->getStart()==originNode)
		isForward = true;
	else
		isForward = false;

	currLinkOffset = 0;

	//Retrieve the first road segment
	roadSegments = &(currLink->getPath(isForward));
	RSIndex = 0;
	currRoadSegment = roadSegments->at(RSIndex);

	maxLaneSpeed = currRoadSegment->maxSpeed/3.6;
	targetSpeed = maxLaneSpeed;

	currLaneIndex = 0;
	currLane = currRoadSegment->getLanes().at(currLaneIndex);

	polylineSegIndex = 0;
	currLanePolyLine = &(currLane->getPolyline());
	currPolyLineSegStart = currLanePolyLine->at(polylineSegIndex);
	currPolyLineSegEnd = currLanePolyLine->at(polylineSegIndex+1);

	//Initialise starting position
	xPos = currPolyLineSegStart.getX();
	yPos = currPolyLineSegStart.getY();
	abs_relat();
	abs2relat();
	xVel_=speed;
	yVel_=0;
	xAcc_=0;
	yAcc_=0;

	updateAdjacentLanes();
	updateCurrLaneLength();
}


//TODO
void sim_mob::Driver::findCrossing()
{
	const Crossing* crossing=dynamic_cast<const Crossing*>(currRoadSegment->nextObstacle(xPos_,true).item);
	if(crossing)
	{
		Point2D far1 = crossing->farLine.first;
		Point2D far2 = crossing->farLine.second;

		double slope1, slope2;
		slope1 = (double)(far2.getY()-far1.getY())/(far2.getX()-far1.getX());
		slope2 = (double)(currPolyLineSegEnd.getY()-currPolyLineSegStart.getY())/
				(currPolyLineSegEnd.getX()-currPolyLineSegStart.getX());
		int x = (slope2*currPolyLineSegStart.getX()-slope1*far1.getX()+far1.getY()-currPolyLineSegStart.getY())/(slope2-slope1);
		int y = slope1*(x-far1.getX())+far1.getY();

		double xOffset=x-currPolyLineSegStart.getX();
		double yOffset=y-currPolyLineSegStart.getY();
		xPosCrossing_= xOffset*xDirection+yOffset*yDirection;
		isCrossingAhead = true;
	}
	else
		isCrossingAhead = false;
}

void sim_mob::Driver::updateAcceleration()
{
	xAcc_ = acc_ * 100;
}

void sim_mob::Driver::updatePositionOnLink()
{
	if(xVel_!=0) { //Only update if velocity is non-zero.
		traveledDis = xVel_*timeStep+0.5*xAcc_*timeStep*timeStep;
		xPos_ += traveledDis;
		currLaneOffset += traveledDis;
		currLinkOffset += traveledDis;
	}
}

void sim_mob::Driver::updateNearbyAgents()
{
	CFD = nullptr;
	CBD = nullptr;
	LFD = nullptr;
	LBD = nullptr;
	RFD = nullptr;
	RBD = nullptr;

	Point2D myPos(xPos,yPos);
	distanceInFront = 1000;
	distanceBehind = 500;

	nearby_agents = AuraManager::instance().nearbyAgents(myPos, *currLane,  distanceInFront, distanceBehind);

	minCFDistance = 5000;
	minCBDistance = 5000;
	minLFDistance = 5000;
	minLBDistance = 5000;
	minRFDistance = 5000;
	minRBDistance = 5000;

	for(unsigned int i=0;i<nearby_agents.size();i++)
	{
		const Person *person = dynamic_cast<const Person *>(nearby_agents.at(i));
		if(!person)
			continue;
		Person* p = const_cast<Person*>(person);
		Driver* other_driver = dynamic_cast<Driver*>(p->getRole());
		if(other_driver == this)
			continue;
		if(other_driver)
		{
			const Lane* other_lane = other_driver->getCurrLane();
			const RoadSegment* otherRoadSegment = other_lane->getRoadSegment();
			int other_offset = other_driver->getCurrLaneOffset();
			//the vehicle is in the current road segment
			if(currRoadSegment == otherRoadSegment)
			{
				int distance = other_offset - currLaneOffset;
				//the vehicle is on the current lane
				if(other_lane == currLane)
				{
					//the vehicle is in front
					if(distance > 0)
					{
						distance = distance - length/2 - other_driver->getLength()/2;
						if(distance <= minCFDistance)
						{
							CFD = other_driver;
							minCFDistance = distance;
						}
					}
					else
					{
						distance = - distance;
						distance = distance - length/2 - other_driver->getLength()/2;
						if(distance <= minCBDistance)
						{
							CBD = other_driver;
							minCBDistance = distance;
						}
					}
				}
				//the vehicle is on the left lane
				else if(other_lane == leftLane)
				{
					//the vehicle is in front
					if(distance > 0)
					{
						distance = distance - length/2 - other_driver->getLength()/2;
						if(distance <= minLFDistance)
						{
							LFD = other_driver;
							minLFDistance = distance;
						}
					}
					else
					{
						distance = - distance;
						distance = distance - length/2 - other_driver->getLength()/2;
						if(distance <= minLBDistance)
						{
							LBD = other_driver;
							minLBDistance = distance;
						}
					}
				}
				else if(other_lane == rightLane)
				{
					//the vehicle is in front
					if(distance > 0)
					{
						distance = distance - length/2 - other_driver->getLength()/2;
						if(distance <= minRFDistance)
						{
							RFD = other_driver;
							minRFDistance = distance;
						}
					}
					else if(distance < 0 && -distance <= minRBDistance)
					{
						distance = - distance;
						distance = distance - length/2 - other_driver->getLength()/2;
						if(distance <= minRBDistance)
						{
							RBD = other_driver;
							minRBDistance = distance;
						}
					}
				}
			}
			//the vehicle is in the next road segment
			else if(RSIndex < roadSegments->size()-1 && otherRoadSegment == roadSegments->at(RSIndex + 1))
			{
				const Node* nextNode;
				if(isForward)
					nextNode = currRoadSegment->getEnd();
				else
					nextNode = currRoadSegment->getStart();
				const UniNode* uNode=dynamic_cast<const UniNode*>(nextNode);
				const Lane* nextLane = nullptr;
				const Lane* nextLeftLane = nullptr;
				const Lane* nextRightLane = nullptr;
				if(uNode){
					nextLane = uNode->getOutgoingLane(*currLane);
				}
				//make sure next lane is in the next road segment, although it should be true
				if(nextLane && nextLane->getRoadSegment() == otherRoadSegment)
				{
					//the next lane isn't the left most lane
					size_t nextLaneIndex = getLaneIndex(nextLane, otherRoadSegment);
					if(nextLaneIndex>0)
						nextLeftLane = otherRoadSegment->getLanes().at(nextLaneIndex-1);
					if(nextLaneIndex < otherRoadSegment->getLanes().size()-1)
						nextRightLane = otherRoadSegment->getLanes().at(nextLaneIndex+1);
				}

				int distance = other_offset + currLaneLength - currLaneOffset -
						length/2 - other_driver->getLength()/2;
				//the vehicle is on the current lane
				if(other_lane == nextLane)
				{
					//the vehicle is in front
					if(distance <= minCFDistance)
					{
						CFD = other_driver;
						minCFDistance = distance;
					}
				}
				//the vehicle is on the left lane
				else if(other_lane == nextLeftLane)
				{
					//the vehicle is in front
					if(distance <= minLFDistance)
					{
						LFD = other_driver;
						minLFDistance = distance;
					}
				}
				else if(other_lane == nextRightLane)
				{
					//the vehicle is in front
					if(distance <= minRFDistance)
					{
						RFD = other_driver;
						minRFDistance = distance;
					}
				}
			}
			//the vehicle is in the previous road segment
			else if(RSIndex > 0 && otherRoadSegment == roadSegments->at(RSIndex - 1))
			{
				const Node* preNode;
				if(isForward)
					preNode = otherRoadSegment->getEnd();
				else
					preNode = otherRoadSegment->getStart();
				const UniNode* uNode=dynamic_cast<const UniNode*>(preNode);
				const Lane* preLane = nullptr;
				const Lane* preLeftLane = nullptr;
				const Lane* preRightLane = nullptr;
				if(uNode){
					for(size_t i=0;i<otherRoadSegment->getLanes().size();i++)
					{
						const Lane* tempLane = otherRoadSegment->getLanes().at(i);
						if(uNode->getOutgoingLane(*tempLane) ==
								currLane)
						{
							preLane = tempLane;
							break;
						}
					}
				}
				//make sure next lane is in the next road segment, although it should be true
				if(preLane)
				{
					size_t preLaneIndex = getLaneIndex(preLane, otherRoadSegment);
					//the next lane isn't the left most lane
					if(preLaneIndex>0)
						preLeftLane = otherRoadSegment->getLanes().at(preLaneIndex-1);
					if(preLaneIndex < otherRoadSegment->getLanes().size()-1)
						preRightLane = otherRoadSegment->getLanes().at(preLaneIndex+1);
				}

				int distance = other_driver->getCurrLaneLength() - other_offset + currLaneOffset -
						length/2 - other_driver->getLength()/2;
				//the vehicle is on the current lane
				if(other_lane == preLane)
				{
					//the vehicle is
					if(distance <= minCBDistance)
					{
						CBD = other_driver;
						minCBDistance = distance;
					}
				}
				//the vehicle is on the left lane
				else if(other_lane == preLeftLane)
				{
					if(distance <= minLBDistance)
					{
						LBD = other_driver;
						minLBDistance = distance;
					}
				}
				//the vehicle is on the right lane
				else if(other_lane == preRightLane)
				{
					if(distance <= minRBDistance)
					{
						RBD = other_driver;
						minRBDistance = distance;
					}
				}
			}
		}
	}
}

//Angle shows the velocity direction of vehicles
void sim_mob::Driver::updateAngle()
{
	if(xVel==0 && yVel==0){}
    else if(xVel>=0 && yVel>=0)angle = 360 - atan(yVel/xVel)/3.1415926*180;
	else if(xVel>=0 && yVel<0)angle = - atan(yVel/xVel)/3.1415926*180;
	else if(xVel<0 && yVel>=0)angle = 180 - atan(yVel/xVel)/3.1415926*180;
	else if(xVel<0 && yVel<0)angle = 180 - atan(yVel/xVel)/3.1415926*180;
	else{}
}

void sim_mob :: Driver :: IntersectionVelocityUpdate()
{
	xAcc_=0;
	yAcc_=0;
	xVel = speed * xDirection_entryPoint;
	yVel = speed * yDirection_entryPoint;
}

void sim_mob :: Driver :: enterNextLink()
{
	currLane = nextLane;
	updateCurrInfo(2);
	abs2relat();
	yVel_ = 0;
	yPos_ = 0;
	yAcc_ = 0;
	relat2abs();
}

void sim_mob::Driver::updatePosLC()
{
	//right
	if(changeDecision == 1)
	{
		if(!lcEnterNewLane && -yPos_>=150)//currLane_->getWidth()/2.0)
		{
			currLane = rightLane;
			updateCurrInfo(0);
			abs2relat();
			lcEnterNewLane = true;

		}
		if(lcEnterNewLane && yPos_ <=0)
		{
			isLaneChanging =false;
			yPos_ = 0;
		}
	}
	else if(changeDecision == -1)
	{
		if(!lcEnterNewLane && yPos_>=150)//currLane_->getWidth()/2.0)
		{
			currLane = leftLane;
			updateCurrInfo(0);
			abs2relat();
			lcEnterNewLane = true;
		}
		if(lcEnterNewLane && yPos_ >=0)
		{
			isLaneChanging =false;
			yPos_ = 0;
		}
	}
}

bool sim_mob::Driver::isCloseToLinkEnd()
{
	//when the distance <= 10m
	return isReachLastRS()&&(currLaneLength - currLaneOffset<1000);
}

void sim_mob::Driver::updateTrafficSignal()
{
	const Node* node = currRoadSegment->getEnd();
	if(node)
		trafficSignal = StreetDirectory::instance().signalAt(*node);
	else
		trafficSignal = nullptr;
}

void sim_mob::Driver::trafficSignalDriving()
{
	if(!trafficSignal)
		linkDriving();
	else
	{
		int color = trafficSignal->getDriverLight(*currLane,*nextLane);
		switch(color)
		{
		//red yellow
		case 0:case 1:
			if(xVel_>0)
			{
				xAcc_ = -0.5*xVel_*xVel_/(currLaneLength-currLaneOffset);
				xVel_ += xAcc_*timeStep;
			}
			else
				xVel_ = 0;
			yVel_ = 0;
			relat2abs();
			break;
		//green
		case 2:
			linkDriving();
			break;
		}
	}
}
