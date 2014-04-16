//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverFacets.hpp"
#include "BusDriver.hpp"
#include <limits>
#include <algorithm>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Person.hpp"
#include "entities/AuraManager.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/PathSetManager.hpp"
#include "geospatial/RoadRunnerRegion.hpp"
#include "network/CommunicationDataManager.hpp"

#include "boost/bind.hpp"

using namespace sim_mob;
using std::vector;
using std::set;
using std::string;
using std::endl;

//Helper functions
namespace {
//Helpful constants
const int distanceCheckToChangeLane = 150;

//Output helper
string PrintLCS(LANE_CHANGE_SIDE s) {
	if (s == LCS_LEFT) {
		return "LCS_LEFT";
	} else if (s == LCS_RIGHT) {
		return "LCS_RIGHT";
	}
	return "LCS_SAME";
}

//the minimum speed when approaching to incident
const float APPROACHING_SPEED = 200;

//used in lane changing, find the start index and end index of polyline in the target lane
size_t updateStartEndIndex(const std::vector<sim_mob::Point2D>* const currLanePolyLine, double currLaneOffset,
		size_t defaultValue) {
	double offset = 0;
	for (size_t i = 0; i < currLanePolyLine->size() - 1; i++) {
		double xOffset = currLanePolyLine->at(i + 1).getX() - currLanePolyLine->at(i).getX();
		double yOffset = currLanePolyLine->at(i + 1).getY() - currLanePolyLine->at(i).getY();
		offset += sqrt(xOffset * xOffset + yOffset * yOffset);
		if (offset >= currLaneOffset) {
			return i;
		}
	}
	return defaultValue;
}

//TODO:I think lane index should be a data member in the lane class
size_t getLaneIndex(const Lane* l) {
	if (l) {
		const RoadSegment* r = l->getRoadSegment();
		for (size_t i = 0; i < r->getLanes().size(); i++) {
			if (r->getLanes().at(i) == l) {
				return i;
			}
		}
	}
	return -1; //NOTE: This might not do what you expect! ~Seth
}


} //End anon namespace

namespace sim_mob {
DriverBehavior::DriverBehavior(sim_mob::Person* parentAgent):
	BehaviorFacet(parentAgent), parentDriver(nullptr) {}

DriverBehavior::~DriverBehavior() {}

void DriverBehavior::frame_init() {
	throw std::runtime_error("DriverBehavior::frame_init is not implemented yet");
}

void DriverBehavior::frame_tick() {
	throw std::runtime_error("DriverBehavior::frame_tick is not implemented yet");
}

void DriverBehavior::frame_tick_output() {
	throw std::runtime_error("DriverBehavior::frame_tick_output is not implemented yet");
}

sim_mob::DriverMovement::DriverMovement(sim_mob::Person* parentAgent):
	MovementFacet(parentAgent), parentDriver(nullptr)
{
	if (Debug::Drivers) {
		DebugStream <<"Driver starting: ";
		if (parentAgent) { DebugStream <<parentAgent->getId(); } else { DebugStream <<"<null>"; }
		DebugStream <<endl;
	}
	trafficSignal = nullptr;
	//vehicle = nullptr;
	lastIndex = -1;

//	if (parentAgent) {
//		ReactionTimeDist* r1 = ConfigParams::GetInstance().reactDist1;
//		ReactionTimeDist* r2 = ConfigParams::GetInstance().reactDist2;
//		if (r1 && r2) {
//			reacTime = r1->getReactionTime() + r2->getReactionTime();
//			reacTime = 0;
//		} else {
//			throw std::runtime_error("Reaction time distributions have not been initialized yet.");
//		}
//	}

	//Initialize our models. These should be swapable later.
	lcModel = new MITSIM_LC_Model();
	cfModel = new MITSIM_CF_Model();
	intModel = new SimpleIntDrivingModel();

	//Some one-time flags and other related defaults.
	nextLaneInNextLink = nullptr;
	disToFwdVehicleLastFrame = parentDriver->maxVisibleDis;
//	// record start time
//	startTime = parentDriver->getParams().now.ms()/1000.0;
//	isAleadyStarted = false;
}

sim_mob::DriverMovement::~DriverMovement()
{
	//Our movement models.
	safe_delete_item(lcModel);
	safe_delete_item(cfModel);
	safe_delete_item(intModel);
}

void sim_mob::DriverMovement::frame_init() {
	//Save the path from orign to next activity location in allRoadSegments
	Vehicle* newVeh = initializePath(true);
	if (newVeh) {
		safe_delete_item(parentDriver->vehicle);
		parentDriver->vehicle = newVeh;
	}

	//Set some properties about the current path, such as the current polyline, etc.
	if (parentDriver->vehicle && parentDriver->vehicle->hasPath()) {
		setOrigin(parentDriver->getParams());
	} else {
		Warn() << "ERROR: Vehicle[short] could not be created for driver; no route!" <<std::endl ;
	}
}

void sim_mob::DriverMovement::responseIncidentStatus(DriverUpdateParams& p, timeslice now) {
	//slow down velocity when driver views the incident within the visibility distance
	float incidentGap = parentDriver->vehicle->length;
	if(incidentStatus.getSlowdownVelocity()){
		//calculate the distance to the nearest front vehicle, if no front vehicle exists, the distance is given to a enough large gap as 5 kilometers
		float fwdCarDist = 5000;
		if( p.nvFwd.exists() ){
			DPoint dFwd = p.nvFwd.driver->getVehicle()->getPosition();
			DPoint dCur = parentDriver->vehicle->getPosition();
			DynamicVector movementVect(dFwd.x, dFwd.y, dCur.x, dCur.y);
			fwdCarDist = movementVect.getMagnitude()-parentDriver->vehicle->length;
			if(fwdCarDist < 0) {
				fwdCarDist = parentDriver->vehicle->length;
			}
		}

		//record speed limit for current vehicle
		float speedLimit = 0;
		//record current speed
		float newSpeed = 0;
		//record approaching speed when it is near to incident position
		float approachingSpeed = APPROACHING_SPEED;
		float oldDistToStop = p.perceivedDistToFwdCar;
		LANE_CHANGE_SIDE oldDirect = p.turningDirection;
		p.perceivedDistToFwdCar = std::min(incidentStatus.getDistanceToIncident(), fwdCarDist);
		p.turningDirection = LCS_LEFT;

		//retrieve speed limit decided by whether or not incident lane or adjacent lane
		speedLimit = incidentStatus.getSpeedLimit(p.currLaneIndex);
		if(speedLimit==0 && incidentStatus.getDistanceToIncident()>incidentGap)
			speedLimit = approachingSpeed;

		// recalculate acceleration and velocity when incident happen
		float newFwdAcc = 0;
		if(parentDriver->vehicle->getVelocity() > speedLimit){
			newFwdAcc = cfModel->makeAcceleratingDecision(p, speedLimit, maxLaneSpeed);
			newSpeed = parentDriver->vehicle->getVelocity()+newFwdAcc*p.elapsedSeconds*100;
			if(newSpeed < speedLimit){
				newFwdAcc = 0;
				newSpeed = speedLimit;
			}
		}
		else {
			newFwdAcc = 0;
			newSpeed = speedLimit;
		}

		//update current velocity so as to response the speed limit defined in incident lane.
		parentDriver->vehicle->setVelocity(newSpeed);
		p.perceivedDistToFwdCar = oldDistToStop;
		p.turningDirection = oldDirect;
	}

	//stop cars when it already is near the incident location
	if(incidentStatus.getCurrentStatus() == IncidentStatus::INCIDENT_OCCURANCE_LANE ){
		if(incidentStatus.getSpeedLimit(p.currLaneIndex)==0 && incidentStatus.getDistanceToIncident()<incidentGap) {
			parentDriver->vehicle->setVelocity(0);
			parentDriver->vehicle->setAcceleration(0);
		}
	}

	if(p.nvFwd.exists() ){//avoid cars stacking together
		DPoint dFwd = p.nvFwd.driver->getVehicle()->getPosition();
		DPoint dCur = parentDriver->vehicle->getPosition();
		DynamicVector movementVect(dFwd.x, dFwd.y, dCur.x, dCur.y);
		double len = parentDriver->getVehicle()->length;
		double dist = movementVect.getMagnitude();
		if( dist < len){
			parentDriver->vehicle->setVelocity(0);
			parentDriver->vehicle->setAcceleration(0);
		}
	}
}

void sim_mob::DriverMovement::checkIncidentStatus(DriverUpdateParams& p, timeslice now) {

	const RoadSegment* curSegment = parentDriver->vehicle->getCurrSegment();
	const Lane* curLane = parentDriver->vehicle->getCurrLane();
	int curLaneIndex = curLane->getLaneID() - curSegment->getLanes().at(0)->getLaneID();
	if(curLaneIndex<0){
		return;
	}

	int nextLaneIndex = curLaneIndex;
	LANE_CHANGE_SIDE laneSide = LCS_SAME;
	IncidentStatus::IncidentStatusType status = IncidentStatus::INCIDENT_CLEARANCE;
	incidentStatus.setDistanceToIncident(0);
	const float convertFactor = 1000.0/3600.0;
	incidentStatus.setDefaultSpeedLimit(curSegment->maxSpeed*convertFactor);

	const std::map<centimeter_t, const RoadItem*> obstacles = curSegment->getObstacles();
	std::map<centimeter_t, const RoadItem*>::const_iterator obsIt;
	double realDist = 0;
	bool replan = false;
	const RoadItem* roadItem = getRoadItemByDistance(sim_mob::INCIDENT, realDist);
	if(roadItem) {//retrieve front incident obstacle
		const Incident* incidentObj = dynamic_cast<const Incident*>( roadItem );

		if(incidentObj){
			float visibility = incidentObj->visibilityDistance;
			incidentStatus.setVisibilityDistance(visibility);
			incidentStatus.setCurrentLaneIndex(curLaneIndex);

			if( (now.ms() >= incidentObj->startTime) && (now.ms() < incidentObj->startTime+incidentObj->duration) && realDist<visibility){
				incidentStatus.setDistanceToIncident(realDist);
				replan = incidentStatus.insertIncident(incidentObj);
				float incidentGap = parentDriver->vehicle->length*2;
				if(!incidentStatus.getChangedLane() && incidentStatus.getCurrentStatus()==IncidentStatus::INCIDENT_OCCURANCE_LANE){
					double prob = incidentStatus.getVisibilityDistance()>0 ? incidentStatus.getDistanceToIncident()/incidentStatus.getVisibilityDistance() : 0.0;
					if(incidentStatus.getDistanceToIncident() < 2*incidentGap){
						incidentStatus.setChangedLane(true);
					}
					else {
						if(prob < incidentStatus.getRandomValue()) {
							incidentStatus.setChangedLane(true);
						}
					}
				}
			}
			else if( now.ms()>incidentObj->startTime+incidentObj->duration ){// if incident duration is over, the incident obstacle will be removed
				replan = incidentStatus.removeIncident(incidentObj);
			}
		}
	}
	else {//if vehicle is going beyond this incident obstacle, this one will be removed
		for(obsIt=obstacles.begin(); obsIt!=obstacles.end(); obsIt++){
			const Incident* inc = dynamic_cast<const Incident*>( (*obsIt).second );
			if(inc){
				replan = incidentStatus.removeIncident(inc);
			}
		}
	}

	if(replan){//update decision status for incident.
		incidentStatus.checkIsCleared();
	}
}


void sim_mob::DriverMovement::setRR_RegionsFromCurrentPath()
{
	if (parent->getRegionSupportStruct().isEnabled()) {
		if (parentDriver->vehicle) {
			std::vector<const sim_mob::RoadSegment*> path = parentDriver->vehicle->getPath();
			if (!path.empty()) {
				//We may be partly along this route, but it is unlikely. Still, just to be safe...
				const sim_mob::RoadSegment* currseg = parentDriver->vehicle->getCurrSegment();

				//Now save it, taking into account the "current segment"
				rrPathToSend.clear();
				for (std::vector<const sim_mob::RoadSegment*>::const_iterator it=path.begin(); it!=path.end(); it++) {
					//Have we reached our starting segment yet?
					if (currseg) {
						if (currseg == *it) {
							//Signal this by setting currseg to null.
							currseg = nullptr;
						} else {
							continue;
						}
					}

					//Add it; we've cleared our current segment check one way or another.
					rrPathToSend.push_back(*it);
				}
			}
		}
	}
}


void sim_mob::DriverMovement::frame_tick()
{
	// lost some params
	DriverUpdateParams& p2 = parentDriver->getParams();

	if(!(parentDriver->vehicle)) {
		throw std::runtime_error("Something wrong, Vehicle is NULL");
	}

	//Are we done already?
	if (parentDriver->vehicle->isDone()) {
		if(parent->schedules.size()>1){
			parent->schedules.pop_front();
			std::vector<Node*>& routes = parent->schedules.front().routes;
			std::vector<Node*>::iterator first = routes.begin();
			std::vector<Node*>::iterator second = first;

			vector<WayPoint> path;
			const StreetDirectory& stdir = StreetDirectory::instance();
			for(second++; first!=routes.end() && second!=routes.end(); first++, second++){
				vector<WayPoint> subPath = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(**first), stdir.DrivingVertex(**second));
				path.insert( path.end(), subPath.begin(), subPath.end());
			}
			parentDriver->vehicle->resetPath(path);
		}
		else{
			getParent()->setToBeRemoved();
		}

		return;
	}

