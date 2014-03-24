//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusDriverFacets.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Person.hpp"
#include "geospatial/RoadSegment.hpp"
#include "logging/Log.hpp"

using namespace sim_mob;

using std::vector;
using std::endl;

namespace {
void initSegStatsPath(vector<const sim_mob::RoadSegment*>& rsPath,
		vector<const sim_mob::SegmentStats*>& ssPath) {
	for (vector<const sim_mob::RoadSegment*>::iterator it = rsPath.begin();
			it != rsPath.end(); it++) {
		const sim_mob::RoadSegment* rdSeg = *it;
		const sim_mob::SegmentStats* segStats =
				rdSeg->getParentConflux()->findSegStats(rdSeg);
		ssPath.push_back(segStats);
	}
}
}

namespace sim_mob {
namespace medium {

sim_mob::medium::BusDriverBehavior::BusDriverBehavior(sim_mob::Person* parentAgent):
	DriverBehavior(parentAgent), parentBusDriver(nullptr) {}

sim_mob::medium::BusDriverBehavior::~BusDriverBehavior() {}

void sim_mob::medium::BusDriverBehavior::frame_init() {
	throw std::runtime_error("BusDriverBehavior::frame_init is not implemented yet");
}

void sim_mob::medium::BusDriverBehavior::frame_tick() {
	throw std::runtime_error("BusDriverBehavior::frame_tick is not implemented yet");
}

void sim_mob::medium::BusDriverBehavior::frame_tick_output() {
	throw std::runtime_error("BusDriverBehavior::frame_tick_output is not implemented yet");
}


sim_mob::medium::BusDriverMovement::BusDriverMovement(sim_mob::Person* parentAgent):
	DriverMovement(parentAgent), parentBusDriver(nullptr) {}

sim_mob::medium::BusDriverMovement::~BusDriverMovement() {}

void sim_mob::medium::BusDriverMovement::frame_init() {

	Vehicle* newVeh = new Vehicle();
	initializePath();
	if (newVeh) {
		Vehicle* oldBus = parentBusDriver->getResource();
		safe_delete_item(oldBus);
		parentBusDriver->setResource(newVeh);
	}
}

void sim_mob::medium::BusDriverMovement::frame_tick() {
	DriverMovement::frame_tick();
}

void sim_mob::medium::BusDriverMovement::frame_tick_output() {
	parentBusDriver->getParams();
	DriverUpdateParams &p = parentBusDriver->getParams();
	//Skip?
	if (isPathCompleted() || ConfigManager::GetInstance().FullConfig().using_MPI || ConfigManager::GetInstance().CMakeConfig().OutputDisabled()) {
		return;
	}

	std::stringstream logout;
	logout << "(\"BusDriver\""
			<<","<<getParent()->getId()
			<<","<<parentBusDriver->getParams().now.frame()
			<<",{"
			<<"\"RoadSegment\":\""<< (getParent()->getCurrSegment()->getSegmentID())
			<<"\",\"Lane\":\""<<(getParent()->getCurrLane()->getLaneID())
			<<"\",\"UpNode\":\""<<(getParent()->getCurrSegment()->getStart()->getID())
			<<"\",\"DistanceToEndSeg\":\""<<getParent()->distanceToEndOfSegment;
	if (this->getParent()->isQueuing) {
			logout << "\",\"queuing\":\"" << "true";
	} else {
			logout << "\",\"queuing\":\"" << "false";
	}
	logout << "\"})" << std::endl;
	Print()<<logout.str();
	LogOut(logout.str());
}


void sim_mob::medium::BusDriverMovement::flowIntoNextLinkIfPossible(UpdateParams& p) {
	Print()<<"BusDriver_movement flowIntoNextLinkIfPossible called"<<std::endl;
	DriverMovement::flowIntoNextLinkIfPossible(p);
}

void sim_mob::medium::BusDriverMovement::initializePath()
{
	if ( !getParent()) {
		Print()<<"Person of BusDriverMovement is null" << std::endl;
		return;
	}

	//Only initialize if the next path has not been planned for yet.
	if(!getParent()->getNextPathPlanned()){
		//Save local copies of the parent's origin/destination nodes.
		if( getParent()->originNode.type_ != WayPoint::INVALID){
			parentBusDriver->origin.node = getParent()->originNode.node_;
			parentBusDriver->origin.point = parentBusDriver->origin.node->location;
		}
		if( getParent()->destNode.type_ != WayPoint::INVALID ){
			parentBusDriver->goal.node = getParent()->destNode.node_;
			parentBusDriver->goal.point = parentBusDriver->goal.node->location;
		}

		vector<const RoadSegment*> pathRoadSeg;

		const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(getParent()->currTripChainItem));
		if (!bustrip) {
			Print()<< "bustrip is null"<<std::endl;
		}
		else if ((*(getParent()->currTripChainItem))->itemType== TripChainItem::IT_BUSTRIP) {
			pathRoadSeg = bustrip->getBusRouteInfo().getRoadSegments();
			Print()<< "BusTrip path size = " << pathRoadSeg.size() << std::endl;
		} else {
			if ((*(getParent()->currTripChainItem))->itemType== TripChainItem::IT_TRIP) {
				Print()<< "IT_TRIP\n";
			}
			if ((*(getParent()->currTripChainItem))->itemType== TripChainItem::IT_ACTIVITY) {
				Print()<< "IT_ACTIVITY\n";
			}
			Print() << "BusTrip path not initialized coz it is not a bustrip, (*(getParent()->currTripChainItem))->itemType = "<< (*(getParent()->currTripChainItem))->itemType<< std::endl;
		}

		//For now, empty paths aren't supported.
		if (pathRoadSeg.empty()) {
			throw std::runtime_error("Can't initializePath(); path is empty.");
		}
		initSegStatsPath(pathRoadSeg, path);
		currSegStatIt = path.begin();
	}

	//to indicate that the path to next activity is already planned
	getParent()->setNextPathPlanned(true);
	return;

}

}
}
