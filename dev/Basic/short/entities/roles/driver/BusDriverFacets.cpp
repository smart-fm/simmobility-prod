/*
 * BusDriverFacets.cpp
 *
 *  Created on: May 16th, 2013
 *      Author: Yao Jin
 */

#include "BusDriverFacets.hpp"

#include "entities/Person.hpp"
#include "entities/UpdateParams.hpp"

namespace sim_mob {
BusDriverBehavior::BusDriverBehavior(sim_mob::Person* parentAgent):
	DriverBehavior(parentAgent), parentBusDriver(nullptr) {}

BusDriverBehavior::~BusDriverBehavior() {}

void BusDriverBehavior::frame_init(UpdateParams& p) {
	throw std::runtime_error("BusDriverBehavior::frame_init is not implemented yet");
}

void BusDriverBehavior::frame_tick(UpdateParams& p) {
	throw std::runtime_error("BusDriverBehavior::frame_tick is not implemented yet");
}

void BusDriverBehavior::frame_tick_output(const UpdateParams& p) {
	throw std::runtime_error("BusDriverBehavior::frame_tick_output is not implemented yet");
}

void BusDriverBehavior::frame_tick_output_mpi(timeslice now) {
	throw std::runtime_error("BusDriverBehavior::frame_tick_output_mpi is not implemented yet");
}

sim_mob::BusDriverMovement::BusDriverMovement(sim_mob::Person* parentAgent):
	DriverMovement(parentAgent), parentBusDriver(nullptr), lastTickDistanceToBusStop(-1),
	first_busstop(true), last_busstop(false), no_passengers_boarding(0), no_passengers_alighting(0),
	waitAtStopMS(-1)
{
	BUS_STOP_WAIT_PASSENGER_TIME_SEC = 2;
	dwellTime_record = 0;
	passengerCountOld_display_flag = false;
//	xpos_approachingbusstop=-1;
//	ypos_approachingbusstop=-1;
	demo_passenger_increase = false;
}

sim_mob::BusDriverMovement::~BusDriverMovement()
{

}

void sim_mob::BusDriverMovement::flowIntoNextLinkIfPossible(UpdateParams& p)
{

}

Vehicle* sim_mob::BusDriverMovement::initializePath_bus(bool allocateVehicle) {
	Vehicle* res = nullptr;

	//Only initialize if the next path has not been planned for yet.
	if(parentAgent) {
		if (!parentAgent->getNextPathPlanned()) {
			vector<const RoadSegment*> path;
			//Person* person = dynamic_cast<Person*>(parentAgent);
			int vehicle_id = 0;
			int laneID = -1;
//			if (parentAgent) {
				const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(parentAgent->currTripChainItem));
				if (!bustrip)
					std::cout << "bustrip is null\n";
				if (bustrip&& (*(parentAgent->currTripChainItem))->itemType== TripChainItem::IT_BUSTRIP) {
					path = bustrip->getBusRouteInfo().getRoadSegments();
					std::cout << "BusTrip path size = " << path.size() << std::endl;
					vehicle_id = bustrip->getVehicleID();
					if (path.size() > 0) {
						laneID = path.at(0)->getLanes().size() - 2;
					}
				} else {
					if ((*(parentAgent->currTripChainItem))->itemType== TripChainItem::IT_TRIP)
						std::cout << TripChainItem::IT_TRIP << " IT_TRIP\n";
					if ((*(parentAgent->currTripChainItem))->itemType== TripChainItem::IT_ACTIVITY)
						std::cout << "IT_ACTIVITY\n";
					if ((*(parentAgent->currTripChainItem))->itemType== TripChainItem::IT_BUSTRIP)
				       std::cout << "IT_BUSTRIP\n";
					std::cout<< "BusTrip path not initialized coz it is not a bustrip, (*(parentAgent->currTripChainItem))->itemType = "<< (*(parentAgent->currTripChainItem))->itemType<< std::endl;
				}

//			}

			//TODO: Start in lane 0?
			int startlaneID = 1;

			BusDriver* v = dynamic_cast<BusDriver*>(this);
			if (v && laneID != -1) {
				startlaneID = laneID; //need to check if lane valid
				//parentP->laneID = -1;
			}

			// Bus should be at least 1200 to be displayed on Visualizer
			const double length = dynamic_cast<BusDriver*>(this) ? 1200 : 400;
			const double width = 200;

			//A non-null vehicle means we are moving.
			if (allocateVehicle) {
				res = new Vehicle(path, startlaneID, vehicle_id, length, width);
			}
		}

		//to indicate that the path to next activity is already planned
		parentAgent->setNextPathPlanned(true);
	}

