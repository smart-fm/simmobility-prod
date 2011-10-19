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


using namespace sim_mob;
using std::numeric_limits;
using std::max;
using std::vector;
using std::set;

//Some static properties require initialization in the CPP file. ~Seth
const double sim_mob::Driver::MAX_NUM = numeric_limits<double>::max();
const int sim_mob::Driver::MAX_NUM_INT = numeric_limits<int>::max();

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
	perceivedVelocityOfFwdCar(reactTime, true), perceivedDistToFwdCar(reactTime, false),
leader(nullptr)
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
	currLaneOffset_ = 0;



	//assume that all the car has the same size
	length=400;
	width=200;

	timeStep=0.1;			//assume that time step is constant
	isGoalSet = false;
	isOriginSet = false;
	inIntersection=false;
	ischanging = false;
	disToCrossing = 500;
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
		/*boost::mutex::scoped_lock local_lock(BufferedBase::global_mutex);
		std::cout <<"At time " <<currTimeMS <<"ms, with a perception delay of " <<reactTime
				  <<"ms, my actual velocity is (" <<xVel <<"," <<yVel <<"), and my perceived velocity is ("
				  <<perceivedXVelocity <<"," <<perceivedYVelocity <<")\n";*/
	}


	//Also, in case you're wondering, the Point2D that you "new"'d in the FixedDelayed objects is
	// automatically reclaimed. This behavior can be turned off, if the object you are storing is shared.
	//~Seth

	traveledDis_ = 0;
	if(!isOriginSet){
		setOrigin();
		isOriginSet=true;
	}

	//Avoid crashing
	if (!currLane_) {
		return;
	}

	if(!isGoalSet){
		setGoal();
		isGoalSet = true;
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
	else
	{
		//if the vehicle reaches the end of the last road segment on current link
		//I assume this vehicle enters the intersection
		if(isReachLinkEnd())
		{
			inIntersection = true;
			//getTargetLane();
			chooseNextLaneIntersection();
			IntersectionVelocityUpdate();
			xPos += xVel*timeStep;
			yPos += yVel*timeStep;
			abs2relat();
		}
//		else if(isCloseToCrossing())
//		{
//		}
		//not in the last segment of the polyline
		else
		{
			if(isReachPolyLineSegEnd())
			{
				xPos_ -= polyLineSegLength;
				if(!isReachLastPolyLineSeg())
					updatePolyLineSeg();
				else
					updateRSInCurrLink();
			}
			if(ischanging)
				lcPosUpdate();
			//after all cars has been loaded, start lane changing mode
			if(frameNumber>25)
				excuteLaneChanging();
			makeAcceleratingDecision();
			//update
			updateAcceleration();
			updatePosition();
			//yPos_ = 0;

			xVel_ = xVel_ + xAcc_ * timeStep;
			if(xVel_<0)
				std::cout<<"abc"<<xAcc_<<std::endl;
			yVel_ = 0;
			relat2abs();
		}
	}

	setToParent();
	updateAngle();
	output(frameNumber);
}

void sim_mob::Driver::output(frame_t frameNumber)
{
	boost::mutex::scoped_lock local_lock(BufferedBase::global_mutex);
	BufferedBase::log_file()<<"(\"Driver\""
			<<","<<frameNumber
			<<","<<parent->getId()
			<<",{"
			<<"\"xPos\":\""<<xPos
			<<"\",\"yPos\":\""<<yPos
			<<"\",\"angle\":\""<<angle
			<<"\"})"<<std::endl;
}

void sim_mob::Driver::getFromParent()
{
	xPos=parent->xPos.get();
	yPos=parent->yPos.get();
	xVel=parent->xVel.get();
	yVel=parent->yVel.get();
	xAcc=parent->xAcc.get();
	yAcc=parent->yAcc.get();
}

void sim_mob::Driver::setToParent()
{
	parent->xPos.set(xPos);
	parent->yPos.set(yPos);
	parent->xVel.set(xVel);
	parent->yVel.set(yVel);
	parent->xAcc.set(xAcc);
	parent->yAcc.set(yAcc);
}

void sim_mob::Driver::abs_relat()
{
	double xDir = currPolyLineSegEnd.getX()-currPolyLineSegStart.getX();
	double yDir = currPolyLineSegEnd.getY()-currPolyLineSegStart.getY();
	polyLineSegLength = sqrt(xDir*xDir+yDir*yDir);
	xDirection = xDir/polyLineSegLength;
	yDirection = yDir/polyLineSegLength;

}
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