	//Specific for Region support.
	if (parent->getRegionSupportStruct().isEnabled()) {
		//Currently all_regions only needs to be sent once.
		if (sentAllRegions.check()) {
			//Send the Regions.
			std::vector<RoadRunnerRegion> allRegions;
			const RoadNetwork& net = ConfigManager::GetInstance().FullConfig().getNetwork();
			for (std::map<int, RoadRunnerRegion>::const_iterator it=net.roadRunnerRegions.begin(); it!=net.roadRunnerRegions.end(); it++) {
				allRegions.push_back(it->second);
			}
			parent->getRegionSupportStruct().setNewAllRegionsSet(allRegions);

			//If a path has already been set, we will need to transmit it.
			setRR_RegionsFromCurrentPath();
		}

		//We always need to send a path if one is available.
		if (!rrPathToSend.empty()) {
			std::vector<RoadRunnerRegion> regPath;
			for (std::vector<const RoadSegment*>::const_iterator it=rrPathToSend.begin(); it!=rrPathToSend.end(); it++) {
				//Determine if this road segment is within a Region.
				std::pair<RoadRunnerRegion, bool> rReg = StreetDirectory::instance().getRoadRunnerRegion(*it);
				if (rReg.second) {
					//Don't add if it's the last item in the list.
					if (regPath.empty() || (regPath.back().id != rReg.first.id)) {
						regPath.push_back(rReg.first);
					}
				}
			}

			parent->getRegionSupportStruct().setNewRegionPath(regPath);
			rrPathToSend.clear();
		}
	}

	//Just a bit glitchy...
	updateAdjacentLanes(p2);

	//Update "current" time
	parentDriver->perceivedFwdVel->update(parentDriver->getParams().now.ms());
	parentDriver->perceivedFwdAcc->update(parentDriver->getParams().now.ms());
	parentDriver->perceivedDistToFwdCar->update(parentDriver->getParams().now.ms());
	parentDriver->perceivedVelOfFwdCar->update(parentDriver->getParams().now.ms());
	parentDriver->perceivedAccOfFwdCar->update(parentDriver->getParams().now.ms());
	parentDriver->perceivedTrafficColor->update(parentDriver->getParams().now.ms());
	parentDriver->perceivedDistToTrafficSignal->update(parentDriver->getParams().now.ms());

	//retrieved their current "sensed" values.
	if (parentDriver->perceivedFwdVel->can_sense()) {
		p2.perceivedFwdVelocity = parentDriver->perceivedFwdVel->sense();
	}
	else
		p2.perceivedFwdVelocity = parentDriver->vehicle->getVelocity();

	//General update behavior.
	//Note: For now, most updates cannot take place unless there is a Lane and vehicle.
	if (p2.currLane && parentDriver->vehicle) {

		if (update_sensors(p2.now) && update_movement(p2.now) && update_post_movement(p2.now)) {

			//Update parent data. Only works if we're not "done" for a bad reason.
			setParentBufferedData();
		}
	}

	//Update our Buffered types
	//TODO: Update parent buffered properties, or perhaps delegate this.
	if (!(parentDriver->vehicle->isInIntersection())) {
		parentDriver->currLane_.set(parentDriver->vehicle->getCurrLane());
		parentDriver->currLaneOffset_.set(parentDriver->vehicle->getDistanceMovedInSegment());
		parentDriver->currLaneLength_.set(parentDriver->vehicle->getCurrLinkLaneZeroLength());
	}

	parentDriver->isInIntersection.set(parentDriver->vehicle->isInIntersection());
	parentDriver->latMovement.set(parentDriver->vehicle->getLateralMovement());
	parentDriver->fwdVelocity.set(parentDriver->vehicle->getVelocity());
	parentDriver->latVelocity.set(parentDriver->vehicle->getLatVelocity());
	parentDriver->fwdAccel.set(parentDriver->vehicle->getAcceleration());
	parentDriver->turningDirection.set(parentDriver->vehicle->getTurningDirection());
	//Update your perceptions
	parentDriver->perceivedFwdVel->delay(parentDriver->vehicle->getVelocity());
	parentDriver->perceivedFwdAcc->delay(parentDriver->vehicle->getAcceleration());

	//Print output for this frame.
	disToFwdVehicleLastFrame = p2.nvFwd.distance;

//	std::cout << "parentDriver->vehicle->getX():" << parentDriver->vehicle->getX() << std::endl;
//	std::cout << "parentDriver->vehicle->getY():" << parentDriver->vehicle->getY() << std::endl;

//	this->parent->xPos_Sim = static_cast<int>(parentDriver->vehicle->getX());
//	this->parent->yPos_Sim = static_cast<int>(parentDriver->vehicle->getY());

//	std::cout << "ID:" << this->parent->getId() << ",this->parent->xPos_Sim:" << this->parent->xPos_Sim << std::endl;
//	std::cout << "this->parent->yPos_Sim:" << this->parent->yPos_Sim << std::endl;
}

void sim_mob::DriverMovement::frame_tick_output() {
	DriverUpdateParams &p = parentDriver->getParams();
	//Skip?
	if (parentDriver->vehicle->isDone()) {
		return;
	}
	if (ConfigManager::GetInstance().CMakeConfig().OutputDisabled()) {
		return;
	}

	double baseAngle = parentDriver->vehicle->isInIntersection() ? intModel->getCurrentAngle() : parentDriver->vehicle->getAngle();

	//Inform the GUI if interactive mode is active.
	if (ConfigManager::GetInstance().CMakeConfig().InteractiveMode()) {
		std::ostringstream stream;
		stream<<"DriverSegment"
				<<","<<p.now.frame()
				<<","<<parentDriver->vehicle->getCurrSegment()
				<<","<<parentDriver->vehicle->getCurrentSegmentLength()/100.0;
		std::string s=stream.str();
		ConfigManager::GetInstance().FullConfig().getCommDataMgr().sendTrafficData(s);
	}

	const bool inLane = parentDriver->vehicle && (!parentDriver->vehicle->isInIntersection());

	//MPI-specific output.
	std::stringstream addLine;
	if (ConfigManager::GetInstance().FullConfig().using_MPI) {
		addLine <<"\",\"fake\":\"" <<(this->getParent()->isFake?"true":"false");
	}

	LogOut("(\"Driver\""
			<<","<<p.now.frame()
			<<","<<getParent()->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(parentDriver->vehicle->getX())
			<<"\",\"yPos\":\""<<static_cast<int>(parentDriver->vehicle->getY())
			<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
			<<"\",\"length\":\""<<static_cast<int>(parentDriver->vehicle->length)
			<<"\",\"width\":\""<<static_cast<int>(parentDriver->vehicle->width)
			<<"\",\"curr-segment\":\""<<(inLane?parentDriver->vehicle->getCurrLane()->getRoadSegment():0x0)
			<<"\",\"fwd-speed\":\""<<parentDriver->vehicle->getVelocity()
			<<"\",\"fwd-accel\":\""<<parentDriver->vehicle->getAcceleration()
			<<"\",\"mandatory\":\""<<incidentStatus.getChangedLane()
			<<addLine.str()
			<<"\"})"<<std::endl);
}

bool sim_mob::DriverMovement::update_sensors(timeslice now) {
	DriverUpdateParams& params = parentDriver->getParams();
	//Are we done?
	if (parentDriver->vehicle->isDone()) {
		return false;
	}

	//Save the nearest agents in your lane and the surrounding lanes, stored by their
	// position before/behind you. Save nearest fwd pedestrian too.

	//Manage traffic signal behavior if we are close to the end of the link.
	//TODO: This might be slightly inaccurate if a vehicle leaves an intersection
	//      on a particularly short road segment. For now, though, I'm just organizing these
	//      functions with structure in mind, and it won't affect our current network.
	params.isApproachingToIntersection = false;
	if (!(parentDriver->vehicle->getNextSegment()) && !(parentDriver->vehicle->isInIntersection())) {
		params.isApproachingToIntersection = true;
		setTrafficSignalParams(params);

	}

	updateNearbyAgents();

	return true;
}

bool sim_mob::DriverMovement::update_movement(timeslice now) {
	DriverUpdateParams& params = parentDriver->getParams();
	//If reach the goal, get back to the origin
	if (parentDriver->vehicle->isDone()) {
		//Output
		if (Debug::Drivers && !DebugStream.str().empty()) {
			if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
				DebugStream << ">>>Vehicle done." << endl;
				PrintOut(DebugStream.str());
				DebugStream.str("");
			}
		}

		return false;
	}


	//Save some values which might not be available later.
	//TODO: LastKnownPolypoint should actually be the _new_ polypoint.

	const RoadSegment* prevSegment = parentDriver->vehicle->getCurrSegment();

	params.TEMP_lastKnownPolypoint = DPoint(parentDriver->vehicle->getCurrPolylineVector2().getEndX(),
			parentDriver->vehicle->getCurrPolylineVector2().getEndY());


	//First, handle driving behavior inside an intersection.
	if (parentDriver->vehicle->isInIntersection()) {
		parentDriver->perceivedDistToTrafficSignal->clear();
		parentDriver->perceivedTrafficColor->clear();
		intersectionDriving(params);

		/*double distance = parentDriver->vehicle->getDistanceToSegmentEnd();
		std::cout << "intersection distance is : " << distance << std::endl;

		parentDriver->vehicle->setAcceleration(-5000);
		parentDriver->vehicle->setVelocity(0);
		params.currSpeed = parentDriver->vehicle->getVelocity() / 100;*/

	}

	//Next, handle driving on links.
	// Note that a vehicle may leave an intersection during intersectionDriving(), so the conditional check is necessary.
	// Note that there is no need to chain this back to intersectionDriving.
	if (!(parentDriver->vehicle->isInIntersection())) {
		params.overflowIntoIntersection = linkDriving(params);
		//Did our last move forward bring us into an intersection?
		if (parentDriver->vehicle->isInIntersection()) {
			params.justMovedIntoIntersection = true;
			parentDriver->vehicle->setLatVelocity(0);
			parentDriver->vehicle->setTurningDirection(LCS_SAME);
		}
	}


	//Has the segment changed?
	if ((!(parentDriver->vehicle->isDone())) && ((parentDriver->vehicle->hasPath())) ) {
		params.justChangedToNewSegment = ( (parentDriver->vehicle->getCurrSegment() != prevSegment) );
	}

	//change segment happen, calculate link travel time
	if(params.justChangedToNewSegment == true ){
		const RoadSegment* prevSeg = parentDriver->vehicle->getCurrSegment();
		const Link* prevLink = prevSeg->getLink();
		double actualTime = parentDriver->getParams().elapsedSeconds + (parentDriver->getParams().now.ms()/1000.0);
		//if prevLink is already in travelStats, update it's linkTT and add to travelStatsMap
		Agent* parentAgent = parentDriver->getDriverParent(parentDriver);
		if(prevLink == parentAgent->getLinkTravelStats().link_){
			parentAgent->addToLinkTravelStatsMap(parentAgent->getLinkTravelStats(), actualTime); //in seconds
			//prevSeg->getParentConflux()->setTravelTimes(parentAgent, linkExitTimeSec);
		}
		//creating a new entry in agent's travelStats for the new link, with entry time
		parentAgent->initLinkTravelStats(parentDriver->vehicle->getCurrSegment()->getLink(), actualTime);
	}

	params.TEMP_lastKnownPolypoint = DPoint(parentDriver->vehicle->getCurrPolylineVector2().getEndX(),
				parentDriver->vehicle->getCurrPolylineVector2().getEndY());

	return true;
}

bool sim_mob::DriverMovement::update_post_movement(timeslice now) {
	DriverUpdateParams& params = parentDriver->getParams();
	//Are we done?
	if (parentDriver->vehicle->isDone()) {
		return false;
	}

	//Has the segment changed?
	if (!(parentDriver->vehicle->isInIntersection()) && params.justChangedToNewSegment) {
		//Make pre-intersection decisions?
		if (!(parentDriver->vehicle->hasNextSegment(true)))
		{
			saveCurrTrafficSignal();
//			resetPath(params);
		}
	}

	if (!(parentDriver->vehicle->isInIntersection()) && !(parentDriver->vehicle->hasNextSegment(true)) && parentDriver->vehicle->hasNextSegment(false))
		chooseNextLaneForNextLink(params);

	//Have we just entered into an intersection?
	if (parentDriver->vehicle->isInIntersection() && params.justMovedIntoIntersection) {
		//Calculate a trajectory and init movement on that intersection.
		calculateIntersectionTrajectory(params.TEMP_lastKnownPolypoint, params.overflowIntoIntersection);
		intersectionVelocityUpdate();

		//Fix: We need to perform this calculation at least once or we won't have a heading within the intersection.
		DPoint res = intModel->continueDriving(0);
		parentDriver->vehicle->setPositionInIntersection(res.x, res.y);
	}

	return true;
}