	return res;
}

void sim_mob::BusDriverMovement::frame_init(UpdateParams& p) {
	//TODO: "initializePath()" in Driver mixes initialization of the path and
	//      creation of the Vehicle (e.g., its width/height). These are both
	//      very different for Cars and Buses, but until we un-tangle the code
	//      we'll need to rely on hacks like this.
	Vehicle* newVeh = nullptr;
	//Person* person = dynamic_cast<Person*>(parent);
	if (parentAgent) {
		if (parentAgent->getAgentSrc() == "BusController") {
			newVeh = initializePath_bus(true); // no need any node information
		} else {
			newVeh = initializePath(true); // previous node to node calculation
		}
	}

	//Save the path, create a vehicle.
	if (newVeh) {
		//Use this sample vehicle to build our Bus, then delete the old vehicle.
		BusRoute nullRoute; //Buses don't use the route at the moment.

		TripChainItem* tci = *(parentAgent->currTripChainItem);
		BusTrip* bustrip_change =dynamic_cast<BusTrip*>(tci);
		if (!bustrip_change) {
			throw std::runtime_error("BusDriver created without an appropriate BusTrip item.");
		}

		parentBusDriver->vehicle = new Bus(nullRoute, newVeh,bustrip_change->getBusline()->getBusLineID());
		delete newVeh;

		//This code is used by Driver to set a few properties of the Vehicle/Bus.
		if (!(parentBusDriver->vehicle->hasPath())) {
			throw std::runtime_error(
					"Vehicle could not be created for bus driver; no route!");
		}

		//Set the bus's origin and set of stops.
		setOrigin(parentBusDriver->params);
		if (parentAgent) {
			if (parentAgent->getAgentSrc() == "BusController") {
				const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(parentAgent->currTripChainItem));
				if (bustrip && bustrip->itemType == TripChainItem::IT_BUSTRIP) {
					busStops = bustrip->getBusRouteInfo().getBusStops();
					if (busStops.empty()) {
						std::cout<< "Error: No BusStops assigned from BusTrips!!! "<< std::endl;
						// This case can be true, so use the BusStops found by Path instead
						busStops = findBusStopInPath(parentBusDriver->vehicle->getCompletePath());
					}
				}
			} else {
				busStops = findBusStopInPath(parentBusDriver->vehicle->getCompletePath());
			}
		}
		//Unique to BusDrivers: reset your route
		waitAtStopMS = 0.0;
	}
}

vector<const BusStop*> sim_mob::BusDriverMovement::findBusStopInPath(const vector<const RoadSegment*>& path) const {
	//NOTE: Use typedefs instead of defines.
	typedef vector<const BusStop*> BusStopVector;
	BusStopVector res;
	int busStopAmount = 0;
	vector<const RoadSegment*>::const_iterator it;
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
				res.push_back(bs);
			}
		}
	}
	return res;
}

