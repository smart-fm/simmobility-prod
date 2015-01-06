//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * BusDriverFacets.cpp
 *
 *  Created on: May 16th, 2013
 *      Author: Yao Jin
 */

#include "BusDriverFacets.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/Person.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/roles/driver/models/LaneChangeModel.hpp"
#include "entities/roles/waitBusActivityRole/WaitBusActivityRole.hpp"
#include "logging/Log.hpp"
#include "path/PathSetManager.hpp"

namespace {
	// default bus length cm to be displayed on visualizer
	const double DEFAULT_BUS_LENGTH_CM = 1200;
	// default car length cm to be displayed on visualizer
	const double DEFAULT_CAR_LENGTH_CM = 400;
	// default vehicle width cm to be displayed on visualizer
	const double DEFAULT_VEHICLE_WIDTH_CM = 200;
}

namespace sim_mob {
BusDriverBehavior::BusDriverBehavior(sim_mob::Person* parentAgent):
	DriverBehavior(parentAgent), parentBusDriver(nullptr) {}

BusDriverBehavior::~BusDriverBehavior() {}

void BusDriverBehavior::frame_init() {
	throw std::runtime_error("BusDriverBehavior::frame_init is not implemented yet");
}

void BusDriverBehavior::frame_tick() {
	throw std::runtime_error("BusDriverBehavior::frame_tick is not implemented yet");
}

void BusDriverBehavior::frame_tick_output() {
	throw std::runtime_error("BusDriverBehavior::frame_tick_output is not implemented yet");
}


sim_mob::BusDriverMovement::BusDriverMovement(sim_mob::Person* parentAgent):
	DriverMovement(parentAgent), parentBusDriver(nullptr), lastTickDistanceToBusStop(-1), demoPassengerIncrease(false),
	dwellTimeRecord(0), firstBusStop(true), lastBusStop(false),
	noPassengersBoarding(0), noPassengersAlighting(0), allowBoardingAlightingFlag(false), firstBoardingAlightingMS(0),
	lastBoardingAlightingMS(0), boardingmsOffset(0), alightingmsOffset(0),
	busStopHoldingTimeSec(2), busStopWaitBoardingAlightingSec(2), waitAtStopMS(-1), busStopWaitTime(2)
{
}

sim_mob::BusDriverMovement::~BusDriverMovement()
{

}

Vehicle* sim_mob::BusDriverMovement::initializePath_bus(bool allocateVehicle) {
	Vehicle* res = nullptr;

	//Only initialize if the next path has not been planned for yet.
	if(getParent()) {
		if (!getParent()->getNextPathPlanned()) {
			std::vector<const RoadSegment*> path;
			int vehicle_id = 0;
			int laneID = -1;
			const BusTrip* bustrip =
					dynamic_cast<const BusTrip*>(*(getParent()->currTripChainItem));
			if (!bustrip)
			{
				WarnOut("bustrip is null");
			}
			if (bustrip
					&& (*(getParent()->currTripChainItem))->itemType
							== TripChainItem::IT_BUSTRIP)
			{
				path = bustrip->getBusRouteInfo().getRoadSegments();
				std::cout << "BusTrip path size = " << path.size() << std::endl;
				vehicle_id = bustrip->getVehicleID();
				if (path.size() > 0)
				{
					laneID = path.at(0)->getLanes().size() - 2;
				}
			}
			else
			{
				if ((*(getParent()->currTripChainItem))->itemType
						== TripChainItem::IT_TRIP)
					std::cout << TripChainItem::IT_TRIP << " IT_TRIP\n";
				if ((*(getParent()->currTripChainItem))->itemType
						== TripChainItem::IT_ACTIVITY)
					std::cout << "IT_ACTIVITY\n";
				if ((*(getParent()->currTripChainItem))->itemType
						== TripChainItem::IT_BUSTRIP)
					std::cout << "IT_BUSTRIP\n";
				Print() << "BusTrip path not initialized coz it is not a bustrip, (*(getParent()->currTripChainItem))->itemType = "
						<< (*(getParent()->currTripChainItem))->itemType
						<< std::endl;
			}

			//TODO: Start in lane 0?
			int startlaneID = 1;

			if (this->getParentBusDriver() && laneID != -1) {
				startlaneID = laneID; //need to check if lane valid
			}

			// Bus should be at least DEFAULT_BUS_LENGTH_CM to be displayed on Visualizer
			const double length = this->getParentBusDriver() ? DEFAULT_BUS_LENGTH_CM : DEFAULT_CAR_LENGTH_CM;
			const double width = DEFAULT_VEHICLE_WIDTH_CM;

			//A non-null vehicle means we are moving.
			if (allocateVehicle) {
				res = new Vehicle(VehicleBase::BUS, vehicle_id, length, width);
				fwdDriverMovement.setPath(path, startlaneID);
			}
		}

		//to indicate that the path to next activity is already planned
		getParent()->setNextPathPlanned(true);
	}

	return res;
}

void sim_mob::BusDriverMovement::frame_init() {
	//TODO: "initializePath()" in Driver mixes initialization of the path and
	//      creation of the Vehicle (e.g., its width/height). These are both
	//      very different for Cars and Buses, but until we un-tangle the code
	//      we'll need to rely on hacks like this.
	Vehicle* newVeh = nullptr;

	if (getParent()) {
		if (getParent()->getAgentSrc() == "BusController") {
			newVeh = initializePath_bus(true); // no need any node information
		} else {
			newVeh = initializePath(true); // previous node to node calculation
		}
	}

	//Save the path, create a vehicle.
	if (newVeh) {
		//Use this sample vehicle to build our Bus, then delete the old vehicle.
		BusRoute nullRoute; //Buses don't use the route at the moment.

		TripChainItem* tci = *(getParent()->currTripChainItem);
		BusTrip* bustrip_change =dynamic_cast<BusTrip*>(tci);
		if (!bustrip_change) {
			throw std::runtime_error("BusDriver created without an appropriate BusTrip item.");
		}

		Vehicle *bus = new Bus(nullRoute, newVeh,bustrip_change->getBusline()->getBusLineID());
		parentBusDriver->setVehicle(bus);
		delete newVeh;

		//This code is used by Driver to set a few properties of the Vehicle/Bus.
		if (!(fwdDriverMovement.isPathSet())) {
			throw std::runtime_error(
					"Vehicle could not be created for bus driver; no route!");
		}

		//Set the bus's origin and set of stops.
		setOrigin(parentBusDriver->getParams());
		if (getParent()) {
			if (getParent()->getAgentSrc() == "BusController") {
				const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(getParent()->currTripChainItem));
				if (bustrip && bustrip->itemType == TripChainItem::IT_BUSTRIP) {
					busStops = bustrip->getBusRouteInfo().getBusStops();
					if (busStops.empty()) {
						Warn() << "Error: No BusStops assigned from BusTrips!!! "<< std::endl;
						// This case can be true, so use the BusStops found by Path instead
						busStops = findBusStopInPath(fwdDriverMovement.fullPath);
					}
				}
			} else {
				busStops = findBusStopInPath(fwdDriverMovement.fullPath);
			}
		}
		//Unique to BusDrivers: reset your route
		waitAtStopMS = 0.0;

		//Set initial speed of bus to 0
		parentBusDriver->getParams().initSpeed = 0;
	}

