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
#include "util/DynamicVector.hpp"


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


//Helper functions
namespace {

//used in lane changing, find the start index and end index of polyline in the target lane
size_t updateStartEndIndex(const std::vector<sim_mob::Point2D>* const currLanePolyLine, double currLaneOffset, size_t defaultValue)
{
	double offset = 0;
	for(size_t i=0;i<currLanePolyLine->size()-1;i++)
	{
		double xOffset = currLanePolyLine->at(i+1).getX() - currLanePolyLine->at(i).getX();
		double yOffset = currLanePolyLine->at(i+1).getY() - currLanePolyLine->at(i).getY();
		offset += sqrt(xOffset*xOffset + yOffset*yOffset);
		if(offset>=currLaneOffset)
		{
			return i;
		}
	}
	return defaultValue;
}

} //End anon namespace



//initiate
sim_mob::Driver::Driver(Agent* parent) : Role(parent), vehicle(nullptr), perceivedVelocity(reactTime, true),
	perceivedVelocityOfFwdCar(reactTime, true), perceivedDistToFwdCar(reactTime, false),
	distanceInFront(2000), distanceBehind(500), currLane_(nullptr), currLaneOffset_(0), currLaneLength_(0), isInIntersection(false)
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


	timeStep = ConfigParams::GetInstance().baseGranMS/1000.0;
	firstFrameTick = true;
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
	res.push_back(&(isInIntersection));
	return res;
}



void sim_mob::Driver::new_update_params(UpdateParams& res)
{
	//Set to the previous known buffered values
	res.currLane = currLane_.get();
	res.currLaneLength = currLaneLength_.get();
	res.currLaneOffset = currLaneOffset_.get();
	res.isInIntersection = isInIntersection.get();

	//Current lanes to the left and right. May be null
	res.leftLane = nullptr;
	res.rightLane = nullptr;

	//Reset; these will be set before they are used.
	res.currSpeed = 0;
	res.perceivedFwdVelocity = 0;
	res.perceivedLatVelocity = 0;

	//Nearest vehicles in the current lane, and left/right (including fwd/back for each).
	res.nvFwd.driver = nullptr;
	res.nvBack.driver = nullptr;
	res.nvLeftFwd.driver = nullptr;
	res.nvLeftBack.driver = nullptr;
	res.nvRightFwd.driver = nullptr;
	res.nvRightBack.driver = nullptr;

	//Nearest vehicles' distances are initialized to threshold values.
	res.nvFwd.distance = 5000;
	res.nvBack.distance = 5000;
	res.nvLeftFwd.distance = 5000;
	res.nvLeftBack.distance = 5000;
	res.nvRightFwd.distance = 5000;
	res.nvRightBack.distance = 5000;
}



void sim_mob::Driver::update_first_frame(UpdateParams& params, frame_t frameNumber)
{
	//Save the path from orign to destination in allRoadSegments
	initializePath();

	//Set some properties about the current path, such as the current polyline, etc.
	if(pathMover.isPathSet()) {
		setOrigin(params);
	}
}