double sim_mob::BusDriverMovement::linkDriving(DriverUpdateParams& p)
{
	if ((parentBusDriver->params.now.ms() / 1000.0 - parentBusDriver->startTime > 10)&& (parentBusDriver->vehicle->getDistanceMovedInSegment() > 2000) && parentBusDriver->isAleadyStarted == false) {
		parentBusDriver->isAleadyStarted = true;
	}
	p.isAlreadyStart = parentBusDriver->isAleadyStarted;
	if (!(parentBusDriver->vehicle->hasNextSegment(true))) {
		p.dis2stop = parentBusDriver->vehicle->getAllRestRoadSegmentsLength()- parentBusDriver->vehicle->getDistanceMovedInSegment() - parentBusDriver->vehicle->length / 2- 300;
		if (p.nvFwd.distance < p.dis2stop)
			p.dis2stop = p.nvFwd.distance;
		p.dis2stop /= 100;
	} else {
		p.nextLaneIndex = std::min<int>(p.currLaneIndex,parentBusDriver->vehicle->getNextSegment()->getLanes().size() - 1);
		if (parentBusDriver->vehicle->getNextSegment()->getLanes().at(p.nextLaneIndex)->is_pedestrian_lane()) {
			p.nextLaneIndex--;
			p.dis2stop = parentBusDriver->vehicle->getCurrPolylineLength()- parentBusDriver->vehicle->getDistanceMovedInSegment() + 1000;
		} else
			p.dis2stop = 1000;//defalut 1000m
	}

	//get nearest car, if not making lane changing, the nearest car should be the leading car in current lane.
	//if making lane changing, adjacent car need to be taken into account.
	NearestVehicle & nv = nearestVehicle(p);
	if (parentBusDriver->isAleadyStarted == false) {
		if (nv.distance <= 0) {
			if (parentBusDriver->getDriverParent(nv.driver)->getId() > this->parentAgent->getId()) {
				nv = NearestVehicle();
			}

		}
	}
	//this function make the issue Ticket #86
	perceivedDataProcess(nv, p);
	//Person* person = dynamic_cast<Person*>(parent);
	const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(parentAgent->currTripChainItem));

	//bus approaching bus stop reduce speed
	//and if its left has lane, merge to left lane
	p.currSpeed = parentBusDriver->vehicle->getVelocity() / 100;
	double newFwdAcc = 0;
	newFwdAcc = cfModel->makeAcceleratingDecision(p, targetSpeed, maxLaneSpeed);
	if (abs(parentBusDriver->vehicle->getTurningDirection() != LCS_SAME) && newFwdAcc > 0&& parentBusDriver->vehicle->getVelocity() / 100 > 10) {
		newFwdAcc = 0;
	}
	parentBusDriver->vehicle->setAcceleration(newFwdAcc * 100);

	//NOTE: Driver already has a lcModel; we should be able to just use this. ~Seth
	LANE_CHANGE_SIDE lcs = LCS_SAME;
	MITSIM_LC_Model* mitsim_lc_model = dynamic_cast<MITSIM_LC_Model*>(lcModel);
	if (mitsim_lc_model) {
		lcs = mitsim_lc_model->makeMandatoryLaneChangingDecision(p);
	} else {
		throw std::runtime_error("TODO: BusDrivers currently require the MITSIM lc model.");
	}

	parentBusDriver->vehicle->setTurningDirection(lcs);
	double newLatVel;
	newLatVel = lcModel->executeLaneChanging(p,parentBusDriver->vehicle->getAllRestRoadSegmentsLength(), parentBusDriver->vehicle->length,parentBusDriver->vehicle->getTurningDirection());
	parentBusDriver->vehicle->setLatVelocity(newLatVel * 10);
	if (parentBusDriver->vehicle->getLatVelocity() > 0)
		parentBusDriver->vehicle->setTurningDirection(LCS_LEFT);
	else if (parentBusDriver->vehicle->getLatVelocity() < 0)
		parentBusDriver->vehicle->setTurningDirection(LCS_RIGHT);
	else
		parentBusDriver->vehicle->setTurningDirection(LCS_SAME);

	p.turningDirection = parentBusDriver->vehicle->getTurningDirection();

	if (isBusApproachingBusStop()) {
		double acc = busAccelerating(p) * 100;

		//move to most left lane
		p.nextLaneIndex =parentBusDriver->vehicle->getCurrSegment()->getLanes().back()->getLaneID();
		LANE_CHANGE_SIDE lcs =mitsim_lc_model->makeMandatoryLaneChangingDecision(p);
		parentBusDriver->vehicle->setTurningDirection(lcs);
		double newLatVel;
		newLatVel = mitsim_lc_model->executeLaneChanging(p,parentBusDriver->vehicle->getAllRestRoadSegmentsLength(), parentBusDriver->vehicle->length,parentBusDriver->vehicle->getTurningDirection());
		parentBusDriver->vehicle->setLatVelocity(newLatVel * 5);

		// reduce speed
		if (parentBusDriver->vehicle->getVelocity() / 100.0 > 2.0) {
			if (acc < -500.0) {
				parentBusDriver->vehicle->setAcceleration(acc);
			} else
				parentBusDriver->vehicle->setAcceleration(-500);
		}
		//Person* person = dynamic_cast<Person*>(parent);
		const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(parentAgent->currTripChainItem));
		waitAtStopMS = 0;
	}


	if (isBusArriveBusStop() && (waitAtStopMS >= 0)&& (waitAtStopMS < BUS_STOP_WAIT_PASSENGER_TIME_SEC)) {

//		if ((vehicle->getVelocity()/100) > 0)
		parentBusDriver->vehicle->setAcceleration(-5000);
		if (parentBusDriver->vehicle->getVelocity()/100 < 1)
			parentBusDriver->vehicle->setVelocity(0);

		if ((parentBusDriver->vehicle->getVelocity()/100 < 0.1) && (waitAtStopMS < BUS_STOP_WAIT_PASSENGER_TIME_SEC)) {
			waitAtStopMS = waitAtStopMS + p.elapsedSeconds;

			//Pick up a semi-random number of passengers
			Bus* bus = dynamic_cast<Bus*>(parentBusDriver->vehicle);
			if ((waitAtStopMS == p.elapsedSeconds) && bus)
			{
				std::cout << "real_ArrivalTime value: " << parentBusDriver->real_ArrivalTime.get() << "  DwellTime_ijk: " << parentBusDriver->DwellTime_ijk.get() << std::endl;
				parentBusDriver->real_ArrivalTime.set(p.now.ms());// BusDriver set RealArrival Time, set once(the first time comes in)
				bus->TimeOfBusreachingBusstop=p.now.ms();

				//From Meenu's branch; enable if needed.
				//dwellTime_record = passengerGeneration(bus);//if random distribution is to be used,uncomment4

				no_passengers_alighting=0;
				no_passengers_boarding=0;
				bus->setPassengerCountOld(bus->getPassengerCount());// record the old passenger number
				AlightingPassengers(bus);//first alight passengers inside the bus
				BoardingPassengers_Choice(bus);//then board passengers waiting at the bus stop
				//BoardingPassengers_Normal(bus);
				dwellTime_record = dwellTimeCalculation(no_passengers_alighting,no_passengers_boarding,0,0,0,bus->getPassengerCountOld());

				//Back to both branches:
				parentBusDriver->DwellTime_ijk.set(dwellTime_record);

				//create request for communication with bus controller
				parentBusDriver->existed_Request_Mode.set( Role::REQUEST_NONE );
				//Person* person = dynamic_cast<Person*>(parent);
				if(parentAgent) {
					BusTrip* bustrip = const_cast<BusTrip*>(dynamic_cast<const BusTrip*>(*(parentAgent->currTripChainItem)));
					if(bustrip && bustrip->itemType==TripChainItem::IT_BUSTRIP) {
						const Busline* busline = bustrip->getBusline();
						parentBusDriver->lastVisited_Busline.set(busline->getBusLineID());
						parentBusDriver->lastVisited_BusTrip_SequenceNo.set(bustrip->getBusTripRun_SequenceNum());
						if (busline) {
							if(busline->getControl_TimePointNum0() == parentBusDriver->busstop_sequence_no.get() || busline->getControl_TimePointNum1() == parentBusDriver->busstop_sequence_no.get()) { // only use holding control at selected time points
								parentBusDriver->existed_Request_Mode.set( Role::REQUEST_DECISION_TIME );
							}
							else{
								parentBusDriver->existed_Request_Mode.set( Role::REQUEST_STORE_ARRIVING_TIME );
							}
						}
					}
				}
			}
			else if(fabs(waitAtStopMS-p.elapsedSeconds * 3.0)<0.0000001 && bus)
			{
				int mode = parentBusDriver->existed_Request_Mode.get();
				if(mode == Role::REQUEST_DECISION_TIME ){
					double waitingtime = parentBusDriver->waiting_Time.get();
					setWaitTime_BusStop(waitingtime);
				}
				else if(mode == Role::REQUEST_STORE_ARRIVING_TIME ){
					setWaitTime_BusStop(parentBusDriver->DwellTime_ijk.get());
				}
				else{
					std::cout << "no request existed, something is wrong!!! " << std::endl;
					setWaitTime_BusStop(parentBusDriver->DwellTime_ijk.get());
				}
				parentBusDriver->existed_Request_Mode.set( Role::REQUEST_NONE );
				parentBusDriver->busStopRealTimes_vec_bus[parentBusDriver->busstop_sequence_no.get()]->set(parentBusDriver->last_busStopRealTimes->get());
			}
			if (waitAtStopMS >= dwellTime_record) {
				passengerCountOld_display_flag = false;
			} else {
				passengerCountOld_display_flag = true;
			}
		}
	}
	if (isBusLeavingBusStop()
			|| (waitAtStopMS >= BUS_STOP_WAIT_PASSENGER_TIME_SEC)) {
		std::cout << "BusDriver::updatePositionOnLink: bus isBusLeavingBusStop"
				<< std::endl;
		waitAtStopMS = -1;
		BUS_STOP_WAIT_PASSENGER_TIME_SEC = 2;// reset when leaving bus stop
		parentBusDriver->vehicle->setAcceleration(busAccelerating(p) * 100);
	}

	//Update our distance
	lastTickDistanceToBusStop = distanceToNextBusStop();

	DynamicVector segmentlength(
			parentBusDriver->vehicle->getCurrSegment()->getStart()->location.getX(),
			parentBusDriver->vehicle->getCurrSegment()->getStart()->location.getY(),
			parentBusDriver->vehicle->getCurrSegment()->getEnd()->location.getX(),
			parentBusDriver->vehicle->getCurrSegment()->getEnd()->location.getY());

	//Return the remaining amount (obtained by calling updatePositionOnLink)
	return updatePositionOnLink(p);
}

