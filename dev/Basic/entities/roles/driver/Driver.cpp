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
#include "util/DynamicVector.hpp"
#include "util/GeomHelpers.hpp"
#include "util/DebugFlags.hpp"


using namespace sim_mob;
using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;


//Helper functions
namespace {

//Helpful constants
const int distanceInFront = 2000;
const int distanceBehind = 500;

//Output helper
string PrintLCS(LANE_CHANGE_SIDE s) {
	if (s==LCS_LEFT) {
		return "LCS_LEFT";
	} else if (s==LCS_RIGHT) {
		return "LCS_RIGHT";
	}
	return "LCS_SAME";
}

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


//PathA: Small loop (south)
const Point2D SpecialPathA[] = {
	Point2D(37218351,14335255),   //AIMSUN 75780
	Point2D(37227139,14327875),   //AIMSUN 91218
	Point2D(37250760,14355120),   //AIMSUN 66508
	Point2D(37241080,14362955),   //AIMSUN 61688
};

//PathB: Large loop
const Point2D SpecialPathB[] = {
	Point2D(37218351,14335255),   //AIMSUN 75780
	Point2D(37227139,14327875),   //AIMSUN 91218
	Point2D(37250760,14355120),   //AIMSUN 66508
	Point2D(37270984,14378959),   //AIMSUN 45666
	Point2D(37262150,14386897),   //AIMSUN 45690
	Point2D(37241080,14362955),   //AIMSUN 61688
};


//Path is in multi-node positions
vector<WayPoint> ConvertToWaypoints(const Node* origin, const vector<Point2D>& path)
{
	vector<WayPoint> res;

	//Double-check our first node. Also ensure we have at least 2 nodes (or a path can't be found).
	if (path.size()<2 || origin->location->getX()!=path.front().getX() || origin->location->getY()!=path.front().getY()) {
		throw std::runtime_error("Special path does not begin on origin.");
	}

	//Starting at the origin, find the outgoing Link to each node in the list. Then loop around back to the origin.
	const MultiNode* curr = dynamic_cast<const MultiNode*>(origin);
	for (vector<Point2D>::const_iterator it=path.begin(); it!=path.end(); it++) {
		if (!curr) {
			throw std::runtime_error("Not a multinode (in special path).");
		}

		//Search for the Link to the next point.
		Point2D nextPt = it+1==path.end()?path.front():*(it+1);
		std::pair<const Link*, bool> nextLink(nullptr,false); //bool == fwd?
		const set<RoadSegment*>& segs = curr->getRoadSegments();
		for (set<RoadSegment*>::const_iterator segIt=segs.begin(); segIt!=segs.end(); segIt++) {
			const Link* ln = (*segIt)->getLink();
			if (ln->getStart()->location->getX()==nextPt.getX() && ln->getStart()->location->getY()==nextPt.getY()) {
				nextLink.first = ln;
				nextLink.second = false;
				break;
			} else if (ln->getEnd()->location->getX()==nextPt.getX() && ln->getEnd()->location->getY()==nextPt.getY()) {
				nextLink.first = ln;
				nextLink.second = true;
				break;
			}
		}
		if (!nextLink.first) {
			throw std::runtime_error("Couldn't find a Link between nodes in the Special path");
		}

		//Add each Segment in the Link's fwd/rev path to the result.
		const vector<RoadSegment*>& segPath = nextLink.first->getPath(nextLink.second);
		for (vector<RoadSegment*>::const_iterator pthIt=segPath.begin(); pthIt!=segPath.end(); pthIt++) {
			res.push_back(WayPoint(*pthIt));
		}

		//Continue
		curr = dynamic_cast<const MultiNode*>(nextLink.second ? nextLink.first->getEnd() : nextLink.first->getStart());
	}

	return res;
}


//For the NS3 paths
vector<WayPoint> LoadSpecialPath(const Node* origin, char pathLetter)
{
	if (pathLetter=='A') {
		size_t sz = sizeof(SpecialPathA) / sizeof(SpecialPathA[0]);
		return ConvertToWaypoints(origin, vector<Point2D>(SpecialPathA, &SpecialPathA[sz]));
	} else if (pathLetter=='B') {
		size_t sz = sizeof(SpecialPathB) / sizeof(SpecialPathB[0]);
		return ConvertToWaypoints(origin, vector<Point2D>(SpecialPathB, &SpecialPathB[sz]));
	} else {
		throw std::runtime_error("Invalid special path.");
	}
}



} //End anon namespace



