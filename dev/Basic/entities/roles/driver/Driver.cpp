/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Driver.cpp
 *
 *  Created on: 2011-7-5
 *      Author: wangxy & Li Zhemin
 */


#include "Driver.hpp"

using namespace sim_mob;
using std::numeric_limits;
using std::max;
using std::vector;
using std::set;

//Some static properties require initialization in the CPP file. ~Seth
const double sim_mob::Driver::maxLaneSpeed[] = {120,140,180};
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
sim_mob::Driver::Driver(Agent* parent) : Role(parent), leader(nullptr)
{
	//Set random seed
	//NOTE: This will reset the sequence returned by rand(); it's not a good idea.
	//      I moved srand() initialization into main.cpp; we'll need to make our own
	//      random data management classes later.
	//srand(parent->getId());

	//Set default speed in the range of 1m/s to 1.9m/s
	//speed = 1+((double)(rand()%10))/10;
	//speed up
	//speed *= 16;
	speed = 1000;

	//Set default data for acceleration
	acc_ = 0;
	maxAcceleration = MAX_ACCELERATION;
	normalDeceleration = -maxAcceleration*0.6;
	maxDeceleration = -maxAcceleration;

	//Other basic parameters
	xPos = 0;
	yPos = 0;
	xVel = 0;
	yVel = 0;
	xAcc = 0;
	yAcc = 0;

	//assume that all the car has the same size
	length=1000;
	width=200;

	timeStep=0.1;			//assume that time step is constant
	isGoalSet = false;
	isOriginSet = false;
	angle = 0;
	inIntersection=false;
	disToCrossing = 500;


}


//Main update functionality
void sim_mob::Driver::update(frame_t frameNumber)
{
	if(!isOriginSet){
		setOrigin();
		isOriginSet=true;
		setToParent();
	}
	if(!isGoalSet){
		setGoal();
		isGoalSet = true;
	}

	getFromParent();
	//still need to be modified.
	//if reach the goal, get back to the origin
	if(isGoalReached()){
		//TODO:reach destination
		return;
	}


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
		if(isEnterIntersection())
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
//
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
					crossRSinCurrLink();
				xVel_ = speed;
				yVel_ = 0;
				xPos_ += xVel_*timeStep;
				yPos_ = 0;
				relat2abs();
			}
			else
			{
				updatePosition();
				relat2abs();
			}
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
	                        <<"\"xPos\":\""<<(int)parent->xPos.get()
	                        <<"\",\"yPos\":\""<<(int)parent->yPos.get()
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

void sim_mob::Driver::crossRSinCurrLink()
{
	RSIndex ++;
	currLane_ = roadSegments->at(RSIndex)->getLanes().at(0);
	updateCurrInfo(1);
//	std::cout<<"RS changed"<<std::endl;
//	targetLane.erase(targetLane.begin(),targetLane.end());
//	if(isForward)
//			currNode = currRoadSegment_->getEnd();
//		else
//			currNode = currRoadSegment_->getStart();
//	const MultiNode* mNode=dynamic_cast<const MultiNode*>(currNode);
//	if(mNode){			//end of link is a intersection
//		set<LaneConnector*>::const_iterator i;
//		set<LaneConnector*> lcs=mNode->getOutgoingLanes(*currRoadSegment_);
//		for(i=lcs.begin();i!=lcs.end();i++){
//			if((*i)->getLaneTo()->getRoadSegment()->getLink()==currLink_
//					&& (*i)->getLaneFrom()==currLane_){
//				targetLane.push_back((*i)->getLaneFrom());
//			}
//		}
//		if(targetLane.size()>0)
//		{
//			currLane_ = targetLane.at(0);
//			updateCurrInfo(1);
//		}
//		return;
//	}
//
//	std::cout<<"RS:start "<<currRoadSegment_->getStart()->location->getX()<<" "<<currRoadSegment_->getStart()->location->getY()<<std::endl;
//	std::cout<<"RS:end "<<currRoadSegment_->getEnd()->location->getX()<<" "<<currRoadSegment_->getEnd()->location->getY()<<std::endl;
//
//	std::cout<<currLane_->getPolyline().at(0).getX()<<" "<<currLane_->getPolyline().at(0).getY()<<std::endl;
//	std::cout<<currLane_->getPolyline().at(1).getX()<<" "<<currLane_->getPolyline().at(1).getY()<<std::endl;
//
//	for(int i=0;i<currLanePolyLine->size();i++)
//				std::cout<<"lane polyline "<<currLanePolyLine->at(i).getX()<<" "<<currLanePolyLine->at(i).getY()<<std::endl;
//
//			for(int i=0;i<currRoadSegment_->polyline.size();i++)
//					std::cout<<"road polyline "<<currRoadSegment_->polyline.at(i).getX()<<" "<<currRoadSegment_->polyline.at(i).getY()<<std::endl;
//
//	//when end node of current road segment is a uninode
//	const UniNode* uNode=dynamic_cast<const UniNode*>(currNode);
//	if(uNode){
//		std::cout<<"uninode "<<uNode->location->getX()<<" "<<uNode->location->getY()<<std::endl;
//		currLane_ = uNode->getOutgoingLane(*currLane_);
//		updateCurrInfo(1);
//	}
//
//		for(int i=0;i<currLanePolyLine->size();i++)
//			std::cout<<"lane polyline "<<currLanePolyLine->at(i).getX()<<" "<<currLanePolyLine->at(i).getY()<<std::endl;
//
//		for(int i=0;i<currRoadSegment_->polyline.size();i++)
//				std::cout<<"road polyline "<<currRoadSegment_->polyline.at(i).getX()<<" "<<currRoadSegment_->polyline.at(i).getY()<<std::endl;
//
//	std::cout<<"Curr "<<uNode->location->getX()<<" "<<uNode->location->getY()<<std::endl;
}

