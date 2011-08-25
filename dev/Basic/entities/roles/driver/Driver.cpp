/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Driver.cpp
 *
 *  Created on: 2011-7-5
 *      Author: wangxy
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

const badArea sim_mob::Driver::badareas[] = {
		//{200,350,0},
		//{750,800,1},
		//{200,350,2}
};

const link_ sim_mob::Driver::testLinks[] = {
		//ID	startX		startY		endX		endY		laneWidth	laneNum
		/*{ 0,	 372073.27,	 143216.05,	 372183.51,	 143352.55,	10,			3},
		{ 1, 	 372271.39,	 143278.75,	 372183.51,	 143352.55,	10,			3},
		{ 2,	 372287.14,	 143477.88,	 372183.51,	 143352.55,	10,			3},
		{ 3, 	 372096.05,	 143429.34,	 372183.51,	 143352.55,	10,			3},
		{ 4, 	 372183.51,	 143352.55,	 372073.27,	 143216.05,	10,			3},
		{ 5,	 372183.51,	 143352.55,	 372271.39,	 143278.75,	10,			3},
		{ 6, 	 372183.51,	 143352.55,	 372287.14,	 143477.88,	10,			3},
		{ 7, 	 372183.51,	 143352.55,	 372096.05,	 143429.34,	10,			3}*/
		{ 0,   0, 270, 460, 270,10,3},
		{ 1, 530,   0, 530, 260,10,3},
		{ 2,1000, 330, 540, 330,10,3},
		{ 3, 470, 600, 470, 340,10,3},
		{ 4, 460, 330,   0, 330,10,3},
		{ 5, 470, 260, 470,   0,10,3},
		{ 6, 540, 270,1000, 270,10,3},
		{ 7, 530, 340, 530, 600,10,3}
};

//initiate
sim_mob::Driver::Driver(Agent* parent) : Role(parent), leader(nullptr)
{
	//Set random seed
	//NOTE: This will reset the sequence returned by rand(); it's not a good idea.
	//      I moved srand() initialization into main.cpp; we'll need to make our own
	//      random data management classes later.
	//srand(parent->getId());

	//Set default speed in the range of 1m/s to 1.4m/s
	speed_ = 1+((double)(rand()%10))/10;
	//speed up
	speed_ *= 16;

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
	length=12;
	width=8;

	targetSpeed=speed_*2;
	satisfiedDistance=100;
	avoidBadAreaDistance=1.5*satisfiedDistance;
	dis2stop=MAX_NUM;

	timeStep=0.1;			//assume that time step is constant
	isGoalSet = false;
	isOriginSet = false;
	LF=nullptr;LB=nullptr;RF=nullptr;RB=nullptr;

	//specify the link
	currentLink = (parent->getId()/3)%4;
	/*if(currentLink==0)currentLink=3;
	if(currentLink==2)currentLink=0;
	if(currentLink==3)currentLink=2;*/

	nextLane=-1;
	nextLink=((parent->getId()+1)%3+currentLink+5)%4+4;

	ischanging=false;
	isback=false;
	isWaiting=false;
	fromLane=getLane();
	toLane=getLane();
	changeDecision=0;

	angle = 0;
	inIntersection=false;

	trafficSignal=nullptr;


}