//responsible for vehicle behaviour inside intersection
//the movement is based on absolute position
void sim_mob::DriverMovement::intersectionDriving(DriverUpdateParams& p) {
	//Don't move if we have no target
	if (!nextLaneInNextLink) {
		return;
	}

	//First, update movement along the vector.
	DPoint res = intModel->continueDriving(parentDriver->vehicle->getVelocity() * p.elapsedSeconds);
	parentDriver->vehicle->setPositionInIntersection(res.x, res.y);

	//Next, detect if we've just left the intersection. Otherwise, perform regular intersection driving.
	if (intModel->isDone()) {
		p.currLane = parentDriver->vehicle->moveToNextSegmentAfterIntersection();
		if(lastIndex != -1)
		{
			p.currLane = parentDriver->vehicle->getCurrSegment()->getLane(lastIndex);
			lastIndex = -1;
		}
		justLeftIntersection(p);
	}
}

bool sim_mob::DriverMovement::AvoidCrashWhenLaneChanging(DriverUpdateParams& p)
{
	double distanceRange =500; //currently, set 5m as the range which maybe cause two vehicles crash
	//the subject vehicle isn't doing lane changing
	if(parentDriver->vehicle->getLatVelocity()==0)
		return false;
	//the subject vehicle is changing to left lane
	if(parentDriver->vehicle->getLatVelocity()>0 && p.nvLeftFwd2.exists() && p.nvLeftFwd2.distance < distanceRange
			&& p.nvLeftFwd2.driver->latVelocity.get()<0)
		return true;
	if(parentDriver->vehicle->getLatVelocity()<0 && p.nvRightFwd2.exists() && p.nvRightFwd2.distance < distanceRange
			&& p.nvRightFwd2.driver->latVelocity.get()>0)
		return true;
	return false;
}

//vehicle movement on link, perform acceleration, lane changing if necessary
//the movement is based on relative position
double sim_mob::DriverMovement::linkDriving(DriverUpdateParams& p) {

if ( (parentDriver->getParams().now.ms()/1000.0 - parentDriver->startTime > 10) &&  (parentDriver->vehicle->getDistanceMovedInSegment()>2000) && (parentDriver->isAleadyStarted == false))
	{
	parentDriver->isAleadyStarted = true;
	}
	p.isAlreadyStart = parentDriver->isAleadyStarted;

	if (!(parentDriver->vehicle->hasNextSegment(true))) // has seg in current link
	{
		p.dis2stop = parentDriver->vehicle->getAllRestRoadSegmentsLength() - parentDriver->vehicle->getDistanceMovedInSegment() - parentDriver->vehicle->length / 2 - 300;
		if (p.nvFwd.distance < p.dis2stop)
			p.dis2stop = p.nvFwd.distance;
		p.dis2stop /= 100;
	}
	else
	{
		p.nextLaneIndex = std::min<int>(p.currLaneIndex, parentDriver->vehicle->getNextSegment()->getLanes().size() - 1);
		if(parentDriver->vehicle->getNextSegment()->getLanes().at(p.nextLaneIndex)->is_pedestrian_lane())
		{
			p.nextLaneIndex--;
			p.dis2stop = parentDriver->vehicle->getCurrPolylineLength() - parentDriver->vehicle->getDistanceMovedInSegment() + 1000;
			p.dis2stop /= 100;
		}
		else
			p.dis2stop = 1000;//defalut 1000m
	}

	// check current lane has connector to next link
	p.isMLC = false;
	if(p.dis2stop<distanceCheckToChangeLane) // <150m need check above, ready to change lane
	{
		p.isMLC = true;
////		const RoadSegment* currentSegment = vehicle->getCurrSegment();
		const RoadSegment* nextSegment = parentDriver->vehicle->getNextSegment(false);
		const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (parentDriver->vehicle->getNodeMovingTowards());
		if(currEndNode)
		{
			// get lane connector
			const std::set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(parentDriver->vehicle->getCurrSegment());

			if (lcs.size()>0)
			{
				//
				if(p.currLane->is_pedestrian_lane()) {
					//if can different DEBUG or RELEASE mode, that will be perfect, but now comment it out, so that does nor affect performance.
					//I remember the message is not critical
					WarnOut("drive on pedestrian lane");
				}
				bool currentLaneConnectToNextLink = false;
				int targetLaneIndex=p.currLaneIndex;
				std::map<int,vector<int> > indexes;
				std::set<int> noData;

				for (std::set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++) {
					if ((*it)->getLaneTo()->getRoadSegment() == nextSegment && (*it)->getLaneFrom() == p.currLane) {
						// current lane connect to next link
						currentLaneConnectToNextLink = true;
						p.nextLaneIndex = p.currLaneIndex;
						break;
					}
					//find target lane with same index, use this lane
					if ((*it)->getLaneTo()->getRoadSegment() == nextSegment)
					{
						targetLaneIndex = getLaneIndex((*it)->getLaneFrom());
					}
				}
				if( currentLaneConnectToNextLink == false ) // wow! we need change lane
				{
					//check target lane first
//					if(targetLaneIndex == -1) // no target lane?
//					{
//						p.nextLaneIndex = p.currLaneIndex;
////						std::cout<<"Driver::linkDriving: can't find target lane!"<<std::endl;
//					}
//					else

					p.nextLaneIndex = targetLaneIndex;
					//NOTE: Driver already has a lcModel; we should be able to just use this. ~Seth
					MITSIM_LC_Model* mitsim_lc_model = dynamic_cast<MITSIM_LC_Model*> (lcModel);
					if (mitsim_lc_model) {
						LANE_CHANGE_SIDE lcs = LCS_SAME;
//						lcs = mitsim_lc_model->makeMandatoryLaneChangingDecision(p);
						lcs = mitsim_lc_model->makeMandatoryLaneChangingDecision(p);
						parentDriver->vehicle->setTurningDirection(lcs);
						p.isMLC = true;
					} else {
						throw std::runtime_error("TODO: BusDrivers currently require the MITSIM lc model.");
					}
				}
			} // end of if (!lcs)
		}
	}

	Person* parentP = dynamic_cast<Person*> (parent);
	if(parentP && parentP->schedules.size() && processFMODSchedule(&parentP->schedules.front(), p)){
		parentDriver->vehicle->setAcceleration(0);
		parentDriver->vehicle->setVelocity(0);
		p.currSpeed = parentDriver->vehicle->getVelocity() / 100;
		return updatePositionOnLink(p);
	}

	//check incident status and decide whether or not do lane changing
	LANE_CHANGE_MODE mode = DLC;
	checkIncidentStatus(p, parentDriver->getParams().now);
	if(incidentStatus.getChangedLane() && incidentStatus.getNextLaneIndex()>=0){
		p.nextLaneIndex = incidentStatus.getNextLaneIndex();
		parentDriver->vehicle->setTurningDirection(incidentStatus.getLaneSide());
		mode = MLC;
	}
	else if( (incidentStatus.getCurrentStatus()==IncidentStatus::INCIDENT_ADJACENT_LANE && p.lastChangeMode==MLC )
			|| (incidentStatus.getCurrentStatus()==IncidentStatus::INCIDENT_CLEARANCE && incidentStatus.getCurrentIncidentLength()>0)) {
		p.nextLaneIndex = p.currLaneIndex;
		parentDriver->vehicle->setTurningDirection(LCS_SAME);
		mode = MLC;
	}

	//Check if we should change lanes.
	double newLatVel;
	newLatVel = lcModel->executeLaneChanging(p, parentDriver->vehicle->getAllRestRoadSegmentsLength(), parentDriver->vehicle->length,
			parentDriver->vehicle->getTurningDirection(), mode);

	if(newLatVel>0 && p.nextLaneIndex>0){
		const RoadSegment* curSegment = parentDriver->vehicle->getCurrSegment();
		const Lane* lane = curSegment->getLane(p.nextLaneIndex);
		int laneNum = curSegment->getLanes().size()-1;
		if(lane &&(!lane->is_vehicle_lane() || p.nextLaneIndex>laneNum )) {
			parentDriver->vehicle->setTurningDirection(LCS_SAME);
			parentDriver->vehicle->setLatVelocity(0);
			p.nextLaneIndex = p.currLaneIndex;
		}
	}

	parentDriver->vehicle->setLatVelocity(newLatVel);
	if(parentDriver->vehicle->getLatVelocity()>0)
		parentDriver->vehicle->setTurningDirection(LCS_LEFT);
	else if(parentDriver->vehicle->getLatVelocity()<0)
		parentDriver->vehicle->setTurningDirection(LCS_RIGHT);
	else{
		parentDriver->vehicle->setTurningDirection(LCS_SAME);
		if(p.currLaneIndex == incidentStatus.getNextLaneIndex() && incidentStatus.getCurrentStatus() == IncidentStatus::INCIDENT_OCCURANCE_LANE){
			incidentStatus.setCurrentStatus(IncidentStatus::INCIDENT_ADJACENT_LANE);
			incidentStatus.setChangedLane(false);
		}
	}

	p.turningDirection = parentDriver->vehicle->getTurningDirection();

	//get nearest car, if not making lane changing, the nearest car should be the leading car in current lane.
	//if making lane changing, adjacent car need to be taken into account.
	NearestVehicle & nv = nearestVehicle(p);

	if ( parentDriver->isAleadyStarted == false )
	{
		if(nv.distance<=0)
		{
			if(nv.driver->parent->getId() > getParent()->getId())
			{
				nv = NearestVehicle();
			}
		}
	}


	perceivedDataProcess(nv, p);

	//Retrieve a new acceleration value.
	double newFwdAcc = 0;

	//Convert back to m/s
	//TODO: Is this always m/s? We should rename the variable then...
	p.currSpeed = parentDriver->vehicle->getVelocity() / 100;
	//Call our model


	newFwdAcc = cfModel->makeAcceleratingDecision(p, targetSpeed, maxLaneSpeed);
	if(abs(parentDriver->vehicle->getTurningDirection() != LCS_SAME) && newFwdAcc>0 && parentDriver->vehicle->getVelocity() / 100>10)
	{
		newFwdAcc = 0;
	}

	//Update our chosen acceleration; update our position on the link.
	parentDriver->vehicle->setAcceleration(newFwdAcc * 100);

	//response incident
	responseIncidentStatus(p, parentDriver->getParams().now);

	return updatePositionOnLink(p);
}

void sim_mob::DriverMovement::assignNewFMODSchedule(const sim_mob::FMOD_RequestEventArgs& request)
{
	const std::list<FMOD_Schedule>& schedules = request.schedules;
}

bool sim_mob::DriverMovement::processFMODSchedule(FMOD_Schedule* schedule, DriverUpdateParams& p)
{
	bool ret = false;
	if(schedule) // check whether need stop here
	{
		const RoadSegment* currSegment = parentDriver->vehicle->getCurrSegment();
		const Node* stop = currSegment->getEnd();
		bool isFound = false;
		static int count = 0;
		double dwellTime = 0;
		double distance = parentDriver->vehicle->getDistanceToSegmentEnd();

		//judge whether near to stopping node
		const int stopRegion = 800;
		if( distance<stopRegion ){

			//std::cout << "distance (to node id : "<< stop->getID() << " ) is : " << distance << std::endl;

			for(int i = 0; i<schedule->stopSchdules.size(); i++){

				FMOD_Schedule::Stop& stopSchedule = schedule->stopSchdules[i];

				if( stopSchedule.stopId==stop->getID()){

					isFound = true;
					dwellTime = stopSchedule.dwellTime;

					//arrive at scheduling node
					if(dwellTime==0){

						parentDriver->stop_event_type.set(1);
						parentDriver->stop_event_scheduleid.set(schedule->scheduleId);
						parentDriver->stop_event_nodeid.set(stop->getID());

						int passengersnum = stopSchedule.alightingPassengers.size()+stopSchedule.boardingPassengers.size();
						dwellTime = stopSchedule.dwellTime = dwellTimeCalculation(3, 3, 0, 0,0, passengersnum);

						//boarding and alighting
						const RoadSegment* seg = parentDriver->vehicle->getCurrSegment();
						const Node* node = seg->getEnd();
						const Agent* parentAgent = (parentDriver?parentDriver->getParent():nullptr);
					 	vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(Point2D((node->getLocation().getX() - 3500),(node->getLocation().getY() - 3500)),Point2D((node->getLocation().getX() + 3500),(node->getLocation().getY() + 3500)), parentAgent);
					 	for (vector<const Agent*>::iterator it = nearby_agents.begin();it != nearby_agents.end(); it++)
					 	{
							const Person* p = dynamic_cast<const Person*>( (*it) );
							Passenger* passenger = p ? dynamic_cast<Passenger*>(p->getRole()) : nullptr;

							if (!passenger) {
							  continue;
							}

					 		//passenger boarding
							vector<int>& boardingpeople = stopSchedule.boardingPassengers;
							if( std::find(boardingpeople.begin(), boardingpeople.end(), p->client_id ) != boardingpeople.end() )
							{

								schedule->insidePassengers.push_back( p );
								PassengerMovement* passenger_movement = dynamic_cast<PassengerMovement*> (passenger->Movement());
								if(passenger_movement) {
									passenger_movement->PassengerBoardBus_Choice( this->getParentDriver() );
									passenger_movement->alightingMS = 1;
								}
					 	 	}
					 	}

						//alighting
						vector<int>& alightingpeople = stopSchedule.alightingPassengers;
						for( vector<int>::iterator it=alightingpeople.begin(); it!=alightingpeople.end(); it++ )
						{
							vector<const Person*>::iterator itPerson=schedule->insidePassengers.begin();
							while(itPerson!=schedule->insidePassengers.end()){
								if((*it) == (int)(*itPerson)->client_id ){
									Passenger* passenger = dynamic_cast<Passenger*>((*itPerson)->getRole());
									if (!passenger)
										continue;

									PassengerMovement* passenger_movement = dynamic_cast<PassengerMovement*> (passenger->Movement());
									if(passenger_movement) {
										passenger_movement->PassengerAlightBus(this->getParentDriver());
										passenger_movement->alightingMS = 1;
									}

									itPerson = schedule->insidePassengers.erase(itPerson);
								}
								else{
									itPerson++;
								}
							}
						}

						//update shared parameters to record boarding and alighting person
						parentDriver->stop_event_lastAlightingPassengers.set( stopSchedule.alightingPassengers );
						parentDriver->stop_event_lastBoardingPassengers.set( stopSchedule.boardingPassengers );
					}

					// stopping at scheduling node
					dwellTime -= p.elapsedSeconds;
					schedule->stopSchdules[i].dwellTime = dwellTime;

					//depature from this node
					if(dwellTime < 0 ){
						parentDriver->stop_event_type.set(0);
					}
				}
			}
		}

		if(isFound && dwellTime>0.0){
			ret = true;
		}
	}
	return ret;
}


