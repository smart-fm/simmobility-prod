/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Pedestrian2.cpp
 *
 * \author Max
 */

#include "Pedestrian2.hpp"
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

#ifdef SIMMOB_NEW_SIGNAL
#include "entities/signal/Signal.hpp"
#else
#include "entities/Signal.hpp"
#endif

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

double Pedestrian2::collisionForce = 20;
double Pedestrian2::agentRadius = 0.5; //Shoulder width of a person is about 0.5 meter


sim_mob::Pedestrian2::Pedestrian2(Agent* parent) : Role(parent),
	trafficSignal(nullptr), currCrossing(nullptr),
	isUsingGenPathMover(true), params(parent->getGenerator()) {
	//Check non-null parent. Perhaps references may be of use here?

	//Init
#ifdef SIMMOB_NEW_SIGNAL
	sigColor = sim_mob::Green; //Green by default
#else
	sigColor = Signal::Green; //Green by default
#endif

	//Set default speed in the range of 1.2m/s to 1.6m/s
	speed = 1.2;

	xVel = 0;
	yVel = 0;

	xCollisionVector = 0;
	yCollisionVector = 0;
}

//Note that a destructor is not technically needed, but I want to enforce the idea
//  of overriding virtual destructors if they exist.
sim_mob::Pedestrian2::~Pedestrian2() {
}

vector<BufferedBase*> sim_mob::Pedestrian2::getSubscriptionParams() {
	vector<BufferedBase*> res;
	return res;
}



void sim_mob::Pedestrian2::frame_init(UpdateParams& p)
{
	setSubPath();
	dynamic_cast<PedestrianUpdateParams2&>(p).skipThisFrame = true;
}
Role* sim_mob::Pedestrian2::clone(Person* parent) const
{
	return new Pedestrian2(parent);
}

UpdateParams& sim_mob::Pedestrian2::make_frame_tick_params(frame_t frameNumber, unsigned int currTimeMS)
{
	params.reset(frameNumber, currTimeMS);
	return params;
}


//Main update method
void sim_mob::Pedestrian2::frame_tick(UpdateParams& p)
{
	PedestrianUpdateParams2& p2 = dynamic_cast<PedestrianUpdateParams2&>(p);

	//Is this the first frame tick?
	if (p2.skipThisFrame) {
		return;
	}

	double vel = 0;

	int signalGreen = 3;
#ifdef SIMMOB_NEW_SIGNAL
	signalGreen = sim_mob::Green; //Green by default
#else
	signalGreen = Signal::Green; //Green by default
#endif
	if(pedMovement.isAtCrossing()){
		//Check whether to start to cross or not
		updatePedestrianSignal();

		if (sigColor == signalGreen) //Green phase
			vel = speed * 2.0 * 100 * ConfigParams::GetInstance().agentTimeStepInMilliSeconds() / 1000.0;
		else
			vel = 0;
	}
	else {
		if (!pedMovement.isDoneWithEntireRoute())
			vel = speed * 1.2 * 100 * ConfigParams::GetInstance().agentTimeStepInMilliSeconds() / 1000.0;
	}
		pedMovement.advance(vel);

		parent->xPos.set(pedMovement.getPosition().x);
		parent->yPos.set(pedMovement.getPosition().y);

}

void sim_mob::Pedestrian2::frame_tick_med(UpdateParams& p){
	/*to be implemented by supply team to move the pedestrian after each time tick
	 *
	 */
}