//when current lane has been changed, update current information.
//mode 0: within same RS, for lane changing model
//mode 1: during RS changing, but in the same link
//mode 2: during crossing intersection
void sim_mob::Driver::updateCurrInfo(unsigned int mode)
{
	currLaneID = currLane_->getLaneID();
	currRoadSegment_ = currLane_->getRoadSegment();
	currLanePolyLine = &(currLane_->getPolyline());

	startIndex = 0;
	endIndex = 1;

	switch(mode)
	{
	case 0:
		//TODO:
		//need to specify start index and end index
		break;
	case 1:
		currPolyLineSegStart = currLanePolyLine->at(startIndex);
		currPolyLineSegEnd = currLanePolyLine->at(endIndex);
		findCrossing();
		break;
	case 2:
		currPolyLineSegStart = currLanePolyLine->at(startIndex);
		currPolyLineSegEnd = currLanePolyLine->at(endIndex);
		currLink_ = currRoadSegment_->getLink();
		linkIndex ++;
		RSIndex = 0;
		roadSegments = &(currLink_->getPath(isForward));
		findCrossing();
		break;
	default:
		break;
	}
	abs_relat();
	//abs2relat();//velocity should be updated separately
}

bool sim_mob::Driver::isReachLastPolyLineSeg()
{
	return endIndex == currLanePolyLine->size()-1;
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
	if(isForward)
	{
		if(currLink_->getEnd()==linkPath.at(linkIndex+1)->getStart())
			isForward = true;
		else
			isForward = false;
	}
	else
	{
		if(currLink_->getStart()==linkPath.at(linkIndex+1)->getStart())
			isForward = true;
		else
			isForward = false;
	}


//	if(targetLane.size()>=1)
//	{
		nextLane_ = linkPath[linkIndex+1]->getPath(isForward).at(0)->getLanes().at(0);
		entryPoint = nextLane_->getPolyline().at(0);

		double xDir = entryPoint.getX() - xPos;
		double yDir = entryPoint.getY() - yPos;
		disToEntryPoint = sqrt(xDir*xDir+yDir*yDir);
		xDirection_entryPoint = xDir/disToEntryPoint;
		yDirection_entryPoint = yDir/disToEntryPoint;
		xTurningStart = xPos;
		yTurningStart = yPos;
	//}
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

	//boost::mutex::scoped_lock local_lock(BufferedBase::global_mutex);
	//std::cout <<start->location->getX() <<"," <<start->location->getY() <<" => " <<end->location->getX() <<"," <<end->location->getY() <<"\n";

	return nullptr;
}

void sim_mob::Driver::setOrigin()
{
	originNode = parent->originNode;
	const MultiNode* multiOriginNode=dynamic_cast<const MultiNode*>(originNode);
	const MultiNode* end = dynamic_cast<const MultiNode*>(ConfigParams::GetInstance().getNetwork().locateNode(Point2D(37250760,14355120), true));
	if(multiOriginNode)
	{
		currLink_ = findLink(multiOriginNode,end);
		if (!currLink_) {
			return;
		}
		if(currLink_->getEnd()==end)
			isForward = true;
		else
			isForward = false;
	}

	roadSegments = &(currLink_->getPath(isForward));
	RSIndex = 0;
	currRoadSegment_ = roadSegments->at(RSIndex);
	currLane_ = currRoadSegment_->getLanes().at(0);
	startIndex = 0;
	endIndex = 1;

	currLaneID = currLane_->getLaneID();
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

//	for(int i=0;i<currLanePolyLine->size();i++)
//		std::cout<<"lane polyline "<<currLanePolyLine->at(i).getX()<<" "<<currLanePolyLine->at(i).getY()<<std::endl;
//
//	for(int i=0;i<currRoadSegment_->polyline.size();i++)
//			std::cout<<"road polyline "<<currRoadSegment_->polyline.at(i).getX()<<" "<<currRoadSegment_->polyline.at(i).getY()<<std::endl;
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
		//maybe need more accurate calculation
		double xOffset=(far1.getX()+far2.getX())/2-currPolyLineSegStart.getX();
		double yOffset=(far1.getY()+far2.getY())/2-currPolyLineSegStart.getY();
		xPosCrossing_ = xOffset*xDirection+yOffset*yDirection;
		isCrossingAhead = true;
	}
	else
		isCrossingAhead = false;
}