//Main update functionality
void sim_mob::Driver::update(frame_t frameNumber)
{
	getFromParent();
	updateCurrentLink();
	abs2relat();
	updateCurrentLane();

	//Set the goal of agent
	if(!isGoalSet){
		setGoal();
		isGoalSet = true;
	}
	//Set the origin of agent
	if(!isOriginSet){
		setOrigin();
		isOriginSet=true;
	}

	updateTrafficSignal();

	//if reach the goal, get back to the origin
	if(isGoalReached()){
		currentLink=originLink;
		//currentLink=(currentLink+1)%8;
		//double fallback=0;
		xPos_=origin.getX();
		yPos_=origin.getY();
		nextLane=-1;
		nextLink=((parent->getId()+1)%3+currentLink+5)%4+4;
		//isGoalSet=false;
		relat2abs();
		setToParent();
		return;
	}

	if(isInTheIntersection()){
		IntersectionVelocityUpdate();
		xPos = xPos + xVel*timeStep;
		yPos = yPos + yVel*timeStep;
		modifyPosition();
	}
	else {
		//To check if the vehicle reaches the lane it wants to move to
		if(getLane()==toLane){
			ischanging=false;
			isWaiting=false;
			isback=false;
			fromLane=toLane=getLane();
			changeDecision=0;
		}
		VelOfLaneChanging=	testLinks[currentLink].laneWidth/5;		//assume that 5 time steps is need when changing lane

		//update information
		updateLeadingDriver();
		if(xPos_ > 10.){		//at the beginning of the link,no lane changing
			excuteLaneChanging();
		}
		//accelerating part
		makeAcceleratingDecision();
		//update
		updateAcceleration();
		updateVelocity();
		updatePosition();
		modifyPosition();
		relat2abs();
	}

	setToParent();

	updateAngle();

//	boost::mutex::scoped_lock local_lock(BufferedBase::global_mutex);
//	std::cout <<"("
//			<<parent->getId()
//			<<"," <<frameNumber
//			<<"," <<parent->xPos.get()
//			<<"," <<parent->yPos.get()
//			<<"," <<trafficSignal->getcurrPhase()
//			<<"," <<"0.95"
//			<<"," <<floor(trafficSignal->getnextCL())
//			<<"," <<trafficSignal->getphaseCounter()
//			<<"," <<angle
//			<<")"<<std::endl;
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
	//parent->currentLink.set(currentLink);
}

void sim_mob::Driver::abs2relat()
{
	double xDir=testLinks[currentLink].endX-testLinks[currentLink].startX;
	double yDir=testLinks[currentLink].endY-testLinks[currentLink].startY;
	double xOffset=xPos-testLinks[currentLink].startX;
	double yOffset=yPos-testLinks[currentLink].startY;
	double magnitude=sqrt(xDir*xDir+yDir*yDir);
	xDirection=xDir/magnitude;
	yDirection=yDir/magnitude;
	xPos_= xOffset*xDirection+yOffset*yDirection;
	yPos_=-xOffset*yDirection+yOffset*xDirection;
	xVel_= xVel*xDirection+yVel*yDirection;
	yVel_=-xVel*yDirection+yVel*xDirection;
	xAcc_= xAcc*xDirection+yAcc*yDirection;
	yAcc_=-xAcc*yDirection+yAcc*xDirection;
}

void sim_mob::Driver::relat2abs()
{
	double xDir=testLinks[currentLink].endX-testLinks[currentLink].startX;
	double yDir=testLinks[currentLink].endY-testLinks[currentLink].startY;
	double magnitude=sqrt(xDir*xDir+yDir*yDir);
	xDirection=xDir/magnitude;
	yDirection=yDir/magnitude;
	xPos=xPos_*xDirection-yPos_*yDirection+testLinks[currentLink].startX;
	yPos=xPos_*yDirection+yPos_*xDirection+testLinks[currentLink].startY;
	xVel=xVel_*xDirection-yVel_*yDirection;
	yVel=xVel_*yDirection+yVel_*xDirection;
	xAcc=xAcc_*xDirection-yAcc_*yDirection;
	yAcc=xAcc_*yDirection+yAcc_*xDirection;
}

void sim_mob::Driver::road2Map(){
	Point2D start=currRoadSegment_->getLanePolyline(currLane_->getLaneID())[polylineIndex_];
	Point2D end=currRoadSegment_->getLanePolyline(currLane_->getLaneID())[polylineIndex_+1];
	double lengthOfPolyline=sqrt((start.getX()-end.getX())*(start.getX()-end.getX())
			+(start.getY()-end.getY())*(start.getY()-end.getY()));
	double persentage=offsetInPolyline_/lengthOfPolyline;
	xPos=start.getX()+persentage*(end.getX()-start.getX());
	yPos=start.getY()+persentage*(end.getY()-start.getY());
}