	// put busStops to StopPointPool
	for(int i=0;i<busStops.size();++i){
		const BusStop* bs = busStops[i];
		std::string segAimsunId = bs->getParentSegment()->originalDB_ID.getLogItem();
		std::string segid = Utils::getNumberFromAimsunId(segAimsunId);
		double dd = sim_mob::BusStop::EstimateStopPoint(bs->xPos, bs->yPos, bs->getParentSegment()) /100.0;
		double dis = bs->distance;
		double fd = dd;
		double segl = bs->getParentSegment()->getLengthOfSegment() /100.0;

		if(fd > (segl -10) ){
			fd = segl -10;
		}
		if(fd<0){
			fd = segl/2;
		}

		Print()<<"id: "<<segid<<" dis:"<<dis<<" dd:"<<dd<<" len:"<<bs->getParentSegment()->getLengthOfSegment() /100.0<<" fd:"<<fd<<std::endl;
		double dwelltime = 10;
		StopPoint sp(segid,fd,dwelltime);
		parentBusDriver->getParams().insertStopPoint(sp);
	}

}

std::vector<const BusStop*> sim_mob::BusDriverMovement::findBusStopInPath(const std::vector<const RoadSegment*>& path) const {
	//NOTE: Use typedefs instead of defines.
	typedef std::vector<const BusStop*> BusStopVector;
	BusStopVector res;

	std::vector<const RoadSegment*>::const_iterator it;
	for (it = path.begin(); it != path.end(); ++it) {
		// get obstacles in road segment
		const RoadSegment* rs = (*it);
		const std::map<centimeter_t, const RoadItem*> & obstacles =rs->obstacles;

		//Check each of these.
		std::map<centimeter_t, const RoadItem*>::const_iterator ob_it;
		for (ob_it = obstacles.begin(); ob_it != obstacles.end(); ++ob_it) {
			RoadItem* ri = const_cast<RoadItem*>(ob_it->second);
			BusStop *bs = dynamic_cast<BusStop*>(ri);
			if (bs) {
				bs->distance = ob_it->first /100.0;
				res.push_back(bs);
			}
		}
	}
	return res;
}