void sim_mob::Driver::updateRSInCurrLink()
{
	targetLane.erase(targetLane.begin(),targetLane.end());
	currNode_ = currRoadSegment_->getEnd();
	const MultiNode* mNode=dynamic_cast<const MultiNode*>(currNode_);
	if(mNode){			//end of link is a intersection
		set<LaneConnector*>::const_iterator i;
		std::cout<<"abc1"<<std::endl;
		set<LaneConnector*> lcs=mNode->getOutgoingLanes(*currRoadSegment_);
		std::cout<<"abc2"<<std::endl;
		for(i=lcs.begin();i!=lcs.end();i++){
			if((*i)->getLaneTo()->getRoadSegment()->getLink()==currLink_
					&& (*i)->getLaneFrom()==currLane_){
				targetLane.push_back((*i)->getLaneTo());
			}
		}
		if(targetLane.size()>0)
		{
			currLane_ = targetLane.at(0);
			updateCurrInfo(1);
		}
		//if can't find next lane, set currlane_ to null
		else
		{
			currLane_ = nullptr;
		}
		return;
	}

	//when end node of current road segment is a uninode

	const UniNode* uNode=dynamic_cast<const UniNode*>(currNode_);
	if(uNode){
		currLane_ = uNode->getOutgoingLane(*currLane_);
		if(currLane_)
			updateCurrInfo(1);
	}
}

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

size_t sim_mob::Driver::getLaneIndex(const Lane* l, const RoadSegment* r)
{
	for(size_t i=0;i<r->getLanes().size();i++)
	{
		if(r->getLanes().at(i)==l)
			return i;
	}
	return -1;
}

void sim_mob::Driver::updateAdjacentLanes()
{
	leftLane_ = nullptr;
	rightLane_ = nullptr;
	if(currLaneIndex_>0)
		rightLane_ = currRoadSegment_->getLanes().at(currLaneIndex_-1);
	if(currLaneIndex_ < currRoadSegment_->getLanes().size()-1)
		leftLane_ = currRoadSegment_->getLanes().at(currLaneIndex_+1);

}