//initiate
sim_mob::Driver::Driver(Person* parent, unsigned int reacTime_LeadingVehicle,
		unsigned int reacTime_SubjectVehicle, unsigned int reacTime_Gap) : Role(parent), currLane_(nullptr), currLaneOffset_(0),
	currLaneLength_(0), isInIntersection(false), vehicle(nullptr), perceivedVelocity(reacTime_SubjectVehicle, true),
	perceivedVelocityOfFwdCar(reacTime_LeadingVehicle, true), perceivedAccelerationOfFwdCar(reacTime_LeadingVehicle, true),perceivedDistToFwdCar(reacTime_Gap, true)
{
	if (Debug::Drivers) { DebugStream <<"Driver starting: " <<parent->getId() <<endl; }

	trafficSignal = nullptr;

	//Initialize our models. These should be swapable later.
	lcModel = new MITSIM_LC_Model();
	cfModel = new MITSIM_CF_Model();
	intModel = new SimpleIntDrivingModel();

	//Some one-time flags and other related defaults.
	firstFrameTick = true;
	nextLaneInNextLink = nullptr;
}


///Note that Driver's destructor is only for reclaiming memory.
///  If you want to remove its registered properties from the Worker (which you should do!) then
///  this should occur elsewhere.
sim_mob::Driver::~Driver()
{
	//Our movement models.
	safe_delete(lcModel);
	safe_delete(cfModel);
	safe_delete(intModel);

	//Our vehicle
	safe_delete(vehicle);
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

	//Set to true if we have just moved to a new segment.
	justChangedToNewSegment = false;

	//Will be removed later.
	TEMP_lastKnownPolypoint = DPoint(0, 0);

	//Set to true if we have just moved into an intersection.
	justMovedIntoIntersection = false;

	//If we've just moved into an intersection, is set to the amount of overflow (e.g.,
	//  how far into it we already are.)
	overflowIntoIntersection = 0;
}



void sim_mob::Driver::update_first_frame(UpdateParams& params, frame_t frameNumber)
{
	//Save the path from orign to destination in allRoadSegments
	initializePath();

	//Set some properties about the current path, such as the current polyline, etc.
	if(vehicle && vehicle->hasPath()) {
		setOrigin(params);
	} else {
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout <<"ERROR: Vehicle could not be created for driver; no route!\n";
	}
}



bool sim_mob::Driver::update_sensors(UpdateParams& params, frame_t frameNumber)
{
	//Are we done?
	if (vehicle->isDone()) {
		return false;
	}

	//Save the nearest agents in your lane and the surrounding lanes, stored by their
	// position before/behind you. Save nearest fwd pedestrian too.
	updateNearbyAgents(params);

	//Manage traffic signal behavior if we are close to the end of the link.
	//TODO: This might be slightly inaccurate if a vehicle leaves an intersection
	//      on a particularly short road segment. For now, though, I'm just organizing these
	//      functions with structure in mind, and it won't affect our current network.
	if(!vehicle->getNextSegment()&&!vehicle->isInIntersection()) {
		setTrafficSignalParams(params);
	}

	return true;
}