void sim_mob::DriverMovement::setParentBufferedData() {
	getParent()->xPos.set(parentDriver->vehicle->getX());
	getParent()->yPos.set(parentDriver->vehicle->getY());

	//TODO: Need to see how the parent agent uses its velocity vector.
	getParent()->fwdVel.set(parentDriver->vehicle->getVelocity());
	getParent()->latVel.set(parentDriver->vehicle->getLatVelocity());
}
const sim_mob::RoadItem* sim_mob::DriverMovement::getRoadItemByDistance(sim_mob::RoadItemType type,double &itemDis,double perceptionDis,bool isInSameLink)
{
	const sim_mob::RoadItem* res = nullptr;
	itemDis = 0.0;

	if(type != sim_mob::INCIDENT) {
		return res;
	}

	std::vector<const sim_mob::RoadSegment*>::iterator currentSegIt = parentDriver->vehicle->getPathIterator();
	std::vector<const sim_mob::RoadSegment*>::iterator currentSegItEnd = parentDriver->vehicle->getPathIteratorEnd();
//	std::vector<const sim_mob::RoadSegment*> path = parentDriver->vehicle->getPath();

	for(;currentSegIt != currentSegItEnd;++currentSegIt)
	{
		if(currentSegIt == currentSegItEnd){
			break;
		}

		const RoadSegment* rs = *currentSegIt;
		if (!rs) {
			break;
		}

		const std::map<centimeter_t, const RoadItem*> obstacles = rs->getObstacles();
		std::map<centimeter_t, const RoadItem*>::const_iterator obsIt;

		if(obstacles.size()==0){
			if(rs == parentDriver->vehicle->getCurrSegment()){
				itemDis = parentDriver->vehicle->getCurrentSegmentLength() - parentDriver->vehicle->getDistanceMovedInSegment();
			}
			else{
				itemDis += rs->getLengthOfSegment();
			}
		}

		for(obsIt=obstacles.begin(); obsIt!=obstacles.end(); obsIt++){
			const Incident* inc = dynamic_cast<const Incident*>( (*obsIt).second );

				if(rs == parentDriver->vehicle->getCurrSegment())
				{
					//1. in current seg
					if(inc){
						//1.1 find incident
						double incidentDis = (*obsIt).first;
						double moveDis  = parentDriver->vehicle->getDistanceMovedInSegment();
						//1.2 incident in forward
						if(moveDis <= incidentDis)
						{
							itemDis = incidentDis - moveDis;
							if(itemDis < 0)
							{
								std::cout<<"getRoadItemByDistance: getDistanceMovedInSegment not right"<<std::endl;
							}
							if(itemDis <= perceptionDis)
							{
								res = inc;
								return res;
							}
							else
							{
								// the incident already out of perception, no need check far more
								return nullptr;
							}
						}// end if moveDis
					} //end if inc
					itemDis = parentDriver->vehicle->getCurrentSegmentLength() - parentDriver->vehicle->getDistanceMovedInSegment();
				}//end rs==
				else
				{
					//2.0 in forword seg
					if(isInSameLink == true)
					{
						// seg not in current link
						if(parentDriver->vehicle->getCurrSegment()->getLink() != rs->getLink())
						{
							return res;
						}
					}
					if(inc){
						//2.1 find incident
						double incidentDis = (*obsIt).first;
						itemDis += incidentDis;
						if(itemDis <= perceptionDis)
						{
							res = inc;
							return res;
						}
						else
						{
							// the incident already out of perception, no need check far more
							return nullptr;
						}
					}//end inc
					itemDis += rs->getLengthOfSegment();
				}


		}//end for obstacles

	}//end for segs

	return res;
}


bool sim_mob::DriverMovement::isPedestrianOnTargetCrossing() const {
	if ((!trafficSignal)||(!(parentDriver->vehicle->getNextSegment()))) {
		return false;
	}



	//oh! we really dont neeeeeeeeed all this! r u going to repeat these two iterations for all the corresponding drivers?
//	map<Link const*, size_t> const linkMap = trafficSignal->links_map();
//	int index = -1;
//	for (map<Link const*, size_t>::const_iterator link_i = linkMap.begin(); link_i != linkMap.end(); link_i++) {
//		if (vehicle->getNextSegment() && link_i->first == vehicle->getNextSegment()->getLink()) {
//			index = (*link_i).second;
//			break;
//		}
//	}
//
//	map<Crossing const *, size_t> const crossingMap = trafficSignal->crossings_map();
//	const Crossing* crossing = nullptr;
//	for (map<Crossing const *, size_t>::const_iterator crossing_i = crossingMap.begin(); crossing_i
//			!= crossingMap.end(); crossing_i++) {
//		if (static_cast<int> (crossing_i->second) == index) {
//			crossing = crossing_i->first;
//			break;
//		}
//	}
	sim_mob::Link * targetLink = parentDriver->vehicle->getNextSegment()->getLink();
	const Crossing* crossing = nullptr;
	const LinkAndCrossingC& LAC = trafficSignal->getLinkAndCrossing();
	LinkAndCrossingC::iterator it = LAC.begin();
	for(; it != LAC.end(); it++){
		if(it->link == targetLink)
		{
			break;
		}
	}

	if(it != LAC.end()) {
		crossing = (*it).crossing;
	}
	else{

		return false;
	}
	//Have we found a relevant crossing?
	if (!crossing) {
		return false;
	}

	//Search through the list of agents in that crossing.
	//------------this cause error
//	vector<const Agent*> agentsInRect = GetAgentsInCrossing(crossing);
//	for (vector<const Agent*>::iterator it = agentsInRect.begin(); it != agentsInRect.end(); it++) {
//		const Person* other = dynamic_cast<const Person *> (*it);
//		if (other) {
//			const Pedestrian* pedestrian = dynamic_cast<const Pedestrian*> (other->getRole());
//			if (pedestrian && pedestrian->isOnCrossing()) {
//				return true;
//			}
//		}
//	}
	return false;
}

double sim_mob::DriverMovement::dwellTimeCalculation(int A, int B, int delta_bay, int delta_full,int Pfront, int no_of_passengers)
{
	//assume single channel passenger movement
	 double alpha1 = 2.1;//alighting passenger service time,assuming payment by smart card
	 double alpha2 = 3.5;//boarding passenger service time,assuming alighting through rear door
	 double alpha3 = 3.5;//door opening and closing times
	 double alpha4 = 1.0;
	 double beta1 = 0.7;//fixed parameters
	 double beta2 = 0.7;
	 double beta3 = 5;
	 double DTijk = 0.0;
	 bool bus_crowdness_factor;
	int no_of_seats = 40;
	if (no_of_passengers > no_of_seats) //standees are present
		alpha1 += 0.5; //boarding time increase if standees are present
	if (no_of_passengers > no_of_seats)
		bus_crowdness_factor = 1;
	else
		bus_crowdness_factor = 0;
	double PTijk_front = alpha1 * Pfront * A + alpha2 * B+ alpha3 * bus_crowdness_factor * B;
	double PTijk_rear = alpha4 * (1 - Pfront) * A;
	double PT;
	PT = std::max(PTijk_front, PTijk_rear);
	DTijk = beta1 + PT + beta2 * delta_bay + beta3 * delta_full;
	std::cout<<"Dwell__time "<<DTijk<<std::endl;
	return DTijk;
}

//update left and right lanes of the current lane
//if there is no left or right lane, it will be null
void sim_mob::DriverMovement::updateAdjacentLanes(DriverUpdateParams& p) {
	//Need to reset, we can call this after DriverUpdateParams is initialized.
	p.leftLane = nullptr;
	p.rightLane = nullptr;
	p.leftLane2 = nullptr;
	p.rightLane2 = nullptr;

	if (!p.currLane) {
		return; //Can't do anything without a lane to reference.
	}

	if (p.currLaneIndex > 0) {
		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex - 1);
		if(!temp->is_pedestrian_lane())
			p.rightLane = temp;
	}
	if (p.currLaneIndex > 1) {
		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex - 2);
		if(!temp->is_pedestrian_lane())
			p.rightLane2 = temp;
	}

	if (p.currLaneIndex < p.currLane->getRoadSegment()->getLanes().size() - 1) {
		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex + 1);
		if(!temp->is_pedestrian_lane())
			p.leftLane = temp;
	}

	if (p.currLaneIndex < p.currLane->getRoadSegment()->getLanes().size() - 2) {
		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex + 2);
		if(!temp->is_pedestrian_lane())
			p.leftLane2 = temp;
	}
}

//General update information for whenever a Segment may have changed.
void sim_mob::DriverMovement::syncCurrLaneCachedInfo(DriverUpdateParams& p) {
	//The lane may have changed; reset the current lane index.
	p.currLaneIndex = getLaneIndex(p.currLane);

	//Update which lanes are adjacent.
	updateAdjacentLanes(p);

	//Update the length of the current road segment.
	p.currLaneLength = parentDriver->vehicle->getCurrLinkLaneZeroLength();

	//Finally, update target/max speed to match the new Lane's rules.
	maxLaneSpeed = parentDriver->vehicle->getCurrSegment()->maxSpeed / 3.6; //slow down
	targetSpeed = maxLaneSpeed;
}