double sim_mob::BusDriverMovement::busAccelerating(DriverUpdateParams& p) {
	//Retrieve a new acceleration value.
	double newFwdAcc = 0;

	//Convert back to m/s
	//TODO: Is this always m/s? We should rename the variable then...
	p.currSpeed = parentBusDriver->getVehicle()->getVelocity() / 100;

	//Call our model
	newFwdAcc = cfModel->makeAcceleratingDecision(p, targetSpeed, p.maxLaneSpeed);

	return newFwdAcc;
}

bool sim_mob::BusDriverMovement::isBusFarawayBusStop() {
	bool res = false;
	double distance = distanceToNextBusStop();
	if (distance < 0 || distance > 50)
		res = true;

	return res;
}

bool sim_mob::BusDriverMovement::isBusApproachingBusStop() {
	double distance = distanceToNextBusStop();
	if (distance >= 10 && distance <= 50) {
		if (lastTickDistanceToBusStop < 0) {
			return true;
		} else if (lastTickDistanceToBusStop > distance) {
			return true;
		}
	}
	return false;
}

bool sim_mob::BusDriverMovement::isBusArriveBusStop() {
	double distance = distanceToNextBusStop();
	return (distance > 0 && distance < 10);
}

bool sim_mob::BusDriverMovement::isBusGngtoBreakDown() {
	double distance = distanceToNextBusStop();
	return (distance > 10 && distance < 14);
}

bool sim_mob::BusDriverMovement::isBusLeavingBusStop() {
	double distance = distanceToNextBusStop();
	if (distance >= 10 && distance < 50) {
		if (distance < 0) {
			lastTickDistanceToBusStop = distance;
			return true;
		} else if (lastTickDistanceToBusStop < distance) {
			lastTickDistanceToBusStop = distance;
			return true;
		}
	}

	lastTickDistanceToBusStop = distance;
	return false;
}

double sim_mob::BusDriverMovement::distanceToNextBusStop() {
	double distanceToCurrentSegmentBusStop = getDistanceToBusStopOfSegment(
			fwdDriverMovement.getCurrSegment());
	if(distanceToCurrentSegmentBusStop >= 0) {
		return distanceToCurrentSegmentBusStop;
	}
	double distanceToNextSegmentBusStop = -1;
	if (hasNextSegment(true))
		distanceToNextSegmentBusStop = getDistanceToBusStopOfSegment(
				fwdDriverMovement.getNextSegment(true));

	if (distanceToCurrentSegmentBusStop >= 0
			&& distanceToNextSegmentBusStop >= 0) {
		return ((distanceToCurrentSegmentBusStop <= distanceToNextSegmentBusStop) ?
				distanceToCurrentSegmentBusStop : distanceToNextSegmentBusStop);
	} else if (distanceToCurrentSegmentBusStop > 0) {
		return distanceToCurrentSegmentBusStop;
	}

	return distanceToNextSegmentBusStop;
}


double sim_mob::BusDriverMovement::dwellTimeCalculation(int A, int B, int delta_bay, int delta_full,int Pfront, int no_of_passengers)
{
	return sim_mob::DriverMovement::dwellTimeCalculation(A, B, delta_bay, delta_full, Pfront, no_of_passengers);
}