bool sim_mob::Driver::update_movement(UpdateParams& params, frame_t frameNumber)
{
	//If reach the goal, get back to the origin
	if(vehicle->isDone()){
		//Output
		if (Debug::Drivers && !DebugStream.str().empty()) {
			DebugStream <<">>>Vehicle done." <<endl;
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			std::cout <<DebugStream.str();
			DebugStream.str("");
		}

		return false;
	}

	//Save some values which might not be available later.
	//TODO: LastKnownPolypoint should actually be the _new_ polypoint.
	const RoadSegment* prevSegment = vehicle->getCurrSegment();
	params.TEMP_lastKnownPolypoint = DPoint(vehicle->getCurrPolylineVector().getEndX(), vehicle->getCurrPolylineVector().getEndY());

	//First, handle driving behavior inside an intersection.
	if(vehicle->isInIntersection()) {
		intersectionDriving(params);
	}

	//Next, handle driving on links.
	// Note that a vehicle may leave an intersection during intersectionDriving(), so the conditional check is necessary.
	// Note that there is no need to chain this back to intersectionDriving.
	if(!vehicle->isInIntersection()) {
		//Drive forward. Save how far "over" we go through the intersection.
		params.overflowIntoIntersection = linkDriving(params);

		//Did our last move forward bring us into an intersection?
		if(vehicle->isInIntersection()) {
			params.justMovedIntoIntersection = true;
		}
	}

	//Has the segment changed?
	if (!vehicle->isDone()) {
		params.justChangedToNewSegment = (vehicle->getCurrSegment()!=prevSegment);
	}

	return true;
}


bool sim_mob::Driver::update_post_movement(UpdateParams& params, frame_t frameNumber)
{
	//Are we done?
	if (vehicle->isDone()) {
		return false;
	}

	//Has the segment changed?
	if (!vehicle->isInIntersection() && params.justChangedToNewSegment) {
		//Make pre-intersection decisions?
		if(!vehicle->hasNextSegment(true)) {
			saveCurrTrafficSignal();
			chooseNextLaneForNextLink(params);
		}
	}

	//Have we just entered into an intersection?
	if (vehicle->isInIntersection() && params.justMovedIntoIntersection) {
		//Calculate a trajectory and init movement on that intersection.
		calculateIntersectionTrajectory(params.TEMP_lastKnownPolypoint, params.overflowIntoIntersection);
		intersectionVelocityUpdate();

		//Fix: We need to perform this calculation at least once or we won't have a heading within the intersection.
		DPoint res = intModel->continueDriving(0);
		vehicle->setPositionInIntersection(res.x, res.y);
	}

	return true;
}



//Main update functionality
void sim_mob::Driver::update(frame_t frameNumber)
{
	//Convert the current time to ms
	currTimeMS = frameNumber * ConfigParams::GetInstance().baseGranMS;

	//Do nothing?
	if(currTimeMS<parent->getStartTime()) {
#ifndef DISABLE_DYNAMIC_DISPATCH
		std::stringstream msg;
		msg <<"Driver(" <<parent->getId() <<") specifies a start time of: " <<parent->getStartTime() <<" but it is currently: "
			<<currTimeMS <<"; this indicates an error, and should be handled automatically.";
		throw std::runtime_error(msg.str().c_str());
#else
		return;
#endif
	}

	//Create a new set of local parameters for this frame update.
	UpdateParams params(*this);

	//First frame update
	if (firstFrameTick) {
		//Helper check; not needed once we trust our Workers.
#ifndef DISABLE_DYNAMIC_DISPATCH
		if (abs(currTimeMS-parent->getStartTime())>=ConfigParams::GetInstance().baseGranMS) {
			std::stringstream msg;
			msg <<"Driver was not started within one timespan of its requested start time.";
			msg <<"\nStart was: " <<parent->getStartTime() <<",  Curr time is: " <<currTimeMS <<"\n";
			msg <<"Agent ID: " <<parent->getId() <<"\n";
			throw std::runtime_error(msg.str().c_str());
		}
#endif

		update_first_frame(params, frameNumber);
		firstFrameTick = false;
	}

	//Are we done already?
	if (vehicle->isDone()) {
#ifndef DISABLE_DYNAMIC_DISPATCH
		if (parent->isToBeRemoved()) {
			throw std::runtime_error("Driver is already done, but hasn't been removed.");
		}
		parent->setToBeRemoved();
#else
		return;
#endif
	}

	//Just a bit glitchy...
	updateAdjacentLanes(params);

	//retrieved their current "sensed" values.
	if (perceivedVelocity.can_sense(currTimeMS)) {
		params.perceivedFwdVelocity = perceivedVelocity.sense(currTimeMS)->x;
		params.perceivedLatVelocity = perceivedVelocity.sense(currTimeMS)->y;
	}
	//General update behavior.
	//Note: For now, most updates cannot take place unless there is a Lane and vehicle.
	if (params.currLane && vehicle) {
		if (   update_sensors(params, frameNumber)
			&& update_movement(params, frameNumber)
			&& update_post_movement(params, frameNumber)) {

			//Update parent data. Only works if we're not "done" for a bad reason.
			setParentBufferedData();
		}
	}

	//Update our Buffered types
	//TODO: Update parent buffered properties, or perhaps delegate this.
//	currLane_.set(params.currLane);
//	currLaneOffset_.set(params.currLaneOffset);
//	currLaneLength_.set(params.currLaneLength);
	if(!vehicle->isInIntersection())
	{
		currLane_.set(vehicle->getCurrLane());
		currLaneOffset_.set(vehicle->getDistanceMovedInSegment());
		currLaneLength_.set(vehicle->getCurrLinkLaneZeroLength());
	}
	isInIntersection.set(vehicle->isInIntersection());
	//Update your perceptions
	perceivedVelocity.delay(new DPoint(vehicle->getVelocity(), vehicle->getLatVelocity()), currTimeMS);
	//Print output for this frame.
	if (!vehicle->isDone()) {
		output(frameNumber);
	}
}