//double sim_mob::BusDriverMovement::getPositionX() const {
//	return parentBusDriver->vehicle ? parentBusDriver->vehicle->getX() : 0;
//}
//
//double sim_mob::BusDriverMovement::getPositionY() const {
//	return parentBusDriver->vehicle ? parentBusDriver->vehicle->getY() : 0;
//}

double sim_mob::BusDriverMovement::busAccelerating(DriverUpdateParams& p) {
	//Retrieve a new acceleration value.
	double newFwdAcc = 0;

	//Convert back to m/s
	//TODO: Is this always m/s? We should rename the variable then...
	p.currSpeed = parentBusDriver->vehicle->getVelocity() / 100;

	//Call our model
	newFwdAcc = cfModel->makeAcceleratingDecision(p, targetSpeed, maxLaneSpeed);

	return newFwdAcc;
	//Update our chosen acceleration; update our position on the link.
	//vehicle->setAcceleration(newFwdAcc * 100);
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
			parentBusDriver->vehicle->getCurrSegment());
	double distanceToNextSegmentBusStop = -1;
	if (parentBusDriver->vehicle->hasNextSegment(true))
		distanceToNextSegmentBusStop = getDistanceToBusStopOfSegment(
				parentBusDriver->vehicle->getNextSegment(true));

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