double sim_mob::BusDriverMovement::getDistanceToBusStopOfSegment(const RoadSegment* rs) {

	double distance = -100;
	double currentX = parentBusDriver->getPositionX();
	double currentY = parentBusDriver->getPositionY();
	const std::map<centimeter_t, const RoadItem*> & obstacles = rs->obstacles;
	for (std::map<centimeter_t, const RoadItem*>::const_iterator o_it =
			obstacles.begin(); o_it != obstacles.end(); ++o_it) {
		RoadItem* ri = const_cast<RoadItem*>(o_it->second);
		BusStop *bs = dynamic_cast<BusStop *>(ri);
		int stopPoint = o_it->first;

		if (bs) {
			// check bs
			int i = 0;
			//int busstop_sequence_no = 0;
			bool isFound = false;

			for (i = 0; i < busStops.size(); ++i) {
				if (bs->getBusstopno_() == busStops[i]->getBusstopno_()) {
					isFound = true;
					parentBusDriver->busstop_sequence_no.set(i);
					parentBusDriver->lastVisited_BusStop.set(busStops[i]);
					break;
				}
			}
			if (isFound) {

				parentBusDriver->xpos_approachingbusstop = bs->xPos;
				parentBusDriver->ypos_approachingbusstop = bs->yPos;
				if (parentBusDriver->busstop_sequence_no.get() == (busStops.size() - 1)) // check whether it is the last bus stop in the busstop list
				{
					lastBusStop = true;
				}
				if (rs == fwdDriverMovement.getCurrSegment()) {

					if (stopPoint < 0) {
						throw std::runtime_error(
								"BusDriver offset in obstacles list should never be <0");
					}

					if (stopPoint >= 0) {
						DynamicVector BusDistfromStart(parentBusDriver->getPositionX(),
								parentBusDriver->getPositionY(),
								rs->getStart()->location.getX(),
								rs->getStart()->location.getY());

						// Buses not stopping near the busstop at few places.
						// one easy way to fix it
						double actualDistance = sim_mob::BusStop::EstimateStopPoint(bs->xPos, bs->yPos, rs);

						distance = actualDistance
								- BusDistfromStart.getMagnitude();

						break;
					}
				} else {
					DynamicVector busToSegmentStartDistance(currentX, currentY,
							rs->getStart()->location.getX(),
							rs->getStart()->location.getY());
					distance = fwdDriverMovement.getCurrentSegmentLengthCM()
							- fwdDriverMovement.getCurrDistAlongRoadSegmentCM() + stopPoint;

				}
			} // end of if isFound
		}
	}

	return distance / 100.0;
}

//Main update functionality
void sim_mob::BusDriverMovement::frame_tick() {
	//NOTE: If this is all that is doen, we can simply delete this function and
	//      let its parent handle it automatically. ~Seth
	DriverMovement::frame_tick();
}

void sim_mob::BusDriverMovement::frame_tick_output() {
	DriverUpdateParams &p = parentBusDriver->getParams();
	if (this->getParentDriver()->IsVehicleInLoadingQueue() || fwdDriverMovement.isDoneWithEntireRoute()) {
		return;
	}

	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
		double baseAngle =
				fwdDriverMovement.isInIntersection() ?
						intModel->getCurrentAngle() : getAngle();

		//MPI-specific output.
		std::stringstream addLine;
		if (ConfigManager::GetInstance().FullConfig().using_MPI) {
			addLine <<"\",\"fake\":\"" <<(this->getParent()->isFake?"true":"false");
		}

		Bus* bus = dynamic_cast<Bus*>(parentBusDriver->getVehicle());
		int passengerCount = 0;
		if (bus) {
			passengerCount = bus->getPassengerCount();
		}

		LogOut(
			"(\"BusDriver\""
			<<","<<p.now.frame()
			<<","<<getParent()->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(parentBusDriver->getPositionX())
			<<"\",\"yPos\":\""<<static_cast<int>(parentBusDriver->getPositionY())
			<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
			<<"\",\"length\":\""<<static_cast<int>(bus->getLengthCm())
			<<"\",\"width\":\""<<static_cast<int>(bus->getWidthCm())
			<<"\",\"passengers\":\""<<passengerCount
			<<"\",\"real_ArrivalTime\":\""<<(bus?parentBusDriver->real_ArrivalTime.get():0)
			<<"\",\"DwellTime_ijk\":\""<<(bus?parentBusDriver->DwellTime_ijk.get():0)
			<<"\",\"buslineID\":\""<<(bus?bus->getBusLineID():0)
			<<addLine.str()
			<<"\",\"info\":\""<<p.debugInfo
			<<"\"})"<<std::endl);
	}
}


