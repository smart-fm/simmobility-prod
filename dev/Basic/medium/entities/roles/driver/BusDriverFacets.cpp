//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusDriverFacets.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/Person.hpp"
#include "entities/Vehicle.hpp"
#include "geospatial/RoadSegment.hpp"
#include "logging/Log.hpp"
#include "message/MessageBus.hpp"
#include "message/MT_Message.hpp"

using namespace sim_mob;

using std::vector;
using std::endl;

namespace {
void initSegStatsPath(const vector<const sim_mob::RoadSegment*>& rsPath,
		vector<const sim_mob::SegmentStats*>& ssPath) {
	for (vector<const sim_mob::RoadSegment*>::const_iterator it = rsPath.begin();
			it != rsPath.end(); it++) {
		const sim_mob::RoadSegment* rdSeg = *it;
		const vector<sim_mob::SegmentStats*>& statsInSegment =
				rdSeg->getParentConflux()->findSegStats(rdSeg);
		ssPath.insert(ssPath.end(), statsInSegment.begin(), statsInSegment.end());
	}
}

/**
 * converts time from milli-seconds to seconds
 */
inline double converToSeconds(uint32_t timeInMs) {
	return (timeInMs/1000.0);
}

const double BUS_LENGTH = 3 * sim_mob::PASSENGER_CAR_UNIT; // 3 times PASSENGER_CAR_UNIT
const double INFINITESIMAL_DOUBLE = 0.0001;
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
		Vehicle* newVeh = new Vehicle(Vehicle::BUS, BUS_LENGTH);
		VehicleBase* oldBus = parentBusDriver->getResource();
		safe_delete_item(oldBus);
		parentBusDriver->setResource(newVeh);
	}
}

void BusDriverMovement::frame_tick() {
	sim_mob::medium::DriverUpdateParams& params = parentBusDriver->getParams();
	if(!parentBusDriver->getResource()->isMoving()) {
		// isMoving()==false implies the bus is serving a stop
		if (parentBusDriver->waitingTimeAtbusStop > params.secondsInTick) {
			params.elapsedSeconds = params.secondsInTick;
			parentBusDriver->waitingTimeAtbusStop -= params.secondsInTick;
		}
		else {
			params.elapsedSeconds = parentBusDriver->waitingTimeAtbusStop;
			parentBusDriver->waitingTimeAtbusStop = 0.0;
			//the bus has expired its waiting time
			//send bus departure message
			const BusStop* stop = routeTracker.getNextStop();
			BusStopAgent* stopAg = BusStopAgent::findBusStopAgentByBusStop(stop);
			parentBusDriver->closeBusDoors(stopAg);
			routeTracker.updateNextStop();
		}
	}
	if(params.elapsedSeconds < params.secondsInTick) {
		DriverMovement::frame_tick();
	}
	std::stringstream logout;
	sim_mob::Person* person = getParent();
	if(person->getCurrSegStats()) {
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
	}
}

