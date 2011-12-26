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
				return BuildUpPath(it, cand.end());
			}
		}
	}

	//Error:
	throw std::runtime_error("Can't retrieve forward subpath for the given candidates.");
}

}

double Pedestrian::collisionForce = 20;
double Pedestrian::agentRadius = 0.5; //Shoulder width of a person is about 0.5 meter


sim_mob::Pedestrian::Pedestrian(Agent* parent, boost::mt19937& gen) :
	Role(parent), prevSeg(nullptr), isUsingGenPathMover(false), params(parent->getGenerator()) {
	//Check non-null parent. Perhaps references may be of use here?
	if (!parent) {
		std::cout << "Role constructed with no parent Agent." << std::endl;
		throw 1;
	}

	//Init
	sigColor = Signal::Green; //Green by default
	currentStage = ApproachingIntersection;
	startToCross = false;

	//Set default speed in the range of 1.2m/s to 1.6m/s
	speed = 1.2+(double(zero_to_five(gen)))/10;

	xVel = 0;
	yVel = 0;

	xCollisionVector = 0;
	yCollisionVector = 0;

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
	setGoal(currentStage, nullptr);
	p.skipThisFrame = true;
}


UpdateParams& sim_mob::Pedestrian::make_frame_tick_params(frame_t frameNumber, unsigned int currTimeMS)
{
	params.reset(franeNumber, currTimeMS);
	return params;
}


//Main update method
bool sim_mob::Pedestrian::frame_tick(UpdateParams& p)
{
	//Is this the first frame tick?
	if (p.skipThisFrame) {
		return;
	}

	//Check if the agent has reached the destination
	if (isDestReached()) {
		parent->setToBeRemoved();
		return;
	}

	if (isGoalReached()) {
		++currentStage;
		setGoal(currentStage, prevSeg); //Set next goal
		return;
	}

	if (currentStage == ApproachingIntersection && isUsingGenPathMover) {
		double vel = speed * 1.2 * 100 * ConfigParams::GetInstance().agentTimeStepInMilliSeconds() / 1000.0;

		prevSeg = fwdMovement.getCurrSegment();
		fwdMovement.advance(vel);
		if (!fwdMovement.isDoneWithEntireRoute() && !fwdMovement.isInIntersection() && prevSeg
				!= fwdMovement.getCurrSegment()) {
			//Move onto the outer lane (sidewalk).
			//TODO: This isn't always correct on one-way streets.
			fwdMovement.moveToNewPolyline(fwdMovement.getCurrSegment()->getLanes().size() - 1);
		}

		parent->xPos.set(fwdMovement.getPosition().x);
		parent->yPos.set(fwdMovement.getPosition().y);
	} else if (currentStage == ApproachingIntersection || currentStage == LeavingIntersection) {
		updateVelocity(0);
		updatePosition();
		LogOut("Pedestrian " <<parent->getId() <<" is walking on sidewalk" <<std::endl);
	} else if (currentStage == NavigatingIntersection) {
		//Check whether to start to cross or not
		updatePedestrianSignal();
		if (!startToCross) {
			if (sigColor == Signal::Green) //Green phase
				startToCross = true;
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
		} else {
			//Output (temp)
			LogOut("Pedestrian " <<parent->getId() <<" is waiting at the crossing" <<std::endl);
		}
	}
}