void sim_mob::Driver::output(frame_t frameNumber)
{
	//Get angle
	double baseAngle = vehicle->isInIntersection() ? intModel->getCurrentAngle() : vehicle->getAngle();

	LogOut("(\"Driver\""
			<<","<<frameNumber
			<<","<<parent->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(vehicle->getX())
			<<"\",\"yPos\":\""<<static_cast<int>(vehicle->getY())
			<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
			<<"\",\"length\":\""<<static_cast<int>(vehicle->length)
			<<"\",\"width\":\""<<static_cast<int>(vehicle->width)
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
	DPoint res = intModel->continueDriving(vehicle->getVelocity() * p.elapsedSeconds);
	vehicle->setPositionInIntersection(res.x, res.y);

	//Next, detect if we've just left the intersection. Otherwise, perform regular intersection driving.
	if(intModel->isDone()) {
		p.currLane = vehicle->moveToNextSegmentAfterIntersection();
		justLeftIntersection(p);
	}
}

//vehicle movement on link, perform acceleration, lane changing if necessary
//the movement is based on relative position
double sim_mob::Driver::linkDriving(UpdateParams& p)
{
	//TODO: This might not be in the right location
	p.dis2stop = vehicle->getAllRestRoadSegmentsLength() - vehicle->getDistanceMovedInSegment() - vehicle->length/2 - 300;
	p.dis2stop /= 100;

	//Check if we should change lanes.
	double newLatVel = lcModel->executeLaneChanging(p, vehicle->getAllRestRoadSegmentsLength(), vehicle->length, getCurrLaneChangeDirection());
	if (newLatVel>=0.0) {
		vehicle->setLatVelocity(newLatVel);
	}
	//when vehicle stops, don't do lane changing
	if(vehicle->getVelocity()<=0)
	{
		vehicle->setLatVelocity(0);
	}

	//Retrieve a new acceleration value.
	double newFwdAcc = 0;

	//Convert back to m/s
	//TODO: Is this always m/s? We should rename the variable then...
	p.currSpeed = vehicle->getVelocity()/100;
	//Call our model
	newFwdAcc = cfModel->makeAcceleratingDecision(p, targetSpeed, maxLaneSpeed);


	//Update our chosen acceleration; update our position on the link.
	vehicle->setAcceleration(newFwdAcc * 100);
	return updatePositionOnLink(p);
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


bool sim_mob::Driver::isPedestrianOnTargetCrossing() const
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
/*void sim_mob::Driver::updateCurrLaneLength(UpdateParams& p)
{
	p.currLaneLength = vehicle->getCurrPolylineVector().getMagnitude();
}*/

//update left and right lanes of the current lane
//if there is no left or right lane, it will be null
void sim_mob::Driver::updateAdjacentLanes(UpdateParams& p)
{
	//Need to reset, we can call this after UpdateParams is initialized.
	p.leftLane = nullptr;
	p.rightLane = nullptr;

	if (!p.currLane) {
		return; //Can't do anything without a lane to reference.
	}

	if(p.currLaneIndex>0) {
		p.rightLane = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex-1);
	}
	if(p.currLaneIndex < p.currLane->getRoadSegment()->getLanes().size()-1) {
		p.leftLane = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex+1);
	}
}