//currently it just chooses the first lane from the targetLane
//Note that this also sets the target lane so that we (hopefully) merge before the intersection.
void sim_mob::DriverMovement::chooseNextLaneForNextLink(DriverUpdateParams& p) {
	p.nextLaneIndex = p.currLaneIndex;
	//Retrieve the node we're on, and determine if this is in the forward direction.
	const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (parentDriver->vehicle->getNodeMovingTowards());
	const RoadSegment* nextSegment = parentDriver->vehicle->getNextSegment(false);
	const RoadSegment* currentLinkNextSegment = parentDriver->vehicle->getNextSegment(true);

	//Build up a list of target lanes.
	nextLaneInNextLink = nullptr;
	vector<const Lane*> targetLanes;
	if (currEndNode && nextSegment) {
		const set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(parentDriver->vehicle->getCurrSegment());
		for (set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++) {
			if ((*it)->getLaneTo()->getRoadSegment() == nextSegment && (*it)->getLaneFrom() == p.currLane) {
				//It's a valid lane.
				targetLanes.push_back((*it)->getLaneTo());

				//find target lane with same index, use this lane
				size_t laneIndex = getLaneIndex((*it)->getLaneTo());
				if (laneIndex == p.currLaneIndex&&!((*it)->getLaneTo()->is_pedestrian_lane())){
					nextLaneInNextLink = (*it)->getLaneTo();
					targetLaneIndex = laneIndex;
					break;
				}
			}
		}

		//Still haven't found a lane?
		if (!nextLaneInNextLink) {
			//no lane with same index, use the first lane in the vector if possible.
			if (targetLanes.size() > 0) {
				nextLaneInNextLink = targetLanes.at(0);
				if(nextLaneInNextLink->is_pedestrian_lane()&&targetLanes.size()>1)
					nextLaneInNextLink = targetLanes.at(1);
				targetLaneIndex = getLaneIndex(nextLaneInNextLink);
			} else if (nextSegment) { //Fallback
				size_t fallbackIndex = std::min(p.currLaneIndex, nextSegment->getLanes().size() - 1);
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
void sim_mob::DriverMovement::calculateIntersectionTrajectory(DPoint movingFrom, double overflow) {
	//If we have no target link, we have no target trajectory.
	if (!nextLaneInNextLink) {
		Warn() <<"WARNING: nextLaneInNextLink has not been set; can't calculate intersection trajectory." << std::endl;
		return;
	}

	//Get the entry point.
	int id = getLaneIndex(parentDriver->vehicle->getCurrLane());
	int startOldLane = -1;

	for(vector<Lane*>::const_iterator it = parentDriver->vehicle->getCurrSegment()->getLanes().begin();it!=parentDriver->vehicle->getCurrSegment()->getLanes().end();it++)
	{
		if((*it)->is_pedestrian_lane() || (*it)->is_bicycle_lane())
		{

		}
		else
		{
			startOldLane = getLaneIndex((*it));
			break;
		}
	}


	int total = nextLaneInNextLink->getRoadSegment()->getLanes().size()-1;
	int offset = parentDriver->vehicle->getCurrSegment()->getLanes().size() -1 -id;
	set<int> laneIDS;
	bool first = true;
	int StartnewLane = -1;
	int last = 1;
	Point2D entry = nextLaneInNextLink->getPolyline().at(0);

	for(vector<Lane*>::const_iterator it = nextLaneInNextLink->getRoadSegment()->getLanes().begin();it!=nextLaneInNextLink->getRoadSegment()->getLanes().end();it++)
	{
		if((*it)->is_pedestrian_lane() || (*it)->is_bicycle_lane())
		{

		}
		else
		{
			if(first)
			{
				first = false;
				StartnewLane = getLaneIndex((*it));
			}
			//std::cout<<getLaneIndex((*it))<<std::endl;
			laneIDS.insert(getLaneIndex((*it)));
			last = getLaneIndex((*it));
		}
	}

	if((startOldLane !=-1) && StartnewLane != -1) id = id + (StartnewLane - startOldLane);


	if(laneIDS.find(id) != laneIDS.end())
	{
		entry = nextLaneInNextLink->getRoadSegment()->getLanes().at(id)->getPolyline().at(0);//  getLaneEdgePolyline(findID).at(0);
		lastIndex = id;
	}
	else
	{
		int findID = total - offset;
		if(findID > 0)
		{
			if(laneIDS.find(findID) != laneIDS.end())
			{
				entry = nextLaneInNextLink->getRoadSegment()->getLanes().at(findID)->getPolyline().at(0);//  getLaneEdgePolyline(findID).at(0);
				lastIndex = findID;
			}
			else
			{
				entry = nextLaneInNextLink->getRoadSegment()->getLanes().at(last)->getPolyline().at(0);//->getLaneEdgePolyline(last).at(0);
				lastIndex = last;
			}
		}
		else
		{
			lastIndex = *(laneIDS.begin());
			entry = nextLaneInNextLink->getRoadSegment()->getLanes().at(*(laneIDS.begin()))->getPolyline().at(0);//>getLaneEdgePolyline(*(laneIDS.begin())).at(0);
		}
	}
	//Compute a movement trajectory.
	intModel->startDriving(movingFrom, DPoint(entry.getX(), entry.getY()), overflow);
}


//link path should be retrieved from other class
//for now, it serves as this purpose
//Edited by Jenny (11th June)
//Try to initialize only the path from the current location to the next activity location
//Added in a parameter for the function call: next
///Returns the new vehicle, if requested to build one.
Vehicle* sim_mob::DriverMovement::initializePath(bool allocateVehicle) {
	Vehicle* res = nullptr;

	//Only initialize if the next path has not been planned for yet.
	if(!getParent()->getNextPathPlanned()) {
		//Save local copies of the parent's origin/destination nodes.
		parentDriver->origin.node = getParent()->originNode.node_;
		parentDriver->origin.point = parentDriver->origin.node->location;
		parentDriver->goal.node = getParent()->destNode.node_;
		parentDriver->goal.point = parentDriver->goal.node->location;

		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
		vector<WayPoint> path;

		Person* parentP = dynamic_cast<Person*> (parent);

		if(!parentP->amodPath.empty()){
			path = parent->amodPath;
		}
		else
		{
			sim_mob::SubTrip* subTrip = (&(*(parentP->currSubTrip)));
			const StreetDirectory& stdir = StreetDirectory::instance();

			if(parentP && parentP->schedules.size()==0){
				// if use path set
				if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
					path = PathSetManager::getInstance()->getPathByPerson(getParent());
				}
				else
				{
					const StreetDirectory& stdir = StreetDirectory::instance();
					path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*(parentDriver->origin).node), stdir.DrivingVertex(*(parentDriver->goal).node));
				}

			}
			else {
				std::vector<Node*>& routes = parentP->schedules.front().routes;
				std::vector<Node*>::iterator first = routes.begin();
				std::vector<Node*>::iterator second = first;

				path.clear();
				for(second++; first!=routes.end() && second!=routes.end(); first++, second++){
					vector<WayPoint> subPath = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(**first), stdir.DrivingVertex(**second));
					path.insert( path.end(), subPath.begin(), subPath.end());
				}
			}
		}//if amod path

		//For now, empty paths aren't supported.
		if (path.empty()) {
			throw std::runtime_error("Can't initializePath(); path is empty.");
		}

		//RoadRunner may need to know of our path, but it can't be send inevitably.
		if (getParent()->getRegionSupportStruct().isEnabled()) {
			rrPathToSend.clear();
			for (std::vector<WayPoint>::const_iterator it=path.begin(); it!=path.end(); it++) {
				if (it->type_ == WayPoint::ROAD_SEGMENT) {
					rrPathToSend.push_back(it->roadSegment_);
				}
			}
		}

		//TODO: Start in lane 0?
		int startLaneId = 0;

		if(parentP->laneID != -1)
		{
			// path[1] is currently the starting segment from the shortest driving path algorithm
			if(path[1].type_ == WayPoint::ROAD_SEGMENT) {
				if(parent->laneID >= 0 && parent->laneID < path[1].roadSegment_->getLanes().size()) {
					startLaneId = parentP->laneID;//need to check if lane valid
				}
			}
			parentP->laneID = -1;
		}

		// Bus should be at least 1200 to be displayed on Visualizer
		const double length = dynamic_cast<BusDriver*>(this->getParentDriver()) ? 1200 : 400;
		const double width = 200;

		//A non-null vehicle means we are moving.
		if (allocateVehicle) {
			res = new Vehicle(path, startLaneId, length, width);
		}
	}

	//to indicate that the path to next activity is already planned
	getParent()->setNextPathPlanned(true);
	return res;
}
void sim_mob::DriverMovement::rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment*>& blacklisted)
{
	//Skip if we're somehow not driving on a road.
	if (!(parentDriver && parentDriver->vehicle && parentDriver->vehicle->getCurrSegment())) {
		return;
	}

	//Retrieve the shortest path from the current intersection node to destination and save all RoadSegments in this path.
	//NOTE: This path may be invalid, is there is no LaneConnector from the current Segment to the first segment of the result path.
	const RoadSegment* currSeg = parentDriver->vehicle->getCurrSegment();
	const Node* node = currSeg->getEnd();
	const StreetDirectory& stdir = StreetDirectory::instance();
	vector<WayPoint> path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*node), stdir.DrivingVertex(*(parentDriver->goal.node)), blacklisted);

	//Given this (tentative) path, we still have to transition from the current Segment.
	//At the moment this is a bit tedious (since we can't search mid-segment in the StreetDir), but
	//  the following heuristic should work well enough.
	const RoadSegment* nextSeg = nullptr;
	if (path.size() > 1) { //Node, Segment.
		vector<WayPoint>::iterator it = path.begin();
		if ((it->type_==WayPoint::NODE) && (it->node_ == node)) {
			it++;
			if (it->type_==WayPoint::ROAD_SEGMENT) {
				nextSeg = it->roadSegment_;

			}
		}
	}

	//Now find the LaneConnectors. For a UniNode, this is trivial. For a MultiNode, we have to check.
	//NOTE: If a lane connector is NOT found, there may still be an alternate route... but we can't check this without
	//      blacklisting the "found" segment and repeating. I think a far better solution would be to modify the
	//      shortest-path algorithm to allow searching from Segments (the data structure already can handle it),
	//      but for now we will have to deal with the rare true negative in cities with lots of one-way streets.
	if (nextSeg) {
		bool found = false;
		const MultiNode* mn = dynamic_cast<const MultiNode*>(node);
		if (mn) {
			const set<LaneConnector*> lcs = mn->getOutgoingLanes(currSeg);
			for (set<LaneConnector*>::const_iterator it=lcs.begin(); it!=lcs.end(); it++) {
				if (((*it)->getLaneFrom()->getRoadSegment()==currSeg) && ((*it)->getLaneTo()->getRoadSegment()==nextSeg)) {
					found = true;
					break;
				}
			}
		}
		if (!found) {
			path.clear();
		}
	}

	//If there's no path, keep the current one.
	if (path.empty()) {
		return;
	}

	//Else, pre-pend the current segment, and reset the current driver.
	//NOTE: This will put the current driver back onto the start of the current Segment, but since this is only
	//      used in Road Runner, it doesn't matter right now.
	//TODO: This *might* work if we save the current advance on the current segment and reset it.
	vector<WayPoint>::iterator it = path.begin();
	path.insert(it, WayPoint(parentDriver->vehicle->getCurrSegment()));
	parentDriver->vehicle->resetPath(path);

	//Finally, update the client with the new list of Region tokens it must acquire.
	setRR_RegionsFromCurrentPath();
}


void sim_mob::DriverMovement::setOrigin(DriverUpdateParams& p) {
	//Set the max speed and target speed.
	maxLaneSpeed = parentDriver->vehicle->getCurrSegment()->maxSpeed / 3.6;
	targetSpeed = maxLaneSpeed;

	//Set our current and target lanes.
	p.currLane = parentDriver->vehicle->getCurrLane();
	p.currLaneIndex = getLaneIndex(p.currLane);
	targetLaneIndex = p.currLaneIndex;

	//Vehicles start at rest
	parentDriver->vehicle->setVelocity(0);
	parentDriver->vehicle->setLatVelocity(0);
	parentDriver->vehicle->setAcceleration(0);

	//Calculate and save the total length of the current polyline.
	p.currLaneLength = parentDriver->vehicle->getCurrLinkLaneZeroLength();

	//if the first road segment is the last one in this link
	if (!(parentDriver->vehicle->hasNextSegment(true))) {
		saveCurrTrafficSignal();
	}
	if (!(parentDriver->vehicle->hasNextSegment(true)) && parentDriver->vehicle->hasNextSegment(false)) {
		//Don't do this if there is no next link.
		chooseNextLaneForNextLink(p);
	}
}

//TODO
void sim_mob::DriverMovement::findCrossing(DriverUpdateParams& p) {
	const Crossing* crossing = dynamic_cast<const Crossing*> (parentDriver->vehicle->getCurrSegment()->nextObstacle(
			parentDriver->vehicle->getDistanceMovedInSegment(), true).item);

	if (crossing) {
		//TODO: Please double-check that this does what's intended.
		Point2D interSect = LineLineIntersect(parentDriver->vehicle->getCurrPolylineVector(), crossing->farLine.first,
				crossing->farLine.second);
		DynamicVector toCrossing(parentDriver->vehicle->getX(), parentDriver->vehicle->getY(), interSect.getX(), interSect.getY());

		p.crossingFwdDistance = toCrossing.getMagnitude();
		p.isCrossingAhead = true;
	}
}