void sim_mob::medium::BusDriverMovement::frame_tick_output() {
	sim_mob::medium::DriverUpdateParams &p = parentBusDriver->getParams();
	//Skip?
	if (pathMover.isPathCompleted()
			|| ConfigManager::GetInstance().FullConfig().using_MPI
			|| ConfigManager::GetInstance().CMakeConfig().OutputDisabled()) {
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

bool sim_mob::medium::BusDriverMovement::initializePath()
{
	sim_mob::Person* person = getParent();
	if (!person) {
		Print()<<"Parent person of BusDriverMovement is NULL" << std::endl;
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

		const BusTrip* bustrip = dynamic_cast<const BusTrip*>(*(person->currTripChainItem));
		if (!bustrip) {
			Print()<< "bustrip is null"<<std::endl;
		}
		else if ((*(person->currTripChainItem))->itemType== TripChainItem::IT_BUSTRIP) {
			routeTracker = BusRouteTracker(bustrip->getBusRouteInfo());
			Print()<< "BusTrip path size = " << routeTracker.getRoadSegments().size() << std::endl;
		} else {
			Print() << "BusTrip path not initialized because it is not a bustrip, (*(person->currTripChainItem))->itemType = "
					<< (*(person->currTripChainItem))->itemType << std::endl;
		}

		//For now, empty paths aren't supported.
		if (routeTracker.getRoadSegments().empty()) {
			throw std::runtime_error("Can't initializePath(); path is empty.");
		}
		std::vector<const sim_mob::SegmentStats*> path;
		initSegStatsPath(routeTracker.getRoadSegments(), path);
		if(path.empty()) {
			return false;
		}
		pathMover.setPath(path);
		const sim_mob::SegmentStats* firstSegStat = path.front();
		person->setCurrSegStats(firstSegStat);
		person->setCurrLane(firstSegStat->laneInfinity);
		person->distanceToEndOfSegment = firstSegStat->getLength();

		routeTracker.printBusRoue();
	}

	//to indicate that the path to next activity is already planned
	person->setNextPathPlanned(true);
	return true;

}

const sim_mob::Lane* BusDriverMovement::getBestTargetLane(
		const sim_mob::SegmentStats* nextSegStats,
		const SegmentStats* nextToNextSegStats) {
	const BusStop* nextStop = routeTracker.getNextStop();
	if(nextStop && nextSegStats->hasBusStop(nextStop)) {
		return nextSegStats->getOutermostLane();
	}
	else {
		return DriverMovement::getBestTargetLane(nextSegStats, nextToNextSegStats);
	}
}

bool BusDriverMovement::moveToNextSegment(DriverUpdateParams& params) {
	const sim_mob::SegmentStats* currSegStat = pathMover.getCurrSegStats();
	const BusStop* nextStop = routeTracker.getNextStop();

	const sim_mob::RoadSegment* rs = currSegStat->getRoadSegment();
	const std::map<centimeter_t, const RoadItem*> & obstacles = rs->obstacles;
	for (std::map<centimeter_t, const RoadItem*>::const_iterator o_it =
			obstacles.begin(); o_it != obstacles.end(); o_it++) {
		RoadItem* item = const_cast<RoadItem*>(o_it->second);
		BusStop *busStop = dynamic_cast<BusStop *>(item);
		int stopPoint = o_it->first;

		if (busStop) {
			const std::vector<const sim_mob::BusStop*>* stopsVec =
					this->getParentBusDriver()->getBusStopsVector();

			std::vector<const sim_mob::BusStop*>::const_iterator itStop = std::find(
					stopsVec->begin(), stopsVec->end(), busStop);
			if (itStop != stopsVec->end()) {
				int ii=0;
			}
		}
	}

	if(nextStop && currSegStat->hasBusStop(nextStop)) {
		//send bus arrival message
		BusStopAgent* stopAg = BusStopAgent::findBusStopAgentByBusStop(nextStop);
		if(stopAg->canAccommodate(parentBusDriver->getResource()->getLengthCm())) {
			if(isQueuing) {
				removeFromQueue();
			}
			messaging::MessageBus::SendInstantaneousMessage(stopAg, BUS_ARRIVAL,
					messaging::MessageBus::MessagePtr(new BusDriverMessage(getParentBusDriver())));
			parentBusDriver->openBusDoors(stopAg);
		}
		else {
			if (isQueuing){
				moveInQueue();
			}
			else {
				addToQueue();
			}
			params.elapsedSeconds = params.secondsInTick;
			getParent()->setRemainingTimeThisTick(0.0);
		}
		return false;
	}
	else {
		return DriverMovement::moveToNextSegment(params);
	}
}

BusRouteTracker::BusRouteTracker(const BusRouteInfo& routeInfo) : BusRouteInfo(routeInfo) {
	nextStopIt = busStopList.begin();
}

const BusStop* BusRouteTracker::getNextStop() const {
	if(nextStopIt==busStopList.end()) {
		return nullptr;
	}
	return *(nextStopIt);
}

void BusRouteTracker::updateNextStop() {
	if(nextStopIt==busStopList.end()) {
		return;
	}
	nextStopIt++;
}

void BusRouteTracker::printBusRoue(){
	const vector<const sim_mob::RoadSegment*>& rsPath = getRoadSegments();
	std::cout<< "bus line is : "<< this->busRouteId << std::endl;
	std::cout<< "segments' size of bus trip is "<< rsPath.size() << std::endl;
	for (vector<const sim_mob::RoadSegment*>::const_iterator it = rsPath.begin();
			it != rsPath.end(); it++) {
		const sim_mob::RoadSegment* rdSeg = *it;
		std::cout<< rdSeg->getSegmentAimsunId() << std::endl;
	}
	const vector<const sim_mob::BusStop*>& stops = this->getBusStops();
	std::cout<< "stops' size of bus trip is "<< stops.size() << std::endl;
	for (vector<const sim_mob::BusStop*>::const_iterator it = stops.begin();
			it != stops.end(); it++) {
		std::cout<< (*it)->busstopno_ << std::endl;
	}
}
}
}
