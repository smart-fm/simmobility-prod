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
#include "entities/vehicle/Vehicle.hpp"
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
using std::max;
using std::vector;
using std::set;

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
sim_mob::Driver::Driver(Agent* parent) : Role(parent), vehicle(nullptr), perceivedVelocity(reactTime, true),
	perceivedVelocityOfFwdCar(reactTime, true), perceivedDistToFwdCar(reactTime, false), currLane_(nullptr)
{
	//Set default speed in the range of 10m/s to 19m/s
	//speed = 0;//1000*(1+((double)(rand()%10))/10);

	//Set default data for acceleration
	acc_ = 0;
	maxAcceleration = MAX_ACCELERATION;
	normalDeceleration = -maxAcceleration*0.6;
	maxDeceleration = -maxAcceleration;

	//basic absolute parameters

	//basic relative parameters

	currLaneLength = 0;
	currLaneOffset = 0;


	timeStep = ConfigParams::GetInstance().baseGranMS/1000.0;
	isGoalSet = false;
	isOriginSet = false;
	inIntersection=false;
	isLaneChanging = false;
	isPedestrianAhead = false;
	isTrafficLightStop = false;
	angle = 0;
	tsStopDistance = 5000;
	nextLaneInNextLink = nullptr;
}


vector<BufferedBase*> sim_mob::Driver::getSubscriptionParams()
{
	vector<BufferedBase*> res;
	res.push_back(&(currLane_));
	res.push_back(&(currLaneOffset_));
	res.push_back(&(currLaneLength_));
	return res;
}



void sim_mob::Driver::new_update_params(UpdateParams& res)
{
	//Set to the previous known lane.
	res.currLane = currLane_.get();


	//Reset; these will be set before they are used.
	res.currSpeed = 0;
}



//Main update functionality
void sim_mob::Driver::update(frame_t frameNumber)
{
	//We create a temporary variable called currLane to hold the result of our update of the current lane.
	// We initialize it to currLane_.get(), which is the value of currLane_ last turn (or null if this is turn 1).
	//const Lane* currLane = currLane_.get();

	//NOTE: If you end up with a lot of these local variables, you might want to put them into their
	//      own struct. That way, you don't need to change the function signatures of your various update
	//      methods each time you change your variables around.
	//This approach is preferred.
	UpdateParams params;
	new_update_params(params);

	//Normal update code
	if(frameNumber<parent->startTime)
		return;
	traveledDis = 0;
	if(!isOriginSet){
		initializePath();
		if(allRoadSegments.size()==0)
			return;
		setOrigin(params);
		isOriginSet=true;
	}
	//still need to be modified.
	//if reach the goal, get back to the origin
	if(isGoalReached()){
		//TODO:reach destination
		setBackToOrigin();
		return;
	}

	//Retrieve the current time in ms
	unsigned int currTimeMS = frameNumber * ConfigParams::GetInstance().baseGranMS;

	//Update your perceptions.
	//NOTE: This should be done as perceptions arrive, but the following code kind of "mixes"
	//      input and decision-making. ~Seth

	//perceivedVelocity.delay(new Point2D(vehicle->xVel_, vehicle->yVel_), currTimeMS);
	perceivedVelocity.delay(new Point2D(vehicle->velocity.getRelX(), vehicle->velocity.getRelY()), currTimeMS);


	//perceivedVelocityOfFwdCar.delay(new Point2D(otherCarXVel, otherCarYVel), currTimeMS);
	//perceivedDistToFwdCar.delay(distToOtherCar, currTimeMS);

	//Now, retrieve your sensed velocity, distance, etc.
	if (perceivedVelocity.can_sense(currTimeMS)) {
		perceivedXVelocity_ = perceivedVelocity.sense(currTimeMS)->getX();
		perceivedYVelocity_ = perceivedVelocity.sense(currTimeMS)->getY();
	}

	//Here, you can use the "perceived" velocity to perform decision-making. Just be
	// careful about how you're saving the velocity values. ~Seth
	if (parent->getId()==0) {
		/*
		LogOut("At time " <<currTimeMS <<"ms, with a perception delay of " <<reactTime
				  <<"ms, my actual velocity is (" <<xVel <<"," <<yVel <<"), and my perceived velocity is ("
				  <<perceivedXVelocity <<"," <<perceivedYVelocity <<")\n");*/
	}


	//Note: For now, most updates cannot take place unless there is a Lane set
	if (params.currLane) {
		//perceivedXVelocity = vehicle->xVel;
		//perceivedYVelocity = vehicle->yVel;
		//abs2relat();
		updateNearbyAgents(params);
		//inside intersection
		if(inIntersection)
		{
			intersectionDriving(params);
		}
		else
		{
			//the first time vehicle pass the end of current link and enter intersection
			//if the vehicle reaches the end of the last road segment on current link
			//I assume this vehicle enters the intersection
			if(isReachLinkEnd())
			{
				inIntersection = true;
				directionIntersection();
				intersectionVelocityUpdate();
				intersectionDriving(params);
			}
			else
			{
				//the relative coordinate system is based on each polyline segment
				//so when the polyline segment has been updated, the coordinate system should also be updated
				if(isReachPolyLineSegEnd())
				{
					vehicle->xPos_ -= polylineSegLength;
					if(!isReachLastPolyLineSeg())
						updatePolyLineSeg();
					else
					{
						updateRSInCurrLink(params);
						//if can't find available lane in new road segment
						//this indicates an error
						//if(!currLane) //NOTE: We don't really want to "return"; we want to skip to the end of the function.
						//	return;
					}
				}

				if(isCloseToLinkEnd())
					trafficSignalDriving(params);
				linkDriving(params);
			}
		}
		relat2abs();
		setToParent();
		updateAngle();
	}


	//Update currLane_ with the latest computed value (from our local currLane).
	currLane_.set(params.currLane);
	currLaneOffset_.set(currLaneOffset);
	currLaneLength_.set(currLaneLength);

	output(frameNumber);
}

