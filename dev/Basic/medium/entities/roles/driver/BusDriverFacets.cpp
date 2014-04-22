//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusDriverFacets.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Person.hpp"
#include "entities/Vehicle.hpp"
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
		const vector<sim_mob::SegmentStats*>& statsInSegment =
				rdSeg->getParentConflux()->findSegStats(rdSeg);
		ssPath.insert(ssPath.end(), statsInSegment.begin(), statsInSegment.end());
	}
}

const double PASSENGER_CAR_UNIT = 400.0; //cm; 4 m.
const double BUS_LENGTH = 1200.0; // 3 times PASSENGER_CAR_UNIT
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
	bool pathInitialized = initializePath();
	if (pathInitialized) {
		Vehicle* newVeh = new Vehicle(Vehicle::BUS, BUS_LENGTH, BUS_LENGTH/PASSENGER_CAR_UNIT);
		Vehicle* oldBus = parentBusDriver->getVehicle();
		safe_delete_item(oldBus);
		parentBusDriver->setVehicle(newVeh);
	}
}

void sim_mob::medium::BusDriverMovement::frame_tick() {
	DriverMovement::frame_tick();
}

void sim_mob::medium::BusDriverMovement::frame_tick_output() {
	DriverUpdateParams &p = parentBusDriver->getParams();
	//Skip?
	if (pathMover.isPathCompleted() || ConfigManager::GetInstance().FullConfig().using_MPI || ConfigManager::GetInstance().CMakeConfig().OutputDisabled()) {
		return;
	}

	std::stringstream logout;
	sim_mob::Person* person = getParent();
	logout << "(\"BusDriver\""
			<<","<<person->getId()
			<<","<<parentBusDriver->getParams().now.frame()
			<<",{"
			<<"\"RoadSegment\":\""<< (person->getCurrSegStats()->getRoadSegment()->getSegmentID())
			<<"\",\"Lane\":\""<<(person->getCurrLane()->getLaneID())
			<<"\",\"UpNode\":\""<<(person->getCurrSegStats()->getRoadSegment()->getStart()->getID())
			<<"\",\"DistanceToEndSeg\":\""<<person->distanceToEndOfSegment;
	if (person->isQueuing) {
			logout << "\",\"queuing\":\"" << "true";
	} else {
			logout << "\",\"queuing\":\"" << "false";
	}
	logout << "\"})" << std::endl;
	Print()<<logout.str();
	LogOut(logout.str());
}


void sim_mob::medium::BusDriverMovement::flowIntoNextLinkIfPossible(DriverUpdateParams& p) {
	Print()<<"BusDriver_movement flowIntoNextLinkIfPossible called"<<std::endl;
	DriverMovement::flowIntoNextLinkIfPossible(p);
}

bool sim_mob::medium::BusDriverMovement::initializePath()
{
	sim_mob::Person* person = getParent();
	if (!person) {
		Print()<<"Person of BusDriverMovement is null" << std::endl;
		return false;
	}

	//Only initialize if the next path has not been planned for yet.
	if(!person->getNextPathPlanned()){
		//Save local copies of the parent's origin/destination nodes.
		if( person->originNode.type_ != WayPoint::INVALID){
			parentBusDriver->origin.node = person->originNode.node_;
			parentBusDriver->origin.point = parentBusDriver->origin.node->location;
		}
		if( person->destNode.type_ != WayPoint::INVALID ){
			parentBusDriver->goal.node = person->destNode.node_;
			parentBusDriver->goal.point = parentBusDriver->goal.node->location;
		}

		vector<const RoadSegment*> pathRoadSeg;

		const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(person->currTripChainItem));
		if (!bustrip) {
			Print()<< "bustrip is null"<<std::endl;
		}
		else if ((*(person->currTripChainItem))->itemType== TripChainItem::IT_BUSTRIP) {
			pathRoadSeg = bustrip->getBusRouteInfo().getRoadSegments();
			Print()<< "BusTrip path size = " << pathRoadSeg.size() << std::endl;
		} else {
			if ((*(person->currTripChainItem))->itemType== TripChainItem::IT_TRIP) {
				Print()<< "IT_TRIP\n";
			}
			if ((*(person->currTripChainItem))->itemType== TripChainItem::IT_ACTIVITY) {
				Print()<< "IT_ACTIVITY\n";
			}
			Print() << "BusTrip path not initialized coz it is not a bustrip, (*(person->currTripChainItem))->itemType = "<< (*(person->currTripChainItem))->itemType<< std::endl;
		}

		//For now, empty paths aren't supported.
		if (pathRoadSeg.empty()) {
			throw std::runtime_error("Can't initializePath(); path is empty.");
		}
		std::vector<const sim_mob::SegmentStats*> path;
		initSegStatsPath(pathRoadSeg, path);
		if(path.empty()) {
			return false;
		}
		pathMover.setPath(path);
		const sim_mob::SegmentStats* firstSegStat = path.front();
		person->setCurrSegStats(firstSegStat);
		person->setCurrLane(firstSegStat->laneInfinity);
		person->distanceToEndOfSegment = firstSegStat->getLength();
	}

	//to indicate that the path to next activity is already planned
	person->setNextPathPlanned(true);
	return true;

}

}
}