void sim_mob::Driver::updateCurrentPositionInRoadNetwork(){		//TODO:road network problems
	Point2D start=currRoadSegment_->getLanePolyline(currLane_->getLaneID())[polylineIndex_];
	Point2D end=currRoadSegment_->getLanePolyline(currLane_->getLaneID())[polylineIndex_+1];
	double lengthOfPolyline=sqrt((start.getX()-end.getX())*(start.getX()-end.getX())
			+(start.getY()-end.getY())*(start.getY()-end.getY()));
	offsetInPolyline_+=moveAlong;
	while(offsetInPolyline_>=lengthOfPolyline){
		offsetInPolyline_ -= lengthOfPolyline;
		if(currRoadSegment_->getLanePolyline(currLane_->getLaneID()).size()==polylineIndex_+2)//the last polyline
		{
			if(dynamic_cast<const MultiNode*>(currRoadSegment_->getEnd())){	//is the last road segment
				if(currLink==linkPath.back()) {		//is the last link in the link path
					//set the flag, end the road trip
					isReachGoal=true;
					return;
				}
				else{								//still has links to go
					//get to the next link, the first road segment,the first segment of polyline
					currLink_=linkPath[linkIndex+1];
					linkIndex++;
					currRoadSegment_=currLink_->getPath(true)[0];
					roadSegmentIndex=0;
					polylineIndex_=0;
				}
			}
			else{			//not the last road segment
				//get to next road segment
				currRoadSegment_=currLink_->getPath(true)[roadSegmentIndex+1];
				roadSegmentIndex++;
				polylineIndex_=0;
			}
		}
		else{		//not the last polyline segment
			//get to next polyline segment
			polylineIndex_++;
		}
		start=currRoadSegment_->getLanePolyline(currLane_->getLaneID())[polylineIndex_];
		end=currRoadSegment_->getLanePolyline(currLane_->getLaneID())[polylineIndex_+1];
		lengthOfPolyline=sqrt((start.getX()-end.getX())*(start.getX()-end.getX())
				+(start.getY()-end.getY())*(start.getY()-end.getY()));
		offsetInPolyline_+=moveAlong;
	}
	//set to buffer data
	currLink.set(currLink_);
	currRoadSegment.set(currRoadSegment_);
	currLane.set(currLane_);
	polylineIndex.set(polylineIndex_);
	offsetInPolyline.set(offsetInPolyline_);
}

void sim_mob::Driver::getTargetLane(Link* nextlink)
{
	targetLane.erase(targetLane.begin(),targetLane.end());
	const MultiNode* mNode=dynamic_cast<const MultiNode*>(currLink_->getEnd());
	if(mNode){			//end of link is a intersection
		if(linkIndex==linkPath.size()){		//last link, no lane changing is needed
			return;
		}
		Link* nextLink=linkPath[linkIndex+1];
		set<LaneConnector*>::const_iterator i;
		set<LaneConnector*> lcs=mNode->getOutgoingLanes(*currRoadSegment_);
		for(i=lcs.begin();i!=lcs.end();i++){
			if((*i)->getLaneTo()->getRoadSegment()->getLink()==nextLink
					&& (*i)->getLaneFrom()->getRoadSegment()==currRoadSegment_){
				targetLane.push_back((*i)->getLaneFrom());
			}
		}
	}
}


void sim_mob::Driver::setOrigin()
{
	originLink = currentLink;
	origin = Point2D(0, yPos_);
}

void sim_mob::Driver::setGoal()
{
	if(currentLink%2==0) {
		goal = Point2D(460, 0);			//all the cars move in x direction to reach the goal
	} else{
		goal = Point2D(260, 0);
	}
}

bool sim_mob::Driver::isGoalReached()
{
	return (xPos < 0 || xPos > 1000 || yPos < 0 || yPos >600);
}

bool sim_mob::Driver::isReachSignal()
{
	return (!isInTheIntersection() && getLinkLength()-xPos_ < length
			&& currentLink<4);
}

void sim_mob::Driver::updateCurrentLink()
{
	for(int i=0;i<numOfLinks;i++){
		if(isOnTheLink(i)){
			currentLink=i;
		}
	}
}