double sim_mob::DriverMovement::updatePositionOnLink(DriverUpdateParams& p) {
	//Determine how far forward we've moved.
	//TODO: I've disabled the acceleration component because it doesn't really make sense.
	//      Please re-enable if you think this is expected behavior. ~Seth
	double fwdDistance = parentDriver->vehicle->getVelocity() * p.elapsedSeconds + 0.5 * parentDriver->vehicle->getAcceleration()
			* p.elapsedSeconds * p.elapsedSeconds;
	if (fwdDistance < 0) {
		fwdDistance = 0;
	}

	if(incidentStatus.getCurrentStatus()==IncidentStatus::INCIDENT_CLEARANCE && incidentStatus.getCurrentIncidentLength()>0){
		incidentStatus.reduceIncidentLength(fwdDistance);
	}


	//double fwdDistance = vehicle->getVelocity()*p.elapsedSeconds;
	double latDistance = parentDriver->vehicle->getLatVelocity() * p.elapsedSeconds;

	//Increase the vehicle's velocity based on its acceleration.
	parentDriver->vehicle->setVelocity(parentDriver->vehicle->getVelocity() + parentDriver->vehicle->getAcceleration() * p.elapsedSeconds);

	//when v_lead and a_lead is 0, space is not negative, the Car Following will generate an acceleration based on free flowing model
	//this causes problem, so i manually set acceleration and velocity to 0
	if (parentDriver->vehicle->getVelocity() < 0 ||(p.space<1&&p.v_lead==0&&p.a_lead==0)) {
		//Set to 0 forward velocity, no acceleration.
		parentDriver->vehicle->setVelocity(0.0);
		parentDriver->vehicle->setAcceleration(0);
	}

	//Move the vehicle forward.
	double res = 0.0;
	try {
		res = parentDriver->vehicle->moveFwd(fwdDistance);
		if(!(parentDriver->vehicle->isInIntersection()))
		{
			double d = parentDriver->vehicle->getDistanceMovedInSegment();
			double c=0;
		}
	} catch (std::exception& ex) {
		if (Debug::Drivers) {
			if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
				DebugStream << ">>>Exception: " << ex.what() << endl;
				PrintOut(DebugStream.str());
			}
		}

		std::stringstream msg;
		msg << "Error moving vehicle forward for Agent ID: " << getParent()->getId() << "," << this->parentDriver->vehicle->getX() << "," << this->parentDriver->vehicle->getY() << "\n" << ex.what();
		throw std::runtime_error(msg.str().c_str());
	}

	//Retrieve what direction we're moving in, since it will "flip" if we cross the relative X axis.
	LANE_CHANGE_SIDE relative = getCurrLaneSideRelativeToCenter();
	//after forwarding, adjacent lanes might be changed
	updateAdjacentLanes(p);
	//there is no left lane when turning left
	//or there is no right lane when turning right
	if((parentDriver->vehicle->getTurningDirection()==LCS_LEFT && !p.leftLane)||
			(parentDriver->vehicle->getTurningDirection()==LCS_RIGHT && !p.rightLane))
	{
		latDistance = 0;
		parentDriver->vehicle->setLatVelocity(0);
	}


	//Lateral movement
	if (!(parentDriver->vehicle->isInIntersection()) ) {
		parentDriver->vehicle->moveLat(latDistance);
		updatePositionDuringLaneChange(p, relative);
	}

	//Update our offset in the current lane.
	if (!(parentDriver->vehicle->isInIntersection())) {
		p.currLaneOffset = parentDriver->vehicle->getDistanceMovedInSegment();
	}
	return res;
}

void sim_mob::DriverMovement::check_and_set_min_car_dist(NearestVehicle& res, double distance, const Vehicle* veh,
		const Driver* other) {
	//Subtract the size of the car from the distance between them
	bool fwd=false;
	if (distance>=0)
		fwd = true;
	distance = fabs(distance) - veh->length / 2 - other->getVehicleLength() / 2;
	if ( parentDriver->isAleadyStarted )
	{
		if(fwd && distance <0)
			distance = 0.1;
	}
	if (distance <= res.distance) {
		res.driver = other;
		res.distance = distance;
	}
}


void sim_mob::DriverMovement::check_and_set_min_car_dist2(NearestVehicle& res, double distance, const Vehicle* other_veh,
		const Driver* me) {
	//Subtract the size of the car from the distance between them
	bool fwd=false;
	if (distance>=0)
		fwd = true;
	distance = fabs(distance) - other_veh->length / 2 - me->getVehicleLength() / 2;
	if ( me->isAleadyStarted )
	{
		if(fwd && distance <0)
			distance = 0.1;
	}
	if (distance <= res.distance) {
		res.driver = me;
		res.distance = distance;
	}
}

void sim_mob::DriverMovement::check_and_set_min_nextlink_car_dist(NearestVehicle& res, double distance, const Vehicle* veh,
		const Driver* other) {
	//Subtract the size of the car from the distance between them
	distance = fabs(distance) - other->getVehicleLength() / 2;
	if (distance <= res.distance) {
		res.driver = other;
		res.distance = distance;
	}
}

//incompleteattempt to rewrite handleUpdateRequestDriver in order to switch analyzer and analyzed objects
//void sim_mob::DriverMovement::handleUpdateRequestDriverTo(const Driver* target, DriverUpdateParams& targetParams) {
//	DriverUpdateParams& myparams = parentDriver->getParams();
//	if (!(this->parentDriver->isInIntersection.get()&& this->parentDriver != target)) {
//		return;
//	}
//
//	//Retrieve the my lane, road segment, and lane offset.
//	const Lane* my_lane = parentDriver->currLane_.get();
//	if (!my_lane) {
//			return;
//		}
//	const RoadSegment* myRoadSegment = my_lane->getRoadSegment();
//
//	if(target->vehicle->isInIntersection() || parentDriver->vehicle->isInIntersection())
//	{
//		return;
//	}
//	int my_offset = parentDriver->vehicle->getDistanceMovedInSegment();
//	//If the vehicle is in the same Road segment
//	if (target->vehicle->getCurrSegment() == myRoadSegment) {
//		int distance = target->vehicle->getDistanceMovedInSegment() - my_offset;
//		if (distance == 0)
//		{
//			return;
//		}
//		bool fwd = distance >= 0;
//		//Set different variables depending on where the car is.
//		if (my_lane == targetParams.currLane) {//the vehicle is on the current lane
//			check_and_set_min_car_dist2((fwd ? targetParams.nvFwd : targetParams.nvBack), distance, target->vehicle, parentDriver);
//		} else if (my_lane == targetParams.leftLane) { //the vehicle is on the left lane
//			check_and_set_min_car_dist2((fwd ? targetParams.nvLeftBack : targetParams.nvLeftFwd), distance, target->vehicle, parentDriver);
//		} else if (my_lane == targetParams.rightLane) { //The vehicle is on the right lane
//			check_and_set_min_car_dist2((fwd ? targetParams.nvRightBack : targetParams.nvRightFwd), distance, target->vehicle, parentDriver);
//		} else if (my_lane == targetParams.leftLane2) { //The vehicle is on the second Left lane
//			check_and_set_min_car_dist2((fwd ? targetParams.nvLeftBack2 : targetParams.nvLeftFwd2), distance, target->vehicle, parentDriver);
//		} else if (my_lane == targetParams.rightLane2) { //The vehicle is on the second right lane
//			check_and_set_min_car_dist2((fwd ? targetParams.nvRightBack2 : targetParams.nvRightFwd2), distance, target->vehicle, parentDriver);
//		}
//
//	}
//
//
//}

//TODO: I have the feeling that this process of detecting nearby drivers in front of/behind you and saving them to
//      the various CFD/CBD/LFD/LBD variables can be generalized somewhat. I shortened it a little and added a
//      helper function; perhaps more cleanup can be done later? ~Seth
bool sim_mob::DriverMovement::updateNearbyAgent(const Agent* other, const Driver* other_driver) {
	DriverUpdateParams& params = parentDriver->getParams();
	//Only update if passed a valid pointer which is not a pointer back to you, and
	//the driver is not actually in an intersection at the moment.

	/*if (getParams().now.ms()/1000.0 > 41.8 && parent->getId() == 25)
			std::cout<<"find vh"<<std::endl;*/
	if (!(other_driver && this->parentDriver != other_driver && !other_driver->isInIntersection.get())) {
		return false;
	}

	//Retrieve the other driver's lane, road segment, and lane offset.
	const Lane* other_lane = other_driver->currLane_.get();
	if (!other_lane) {
			return false;
		}
	const RoadSegment* otherRoadSegment = other_lane->getRoadSegment();

//	if (getParams().now.ms()/1000.0 >  93.7
//	 && parent->getId() == 402)
//	{
//			std::cout<<"find 332288222 " <<other_driver->parent->getId()<<std::endl;
//			if (otherRoadSegment->getLink() != vehicle->getCurrLink()) { //We are in the different link.
//					if (!vehicle->isInIntersection() && vehicle->getNextSegment(false) == otherRoadSegment) { //Vehicle is on the next segment,which is in next link after intersection.
//						std::cout<<"find 3322882asdfa22 " <<other_driver->parent->getId()<<std::endl;
//					}
//			}
//	}


	if(parentDriver->vehicle->isInIntersection() || other_driver->vehicle->isInIntersection())
		return false;

//	int other_offset = other_driver->currLaneOffset_.get();
	int other_offset = other_driver->vehicle->getDistanceMovedInSegment();

	//If the vehicle is in the same Road segment
	if (parentDriver->vehicle->getCurrSegment() == otherRoadSegment) {
		//Set distance equal to the _forward_ distance between these two vehicles.
//		int distance = other_offset - params.currLaneOffset;
		int distance = other_offset - parentDriver->vehicle->getDistanceMovedInSegment();
		if (distance == 0)
			return false;
		bool fwd = distance >= 0;

		//Set different variables depending on where the car is.
		if (other_lane == params.currLane) {//the vehicle is on the current lane
//			if (getParams().now.ms()/1000.0 >  123.9
//					 && parent->getId() == 404)
//			{
//					std::cout<<"find  " <<other_driver->parent->getId()<<std::endl;
//			}
			check_and_set_min_car_dist((fwd ? params.nvFwd : params.nvBack), distance, parentDriver->vehicle, other_driver);
		} else if (other_lane == params.leftLane) { //the vehicle is on the left lane
			check_and_set_min_car_dist((fwd ? params.nvLeftFwd : params.nvLeftBack), distance, parentDriver->vehicle, other_driver);
		} else if (other_lane == params.rightLane) { //The vehicle is on the right lane
			check_and_set_min_car_dist((fwd ? params.nvRightFwd : params.nvRightBack), distance, parentDriver->vehicle,other_driver);
		} else if (other_lane == params.leftLane2) { //The vehicle is on the second Left lane
			check_and_set_min_car_dist((fwd ? params.nvLeftFwd2 : params.nvLeftBack2), distance, parentDriver->vehicle,other_driver);
		} else if (other_lane == params.rightLane2) { //The vehicle is on the second right lane
			check_and_set_min_car_dist((fwd ? params.nvRightFwd2 : params.nvRightBack2), distance, parentDriver->vehicle,other_driver);
		}

	} else if (otherRoadSegment->getLink() == parentDriver->vehicle->getCurrLink()) { //We are in the same link.
		if (parentDriver->vehicle->getNextSegment() == otherRoadSegment) { //Vehicle is on the next segment.
			//Retrieve the next node we are moving to, cast it to a UniNode.
			const Node* nextNode = parentDriver->vehicle->getCurrSegment()->getEnd();
			const UniNode* uNode = dynamic_cast<const UniNode*> (nextNode);
			//seems the following dynamic_cast is not needed, thereby commenting
//			UniNode* myuode = const_cast<sim_mob::UniNode*> (uNode);
//			if (getParams().now.ms()/1000.0 >  123.9
//					 && parent->getId() == 404)
//			{
//					std::cout<<"find 65298 " <<parent->getId()<<std::endl;
//			}
			//Initialize some lane pointers
			const Lane* nextLane = nullptr;
			const Lane* nextLeftLane = nullptr;
			const Lane* nextRightLane = nullptr;
			const Lane* nextLeftLane2 = nullptr;
			const Lane* nextRightLane2 = nullptr;
			//if (uNode) {
			//	nextLane = uNode->getOutgoingLane(*params.currLane);
			//}

			if (uNode ) {
				nextLane = uNode->getForwardDrivingLane(*params.currLane);
			}

			if(nextLane==nullptr){
				std::cout<<"error getForwardDrivingLane no out lane"<<std::endl;
			}

//			//
//			const sim_mob::Lane * currlan = params.currLane;
//			std::vector<Point2D> currployline = currlan->polyline_;
//			int s = currployline.size() - 1;
//			Point2D currlastp = currployline.back();
//			//
//			std::vector<sim_mob::Lane*> outlanes;
//			if (uNode) {
//				sim_mob::Lane *l = const_cast<sim_mob::Lane*> (params.currLane);
//				outlanes = myuode->getOutgoingLanes(*l);
//			}
//			if (outlanes.size() == 0)
//				std::cout<<"error no out lane"<<std::endl;
//			double dis=100000000;
//			for (std::vector< sim_mob::Lane*>::iterator it=outlanes.begin();it!=outlanes.end();++it)
//			{
//				Lane * l = *it;
//				std::vector<Point2D> pl = l->polyline_;
//				Point2D plzero = currployline[0];
//				DynamicVector zeroPoly(currlastp.getX(), currlastp.getY(), plzero.getX(), plzero.getY());
//				double d = zeroPoly.getMagnitude();
//				if(d<dis)
//				{
//					nextLane = l;
//				}
//			}

			//Make sure next lane exists and is in the next road segment, although it should be true
			if (nextLane && nextLane->getRoadSegment() == otherRoadSegment) {
				//Assign next left/right lane based on lane ID.
				size_t nextLaneIndex = getLaneIndex(nextLane);
				if (nextLaneIndex > 0) {
					nextLeftLane = otherRoadSegment->getLanes().at(nextLaneIndex - 1);
				}
				if (nextLaneIndex < otherRoadSegment->getLanes().size() - 1) {
					nextRightLane = otherRoadSegment->getLanes().at(nextLaneIndex + 1);
				}
				if (nextLaneIndex > 1)
				{
					nextLeftLane2 = otherRoadSegment->getLanes().at(nextLaneIndex - 2);
				}
				if (nextLaneIndex < otherRoadSegment->getLanes().size() - 2) {
				    nextRightLane2 = otherRoadSegment->getLanes().at(nextLaneIndex + 2);
				}
			}

			//Modified distance.
			int distance = other_offset + params.currLaneLength - params.currLaneOffset;

			//Set different variables depending on where the car is.
			if (other_lane == nextLane) { //The vehicle is on the current lane
//				if (getParams().now.ms()/1000.0 >  123.9
//						 && parent->getId() == 404)
//				{
//						std::cout<<"find 65298 " <<other_driver->parent->getId()<<std::endl;
//				}
				check_and_set_min_car_dist(params.nvFwd, distance, parentDriver->vehicle, other_driver);
			} else if (other_lane == nextLeftLane) { //the vehicle is on the left lane
				check_and_set_min_car_dist(params.nvLeftFwd, distance, parentDriver->vehicle, other_driver);
			} else if (other_lane == nextRightLane) { //the vehicle is in front
				check_and_set_min_car_dist(params.nvRightFwd, distance, parentDriver->vehicle, other_driver);
			} else if (other_lane == nextLeftLane2) { //The vehicle is on the second Left lane
				check_and_set_min_car_dist(params.nvLeftFwd2, distance, parentDriver->vehicle,other_driver);
			} else if (other_lane == nextRightLane2) { //The vehicle is on the second right lane
				check_and_set_min_car_dist(params.nvRightFwd2, distance, parentDriver->vehicle,other_driver);
			}
		} else if (parentDriver->vehicle->getPrevSegment() == otherRoadSegment) { //Vehicle is on the previous segment.
			//Retrieve the previous node as a UniNode.
			const Node* prevNode = parentDriver->vehicle->getNodeMovingFrom();
			const UniNode* uNode = dynamic_cast<const UniNode*> (prevNode);

			//Set some lane pointers.
			const Lane* preLane = nullptr;
			const Lane* preLeftLane = nullptr;
			const Lane* preRightLane = nullptr;
			const Lane* preLeftLane2 = nullptr;
			const Lane* preRightLane2 = nullptr;

			//Find the node which leads to this one from the UniNode. (Requires some searching; should probably
			//   migrate this to the UniNode class later).
			const vector<Lane*>& lanes = otherRoadSegment->getLanes();
			if (uNode) {
				for (vector<Lane*>::const_iterator it = lanes.begin(); it != lanes.end() && !preLane; it++) {
					if (uNode->getOutgoingLane(**it) == params.currLane) {
						preLane = *it;
					}
				}
			}

			//Make sure next lane is in the next road segment, although it should be true
			if (preLane) {
				//Save the new left/right lanes
				size_t preLaneIndex = getLaneIndex(preLane);
				if (preLaneIndex > 0) {
					preLeftLane = otherRoadSegment->getLanes().at(preLaneIndex - 1);
				}
				if (preLaneIndex < otherRoadSegment->getLanes().size() - 1) {
					preRightLane = otherRoadSegment->getLanes().at(preLaneIndex + 1);
				}
			}

			//Modified distance.
			int distance = other_driver->currLaneLength_.get() - other_offset + params.currLaneOffset;

			//Set different variables depending on where the car is.
			if (other_lane == preLane) { //The vehicle is on the current lane
				check_and_set_min_car_dist(params.nvBack, distance, parentDriver->vehicle, other_driver);
			} else if (other_lane == preLeftLane) { //the vehicle is on the left lane
				check_and_set_min_car_dist(params.nvLeftBack, distance, parentDriver->vehicle, other_driver);
			} else if (other_lane == preRightLane) { //the vehicle is on the right lane
				check_and_set_min_car_dist(params.nvRightBack, distance, parentDriver->vehicle, other_driver);
			} else if (other_lane == preLeftLane2) { //The vehicle is on the second Left lane
				check_and_set_min_car_dist(params.nvLeftBack2, distance, parentDriver->vehicle,other_driver);
			} else if (other_lane == preRightLane2) { //The vehicle is on the second right lane
				check_and_set_min_car_dist(params.nvRightBack2, distance, parentDriver->vehicle,other_driver);
			}
		}
	}

	if (otherRoadSegment->getLink() != parentDriver->vehicle->getCurrLink()) { //We are in the different link.
		if (!(parentDriver->vehicle->isInIntersection()) && parentDriver->vehicle->getNextSegment(false) == otherRoadSegment) { //Vehicle is on the next segment,which is in next link after intersection.
			// 1. host vh's target lane is == other_driver's lane
			//
//			if (getParams().now.ms()/1000.0 >  93.7
//					 && parent->getId() == 402)
//			{
//					std::cout<<"find 332288 " <<other_driver->parent->getId()<<std::endl;
//			}
			size_t targetLaneIndex = params.nextLaneIndex;
			size_t otherVhLaneIndex = getLaneIndex(other_lane);
			if (targetLaneIndex == otherVhLaneIndex)
			{

				if (params.now.ms()/1000.0 == 92.8)
				{
					int a=1;
				}
				if (params.nvFwd.driver==NULL)
				{
//					std::cout<<"find this " <<other_driver->parent->getId()<<std::endl;
					// 2. other_driver's distance move in the segment, it is also the distance vh to intersection
					double currSL = parentDriver->vehicle->getCurrentSegmentLength();
					double disMIS = parentDriver->vehicle->getDistanceMovedInSegment();
					double otherdis = other_driver->vehicle->getDistanceMovedInSegment();
					double distance =  currSL- disMIS + otherdis;
					// 3. compare the distance and set params.nvFwdNextLink
					check_and_set_min_nextlink_car_dist(params.nvFwdNextLink, distance, parentDriver->vehicle, other_driver);
				}
			}
		}
	} // end of in different link
	return true;
}