void sim_mob::Pedestrian2::frame_tick_output(const UpdateParams& p)
{
	if (dynamic_cast<const PedestrianUpdateParams2&>(p).skipThisFrame) {
		return;
	}

	if (ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

#ifndef SIMMOB_DISABLE_OUTPUT
	LogOut("("<<"\"pedestrian\","<<p.frameNumber<<","<<parent->getId()<<","<<"{\"xPos\":\""<<parent->xPos.get()<<"\"," <<"\"yPos\":\""<<this->parent->yPos.get()<<"\",})"<<std::endl);
#endif
}

void sim_mob::Pedestrian2::frame_tick_output_mpi(frame_t frameNumber)
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

void sim_mob::Pedestrian2::setSubPath() {

	vector<WayPoint> wp_path = StreetDirectory::instance().shortestWalkingPath(parent->originNode->location, parent->destNode->location);

	//Used to debug pedestrian walking paths.
	/*LogOut("Pedestrian requested path from: " <<parent->originNode->originalDB_ID.getLogItem() <<" => " <<parent->destNode->originalDB_ID.getLogItem() <<"  {" <<std::endl);
	for (vector<WayPoint>::iterator it = wp_path.begin(); it != wp_path.end(); it++) {
		if (it->type_ == WayPoint::SIDE_WALK) {
			const Node* start = !it->directionReverse ? it->lane_->getRoadSegment()->getStart() : it->lane_->getRoadSegment()->getEnd();
			const Node* end = !it->directionReverse ? it->lane_->getRoadSegment()->getEnd() : it->lane_->getRoadSegment()->getStart();
			LogOut("  Side-walk: " <<start->originalDB_ID.getLogItem() <<" => " <<end->originalDB_ID.getLogItem() <<"   (Reversed: " <<it->directionReverse <<")" <<std::endl);
		} else if (it->type_ == WayPoint::ROAD_SEGMENT) {
			LogOut("  Road Segment: (not supported)" <<std::endl);
		} else if (it->type_ == WayPoint::BUS_STOP) {
			LogOut("  Bus Stop: (not supported)"<<std::endl);
		} else if (it->type_ == WayPoint::CROSSING){
			LogOut("  Crossing at Node: " <<StreetDirectory::instance().GetCrossingNode(it->crossing_)->originalDB_ID.getLogItem() <<std::endl);
		} else if (it->type_ == WayPoint::NODE) {
			LogOut("  Node: " <<it->node_->originalDB_ID.getLogItem() <<std::endl);
		} else if (it->type_ == WayPoint::INVALID) {
			LogOut("  <Invalid>"<<std::endl);
		} else {
			LogOut("  Unknown type."<<std::endl);
		}
	}
	LogOut("}" <<std::endl);*/

	pedMovement.setPath(wp_path);

}

void sim_mob::Pedestrian2::updatePedestrianSignal() {
	const MultiNode* node = StreetDirectory::instance().GetCrossingNode(pedMovement.getCurrentWaypoint()->crossing_);
	if (!node) {
		throw std::runtime_error("Coulding find Pedestrian Sginal for crossing.");
	}

/*	const RoadSegment *rs = pedMovement.getCurrentWaypoint()->crossing_->getRoadSegment();

	// find intersection's multi node,compare the distance to start ,end nodes of segment,any other way?
	Point2D currentSegmentStartLocation(rs->getStart()->location);
	Point2D currentSegmentEndLocation(rs->getEnd()->location);
	DynamicVector pedDistanceToStartLocation(pedMovement.getPosition().x, pedMovement.getPosition().y,
			currentSegmentStartLocation.getX(), currentSegmentStartLocation.getY());
	DynamicVector pedDistanceToEndLocation(pedMovement.getPosition().x, pedMovement.getPosition().y,
			currentSegmentEndLocation.getX(), currentSegmentEndLocation.getY());

	const Node* node = NULL;
	Point2D location;
	if(pedDistanceToStartLocation.getMagnitude() >= pedDistanceToEndLocation.getMagnitude())
		location = rs->getEnd()->location;
	else
		location =rs->getStart()->location;

	// we have multi node ,so get signal
	if(rs)
		node = ConfigParams::GetInstance().getNetwork().locateNode(location, true);*/
	if (node)
		trafficSignal = StreetDirectory::instance().signalAt(*node);
	else
		trafficSignal = nullptr;

	if (!trafficSignal)
		std::cout << "Traffic signal not found!" << std::endl;
	else {
		if (pedMovement.getCurrentWaypoint()->crossing_) {
			sigColor = trafficSignal->getPedestrianLight(*pedMovement.getCurrentWaypoint()->crossing_);
			//			std::cout<<"Debug: signal color "<<sigColor<<std::endl;
		} else
			std::cout << "Current crossing not found!" << std::endl;
	}
}

/*---------------------Decision-related functions------------------------*/

bool sim_mob::Pedestrian2::checkGapAcceptance() {

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

//Simple implementations for testing

void sim_mob::Pedestrian2::checkForCollisions() {
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