bool sim_mob::Driver::isInTheIntersection()
{
	return (parent->xPos.get()>460 && parent->xPos.get() < 540
				&& parent->yPos.get() > 260 && parent->yPos.get() < 340);
}

void sim_mob::Driver::updateCurrentLane()
{
	if(!isInTheIntersection()){
		currentLane=getLane();
	}
}

bool sim_mob::Driver::isOnTheLink(int linkid)
{
	double xDir=testLinks[linkid].endX-testLinks[linkid].startX;
	double yDir=testLinks[linkid].endY-testLinks[linkid].startY;
	double xOffset=xPos-testLinks[linkid].startX;
	double yOffset=yPos-testLinks[linkid].startY;
	double magnitude=sqrt(xDir*xDir+yDir*yDir);
	double xD=xDir/magnitude;
	double yD=yDir/magnitude;
	double xP= xOffset*xD+yOffset*yD;
	double yP=-xOffset*yD+yOffset*xD;
	if(xP>=0 && xP <= magnitude && yP >= 0
			&& yP <=testLinks[linkid].laneWidth*((double)testLinks[linkid].laneNum-1)){
			return true;
	} else {
		return false;
	}
}

void sim_mob::Driver::updateAcceleration()
{
	//Set actual acceleration
	xAcc_ = acc_;
	yAcc_ = 0;
}