void sim_mob::DriverMovement::updateNearbyAgent(const Agent* other, const Pedestrian* pedestrian) {
	DriverUpdateParams& params = parentDriver->getParams();
	//Only update if passed a valid pointer and this is on a crossing.

	if (!(pedestrian && pedestrian->isOnCrossing())) {
		return;
	}

	//TODO: We are using a vector to check the angle to the Pedestrian. There are other ways of doing this which may be more accurate.
	const std::vector<sim_mob::Point2D>& polyLine = parentDriver->vehicle->getCurrSegment()->getLanes().front()->getPolyline();
	DynamicVector otherVect(polyLine.front().getX(), polyLine.front().getY(), other->xPos.get(), other->yPos.get());

	//Calculate the distance between these two vehicles and the distance between the angle of the
	// car's forward movement and the pedestrian.
	//NOTE: I am changing this slightly, since cars were stopping for pedestrians on the opposite side of
	//      the road for no reason (traffic light was green). ~Seth
	//double distance = otherVect.getMagnitude();
	double angleDiff = 0.0;
	{
		//Retrieve
		DynamicVector fwdVector(parentDriver->vehicle->getCurrPolylineVector());
		fwdVector.scaleVectTo(100);

		//Calculate the difference
		//NOTE: I may be over-complicating this... we can probably use the dot product but that can be done later. ~Seth
		double angle1 = atan2(fwdVector.getEndY() - fwdVector.getY(), fwdVector.getEndX() - fwdVector.getX());
		double angle2 = atan2(otherVect.getEndY() - otherVect.getY(), otherVect.getEndX() - otherVect.getX());
		double diff = fabs(angle1 - angle2);
		angleDiff = std::min(diff, fabs(diff - 2 * M_PI));
	}

	//If the pedestrian is not behind us, then set our flag to true and update the minimum pedestrian distance.
	if (angleDiff < 0.5236) { //30 degrees +/-
		params.npedFwd.distance = std::min(params.npedFwd.distance, otherVect.getMagnitude() - parentDriver->vehicle->length / 2
				- 300);
	}
}

void sim_mob::DriverMovement::updateNearbyAgents() {
	DriverUpdateParams& params = parentDriver->getParams();
	//Retrieve a list of nearby agents

	double dis = 10000.0;

#if 0
//	std::cout << "this->parent->run_on_thread_id:" << this->parent->run_on_thread_id << std::endl;
	//sim_mob::PerformanceProfile::instance().markStartQuery(this->parent->run_on_thread_id);
	vector<const Agent*> nearby_agents = AuraManager::instance().nearbyAgents(
			Point2D(parentDriver->vehicle->getX(), parentDriver->vehicle->getY()), *params.currLane, dis, parentDriver->distanceBehind);
	//sim_mob::PerformanceProfile::instance().markEndQuery(this->parent->run_on_thread_id);
#else
	PROFILE_LOG_QUERY_START(getParent()->currWorkerProvider, getParent(), params.now);

	//NOTE: Let the AuraManager handle dispatching to the "advanced" function.
	vector<const Agent*> nearby_agents;
	if(parentDriver->vehicle->getX() > 0 && parentDriver->vehicle->getY() > 0) {
		const Agent* parentAgent = (parentDriver?parentDriver->getParent():nullptr);
		nearby_agents = AuraManager::instance().nearbyAgents(Point2D(parentDriver->vehicle->getX(), parentDriver->vehicle->getY()), *params.currLane, dis, parentDriver->distanceBehind, parentAgent);
	} else {
		Warn() << "A driver's location (x or y) is < 0, X:" << parentDriver->vehicle->getX() << ",Y:" << parentDriver->vehicle->getY() << std::endl;
	}
	/*if (this->parent->connector_to_Sim_Tree) {
		if(parentDriver->vehicle->getX() > 0 && parentDriver->vehicle->getY() > 0) {
			nearby_agents = AuraManager::instance().advanced_nearbyAgents(Point2D(parentDriver->vehicle->getX(), parentDriver->vehicle->getY()), *params.currLane, dis, parentDriver->distanceBehind, this->parent->connector_to_Sim_Tree);
		} else {
			Warn() << "A driver's location (x or y) is < 0, X:" << parentDriver->vehicle->getX() << ",Y:" << parentDriver->vehicle->getY() << std::endl;
		}
	} else {
		if(parentDriver->vehicle->getX() > 0 && parentDriver->vehicle->getY() > 0)
		nearby_agents = AuraManager::instance().nearbyAgents(Point2D(parentDriver->vehicle->getX(), parentDriver->vehicle->getY()), *params.currLane, dis, parentDriver->distanceBehind);
	}*/


	PROFILE_LOG_QUERY_END(getParent()->currWorkerProvider, getParent(), params.now);
#endif
	//Update each nearby Pedestrian/Driver

	//
	params.nvFwdNextLink.driver=NULL;
	params.nvFwdNextLink.distance = 50000;
	params.nvFwd.driver=NULL;
	params.nvFwd.distance = 50000;

//	if (getParams().now.ms()/1000.0 >  93.7
//			 && parent->getId() == 402)
//		{
//			std::cout<<"asdf"<<std::endl;
//		}

	for (vector<const Agent*>::iterator it = nearby_agents.begin(); it != nearby_agents.end(); it++) {
		//Perform no action on non-Persons
		const Person* other = dynamic_cast<const Person *> (*it);
		if (!other) {
			continue;
		}

//		other->getRole()->handleUpdateRequest();
		//Perform a different action depending on whether or not this is a Pedestrian/Driver/etc.
		/*Note:
		 * In the following methods(updateNearbyDriver and updateNearbyPedestrian), the variable "other"
		 * is the target which is being analyzed, and the current object is the one who i object is the analyzer.
		 *
		 * In order to remove the ugly dynamic_cast s passed into the following method,the analyzed and anlayzer
		 * should switch their place and, consequently, the following methods and some of their sub-methods
		 * need to be rewritten. for now, we reduce the number of dynamic_casts by calling only one of the functions.
		 * It originally had to be like this(only one of them need to be called).
		 */

		other->getRole()->handleUpdateRequest(this);
//		boost::function<void(sim_mob::Person*,sim_mob::Role*)> Fn = boost::bind(&DriverMovement::p, this,_1,_2);
//		if(!updateNearbyAgent(other, dynamic_cast<const Driver*> (other->getRole())))
//		{
//			updateNearbyAgent(other, dynamic_cast<const Pedestrian*> (other->getRole()));
//		}
	}
}