void sim_mob::BusDriverMovement::AlightingPassengers(Bus* bus)//for alighting passengers
{
	if (bus->getPassengerCount() > 0) {
		int i = 0;
		std::vector<Person*>::iterator itr = bus->passengers_inside_bus.begin();
		while (itr != bus->passengers_inside_bus.end())
		{
			//Retrieve only Passenger agents inside the bus
			Person* p = bus->passengers_inside_bus[i];
			Passenger* passenger =p ? dynamic_cast<Passenger*>(p->getRole()) : nullptr;
			if (!passenger)
				continue;
			PassengerMovement* passenger_movement = dynamic_cast<PassengerMovement*> (passenger->Movement());
			if(passenger_movement) {
				if (passenger_movement->isBusBoarded() == true) //alighting is only for a passenger who has boarded the bus
				{
					if (passenger_movement->PassengerAlightBus(this->getParentBusDriver()) == true) //check if passenger wants to alight the bus
					{
						itr = (bus->passengers_inside_bus).erase(bus->passengers_inside_bus.begin() + i);
						noPassengersAlighting++;
					}
					else
					{
						++itr;
						i++;
					}
				}
			}

		}
	}
}


void sim_mob::BusDriverMovement::BoardingPassengers_Choice(Bus* bus)
{
	const Agent* parentAgent = (parentBusDriver?parentBusDriver->getParent():nullptr);
 	std::vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(Point2D((parentBusDriver->lastVisited_BusStop.get()->xPos - 3500),(parentBusDriver->lastVisited_BusStop.get()->yPos - 3500)),Point2D((parentBusDriver->lastVisited_BusStop.get()->xPos + 3500),(parentBusDriver->lastVisited_BusStop.get()->yPos + 3500)), parentAgent); //  nearbyAgents(Point2D(lastVisited_BusStop.get()->xPos, lastVisited_BusStop.get()->yPos), *params.currLane,3500,3500);
 	for (std::vector<const Agent*>::iterator it = nearby_agents.begin();it != nearby_agents.end(); ++it)
 	{
 		//Retrieve only Passenger agents.
 		const Person* person = dynamic_cast<const Person *>(*it);
 		Person* p = const_cast<Person *>(person);
 		Passenger* passenger = p ? dynamic_cast<Passenger*>(p->getRole()) : nullptr;
 		if (!passenger)
 		  continue;
		PassengerMovement* passenger_movement = dynamic_cast<PassengerMovement*> (passenger->Movement());
		if(passenger_movement) {
	 		if ((abs((passenger_movement->getXYPosition().getX())- (parentBusDriver->xpos_approachingbusstop)) <= 2)and (abs((passenger_movement->getXYPosition().getY() / 1000)- (parentBusDriver->ypos_approachingbusstop / 1000)) <= 2))
	 		 {
	 	       if (passenger_movement->isAtBusStop() == true) //if passenger agent is waiting at the approaching bus stop
	 			{
	 	    	   if(passenger_movement->PassengerBoardBus_Choice(this->getParentBusDriver())==true)
	 	    		   noPassengersBoarding++;
	 			}

	 		}
		}
 	}
}

void sim_mob::BusDriverMovement::IndividualBoardingAlighting_New(Bus* bus)
{
	uint32_t currMS = parentBusDriver->getParams().now.ms();

	// determine the alighting and boarding frames, if allowBoardingAlightingFlag is true, no need to dertermined
	if((!allowBoardingAlightingFlag) && (currMS % busStopUpdateFrequencyMS != 0)) {// skip the case when (curr_ms % 5000 == 0), one time determine the boarding and alighting MSs
		DetermineBoardingAlightingMS(bus);// keep determining the boarding and alighting MSs, once determined, allowBoardingAlightingFlag is true
	}
	if(allowBoardingAlightingFlag) {
		StartBoardingAlighting(bus);// can support boarding and alighting at the same time
	}
}