void sim_mob::Driver::updateVelocity()
{
	if(isReachSignal()){
		//nextLink=(currentLane+currentLink+1)%4+4;
		//updateTrafficSignal();
		nextLane=currentLane;
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
			xVel_ = max(0.0, speed_+xAcc_*timeStep);
			yVel_ = 0;//max(0.0, yDirection*speed_+yAcc*timeStep);
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
	speed_=sqrt(xVel_*xVel_+yVel_*yVel_);
}


void sim_mob::Driver::updatePosition()
{
	if(xVel_!=0) { //Only update if velocity is non-zero.
		xPos_ = xPos_+xVel_*timeStep+0.5*xAcc_*timeStep*timeStep;
	}
}


void sim_mob::Driver::updateLeadingDriver()
{
	/*
	 * In fact the so-called big brother can return the leading driver.
	 * Since now there is no such big brother, so I assume that each driver can know information of other vehicles.
	 * It will find it's leading vehicle itself.
	 * */
	Agent* other = nullptr;
	double leadingDistance=MAX_NUM;
	size_t leadingID=Agent::all_agents.size();
	for (size_t i=0; i<Agent::all_agents.size(); i++) {
		//Skip self
		other = Agent::all_agents[i];
		Person* p = dynamic_cast<Person*>(other);
		if (!p) {
			continue;
		}
		Driver* d = dynamic_cast<Driver*>(p->getRole());
		if (other->getId()==parent->getId()
				|| !d || d->getLink()!=currentLink)
		{
			continue;
		}

		double other_xOffset	= other->xPos.get()	- testLinks[currentLink].startX;
		double other_yOffset	= other->yPos.get()	- testLinks[currentLink].startY;
		double other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
		double other_yPos_		=-other_xOffset	* yDirection	+ other_yOffset	* xDirection;

		//Check. Search all the vehicle it may get crashed and find the closest one ahead.
		if(other_yPos_ < yPos_+width && other_yPos_ > yPos_-width){
			double tmpLeadingDistance=other_xPos_-xPos_;
			if(tmpLeadingDistance<leadingDistance && tmpLeadingDistance >0)	{
				leadingDistance=tmpLeadingDistance;
				leadingID=i;
			}
		}
	}

	if(leadingID == Agent::all_agents.size()) {
		leader=nullptr;
	} else {
		leader=Agent::all_agents[leadingID];
		double xOffset	= leader->xPos.get()	- testLinks[currentLink].startX;
		double yOffset	= leader->yPos.get()	- testLinks[currentLink].startY;
		leader_xPos_	= xOffset	* xDirection	+ yOffset	* yDirection;
		leader_yPos_	=-xOffset	* yDirection	+ yOffset	* xDirection;
		leader_xVel_	= leader->xVel.get()	* xDirection	+ leader->yVel.get()	* yDirection;
		leader_yVel_	=-leader->xVel.get()	* yDirection	+ leader->yVel.get()	* xDirection;
		leader_xAcc_	= leader->xAcc.get()	* xDirection	+ leader->yAcc.get()	* yDirection;
		leader_yAcc_	=-leader->xAcc.get()	* yDirection	+ leader->yAcc.get()	* xDirection;
	}
}

int sim_mob::Driver::getLane()
{
	//This function will be part of the big brother, so this is just a simple temporary function for test.
	//Later, it may be replaced by function of the big brother.
	for (int i=0;i<3;i++){
		if(yPos_==testLinks[currentLink].laneWidth*i) {
			return i;
		}
	}
	return -1;
}

double sim_mob::Driver::getLinkLength()
{
	double dx=testLinks[currentLink].endX-testLinks[currentLink].startX;
	double dy=testLinks[currentLink].endY-testLinks[currentLink].startY;
	return sqrt(dx*dx+dy*dy);
}

int sim_mob::Driver::checkIfBadAreaAhead()
{
	double disToBA=MAX_NUM;
	int BA=-1;
	// First, find the closest bad area ahead that the vehicle may knock on
	// If there is no such bad area, BA will be -1. Else, BA will be location of the bad area in the array.
	for(int i=0;i<numOfBadAreas;i++){
		if(
			yPos_ > testLinks[currentLink].laneWidth*badareas[i].lane-testLinks[currentLink].laneWidth/2-width/2
			&& yPos_ < testLinks[currentLink].laneWidth*badareas[i].lane+testLinks[currentLink].laneWidth/2+width/2
			&& badareas[i].startX > xPos_+length/2
		){
			if(badareas[i].startX - xPos_-length < disToBA)
				{disToBA=badareas[i].startX - xPos_-length/2;BA=i;}
		}
	}

	/*
	 * If the subject vehicle has no leading vehicle, it will return BA+1, the sequence number of it in the array
	 * 0 and -1 means no such bad area.
	 * If it has a leading vehicle and such bad area ahead, the distance to the leading vehicle and to the bad area.
	 * If the former one is smaller, return -(BA+2); else return (BA+1).
	 * So that return which is not 0 or -1 means there is bad area ahead,
	 *   while the positive return means bad area is closer and the sequence number will be return-1
	 *   and negative return means leading vehicle is closer and the sequence number will be -return-2.
	 * */
	if(!leader){
		return BA+1;
	} else {
		if(BA==-1) {
			return -1;
		} else {
			double temp1=badareas[BA].startX-xPos_-length/2;
			double temp2=leader_xPos_-xPos_-length;
			return (temp1<temp2)?(BA+1):-(BA+2);
		}
	}
}

double sim_mob::Driver::getDistance()
{
	int BA=checkIfBadAreaAhead();
	if( BA > 0 ) { 		//There is bad area ahead and no leading vehicle ahead.
		return badareas[BA-1].startX-xPos_-length/2;
	} else if( BA < 0 ) { 	//Leading vehicle is closer(maybe there is no bad area ahead)
		return max(0.001,leader_xPos_-xPos_-length);
	} else {				//Nothing ahead, just go as fast as it can.
		return MAX_NUM;
	}
}

Agent* sim_mob::Driver::getNextForBDriver(bool isLeft,bool isFront)
{
	int border;			//the sequence number of border lane
	double offset;		//to get the adjacent lane, yPos should minus the offset

	if(isLeft) {
		border = 0;
		offset = testLinks[currentLink].laneWidth;
	} else{
		border = 2;
		offset = -testLinks[currentLink].laneWidth;
	}

	double NFBDistance;
	if(isFront) {
		NFBDistance=MAX_NUM;
	} else {
		NFBDistance=-MAX_NUM;
	}

	size_t NFBID=Agent::all_agents.size();
	if(getLane()==border) {
		return nullptr;		//has no left side or right side
	} else {
		Agent* other = nullptr;
		for (size_t i=0; i<Agent::all_agents.size(); i++) {
			//Skip self
			other = Agent::all_agents[i];
			Person* p = dynamic_cast<Person*>(other);
			if (!p) {
				continue;
			}
			Driver* d = dynamic_cast<Driver*>(p->getRole());
			if (other->getId()==parent->getId()
					|| !d || d->getLink()!=currentLink)
			{
				continue;
			}
			//Check. Find the closest one
			double other_xOffset	= other->xPos.get()	- testLinks[currentLink].startX;
			double other_yOffset	= other->yPos.get()	- testLinks[currentLink].startY;
			double other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
			double other_yPos_		=-other_xOffset	* yDirection	+ other_yOffset	* xDirection;

			if(other_yPos_ == yPos_-offset){		//now it just searches vehicles on the lane
				double forward=other_xPos_-xPos_;
				if(
						(isFront && forward>0 && forward < NFBDistance)||
						((!isFront) && forward<0 && forward > NFBDistance)
						) {
					NFBDistance=forward;NFBID=i;
				}
			}
		}
	}

	if(NFBID == Agent::all_agents.size()) {
		return nullptr;
	} else {
		return Agent::all_agents[NFBID];
	}
}


int sim_mob::Driver::findClosestBadAreaAhead(int lane)
{
	double disToBA=MAX_NUM;
	int BA=-1;
	for(int j=0;j<numOfBadAreas;j++){
		if( lane==badareas[j].lane
				&& badareas[j].startX > xPos_ + length / 2
				&& badareas[j].startX - xPos_ - length / 2 < disToBA	){
			BA = j;
			disToBA = badareas[j].startX - xPos_ - length / 2;
			}
	}
	return BA;
}


bool sim_mob::Driver::checkIfOnTheLane(double y)
{
	for(int i=0;i<3;i++){
		if(y==testLinks[currentLink].laneWidth*i){
			return true;
		}
	}
	return false;
}

/*
 * When vehicles change lane slowly, crash may happen.
 * To avoid such crash, vehicle should be able to detect it.
 * While here are some rules that some crash will be ignored.
 * More discussion is needed.
 *
 * -wangxy
 * */
bool sim_mob::Driver::checkForCrash()
{
	// now, only vehicles changing lane check if crash may happen
	if(!ischanging) {
		return false;
	}
	//check other vehicles
	Agent* other = nullptr;
	for (size_t i=0; i<Agent::all_agents.size(); i++) {
		//Skip self
		other = Agent::all_agents[i];
		Person* p = dynamic_cast<Person*>(other);
		if (!p) {
			continue;
		}
		Driver* d = dynamic_cast<Driver*>(p->getRole());
		if (other->getId()==parent->getId()
				|| !d || d->getLink()!=currentLink)
		{
			continue;
		}
		double other_xOffset	= other->xPos.get()	- testLinks[currentLink].startX;
		double other_yOffset	= other->yPos.get()	- testLinks[currentLink].startY;
		double other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
		double other_yPos_		=-other_xOffset	* yDirection	+ other_yOffset	* xDirection;

		//Check. When other vehicle is too close to subject vehicle, crash will happen
		if(
				(other_yPos_ < yPos_+width*1.1)
				&& (other_yPos_ > yPos_-width*1.1)
				&& (other_xPos_ < xPos_+length*0.9)
				&& (other_xPos_ > xPos_-length*0.9)
				)
		{
			//but some kind of crash can be ignored. Cases below must be regard as crash.
			if(
					((fromLane>toLane && other_yPos_ < yPos_)
				||(fromLane<toLane && other_yPos_ > yPos_))
						)
			{		//the other vehicle is on the position where subject vehicle is approaching
				if(checkIfOnTheLane(other_yPos_)){		//if other vehicle is on the lane
					return true;								//subject vehicle should avoid it
				} else if(other_xPos_>xPos_){	//if both vehicle is changing lane
					if(parent->getId()<other->getId()){
						return true;		//one has bigger ID will not be affected
					}
					else{
						return false;
					}
				} else {
					return false;
				}
			} else{
				return false;
			}
		}
	}
	//check bad areas
	for(int i=0;i<numOfBadAreas;i++) {
		if(
				badareas[i].startX < xPos_-length
				&& badareas[i].endX > xPos_+length
				&& testLinks[currentLink].laneWidth*badareas[i].lane + testLinks[currentLink].laneWidth/2+width/2 > yPos_
				&& testLinks[currentLink].laneWidth*badareas[i].lane - testLinks[currentLink].laneWidth/2-width/2 < yPos_
		) {
			return true;
		}
	}
	return false;
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



bool sim_mob :: Driver :: reachSignalDecision()
{
	return trafficSignal->get_Driver_Light(currentLink,currentLane)!=1;
}

void sim_mob :: Driver :: IntersectionVelocityUpdate()
{
	double speed = 36;

	switch (currentLane) {
		case 1:
			//vehicles that are going to go straight
			if( currentLink==0) {
				xVel = speed;yVel = 0;
			} else if( currentLink==1) {
				xVel = 0;yVel = speed;
			} else if( currentLink==2) {
				xVel = -speed;yVel = 0;
			} else if( currentLink==3) {
				xVel = 0;yVel = -speed;
			}
			break;

		case 2:
			//vehicles that are going to turn right, their routes are based on circles
			if( currentLink==0) {
				//the center of circle are (450,350)
				double xD = 450- parent->xPos.get();
				double yD = 350 - parent->yPos.get();
				double magnitude = sqrt(xD*xD + yD*yD);

				double xDirection = yD/magnitude;
				double yDirection = - xD/magnitude;

				xVel = xDirection*speed*0.7;
				yVel = yDirection*speed*0.7;
			} else if( currentLink==1) {
				//the center of circle are (450,250)
				double xD = 450 - parent->xPos.get();
				double yD = 250 - parent->yPos.get();
				double magnitude = sqrt(xD*xD + yD*yD);

				double xDirection = yD/magnitude;
				double yDirection = - xD/magnitude;

				xVel = xDirection*speed*0.7;
				yVel = yDirection*speed*0.7;
			} else if( currentLink==2) {
				//the center of circle are (550,250)
				double xD = 550 - parent->xPos.get();
				double yD = 250 - parent->yPos.get();
				double magnitude = sqrt(xD*xD + yD*yD);

				double xDirection =  yD/magnitude;
				double yDirection =  - xD/magnitude;

				xVel = xDirection*speed*0.7;
				yVel = yDirection*speed*0.7;
			} else if( currentLink==3) {
				//the center of circle are (550,350)
				double xD = 550- parent->xPos.get();
				double yD = 350 - parent->yPos.get();
				double magnitude = sqrt(xD*xD + yD*yD);

				double xDirection = yD/magnitude;
				double yDirection = - xD/magnitude;

				xVel = xDirection*speed*0.7;
				yVel = yDirection*speed*0.7;
			}
			break;

		case 0:
			//Vehicles that are going to turn left
			if(currentLink==0) {
				xVel = 0.5*speed;yVel = -0.5*speed;
			} else if(currentLink==1) {
				xVel = 0.5*speed;yVel = 0.5*speed;
			} else if(currentLink==2) {
				xVel = -0.5*speed;yVel = 0.5*speed;
			} else if(currentLink==3) {
				xVel = -0.5*speed;yVel = -0.5*speed;
			}
			break;

		default:
			//Does this represent an error condition? ~Seth
			return;
	}
}

//modify vehicles' positions when they just finishing crossing intersection,
//make sure they are on one of the 3 lanes inside link.
void sim_mob::Driver::modifyPosition()
{
	//it is in the intersection last frame, but not in the intersection now
	if(!isInTheIntersection()) {
		if(inIntersection) {
			currentLink=nextLink;
			currentLane=nextLane;
			xPos_=5;
			yPos_=currentLane*testLinks[currentLink].laneWidth;
			xVel_=targetSpeed/2;
			yVel_=0;
			xAcc_=0;
			yAcc_=0;
			nextLane=-1;
			nextLink=-1;
			inIntersection=false;
		}
	} else {
		inIntersection=true;
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