void sim_mob::Driver::output(frame_t frameNumber)
{
	LogOut("(\"Driver\""
			<<","<<frameNumber
			<<","<<parent->getId()
			<<",{"
			<<"\"xPos\":\""<<vehicle->xPos
			<<"\",\"yPos\":\""<<vehicle->yPos
			<<"\",\"angle\":\""<<angle
			<<"\"})"<<std::endl);
}


//responsible for vehicle behaviour inside intersection
//the movement is based on absolute position
void sim_mob::Driver::intersectionDriving(UpdateParams& p)
{
	if(isLeaveIntersection())
	{
		inIntersection = false;
		enterNextLink(p);
	}
	else
	{
		//vehicle->xPos += vehicle->xVel*timeStep;
		//vehicle->yPos += vehicle->yVel*timeStep;
		vehicle->xPos += vehicle->velocity.getAbsX()*timeStep;
		vehicle->yPos += vehicle->velocity.getAbsY()*timeStep;

		abs2relat();
	}
}

//vehicle movement on link, perform acceleration, lane changing if necessary
//the movement is based on relative position
void sim_mob::Driver::linkDriving(UpdateParams& p)
{
	if(isLaneChanging)
		updatePosLC(p);
	excuteLaneChanging();

	//if(isTrafficLightStop && vehicle->xVel_ < 50)
	if(isTrafficLightStop && vehicle->velocity.getRelX() < 50)
	{
		//vehicle->xVel_ = 0;
		vehicle->velocity.setRelX(0);

		//vehicle->xAcc_ = 0;
		vehicle->accel.setRelX(0);
	}
	else
		makeAcceleratingDecision(p);
	updateAcceleration();
	updatePositionOnLink();
}


//for buffer data, maybe need more parameters to be stored
//TODO: need to discuss
void sim_mob::Driver::setToParent()
{
	parent->xPos.set(vehicle->xPos);
	parent->yPos.set(vehicle->yPos);

	//parent->xVel.set(vehicle->xVel);
	//parent->yVel.set(vehicle->yVel);
	parent->xVel.set(vehicle->velocity.getAbsX());
	parent->yVel.set(vehicle->velocity.getAbsY());

	//parent->xAcc.set(vehicle->xAcc);
	//parent->yAcc.set(vehicle->yAcc);
	parent->xAcc.set(vehicle->accel.getAbsX());
	parent->yAcc.set(vehicle->accel.getAbsY());
}


void sim_mob::Driver::sync_relabsobjs()
{
	vehicle->velocity.changeCoords(currPolylineSegStart, currPolylineSegEnd);
	vehicle->accel.changeCoords(currPolylineSegStart, currPolylineSegEnd);
}


//calculate absolute-relative transform vectors
void sim_mob::Driver::abs_relat()
{
	double xDir = currPolylineSegEnd.getX()-currPolylineSegStart.getX();
	double yDir = currPolylineSegEnd.getY()-currPolylineSegStart.getY();
	polylineSegLength = sqrt(xDir*xDir+yDir*yDir);
	xDirection = xDir/polylineSegLength;
	yDirection = yDir/polylineSegLength;
}