//General update information for whenever a Segment may have changed.
void sim_mob::Driver::syncCurrLaneCachedInfo(UpdateParams& p)
{
	//The lane may have changed; reset the current lane index.
	p.currLaneIndex = getLaneIndex(p.currLane);

	//Update which lanes are adjacent.
	updateAdjacentLanes(p);

	//Update the length of the current road segment.
	p.currLaneLength = vehicle->getCurrLinkLaneZeroLength();

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

		//We should have generated a nextLaneInNextLink here.
		if (!nextLaneInNextLink) {
			throw std::runtime_error("Can't find nextLaneInNextLink.");
		}
	}
}

//TODO: For now, we're just using a simple trajectory model. Complex curves may be added later.
void sim_mob::Driver::calculateIntersectionTrajectory(DPoint movingFrom, double overflow)
{
	//If we have no target link, we have no target trajectory.
	if (!nextLaneInNextLink) {
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout <<"WARNING: nextLaneInNextLink has not been set; can't calculate intersection trajectory." <<std::endl;
		return;
	}

	//Get the entry point.
	Point2D entry = nextLaneInNextLink->getPolyline().at(0);

	//Compute a movement trajectory.
	intModel->startDriving(movingFrom, DPoint(entry.getX(), entry.getY()), overflow);
}


