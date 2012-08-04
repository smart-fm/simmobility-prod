/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Pedestrian.cpp
 *
 *  Created on: 2011-6-20
 *      Author: Linbo
 */

#include "Pedestrian.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "geospatial/Node.hpp"
#include "util/OutputUtil.hpp"
#include "util/GeomHelpers.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/BusStop.hpp"
#include "entities/Signal.hpp"
#include "util/GeomHelpers.hpp"
#include "geospatial/Point2D.hpp"

using std::vector;
using namespace sim_mob;

namespace {

//For random number generating
boost::uniform_int<> zero_to_five(0, 5);
boost::uniform_int<> zero_to_max(0, RAND_MAX);

vector<const RoadSegment*> BuildUpPath(vector<RoadSegment*>::iterator curr, vector<RoadSegment*>::iterator end)
{
	vector<const RoadSegment*> res;
	for (; curr != end; curr++) {
		res.push_back(*curr);
	}
	return res;
}

vector<const RoadSegment*> ForceForwardSubpath(const RoadSegment* revSegment, vector<RoadSegment*> candList1, vector<
		RoadSegment*> candList2) {
	//First, iterate through each list until we find an item that is the REVERSE of our revSegment
	for (int i = 0; i < 2; i++) {
		vector<RoadSegment*>& cand = (i == 0) ? candList1 : candList2;
		for (vector<RoadSegment*>::iterator it = cand.begin(); it != cand.end(); it++) {

			//Negative: break early if we find the same segment.
			if ((*it)->getStart() == revSegment->getStart() && (*it)->getEnd() == revSegment->getEnd()) {
				break;
			}

			//Positive: return if we find the reverse segment
			if ((*it)->getStart() == revSegment->getEnd() && (*it)->getEnd() == revSegment->getStart()) {
				//LogOut("noteForDebug ForceForwardSubpath run reverse"<<std::endl);
				return BuildUpPath(it, cand.end());

			}
		}
	}

	//Error:
	throw std::runtime_error("Can't retrieve forward subpath for the given candidates.");
}


//Helper function: Return the closest bus stop and its distance from the pedestrian
std::pair<const BusStop*, double> calcNearestBusStop(const RoadSegment* rs, const DPoint& pos, double stoppingDist) {
	typedef std::map<centimeter_t, const RoadItem*>::const_iterator RoadObstIt;

	std::pair<const BusStop*, double> res(nullptr, 0);
	for(RoadObstIt o_it=rs->obstacles.begin(); o_it!=rs->obstacles.end(); o_it++) {
		const BusStop* bs = dynamic_cast<const BusStop*>(o_it->second);
		if(!bs) {
			continue;
		}

		//Check if it's closer.
		double newDist = sim_mob::dist(bs->xPos, bs->yPos, pos.x, pos.y);
		if ((!res.first) || newDist<res.second) {
			res.first = bs;
			res.second = newDist;

			//Stop early?
			if (newDist < stoppingDist) {
				break;
			}
		}
	}

	return res;
}

std::pair<const BusDriver*, double> calcNearestBusDriver(unsigned int myID, const DPoint& pos, double stoppingDist) {
	std::pair<BusDriver*, double> res(nullptr, 0);
	for (size_t i = 0; i < Agent::all_agents.size(); i++) {
		//Retrieve only Bus Driver agents.
		Person* p = dynamic_cast<Person*>(Agent::all_agents[i]);
		BusDriver* bd = p ? dynamic_cast<BusDriver*>(p->getRole()) : nullptr;
		if (!bd) {
			continue;
		}

		//Determine its distance; compare.
		double newDist = sim_mob::dist(bd->getPositionX(), bd->getPositionY(), pos.x, pos.y);
		if ((!res.first) || newDist<res.second) {
			res.first = bd;
			res.second = newDist;

			//Stop early?
			if (newDist < stoppingDist) {
				break;
			}
		}
	}
	return res;
}

}//End anonymous namespace