void sim_mob::Pedestrian::frame_tick_output(const UpdateParams& p)
{
	if (ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

#ifndef SIMMOB_DISABLE_MPI
	if (p.frameNumber < parent->getStartTime())
		return;

	if (this->parent->isFake) {
		LogOut("("<<"\"pedestrian\","<<p.frameNumber<<","<<parent->getId()<<","<<"{\"xPos\":\""<<parent->xPos.get()<<"\"," <<"\"yPos\":\""<<this->parent->yPos.get() <<"\"," <<"\"xVel\":\""<< this->xVel <<"\"," <<"\"yVel\":\""<< this->yVel <<"\"," <<"\"fake\":\""<<"true" <<"\",})"<<std::endl);
	} else {
		LogOut("("<<"\"pedestrian\","<<p.frameNumber<<","<<parent->getId()<<","<<"{\"xPos\":\""<<parent->xPos.get()<<"\"," <<"\"yPos\":\""<<this->parent->yPos.get() <<"\"," <<"\"xVel\":\""<< this->xVel <<"\"," <<"\"yVel\":\""<< this->yVel <<"\"," <<"\"fake\":\""<<"false" <<"\",})"<<std::endl);
	}
#else
	LogOut("("<<"\"pedestrian\","<<p.frameNumber<<","<<parent->getId()<<","<<"{\"xPos\":\""<<parent->xPos.get()<<"\"," <<"\"yPos\":\""<<this->parent->yPos.get()<<"\",})"<<std::endl);
#endif
}

/*---------------------Perception-related functions----------------------*/

void sim_mob::Pedestrian::setGoal(PedestrianStage currStage, const RoadSegment* prevSegment) {
	if (currStage == ApproachingIntersection) {
		//Retrieve the walking path to the next intersection.
		vector<WayPoint> wp_path = StreetDirectory::instance().shortestWalkingPath(parent->originNode->location,
				parent->destNode->location);

		//Create a vector of RoadSegments, which the GeneralPathMover will expect.
		const Lane* nextSideWalk = nullptr; //For the old code
		vector<const RoadSegment*> path;
		int laneID = -1; //Also save the lane id.
		for (vector<WayPoint>::iterator it = wp_path.begin(); it != wp_path.end(); it++) {
			if (it->type_ == WayPoint::SIDE_WALK) {
				//Save
				if (!nextSideWalk) {
					nextSideWalk = it->lane_;
				}

				//If we're changing Links, stop. Thus, "path" contains only the Segments needed to reach the Intersection, and no more.
				//NOTE: Later, you can send the ENTIRE shortestWalkingPath to fwdMovement and just handle "isInIntersection()"
				RoadSegment* rs = it->lane_->getRoadSegment();
				if (!path.empty() && path.back()->getLink() != rs->getLink()) {
					break;
				}

				//Add it.
				path.push_back(rs);
				laneID = it->lane_->getLaneID();
			}
		}

		//If there is exactly 1 Road Segment before the intersection, use the old movement code.
		isUsingGenPathMover = path.size() != 1;
		if (!isUsingGenPathMover) {
			//Old code
			if (nextSideWalk->getRoadSegment()->getStart() == parent->originNode) {
				goal = Point2D(nextSideWalk->getRoadSegment()->getEnd()->location);
				interPoint = Point2D(nextSideWalk->getRoadSegment()->getEnd()->location);
			} else {
				goal = Point2D(nextSideWalk->getRoadSegment()->getStart()->location);
				interPoint = Point2D(nextSideWalk->getRoadSegment()->getStart()->location);
			}
			setSidewalkParas(parent->originNode, ConfigParams::GetInstance().getNetwork().locateNode(goal, true), false);

		} else { //New code
			{
				boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
				std::cout << "Pedestrian: " << parent->getId() << " is using the NEW movement model." << std::endl;
			}

			//Sanity check
			if (path.empty() || laneID == -1) {
				throw std::runtime_error("Can't find path for Pedestrian.");
			}

			//TEMP: Currently, GeneralPathMover doesn't like walking on Segments in reverse. This is not too
			//      difficult to fix, but for now I'm just flipping the path.
			if (path.front()->getEnd() == parent->originNode) {
				path = ForceForwardSubpath(path.front(), path.front()->getLink()->getPath(true),
						path.front()->getLink()->getPath(false));
				laneID = path.front()->getLanes().size() - 1;
			}

			//Set the path
			fwdMovement.setPath(path, laneID);

			//NOTE: "goal" and "interPoint" are not really needed. We will keep them for now, but
			//      later we can just use the "isInIntersection()" check.
			goal = Point2D(path.back()->getEnd()->location);
			interPoint = Point2D(path.back()->getEnd()->location);

			//Update the Parent
			parent->xPos.set(fwdMovement.getPosition().x);
			parent->yPos.set(fwdMovement.getPosition().y);
		}
	} else if (currStage == NavigatingIntersection) {
		//Set the agent's position at the start of crossing and set the goal to the end of crossing
		setCrossingParas(prevSegment, parent->getGenerator());
	} else if (currStage == LeavingIntersection) {
		goal = Point2D(parent->destNode->location);
		setSidewalkParas(ConfigParams::GetInstance().getNetwork().locateNode(interPoint, true), parent->destNode, true);
	}

}

void sim_mob::Pedestrian::setSidewalkParas(Node* start, Node* end, bool isStartMulti) {

	unsigned int numOfLanes;
	const RoadSegment* segToWalk = nullptr;
	//bool isForward;
	//	const Lane* sideWalk;
	Point2D startPt, endPt;
	const std::vector<sim_mob::Point2D>* sidewalkPolyLine;
	if (isStartMulti) {
		const MultiNode* startNode = dynamic_cast<const MultiNode*> (start);
		if (startNode) {
			std::set<sim_mob::RoadSegment*>::const_iterator i;
			const std::set<sim_mob::RoadSegment*>& roadsegments = startNode->getRoadSegments();
			//			std::cout<<"Multinode Road segments size: "<<roadsegments.size()<<std::endl;
			for (i = roadsegments.begin(); i != roadsegments.end(); i++) {
				if ((*i)->getStart() == end && (*i)->getEnd() == start) {
					segToWalk = (*i);
					//					isForward=false;
					break;
				}
				//				else if ((*i)->getStart()==start&&(*i)->getEnd()==end){
				//					segToWalk = (*i);
				//					isForward=true;
				//					break;
				//				}
			}
			if (segToWalk) {
				numOfLanes = (unsigned int) segToWalk->getLanes().size();
				sidewalkPolyLine = &(const_cast<RoadSegment*> (segToWalk)->getLaneEdgePolyline(numOfLanes));
				//				if(isForward){
				//					startPt=sidewalkPolyLine->at(0);
				//					endPt=sidewalkPolyLine->at(sidewalkPolyLine->size()-1);
				//				}
				//				else{
				endPt = sidewalkPolyLine->at(0);
				startPt = sidewalkPolyLine->at(sidewalkPolyLine->size() - 1);
				//				}
				parent->xPos.set(startPt.getX());
				parent->yPos.set(startPt.getY());
				goalInLane = Point2D(endPt.getX(), endPt.getY());
			} else
				std::cout << "Cannot find segment from multinode!" << std::endl;
		} else
			std::cout << "Cannot cast to Multinode!" << std::endl;

	} else {
		const UniNode* startNode = dynamic_cast<const UniNode*> (start);
		if (startNode) {
			std::vector<const sim_mob::RoadSegment*>::const_iterator i;
			std::vector<sim_mob::Lane*>::const_iterator j;
			const std::vector<const sim_mob::RoadSegment*>& roadsegments = startNode->getRoadSegments();
			//			std::cout<<"Uninode Road segments size: "<<roadsegments.size()<<std::endl;
			for (i = roadsegments.begin(); i != roadsegments.end(); i++) {
				if ((*i)->getStart() == end && (*i)->getEnd() == start) {
					segToWalk = (*i);
					break;
				}
			}
			numOfLanes = (unsigned int) segToWalk->getLanes().size();
			sidewalkPolyLine = &(const_cast<RoadSegment*> (segToWalk)->getLaneEdgePolyline(numOfLanes));
			endPt = sidewalkPolyLine->at(0);
			startPt = sidewalkPolyLine->at(sidewalkPolyLine->size() - 1);
			parent->xPos.set(startPt.getX());
			parent->yPos.set(startPt.getY());
			goalInLane = Point2D(endPt.getX(), endPt.getY());
			//	const std::vector<sim_mob::Lane*>& lanes = segToWalk->getLanes();
			//	for (j=lanes.begin();j!=lanes.end();j++){
			//		if((*j)->is_pedestrian_lane()){
			//			sideWalk = (*j);
			//			break;
			//		}
			//	}
			//	sidewalkPolyLine = &(sideWalk->getPolyline());
		} else
			std::cout << "Cannot cast to Uninode!" << std::endl;
	}

	//	if(startNode){
	//		std::vector<const sim_mob::RoadSegment*>::const_iterator i;
	//		std::vector<sim_mob::Lane*>::const_iterator j;
	//		const std::vector<const sim_mob::RoadSegment*>& roadsegments=startNode->getRoadSegments();
	//		for(i=roadsegments.begin();i!=roadsegments.end();i++){
	//			if((*i)->getStart()==end&&(*i)->getEnd()==start){
	//				segToWalk = (*i);
	//				break;
	//			}
	//		}
	//		numOfLanes=(unsigned int)segToWalk->getLanes().size();
	//		sidewalkPolyLine = &(const_cast<RoadSegment*>(segToWalk)->getLaneEdgePolyline(numOfLanes));
	//	//	const std::vector<sim_mob::Lane*>& lanes = segToWalk->getLanes();
	//	//	for (j=lanes.begin();j!=lanes.end();j++){
	//	//		if((*j)->is_pedestrian_lane()){
	//	//			sideWalk = (*j);
	//	//			break;
	//	//		}
	//	//	}
	//	//	sidewalkPolyLine = &(sideWalk->getPolyline());
	//		endPt=sidewalkPolyLine->at(0);
	//		startPt=sidewalkPolyLine->at(sidewalkPolyLine->size()-1);
	//		parent->xPos.set(startPt.getX());
	//		parent->yPos.set(startPt.getY());
	//		goalInLane = Point2D(endPt.getX(),endPt.getY());
	//	}
	//	else{
	//		std::cout<<"Cannot cast to Uninode!"<<std::endl;
	////		const MultiNode* startNode=dynamic_cast<const MultiNode*>(start);
	////		std::set<sim_mob::RoadSegment*>::const_iterator i;
	////		const std::set<sim_mob::RoadSegment*>& roadsegments=startNode->getRoadSegments();
	////		for(i=roadsegments.begin();i!=roadsegments.end();i++){
	////			if((*i)->getStart()==end&&(*i)->getEnd()==start){
	////				segToWalk = (*i);
	////				break;
	////			}
	////		}
	////		numOfLanes=(unsigned int)segToWalk->getLanes().size();
	////		sidewalkPolyLine = &(const_cast<RoadSegment*>(segToWalk)->getLaneEdgePolyline(numOfLanes));
	////	//	const std::vector<sim_mob::Lane*>& lanes = segToWalk->getLanes();
	////	//	for (j=lanes.begin();j!=lanes.end();j++){
	////	//		if((*j)->is_pedestrian_lane()){
	////	//			sideWalk = (*j);
	////	//			break;
	////	//		}
	////	//	}
	////	//	sidewalkPolyLine = &(sideWalk->getPolyline());
	////		startPt=sidewalkPolyLine->at(0);
	////		endPt=sidewalkPolyLine->at(sidewalkPolyLine->size()-1);
	////		parent->xPos.set(startPt.getX());
	////		parent->yPos.set(startPt.getY());
	////		goalInLane = Point2D(endPt.getX(),endPt.getY());
	//	}

}

bool sim_mob::Pedestrian::isDestReached() {
	if (currentStage == LeavingIntersection) {
		double dX = ((double) abs(goalInLane.getX() - parent->xPos.get())) / 100;
		double dY = ((double) abs(goalInLane.getY() - parent->yPos.get())) / 100;
		double dis = sqrt(dX * dX + dY * dY);
		return dis < agentRadius * 4;
	}
	return false;
}

bool sim_mob::Pedestrian::isGoalReached() {
	if (currentStage == ApproachingIntersection && isUsingGenPathMover) {
		return fwdMovement.isDoneWithEntireRoute();
	}

	double dX = ((double) abs(goalInLane.getX() - parent->xPos.get())) / 100;
	double dY = ((double) abs(goalInLane.getY() - parent->yPos.get())) / 100;
	double dis = sqrt(dX * dX + dY * dY);
	return dis < agentRadius * 4;
}

//bool sim_mob::Pedestrian::reachStartOfCrossing()
//{
//
//
//	return false;
//
////	int lowerRightCrossingY = ConfigParams::GetInstance().crossings["lowerright"].getY();
////
////	if(parent->yPos.get()<=lowerRightCrossingY){
////		double dist = lowerRightCrossingY - parent->yPos.get();
////		if(dist<speed*1)
////			return true;
////		else
////			return false;
////	}
////	else
////		return false;
//}

void sim_mob::Pedestrian::updatePedestrianSignal() {
	if (!trafficSignal)
		std::cout << "Traffic signal not found!" << std::endl;
	else {
		if (currCrossing) {
			sigColor = trafficSignal->getPedestrianLight(*currCrossing);
			//			std::cout<<"Debug: signal color "<<sigColor<<std::endl;
		} else
			std::cout << "Current crossing not found!" << std::endl;
	}

	//	Agent* a = nullptr;
	//	Signal* s = nullptr;
	//	for (size_t i=0; i<Agent::all_agents.size(); i++) {
	//		//Skip self
	//		a = Agent::all_agents[i];
	//		if (a->getId()==parent->getId()) {
	//			a = nullptr;
	//			continue;
	//		}
	//
	//		if (dynamic_cast<Signal*>(a)) {
	//		   s = dynamic_cast<Signal*>(a);
	//		   currPhase=1;//s->get_Pedestrian_Light(0);
	//			//It's a signal
	//		}
	//		s = nullptr;
	//		a = nullptr;
	//	}
	//	s = nullptr;
	//	a = nullptr;

	//	currPhase = sig.get_Pedestrian_Light(0);
	//	if(phaseCounter==60){ //1 minute period for switching phases (testing only)
	//		phaseCounter=0;
	//		if(currPhase==0)
	//			currPhase = 1;
	//		else
	//			currPhase = 0;
	//	}
	//	else
	//		phaseCounter++;
	//	currPhase=1 ;

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

void sim_mob::Pedestrian::setCrossingParas(const RoadSegment* prevSegment, boost::mt19937& gen) {
	double xRel, yRel;
	double xAbs, yAbs;
	double width, length, tmp;

	//Get traffic signal
	const Node* node = ConfigParams::GetInstance().getNetwork().locateNode(goal, true);
	if (node)
		trafficSignal = StreetDirectory::instance().signalAt(*node);
	else
		trafficSignal = nullptr;

	std::set<sim_mob::RoadSegment*>::const_iterator i;
	const Node* currNodeA = ConfigParams::GetInstance().getNetwork().locateNode(goal, true);
	if (!currNodeA) {
		std::stringstream msg;
		msg << "Could not retrieve node at position: " << goal.getX() << "," << goal.getY();
		msg << "\n   for Pedestrian traveling from: " << parent->originNode->originalDB_ID.getLogItem() << "  to: "
				<< parent->destNode->originalDB_ID.getLogItem();
		throw std::runtime_error(msg.str().c_str());
	}
	const MultiNode* currNode = dynamic_cast<const MultiNode*> (currNodeA);
	if (!currNode) {
		std::stringstream msg;
		msg << "Node is not a multiNode: " << currNodeA->originalDB_ID.getLogItem();
		msg << "\n   for Pedestrian traveling from: " << parent->originNode->originalDB_ID.getLogItem() << "  to: "
				<< parent->destNode->originalDB_ID.getLogItem();
		throw std::runtime_error(msg.str().c_str());
	}

	//Determine the Segment that we need to cross.
	const RoadSegment* segToCross = nullptr;
	const std::set<sim_mob::RoadSegment*>& roadsegments = currNode->getRoadSegments();
	for (i = roadsegments.begin(); i != roadsegments.end(); i++) {
		if ((*i)->getStart() != parent->originNode && (*i)->getEnd() != parent->originNode && (*i)->getStart()
				!= parent->destNode && (*i)->getEnd() != parent->destNode) {
			cStartX = (double) goal.getX();
			cStartY = (double) goal.getY();
			cEndX = (double) parent->destNode->location.getX();
			cEndY = (double) parent->destNode->location.getY();
			if ((*i)->getStart() == currNode) {
				absToRel((*i)->getEnd()->location.getX(), (*i)->getEnd()->location.getY(), xRel, yRel);
				if (yRel < 0) {
					segToCross = (*i);
					break;
				}
			} else {
				absToRel((*i)->getStart()->location.getX(), (*i)->getStart()->location.getY(), xRel, yRel);
				if (yRel < 0) {
					segToCross = (*i);
					break;
				}
			}
		}
	}

	//TEMP: We can fold this in later
	if (!segToCross) {
		//Create a vector in the direction that we were moving.
		DynamicVector dir(prevSegment->getStart()->location.getX(), prevSegment->getStart()->location.getY(),
				prevSegment->getEnd()->location.getX(), prevSegment->getEnd()->location.getY());
		if (prevSegment->getStart() == currNode) {
			dir.translateVect().flipMirror();
		}
		dir.scaleVectTo(dir.getMagnitude() * 2); //Just to make sure we have something to work with.

		//Now, check all outgoing segments and stop when we find one which starts "left" of the current one.
		//TODO: This is not the best way to check this. Eventually we should use "getCircular()" to retrieve
		//      an entire path around the intersection. But it should be solid enough for now.
		for (i = roadsegments.begin(); i != roadsegments.end(); i++) {
			const Node* check = (*i)->getStart() == currNode ? (*i)->getEnd() : (*i)->getStart();
			if (PointIsLeftOfVector(dir, check->location.getX(), check->location.getY())) {
				segToCross = *i;
				break;
			}
		}
	}

	if (!segToCross) {
		throw std::runtime_error("Can't find segment to cross.");
	}

	if (segToCross->getStart() == currNode) {
		currCrossing = dynamic_cast<const Crossing*> (segToCross->nextObstacle(0, true).item);
	} else {
		currCrossing = dynamic_cast<const Crossing*> (segToCross->nextObstacle(segToCross->length, true).item);
	}

	if (currCrossing) {
		Point2D far1 = currCrossing->farLine.first;
		Point2D far2 = currCrossing->farLine.second;
		Point2D near1 = currCrossing->nearLine.first;
		Point2D near2 = currCrossing->nearLine.second;

		//Determine the direction of two points
		if ((near1.getY() > near2.getY() && goal.getY() > parent->originNode->location.getY()) || (near1.getY()
				< near2.getY() && goal.getY() < parent->originNode->location.getY())) {
			cStartX = (double) near2.getX();
			cStartY = (double) near2.getY();
			cEndX = (double) near1.getX();
			cEndY = (double) near1.getY();
			absToRel(cEndX, cEndY, length, tmp);
			absToRel((double) far2.getX(), (double) far2.getY(), tmp, width);
		} else {
			cStartX = (double) near1.getX();
			cStartY = (double) near1.getY();
			cEndX = (double) near2.getX();
			cEndY = (double) near2.getY();
			absToRel(cEndX, cEndY, length, tmp);
			absToRel((double) far1.getX(), (double) far1.getY(), tmp, width);
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
		goal = Point2D((int)xAbs,(int)yAbs);
		goalInLane = Point2D((int)xAbs,(int)yAbs);

//		double slope1, slope2;
//		slope1 = (double)(far2.getY()-far1.getY())/(far2.getX()-far1.getX());

	}
	else
		std::cout<<"Crossing not found!"<<std::endl;

}


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
	if (currentStage == NavigatingIntersection && startToCross == true)
		return true;
	else
		return false;
}

#ifndef SIMMOB_DISABLE_MPI
void sim_mob::Pedestrian::package(PackageUtils& packageUtil) {
	//Part 1

	packageUtil.packageBasicData(speed);
	packageUtil.packageBasicData(xVel);
	packageUtil.packageBasicData(yVel);
	packageUtil.packagePoint2D(goal);
	packageUtil.packagePoint2D(goalInLane);
	packageUtil.packageBasicData(currentStage);

//	if (trafficSignal) {
//		bool hasSignal = true;
//		packageUtil.packageBasicData(hasSignal);
//		packageUtil.packagePoint2D(trafficSignal->getNode().location);
//	} else {
//		bool hasSignal = false;
//		packageUtil.packageBasicData(hasSignal);
//	}

//	if (currCrossing) {
//		bool hasCrossing = true;
//		packageUtil.packageBasicData(hasCrossing);
//		packageUtil.packageCrossing(currCrossing);
//	} else {
//		bool hasCrossing = false;
//		packageUtil.packageBasicData(hasCrossing);
//	}


	//Part 2
	packageUtil.packageBasicData(sigColor);
	packageUtil.packageBasicData(curCrossingID);
	packageUtil.packageBasicData(startToCross);
	packageUtil.packageBasicData(cStartX);
	packageUtil.packageBasicData(cStartY);
	packageUtil.packageBasicData(cEndX);
	packageUtil.packageBasicData(cEndY);
	packageUtil.packageBasicData(firstTimeUpdate);

	packageUtil.packagePoint2D(interPoint);

	packageUtil.packageBasicData(xCollisionVector);
	packageUtil.packageBasicData(yCollisionVector);
	packageUtil.packageGeneralPathMover(&fwdMovement);

	if (prevSeg) {
		bool hasSegment = true;
		packageUtil.packageBasicData(hasSegment);
		packageUtil.packageRoadSegment(prevSeg);
	} else {
		bool hasSegment = false;
		packageUtil.packageBasicData(hasSegment);
	}

	packageUtil.packageBasicData(isUsingGenPathMover);
}

void sim_mob::Pedestrian::unpackage(UnPackageUtils& unpackageUtil) {
	//Part 1
	speed = unpackageUtil.unpackageBasicData<double> ();
	xVel = unpackageUtil.unpackageBasicData<double> ();
	yVel = unpackageUtil.unpackageBasicData<double> ();

	goal = *(unpackageUtil.unpackagePoint2D());
	goalInLane = *(unpackageUtil.unpackagePoint2D());
	int value = unpackageUtil.unpackageBasicData<int> ();
	currentStage = PedestrianStage(value);

//	bool hasSignal = unpackageUtil.unpackageBasicData<bool> ();
//	if (hasSignal) {
//		Point2D* signal_location = unpackageUtil.unpackagePoint2D();
//		trafficSignal = sim_mob::getSignalBasedOnNode(signal_location);
//	}

//	bool hasCrossing = unpackageUtil.unpackageBasicData<bool> ();
//	if (hasCrossing) {
//		currCrossing = unpackageUtil.unpackageCrossing();
//	}

	//Part 2
	sigColor = unpackageUtil.unpackageBasicData<int> ();
	curCrossingID = unpackageUtil.unpackageBasicData<int> ();
	startToCross = unpackageUtil.unpackageBasicData<bool> ();
	cStartX = unpackageUtil.unpackageBasicData<double> ();
	cStartY = unpackageUtil.unpackageBasicData<double> ();
	cEndX = unpackageUtil.unpackageBasicData<double> ();
	cEndY = unpackageUtil.unpackageBasicData<double> ();
	firstTimeUpdate = unpackageUtil.unpackageBasicData<bool> ();

	interPoint = *(unpackageUtil.unpackagePoint2D());

	xCollisionVector = unpackageUtil.unpackageBasicData<double> ();
	yCollisionVector = unpackageUtil.unpackageBasicData<double> ();
	unpackageUtil.unpackageGeneralPathMover(&fwdMovement);
	//fwdMovement = *(unpackageUtil.unpackageGeneralPathMover());
	bool hasSegment = unpackageUtil.unpackageBasicData<bool> ();
	if (hasSegment) {
		prevSeg = unpackageUtil.unpackageRoadSegment();
	}

	isUsingGenPathMover = unpackageUtil.unpackageBasicData<bool> ();
}

void sim_mob::Pedestrian::packageProxy(PackageUtils& packageUtil) {
	//Part 1
	//std::cout << "1-1-6-1" << std::endl;
	//Part 1
	packageUtil.packageBasicData(speed);
	packageUtil.packageBasicData(xVel);
	packageUtil.packageBasicData(yVel);
	packageUtil.packagePoint2D(goal);
	packageUtil.packagePoint2D(goalInLane);
	packageUtil.packageBasicData(currentStage);

//	if(trafficSignal)
//	{
//		bool hasSignal = true;
//		packageUtil.packageBasicData(hasSignal);
//		packageUtil.packagePoint2D(trafficSignal->getNode().location);
//	}
//	else
//	{
//		bool hasSignal = false;
//		packageUtil.packageBasicData(hasSignal);
//	}
//
//	if (currCrossing) {
//		bool hasCrossing = true;
//		packageUtil.packageBasicData(hasCrossing);
//		packageUtil.packageCrossing(currCrossing);
//	} else {
//		bool hasCrossing = false;
//		packageUtil.packageBasicData(hasCrossing);
//	}

	//Part 2
	packageUtil.packageBasicData(sigColor);
	packageUtil.packageBasicData(curCrossingID);
	packageUtil.packageBasicData(startToCross);
	packageUtil.packageBasicData(cStartX);
	packageUtil.packageBasicData(cStartY);
	packageUtil.packageBasicData(cEndX);
	packageUtil.packageBasicData(cEndY);
	packageUtil.packageBasicData(firstTimeUpdate);
	packageUtil.packagePoint2D(interPoint);

	packageUtil.packageBasicData(xCollisionVector);
	packageUtil.packageBasicData(yCollisionVector);
	packageUtil.packageGeneralPathMover(&fwdMovement);
//
//	if(prevSeg)
//	{
//		bool hasSegment = true;
//		packageUtil.packageBasicData(hasSegment);
//		packageUtil.packageRoadSegment(prevSeg);
//	}
//	else
//	{
//		bool hasSegment = false;
//		packageUtil.packageBasicData(hasSegment);
//	}

	packageUtil.packageBasicData(isUsingGenPathMover);

}

void sim_mob::Pedestrian::unpackageProxy(UnPackageUtils& unpackageUtil) {
	//Part 1
	//std::cout << "1-1-6-2" << std::endl;

	speed = unpackageUtil.unpackageBasicData<double> ();
	xVel = unpackageUtil.unpackageBasicData<double> ();
	yVel = unpackageUtil.unpackageBasicData<double> ();

	goal = *(unpackageUtil.unpackagePoint2D());
	goalInLane = *(unpackageUtil.unpackagePoint2D());
	int value = unpackageUtil.unpackageBasicData<int> ();
	currentStage = PedestrianStage(value);
//
//	bool hasSignal = unpackageUtil.unpackageBasicData<bool> ();
//	if (hasSignal) {
//		Point2D* signal_location = unpackageUtil.unpackagePoint2D();
//		trafficSignal = sim_mob::getSignalBasedOnNode(signal_location);
//	}
//
//	bool hasCrossing = unpackageUtil.unpackageBasicData<bool> ();
//	if (hasCrossing) {
//		currCrossing = unpackageUtil.unpackageCrossing();
//	}

	//Part 2
	sigColor = unpackageUtil.unpackageBasicData<int> ();
	curCrossingID = unpackageUtil.unpackageBasicData<int> ();
	startToCross = unpackageUtil.unpackageBasicData<bool> ();
	cStartX = unpackageUtil.unpackageBasicData<double> ();
	cStartY = unpackageUtil.unpackageBasicData<double> ();
	cEndX = unpackageUtil.unpackageBasicData<double> ();
	cEndY = unpackageUtil.unpackageBasicData<double> ();
	firstTimeUpdate = unpackageUtil.unpackageBasicData<bool> ();

	interPoint = *(unpackageUtil.unpackagePoint2D());

	xCollisionVector = unpackageUtil.unpackageBasicData<double> ();
	yCollisionVector = unpackageUtil.unpackageBasicData<double> ();
	unpackageUtil.unpackageGeneralPathMover(&fwdMovement);
//	//fwdMovement = *(unpackageUtil.unpackageGeneralPathMover());
//	bool hasSegment = unpackageUtil.unpackageBasicData<bool> ();
//	if (hasSegment) {
//		prevSeg = unpackageUtil.unpackageRoadSegment();
//	}

	isUsingGenPathMover = unpackageUtil.unpackageBasicData<bool> ();
}
#endif
