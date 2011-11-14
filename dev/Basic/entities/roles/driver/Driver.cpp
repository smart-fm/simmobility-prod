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
	firstFrameTick = true;
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



void sim_mob::Driver::update_first_frame(UpdateParams& params, frame_t frameNumber)
{
	//Save the path from orign to destination in allRoadSegments
	initializePath();


	if(!allRoadSegments.empty()) {
		setOrigin(params);
	}
}



void sim_mob::Driver::update_general(UpdateParams& params, frame_t frameNumber)
{
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
	perceivedVelocity.delay(new DPoint(vehicle->getVelocity(), vehicle->getLatVelocity()), currTimeMS);


	//perceivedVelocityOfFwdCar.delay(new Point2D(otherCarXVel, otherCarYVel), currTimeMS);
	//perceivedDistToFwdCar.delay(distToOtherCar, currTimeMS);

	//Now, retrieve your sensed velocity, distance, etc.
	if (perceivedVelocity.can_sense(currTimeMS)) {
		perceivedXVelocity_ = perceivedVelocity.sense(currTimeMS)->x;
		perceivedYVelocity_ = perceivedVelocity.sense(currTimeMS)->y;
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
					//vehicle->xPos_ -= polylineSegLength;


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

	update_general(params, frameNumber);
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

	//TODO: Is this right?
	parent->fwdVel.set(vehicle->getVelocity());
	parent->latVel.set(vehicle->getLatVelocity());
}


void sim_mob::Driver::sync_relabsobjs()
{
	//NOTE: I'm trying to make this automatic.

	//vehicle->velocity.changeCoords(currPolylineSegStart, currPolylineSegEnd);
	/*double oldMag = vehicle->velocity.getMagnitude();
	vehicle->velocity = DynamicVector(
		0, 0,
		currPolylineSegEnd.getX()-currPolylineSegStart.getX(),
		currPolylineSegEnd.getY()-currPolylineSegStart.getY()
	);
	vehicle->velocity.scaleVectTo(oldMag);

	oldMag = vehicle->velocity_lat.getMagnitude();
	vehicle->velocity_lat = DynamicVector(
		0, 0,
		currPolylineSegEnd.getX()-currPolylineSegStart.getX(),
		currPolylineSegEnd.getY()-currPolylineSegStart.getY()
	);
	vehicle->velocity_lat.scaleVectTo(oldMag);
	vehicle->velocity_lat.flipLeft();

	//vehicle->accel.changeCoords(currPolylineSegStart, currPolylineSegEnd);
	oldMag = vehicle->accel.getMagnitude();
	vehicle->accel = DynamicVector(
		0, 0,
		currPolylineSegEnd.getX()-currPolylineSegStart.getX(),
		currPolylineSegEnd.getY()-currPolylineSegStart.getY()
	);
	vehicle->accel.scaleVectTo(oldMag);

	//Sync position
	vehicle->pos.moveToNewVect(DynamicVector(currPolylineSegStart.getX(), currPolylineSegStart.getY(),
		currPolylineSegEnd.getX(), currPolylineSegEnd.getY()));*/
}


bool sim_mob::Driver::isReachPolyLineSegEnd()
{
	return vehicle-> reachedSegmentEnd();
}

bool sim_mob::Driver::isReachLastRSinCurrLink()
{
	return currRoadSegment == currLink->getPath(isForward).at(currLink->getPath(isForward).size()-1);
}

bool sim_mob::Driver::isReachLastPolyLineSeg()
{
	return (polylineSegIndex >= currLanePolyLine->size()-2);
}

//TODO
bool sim_mob::Driver::isCloseToCrossing()
{
	return (isReachLastRSinCurrLink()&&isCrossingAhead && xPosCrossing_-vehicle->getX()-vehicle->length/2 <= 1000);
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




//when current lane has been changed, update current information.
//mode 0: within same RS, for lane changing model
void sim_mob::Driver::updateCurrInfo_SameRS(UpdateParams& p)
{
	updateCurrGeneralInfo(p);

	polylineSegIndex = updateStartEndIndex(currLanePolyLine, currLaneOffset, polylineSegIndex);

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

	abs_relat();
}


//when current lane has been changed, update current information.
//mode 2: during crossing intersection
void sim_mob::Driver::updateCurrInfo_CrossIntersection(UpdateParams& p)
{
	updateCurrGeneralInfo(p);

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
	//vehicle->xPos = parent->originNode->location->getX();
	//vehicle->yPos = parent->originNode->location->getY();
	DynamicVector someVector(parent->originNode->location->getX(), parent->originNode->location->getY(), 1, 1);
	vehicle->pos = MovementVector(someVector);

	//vehicle->xVel = 0;
	//vehicle->yVel = 0;
	//vehicle->velocity.setAbs(0, 0);
	vehicle->velocity.scaleVectTo(0);
	vehicle->velocity_lat.scaleVectTo(0);

	//vehicle->xAcc = 0;
	//vehicle->yAcc = 0;
	//vehicle->accel.setAbs(0, 0);
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
	/*for(vector<WayPoint>::iterator it=a.begin(); it!=a.end(); it++) {
		if(it->type_ == WayPoint::ROAD_SEGMENT)
			allRoadSegments.push_back(it->roadSegment_);
	}*/
}

void sim_mob::Driver::setOrigin(UpdateParams& p)
{
	//A non-null vehicle means we are moving.
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
	//vehicle->xPos = currPolylineSegStart.getX();
	//vehicle->yPos = currPolylineSegStart.getY();
	vehicle->pos = MovementVector(DynamicVector(currPolylineSegStart.getX(), currPolylineSegStart.getY(), currPolylineSegEnd.getX(), currPolylineSegEnd.getY()));

	  {
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout <<"TestingX: " <<currPolylineSegStart.getX() <<" -> " <<vehicle->pos.getX() <<"\n";
		std::cout <<"TestingY: " <<currPolylineSegStart.getY() <<" -> " <<vehicle->pos.getY() <<"\n";
	  }


	abs_relat();
	abs2relat();

	//vehicle->xVel_=0;
	//vehicle->yVel_=0;
	//vehicle->velocity.setRel(0, 0);
	vehicle->velocity.scaleVectTo(0);
	vehicle->velocity_lat.scaleVectTo(0);

	//vehicle->xAcc_=0;
	//vehicle->yAcc_=0;
	//vehicle->accel.setRel(0, 0);
	vehicle->accel.scaleVectTo(0);

	//perceivedXVelocity_=vehicle->xVel_;
	//perceivedYVelocity_=vehicle->yVel_;
	//perceivedXVelocity_ = vehicle->velocity.getRelX();
	//perceivedYVelocity_ = vehicle->velocity.getRelY();

	relat2abs();
	updateAdjacentLanes();
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

void sim_mob::Driver::updatePositionOnLink()
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

	//Point2D myPos(vehicle->xPos,vehicle->yPos);
	Point2D myPos(vehicle->pos.getX(),vehicle->pos.getY());

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
			//int distance = otherX_ - vehicle->xPos_;
			int distance = otherX_ - vehicle->pos.getX();

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
	isForward = nextIsForward;
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
			p.currLane = rightLane;
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
			p.currLane = leftLane;
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
		//vehicle->accel.setRelX(-0.5*perceivedXVelocity_*perceivedXVelocity_/(0.5*minPedestrianDis));
		vehicle->accel.scaleVectTo(-0.5*perceivedXVelocity_*perceivedXVelocity_/(0.5*minPedestrianDis));
	} else {
		//vehicle->xAcc_ = 0;
		//vehicle->accel.setRelX(0);
		vehicle->accel.scaleVectTo(0);

		//vehicle->xVel_ = 0;
		//vehicle->velocity.setRelX(0);
		vehicle->velocity.scaleVectTo(0);
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