void sim_mob::DriverMovement::perceivedDataProcess(NearestVehicle & nv, DriverUpdateParams& params)
{
	//Update your perceptions for leading vehicle and gap
	if (nv.exists()) {

		if (parentDriver->reacTime == 0)
		{
			params.perceivedFwdVelocityOfFwdCar = nv.driver?nv.driver->fwdVelocity.get():0;
			params.perceivedLatVelocityOfFwdCar = nv.driver?nv.driver->latVelocity.get():0;
			params.perceivedAccelerationOfFwdCar = nv.driver?nv.driver->fwdAccel.get():0;
			params.perceivedDistToFwdCar = nv.distance;

			return;
		}
		//Change perception delay
		parentDriver->perceivedDistToFwdCar->set_delay(parentDriver->reacTime);
		parentDriver->perceivedVelOfFwdCar->set_delay(parentDriver->reacTime);
		parentDriver->perceivedAccOfFwdCar->set_delay(parentDriver->reacTime);

		//Now sense.
		if (parentDriver->perceivedVelOfFwdCar->can_sense()
				&&parentDriver->perceivedAccOfFwdCar->can_sense()
				&&parentDriver->perceivedDistToFwdCar->can_sense()) {
			params.perceivedFwdVelocityOfFwdCar = parentDriver->perceivedVelOfFwdCar->sense();
			params.perceivedAccelerationOfFwdCar = parentDriver->perceivedAccOfFwdCar->sense();
			params.perceivedDistToFwdCar = parentDriver->perceivedDistToFwdCar->sense();
//			std::cout<<"perceivedDataProcess: perceivedFwdVelocityOfFwdCar: vel,acc,dis:"<<params.perceivedFwdVelocityOfFwdCar
//					<<" "<<params.perceivedAccelerationOfFwdCar<<" "<<
//					params.perceivedDistToFwdCar<<std::endl;
		}
		else
		{
			params.perceivedFwdVelocityOfFwdCar = nv.driver?nv.driver->fwdVelocity.get():0;
			params.perceivedLatVelocityOfFwdCar = nv.driver?nv.driver->latVelocity.get():0;
			params.perceivedAccelerationOfFwdCar = nv.driver?nv.driver->fwdAccel.get():0;
			params.perceivedDistToFwdCar = nv.distance;
		}
		parentDriver->perceivedDistToFwdCar->delay(nv.distance);
		parentDriver->perceivedVelOfFwdCar->delay(nv.driver->fwdVelocity.get());
		parentDriver->perceivedAccOfFwdCar->delay(nv.driver->fwdAccel.get());
	}


}

NearestVehicle & sim_mob::DriverMovement::nearestVehicle(DriverUpdateParams& p)
{
	double leftDis = 5000;
	double rightDis = 5000;
	double currentDis = 5000;
	p.isBeforIntersecton = false;
	if(p.nvLeftFwd.exists())
	  leftDis = p.nvLeftFwd.distance;
	if(p.nvRightFwd.exists())
	  rightDis = p.nvRightFwd.distance;
	if(p.nvFwd.exists())
	{
	  currentDis = p.nvFwd.distance;
	}
	else if(p.nvFwdNextLink.exists() && p.turningDirection == LCS_SAME)
	{
		currentDis = p.nvFwdNextLink.distance;
		p.isBeforIntersecton = true;
//		if (currentDis<200 && getParams().now.ms()/1000.0 > 100.0 )
//		{
//			std::cout<<"find one"<<std::endl;
//		}
		/*if (getParams().now.ms()/1000.0 > 41.8 && parent->getId() == 25)
					std::cout<<"find vh"<<std::endl;*/
		return p.nvFwdNextLink;
	}
	if(leftDis<currentDis)
	{
		//the vehicle in the left lane is turning to right
		//or subject vehicle is turning to left
		if(p.nvLeftFwd.driver->turningDirection.get()==LCS_RIGHT &&
				parentDriver->vehicle->getTurningDirection()==LCS_LEFT && p.nvLeftFwd.driver->getVehicle()->getVelocity()>500)
		{
//			std::cout<<"nearestVehicle: left forward"<<std::endl;
			return p.nvLeftFwd;
		}
	}
	else if(rightDis<currentDis)
	{
		if(p.nvRightFwd.driver->turningDirection.get()==LCS_LEFT &&
				parentDriver->vehicle->getTurningDirection()==LCS_RIGHT && p.nvRightFwd.driver->getVehicle()->getVelocity()>500)
		{
//			std::cout<<"nearestVehicle: right forward: rightDis,currentDis: "<<rightDis<<" "<<currentDis<<std::endl;
			return p.nvRightFwd;
		}
	}
//	if (p.nvFwd.exists())
//		std::cout<<"nearestVehicle: forward"<<std::endl;
	return p.nvFwd;
}

void sim_mob::DriverMovement::intersectionVelocityUpdate() {
	double inter_speed = 1000;//10m/s
	parentDriver->vehicle->setAcceleration(0);

	//Set velocity for intersection movement.
	parentDriver->vehicle->setVelocity(inter_speed);
}

void sim_mob::DriverMovement::justLeftIntersection(DriverUpdateParams& p) {
	//p.currLane = nextLaneInNextLink;
	p.currLaneIndex = getLaneIndex(p.currLane);
	parentDriver->vehicle->moveToNewLanePolyline(p.currLaneIndex);
	syncCurrLaneCachedInfo(p);
	p.currLaneOffset = parentDriver->vehicle->getDistanceMovedInSegment();
	targetLaneIndex = p.currLaneIndex;

	//Reset lateral movement/velocity to zero.
	parentDriver->vehicle->setLatVelocity(0);
	parentDriver->vehicle->resetLateralMovement();
}

LANE_CHANGE_SIDE sim_mob::DriverMovement::getCurrLaneChangeDirection() const {
	if (parentDriver->vehicle->getLatVelocity() > 0) {
		return LCS_LEFT;
	} else if (parentDriver->vehicle->getLatVelocity() < 0) {
		return LCS_RIGHT;
	}
	return LCS_SAME;
}

LANE_CHANGE_SIDE sim_mob::DriverMovement::getCurrLaneSideRelativeToCenter() const {
	if (parentDriver->vehicle->getLateralMovement() > 0) {
		return LCS_LEFT;
	} else if (parentDriver->vehicle->getLateralMovement() < 0) {
		return LCS_RIGHT;
	}
	return LCS_SAME;
}

//TODO: I think all lane changing occurs after 150m. Double-check please. ~Seth
void sim_mob::DriverMovement::updatePositionDuringLaneChange(DriverUpdateParams& p, LANE_CHANGE_SIDE relative) {

	double halfLaneWidth = p.currLane->getWidth() / 2.0;

	//The direction we are attempting to change lanes in
	LANE_CHANGE_SIDE actual = parentDriver->vehicle->getTurningDirection();
	//LANE_CHANGE_SIDE relative = getCurrLaneSideRelativeToCenter();
	if (actual == LCS_SAME && relative == LCS_SAME) {
		if (Debug::Drivers) {
			DebugStream << "  Lane change: " << "ERROR: Called with \"same\"" << endl;
		}
		return; //Not actually changing lanes.
	}
	if (relative == LCS_SAME) {
		relative = actual; //Unlikely to occur, but we can still work off this.
	}
	if (actual == relative) { //We haven't merged halfway yet; check there's actually a lane for us to merge into.
		if ((actual == LCS_LEFT && !p.leftLane) || (actual == LCS_RIGHT && !p.rightLane)) {
			std::stringstream msg;
			msg <<"Agent (" <<getParent()->getId() <<") is attempting to merge into a lane that doesn't exist.";
			throw std::runtime_error(msg.str().c_str());
			//return; //Error condition
		}
	}

	if (Debug::Drivers) {
		DebugStream << "  Lane change: " << PrintLCS(actual) << " (" << PrintLCS(relative) << ")" << endl;
	}

	//Basically, we move "halfway" into the next lane, and then move "halfway" back to its midpoint.
	if (actual == relative) {
		//Moving "out".
		double remainder = fabs(parentDriver->vehicle->getLateralMovement()) - halfLaneWidth;

		if (Debug::Drivers) {
			DebugStream << "    Moving out on Lane " << p.currLaneIndex << ": " << remainder << endl;
		}

		if (remainder >= 0) {
			//Update Lanes, polylines, RoadSegments, etc.
			p.currLane = (actual == LCS_LEFT ? p.leftLane : p.rightLane);
			syncCurrLaneCachedInfo(p);

			parentDriver->vehicle->shiftToNewLanePolyline(actual == LCS_LEFT);

			if (Debug::Drivers) {
				DebugStream << "    Shifting to new lane." << endl;
			}

			//Check
			if (p.currLane->is_pedestrian_lane()) {
				//Flush debug output (we are debugging this error).
				if (Debug::Drivers) {
					if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
						DebugStream << ">>>Exception: Moved to sidewalk." << endl;
						PrintOut(DebugStream.str());
					}
				}

				//TEMP OVERRIDE:
				//TODO: Fix!
				getParent()->setToBeRemoved();
				return;

				std::stringstream msg;
				msg << "Error: Car has moved onto sidewalk. Agent ID: " << getParent()->getId();
				throw std::runtime_error(msg.str().c_str());
			}

			//Set to the far edge of the other lane, minus any extra amount.
			halfLaneWidth = p.currLane->getWidth() / 2.0;
			parentDriver->vehicle->resetLateralMovement();
			parentDriver->vehicle->moveLat((halfLaneWidth - remainder) * (actual == LCS_LEFT ? -1 : 1));
		}
	} else {
		//Moving "in".
		if (fabs(parentDriver->vehicle->getLateralMovement()) < 30)
		{
			parentDriver->vehicle->resetLateralMovement();
			return;
		}
		if (relative == LCS_LEFT)
			parentDriver->vehicle->moveLat(-15);
		else
			parentDriver->vehicle->moveLat(15);
//		bool pastZero = (relative == LCS_LEFT) ? (vehicle->getLateralMovement() >= 0)
//				: (vehicle->getLateralMovement() <= 0);
//
//		if (Debug::Drivers) {
//			DebugStream << "    Moving in on lane " << p.currLaneIndex << ": " << vehicle->getLateralMovement() << endl;
//		}
//
//		if (pastZero) {
//
//			//Reset all
//			vehicle->resetLateralMovement();
//			vehicle->setLatVelocity(0);
//			vehicle->setTurningDirection(LCS_SAME);
//
//			if (Debug::Drivers) {
//				DebugStream << "    New lane shift complete." << endl;
//			}
//		}

	}
}

//Retrieve the current traffic signal based on our RoadSegment's end node.
void sim_mob::DriverMovement::saveCurrTrafficSignal() {
//	const Node* node = vehicle->getCurrSegment()->getEnd();
	const Node* node;
	if(parentDriver->vehicle->isMovingForwardsInLink())
		node = parentDriver->vehicle->getCurrLink()->getEnd();
	else
		node = parentDriver->vehicle->getCurrLink()->getStart();
	trafficSignal = node ? StreetDirectory::instance().signalAt(*node) : nullptr;
}

void sim_mob::DriverMovement::setTrafficSignalParams(DriverUpdateParams& p) {


	if (!trafficSignal) {
			p.trafficColor = sim_mob::Green;

			parentDriver->perceivedTrafficColor->delay(p.trafficColor);
	} else {
		sim_mob::TrafficColor color;

		if (parentDriver->vehicle->hasNextSegment(false)) {

//			std::cout << "In Driver::setTrafficSignalParams, frame number " << p.frameNumber <<
//					"  Getting the driver light from lane " << p.currLane  <<
//					"(" << p.currLane->getRoadSegment()->getLink()->roadName << ")" <<
//					" To "<< nextLaneInNextLink  <<
//					"(" << nextLaneInNextLink->getRoadSegment()->getLink()->roadName << ")" << std::endl;


			color = trafficSignal->getDriverLight(*p.currLane, *nextLaneInNextLink);


//			std::cout << "The driver light is " << color << std::endl;
		} else {
			/*vahid:
			 * Basically,there is no notion of left, right forward any more.
			 * (I said "Basically" coz I can think of at least one "if" :left turn in singapore, right turn in US...)
			 * so it is omitted by  If you insist on having this type of function, I can give you a vector/container
			 * of a map between lane/link and their corresponding current color with respect to the currLane
			 * todo:think of something for this else clause! you are going continue with No color!S
			 */
//			color = trafficSignal->getDriverLight(*p.currLane).forward;
			color = sim_mob::Green;
		}
		switch (color) {
		case sim_mob::Red:

//			std::cout<< "Driver is getting Red light \n";
			p.trafficColor = color;
			break;
		case sim_mob::Amber:
		case sim_mob::Green:

//			std::cout<< "Driver is getting Green or Amber light \n";
			if (!isPedestrianOnTargetCrossing())
				p.trafficColor = color;
			else
				p.trafficColor = sim_mob::Red;
				break;
		default:
			Warn() <<"Unknown signal color[" << color << "]\n";
			break;
		}

		parentDriver->perceivedTrafficColor->set_delay(parentDriver->reacTime);
		if(parentDriver->perceivedTrafficColor->can_sense())
		{
			p.perceivedTrafficColor = parentDriver->perceivedTrafficColor->sense();
		}
		else
			p.perceivedTrafficColor = color;

		parentDriver->perceivedTrafficColor->delay(p.trafficColor);


		p.trafficSignalStopDistance = parentDriver->vehicle->getAllRestRoadSegmentsLength() - parentDriver->vehicle->getDistanceMovedInSegment() - parentDriver->vehicle->length / 2;
		parentDriver->perceivedDistToTrafficSignal->set_delay(parentDriver->reacTime);
		if(parentDriver->perceivedDistToTrafficSignal->can_sense())
		{
			p.perceivedDistToTrafficSignal = parentDriver->perceivedDistToTrafficSignal->sense();
		}
		else
			p.perceivedDistToTrafficSignal = p.trafficSignalStopDistance;
		parentDriver->perceivedDistToTrafficSignal->delay(p.trafficSignalStopDistance);
	}
}
}