double sim_mob::BusDriverMovement::getDistanceToBusStopOfSegment(const RoadSegment* rs) {

	double distance = -100;
	double currentX = parentBusDriver->vehicle->getX();
	double currentY = parentBusDriver->vehicle->getY();
	const std::map<centimeter_t, const RoadItem*> & obstacles = rs->obstacles;
	for (std::map<centimeter_t, const RoadItem*>::const_iterator o_it =
			obstacles.begin(); o_it != obstacles.end(); o_it++) {
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
					last_busstop = true;
				}
				if (rs == parentBusDriver->vehicle->getCurrSegment()) {

					if (stopPoint < 0) {
						throw std::runtime_error(
								"BusDriver offset in obstacles list should never be <0");
					}

					if (stopPoint >= 0) {
						DynamicVector BusDistfromStart(parentBusDriver->vehicle->getX(),
								parentBusDriver->vehicle->getY(),
								rs->getStart()->location.getX(),
								rs->getStart()->location.getY());
//						distance = stopPoint
//								- vehicle->getDistanceMovedInSegment();
						// Buses not stopping near the busstop at few places.
						// one easy way to fix it
						double actualDistance = sim_mob::BusStop::EstimateStopPoint(bs->xPos, bs->yPos, rs);

						std::cout<<parentBusDriver->vehicle->getDistanceMovedInSegment()<<" "<<BusDistfromStart.getMagnitude()<<std::endl;

						distance = actualDistance
								- BusDistfromStart.getMagnitude();


						break;

					}

				} else {
					DynamicVector busToSegmentStartDistance(currentX, currentY,
							rs->getStart()->location.getX(),
							rs->getStart()->location.getY());
					distance = parentBusDriver->vehicle->getCurrentSegmentLength()
							- parentBusDriver->vehicle->getDistanceMovedInSegment() + stopPoint;

				}
			} // end of if isFound
		}
	}

	return distance / 100.0;
}