bool sim_mob::Driver::isCloseToCrossing()
{
	return (isCrossingAhead && xPosCrossing_-xPos_-length/2 <= disToCrossing);
}


//TODO: need to check current link is the destination link
bool sim_mob::Driver::isGoalReached()
{
	return (linkIndex == linkPath.size()-1 && isReachLastRS() && isReachLastPolyLineSeg() && isReachPolyLineSegEnd());
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

bool sim_mob::Driver::isEnterIntersection()
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
	xAcc_ = acc_;
	yAcc_ = 0;
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
		xPos_ = xPos_+xVel_*timeStep+0.5*xAcc_*timeStep*timeStep;
	}
}
//
//
//void sim_mob::Driver::updateLeadingDriver()
//{
//	/*
//	 * In fact the so-called big brother can return the leading driver.
//	 * Since now there is no such big brother, so I assume that each driver can know information of other vehicles.
//	 * It will find it's leading vehicle itself.
//	 * */
//	Agent* other = nullptr;
//	double leadingDistance=MAX_NUM;
//	size_t leadingID=Agent::all_agents.size();
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
//
//		double other_xOffset	= other->xPos.get()	- testLinks[currentLink].startX;
//		double other_yOffset	= other->yPos.get()	- testLinks[currentLink].startY;
//		double other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
//		double other_yPos_		=-other_xOffset	* yDirection	+ other_yOffset	* xDirection;
//
//		//Check. Search all the vehicle it may get crashed and find the closest one ahead.
//		if(other_yPos_ < yPos_+width && other_yPos_ > yPos_-width){
//			double tmpLeadingDistance=other_xPos_-xPos_;
//			if(tmpLeadingDistance<leadingDistance && tmpLeadingDistance >0)	{
//				leadingDistance=tmpLeadingDistance;
//				leadingID=i;
//			}
//		}
//	}
//
//	if(leadingID == Agent::all_agents.size()) {
//		leader=nullptr;
//	} else {
//		leader=Agent::all_agents[leadingID];
//		double xOffset	= leader->xPos.get()	- testLinks[currentLink].startX;
//		double yOffset	= leader->yPos.get()	- testLinks[currentLink].startY;
//		leader_xPos_	= xOffset	* xDirection	+ yOffset	* yDirection;
//		leader_yPos_	=-xOffset	* yDirection	+ yOffset	* xDirection;
//		leader_xVel_	= leader->xVel.get()	* xDirection	+ leader->yVel.get()	* yDirection;
//		leader_yVel_	=-leader->xVel.get()	* yDirection	+ leader->yVel.get()	* xDirection;
//		leader_xAcc_	= leader->xAcc.get()	* xDirection	+ leader->yAcc.get()	* yDirection;
//		leader_yAcc_	=-leader->xAcc.get()	* yDirection	+ leader->yAcc.get()	* xDirection;
//	}
//}
//
//int sim_mob::Driver::getLane()
//{
//	//This function will be part of the big brother, so this is just a simple temporary function for test.
//	//Later, it may be replaced by function of the big brother.
//	for (int i=0;i<3;i++){
//		if(yPos_==-testLinks[currentLink].laneWidth*((double)i+0.5)) {
//			return i;
//		}
//	}
//	return -1;
//}
//*/
//double sim_mob::Driver::getLinkLength()
//{
//	double dx=testLinks[currentLink].endX-testLinks[currentLink].startX;
//	double dy=testLinks[currentLink].endY-testLinks[currentLink].startY;
//	return sqrt(dx*dx+dy*dy);
//}


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
	xPos = entryPoint.getX();
	yPos = entryPoint.getY();
	currLane_ = nextLane_;
	updateCurrInfo(2);
	xVel_ = speed;
	yVel_ = 0;
	xPos_ = 0;
	yPos_ = 0;
	xAcc_ = 0;
	yAcc_ = 0;
	relat2abs();


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
