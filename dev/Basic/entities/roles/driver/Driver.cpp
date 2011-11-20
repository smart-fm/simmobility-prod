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
#include "util/GeomHelpers.hpp"


using namespace sim_mob;
using std::max;
using std::vector;
using std::set;
using std::map;


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

//TODO:I think lane index should be a data member in the lane class
size_t getLaneIndex(const Lane* l)
{
	if (l) {
		const RoadSegment* r = l->getRoadSegment();
		for(size_t i=0;i<r->getLanes().size();i++) {
			if(r->getLanes().at(i)==l) {
				return i;
			}
		}
	}
	return -1;  //NOTE: This might not do what you expect! ~Seth
}


} //End anon namespace



//initiate
sim_mob::Driver::Driver(Agent* parent) : Role(parent), vehicle(nullptr), perceivedVelocity(reactTime, true),
	perceivedVelocityOfFwdCar(reactTime, true), perceivedDistToFwdCar(reactTime, false),
	distanceInFront(2000), distanceBehind(500), currLane_(nullptr), currLaneOffset_(0), currLaneLength_(0), isInIntersection(false)
{
	//Set default speed in the range of 10m/s to 19m/s
	//speed = 0;//1000*(1+((double)(rand()%10))/10);

	//Initialize our models. These should be swapable later.
	lcModel = new MITSIM_LC_Model();
	cfModel = new MITSIM_CF_Model();

	//Some one-time flags and other related defaults.
	firstFrameTick = true;
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



//TODO: We can use initializer lists later to make some of these params const.
sim_mob::UpdateParams::UpdateParams(const Driver& owner)
{
	//Set to the previous known buffered values
	currLane = owner.currLane_.get();
	currLaneIndex = getLaneIndex(currLane);
	currLaneLength = owner.currLaneLength_.get();
	currLaneOffset = owner.currLaneOffset_.get();

	//Current lanes to the left and right. May be null
	leftLane = nullptr;
	rightLane = nullptr;

	//Reset; these will be set before they are used; the values here represent either defaul
	//       values or are unimportant.
	currSpeed = 0;
	perceivedFwdVelocity = 0;
	perceivedLatVelocity = 0;
	isTrafficLightStop = false;
	trafficSignalStopDistance = 5000;
	elapsedSeconds = ConfigParams::GetInstance().baseGranMS/1000.0;

	//Lateral velocity of lane changing.
	laneChangingVelocity = 100;

	//Are we near a crossing?
	isCrossingAhead = false;

	//relative x coordinate for crossing, the intersection point of crossing's front line and current polyline
	crossingFwdDistance = 0;

	//Space to next car
	space = 0;

	//the acceleration of leading vehicle
	a_lead = 0;

	//the speed of leading vehicle
	v_lead = 0;

	//the distance which leading vehicle will move in next time step
	space_star = 0;

	distanceToNormalStop = 0;

	//distance to where critical location where lane changing has to be made
	dis2stop = 0;

	//in MLC: is the vehicle waiting acceptable gap to change lane
	isWaiting = false; //TODO: This might need to be saved between turns.
}



void sim_mob::Driver::update_first_frame(UpdateParams& params, frame_t frameNumber)
{
	//Save the path from orign to destination in allRoadSegments
	initializePath();

	//Set some properties about the current path, such as the current polyline, etc.
	if(vehicle->hasPath()) {
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
	if(vehicle->isDone()){
		//TEMP: Move to (0,0). This should prevent collisions.
		//TODO: Remove from simulation. Do this in the dispatcher at the same time...
		parent->xPos.set(0);
		parent->yPos.set(0);

		//TODO:reach destination
		vehicle->setAcceleration(0);
		vehicle->setVelocity(0);
		vehicle->setLatVelocity(0);
		return;
	}

	//Save the nearest agents in your lane and the surrounding lanes, stored by their
	// position before/behind you. Save nearest fwd pedestrian too.
	updateNearbyAgents(params);

	//Driving behavior differs drastically inside intersections.
	const RoadSegment* prevSegment = vehicle->getCurrSegment();
	if(vehicle->isInIntersection()) {
		intersectionDriving(params);
	} else {
		//Manage traffic signal behavior if we are close to the end of the link.
		if(isCloseToLinkEnd(params)) {
			trafficSignalDriving(params);
		}
		linkDriving(params);

		//Did our last move forward bring us into an intersection?
		if(vehicle->isInIntersection()) {
			//the first time vehicle pass the end of current link and enter intersection
			//if the vehicle reaches the end of the last road segment on current link
			//I assume this vehicle enters the intersection
			calculateIntersectionTrajectory();
			intersectionVelocityUpdate();
			intersectionDriving(params);
		}
	}

	//Has the segment changed?
	if (!vehicle->isDone() && (vehicle->getCurrSegment()!=prevSegment)) {
		//Make pre-intersection decisions?
		if(!vehicle->hasNextSegment(true)) {
			updateTrafficSignal();
			if(!vehicle->hasNextSegment(true)) { //TODO: Logic is clearly wrong here.
				chooseNextLaneForNextLink(params);
			}
		}
	}

	//Update parent/angle.
	setParentBufferedData();
	updateAngle(params);
}



//Main update functionality
void sim_mob::Driver::update(frame_t frameNumber)
{
	//Do nothing?
	if(frameNumber<parent->startTime)
		return;

	//Create a new set of local parameters for this frame update.
	UpdateParams params(*this);

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
	//TODO: Update parent buffered properties, or perhaps delegate this.
	currLane_.set(params.currLane);
	currLaneOffset_.set(params.currLaneOffset);
	currLaneLength_.set(params.currLaneLength);
	isInIntersection.set(vehicle->isInIntersection());

	//Print output for this frame.
	if (!vehicle->isDone()) {
		output(params, frameNumber);
	}
}

void sim_mob::Driver::output(UpdateParams& p, frame_t frameNumber)
{
	LogOut("(\"Driver\""
			<<","<<frameNumber
			<<","<<parent->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(vehicle->getX())
			<<"\",\"yPos\":\""<<static_cast<int>(vehicle->getY())
			<<"\",\"angle\":\""<<p.vehicleAngle
			<<"\"})"<<std::endl);
}


//responsible for vehicle behaviour inside intersection
//the movement is based on absolute position
void sim_mob::Driver::intersectionDriving(UpdateParams& p)
{
	//Don't move if we have no target
	if (!nextLaneInNextLink) {
		return;
	}

	//First, update movement along the vector.
	intersectionDistAlongTrajectory += vehicle->getVelocity() * p.elapsedSeconds;

	//Next, detect if we've just left the intersection. Otherwise, perform regular intersection driving.
	if(isLeaveIntersection()) {
		p.currLane = vehicle->moveToNextSegmentAfterIntersection();
		justLeftIntersection(p);
		linkDriving(p); //Chain to regular link driving behavior.
	} else {
		//Scale a vector to find our new position.
		DynamicVector movement(intersectionTrajectory);
		movement.scaleVectTo(intersectionDistAlongTrajectory).translateVect();
		vehicle->setPositionInIntersection(movement.getX(), movement.getY());
	}
}

//vehicle movement on link, perform acceleration, lane changing if necessary
//the movement is based on relative position
void sim_mob::Driver::linkDriving(UpdateParams& p)
{
	//TODO: This might not be in the right location
	p.dis2stop = vehicle->getCurrLinkLength() - currLinkOffset - vehicle->length/2 - 300;
	p.dis2stop /= 100;

	//Check if we should change lanes.
	double newLatVel = lcModel->executeLaneChanging(p, vehicle->getCurrLinkLength(), vehicle->length, getCurrLaneChangeDirection());
	if (newLatVel>=0.0) {
		vehicle->setLatVelocity(newLatVel);
	}

	//Retrieve a new acceleration value.
	double newFwdAcc = 0;
	if(p.isTrafficLightStop && vehicle->getVelocity() < 50) {
		//Slow down.
		//TODO: Shouldn't acceleration be set to negative in this case?
		vehicle->setVelocity(0);
		vehicle->setAcceleration(0);
	} else {
		//Convert back to m/s
		//TODO: Is this always m/s? We should rename the variable then...
		p.currSpeed = vehicle->getVelocity()/100;

		//Call our model
		newFwdAcc = cfModel->makeAcceleratingDecision(p, targetSpeed, maxLaneSpeed);
	}

	//Update our chosen acceleration; update our position on the link.
	updateAcceleration(newFwdAcc);
	updatePositionOnLink(p);
}


//for buffer data, maybe need more parameters to be stored
//TODO: need to discuss
void sim_mob::Driver::setParentBufferedData()
{
	parent->xPos.set(vehicle->getX());
	parent->yPos.set(vehicle->getY());

	//TODO: Need to see how the parent agent uses its velocity vector.
	parent->fwdVel.set(vehicle->getVelocity());
	parent->latVel.set(vehicle->getLatVelocity());
}

bool sim_mob::Driver::isLeaveIntersection() const
{
	return intersectionDistAlongTrajectory >= intersectionTrajectory.getMagnitude();
}


namespace {
vector<const Agent*> GetAgentsInCrossing(const Crossing* crossing) {
	//Put x and y coordinates into planar arrays.
	int x[4] = {crossing->farLine.first.getX(),crossing->farLine.second.getX(),crossing->nearLine.first.getX(),crossing->nearLine.second.getX()};
	int y[4] = {crossing->farLine.first.getY(),crossing->farLine.second.getY(),crossing->nearLine.first.getY(),crossing->nearLine.second.getY()};

	//Prepare minimum/maximum values.
    int xmin = x[0];
    int xmax = x[0];
    int ymin = y[0];
    int ymax = y[0];
	for(int i=0;i<4;i++) {
		if(x[i]<xmin)
			xmin = x[i];
		if(x[i]>xmax)
			xmax = x[i];
		if(y[i]<ymin)
			ymin = y[i];
		if(y[i]>ymax)
			ymax = y[i];
    }

	//Create a rectangle from xmin,ymin to xmax,ymax
	//TODO: This is completely unnecessary; Crossings are already in order, so
	//      crossing.far.first and crossing.near.second already defines a rectangle.
	Point2D rectMinPoint = Point2D(xmin,ymin);
	Point2D rectMaxPoint = Point2D(xmax,ymax);

	return AuraManager::instance().agentsInRect(rectMinPoint, rectMaxPoint);
}
} //End anon namespace


bool sim_mob::Driver::isPedetrianOnTargetCrossing()
{
	if(!trafficSignal) {
		return false;
	}

	map<Link const*, size_t> const linkMap = trafficSignal->links_map();
	int index = -1;
	for(map<Link const*, size_t>::const_iterator link_i=linkMap.begin(); link_i!=linkMap.end(); link_i++) {
		if(vehicle->getNextSegment() && link_i->first==vehicle->getNextSegment()->getLink()) {
			index = (*link_i).second;
			break;
		}
	}

	map<Crossing const *, size_t> const crossingMap = trafficSignal->crossings_map();
	const Crossing* crossing = nullptr;
	for(map<Crossing const *, size_t>::const_iterator crossing_i=crossingMap.begin(); crossing_i!=crossingMap.end(); crossing_i++) {
		if(static_cast<int>(crossing_i->second)==index) {
			crossing = crossing_i->first;
			break;
		}
	}

	//Have we found a relevant crossing?
	if(!crossing) {
		return false;
	}

	//Search through the list of agents in that crossing.
	vector<const Agent*> agentsInRect = GetAgentsInCrossing(crossing);
	for(vector<const Agent*>::iterator it=agentsInRect.begin(); it!=agentsInRect.end(); it++) {
		const Person* other = dynamic_cast<const Person *>(*it);
		if(other) {
			const Pedestrian* pedestrian = dynamic_cast<const Pedestrian*>(other->getRole());
			if(pedestrian && pedestrian->isOnCrossing()) {
				return true;
			}
		}
	}
	return false;
}


//calculate current lane length
void sim_mob::Driver::updateCurrLaneLength(UpdateParams& p)
{
	p.currLaneLength = vehicle->getCurrPolylineVector().getMagnitude();
}

//update left and right lanes of the current lane
//if there is no left or right lane, it will be null
void sim_mob::Driver::updateAdjacentLanes(UpdateParams& p)
{
	p.leftLane = nullptr;
	p.rightLane = nullptr;

	if(p.currLaneIndex>0) {
		p.rightLane = vehicle->getCurrSegment()->getLanes().at(p.currLaneIndex-1);
	}
	if(p.currLaneIndex < vehicle->getCurrSegment()->getLanes().size()-1) {
		p.leftLane = vehicle->getCurrSegment()->getLanes().at(p.currLaneIndex+1);
	}
}


//General update information for whenever a Segment may have changed.
void sim_mob::Driver::syncCurrLaneCachedInfo(UpdateParams& p)
{
	//The lane may have changed; reset the current lane index.
	p.currLaneIndex = getLaneIndex(p.currLane);

	//Update which lanes are adjacent, and the length of the current polyline.
	updateAdjacentLanes(p);
	updateCurrLaneLength(p);

	//Finally, update target/max speed to match the new Lane's rules.
	maxLaneSpeed = vehicle->getCurrSegment()->maxSpeed/3.6; //slow down
	targetSpeed = maxLaneSpeed;
}


//currently it just chooses the first lane from the targetLane
//Note that this also sets the target lane so that we (hopefully) merge before the intersection.
void sim_mob::Driver::chooseNextLaneForNextLink(UpdateParams& p)
{
	//Retrieve the node we're on, and determine if this is in the forward direction.
	const MultiNode* currEndNode = dynamic_cast<const MultiNode*>(vehicle->getNodeMovingTowards());
	const RoadSegment* nextSegment = vehicle->getNextSegment(false);

	//Build up a list of target lanes.
	nextLaneInNextLink = nullptr;
	vector<const Lane*> targetLanes;
	if(currEndNode && nextSegment){
		const set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(*vehicle->getCurrSegment());
		for(set<LaneConnector*>::const_iterator it=lcs.begin(); it!=lcs.end(); it++){
			if((*it)->getLaneTo()->getRoadSegment() == nextSegment && (*it)->getLaneFrom()==p.currLane) {
				//It's a valid lane.
				targetLanes.push_back((*it)->getLaneTo());

				//find target lane with same index, use this lane
				size_t laneIndex = getLaneIndex((*it)->getLaneTo());
				if(laneIndex == p.currLaneIndex) {
					nextLaneInNextLink = (*it)->getLaneTo();
					targetLaneIndex = laneIndex;
					break;
				}
			}
		}

		//Still haven't found a lane?
		if(!nextLaneInNextLink){
			//no lane with same index, use the first lane in the vector if possible.
			if (targetLanes.size()>0) {
				nextLaneInNextLink = targetLanes.at(0);
				targetLaneIndex = getLaneIndex(nextLaneInNextLink);
			} else if (nextSegment) { //Fallback
				size_t fallbackIndex = std::min(p.currLaneIndex, nextSegment->getLanes().size()-1);
				nextLaneInNextLink = nextSegment->getLanes().at(fallbackIndex);
				targetLaneIndex = fallbackIndex;
			}
		}
	}
}

//TODO: For now, we're just using a simple trajectory model. Complex curves may be added later.
void sim_mob::Driver::calculateIntersectionTrajectory()
{
	//If we have no target link, we have no target trajectory.
	if (!nextLaneInNextLink) {
		return;
	}

	//Get the entry point.
	Point2D entryPoint = nextLaneInNextLink->getPolyline().at(0);

	//Compute a movement trajectory.
	intersectionTrajectory = DynamicVector(vehicle->getX(), vehicle->getY(), entryPoint.getX(), entryPoint.getY());
	intersectionDistAlongTrajectory = 0;
}


//link path should be retrieved from other class
//for now, it serves as this purpose
void sim_mob::Driver::initializePath()
{
	//Save local copies of the parent's origin/destination nodes.
	originNode = parent->originNode;
	destNode = parent->destNode;

	//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
	//TODO: Start in lane 0?
	//A non-null vehicle means we are moving.
	vehicle = new Vehicle(StreetDirectory::instance().shortestDrivingPath(*originNode, *destNode), 0);
}

void sim_mob::Driver::setOrigin(UpdateParams& p)
{
	//Set the max speed and target speed.
	maxLaneSpeed = vehicle->getCurrSegment()->maxSpeed/3.6;//slow down
	targetSpeed = maxLaneSpeed;

	//Set our current and target lanes.
	p.currLaneIndex = 0;
	p.currLane = vehicle->getCurrSegment()->getLanes().at(p.currLaneIndex);
	targetLaneIndex = p.currLaneIndex;

	//Vehicles start at rest
	vehicle->setVelocity(0);
	vehicle->setLatVelocity(0);
	vehicle->setAcceleration(0);

	//Scan and save the lanes to the left and right.
	updateAdjacentLanes(p);

	//Calculate and save the total length of the current polyline.
	updateCurrLaneLength(p);
}


//TODO
void sim_mob::Driver::findCrossing(UpdateParams& p)
{
	const Crossing* crossing=dynamic_cast<const Crossing*>(vehicle->getCurrSegment()->nextObstacle(vehicle->getDistanceMovedInSegment(),true).item);

	if(crossing) {
		//TODO: Please double-check that this does what's intended.
		Point2D interSect = LineLineIntersect(vehicle->getCurrPolylineVector(), crossing->farLine.first, crossing->farLine.second);
		DynamicVector toCrossing(vehicle->getX(), vehicle->getY(), interSect.getX(), interSect.getY());

		p.crossingFwdDistance = toCrossing.getMagnitude();
		p.isCrossingAhead = true;
	}
}

void sim_mob::Driver::updateAcceleration(double newFwdAcc)
{
	vehicle->setAcceleration(newFwdAcc * 100);
}

void sim_mob::Driver::updatePositionOnLink(UpdateParams& p)
{
	//Determine how far forward we've moved.
	//TODO: I've disabled the acceleration component because it doesn't really make sense.
	//      Please re-enable if you think this is expected behavior. ~Seth
	//fwdDistance = vehicle->getVelocity()*timeStep + 0.5*vehicle->getAcceleration()*timeStep*timeStep;
	double fwdDistance = vehicle->getVelocity()*p.elapsedSeconds;
	double latDistance = vehicle->getLatVelocity()*p.elapsedSeconds;

	//Increase the vehicle's velocity based on its acceleration.
	vehicle->setVelocity(vehicle->getVelocity() + vehicle->getAcceleration()*p.elapsedSeconds);
	if (vehicle->getVelocity() < 0) {
		//Set to a small forward velocity, no acceleration.
		vehicle->setVelocity(0.1); //TODO: Why not 0.0?
		vehicle->setAcceleration(0);
	}

	//Move the vehicle forward.
	vehicle->moveFwd(fwdDistance);

	//Lateral movement
	if (latDistance!=0) {
		vehicle->moveLat(latDistance);
		updatePositionDuringLaneChange(p);
	}

	//Update our offset in the current lane.
	p.currLaneOffset += fwdDistance;
}


//Helper function: check if a modified distanc is less than the current minimum and save it.
void sim_mob::Driver::check_and_set_min_car_dist(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other)
{
	//Subtract the size of the car from the distance between them
	distance = fabs(distance) - veh->length/2 - other->getVehicle()->length/2;
	if(distance <= res.distance) {
		res.driver = other;
		res.distance = distance;
	}

}


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
	if(vehicle->getCurrSegment() == otherRoadSegment) {
		//Set distance equal to the _forward_ distance between these two vehicles.
		int distance = other_offset - params.currLaneOffset;
		bool fwd = distance > 0;

		//Set different variables depending on where the car is.
		if(other_lane == params.currLane) {//the vehicle is on the current lane
			check_and_set_min_car_dist((fwd?params.nvFwd:params.nvBack), distance, vehicle, other_driver);
		} else if(other_lane == params.leftLane) { //the vehicle is on the left lane
			check_and_set_min_car_dist((fwd?params.nvLeftFwd:params.nvLeftBack), distance, vehicle, other_driver);
		} else if(other_lane == params.rightLane) { //The vehicle is on the right lane
			if(distance>0 || (distance < 0 && -distance<=params.nvRightBack.distance)) {
				check_and_set_min_car_dist((fwd?params.nvRightFwd:params.nvRightBack), distance, vehicle, other_driver);
			}
		}
	} else if(otherRoadSegment->getLink() == vehicle->getCurrLink()) { //We are in the same link.
		if (vehicle->getNextSegment() == otherRoadSegment) { //Vehicle is on the next segment.
			//Retrieve the next node we are moving to, cast it to a UniNode.
			const Node* nextNode = vehicle->getNodeMovingTowards();
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
		} else if(vehicle->getPrevSegment() == otherRoadSegment) { //Vehicle is on the previous segment.
			//Retrieve the previous node as a UniNode.
			const Node* prevNode = vehicle->getNodeMovingFrom();
			const UniNode* uNode=dynamic_cast<const UniNode*>(prevNode);

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

	//TODO: We are using a vector to check the angle to the Pedestrian. There are other ways of doing this which may be more accurate.
	const std::vector<sim_mob::Point2D>& polyLine = vehicle->getCurrSegment()->getLanes().front()->getPolyline();
	DynamicVector otherVect(
		polyLine.front().getX(), polyLine.front().getY(),
		other->xPos.get(), other->yPos.get()
	);

	//Calculate the distance between these two vehicles and the distance between the angle of the
	// car's forward movement and the pedestrian.
	//NOTE: I am changing this slightly, since cars were stopping for pedestrians on the opposite side of
	//      the road for no reason (traffic light was green). ~Seth
	//double distance = otherVect.getMagnitude();
	double angleDiff = 0.0;
	{
		//Retrieve
		DynamicVector fwdVector(vehicle->getCurrPolylineVector());
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
		params.npedFwd.distance = std::min(params.npedFwd.distance, otherVect.getMagnitude()-vehicle->length/2-300);
	}
}


void sim_mob::Driver::updateNearbyAgents(UpdateParams& params)
{
	//Retrieve a list of nearby agents
	vector<const Agent*> nearby_agents = AuraManager::instance().nearbyAgents(Point2D(vehicle->getX(),vehicle->getY()), *params.currLane,  distanceInFront, distanceBehind);

	//Update each nearby Pedestrian/Driver
	for (vector<const Agent*>::iterator it=nearby_agents.begin(); it!=nearby_agents.end(); it++) {
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
void sim_mob::Driver::updateAngle(UpdateParams& p)
{
	//Set angle based on the vehicle's heading and velocity.
	p.vehicleAngle = 360 - (vehicle->getAngle() * 180 / M_PI);
}

void sim_mob::Driver::intersectionVelocityUpdate()
{
	double inter_speed = 1000;//10m/s
	vehicle->setAcceleration(0);

	//Set velocity for intersection movement.
	vehicle->setVelocity(inter_speed);
}

void sim_mob::Driver::justLeftIntersection(UpdateParams& p)
{
	p.currLane = nextLaneInNextLink;
	vehicle->moveToNewLanePolyline(getLaneIndex(p.currLane));
	syncCurrLaneCachedInfo(p);
	p.currLaneOffset = 0;
	targetLaneIndex = p.currLaneIndex;

	//Reset lateral movement/velocity to zero.
	vehicle->setLatVelocity(0);
	vehicle->resetLateralMovement();
}

LANE_CHANGE_SIDE sim_mob::Driver::getCurrLaneChangeDirection() const
{
	if (vehicle->getLatVelocity()>0) {
		return LCS_LEFT;
	} else if (vehicle->getLatVelocity()<0) {
		return LCS_RIGHT;
	}
	return LCS_SAME;
}

LANE_CHANGE_SIDE sim_mob::Driver::getCurrLaneSideRelativeToCenter() const
{
	if (vehicle->getLateralMovement()>0) {
		return LCS_LEFT;
	} else if (vehicle->getLateralMovement()<0) {
		return LCS_RIGHT;
	}
	return LCS_SAME;
}


//TODO: I think all lane changing occurs after 150m. Double-check please. ~Seth
void sim_mob::Driver::updatePositionDuringLaneChange(UpdateParams& p)
{
	double halfLaneWidth = p.currLane->getWidth() / 2.0;

	//The direction we are attempting to change lanes in
	LANE_CHANGE_SIDE actual = getCurrLaneChangeDirection();
	LANE_CHANGE_SIDE relative = getCurrLaneSideRelativeToCenter();
	if (actual==LCS_SAME) {
		return;  //Not actually changing lanes.
	}
	if ((actual==LCS_LEFT && !p.leftLane) || (actual==LCS_RIGHT && !p.rightLane)) {
		return; //Error condition
	}
	if (relative==LCS_SAME) {
		relative=actual; //Unlikely to occur, but we can still work off this.
	}

	//Basically, we move "halfway" into the next lane, and then move "halfway" back to its midpoint.
	if (actual==relative) {
		//Moving "out".
		double remainder = fabs(vehicle->getLateralMovement()) - halfLaneWidth;
		if (remainder>0) {
			//Update Lanes, polylines, RoadSegments, etc.
			p.currLane = (actual==LCS_LEFT?p.leftLane:p.rightLane);
			syncCurrLaneCachedInfo(p);
			vehicle->shiftToNewLanePolyline(actual==LCS_LEFT);

			//Set to the far edge of the other lane, minus any extra amount.
			halfLaneWidth = p.currLane->getWidth() / 2.0;
			vehicle->resetLateralMovement();
			vehicle->moveLat((halfLaneWidth - remainder) * (actual==LCS_LEFT?1:-1));
		}
	} else {
		//Moving "in".
		bool pastZero = (actual==LCS_LEFT) ? (vehicle->getLateralMovement()>0) : (vehicle->getLateralMovement()<0);
		if (pastZero) {
			//Reset all
			vehicle->resetLateralMovement();
			vehicle->setLatVelocity(0);
		}

	}
}

bool sim_mob::Driver::isCloseToLinkEnd(UpdateParams& p)
{
	//when the distance <= 10m
	return !vehicle->getNextSegment() && (p.currLaneLength-p.currLaneOffset<2000);
}

//Retrieve the current traffic signal based on our RoadSegment's end node.
void sim_mob::Driver::updateTrafficSignal()
{
	const Node* node = vehicle->getCurrSegment()->getEnd();
	trafficSignal = node ? StreetDirectory::instance().signalAt(*node) : nullptr;
}

void sim_mob::Driver::trafficSignalDriving(UpdateParams& p)
{
	if(!trafficSignal) {
		p.isTrafficLightStop = false;
	} else {
		Signal::TrafficColor color;
		if(nextLaneInNextLink) {
			color = trafficSignal->getDriverLight(*p.currLane,*nextLaneInNextLink);
		} else {
			color = trafficSignal->getDriverLight(*p.currLane).forward;
		}

		switch(color) {
			case Signal::Red :case Signal::Amber:
				p.isTrafficLightStop = true;
				p.trafficSignalStopDistance = p.currLaneLength - p.currLaneOffset - vehicle->length/2 -300;
				break;

			case Signal::Green:
				if(!isPedetrianOnTargetCrossing())
					p.isTrafficLightStop = false;
				else
					p.isTrafficLightStop = true;
				break;
		}
	}
}