double Pedestrian::collisionForce = 20;
double Pedestrian::agentRadius = 0.5; //Shoulder width of a person is about 0.5 meter


sim_mob::Pedestrian::Pedestrian(Agent* parent, boost::mt19937& gen) :
	Role(parent), prevSeg(nullptr), isUsingGenPathMover(true), params(parent->getGenerator()) {
	//Check non-null parent. Perhaps references may be of use here?
	if (!parent) {
		std::cout << "Role constructed with no parent Agent." << std::endl;
		throw 1;
	}

	//Init
#ifdef SIMMOB_NEW_SIGNAL
	sigColor = sim_mob::Green; //Green by default
#else
	sigColor = Signal::Green; //Green by default
#endif

	startToCross = false;

	//Set default speed in the range of 1.2m/s to 1.6m/s
	speed = 1.2+(double(zero_to_five(gen)))/10;

	xVel = 0;
	yVel = 0;

	xCollisionVector = 0;
	yCollisionVector = 0;

	atSidewalk = true;
	atCrossing = false;
	gotoCrossing = false;
	crossingCount = 0;

}

//Note that a destructor is not technically needed, but I want to enforce the idea
//  of overriding virtual destructors if they exist.
sim_mob::Pedestrian::~Pedestrian() {
}

vector<BufferedBase*> sim_mob::Pedestrian::getSubscriptionParams() {
	vector<BufferedBase*> res;
	return res;
}



void sim_mob::Pedestrian::frame_init(UpdateParams& p)
{
	setSubPath();
	dynamic_cast<PedestrianUpdateParams&>(p).skipThisFrame = true;
}


UpdateParams& sim_mob::Pedestrian::make_frame_tick_params(frame_t frameNumber, unsigned int currTimeMS)
{
	params.reset(frameNumber, currTimeMS);
	return params;
}


//Main update method
void sim_mob::Pedestrian::frame_tick(UpdateParams& p)
{
	PedestrianUpdateParams& p2 = dynamic_cast<PedestrianUpdateParams&>(p);

	//Is this the first frame tick?
	if (p2.skipThisFrame) {
		return;
	}

	//Check for Bus Stops
	if (isAtBusStop()) {
		return;
	}

	//Check if the agent has reached the destination
	if (isDestReached()) {
		parent->setToBeRemoved();
		return;
	}

	if (isGoalReached()) {
		//++currentStage;
		setSubPath(); //Set next goal
		return;
	}

	if(atSidewalk){
		double vel = speed * 1.2 * 100 * ConfigParams::GetInstance().agentTimeStepInMilliSeconds() / 1000.0;

		prevSeg = fwdMovement.getCurrSegment();
		fwdMovement.advance(vel);
		//fwdMovement.advance(fwdMovement.getCurrSegment(), pathWithDirection.path, pathWithDirection.areFwds, vel);
		if (!fwdMovement.isDoneWithEntireRoute() && !fwdMovement.isInIntersection() && prevSeg
				!= fwdMovement.getCurrSegment()) {
			//Move onto the outer lane (sidewalk).
			//TODO: This isn't always correct on one-way streets.
			fwdMovement.moveToNewPolyline(fwdMovement.getCurrSegment()->getLanes().size() - 1);
		}

		parent->xPos.set(fwdMovement.getPosition().x);
		parent->yPos.set(fwdMovement.getPosition().y);

	}
	else if(atCrossing){
		//Check whether to start to cross or not
		LogOut("noteForDebug updatePedestrianSignal run"<<std::endl);
		updatePedestrianSignal(fwdMovement.pathWithDirection.areFwds.front());

#ifdef SIMMOB_NEW_SIGNAL
		if (!startToCross) {
			if (sigColor == sim_mob::Green){ //Green phase
				LogOut("noteForDebug Green signal 1"<<std::endl);
				startToCross = true;
			}
			else if (sigColor == sim_mob::Red) { //Red phase
				if (checkGapAcceptance() == true)
					startToCross = true;
			}
		}

		if (startToCross) {
			if (sigColor == sim_mob::Green) //Green phase
				updateVelocity(1);
			else if (sigColor == sim_mob::Red) //Red phase
				updateVelocity(2);
			updatePosition();
		}
#else
		if (!startToCross) {
			if (sigColor == Signal::Green){ //Green phase
				LogOut("noteForDebug Green signal 2"<<std::endl);
				startToCross = true;
			}
			else if (sigColor == Signal::Red) { //Red phase
				if (checkGapAcceptance() == true)
					startToCross = true;
			}
		}

		if (startToCross) {
			if (sigColor == Signal::Green) //Green phase
				updateVelocity(1);
			else if (sigColor == Signal::Red) //Red phase
				updateVelocity(2);
			updatePosition();
		}
#endif
		else {
			//Output (temp)
#ifndef SIMMOB_DISABLE_OUTPUT
			LogOut("Pedestrian " <<parent->getId() <<" is waiting at the crossing" <<std::endl);
#endif
		}
	}
}