//convert absolute coordinate system to relative one
void sim_mob::Driver::abs2relat()
{
	double xOffset=vehicle->xPos-currPolylineSegStart.getX();
	double yOffset=vehicle->yPos-currPolylineSegStart.getY();

	vehicle->xPos_= xOffset*xDirection+yOffset*yDirection;
	vehicle->yPos_=-xOffset*yDirection+yOffset*xDirection;

	//NOTE: This is done automatically by the RelAbsPoint class
	//vehicle->xVel_= vehicle->xVel*xDirection+vehicle->yVel*yDirection;
	//vehicle->yVel_=-vehicle->xVel*yDirection+vehicle->yVel*xDirection;

	//NOTE: This is done automatically by the RelAbsPoint class
	//vehicle->xAcc_= vehicle->xAcc*xDirection+vehicle->yAcc*yDirection;
	//vehicle->yAcc_=-vehicle->xAcc*yDirection+vehicle->yAcc*xDirection;

	//perceivedXVelocity_ = perceivedXVelocity*xDirection+perceivedYVelocity*yDirection;
	//perceivedYVelocity_ = -perceivedXVelocity*yDirection+perceivedYVelocity*xDirection;
}

//convert relative coordinate system to absolute one
void sim_mob::Driver::relat2abs()
{
	vehicle->xPos=vehicle->xPos_*xDirection-vehicle->yPos_*yDirection+currPolylineSegStart.getX();
	vehicle->yPos=vehicle->xPos_*yDirection+vehicle->yPos_*xDirection+currPolylineSegStart.getY();

	//NOTE: This is done automatically by the RelAbsPoint class
	//vehicle->xVel=vehicle->xVel_*xDirection-vehicle->yVel_*yDirection;
	//vehicle->yVel=vehicle->xVel_*yDirection+vehicle->yVel_*xDirection;

	//NOTE: This is done automatically by the RelAbsPoint class
	//vehicle->xAcc=vehicle->xAcc_*xDirection-vehicle->yAcc_*yDirection;
	//vehicle->yAcc=vehicle->xAcc_*yDirection+vehicle->yAcc_*xDirection;
}

bool sim_mob::Driver::isReachPolyLineSegEnd()
{
	if(vehicle->xPos_>=polylineSegLength)
		return true;
	else
		return false;
}

bool sim_mob::Driver::isReachLastRSinCurrLink()
{
	if(currRoadSegment == currLink->getPath(isForward).at(currLink->getPath(isForward).size()-1))
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
	return (isReachLastRSinCurrLink()&&isCrossingAhead && xPosCrossing_-vehicle->xPos_-vehicle->length/2 <= 1000);
}


//when the vehicle reaches the end of last link in link path
bool sim_mob::Driver::isGoalReached()
{
	return (RSIndex == allRoadSegments.size()-1 && isReachRoadSegmentEnd());
}

bool sim_mob::Driver::isReachRoadSegmentEnd()
{
	return isReachLastPolyLineSeg()&&isReachPolyLineSegEnd();
}

bool sim_mob::Driver::isReachLinkEnd()
{
	return (isReachLastRSinCurrLink()&&isReachLastPolyLineSeg()&&isReachPolyLineSegEnd());
}

bool sim_mob::Driver::isLeaveIntersection()
{
	double currXoffset = vehicle->xPos - xTurningStart;
	double currYoffset = vehicle->yPos - yTurningStart;
	int currDisToEntrypoint = sqrt(currXoffset*currXoffset + currYoffset*currYoffset);
	return currDisToEntrypoint >= disToEntryPoint;
}

bool sim_mob::Driver::isPedetrianOnTargetCrossing()
{
	if(!trafficSignal)
		return false;
	std::map<Link const *, size_t> const linkMap = trafficSignal->links_map();
	std::map<Link const *, size_t>::const_iterator link_i;
	size_t index = 0;
	for(link_i=linkMap.begin();link_i!=linkMap.end();link_i++)
	{
		if((*link_i).first==allRoadSegments.at(RSIndex+1)->getLink())
		{
			index = (*link_i).second;
			break;
		}
	}

	std::map<Crossing const *, size_t> const crossingMap = trafficSignal->crossings_map();
	std::map<Crossing const *, size_t>::const_iterator crossing_i;
	const Crossing* crossing = nullptr;
	for(crossing_i=crossingMap.begin();crossing_i!=crossingMap.end();crossing_i++)
	{
		if((*crossing_i).second==index)
		{
			crossing = (*crossing_i).first;
			break;
		}
	}

	if(!crossing)
		return false;
	int x[4] = {crossing->farLine.first.getX(),crossing->farLine.second.getX(),crossing->nearLine.first.getX(),crossing->nearLine.second.getX()};
	int y[4] = {crossing->farLine.first.getY(),crossing->farLine.second.getY(),crossing->nearLine.first.getY(),crossing->nearLine.second.getY()};
    int xmin = x[0],xmax = x[0],ymin = y[0],ymax = y[0];
	for(int i=0;i<4;i++)
    {
		if(x[i]<xmin)
			xmin = x[i];
		if(x[i]>xmax)
			xmax = x[i];
		if(y[i]<ymin)
			ymin = y[i];
		if(y[i]>ymax)
			ymax = y[i];
    }
	Point2D p1 = Point2D(xmin,ymin);
	Point2D p2 = Point2D(xmax,ymax);

	std::vector<const Agent*> agentsInRect = AuraManager::instance().agentsInRect(p1,p2);
	for(size_t i=0;i<agentsInRect.size();i++)
	{
		const Person *person = dynamic_cast<const Person *>(agentsInRect.at(i));
		if(!person)
			continue;
		Person* p = const_cast<Person*>(person);
		Pedestrian* pedestrian = dynamic_cast<Pedestrian*>(p->getRole());
		if(pedestrian && pedestrian->isOnCrossing())
			return true;
	}
	return false;
}

