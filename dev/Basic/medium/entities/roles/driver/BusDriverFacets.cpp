/*
 * DriverFacets.cpp
 *
 *  Created on: Apr 1, 2013
 *      Author: melani
 */

#include "BusDriverFacets.hpp"
#include "entities/Person.hpp"
#include "logging/Log.hpp"

using namespace sim_mob;

using std::vector;
using std::endl;

namespace sim_mob {
namespace medium {

sim_mob::medium::BusDriverBehavior::BusDriverBehavior(sim_mob::Person* parentAgent):
	DriverBehavior(parentAgent), parentBusDriver(nullptr) {}

sim_mob::medium::BusDriverBehavior::~BusDriverBehavior() {}

void sim_mob::medium::BusDriverBehavior::frame_init(UpdateParams& p) {
	throw std::runtime_error("BusDriverBehavior::frame_init is not implemented yet");
}

void sim_mob::medium::BusDriverBehavior::frame_tick(UpdateParams& p) {
	throw std::runtime_error("BusDriverBehavior::frame_tick is not implemented yet");
}

void sim_mob::medium::BusDriverBehavior::frame_tick_output(const UpdateParams& p) {
	throw std::runtime_error("BusDriverBehavior::frame_tick_output is not implemented yet");
}

void sim_mob::medium::BusDriverBehavior::frame_tick_output_mpi(timeslice now) {
	throw std::runtime_error("BusDriverBehavior::frame_tick_output_mpi is not implemented yet");
}

sim_mob::medium::BusDriverMovement::BusDriverMovement(sim_mob::Person* parentAgent):
	DriverMovement(parentAgent), parentBusDriver(nullptr) {}

sim_mob::medium::BusDriverMovement::~BusDriverMovement() {}

}

void sim_mob::medium::BusDriverMovement::frame_init(UpdateParams& p) {

	Vehicle* newVeh = initializePath(true);
	if (newVeh) {
		safe_delete_item(vehicle);
		vehicle = newVeh;
		parentBusDriver->setResource(newVeh);
	}
}

void sim_mob::medium::BusDriverMovement::frame_tick(UpdateParams& p) {
	DriverMovement::frame_tick(p);
}

void sim_mob::medium::BusDriverMovement::frame_tick_output(const UpdateParams& p) {
	//Skip?
	if (vehicle->isDone() || ConfigParams::GetInstance().is_run_on_many_computers || ConfigParams::GetInstance().OutputDisabled()) {
		return;
	}

	std::stringstream logout;
	logout << "(\"BusDriver\""
			<<","<<parentAgent->getId()
			<<","<<p.now.frame()
			<<",{"
			<<"\"RoadSegment\":\""<< (parentAgent->getCurrSegment()->getSegmentID())
			<<"\",\"Lane\":\""<<(parentAgent->getCurrLane()->getLaneID())
			<<"\",\"UpNode\":\""<<(parentAgent->getCurrSegment()->getStart()->getID())
			<<"\",\"DistanceToEndSeg\":\""<<parentAgent->distanceToEndOfSegment;
	if (this->parentAgent->isQueuing) {
			logout << "\",\"queuing\":\"" << "true";
	} else {
			logout << "\",\"queuing\":\"" << "false";
	}
	logout << "\"})" << std::endl;
	Print()<<logout.str();
	LogOut(logout.str());
}

void sim_mob::medium::BusDriverMovement::frame_tick_output_mpi(timeslice now) {
	throw std::runtime_error("BusDriverMovement::frame_tick_output_mpi is not implemented yet");
}

void sim_mob::medium::BusDriverMovement::flowIntoNextLinkIfPossible(UpdateParams& p) {
	Print()<<"BusDriver_movement flowIntoNextLinkIfPossible called"<<std::endl;
	DriverMovement::flowIntoNextLinkIfPossible(p);
}

sim_mob::Vehicle* sim_mob::medium::BusDriverMovement::initializePath(bool allocateVehicle)
{
	Vehicle* res = nullptr;
	if ( !parentAgent) {
		Print()<<"Person of BusDriverMovement is null" << std::endl;
		return nullptr;
	}

	//Only initialize if the next path has not been planned for yet.
	if(!parentAgent->getNextPathPlanned()){
		//Save local copies of the parent's origin/destination nodes.
		if( parentAgent->originNode.type_ != WayPoint::INVALID){
			parentBusDriver->origin.node = parentAgent->originNode.node_;
			parentBusDriver->origin.point = parentBusDriver->origin.node->location;
		}
		if( parentAgent->destNode.type_ != WayPoint::INVALID ){
			parentBusDriver->goal.node = parentAgent->destNode.node_;
			parentBusDriver->goal.point = parentBusDriver->goal.node->location;
		}

		//Retrieve the path from origin to destination and save all RoadSegments in this path.
		vector<WayPoint> path;

		vector<const RoadSegment*> pathRoadSeg;

		const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(parentAgent->currTripChainItem));
		if (!bustrip)
			Print()<< "bustrip is null"<<std::endl;
		if (bustrip&& (*(parentAgent->currTripChainItem))->itemType== TripChainItem::IT_BUSTRIP) {
			pathRoadSeg = bustrip->getBusRouteInfo().getRoadSegments();
			Print()<< "BusTrip path size = " << pathRoadSeg.size() << std::endl;
			std::vector<const RoadSegment*>::iterator itor;
			for(itor=pathRoadSeg.begin(); itor!=pathRoadSeg.end(); itor++){
				path.push_back(WayPoint(*itor));
			}
		} else {
			if ((*(parentAgent->currTripChainItem))->itemType== TripChainItem::IT_TRIP)
				Print()<< TripChainItem::IT_TRIP << " IT_TRIP\n";
			if ((*(parentAgent->currTripChainItem))->itemType== TripChainItem::IT_ACTIVITY)
				Print()<< "IT_ACTIVITY\n";
			if ((*(parentAgent->currTripChainItem))->itemType== TripChainItem::IT_BUSTRIP)
				Print()<< "IT_BUSTRIP\n";
			std::cout<< "BusTrip path not initialized coz it is not a bustrip, (*(parentAgent->currTripChainItem))->itemType = "<< (*(parentAgent->currTripChainItem))->itemType<< std::endl;
		}

		//For now, empty paths aren't supported.
		if (path.empty()) {
			throw std::runtime_error("Can't initializePath(); path is empty.");
		}

		//TODO: Start in lane 0?
		int startlaneID = 0;

		// Bus should be at least 1200 to be displayed on Visualizer
		const double length = 400;
		const double width = 200;

		//A non-null vehicle means we are moving.
		if (allocateVehicle) {
			res = new Vehicle(path, startlaneID, length, width);
		}
	}

	//to indicate that the path to next activity is already planned
	parentAgent->setNextPathPlanned(true);
	return res;

}
}