//Main update functionality
void sim_mob::BusDriverMovement::frame_tick(UpdateParams& p) {
	//NOTE: If this is all that is doen, we can simply delete this function and
	//      let its parent handle it automatically. ~Seth
	DriverMovement::frame_tick(p);
}

void sim_mob::BusDriverMovement::frame_tick_output(const UpdateParams& p) {
	//Skip?
	if (parentBusDriver->vehicle->isDone()
			|| ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

	if (ConfigParams::GetInstance().OutputEnabled()) {
		double baseAngle =
				parentBusDriver->vehicle->isInIntersection() ?
						intModel->getCurrentAngle() : parentBusDriver->vehicle->getAngle();
		Bus* bus = dynamic_cast<Bus*>(parentBusDriver->vehicle);
		if (!passengerCountOld_display_flag) {
			LogOut(
					"(\"BusDriver\""
					<<","<<p.now.frame()
					<<","<<parentAgent->getId()
					<<",{" <<"\"xPos\":\""<<static_cast<int>(bus->getX())
					<<"\",\"yPos\":\""<<static_cast<int>(bus->getY())
					<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
					<<"\",\"length\":\""<<static_cast<int>(3*bus->length)
					<<"\",\"width\":\""<<static_cast<int>(2*bus->width)
					<<"\",\"passengers\":\""<<(bus?bus->getPassengerCount():0)
					<<"\",\"real_ArrivalTime\":\""<<(bus?parentBusDriver->real_ArrivalTime.get():0)
					<<"\",\"DwellTime_ijk\":\""<<(bus?parentBusDriver->DwellTime_ijk.get():0)
					<<"\",\"buslineID\":\""<<(bus?bus->getBusLineID():0) <<"\"})"<<std::endl);
		} else {
			LogOut(
					"(\"BusDriver\""
					<<","<<p.now.frame()
					<<","<<parentAgent->getId()
					<<",{" <<"\"xPos\":\""<<static_cast<int>(bus->getX())
					<<"\",\"yPos\":\""<<static_cast<int>(bus->getY())
					<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
					<<"\",\"length\":\""<<static_cast<int>(3*bus->length)
					<<"\",\"width\":\""<<static_cast<int>(2*bus->width)
					<<"\",\"passengers\":\""<<(bus?bus->getPassengerCountOld():0)
					<<"\",\"real_ArrivalTime\":\""<<(bus?parentBusDriver->real_ArrivalTime.get():0)
					<<"\",\"DwellTime_ijk\":\""<<(bus?parentBusDriver->DwellTime_ijk.get():0)
					<<"\",\"buslineID\":\""<<(bus?bus->getBusLineID():0) <<"\"})"<<std::endl);
		}
	}
}

void sim_mob::BusDriverMovement::frame_tick_output_mpi(timeslice now) {
	//Skip output?
	if (now.frame() < parentAgent->getStartTime() || parentBusDriver->vehicle->isDone()) {
		return;
	}

	if (ConfigParams::GetInstance().OutputEnabled()) {
		double baseAngle =
				parentBusDriver->vehicle->isInIntersection() ?
						intModel->getCurrentAngle() : parentBusDriver->vehicle->getAngle();
//The BusDriver class will only maintain buses as the current vehicle.
		const Bus* bus = dynamic_cast<const Bus*>(parentBusDriver->vehicle);
		std::stringstream logout;
		if (!passengerCountOld_display_flag) {
			logout << "(\"Driver\"" << "," << now.frame() << ","
					<< parentAgent->getId() << ",{" << "\"xPos\":\""
					<< static_cast<int>(bus->getX()) << "\",\"yPos\":\""
					<< static_cast<int>(bus->getY()) << "\",\"segment\":\""
					<< bus->getCurrSegment()->getId() << "\",\"angle\":\""
					<< (360 - (baseAngle * 180 / M_PI)) << "\",\"length\":\""
					<< static_cast<int>(bus->length) << "\",\"width\":\""
					<< static_cast<int>(bus->width) << "\",\"passengers\":\""
					<< (bus ? bus->getPassengerCount() : 0);
		} else {
			logout << "(\"Driver\"" << "," << now.frame() << ","
					<< parentAgent->getId() << ",{" << "\"xPos\":\""
					<< static_cast<int>(bus->getX()) << "\",\"yPos\":\""
					<< static_cast<int>(bus->getY()) << "\",\"segment\":\""
					<< bus->getCurrSegment()->getId() << "\",\"angle\":\""
					<< (360 - (baseAngle * 180 / M_PI)) << "\",\"length\":\""
					<< static_cast<int>(bus->length) << "\",\"width\":\""
					<< static_cast<int>(bus->width) << "\",\"passengers\":\""
					<< (bus ? bus->getPassengerCountOld() : 0);
		}

		if (this->parentAgent->isFake) {
			logout << "\",\"fake\":\"" << "true";
		} else {
			logout << "\",\"fake\":\"" << "false";
		}

		logout << "\"})" << std::endl;

		LogOut(logout.str());
	}
}

void sim_mob::BusDriverMovement::AlightingPassengers(Bus* bus)//for alighting passengers
{
	if (bus->getPassengerCount() > 0) {
		int i = 0;
		vector<Person*>::iterator itr = bus->passengers_inside_bus.begin();
		while (itr != bus->passengers_inside_bus.end())
		{
			//Retrieve only Passenger agents inside the bus
			Person* p = bus->passengers_inside_bus[i];
			Passenger* passenger =p ? dynamic_cast<Passenger*>(p->getRole()) : nullptr;
			if (!passenger)
				continue;
			if (passenger->isBusBoarded() == true) //alighting is only for a passenger who has boarded the bus
			{
				if (passenger->PassengerAlightBus(this->getParentBusDriver()) == true) //check if passenger wants to alight the bus
				{
					itr = (bus->passengers_inside_bus).erase(bus->passengers_inside_bus.begin() + i);
					no_passengers_alighting++;
				}
				else
				{
					itr++;
					i++;
				}
			}
		}
	}
}


void sim_mob::BusDriverMovement::BoardingPassengers_Choice(Bus* bus)
 {
 	vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(Point2D((parentBusDriver->lastVisited_BusStop.get()->xPos - 3500),(parentBusDriver->lastVisited_BusStop.get()->yPos - 3500)),Point2D((parentBusDriver->lastVisited_BusStop.get()->xPos + 3500),(parentBusDriver->lastVisited_BusStop.get()->yPos + 3500))); //  nearbyAgents(Point2D(lastVisited_BusStop.get()->xPos, lastVisited_BusStop.get()->yPos), *params.currLane,3500,3500);
 	for (vector<const Agent*>::iterator it = nearby_agents.begin();it != nearby_agents.end(); it++)
 	{
 		//Retrieve only Passenger agents.
 		const Person* person = dynamic_cast<const Person *>(*it);
 		Person* p = const_cast<Person *>(person);
 		Passenger* passenger = p ? dynamic_cast<Passenger*>(p->getRole()) : nullptr;
 		if (!passenger)
 		  continue;
 		if ((abs((passenger->getXYPosition().getX())- (parentBusDriver->xpos_approachingbusstop)) <= 2)and (abs((passenger->getXYPosition().getY() / 1000)- (parentBusDriver->ypos_approachingbusstop / 1000)) <= 2))
 		 {
 	       if (passenger->isAtBusStop() == true) //if passenger agent is waiting at the approaching bus stop
 			{
 	    	   if(passenger->PassengerBoardBus_Choice(this->getParentBusDriver())==true)
 	    		  no_passengers_boarding++;
 			}

 		}
 	}
 }
}