//link path should be retrieved from other class
//for now, it serves as this purpose
void sim_mob::Driver::initializePath()
{
	//Save local copies of the parent's origin/destination nodes.
	origin.node = parent->originNode;
	origin.point = *origin.node->location;
	goal.node = parent->destNode;
	goal.point = *goal.node->location;

	//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
	vector<WayPoint> path;
	Person* parentP = dynamic_cast<Person*>(parent);
	if (!parentP || parentP->specialStr.empty()) {
		path = StreetDirectory::instance().shortestDrivingPath(*origin.node, *goal.node);
	} else {
		//In special cases, we may be manually specifying a loop, e.g., "loop:A:5" in the special string.
		size_t cInd = parentP->specialStr.find(':');
		if (cInd!=string::npos && cInd+1<parentP->specialStr.length()) {
			//Repeat this path X times.
			vector<WayPoint> part = LoadSpecialPath(parent->originNode, parentP->specialStr[cInd+1]);

			cInd = parentP->specialStr.find(':', cInd+1);
			if (cInd!=string::npos && cInd+1<parentP->specialStr.length()) {
				int amount = -1;
				std::istringstream(parentP->specialStr.substr(cInd+1, string::npos)) >> amount;

				for (size_t i=0; static_cast<int>(i)<amount && amount>1; i++) {
					path.insert(path.end(), part.begin(), part.end());
				}
			}

			//Just in case one of these failed.
			if (path.empty()) {
				path = part;
			}
		}
	}

	//A non-null vehicle means we are moving.
	//TODO: Start in lane 0?
	vehicle = new Vehicle(path, 0);
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
	//updateAdjacentLanes(p);

	//Calculate and save the total length of the current polyline.
	p.currLaneLength = vehicle->getCurrLinkLaneZeroLength();

	//TEMP: Is this in the right place?
	chooseNextLaneForNextLink(p);
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

double sim_mob::Driver::updatePositionOnLink(UpdateParams& p)
{
	//Determine how far forward we've moved.
	//TODO: I've disabled the acceleration component because it doesn't really make sense.
	//      Please re-enable if you think this is expected behavior. ~Seth
	double fwdDistance = vehicle->getVelocity()*p.elapsedSeconds + 0.5*vehicle->getAcceleration()*p.elapsedSeconds*p.elapsedSeconds;
	if(fwdDistance<0)
		fwdDistance = 0;
	//double fwdDistance = vehicle->getVelocity()*p.elapsedSeconds;
	double latDistance = vehicle->getLatVelocity()*p.elapsedSeconds;

	//Increase the vehicle's velocity based on its acceleration.
	vehicle->setVelocity(vehicle->getVelocity() + vehicle->getAcceleration()*p.elapsedSeconds);

	//TEMP: For ns3
	Person* parentP = dynamic_cast<Person*>(parent);
	if (parentP && !parentP->specialStr.empty() && parentP->specialStr.at(5)=='A') {
		vehicle->setVelocity(vehicle->getVelocity()*1.1);
	}

	//when v_lead and a_lead is 0, space is not negative, the Car Following will generate an acceleration based on free flowing model
	//this causes problem, so i manually set acceleration and velocity to 0
	if (vehicle->getVelocity() < 0|| p.space<0){//||(p.space<1&&p.v_lead==0&&p.a_lead==0)) {
		//Set to 0 forward velocity, no acceleration.
		vehicle->setVelocity(0.0);
		vehicle->setAcceleration(0);
	}

	//Move the vehicle forward.
	double res = 0.0;
	try {
		res = vehicle->moveFwd(fwdDistance);
	} catch (std::exception& ex) {
		if (Debug::Drivers) {
			DebugStream <<">>>Exception: " <<ex.what() <<endl;
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			std::cout <<DebugStream.str();
		}

		std::stringstream msg;
		msg <<"Error moving vehicle forward for Agent ID: " <<parent->getId() <<"\n" <<ex.what();
		throw std::runtime_error(msg.str().c_str());
	}

	//Retrieve what direction we're moving in, since it will "flip" if we cross the relative X axis.
	LANE_CHANGE_SIDE relative = getCurrLaneSideRelativeToCenter();

	//Lateral movement
	if (latDistance!=0) {
		vehicle->moveLat(latDistance);
		updatePositionDuringLaneChange(p, relative);
	}

	//Update our offset in the current lane.
	if (!vehicle->isInIntersection()) {
		p.currLaneOffset = vehicle->getDistanceMovedInSegment();
	}
	return res;
}


//Helper function: check if a modified distanc is less than the current minimum and save it.
void sim_mob::Driver::check_and_set_min_car_dist(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other)
{
	//Subtract the size of the car from the distance between them
	distance = fabs(distance) - veh->length/2 - other->getVehicleLength()/2;
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
	if (!other_lane) {
		return;
	}
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
			int distance = other_offset + params.currLaneLength - params.currLaneOffset;

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
			int distance = other_driver->currLaneLength_.get() - other_offset + params.currLaneOffset;

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
	//Update your perceptions for leading vehicle and gap
	if(params.nvFwd.distance!=5000)
	{
		perceivedVelocityOfFwdCar.delay(new DPoint(params.nvFwd.driver->getVehicle()->getVelocity(),params.nvFwd.driver->getVehicle()->getLatVelocity()),currTimeMS);
		perceivedAccelerationOfFwdCar.delay(params.nvFwd.driver->getVehicle()->getAcceleration(),currTimeMS);
		perceivedDistToFwdCar.delay(params.nvFwd.distance,currTimeMS);
	}
	//retrieve perceptions
	if(perceivedVelocityOfFwdCar.can_sense(currTimeMS)){
		params.perceivedFwdVelocityOfFwdCar = perceivedVelocityOfFwdCar.sense(currTimeMS)->x;
		params.perceivedLatVelocityOfFwdCar = perceivedVelocityOfFwdCar.sense(currTimeMS)->y;
	}
	if(perceivedAccelerationOfFwdCar.can_sense(currTimeMS))
		params.perceivedAccelerationOfFwdCar = perceivedAccelerationOfFwdCar.sense(currTimeMS);
	if(perceivedDistToFwdCar.can_sense(currTimeMS))
		params.perceivedDistToFwdCar = perceivedDistToFwdCar.sense(currTimeMS);
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
	p.currLaneIndex = getLaneIndex(p.currLane);
	vehicle->moveToNewLanePolyline(p.currLaneIndex);
	syncCurrLaneCachedInfo(p);
	p.currLaneOffset = vehicle->getDistanceMovedInSegment();
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
void sim_mob::Driver::updatePositionDuringLaneChange(UpdateParams& p, LANE_CHANGE_SIDE relative)
{
	double halfLaneWidth = p.currLane->getWidth() / 2.0;

	//The direction we are attempting to change lanes in
	LANE_CHANGE_SIDE actual = getCurrLaneChangeDirection();
	//LANE_CHANGE_SIDE relative = getCurrLaneSideRelativeToCenter();
	if (actual==LCS_SAME) {
		if (Debug::Drivers) { DebugStream <<"  Lane change: " <<"ERROR: Called with \"same\"" <<endl; }
		return;  //Not actually changing lanes.
	}
	if (relative==LCS_SAME) {
		relative=actual; //Unlikely to occur, but we can still work off this.
	}
	if (actual==relative) { //We haven't merged halfway yet; check there's actually a lane for us to merge into.
		if ((actual==LCS_LEFT && !p.leftLane) || (actual==LCS_RIGHT && !p.rightLane)) {
			throw std::runtime_error("Attempting to merge into a lane that doesn't exist.");
			//return; //Error condition
		}
	}

	if (Debug::Drivers) { DebugStream <<"  Lane change: " <<PrintLCS(actual) <<" (" <<PrintLCS(relative) <<")" <<endl; }

	//Basically, we move "halfway" into the next lane, and then move "halfway" back to its midpoint.
	if (actual==relative) {
		//Moving "out".
		double remainder = fabs(vehicle->getLateralMovement()) - halfLaneWidth;

		if (Debug::Drivers) { DebugStream <<"    Moving out on Lane " <<p.currLaneIndex <<": " <<remainder <<endl; }

		if (remainder>=0) {
			//Update Lanes, polylines, RoadSegments, etc.
			p.currLane = (actual==LCS_LEFT?p.leftLane:p.rightLane);
			syncCurrLaneCachedInfo(p);
			vehicle->shiftToNewLanePolyline(actual==LCS_LEFT);

			if (Debug::Drivers) { DebugStream <<"    Shifting to new lane." <<endl; }

			//Check
			if (p.currLane->is_pedestrian_lane()) {
				//Flush debug output (we are debugging this error).
				if (Debug::Drivers) {
					DebugStream <<">>>Exception: Moved to sidewalk." <<endl;
					boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
					std::cout <<DebugStream.str();
				}

				//TEMP OVERRIDE:
				//TODO: Fix!
				parent->setToBeRemoved();
				return;

				std::stringstream msg;
				msg <<"Error: Car has moved onto sidewalk. Agent ID: " <<parent->getId();
				throw std::runtime_error(msg.str().c_str());
			}

			//Set to the far edge of the other lane, minus any extra amount.
			halfLaneWidth = p.currLane->getWidth() / 2.0;
			vehicle->resetLateralMovement();
			vehicle->moveLat((halfLaneWidth - remainder) * (actual==LCS_LEFT?-1:1));
		}
	} else {
		//Moving "in".
		bool pastZero = (actual==LCS_LEFT) ? (vehicle->getLateralMovement()>0) : (vehicle->getLateralMovement()<0);

		if (Debug::Drivers) { DebugStream <<"    Moving in on lane " <<p.currLaneIndex <<": " <<vehicle->getLateralMovement() <<endl; }

		if (pastZero) {
			//Reset all
			vehicle->resetLateralMovement();
			vehicle->setLatVelocity(0);

			if (Debug::Drivers) { DebugStream <<"    New lane shift complete." <<endl; }
		}

	}
}


//Retrieve the current traffic signal based on our RoadSegment's end node.
void sim_mob::Driver::saveCurrTrafficSignal()
{
	const Node* node = vehicle->getCurrSegment()->getEnd();
	trafficSignal = node ? StreetDirectory::instance().signalAt(*node) : nullptr;
}

void sim_mob::Driver::setTrafficSignalParams(UpdateParams& p) const
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
				p.trafficSignalStopDistance = vehicle->getAllRestRoadSegmentsLength() - vehicle->getDistanceMovedInSegment() - vehicle->length/2;
				break;

			case Signal::Green:
				if(!isPedestrianOnTargetCrossing())
					p.isTrafficLightStop = false;
				else
					p.isTrafficLightStop = true;
				break;
		}
	}
}