void sim_mob::BusDriverMovement::DetermineBoardingAlightingMS(Bus* bus)
{
	uint32_t currMS = parentBusDriver->getParams().now.ms();
	int i = 0;
	int j = 0;
	int boardingNum = 0;
	int alightingNum = 0;
	uint32_t boardingMS = currMS;// set this to curr_frame, later add and advance
	uint32_t alightingMS = currMS;// set this to curr_frame, later add and advance
	uint32_t accumulatedBoardingMS = currMS;// set this to curr_frame, later add and advance
	uint32_t accumulatedAlightedMS = currMS;// set this to curr_frame, later add and advance
	uint32_t lastBoardingMS = 0;
	uint32_t lastAlightingMS = 0;
	const uint32_t baseGranMS = ConfigManager::GetInstance().FullConfig().baseGranMS();// baseGran MS perFrame
	const Busline* busline = nullptr;
	BusStopAgent* busstopAgent = BusStopAgent::findBusStopAgentByBusStopNo(parentBusDriver->lastVisited_BusStop.get()->getBusstopno_());
	std::vector<sim_mob::WaitBusActivityRole*>& boarding_waitBusActivities = busstopAgent->getBoarding_WaitBusActivities();// get the boarding queue of persons for all Buslines
	const BusTrip* bustrip = dynamic_cast<const BusTrip*>(*(getParent()->currTripChainItem));
	if (bustrip && bustrip->itemType == TripChainItem::IT_BUSTRIP) {
		busline = bustrip->getBusline();
		if(!busline) {
			return;
		}
	}

	firstBoardingAlightingMS = currMS;
	if(parentBusDriver->busstop_sequence_no.get() == 0) // first busstop, only boarding
	{
		// determine the boarding frame for each possible persons
		for(i = 0; i < boarding_waitBusActivities.size(); i++) {
			WaitBusActivityRoleMovement* waitbusactivityrolemovement = dynamic_cast<WaitBusActivityRoleMovement*> (boarding_waitBusActivities[i]->Movement());
			if(waitbusactivityrolemovement->getBuslineID() == busline->getBusLineID() && (!waitbusactivityrolemovement->isTagged)) // calculate how many persons want to board this bus based on the BusLineID
			{
				waitbusactivityrolemovement->isTagged = true;
				BoardingNumPos[boardingNum] = i;// record the previous pos in the boardingWaitBusActivities
				boardingNum++;
			}
		}
		if(boardingNum > bus->getBusCapacity()) {// abnormal, only some are boarding
			boardingNum = bus->getBusCapacity();// cut
		}
		for(j = 0; j < boardingNum; j++) {// extract person characteristics and calculate the corresponding boarding frames
			Person* p = boarding_waitBusActivities[BoardingNumPos[j]]->getParent();
			if(p) {
				boardingMS += (p->getBoardingCharacteristics()*1000);// multiplied by 1000 to transfer to ms
				accumulatedBoardingMS += (p->getBoardingCharacteristics()*1000);// multiplied by 1000 to transfer to ms
				if(boardingMS % busStopUpdateFrequencyMS == 0) {// deal with a special case every 5000ms, add one frame tick in case
					boardingMS += baseGranMS;// delay one frame tick, doesnt matter(100ms)
					accumulatedBoardingMS += baseGranMS;// advance also
				}
				WaitBusActivityRoleMovement* waitbusactivityrolemovement = dynamic_cast<WaitBusActivityRoleMovement*> (boarding_waitBusActivities[BoardingNumPos[j]]->Movement());
				waitbusactivityrolemovement->boardingMS = boardingMS;// set Boarding_MS for this WaitBusActivity
				waitbusactivityrolemovement->busDriver = parentBusDriver;// set busDriver for this WaitBusActivity
				boardingMSs.push_back(boardingMS);
				virtualBoardingPersons.push_back(p);// push this person in the virtual queue
			}
		}
		if(!boardingMSs.empty()) {
			lastBoardingMS = boardingMSs.back();// store the last_boarding_ms
		}

	} else if(parentBusDriver->busstop_sequence_no.get() == (busStops.size() - 1)) { // last busstop, only alighting, all alighting without any choice
		for(i = 0; i < (bus->passengers_inside_bus).size(); i++) {
			AlightingNumPos[i] = i;
			Person* p = dynamic_cast<Person*>((bus->passengers_inside_bus)[i]);
			if(p) {
				Passenger* passenger = dynamic_cast<Passenger*>(p->getRole());
				PassengerMovement* passenger_movement = dynamic_cast<PassengerMovement*> (passenger->Movement());
				if(passenger) {
					alightingMS += (p->getAlightingCharacteristics()*1000);// multiplied by 1000 to transfer to ms
					accumulatedAlightedMS += (p->getAlightingCharacteristics()*1000);// multiplied by 1000 to transfer to ms
					if(alightingMS % busStopUpdateFrequencyMS == 0) {// deal with a special case every 50 frames, add one frame tick in case
						alightingMS += baseGranMS;;// delay one frame tick, doesnt matter(100ms)
						accumulatedAlightedMS += baseGranMS;// advance also
					}
					if(passenger_movement) {
						passenger_movement->alightingMS = alightingMS;// set Alighting_MS for this Passenger
						passenger_movement->busTripRunNum = bustrip->getBusTripRun_SequenceNum();
						passenger_movement->buslineId = busline->getBusLineID();
					}
					alightingMSs.push_back(alightingMS);
				}
			}
		}
		if(!alightingMSs.empty()) {
			lastAlightingMS = alightingMSs.back();// store the last_alighting_ms
		}

	} else { // normal busstop, both boarding and alighting
		// determine the alighting frame for each possible persons
		for(i = 0; i < (bus->passengers_inside_bus).size(); i++) {
			Person* p = dynamic_cast<Person*>((bus->passengers_inside_bus)[i]);
			if(p) {
				Passenger* passenger = dynamic_cast<Passenger*>(p->getRole());
				if(passenger) {
					PassengerMovement* passenger_movement = dynamic_cast<PassengerMovement*> (passenger->Movement());
					if(passenger_movement) {
						if(passenger_movement->getDestBusStop() == parentBusDriver->lastVisited_BusStop.get()) // it reached the DestBusStop and it want to alight
						{
							AlightingNumPos[alightingNum] = i;
							alightingNum++;
						}
					}
				}
			}
		}
		for(j = 0; j < alightingNum; j++) {// extract person characteristics and calculate the corresponding alighting frames
			Person* p = dynamic_cast<Person*>((bus->passengers_inside_bus)[AlightingNumPos[j]]);
			if(p) {
				Passenger* passenger = dynamic_cast<Passenger*>(p->getRole());
				PassengerMovement* passenger_movement = dynamic_cast<PassengerMovement*> (passenger->Movement());
				if(passenger) {
					alightingMS += (p->getAlightingCharacteristics()*1000);// multiplied by 1000 to transfer to ms
					accumulatedAlightedMS += (p->getAlightingCharacteristics()*1000);// advance also
					if(alightingMS % busStopUpdateFrequencyMS == 0) {// deal with a special case every 5000ms, add one frame tick in case
						alightingMS += baseGranMS;// delay one frame tick, doesnt matter(100ms)
						accumulatedAlightedMS += baseGranMS;// advance also
					}
					if(passenger_movement) {
						passenger_movement->alightingMS = alightingMS;// set Alighting_MS for this Passenger
						passenger_movement->busTripRunNum = bustrip->getBusTripRun_SequenceNum();
						passenger_movement->buslineId = busline->getBusLineID();
					}
					alightingMSs.push_back(alightingMS);
				}
			}
		}
		if(!alightingMSs.empty()) {
			lastAlightingMS = alightingMSs.back();// store the last_alighting_ms
		}


		// determine the boarding frame for each possible persons
		for(i = 0; i < boarding_waitBusActivities.size(); i++) {
			WaitBusActivityRoleMovement* waitbusactivityrolemovement = dynamic_cast<WaitBusActivityRoleMovement*> (boarding_waitBusActivities[i]->Movement());
			if(waitbusactivityrolemovement->getBuslineID() == busline->getBusLineID() && (!waitbusactivityrolemovement->isTagged)) // calculate how many persons want to board this bus based on the BusLineID
			{
				waitbusactivityrolemovement->isTagged = true;
				BoardingNumPos[boardingNum] = i;// record the previous pos in the boardingWaitBusActivities
				boardingNum++;
			}
		}
		if(boardingNum > (bus->getBusCapacity() - bus->getPassengerCount() + alightingNum)) {// abnormal, only some are boarding
			boardingNum = (bus->getBusCapacity() - bus->getPassengerCount() + alightingNum);// cut
		}
		for(j = 0; j < boardingNum; j++) {// extract person characteristics and calculate the corresponding boarding frames
			Person* p = dynamic_cast<Person*>(boarding_waitBusActivities[BoardingNumPos[j]]->getParent());
			if(p) {
				boardingMS += (p->getBoardingCharacteristics()*1000);// multiplied by 1000 to transfer to ms
				accumulatedBoardingMS += (p->getBoardingCharacteristics()*1000);// advance also
				if(boardingMS % busStopUpdateFrequencyMS == 0) {// deal with a special case every 5000ms, add one frame tick in case
					boardingMS += baseGranMS;// delay one frame tick, doesnt matter(100ms)
					accumulatedBoardingMS += baseGranMS;// advance also
				}
				WaitBusActivityRoleMovement* waitbusactivityrolemovement = dynamic_cast<WaitBusActivityRoleMovement*> (boarding_waitBusActivities[BoardingNumPos[j]]->Movement());
				waitbusactivityrolemovement->boardingMS = boardingMS;// set Boarding_MS for this WaitBusActivity
				waitbusactivityrolemovement->busDriver = parentBusDriver;// set busDriver for this WaitBusActivity
				boardingMSs.push_back(boardingMS);
				virtualBoardingPersons.push_back(p);// push this person in the virtual queue,
				std::cout << "BoardingNumPos[j]: " << BoardingNumPos[j] << std::endl;
				//std::cout << "boarding_waitBusActivities.size(): " << boarding_waitBusActivities.size() << std::endl;
			}
		}
		if(!boardingMSs.empty()) {
			lastBoardingMS = boardingMSs.back();// store the last_boarding_ms
		}
	}

	// no one boarding and alighting
	if(boardingMSs.empty() && alightingMSs.empty()) {
		resetBoardingAlightingVariables();// allowBoardingAlightingFlag = false
		return;
	}
	lastBoardingAlightingMS = (lastBoardingMS > lastAlightingMS) ? lastBoardingMS : lastAlightingMS;// determine the last MS, may be boarding MS or alighting MS
	busStopWaitBoardingAlightingSec = (double)((lastBoardingAlightingMS - firstBoardingAlightingMS) / 1000.0 + 0.1f);// set the dwelltime for output, some precision corrected
	allowBoardingAlightingFlag = true;// next time allow boarding and alighting individually, will not go to this whole loop to check

	// currently only monitor 857_1
	if(busline->getBusLineID() == "857_1") {
		busstopAgent->addBuslineIdAlightingNum(busline->getBusLineID(), alightingMSs.size());
		busstopAgent->addBuslineIdBoardingNum(busline->getBusLineID(), boardingMSs.size());
		busstopAgent->addBuslineIdBoardingAlightingSecs(busline->getBusLineID(), busStopWaitBoardingAlightingSec);
		busstopAgent->addBuslineIdBusTripRunSequenceNum(busline->getBusLineID(), bustrip->getBusTripRun_SequenceNum());
	}
}