void sim_mob::Pedestrian::frame_tick_med(UpdateParams& p){
	/*to be implemented by supply team to move the pedestrian after each time tick
	 *
	 */
}

void sim_mob::Pedestrian::frame_tick_output(const UpdateParams& p)
{
	if (dynamic_cast<const PedestrianUpdateParams&>(p).skipThisFrame) {
		return;
	}

	if (ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

#ifndef SIMMOB_DISABLE_OUTPUT
	LogOut("("<<"\"pedestrian\","<<p.frameNumber<<","<<parent->getId()<<","<<"{\"xPos\":\""<<parent->xPos.get()<<"\"," <<"\"yPos\":\""<<this->parent->yPos.get()<<"\",})"<<std::endl);
#endif
}

void sim_mob::Pedestrian::frame_tick_output_mpi(frame_t frameNumber)
{
	if (frameNumber < 1 || frameNumber < parent->getStartTime())
		return;

#ifndef SIMMOB_DISABLE_OUTPUT
	if (this->parent->isFake) {
		LogOut("("<<"\"pedestrian\","<<frameNumber<<","<<parent->getId()<<","<<"{\"xPos\":\""<<parent->xPos.get()<<"\"," <<"\"yPos\":\""<<this->parent->yPos.get() <<"\"," <<"\"xVel\":\""<< this->xVel <<"\"," <<"\"yVel\":\""<< this->yVel <<"\"," <<"\"fake\":\""<<"true" <<"\",})"<<std::endl);
	} else {
		LogOut("("<<"\"pedestrian\","<<frameNumber<<","<<parent->getId()<<","<<"{\"xPos\":\""<<parent->xPos.get()<<"\"," <<"\"yPos\":\""<<this->parent->yPos.get() <<"\"," <<"\"xVel\":\""<< this->xVel <<"\"," <<"\"yVel\":\""<< this->yVel <<"\"," <<"\"fake\":\""<<"false" <<"\",})"<<std::endl);
	}
#endif
}

/*---------------------Perception-related functions----------------------*/

void sim_mob::Pedestrian::setSubPath() {


	if(atSidewalk){

		//LogOut("noteForDebug setSubPath run atSideWalk"<<std::endl);

		vector<WayPoint> wp_path = StreetDirectory::instance().shortestWalkingPath(parent->originNode->location,
				parent->destNode->location);

		//For debug ---------------------------------------
		std::cout<<"Size: "<< wp_path.size()<<std::endl;
		for (vector<WayPoint>::iterator it = wp_path.begin(); it != wp_path.end(); it++){
			if (it->type_ == WayPoint::SIDE_WALK){
				std::cout<<"Side_walk start node "<<it->lane_->getRoadSegment()->getStart()->getID()<<"("<<it->lane_->getRoadSegment()->getStart()->location.getX()<<","<<it->lane_->getRoadSegment()->getStart()->location.getY()<<") end node "<<it->lane_->getRoadSegment()->getEnd()->getID()<<"("<<it->lane_->getRoadSegment()->getEnd()->location.getX()<<","<<it->lane_->getRoadSegment()->getEnd()->location.getY()<<")"<<std::endl;
//				const Lane* side_walk = it->lane_;
//				const std::vector<Point2D> polyline = side_walk->getPolyline();
//				std::cout << "side-walk start=" << polyline[0] << " end=" << polyline[polyline.size() - 1] << std::endl;

			}
			else if (it->type_ == WayPoint::ROAD_SEGMENT)
				std::cout<<"Road_segment"<<std::endl;
			else if (it->type_ == WayPoint::BUS_STOP)
				std::cout<<"Bus_stop"<<std::endl;
			else if (it->type_ == WayPoint::CROSSING){
//				std::cout<<"Crossing"<<std::endl;
				std::cout << "crossing near-line start=" << it->crossing_->nearLine.first << " end=" << it->crossing_->nearLine.second << std::endl;
			}
			else if (it->type_ == WayPoint::NODE){
//				std::cout<<"Node at xPos "<<it->node_->location.getX()<<" ,yPos "<<it->node_->location.getY()<<std::endl;
				std::cout << "node location=" << it->node_->location << std::endl;
			}
			else if (it->type_ == WayPoint::INVALID)
				std::cout<<"Invalid"<<std::endl;
			else
				std::cout<<"No_match"<<std::endl;
		}

		//----------------------------------------------------

		const Lane* nextSideWalk = nullptr; //For the old code
		sim_mob::GeneralPathMover::PathWithDirection segWithDirection;
		//vector<const RoadSegment*> path;

			int laneID = -1; //Also save the lane id.
			bool isPassedSeg=false;
			for (vector<WayPoint>::iterator it = wp_path.begin(); it != wp_path.end(); it++) {
				if (it->type_ == WayPoint::SIDE_WALK) {
					//Save
					if (!nextSideWalk) {
						nextSideWalk = it->lane_;
					}

					//If we're changing Links, stop. Thus, "path" contains only the Segments needed to reach the Intersection, and no more.
					//NOTE: Later, you can send the ENTIRE shortestWalkingPath to fwdMovement and just handle "isInIntersection()"
					RoadSegment* rs = it->lane_->getRoadSegment();
					isPassedSeg=false;
					if(!currPath.empty()){
						for (int i =0; i<currPath.size(); i++){
							if((currPath.at(i)->getStart()==rs->getStart()&&currPath.at(i)->getEnd()==rs->getEnd())||(currPath.at(i)->getStart()==rs->getEnd()&&currPath.at(i)->getEnd()==rs->getStart()))
								isPassedSeg=true;
						}
					}
					if(isPassedSeg)
						continue;

					if (!segWithDirection.path.empty() && segWithDirection.path.back()->getLink() != rs->getLink()) {
						if((it-1)->type_==WayPoint::CROSSING)
							gotoCrossing=true;
						break;
					}

					//Add it.
					segWithDirection.path.push_back(rs);
					laneID = it->lane_->getLaneID();
				}
			}

			if (segWithDirection.path.empty() || laneID == -1) {
				throw std::runtime_error("Can't find path for Pedestrian.");
			}

			//TEMP: Currently, GeneralPathMover doesn't like walking on Segments in reverse. This is not too
			//      difficult to fix, but for now I'm just flipping the path.
			if (currPath.empty()){

				LogOut("noteForDebug setSubPath run atSideWalk binary 1"<<std::endl);

				if(segWithDirection.path.front()->getEnd() == parent->originNode) {

					LogOut("noteForDebug setSubPath run atSideWalk binary 1.1"<<std::endl);

					//segWithDirection.path = ForceForwardSubpath(segWithDirection.path.front(), segWithDirection.path.front()->getLink()->getPath(true),
					//		segWithDirection.path.front()->getLink()->getPath(false));
					laneID = segWithDirection.path.front()->getLanes().size() - 1;
				}
			}
			else{

				LogOut("noteForDebug setSubPath run atSideWalk binary 2"<<std::endl);

				if(segWithDirection.path.front()->getEnd() == currPath.back()->getEnd()) {

					LogOut("noteForDebug setSubPath run atSideWalk binary 2.1"<<std::endl);

					//segWithDirection.path = ForceForwardSubpath(segWithDirection.path.front(), segWithDirection.path.front()->getLink()->getPath(true),
					//		segWithDirection.path.front()->getLink()->getPath(false));
					laneID = segWithDirection.path.front()->getLanes().size() - 1;
				}
			}

			//Set the path
			fwdMovement.setPath(segWithDirection.path, segWithDirection.areFwds, laneID);

			currPath.insert(currPath.end(),segWithDirection.path.begin(),segWithDirection.path.end());

			parent->xPos.set(fwdMovement.getPosition().x);
			parent->yPos.set(fwdMovement.getPosition().y);

			//pathWithDirection = segWithDirection;
			LogOut("noteForDebug fwdMovement.pathWithDirection set"<<std::endl);
			fwdMovement.pathWithDirection = segWithDirection;

	}
	else if(atCrossing){

		//LogOut("noteForDebug setSubPath run atCrossing"<<std::endl);

		vector<WayPoint> wp_path = StreetDirectory::instance().shortestWalkingPath(parent->originNode->location,
				parent->destNode->location);
		bool isPassedCrossing=false;
		vector<const Crossing*> newCrossings;
		if(currCrossings.empty()){

			//LogOut("noteForDebug setSubPath run atCrossing binary 1"<<std::endl);

			for (vector<WayPoint>::iterator it = wp_path.begin(); it != wp_path.end(); it++) {

				if(it->type_ == WayPoint::CROSSING){
					currCrossings.push_back(it->crossing_);
					if((it+1)->type_!= WayPoint::CROSSING){
						break;
					}
				}
			}
			initCrossing(currCrossings.at(crossingCount),parent->getGenerator());
			crossingCount++;
		}
		else{

			//LogOut("noteForDebug setSubPath run atCrossing binary 2"<<std::endl);

			if(currCrossings.size()==crossingCount){

				for (vector<WayPoint>::iterator it = wp_path.begin(); it != wp_path.end(); it++) {

					if(it->type_ == WayPoint::CROSSING){
						isPassedCrossing=false;
						for (int i =0; i<currCrossings.size(); i++){
							if(currCrossings.at(i)==it->crossing_)
								isPassedCrossing=true;
						}
						if(isPassedCrossing)
							continue;
						newCrossings.push_back(it->crossing_);
						if((it+1)->type_!= WayPoint::CROSSING){
							break;
						}
					}
				}
				currCrossings.clear();
				currCrossings.assign(newCrossings.begin(),newCrossings.end());
				crossingCount=0;
				initCrossing(currCrossings.at(crossingCount),parent->getGenerator());
				crossingCount++;
			}
			else{
				initCrossing(currCrossings.at(crossingCount),parent->getGenerator());
				crossingCount++;
			}
		}
	}
}



void sim_mob::Pedestrian::initCrossing(const Crossing* currCross,boost::mt19937& gen){

	std::cout << "initCrossing run!" << std::endl;

	double xRel, yRel;
	double xAbs, yAbs;
	double width, length, tmp;
	currCrossing = currCross;
	startToCross = false;

	Point2D far1 = currCross->farLine.first;
	Point2D far2 = currCross->farLine.second;
	Point2D near1 = currCross->nearLine.first;
	Point2D near2 = currCross->nearLine.second;


		if  (sim_mob::dist(*parent, near1) < sim_mob::dist(*parent, near2) ){
				std::cout << "ccnear1!" << std::endl;
				cStartX = (double) near1.getX();
				cStartY = (double) near1.getY();
				cEndX = (double) near2.getX();
				cEndY = (double) near2.getY();
				absToRel(cEndX, cEndY, length, tmp);
				absToRel((double) far1.getX(), (double) far1.getY(), tmp, width);
		}
		else{
				std::cout << "ccnear2!" << std::endl;
				cStartX = (double) near2.getX();
				cStartY = (double) near2.getY();
				cEndX = (double) near1.getX();
				cEndY = (double) near1.getY();
				absToRel(cEndX, cEndY, length, tmp);
				absToRel((double) far2.getX(), (double) far2.getY(), tmp, width);
		}

	xRel = 0;
	if(width<0)
		yRel = -((double)(zero_to_max(gen)%(int(abs(width)/2+1)))+(double)(zero_to_max(gen)%(int(abs(width)/2+1))));
	else
		yRel = (double)(zero_to_max(gen)%(int(width/2+1)))+(double)(zero_to_max(gen)%(int(width/2+1)));
	xRel = (yRel*tmp)/width;
	relToAbs(xRel,yRel,xAbs,yAbs);
	parent->xPos.set((int)xAbs);
	parent->yPos.set((int)yAbs);
	xRel = xRel+length;
	relToAbs(xRel,yRel,xAbs,yAbs);
	goalInLane = Point2D((int)xAbs,(int)yAbs);

}


bool sim_mob::Pedestrian::isAtBusStop() {
	//Doesn't matter if we're already done.
	if(fwdMovement.isDoneWithEntireRoute()) {
		return false;
	}

	//Retrieve the nearest bus stop (TODO: This can be done much more efficiently using the
	//  Pedestrian's current offset along the RoadSegment).
	{
		std::pair<const BusStop*, double> nearestBS = calcNearestBusStop(fwdMovement.getCurrSegment(), fwdMovement.getPosition(), 1800);
		if (!nearestBS.first) {
			return false;
		}
	}

	//Retrieve the nearest BusDriver to this stop.
	//NOTE: This should be done via the StreetDirectory; it's much faster than scanning the entire Agents list.
	std::pair<const BusDriver*, double> nearestBD = calcNearestBusDriver(parent->getId(), fwdMovement.getPosition(), 1800);
	if (!nearestBD.first) {
		return false;
	}

	//At this point, we have a valid, nearby Bus Driver. We should board the bus (but for now we
	//  will just remove ourselves).
	parent->setToBeRemoved();
	return true;
}

bool sim_mob::Pedestrian::isDestReached() {
	if(atSidewalk){
		if(fwdMovement.isDoneWithEntireRoute() && (currPath.back()->getEnd()==parent->destNode||currPath.back()->getStart()==parent->destNode)){
			return true;
		}
	}
	return false;
}

bool sim_mob::Pedestrian::isGoalReached() {
	if (atSidewalk){
		if(fwdMovement.isDoneWithEntireRoute()){
			if(gotoCrossing){
				atCrossing=true;
				atSidewalk=false;
				gotoCrossing=false;
			}
			else{
				atCrossing=false;
				atSidewalk=true;
			}
			return true;
		}

	}
	else if (atCrossing){

		double dX = ((double) abs(goalInLane.getX() - parent->xPos.get())) / 100;
		double dY = ((double) abs(goalInLane.getY() - parent->yPos.get())) / 100;
		double dis = sqrt(dX * dX + dY * dY);
		if (dis < agentRadius * 4){
			if(currCrossings.size()==crossingCount){
				atCrossing=false;
				atSidewalk=true;
			}
			return true;
		}
	}

	return false;
}

void sim_mob::Pedestrian::updatePedestrianSignal(bool isFwd) {

	if(isFwd){
		const Node* node = ConfigParams::GetInstance().getNetwork().locateNode(currPath.back()->getEnd()->location, true);
		if (node)
			trafficSignal = StreetDirectory::instance().signalAt(*node);
		else{
			LogOut("noteForDebug node for Traffic signal not found! "<<std::endl);
			trafficSignal = nullptr;
		}

		if (!trafficSignal){
			std::cout << "Traffic signal not found!" << std::endl;
			LogOut("noteForDebug Traffic signal not found!"<<std::endl);
		}
		else {
			if (currCrossing) {
				sigColor = trafficSignal->getPedestrianLight(*currCrossing);
				//			std::cout<<"Debug: signal color "<<sigColor<<std::endl;
			} else
			std::cout << "Current crossing not found!" << std::endl;
		}
	}
	else{
		const Node* node = ConfigParams::GetInstance().getNetwork().locateNode(currPath.back()->getStart()->location, true);
		if (node)
			trafficSignal = StreetDirectory::instance().signalAt(*node);
		else{
			LogOut("noteForDebug node for Traffic signal not found! "<<std::endl);
			trafficSignal = nullptr;
		}

		if (!trafficSignal){
			std::cout << "Traffic signal not found!" << std::endl;
			LogOut("noteForDebug Traffic signal not found!"<<std::endl);
		}
		else {
			if (currCrossing) {
				sigColor = trafficSignal->getPedestrianLight(*currCrossing);
				//			std::cout<<"Debug: signal color "<<sigColor<<std::endl;
			} else
			std::cout << "Current crossing not found!" << std::endl;
		}
	}
}

/*---------------------Decision-related functions------------------------*/

bool sim_mob::Pedestrian::checkGapAcceptance() {

	//	//Search for the nearest driver on the current link
	//	Agent* a = nullptr;
	//	Person* p = nullptr;
	//	double pedRelX, pedRelY, drvRelX, drvRelY;
	//	double minDist=1000000;
	//	for (size_t i=0; i<Agent::all_agents.size(); i++) {
	//		//Skip self
	//		a = Agent::all_agents[i];
	//		if (a->getId()==parent->getId()) {
	//			a = nullptr;
	//			continue;
	//		}
	//
	//		if(dynamic_cast<Person*>(a)){
	//			p=dynamic_cast<Person*>(a);
	//			if(dynamic_cast<Driver*>(p->getRole())){
	//				if(p->currentLink.get()==0){
	//					//std::cout<<"dsahf"<<std::endl;
	//					absToRel(p->xPos.get(),p->yPos.get(),drvRelX,drvRelY);
	//					absToRel(parent->xPos.get(),parent->xPos.get(),pedRelX,pedRelY);
	//					if(minDist>abs(drvRelY-pedRelY))
	//						minDist = abs(drvRelY-pedRelY);
	//				}
	//			}
	//		}
	//		p = nullptr;
	//		a = nullptr;
	//	}
	//	p = nullptr;
	//	a = nullptr;
	//
	//	if((minDist/5-30/speed)>1)
	//	{
	//
	//		return true;
	//	}
	//	else
	//		return false;

	return false;
}

/*---------------------Action-related functions--------------------------*/

void sim_mob::Pedestrian::updateVelocity(int flag) //0-on sidewalk, 1-on crossing green, 2-on crossing red
{
	//Set direction (towards the goal)
	double scale;
	xVel = ((double) (goalInLane.getX() - parent->xPos.get())) / 100;
	yVel = ((double) (goalInLane.getY() - parent->yPos.get())) / 100;
	//Normalize
	double length = sqrt(xVel * xVel + yVel * yVel);
	xVel = xVel / length;
	yVel = yVel / length;
	//Set actual velocity
	if (flag == 0)
		scale = 1.2;
	else if (flag == 1)
		scale = 1;
	else if (flag == 2)
		scale = 1.5;
	xVel = xVel * speed * scale;
	yVel = yVel * speed * scale;

	//	//Set direction (towards the goal)
	//	double xDirection = goal.getX() - parent->xPos.get();
	//	double yDirection = goal.getY() - parent->yPos.get();
	//
	//	//Normalize
	//	double magnitude = sqrt(xDirection*xDirection + yDirection*yDirection);
	//	xDirection = xDirection/magnitude;
	//	yDirection = yDirection/magnitude;
	//
	//	//Set actual velocity
	//	xVel = xDirection*speed;
	//	yVel = yDirection*speed;
}

void sim_mob::Pedestrian::updatePosition() {
	//Compute
	int newX = (int) (parent->xPos.get() + xVel * 100
			* (((double) ConfigParams::GetInstance().agentTimeStepInMilliSeconds()) / 1000));
	int newY = (int) (parent->yPos.get() + yVel * 100
			* (((double) ConfigParams::GetInstance().agentTimeStepInMilliSeconds()) / 1000));

	//Set
	parent->xPos.set(newX);
	parent->yPos.set(newY);
}

//Simple implementations for testing

void sim_mob::Pedestrian::checkForCollisions() {
	//For now, just check all agents and get the first positive collision. Very basic.
	Agent* other = nullptr;
	for (size_t i = 0; i < Agent::all_agents.size(); i++) {
		//Skip self
		other = dynamic_cast<Agent*> (Agent::all_agents[i]);
		if (!other) {
			break;
		} //Shouldn't happen; we might need to write a function for this later.

		if (other->getId() == parent->getId()) {
			other = nullptr;
			continue;
		}

		//Check.
		double dx = other->xPos.get() - parent->xPos.get();
		double dy = other->yPos.get() - parent->yPos.get();
		double distance = sqrt(dx * dx + dy * dy);
		if (distance < 2 * agentRadius) {
			break; //Collision
		}
		other = nullptr;
	}

	//Set collision vector. Overrides previous setting, if any.
	if (other) {
		//Get a heading.
		double dx = other->xPos.get() - parent->xPos.get();
		double dy = other->yPos.get() - parent->yPos.get();

		//If the two agents are directly on top of each other, set
		//  their distances to something non-crashable.
		if (dx == 0 && dy == 0) {
			dx = other->getId() - parent->getId();
			dy = parent->getId() - other->getId();
		}

		//Normalize
		double magnitude = sqrt(dx * dx + dy * dy);
		if (magnitude == 0) {
			dx = dy;
			dy = dx;
		}
		dx = dx / magnitude;
		dy = dy / magnitude;

		//Set collision vector to the inverse
		xCollisionVector = -dx * collisionForce;
		yCollisionVector = -dy * collisionForce;
	}
}

/*---------------------Other helper functions----------------------------*/


void sim_mob::Pedestrian::absToRel(double xAbs, double yAbs, double & xRel, double & yRel) {

	double xDir = cEndX - cStartX;
	double yDir = cEndY - cStartY;
	double xOffset = xAbs - cStartX;
	double yOffset = yAbs - cStartY;
	double magnitude = sqrt(xDir * xDir + yDir * yDir);
	double xDirection = xDir / magnitude;
	double yDirection = yDir / magnitude;
	xRel = xOffset * xDirection + yOffset * yDirection;
	yRel = -xOffset * yDirection + yOffset * xDirection;
}

void sim_mob::Pedestrian::relToAbs(double xRel, double yRel, double & xAbs, double & yAbs) {
	double xDir = cEndX - cStartX;
	double yDir = cEndY - cStartY;
	double magnitude = sqrt(xDir * xDir + yDir * yDir);
	double xDirection = xDir / magnitude;
	double yDirection = yDir / magnitude;
	xAbs = xRel * xDirection - yRel * yDirection + cStartX;
	yAbs = xRel * yDirection + yRel * xDirection + cStartY;
}

bool sim_mob::Pedestrian::isOnCrossing() const {
	if (atCrossing && startToCross == true)
		return true;
	else
		return false;
}