void sim_mob::Driver::updateRSInCurrLink(UpdateParams& p)
{
	const Node * currNode = currRoadSegment->getEnd();
	const MultiNode* mNode=dynamic_cast<const MultiNode*>(currNode);
	if(mNode){
		set<LaneConnector*>::const_iterator i;
		set<LaneConnector*> lcs=mNode->getOutgoingLanes(*currRoadSegment);
		for(i=lcs.begin();i!=lcs.end();i++){
			if((*i)->getLaneTo()->getRoadSegment()==allRoadSegments.at(RSIndex+1)
					&& (*i)->getLaneFrom()==p.currLane){
				p.currLane = (*i)->getLaneTo();
				updateCurrInfo(1, p);
				return;
			}
		}
		p.currLane = nullptr;
		return;
	}

	//when end node of current road segment is a uninode
	const UniNode* uNode=dynamic_cast<const UniNode*>(currNode);
	if(uNode){
		if (p.currLane) { //Avoid possible null dereference
			p.currLane = uNode->getOutgoingLane(*p.currLane);
			if(p.currLane && p.currLane->getRoadSegment()==allRoadSegments.at(RSIndex+1))
				updateCurrInfo(1, p);
		}
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
size_t sim_mob::Driver::getLaneIndex(const Lane* l)
{
	const RoadSegment* r = l->getRoadSegment();
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
void sim_mob::Driver::updateCurrInfo(unsigned int mode, UpdateParams& p)
{
	currRoadSegment = p.currLane->getRoadSegment();
	currLaneIndex = getLaneIndex(p.currLane);
	updateAdjacentLanes();
	currLanePolyLine = &(p.currLane->getPolyline());
	updateCurrLaneLength();
	maxLaneSpeed = currRoadSegment->maxSpeed/3.6;//slow down
	targetSpeed = maxLaneSpeed;

	switch(mode)
	{
	case 0:
		updateStartEndIndex();

		currPolylineSegStart = currLanePolyLine->at(polylineSegIndex);
		currPolylineSegEnd = currLanePolyLine->at(polylineSegIndex+1);
		sync_relabsobjs(); //TODO: This is temporary; there should be a better way of handling the current polyline.

		break;
	case 1:
		currLaneOffset = 0;
		polylineSegIndex = 0;

		currPolylineSegStart = currLanePolyLine->at(polylineSegIndex);
		currPolylineSegEnd = currLanePolyLine->at(polylineSegIndex+1);
		sync_relabsobjs(); //TODO: This is temporary; there should be a better way of handling the current polyline.

		RSIndex ++;
		if(isReachLastRSinCurrLink())
		{
			updateTrafficSignal();
			if(currRoadSegment!=allRoadSegments.at(allRoadSegments.size()-1))
				chooseNextLaneForNextLink(p);
		}
		break;
	case 2:
		currLaneOffset = 0;
		polylineSegIndex = 0;

		currPolylineSegStart = currLanePolyLine->at(polylineSegIndex);
		currPolylineSegEnd = currLanePolyLine->at(polylineSegIndex+1);
		sync_relabsobjs(); //TODO: This is temporary; there should be a better way of handling the current polyline.

		currLink = currRoadSegment->getLink();
		targetLaneIndex = currLaneIndex;
		RSIndex ++;
		if(isReachLastRSinCurrLink())
		{
			updateTrafficSignal();
			if(currRoadSegment!=allRoadSegments.at(allRoadSegments.size()-1))
				chooseNextLaneForNextLink(p);
		}
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
	currPolylineSegStart = currLanePolyLine->at(polylineSegIndex);
	currPolylineSegEnd = currLanePolyLine->at(polylineSegIndex+1);
	sync_relabsobjs(); //TODO: This is temporary; there should be a better way of handling the current polyline.
	abs_relat();
}


//currently it just chooses the first lane from the targetLane
void sim_mob::Driver::chooseNextLaneForNextLink(UpdateParams& p)
{
	std::vector<const Lane*> targetLanes;
	const Node* currNode = currRoadSegment->getEnd();
	if(currRoadSegment->getEnd()==allRoadSegments.at(RSIndex+1)->getLink()->getStart())
		nextIsForward = true;
	else
		nextIsForward = false;

	const MultiNode* mNode=dynamic_cast<const MultiNode*>(currNode);
	if(mNode){
		set<LaneConnector*>::const_iterator i;
		set<LaneConnector*> lcs=mNode->getOutgoingLanes(*currRoadSegment);
		for(i=lcs.begin();i!=lcs.end();i++){
			if((*i)->getLaneTo()->getRoadSegment()==allRoadSegments.at(RSIndex+1)
					&& (*i)->getLaneFrom()==p.currLane){
				targetLanes.push_back((*i)->getLaneTo());
				size_t laneIndex = getLaneIndex((*i)->getLaneTo());
				//find target lane with same index, use this lane
				if(laneIndex == currLaneIndex)
				{
					nextLaneInNextLink = (*i)->getLaneTo();
					targetLaneIndex = laneIndex;
					return;
				}
			}
		}
		//no lane with same index, use the first lane in the vector
		if(targetLanes.size()>0)
		{
			nextLaneInNextLink = targetLanes.at(0);
			targetLaneIndex = getLaneIndex(nextLaneInNextLink);
		}
		else
		{
			nextLaneInNextLink = allRoadSegments.at(RSIndex+1)->getLanes().at(currLaneIndex);
			targetLaneIndex = currLaneIndex;
		}
	}
}

void sim_mob::Driver::directionIntersection()
{
	entryPoint = nextLaneInNextLink->getPolyline().at(0);
	double xDir = entryPoint.getX() - vehicle->xPos;
	double yDir = entryPoint.getY() - vehicle->yPos;
	disToEntryPoint = sqrt(xDir*xDir+yDir*yDir);
	xDirection_entryPoint = xDir/disToEntryPoint;
	yDirection_entryPoint = yDir/disToEntryPoint;
	xTurningStart = vehicle->xPos;
	yTurningStart = vehicle->yPos;
}


//TODO
void sim_mob::Driver::setBackToOrigin()
{
	vehicle->xPos = parent->originNode->location->getX();
	vehicle->yPos = parent->originNode->location->getY();

	//vehicle->xVel = 0;
	//vehicle->yVel = 0;
	vehicle->velocity.setAbs(0, 0);

	//vehicle->xAcc = 0;
	//vehicle->yAcc = 0;
	vehicle->accel.setAbs(0, 0);

	setToParent();
}


//link path should be retrieved from other class
//for now, it serves as this purpose
void sim_mob::Driver::initializePath()
{
	originNode = parent->originNode;
	destNode = parent->destNode;
	std::vector< WayPoint > a = StreetDirectory::instance().shortestDrivingPath(*originNode, *destNode);
	for(size_t i=0;i<a.size();i++)
	{
		WayPoint wp = a.at(i);
		if(wp.type_ == WayPoint::ROAD_SEGMENT)
			allRoadSegments.push_back(wp.roadSegment_);
	}
}

void sim_mob::Driver::setOrigin(UpdateParams& p)
{
	vehicle = new Vehicle();
	//Retrieve the first link in the link path vector
	RSIndex = 0;
	currRoadSegment = allRoadSegments.at(RSIndex);
	currLink = currRoadSegment->getLink();

	//decide the direction
	if(currLink->getStart()==originNode)
		isForward = true;
	else
		isForward = false;

	maxLaneSpeed = currRoadSegment->maxSpeed/3.6;//slow down
	targetSpeed = maxLaneSpeed;

	currLaneIndex = 0;
	p.currLane = currRoadSegment->getLanes().at(currLaneIndex);
	targetLaneIndex = currLaneIndex;

	if (p.currLane) { //Avoid memory corruption if null.
		polylineSegIndex = 0;
		currLanePolyLine = &(p.currLane->getPolyline());
		currPolylineSegStart = currLanePolyLine->at(polylineSegIndex);
		currPolylineSegEnd = currLanePolyLine->at(polylineSegIndex+1);
		sync_relabsobjs(); //TODO: This is temporary; there should be a better way of handling the current polyline.
	} else {
		//Assume a reasonable starting position if the current Lane is null.
		if (getParent()->originNode) {
			currPolylineSegStart = *getParent()->originNode->location;
			sync_relabsobjs(); //TODO: This is temporary; there should be a better way of handling the current polyline.
		}
	}

	//Initialise starting position
	vehicle->xPos = currPolylineSegStart.getX();
	vehicle->yPos = currPolylineSegStart.getY();
	abs_relat();
	abs2relat();

	//vehicle->xVel_=0;
	//vehicle->yVel_=0;
	vehicle->velocity.setRel(0, 0);

	//vehicle->xAcc_=0;
	//vehicle->yAcc_=0;
	vehicle->accel.setRel(0, 0);

	//perceivedXVelocity_=vehicle->xVel_;
	//perceivedYVelocity_=vehicle->yVel_;
	perceivedXVelocity_ = vehicle->velocity.getRelX();
	perceivedYVelocity_ = vehicle->velocity.getRelY();

	relat2abs();
	updateAdjacentLanes();
	updateCurrLaneLength();
}


//TODO
void sim_mob::Driver::findCrossing()
{
	const Crossing* crossing=dynamic_cast<const Crossing*>(currRoadSegment->nextObstacle(vehicle->xPos_,true).item);
	if(crossing)
	{
		Point2D far1 = crossing->farLine.first;
		Point2D far2 = crossing->farLine.second;

		double slope1, slope2;
		slope1 = (double)(far2.getY()-far1.getY())/(far2.getX()-far1.getX());
		slope2 = (double)(currPolylineSegEnd.getY()-currPolylineSegStart.getY())/
				(currPolylineSegEnd.getX()-currPolylineSegStart.getX());
		int x = (slope2*currPolylineSegStart.getX()-slope1*far1.getX()+far1.getY()-currPolylineSegStart.getY())/(slope2-slope1);
		int y = slope1*(x-far1.getX())+far1.getY();

		double xOffset=x-currPolylineSegStart.getX();
		double yOffset=y-currPolylineSegStart.getY();
		xPosCrossing_= xOffset*xDirection+yOffset*yDirection;
		isCrossingAhead = true;
	}
	else
		isCrossingAhead = false;
}

void sim_mob::Driver::updateAcceleration()
{
	//vehicle->xAcc_ = acc_ * 100;
	vehicle->accel.setRelX(acc_ * 100);
}

void sim_mob::Driver::updatePositionOnLink()
{
	//traveledDis = vehicle->xVel_*timeStep+0.5*vehicle->xAcc_*timeStep*timeStep;
	//traveledDis = vehicle->velocity.getRelX()*timeStep+0.5*vehicle->xAcc_*timeStep*timeStep;
	traveledDis = vehicle->velocity.getRelX()*timeStep+0.5*vehicle->accel.getRelX()*timeStep*timeStep;

	if(traveledDis<0) {
		traveledDis = 0;
	}

	//vehicle->xVel_ += vehicle->xAcc_*timeStep;
	//vehicle->velocity.setRelX(vehicle->velocity.getRelX() + vehicle->xAcc_*timeStep);
	vehicle->velocity.setRelX(vehicle->velocity.getRelX() + vehicle->accel.getRelX()*timeStep);

	//if(vehicle->xVel_<0)
	if (vehicle->velocity.getRelX()<0)
	{
		//vehicle->xVel_ = 0.1;
		vehicle->velocity.setRelX(0.1);

		//vehicle->xAcc_ = 0;
		vehicle->accel.setRelX(0);
	}

	vehicle->xPos_ += traveledDis;

	//vehicle->yPos_ += vehicle->yVel_*timeStep;
	vehicle->yPos_ += vehicle->velocity.getRelY()*timeStep;

	currLaneOffset += traveledDis;
}

void sim_mob::Driver::updateNearbyAgents(UpdateParams& params)
{
	isPedestrianAhead = false;
	CFD = nullptr;
	CBD = nullptr;
	LFD = nullptr;
	LBD = nullptr;
	RFD = nullptr;
	RBD = nullptr;

	Point2D myPos(vehicle->xPos,vehicle->yPos);
	distanceInFront = 2000;
	distanceBehind = 500;

	nearby_agents = AuraManager::instance().nearbyAgents(myPos, *params.currLane,  distanceInFront, distanceBehind);

	minCFDistance = 5000;
	minCBDistance = 5000;
	minLFDistance = 5000;
	minLBDistance = 5000;
	minRFDistance = 5000;
	minRBDistance = 5000;
	minPedestrianDis = 5000;

	for(size_t i=0;i<nearby_agents.size();i++)
	{
		const Person *person = dynamic_cast<const Person *>(nearby_agents.at(i));
		if(!person)
			continue;
		Person* p = const_cast<Person*>(person);
		Driver* other_driver = dynamic_cast<Driver*>(p->getRole());
		Pedestrian* pedestrian = dynamic_cast<Pedestrian*>(p->getRole());

		if(other_driver == this)
			continue;
		if(other_driver)
		{
			const Lane* other_lane = other_driver->currLane_.get();// getCurrLane();
			if(other_driver->isInIntersection())
				continue;
			const RoadSegment* otherRoadSegment = other_lane->getRoadSegment();
			int other_offset = other_driver->currLaneOffset_.get();
			//the vehicle is in the current road segment
			if(currRoadSegment == otherRoadSegment)
			{
				int distance = other_offset - currLaneOffset;
				//the vehicle is on the current lane
				if(other_lane == params.currLane)
				{
					//the vehicle is in front
					if(distance > 0)
					{
						distance = distance - vehicle->length/2 - other_driver->getVehicle()->length/2;
						if(distance <= minCFDistance)
						{
							CFD = other_driver;
							minCFDistance = distance;
						}
					}
					else
					{
						distance = - distance;
						distance = distance - vehicle->length/2 - other_driver->getVehicle()->length/2;
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
						distance = distance - vehicle->length/2 - other_driver->getVehicle()->length/2;
						if(distance <= minLFDistance)
						{
							LFD = other_driver;
							minLFDistance = distance;
						}
					}
					else
					{
						distance = - distance;
						distance = distance - vehicle->length/2 - other_driver->getVehicle()->length/2;
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
						distance = distance - vehicle->length/2 - other_driver->getVehicle()->length/2;
						if(distance <= minRFDistance)
						{
							RFD = other_driver;
							minRFDistance = distance;
						}
					}
					else if(distance < 0 && -distance <= minRBDistance)
					{
						distance = - distance;
						distance = distance - vehicle->length/2 - other_driver->getVehicle()->length/2;
						if(distance <= minRBDistance)
						{
							RBD = other_driver;
							minRBDistance = distance;
						}
					}
				}
			}
			//the vehicle is in the next road segment
			else if(RSIndex < allRoadSegments.size()-1 && otherRoadSegment == allRoadSegments.at(RSIndex+1) &&
					otherRoadSegment->getLink() == currLink)
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
					nextLane = uNode->getOutgoingLane(*params.currLane);
				}
				//make sure next lane is in the next road segment, although it should be true
				if(nextLane && nextLane->getRoadSegment() == otherRoadSegment)
				{
					//the next lane isn't the left most lane
					size_t nextLaneIndex = getLaneIndex(nextLane);
					if(nextLaneIndex>0)
						nextLeftLane = otherRoadSegment->getLanes().at(nextLaneIndex-1);
					if(nextLaneIndex < otherRoadSegment->getLanes().size()-1)
						nextRightLane = otherRoadSegment->getLanes().at(nextLaneIndex+1);
				}

				int distance = other_offset + currLaneLength - currLaneOffset -
						vehicle->length/2 - other_driver->getVehicle()->length/2;
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
			else if(otherRoadSegment == allRoadSegments.at(RSIndex - 1) &&
					otherRoadSegment->getLink() == currLink)
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
						if(uNode->getOutgoingLane(*tempLane) == params.currLane)
						{
							preLane = tempLane;
							break;
						}
					}
				}
				//make sure next lane is in the next road segment, although it should be true
				if(preLane)
				{
					size_t preLaneIndex = getLaneIndex(preLane);
					//the next lane isn't the left most lane
					if(preLaneIndex>0)
						preLeftLane = otherRoadSegment->getLanes().at(preLaneIndex-1);
					if(preLaneIndex < otherRoadSegment->getLanes().size()-1)
						preRightLane = otherRoadSegment->getLanes().at(preLaneIndex+1);
				}

				int distance = other_driver->currLaneLength_.get() - other_offset + currLaneOffset -
						vehicle->length/2 - other_driver->getVehicle()->length/2;
				//the vehicle is on the current lane
				if(other_lane == preLane)
				{
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
		else if(pedestrian&&pedestrian->isOnCrossing())
		{
			int otherX = person->xPos;
			int otherY = person->yPos;
			double xOffset=otherX-currPolylineSegStart.getX();
			double yOffset=otherY-currPolylineSegStart.getY();

			int otherX_ = xOffset*xDirection+yOffset*yDirection;
			int distance = otherX_ - vehicle->xPos_;
			if(distance>=0)
			{
				distance = distance - vehicle->length/2 - 300;
				isPedestrianAhead = true;
				if(distance < minPedestrianDis)
					minPedestrianDis = distance;//currLaneLength - currLaneOffset - vehicle->length/2 -300;
			}
		}
	}
}

//Angle shows the velocity direction of vehicles
void sim_mob::Driver::updateAngle()
{
	double xVel = vehicle->velocity.getAbsX();
	double yVel = vehicle->velocity.getAbsY();

	if(xVel==0 && yVel==0){}
    else if(xVel>=0 && yVel>=0) { angle = 360 - atan(yVel/xVel)/3.1415926*180; }
	else if(xVel>=0 && yVel<0 ) { angle = - atan(yVel/xVel)/3.1415926*180; }
	else if(xVel<0  && yVel>=0) { angle = 180 - atan(yVel/xVel)/3.1415926*180; }
	else if(xVel<0  && yVel<0 ) { angle = 180 - atan(yVel/xVel)/3.1415926*180; }
}

void sim_mob :: Driver :: intersectionVelocityUpdate()
{
	double inter_speed = 1000;//10m/s

	//vehicle->xAcc_=0;
	//vehicle->yAcc_=0;
	vehicle->accel.setRel(0, 0);

	//vehicle->xVel = inter_speed * xDirection_entryPoint;
	//vehicle->yVel = inter_speed * yDirection_entryPoint;
	vehicle->velocity.setAbs(
		inter_speed * xDirection_entryPoint,
		inter_speed * yDirection_entryPoint
	);
}

void sim_mob :: Driver :: enterNextLink(UpdateParams& p)
{
	isForward = nextIsForward;
	p.currLane = nextLaneInNextLink;
	updateCurrInfo(2, p);
	abs2relat();

	//vehicle->yVel_ = 0;
	vehicle->velocity.setRelY(0);

	vehicle->yPos_ = 0;

	//vehicle->yAcc_ = 0;
	vehicle->accel.setRelY(0);

	linkDriving(p);
	relat2abs();
}

void sim_mob::Driver::updatePosLC(UpdateParams& p)
{
	//right
	if(changeDecision == 1)
	{
		if(!lcEnterNewLane && -vehicle->yPos_>=150)//currLane->getWidth()/2.0)
		{
			p.currLane = rightLane;
			updateCurrInfo(0, p);
			abs2relat();
			lcEnterNewLane = true;
		}
		if(lcEnterNewLane && vehicle->yPos_ <=0)
		{
			isLaneChanging =false;
			vehicle->yPos_ = 0;

			//vehicle->yVel_ = 0;
			vehicle->velocity.setRelY(0);
		}
	}
	else if(changeDecision == -1)
	{
		if(!lcEnterNewLane && vehicle->yPos_>=150)//currLane->getWidth()/2.0)
		{
			p.currLane = leftLane;
			updateCurrInfo(0, p);
			abs2relat();
			lcEnterNewLane = true;
		}
		if(lcEnterNewLane && vehicle->yPos_ >=0)
		{
			isLaneChanging =false;
			vehicle->yPos_ = 0;

			//vehicle->yVel_ = 0;
			vehicle->velocity.setRelY(0);
		}
	}
}

bool sim_mob::Driver::isCloseToLinkEnd()
{
	//when the distance <= 10m
	return isReachLastRSinCurrLink()&&(currLaneLength - currLaneOffset<2000);
}

void sim_mob::Driver::updateTrafficSignal()
{
	const Node* node = currRoadSegment->getEnd();
	if(node)
		trafficSignal = StreetDirectory::instance().signalAt(*node);
	else
		trafficSignal = nullptr;
}

void sim_mob::Driver::pedestrianAheadDriving()
{
	if(perceivedXVelocity_>0) {
		//vehicle->xAcc_ = -0.5*perceivedXVelocity_*perceivedXVelocity_/(0.5*minPedestrianDis);
		//make sure the vehicle can stop before pedestrian, so distance should be shorter, now I use 0.5*dis
		vehicle->accel.setRelX(-0.5*perceivedXVelocity_*perceivedXVelocity_/(0.5*minPedestrianDis));
	} else {
		//vehicle->xAcc_ = 0;
		vehicle->accel.setRelX(0);

		//vehicle->xVel_ = 0;
		vehicle->velocity.setRelX(0);
	}
	updatePositionOnLink();
}

void sim_mob::Driver::trafficSignalDriving(UpdateParams& p)
{
	tsStopDistance = 5000;
	if(!trafficSignal)
	{
		isTrafficLightStop = false;
	}
	else
	{
		int color;
		if(nextLaneInNextLink)
			color = trafficSignal->getDriverLight(*p.currLane,*nextLaneInNextLink);
		else
			color = trafficSignal->getDriverLight(*p.currLane).forward;

		switch(color)
		{
		//red yellow
		case Signal::Red :case Signal::Amber:
			isTrafficLightStop = true;
			tsStopDistance = currLaneLength - currLaneOffset - vehicle->length/2 -300;
			break;
			//green
		case Signal::Green:
			if(!isPedetrianOnTargetCrossing())
				isTrafficLightStop = false;
			else
				isTrafficLightStop = true;
			break;
		}
	}
}