void sim_mob::BusDriverMovement::StartBoardingAlighting(Bus* bus)
{
	// begin alighting and boarding
	uint32_t curr_ms = parentBusDriver->getParams().now.ms();
	int i = 0;
	
	BusStopAgent* busstopAgent = BusStopAgent::findBusStopAgentByBusStopNo(parentBusDriver->lastVisited_BusStop.get()->getBusstopno_());
	std::vector<sim_mob::WaitBusActivityRole*>& boarding_waitBusActivities = busstopAgent->getBoarding_WaitBusActivities();// get the boarding queue of persons for all Buslines

	const BusTrip* bustrip = dynamic_cast<const BusTrip*>(*(getParent()->currTripChainItem));
	// first alighting
	if(!alightingMSs.empty())// not empty for alighting, otherwise only boarding
	{
		for(i = 0; i < alightingMSs.size(); i++) {// individual alighting
			if(curr_ms == alightingMSs[i]) {
				if(!(bus->passengers_inside_bus).empty()) {
					(bus->passengers_inside_bus).erase((bus->passengers_inside_bus.begin() + AlightingNumPos[i]));// erase also from left-hand side
					// update indexes
					for(int j = 0; j < AlightingNumPos.size(); j++) {
						if (AlightingNumPos[j] > AlightingNumPos[i]){
							AlightingNumPos[j] = AlightingNumPos[j] - 1;
						}
					}
					bus->setPassengerCount(bus->getPassengerCount()-1);
				}
			}
		}
	}
	// then boarding
	if(!boardingMSs.empty())// not empty for boarding, otherwise only alighting
	{
		for(i = 0; i < boardingMSs.size(); i++) {// individual boarding
			if(curr_ms == boardingMSs[i]) {
				std::cout << "busline ID: " << bus->getBusLineID() << " bus trip Run No: " << bustrip->getBusTripRun_SequenceNum() << "BoardingNumPos[i]: " << BoardingNumPos[i] << std::endl;
				if(!boarding_waitBusActivities.empty()) {
					(bus->passengers_inside_bus).push_back(virtualBoardingPersons[i]);
					bus->setPassengerCount(bus->getPassengerCount()+1);
				}
			}
		}
	}
}

void sim_mob::BusDriverMovement::resetBoardingAlightingVariables()
{
	allowBoardingAlightingFlag = false;
	firstBoardingAlightingMS = 0;
	lastBoardingAlightingMS = 0;
	boardingmsOffset = 0;
	alightingmsOffset = 0;
	busStopWaitBoardingAlightingSec = 2;// reset dwelltime
	virtualBoardingPersons.clear();
	BoardingNumPos.clear();
	AlightingNumPos.clear();
	boardingMSs.clear();
	alightingMSs.clear();
}
}