void sim_mob::Driver::updateStartEndIndex()
{
	double length = 0;
	for(size_t i=0;i<currLanePolyLine->size()-1;i++)
	{
		double xOffset = currLanePolyLine->at(i+1).getX() - currLanePolyLine->at(i).getX();
		double yOffset = currLanePolyLine->at(i+1).getY() - currLanePolyLine->at(i).getY();
		length += sqrt(xOffset*xOffset + yOffset*yOffset);
		if(length>=currLaneOffset_)
		{
			startIndex = i;
			endIndex = i+1;
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
	currLaneID_ = currLane_->getLaneID();
	currRoadSegment_ = currLane_->getRoadSegment();

	currLaneIndex_ = getLaneIndex(currLane_,currRoadSegment_);
	updateAdjacentLanes();

	currLanePolyLine = &(currLane_->getPolyline());
	updateCurrLaneLength();
	currLaneOffset_ = 0;

	startIndex = 0;
	endIndex = 1;

	maxLaneSpeed = currRoadSegment_->maxSpeed/3.6;
	targetSpeed = maxLaneSpeed;
	switch(mode)
	{
	case 0:
		updateStartEndIndex();
		currPolyLineSegStart = currLanePolyLine->at(startIndex);
		currPolyLineSegEnd = currLanePolyLine->at(endIndex);
		break;
	case 1:
		currPolyLineSegStart = currLanePolyLine->at(startIndex);
		currPolyLineSegEnd = currLanePolyLine->at(endIndex);
		RSIndex ++;
		break;
	case 2:
		currPolyLineSegStart = currLanePolyLine->at(startIndex);
		currPolyLineSegEnd = currLanePolyLine->at(endIndex);
		currLink_ = currRoadSegment_->getLink();
		currLinkOffset_ = 0;
		linkIndex ++;
		RSIndex = 0;
		roadSegments = &(currLink_->getPath(isForward));
		break;
	default:
		break;
	}
	findCrossing();
	abs_relat();
	//abs2relat();//velocity should be updated separately
}

bool sim_mob::Driver::isReachLastPolyLineSeg()
{
	return (endIndex == currLanePolyLine->size()-1);
}


//update polyline segment in the current RS
void sim_mob::Driver::updatePolyLineSeg()
{
	startIndex++;
	endIndex++;
	currPolyLineSegStart = currLanePolyLine->at(startIndex);
	currPolyLineSegEnd = currLanePolyLine->at(endIndex);
	abs_relat();
}

void sim_mob::Driver::getTargetLane()
{
//	targetLane.erase(targetLane.begin(),targetLane.end());
//	if(isForward)
//		currNode = currLink_->getEnd();
//	else
//		currNode = currLink_->getStart();
//	const MultiNode* mNode=dynamic_cast<const MultiNode*>(currNode);
//	if(mNode){			//end of link is a intersection
//
//		if(linkIndex==linkPath.size()-1){		//last link, no lane changing is needed
//			return;
//		}
//
//		const Link* nextLink=linkPath[linkIndex+1];
//		set<LaneConnector*>::const_iterator i;
//		set<LaneConnector*> lcs=mNode->getOutgoingLanes(*currRoadSegment_);
//		for(i=lcs.begin();i!=lcs.end();i++){
//			if((*i)->getLaneTo()->getRoadSegment()->getLink()==nextLink
//					&& (*i)->getLaneFrom()==currLane_){
//				targetLane.push_back((*i)->getLaneTo());
//			}
//		}
//
//	}
}

//currently it just chooses the first lane from the targetLane
void sim_mob::Driver::chooseNextLaneIntersection()
{
	//only work for 2 links in the path
	if(linkPath[linkIndex+1]->getEnd()==parent->destNode)
	{
		isForward = true;
	}
	else
	{
		isForward = false;
	}



	nextLane_ = linkPath[linkIndex+1]->getPath(isForward).at(0)->getLanes().at(currLaneIndex_);
	entryPoint = nextLane_->getPolyline().at(0);

	double xDir = entryPoint.getX() - xPos;
	double yDir = entryPoint.getY() - yPos;
	disToEntryPoint = sqrt(xDir*xDir+yDir*yDir);
	xDirection_entryPoint = xDir/disToEntryPoint;
	yDirection_entryPoint = yDir/disToEntryPoint;
	xTurningStart = xPos;
	yTurningStart = yPos;

}

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
void sim_mob::Driver::setOrigin()
{
	originNode = parent->originNode;
	const MultiNode* multiOriginNode=dynamic_cast<const MultiNode*>(originNode);
	const MultiNode* end = dynamic_cast<const MultiNode*>(ConfigParams::GetInstance().getNetwork().locateNode(Point2D(37250760,14355120), true));

	if(multiOriginNode)
	{
		currLink_ = findLink(multiOriginNode,end);
		if (currLink_) {
			if(currLink_->getEnd()==end)
				isForward = true;
			else
				isForward = false;
		}
	}

	currLinkOffset_ = 0;
	roadSegments = &(currLink_->getPath(isForward));
	RSIndex = 0;
	currRoadSegment_ = roadSegments->at(RSIndex);
	maxLaneSpeed = currRoadSegment_->maxSpeed/3.6;
	targetSpeed = maxLaneSpeed;
	if(parent->startTime==0)
		currLane_ = currRoadSegment_->getLanes().at(0);
	if(parent->startTime==5)
		currLane_ = currRoadSegment_->getLanes().at(1);
	if(parent->startTime==15)
		currLane_ = currRoadSegment_->getLanes().at(2);
	startIndex = 0;
	endIndex = 1;

	targetSpeed = currRoadSegment_->maxSpeed;
	currLaneID_ = currLane_->getLaneID();
	currLaneIndex_ = getLaneIndex(currLane_,currRoadSegment_);
	currLanePolyLine = &(currLane_->getPolyline());
	currPolyLineSegStart = currLanePolyLine->at(startIndex);
	currPolyLineSegEnd = currLanePolyLine->at(endIndex);
	linkPath.push_back(currLink_);
	linkIndex = 0;
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


void sim_mob::Driver::setGoal()
{
	destNode = parent->destNode;
	const MultiNode* multiDestNode=dynamic_cast<const MultiNode*>(destNode);
	const MultiNode* end = dynamic_cast<const MultiNode*>(ConfigParams::GetInstance().getNetwork().locateNode(Point2D(37250760,14355120), true));
	if(destNode)
	{
		desLink_ = findLink(multiDestNode,end);
	}
	if(desLink_)
		linkPath.push_back(desLink_);
}


void sim_mob::Driver::findCrossing()
{
	const Crossing* crossing=dynamic_cast<const Crossing*>(currRoadSegment_->nextObstacle(xPos_,true).item);
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

bool sim_mob::Driver::isCloseToCrossing()
{
	return (isReachLastRS()&&isCrossingAhead && xPosCrossing_-xPos_-length/2 <= 1000);
}


//TODO: need to check current link is the destination link
bool sim_mob::Driver::isGoalReached()
{
	return (linkIndex == linkPath.size()-1 && isReachLinkEnd());
}


//how to get traffic info?
bool sim_mob::Driver::isReachSignal()
{
	if(closeToCrossing)
	{
		return true;
	}
	else
		return false;
}


//TODO:
bool sim_mob::Driver::isInTheIntersection()
{
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


void sim_mob::Driver::updateAcceleration()
{
	//Set actual acceleration
	xAcc_ = acc_ * 100;
	//yAcc_ = 0;
	//std::cout<<"xacc "<<xAcc_<<" "<<xVel_<<std::endl;
}

/*
void sim_mob::Driver::updateVelocity()
{

	if(isPedestrianAhead()){		//if a pedestrian is ahead, stop
		xVel_ = 0 ; yVel_ =0;
		speed = 0;
		return;
	}

	if(isReachSignal()){
		//nextLink=(currentLane+currentLink+1)%4+4;
		//updateTrafficSignal();
		if(!reachSignalDecision()){
			xVel_ = 0 ; yVel_ =0;
		} else {
		xVel_= targetSpeed/2;yVel_=0;
		}
	} else if(leader && leader_xVel_ == 0 && getDistance()<0.2*length) {
		xVel_=0;
		yVel_=0;
	} else { //when vehicle just gets back to the origin, help them to speed up
		if(xPos_<0) {
			xVel_=(0.2+((double)(rand()%10))/30)*getTargetSpeed();
			yVel_=0;
		} else{
			xVel_ = max(0.0, speed+xAcc_*timeStep);
			yVel_ = 0;//max(0.0, yDirection*speed+yAcc*timeStep);
		}

		if(!ischanging){
			double foward;
			if(!leader) {
				foward=MAX_NUM;
			} else {
				foward=leader_xPos_-xPos_-length;
			}
			if(foward<0) {
				xVel_=0;
			}
			yVel_=0;
		}
	}
	speed=sqrt(xVel_*xVel_+yVel_*yVel_);
}

*/
void sim_mob::Driver::updatePosition()
{
	if(xVel_!=0) { //Only update if velocity is non-zero.
		traveledDis_ = xVel_*timeStep+0.5*xAcc_*timeStep*timeStep;
		xPos_ += traveledDis_;
		currLaneOffset_ += traveledDis_;
		currLinkOffset_ += traveledDis_;
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
	nearby_agents = AuraManager::instance().nearbyAgents(myPos, *currLane_,  distanceInFront, distanceBehind);


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
			if(currRoadSegment_ == otherRoadSegment)
			{
				int distance = other_offset - currLaneOffset_;
				//the vehicle is on the current lane
				if(other_lane == currLane_)
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
				else if(other_lane == leftLane_)
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
				else if(other_lane == rightLane_)
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
					nextNode = currRoadSegment_->getEnd();
				else
					nextNode = currRoadSegment_->getStart();
				const UniNode* uNode=dynamic_cast<const UniNode*>(nextNode);
				const Lane* nextLane = nullptr;
				const Lane* nextLeftLane = nullptr;
				const Lane* nextRightLane = nullptr;
				if(uNode){
					nextLane = uNode->getOutgoingLane(*currLane_);
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

				int distance = other_offset + currLaneLength - currLaneOffset_ -
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
								currLane_)
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

				int distance = other_driver->getCurrLaneLength() - other_offset + currLaneOffset_ -
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

/*
 * When vehicles change lane slowly, crash may happen.
 * To avoid such crash, vehicle should be able to detect it.
 * While here are some rules that some crash will be ignored.
 * More discussion is needed.
 *
 * -wangxy
 * */
//bool sim_mob::Driver::checkForCrash()
//{
//	// now, only vehicles changing lane check if crash may happen
//	if(!ischanging) {
//		return false;
//	}
//	//check other vehicles
//	Agent* other = nullptr;
//	for (size_t i=0; i<Agent::all_agents.size(); i++) {
//		//Skip self
//		other = Agent::all_agents[i];
//		Person* p = dynamic_cast<Person*>(other);
//		if (!p) {
//			continue;
//		}
//		Driver* d = dynamic_cast<Driver*>(p->getRole());
//		if (other->getId()==parent->getId()
//				|| !d || d->getLink()!=currentLink)
//		{
//			continue;
//		}
//		double other_xOffset	= other->xPos.get()	- testLinks[currentLink].startX;
//		double other_yOffset	= other->yPos.get()	- testLinks[currentLink].startY;
//		double other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
//		double other_yPos_		=-other_xOffset	* yDirection	+ other_yOffset	* xDirection;
//
//		//Check. When other vehicle is too close to subject vehicle, crash will happen
//		if(
//				(other_yPos_ < yPos_+width*1.1)
//				&& (other_yPos_ > yPos_-width*1.1)
//				&& (other_xPos_ < xPos_+length*0.9)
//				&& (other_xPos_ > xPos_-length*0.9)
//				)
//		{
//			//but some kind of crash can be ignored. Cases below must be regard as crash.
//			if(
//					((fromLane>toLane && other_yPos_ < yPos_)
//				||(fromLane<toLane && other_yPos_ > yPos_))
//						)
//			{		//the other vehicle is on the position where subject vehicle is approaching
//				if(checkIfOnTheLane(other_yPos_)){		//if other vehicle is on the lane
//					return true;								//subject vehicle should avoid it
//				} else if(other_xPos_>xPos_){	//if both vehicle is changing lane
//					if(parent->getId()<other->getId()){
//						return true;		//one has bigger ID will not be affected
//					}
//					else{
//						return false;
//					}
//				} else {
//					return false;
//				}
//			} else{
//				return false;
//			}
//		}
//	}
//	//check bad areas
//	for(int i=0;i<numOfBadAreas;i++) {
//		if(
//				badareas[i].startX < xPos_-length
//				&& badareas[i].endX > xPos_+length
//				&& testLinks[currentLink].laneWidth*badareas[i].lane + testLinks[currentLink].laneWidth/2+width/2 > yPos_
//				&& testLinks[currentLink].laneWidth*badareas[i].lane - testLinks[currentLink].laneWidth/2-width/2 < yPos_
//		) {
//			return true;
//		}
//	}
//	return false;
//}

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



bool sim_mob :: Driver :: reachSignalDecision()
{
//	return trafficSignal->get_Driver_Light(currentLink,2-currentLane)!=1;
	return true;
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
	currLane_ = nextLane_;
	updateCurrInfo(2);
	abs2relat();
	yVel_ = 0;
	yPos_ = 0;
	yAcc_ = 0;
	relat2abs();
}

void sim_mob::Driver::lcPosUpdate()
{
	//right
	if(changeDecision == 1)
	{
		if(!lrEnterNewLane && -yPos_>=150)//currLane_->getWidth()/2.0)
		{
			currLane_ = rightLane_;
			updateCurrInfo(0);
			abs2relat();
			lrEnterNewLane = true;

		}
		if(lrEnterNewLane && yPos_ <=0)
		{
			ischanging =false;
			yPos_ = 0;
		}
	}
	else if(changeDecision == -1)
	{
		if(!lrEnterNewLane && yPos_>=150)//currLane_->getWidth()/2.0)
		{
			currLane_ = leftLane_;
			updateCurrInfo(0);
			abs2relat();
			lrEnterNewLane = true;
		}
		if(lrEnterNewLane && yPos_ >=0)
		{
			ischanging =false;
			yPos_ = 0;
		}
	}

}

void sim_mob::Driver::updateTrafficSignal()
{
	Agent* other = nullptr;
	for (size_t i=0; i<Agent::all_agents.size(); i++) {
		other = Agent::all_agents[i];
		Signal* s = dynamic_cast<Signal*>(other);
		if (s) {
			trafficSignal=s;	//since there is only 1 traffic signal
			return;
		}
	}
}
//
//bool sim_mob::Driver::isPedestrianAhead()
//{
//
//	Agent* other = nullptr;
//	for (size_t i=0; i<Agent::all_agents.size(); i++) {
//		other = Agent::all_agents[i];
//		Person* p = dynamic_cast<Person*>(other);
//		if (!p) {
//			continue;
//		}
//		Pedestrian* pd = dynamic_cast<Pedestrian*>(p->getRole());
//		if (other->getId()==parent->getId()|| !pd
//				|| other->currentCrossing.get()!=currentLink)//only check pedestrian on crossing of this link
//		{
//			continue;
//		}
//
//		double other_xOffset	= other->xPos.get()	- testLinks[currentLink].startX;
//		double other_yOffset	= other->yPos.get()	- testLinks[currentLink].startY;
//		double other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
//
//		//Check. If pedestrian is right ahead the vehicle, return true
//
//		bool is_right_ahead = false;
//		if(leader_xPos_>xPos_)
//			is_right_ahead = other_xPos_>xPos_&&other_xPos_<leader_xPos_;
//		else
//			is_right_ahead = other_xPos_>xPos_;
//		if(is_right_ahead){
//			if((other_xPos_-xPos_)<=10)
//			{
//
//				return true;
//			}
//		}
//	}
//	return false;
//}