void sim_mob::Driver::update_general(UpdateParams& params, frame_t frameNumber)
{
	//Note: For now, most updates cannot take place unless there is a Lane set
	if (!params.currLane) {
		return;
	}

	//if reach the goal, get back to the origin
	if(isGoalReached()){
		//TODO:reach destination
		setBackToOrigin();
		return;
	}

	//
	updateNearbyAgents(params);


	//inside intersection
	if(params.isInIntersection) {
		intersectionDriving(params);
	} else {
		//the first time vehicle pass the end of current link and enter intersection
		//if the vehicle reaches the end of the last road segment on current link
		//I assume this vehicle enters the intersection
		if(isReachLinkEnd())
		{
			params.isInIntersection = true;
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
				//vehicle->xPos_ -= polylineSegLength;


				if(!polypathMover.isOnLastLine())
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



//Main update functionality
void sim_mob::Driver::update(frame_t frameNumber)
{
	//Do nothing?
	if(frameNumber<parent->startTime)
		return;

	//Create a new set of local parameters for this frame update.
	UpdateParams params;
	new_update_params(params);

	//First frame update
	if (firstFrameTick) {
		update_first_frame(params, frameNumber);
		firstFrameTick = false;
	}

	//Convert the current time to ms
	unsigned int currTimeMS = frameNumber * ConfigParams::GetInstance().baseGranMS;

	//Update your perceptions, and retrieved their current "sensed" values.
	perceivedVelocity.delay(new DPoint(vehicle->getVelocity(), vehicle->getLatVelocity()), currTimeMS);
	if (perceivedVelocity.can_sense(currTimeMS)) {
		params.perceivedFwdVelocity = perceivedVelocity.sense(currTimeMS)->x;
		params.perceivedLatVelocity = perceivedVelocity.sense(currTimeMS)->y;
	}

	//General update behavior.
	update_general(params, frameNumber);

	//Update our Buffered types
	currLane_.set(params.currLane);
	currLaneOffset_.set(params.currLaneOffset);
	currLaneLength_.set(params.currLaneLength);
	isInIntersection.set(params.isInIntersection);

	//Print output for this frame.
	output(frameNumber);
}

void sim_mob::Driver::output(frame_t frameNumber)
{
	LogOut("(\"Driver\""
			<<","<<frameNumber
			<<","<<parent->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(vehicle->getX())
			<<"\",\"yPos\":\""<<static_cast<int>(vehicle->getY())
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
		//TODO: Intersection driving is unlikely to work until lane driving is fixed.
		//double xComp = vehicle->velocity.getEndX() + vehicle->velocity_lat.getEndX();
		//double yComp = vehicle->velocity.getEndY() + vehicle->velocity_lat.getEndY();
		//vehicle->pos.moveFwd(xComp*timeStep);
		//vehicle->pos.moveLat(yComp*timeStep);

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

	if(isTrafficLightStop && vehicle->getVelocity() < 50)
	{
		vehicle->setVelocity(0);
		vehicle->setAcceleration(0);
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
	parent->xPos.set(vehicle->getX());
	parent->yPos.set(vehicle->getY());

	//TODO: Need to see how the parent agent uses its velocity vector.
	parent->fwdVel.set(vehicle->getVelocity());
	parent->latVel.set(vehicle->getLatVelocity());
}


//TODO: Previously, the rel/abs nature of the the code made auto-updating impossible. Now it should work,
//      but for now I'm requiring this function to be called manually. ~Seth
void sim_mob::Driver::sync_relabsobjs()
{
	vehicle->newPolyline(polypathMover.getCurrPolypoint(), polypathMover.getNextPolypoint());
}


bool sim_mob::Driver::isReachPolyLineSegEnd() const
{
	return vehicle->reachedSegmentEnd();
}

bool sim_mob::Driver::isReachLastRSinCurrLink() const
{
	return pathMover.isOnLastSegment();
}

//TODO
bool sim_mob::Driver::isCloseToCrossing() const
{
	return (isReachLastRSinCurrLink()&&isCrossingAhead && xPosCrossing_-vehicle->getX()-vehicle->length/2 <= 1000);
}


//when the vehicle reaches the end of last link in link path
bool sim_mob::Driver::isGoalReached() const
{
	return pathMover.isOnLastSegment() && isReachCurrRoadSegmentEnd();
}

bool sim_mob::Driver::isReachCurrRoadSegmentEnd() const
{
	return polypathMover.isOnLastLine() && isReachPolyLineSegEnd();
}

bool sim_mob::Driver::isReachLinkEnd() const
{
	return (isReachLastRSinCurrLink() && polypathMover.isOnLastLine() && isReachPolyLineSegEnd());
}

bool sim_mob::Driver::isLeaveIntersection() const
{
	double currXoffset = vehicle->getX() - xTurningStart;
	double currYoffset = vehicle->getY() - yTurningStart;

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
		const Person* other = dynamic_cast<const Person *>(agentsInRect.at(i));
		if(!other)
			continue;
		const Pedestrian* pedestrian = dynamic_cast<const Pedestrian*>(other->getRole());
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
				updateCurrInfo_RSChangeSameLink(p);
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
				updateCurrInfo_RSChangeSameLink(p);
		}
	}
}


//calculate current lane length
void sim_mob::Driver::updateCurrLaneLength(UpdateParams& p)
{
	p.currLaneLength = polypathMover.getCurrPolylineLength();
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
void sim_mob::Driver::updateAdjacentLanes(UpdateParams& p)
{
	p.leftLane = nullptr;
	p.rightLane = nullptr;

	if(currLaneIndex>0) {
		p.rightLane = pathMover.getCurrSegment()->getLanes().at(currLaneIndex-1);
	}
	if(currLaneIndex < currRoadSegment->getLanes().size()-1) {
		p.leftLane = pathMover.getCurrSegment()->getLanes().at(currLaneIndex+1);
	}
}




//when current lane has been changed, update current information.
//mode 0: within same RS, for lane changing model
void sim_mob::Driver::updateCurrInfo_SameRS(UpdateParams& p)
{
	updateCurrGeneralInfo(p);

	polylineSegIndex = updateStartEndIndex(currLanePolyLine, p.currLaneOffset, polylineSegIndex);

	currPolylineSegStart = currLanePolyLine->at(polylineSegIndex);
	currPolylineSegEnd = currLanePolyLine->at(polylineSegIndex+1);
	sync_relabsobjs(); //TODO: This is temporary; there should be a better way of handling the current polyline.

	abs_relat();
}


//when current lane has been changed, update current information.
//mode 1: during RS changing, but in the same link
void sim_mob::Driver::updateCurrInfo_RSChangeSameLink(UpdateParams& p)
{
	updateCurrGeneralInfo(p);

	p.currLaneOffset = 0;
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

	abs_relat();
}


//when current lane has been changed, update current information.
//mode 2: during crossing intersection
void sim_mob::Driver::updateCurrInfo_CrossIntersection(UpdateParams& p)
{
	updateCurrGeneralInfo(p);

	p.currLaneOffset = 0;
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

	abs_relat();
}





void sim_mob::Driver::updateCurrGeneralInfo(UpdateParams& p)
{
	currRoadSegment = p.currLane->getRoadSegment();
	currLaneIndex = getLaneIndex(p.currLane);
	updateAdjacentLanes();
	currLanePolyLine = &(p.currLane->getPolyline());
	updateCurrLaneLength();
	maxLaneSpeed = currRoadSegment->maxSpeed/3.6;//slow down
	targetSpeed = maxLaneSpeed;
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

	//TODO: Intersection driving won't work right for now.
	//double xDir = entryPoint.getX() - vehicle->xPos;
	//double yDir = entryPoint.getY() - vehicle->yPos;
	double xDir = entryPoint.getX() - vehicle->pos.getX();
	double yDir = entryPoint.getY() - vehicle->pos.getY();

	disToEntryPoint = sqrt(xDir*xDir+yDir*yDir);
	xDirection_entryPoint = xDir/disToEntryPoint;
	yDirection_entryPoint = yDir/disToEntryPoint;
	xTurningStart = vehicle->pos.getX();
	yTurningStart = vehicle->pos.getY();
}


//TODO
void sim_mob::Driver::setBackToOrigin()
{
	//NOTE: Right now this function isn't called anywhere interesting, so just give it a random heading.
	//TODO: This won't work.
	DynamicVector someVector(parent->originNode->location->getX(), parent->originNode->location->getY(), 1, 1);
	vehicle->pos = MovementVector(someVector);

	vehicle->velocity.scaleVectTo(0);
	vehicle->velocity_lat.scaleVectTo(0);
	vehicle->accel.scaleVectTo(0);

	setToParent();
}


//link path should be retrieved from other class
//for now, it serves as this purpose
void sim_mob::Driver::initializePath()
{
	//Save local copies of the parent's origin/destination nodes.
	originNode = parent->originNode;
	destNode = parent->destNode;

	//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
	pathMover.setPath(StreetDirectory::instance().shortestDrivingPath(*originNode, *destNode));
}

void sim_mob::Driver::setOrigin(UpdateParams& p)
{
	//A non-null vehicle means we are moving.
	vehicle = new Vehicle();

	//Determine the direction we will be moving along this link.
	isLinkForward = (pathMover.getCurrLink()->getStart()==originNode);

	//Set the max speed and target speed.
	maxLaneSpeed = pathMover.getCurrSegment()->maxSpeed/3.6;//slow down
	targetSpeed = maxLaneSpeed;

	//Set our current and target lanes.
	currLaneIndex = 0;
	p.currLane = pathMover.getCurrSegment()->getLanes().at(currLaneIndex);
	targetLaneIndex = currLaneIndex;

	//if (p.currLane) //Avoid memory corruption if null. NOTE: Probably not necessary; this function isn't called if there's no path
	polypathMover.setPath(p.currLane->getPolyline());
	sync_relabsobjs(); //TODO: This is temporary; there should be a better way of handling the current polyline.

	//Vehicles start at rest
	vehicle->setVelocity(0);
	vehicle->setLatVelocity(0);
	vehicle->setAcceleration(0);

	//Scan and save the lanes to the left and right.
	updateAdjacentLanes();

	//Calculate and save the total length of the current polyline.
	updateCurrLaneLength();
}


//TODO
void sim_mob::Driver::findCrossing()
{
	//const Crossing* crossing=dynamic_cast<const Crossing*>(currRoadSegment->nextObstacle(vehicle->xPos_,true).item);
	const Crossing* crossing=dynamic_cast<const Crossing*>(currRoadSegment->nextObstacle(vehicle->pos.getX(),true).item);

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
	//vehicle->accel.setRelX(acc_ * 100);
	vehicle->accel.scaleVectTo(acc_ * 100);
}

void sim_mob::Driver::updatePositionOnLink(UpdateParams& p)
{
	//traveledDis = vehicle->xVel_*timeStep+0.5*vehicle->xAcc_*timeStep*timeStep;
	//traveledDis = vehicle->velocity.getRelX()*timeStep+0.5*vehicle->xAcc_*timeStep*timeStep;
	//traveledDis = vehicle->velocity.getRelX()*timeStep+0.5*vehicle->accel.getRelX()*timeStep*timeStep;
	//traveledDis = vehicle->velocity.getRelX()*timeStep+0.5*vehicle->accel.getMagnitude()*timeStep*timeStep;
	traveledDis = vehicle->velocity.getMagnitude()*timeStep+0.5*vehicle->accel.getMagnitude()*timeStep*timeStep;

	if(traveledDis<0) {
		traveledDis = 0;
	}

	//vehicle->xVel_ += vehicle->xAcc_*timeStep;
	//vehicle->velocity.setRelX(vehicle->velocity.getRelX() + vehicle->xAcc_*timeStep);
	//vehicle->velocity.setRelX(vehicle->velocity.getRelX() + vehicle->accel.getRelX()*timeStep);
	//vehicle->velocity.setRelX(vehicle->velocity.getRelX() + vehicle->accel.getMagnitude()*timeStep);
	vehicle->velocity.scaleVectTo(vehicle->velocity.getMagnitude() + vehicle->accel.getMagnitude()*timeStep);

	//if(vehicle->xVel_<0)
	//if (vehicle->velocity.getRelX()<0)
	if (vehicle->velocity.getMagnitude()<0)
	{
		//vehicle->xVel_ = 0.1;
		//vehicle->velocity.setRelX(0.1);
		vehicle->velocity.scaleVectTo(0.1);

		//vehicle->xAcc_ = 0;
		//vehicle->accel.setRelX(0);
		vehicle->accel.scaleVectTo(0);
	}

	//vehicle->xPos_ += traveledDis;
	vehicle->pos.moveFwd(traveledDis);

	//vehicle->yPos_ += vehicle->yVel_*timeStep;
	//vehicle->yPos_ += vehicle->velocity.getRelY()*timeStep;
	//vehicle->yPos_ += vehicle->velocity_lat.getMagnitude()*timeStep;
	vehicle->pos.moveLat(vehicle->velocity_lat.getMagnitude()*timeStep);

	p.currLaneOffset += traveledDis;
}


namespace {
//Helper function: check if a modified distanc is less than the current minimum and save it.
void check_and_set_min_car_dist(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other) {
	//Subtract the size of the car from the distance between them
	distance = fabs(distance) - veh->length/2 - other->getVehicle()->length/2;
	if(distance <= res.distance) {
		res.driver = other;
		res.distance = distance;
	}

}
} //End anon namespace


//TODO: I have the feeling that this process of detecting nearby drivers in front of/behind you and saving them to
//      the various CFD/CBD/LFD/LBD variables can be generalized somewhat. I shortened it a little and added a
//      helper function; perhaps more cleanup can be done later? ~Seth
void sim_mob::Driver::updateNearbyDriver(UpdateParams& params, const Person* other, const Driver* other_driver)
{
	//Only update if passed a valid pointer which is not a pointer back to you, and
	//the driver is not actually in an intersection at the moment.
	if(!(other_driver && this!=other_driver && !other_driver->isInIntersection.get())) {
		return;
	}

	//Retrieve the other driver's lane, road segment, and lane offset.
	const Lane* other_lane = other_driver->currLane_.get();
	const RoadSegment* otherRoadSegment = other_lane->getRoadSegment();
	int other_offset = other_driver->currLaneOffset_.get();

	//If the vehicle is in the same Road segment
	if(pathMover.getCurrSegment() == otherRoadSegment) {
		//Set distance equal to the _forward_ distance between these two vehicles.
		int distance = other_offset - params.currLaneOffset;
		bool fwd = distance > 0;

		//Set different variables depending on where the car is.
		if(other_lane == params.currLane) {//the vehicle is on the current lane
			check_and_set_min_car_dist((fwd?params.nvFwd:params.nvBack), distance, vehicle, other_driver);
		} else if(other_lane == params.leftLane) { //the vehicle is on the left lane
			check_and_set_min_car_dist((fwd?params.nvLeftFwd:params.nvLeftBack), distance, vehicle, other_driver);
		} else if(other_lane == params.rightLane) { //The vehicle is on the right lane
			if(distance>0 || (distance < 0 && -distance<=minRBDistance)) {
				check_and_set_min_car_dist((fwd?params.nvRightFwd:params.nvRightBack), distance, vehicle, other_driver);
			}
		}
	} else if(otherRoadSegment->getLink() == pathMover.getCurrLink()) { //We are in the same link.
		if (!pathMover.isOnLastSegment() && pathMover.getCurrSegment()+1 == otherRoadSegment) { //Vehicle is on the next segment.
			//Retrieve the next node we are moving to, cast it to a UniNode.
			const Node* nextNode = isLinkForward ? pathMover.getCurrSegment()->getEnd() : pathMover.getCurrSegment()->getStart();
			const UniNode* uNode = dynamic_cast<const UniNode*>(nextNode);

			//Initialize some lane pointers
			const Lane* nextLane = nullptr;
			const Lane* nextLeftLane = nullptr;
			const Lane* nextRightLane = nullptr;
			if(uNode){
				nextLane = uNode->getOutgoingLane(*params.currLane);
			}

			//Make sure next lane exists and is in the next road segment, although it should be true
			if(nextLane && nextLane->getRoadSegment() == otherRoadSegment) {
				//Assign next left/right lane based on lane ID.
				size_t nextLaneIndex = getLaneIndex(nextLane);
				if(nextLaneIndex>0) {
					nextLeftLane = otherRoadSegment->getLanes().at(nextLaneIndex-1);
				}
				if(nextLaneIndex < otherRoadSegment->getLanes().size()-1) {
					nextRightLane = otherRoadSegment->getLanes().at(nextLaneIndex+1);
				}
			}

			//Modified distance.
			int distance = other_offset + params.currLaneLength - params.currLaneOffset;// - vehicle->length/2 - other_driver->getVehicle()->length/2;

			//Set different variables depending on where the car is.
			if(other_lane == nextLane) { //The vehicle is on the current lane
				check_and_set_min_car_dist(params.nvFwd, distance, vehicle, other_driver);
			} else if(other_lane == nextLeftLane) { //the vehicle is on the left lane
				check_and_set_min_car_dist(params.nvLeftFwd, distance, vehicle, other_driver);
			} else if(other_lane == nextRightLane) { //the vehicle is in front
				check_and_set_min_car_dist(params.nvRightFwd, distance, vehicle, other_driver);
			}
		} else if(!pathMover.isOnFirstSegment() && pathMover.getCurrSegment()-1 == otherRoadSegment) { //Vehicle is on the previous segment.
			//Retrieve the previous node as a UniNode.
			const Node* preNode = isLinkForward ? otherRoadSegment->getEnd() : otherRoadSegment->getStart();
			const UniNode* uNode=dynamic_cast<const UniNode*>(preNode);

			//Set some lane pointers.
			const Lane* preLane = nullptr;
			const Lane* preLeftLane = nullptr;
			const Lane* preRightLane = nullptr;

			//Find the node which leads to this one from the UniNode. (Requires some searching; should probably
			//   migrate this to the UniNode class later).
			const vector<Lane*>& lanes = otherRoadSegment->getLanes();
			if(uNode){
				for (vector<Lane*>::const_iterator it=lanes.begin(); it!=lanes.end()&&!preLane; it++) {
					if(uNode->getOutgoingLane(**it) == params.currLane) {
						preLane = *it;
					}
				}
			}

			//Make sure next lane is in the next road segment, although it should be true
			if(preLane) {
				//Save the new left/right lanes
				size_t preLaneIndex = getLaneIndex(preLane);
				if(preLaneIndex>0) {
					preLeftLane = otherRoadSegment->getLanes().at(preLaneIndex-1);
				}
				if(preLaneIndex < otherRoadSegment->getLanes().size()-1) {
					preRightLane = otherRoadSegment->getLanes().at(preLaneIndex+1);
				}
			}

			//Modified distance.
			int distance = other_driver->currLaneLength_.get() - other_offset + params.currLaneOffset;// - vehicle->length/2 - other_driver->getVehicle()->length/2;

			//Set different variables depending on where the car is.
			if(other_lane == preLane) { //The vehicle is on the current lane
				check_and_set_min_car_dist(params.nvBack, distance, vehicle, other_driver);
			} else if(other_lane == preLeftLane) { //the vehicle is on the left lane
				check_and_set_min_car_dist(params.nvLeftBack, distance, vehicle, other_driver);
			} else if(other_lane == preRightLane) { //the vehicle is on the right lane
				check_and_set_min_car_dist(params.nvRightBack, distance, vehicle, other_driver);
			}
		}
	}
}


void sim_mob::Driver::updateNearbyPedestrian(UpdateParams& params, const Person* other, const Pedestrian* pedestrian)
{
	//Only update if passed a valid pointer and this is on a crossing.
	if (!(pedestrian && pedestrian->isOnCrossing())) {
		return;
	}

	//Calculate the other driver's position down the polyline.
	//NOTE: This might be better passed as a Buffered property. ~Seth
	//TODO: This might be slightly inaccurate if you are trying to force the other vehicle into a given
	//      local coordinate system. But it will work for now and we can clean it up later.
	DynamicVector otherVect(
		polypathMover.getCurrPolypoint().getX(), polypathMover.getCurrPolypoint().getY(),
		other->xPos.get(), other->yPos.get()
	);

	//Calculate the distance between these two vehicles and the distance between the angle of the
	// car's forward movement and the pedestrian.
	//NOTE: I am changing this slightly, since cars were stopping for pedestrians on the opposite side of
	//      the road for no reason (traffic light was green).
	double distance = otherVect.getMagnitude();
	double angleDiff = 0.0;
	{
		//We need to retrieve the actual vector so that we maintain its direction in case it was zero.
		// Again, this can be fixed later by just buffering the data.
		DynamicVector fwdVector(vehicle->TEMP_retrieveFwdVelocityVector());
		fwdVector.scaleVectTo(100);

		//Calculate the difference
		//NOTE: I may be over-complicating this... we can probably use the dot product but that can be done later. ~Seth
		double angle1 = atan2(fwdVector.getEndY()-fwdVector.getY(), fwdVector.getEndX()-fwdVector.getX());
		double angle2 = atan2(otherVect.getEndY()-otherVect.getY(), otherVect.getEndX()-otherVect.getX());
		double diff = fabs(angle1 - angle2);
		angleDiff = std::min(diff, fabs(diff - 2*M_PI));
	}

	//If the pedestrian is not behind us, then set our flag to true and update the minimum pedestrian distance.
	if(angleDiff < 0.5236) { //30 degrees +/-
		isPedestrianAhead = true;
		distance = distance - vehicle->length/2 - 300;
		if(distance < minPedestrianDis) {
			minPedestrianDis = distance;
		}
	}
}


void sim_mob::Driver::updateNearbyAgents(UpdateParams& params)
{
	//Reset parameters
	minPedestrianDis = 5000;
	isPedestrianAhead = false;

	//Retrieve a list of nearby agents
	vector<const Agent*> nearby_agents = AuraManager::instance().nearbyAgents(Point2D(vehicle->getX(),vehicle->getY()), *params.currLane,  distanceInFront, distanceBehind);

	//Update each nearby Pedestrian/Driver
	for (vector<const Agent*>::iterator it=nearby_agents.begin(); it!=nearby_agents.begin(); it++) {
		//Perform no action on non-Persons
		const Person* other = dynamic_cast<const Person *>(*it);
		if(!other) {
			continue;
		}

		//Perform a different action depending on whether or not this is a Pedestrian/Driver/etc.
		updateNearbyDriver(params, other, dynamic_cast<const Driver*>(other->getRole()));
		updateNearbyPedestrian(params, other, dynamic_cast<const Pedestrian*>(other->getRole()));
	}
}

//Angle shows the velocity direction of vehicles
void sim_mob::Driver::updateAngle()
{
	//TODO: Angle is read-only, so we can set it more simply later using
	//      atan2. Remember, atan2 is your friend!
	//double xVel = vehicle->velocity.getAbsX();
	//double yVel = vehicle->velocity.getAbsY();
	double xVel = vehicle->velocity.getMagnitude();
	double yVel = vehicle->velocity_lat.getMagnitude();

	if(xVel==0 && yVel==0){}
    else if(xVel>=0 && yVel>=0) { angle = 360 - atan(yVel/xVel)/3.1415926*180; }
	else if(xVel>=0 && yVel<0 ) { angle = - atan(yVel/xVel)/3.1415926*180; }
	else if(xVel<0  && yVel>=0) { angle = 180 - atan(yVel/xVel)/3.1415926*180; }
	else if(xVel<0  && yVel<0 ) { angle = 180 - atan(yVel/xVel)/3.1415926*180; }
}

void sim_mob :: Driver :: intersectionVelocityUpdate()
{
	double inter_speed = 1000;//10m/s
	vehicle->setAcceleration(0);

	//TODO: We can figure this out later. In fact, the whole "traces" approach to
	//      intersections is pretty easy using vectors.
	vehicle->setVelocity(inter_speed * xDirection_entryPoint);
	vehicle->setLatVelocity(inter_speed * yDirection_entryPoint);
}

void sim_mob :: Driver :: enterNextLink(UpdateParams& p)
{
	isLinkForward = nextIsForward;
	p.currLane = nextLaneInNextLink;
	updateCurrInfo_CrossIntersection(p);
	abs2relat();

	//TODO: Lateral accelleration wasn't being used. You might enable it again here if you want it.
	vehicle->setLatVelocity(0);
	vehicle->pos.resetLateral();

	linkDriving(p);
	relat2abs();
}

void sim_mob::Driver::updatePosLC(UpdateParams& p)
{
	//right
	if(changeDecision == 1)
	{
		//if(!lcEnterNewLane && -vehicle->yPos_>=150)//currLane->getWidth()/2.0)
		if(!lcEnterNewLane && -vehicle->pos.getLateralMovement() >=150)//currLane->getWidth()/2.0)
		{
			p.currLane = p.rightLane;
			updateCurrInfo_SameRS(p);
			abs2relat();
			lcEnterNewLane = true;
		}
		//if(lcEnterNewLane && vehicle->yPos_ <=0)
		if(lcEnterNewLane && vehicle->pos.getLateralMovement() <=0)
		{
			isLaneChanging =false;
			//vehicle->yPos_ = 0;
			vehicle->pos.resetLateral();

			//vehicle->yVel_ = 0;
			//vehicle->velocity.setRelY(0);
			vehicle->velocity_lat.scaleVectTo(0);
		}
	}
	else if(changeDecision == -1)
	{
		//if(!lcEnterNewLane && vehicle->yPos_>=150)//currLane->getWidth()/2.0)
		if(!lcEnterNewLane && vehicle->pos.getLateralMovement()>=150)//currLane->getWidth()/2.0)
		{
			p.currLane = p.leftLane;
			updateCurrInfo_SameRS(p);
			abs2relat();
			lcEnterNewLane = true;
		}
		//if(lcEnterNewLane && vehicle->yPos_ >=0)
		if(lcEnterNewLane && vehicle->pos.getLateralMovement() >=0)
		{
			isLaneChanging =false;

			//vehicle->yPos_ = 0;
			vehicle->pos.resetLateral();

			//vehicle->yVel_ = 0;
			//vehicle->velocity.setRelY(0);
			vehicle->velocity_lat.scaleVectTo(0);
		}
	}
}

bool sim_mob::Driver::isCloseToLinkEnd(UpdateParams& p)
{
	//when the distance <= 10m
	return isReachLastRSinCurrLink()&&(p.currLaneLength - p.currLaneOffset<2000);
}

void sim_mob::Driver::updateTrafficSignal()
{
	const Node* node = currRoadSegment->getEnd();
	if(node)
		trafficSignal = StreetDirectory::instance().signalAt(*node);
	else
		trafficSignal = nullptr;
}

/*void sim_mob::Driver::pedestrianAheadDriving(UpdateParams& p)
{
	if(p.perceivedFwdVelocity>0) {
		vehicle->accel.scaleVectTo(-0.5*p.perceivedFwdVelocity*p.perceivedFwdVelocity/(0.5*minPedestrianDis));
	} else {
		vehicle->accel.scaleVectTo(0);
		vehicle->velocity.scaleVectTo(0);
	}
	updatePositionOnLink();
}*/

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
			tsStopDistance = p.currLaneLength - p.currLaneOffset - vehicle->length/2 -300;
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

