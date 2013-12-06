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

#include "entities/Person.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/roles/activityRole/WaitBusActivityRole.hpp"
#include "entities/models/LaneChangeModel.hpp"
#include "entities/UpdateParams.hpp"
#include "logging/Log.hpp"

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
	DriverMovement(parentAgent), parentBusDriver(nullptr), lastTickDistanceToBusStop(-1), demo_passenger_increase(false),
	dwellTime_record(0), first_busstop(true), last_busstop(false), passengerCountOld_display_flag(false),
	no_passengers_boarding(0), no_passengers_alighting(0), allow_boarding_alighting_flag(false), first_boarding_alighting_ms(0),
	last_boarding_alighting_ms(0), boardingMS_offset(0), alightingMS_offset(0),
	BUS_STOP_HOLDING_TIME_SEC(2), BUS_STOP_WAIT_BOARDING_ALIGHTING_SEC(2), waitAtStopMS(-1), BUS_STOP_WAIT_TIME(2)
{
	//BUS_STOP_WAIT_PASSENGER_TIME_SEC = 2;
	//dwellTime_record = 0;
	//passengerCountOld_display_flag = false;
	//xpos_approachingbusstop=-1;
	//ypos_approachingbusstop=-1;
	//demo_passenger_increase = false;
}

sim_mob::BusDriverMovement::~BusDriverMovement()
{

}

Vehicle* sim_mob::BusDriverMovement::initializePath_bus(bool allocateVehicle) {
	Vehicle* res = nullptr;

	//Only initialize if the next path has not been planned for yet.
	if(getParent()) {
		if (!getParent()->getNextPathPlanned()) {
			vector<const RoadSegment*> path;
			//Person* person = dynamic_cast<Person*>(parentAgent);
			int vehicle_id = 0;
			int laneID = -1;
//			if (parentAgent) {
				const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(getParent()->currTripChainItem));
				if (!bustrip){
					WarnOut("bustrip is null");
				}
				if (bustrip&& (*(getParent()->currTripChainItem))->itemType== TripChainItem::IT_BUSTRIP) {
					path = bustrip->getBusRouteInfo().getRoadSegments();
					std::cout << "BusTrip path size = " << path.size() << std::endl;
					vehicle_id = bustrip->getVehicleID();
					if (path.size() > 0) {
						laneID = path.at(0)->getLanes().size() - 2;
					}
				} else {
					if ((*(getParent()->currTripChainItem))->itemType== TripChainItem::IT_TRIP)
						std::cout << TripChainItem::IT_TRIP << " IT_TRIP\n";
					if ((*(getParent()->currTripChainItem))->itemType== TripChainItem::IT_ACTIVITY)
						std::cout << "IT_ACTIVITY\n";
					if ((*(getParent()->currTripChainItem))->itemType== TripChainItem::IT_BUSTRIP)
				       std::cout << "IT_BUSTRIP\n";
					std::cout<< "BusTrip path not initialized coz it is not a bustrip, (*(getParent()->currTripChainItem))->itemType = "<< (*(getParent()->currTripChainItem))->itemType<< std::endl;
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
	//Person* person = dynamic_cast<Person*>(parent);
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

		parentBusDriver->vehicle = new Bus(nullRoute, newVeh,bustrip_change->getBusline()->getBusLineID());
		delete newVeh;

		//This code is used by Driver to set a few properties of the Vehicle/Bus.
		if (!(parentBusDriver->vehicle->hasPath())) {
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
	if ((parentBusDriver->getParams().now.ms() / 1000.0 - parentBusDriver->startTime > 10)&& (parentBusDriver->vehicle->getDistanceMovedInSegment() > 2000) && parentBusDriver->isAleadyStarted == false) {
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
			if (parentBusDriver->getDriverParent(nv.driver)->getId() > this->getParent()->getId()) {
				nv = NearestVehicle();
			}

		}
	}
	//this function make the issue Ticket #86
	perceivedDataProcess(nv, p);
	//Person* person = dynamic_cast<Person*>(parent);
	const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(getParent()->currTripChainItem));

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
	newLatVel = lcModel->executeLaneChanging(p,parentBusDriver->vehicle->getAllRestRoadSegmentsLength(), parentBusDriver->vehicle->length,parentBusDriver->vehicle->getTurningDirection(), MLC);
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
		newLatVel = mitsim_lc_model->executeLaneChanging(p,parentBusDriver->vehicle->getAllRestRoadSegmentsLength(), parentBusDriver->vehicle->length,parentBusDriver->vehicle->getTurningDirection(), MLC);
		parentBusDriver->vehicle->setLatVelocity(newLatVel * 5);

		// reduce speed
		if (parentBusDriver->vehicle->getVelocity() / 100.0 > 2.0) {
			if (acc < -500.0) {
				parentBusDriver->vehicle->setAcceleration(acc);
			} else
				parentBusDriver->vehicle->setAcceleration(-500);
		}
		//Person* person = dynamic_cast<Person*>(parent);
		const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(getParent()->currTripChainItem));
		waitAtStopMS = 0;
	}


	if (isBusArriveBusStop() && (waitAtStopMS >= 0)&& (waitAtStopMS < BUS_STOP_WAIT_TIME)) {
//		if ((vehicle->getVelocity()/100) > 0)
		parentBusDriver->vehicle->setAcceleration(-5000);
		if (parentBusDriver->vehicle->getVelocity()/100 < 1)
			parentBusDriver->vehicle->setVelocity(0);

		if ((parentBusDriver->vehicle->getVelocity()/100 < 0.1) && (waitAtStopMS < BUS_STOP_WAIT_TIME)) {
			waitAtStopMS = waitAtStopMS + p.elapsedSeconds;

			//Pick up a semi-random number of passengers
			Bus* bus = dynamic_cast<Bus*>(parentBusDriver->vehicle);

			//std::cout << "real_ArrivalTime value: " << parentBusDriver->real_ArrivalTime.get() << "  DwellTime_ijk: " << parentBusDriver->DwellTime_ijk.get() << std::endl;
			parentBusDriver->real_ArrivalTime.set(p.now.ms());// BusDriver set RealArrival Time, set once(the first time comes in)
			bus->TimeOfBusreachingBusstop=p.now.ms();

			bus->setPassengerCountOld(bus->getPassengerCount());// record the old passenger number
//			IndividualBoardingAlighting_New(bus);

			//Back to both branches:
			//DwellTime_ijk.set(dwellTime_record);

			if ((waitAtStopMS == p.elapsedSeconds) && bus)// 0.1s
			{
				//create request for communication with bus controller
				parentBusDriver->existed_Request_Mode.set( Role::REQUEST_NONE );
				//Person* person = dynamic_cast<Person*>(parent);
				if(getParent()) {
					BusTrip* bustrip = const_cast<BusTrip*>(dynamic_cast<const BusTrip*>(*(getParent()->currTripChainItem)));
					if(bustrip && bustrip->itemType==TripChainItem::IT_BUSTRIP) {
						const Busline* busline = bustrip->getBusline();
						parentBusDriver->lastVisited_Busline.set(busline->getBusLineID());
						parentBusDriver->lastVisited_BusTrip_SequenceNo.set(bustrip->getBusTripRun_SequenceNum());
//						// any bus visited this bus stop agent, stored the curr_ms with its busline id
//						BusStopAgent* busstopAg = parentBusDriver->lastVisited_BusStop.get()->generatedBusStopAgent;
//						busstopAg->setBuslineIdCurrReachedMS(busline->getBusLineID(), parentBusDriver->getParams().now.ms());
//						// if any bus trip run num greater than 1, start to record the headway gaps for each busline buses
//						if(bustrip->getBusTripRun_SequenceNum() > 89) {
//							busstopAg->setBuslineIdHeadwayGap(busline->getBusLineID(), parentBusDriver->getParams().now.ms());
//						}
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
					//setWaitTime_BusStop(waitingtime);
					BUS_STOP_HOLDING_TIME_SEC = waitingtime;
					if(BUS_STOP_WAIT_BOARDING_ALIGHTING_SEC >= BUS_STOP_HOLDING_TIME_SEC) {// no additional holding time
						BUS_STOP_WAIT_TIME = BUS_STOP_WAIT_BOARDING_ALIGHTING_SEC;
					} else {
						BUS_STOP_WAIT_TIME = BUS_STOP_HOLDING_TIME_SEC;// additional holding time
					}
				}
				else if(mode == Role::REQUEST_STORE_ARRIVING_TIME ){
					//setWaitTime_BusStop(DwellTime_ijk.get());
					BUS_STOP_HOLDING_TIME_SEC = parentBusDriver->DwellTime_ijk.get();
				}
				else{
					std::cout << "no request existed, something is wrong!!! " << std::endl;
					//setWaitTime_BusStop(DwellTime_ijk.get());
					BUS_STOP_HOLDING_TIME_SEC = parentBusDriver->DwellTime_ijk.get();
				}
				parentBusDriver->existed_Request_Mode.set( Role::REQUEST_NONE );
				parentBusDriver->setBusStopRealTimes(parentBusDriver->busstop_sequence_no.get(), parentBusDriver->last_busStopRealTimes->get());
			}

			IndividualBoardingAlighting_New(bus);// after holding time determination, start boarding and alighting
			if(BUS_STOP_WAIT_BOARDING_ALIGHTING_SEC >= BUS_STOP_HOLDING_TIME_SEC) {// boarding alighting time greater than or equal to the holding time
				BUS_STOP_WAIT_TIME = BUS_STOP_WAIT_BOARDING_ALIGHTING_SEC;// no additional holding time
			} else {// boarding alighting time smaller than the holding time
				BUS_STOP_WAIT_TIME = BUS_STOP_HOLDING_TIME_SEC;// additional holding time
			}
			if (waitAtStopMS >= BUS_STOP_WAIT_BOARDING_ALIGHTING_SEC) {// larger than dwell time(boarding and alighting time)
				passengerCountOld_display_flag = false;
			} else {
				passengerCountOld_display_flag = true;
			}
		}
	}
	if (isBusLeavingBusStop()
			|| (waitAtStopMS >= BUS_STOP_WAIT_TIME)) {
//		std::cout << "BusDriver::updatePositionOnLink: bus isBusLeavingBusStop"
//				<< std::endl;
		waitAtStopMS = -1;
		resetBoardingAlightingVariables();// reset boarding alighting variables when leaving bus stop
		BUS_STOP_WAIT_TIME = 2;// reset waiting time
		BUS_STOP_HOLDING_TIME_SEC = 2;// reset holdingtime
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
	if(distanceToCurrentSegmentBusStop >= 0) {
		return distanceToCurrentSegmentBusStop;
	}
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
	return sim_mob::DriverMovement::dwellTimeCalculation(A, B, delta_bay, delta_full, Pfront, no_of_passengers);
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

						//std::cout<<parentBusDriver->vehicle->getDistanceMovedInSegment()<<" "<<BusDistfromStart.getMagnitude()<<std::endl;

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
void sim_mob::BusDriverMovement::frame_tick() {
	//NOTE: If this is all that is doen, we can simply delete this function and
	//      let its parent handle it automatically. ~Seth
	DriverMovement::frame_tick();
}

void sim_mob::BusDriverMovement::frame_tick_output() {
	DriverUpdateParams &p = parentBusDriver->getParams();
	if (parentBusDriver->vehicle->isDone()) {
		return;
	}

	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
		double baseAngle =
				parentBusDriver->vehicle->isInIntersection() ?
						intModel->getCurrentAngle() : parentBusDriver->vehicle->getAngle();

		//MPI-specific output.
		std::stringstream addLine;
		if (ConfigManager::GetInstance().FullConfig().using_MPI) {
			addLine <<"\",\"fake\":\"" <<(this->getParent()->isFake?"true":"false");
		}

		Bus* bus = dynamic_cast<Bus*>(parentBusDriver->vehicle);
		int passengerCount = 0;
		if (bus) {
			passengerCount = passengerCountOld_display_flag ? bus->getPassengerCountOld() : bus->getPassengerCount();
		}

		LogOut(
			"(\"BusDriver\""
			<<","<<p.now.frame()
			<<","<<getParent()->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(bus->getX())
			<<"\",\"yPos\":\""<<static_cast<int>(bus->getY())
			<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
			<<"\",\"length\":\""<<static_cast<int>(3*bus->length)
			<<"\",\"width\":\""<<static_cast<int>(2*bus->width)
			<<"\",\"passengers\":\""<<passengerCount
			<<"\",\"real_ArrivalTime\":\""<<(bus?parentBusDriver->real_ArrivalTime.get():0)
			<<"\",\"DwellTime_ijk\":\""<<(bus?parentBusDriver->DwellTime_ijk.get():0)
			<<"\",\"buslineID\":\""<<(bus?bus->getBusLineID():0)
			<<addLine.str()
			<<"\"})"<<std::endl);
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
			PassengerMovement* passenger_movement = dynamic_cast<PassengerMovement*> (passenger->Movement());
			if(passenger_movement) {
				if (passenger_movement->isBusBoarded() == true) //alighting is only for a passenger who has boarded the bus
				{
					if (passenger_movement->PassengerAlightBus(this->getParentBusDriver()) == true) //check if passenger wants to alight the bus
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
}


void sim_mob::BusDriverMovement::BoardingPassengers_Choice(Bus* bus)
{
	const Agent* parentAgent = (parentBusDriver?parentBusDriver->getParent():nullptr);
 	vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(Point2D((parentBusDriver->lastVisited_BusStop.get()->xPos - 3500),(parentBusDriver->lastVisited_BusStop.get()->yPos - 3500)),Point2D((parentBusDriver->lastVisited_BusStop.get()->xPos + 3500),(parentBusDriver->lastVisited_BusStop.get()->yPos + 3500)), parentAgent); //  nearbyAgents(Point2D(lastVisited_BusStop.get()->xPos, lastVisited_BusStop.get()->yPos), *params.currLane,3500,3500);
 	for (vector<const Agent*>::iterator it = nearby_agents.begin();it != nearby_agents.end(); it++)
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
	 	    		  no_passengers_boarding++;
	 			}

	 		}
		}
 	}
}

void sim_mob::BusDriverMovement::IndividualBoardingAlighting_New(Bus* bus)
{
	uint32_t curr_ms = parentBusDriver->getParams().now.ms();

	// determine the alighting and boarding frames, if allow_boarding_alighting_flag is true, no need to dertermined
	if((!allow_boarding_alighting_flag) && (curr_ms % 5000 != 0)) {// skip the case when (curr_ms % 5000 == 0), one time determine the boarding and alighting MSs
		DetermineBoardingAlightingMS(bus);// keep determining the boarding and alighting MSs, once determined, allow_boarding_alighting_flag is true
	}
	if(allow_boarding_alighting_flag) {
		StartBoardingAlighting(bus);// can support boarding and alighting at the same time
	}
}

void sim_mob::BusDriverMovement::DetermineBoardingAlightingMS(Bus* bus)
{
	uint32_t curr_ms = parentBusDriver->getParams().now.ms();
//	if(parentBusDriver->getParams().now.frame() >= 1201) {
//		std::cout << "Test stop pos " << std::endl;
//	}
	int i = 0;
	int j = 0;
	int boardingNum = 0;
	int alightingNum = 0;
	uint32_t boarding_ms = curr_ms;// set this to curr_frame, later add and advance
	uint32_t alighting_ms = curr_ms;// set this to curr_frame, later add and advance
	uint32_t accumulated_boarding_ms = curr_ms;// set this to curr_frame, later add and advance
	uint32_t accumulated_alighted_ms = curr_ms;// set this to curr_frame, later add and advance
	uint32_t last_boarding_ms = 0;
	uint32_t last_alighting_ms = 0;
	const uint32_t baseGranMS = ConfigManager::GetInstance().FullConfig().baseGranMS();// baseGran MS perFrame
	const RoleFactory& rf = ConfigManager::GetInstance().FullConfig().getRoleFactory();
	const Busline* busline = nullptr;
	BusStopAgent* busstopAgent = parentBusDriver->lastVisited_BusStop.get()->generatedBusStopAgent;
	std::vector<sim_mob::WaitBusActivityRole*>& boarding_waitBusActivities = busstopAgent->getBoarding_WaitBusActivities();// get the boarding queue of persons for all Buslines
	//std::cout << "boarding_waitBusActivities.size(): " << boarding_waitBusActivities.size() << std::endl;
	//Person* person = dynamic_cast<Person*>(parent);
	const BusTrip* bustrip = dynamic_cast<const BusTrip*>(*(getParent()->currTripChainItem));
	if (bustrip && bustrip->itemType == TripChainItem::IT_BUSTRIP) {
		busline = bustrip->getBusline();
		if(!busline) {
			return;
		}
	}

	first_boarding_alighting_ms = curr_ms;
	if(parentBusDriver->busstop_sequence_no.get() == 0) // first busstop, only boarding
	{
		// determine the boarding frame for each possible persons
		for(i = 0; i < boarding_waitBusActivities.size(); i++) {
			WaitBusActivityRoleMovement* waitbusactivityrolemovement = dynamic_cast<WaitBusActivityRoleMovement*> (boarding_waitBusActivities[i]->Movement());
			if(waitbusactivityrolemovement->getBuslineID() == busline->getBusLineID()) // calculate how many persons want to board this bus based on the BusLineID
			{
				BoardingNum_Pos[boardingNum] = i;// record the previous pos in the boarding_WaitBusActivities
				boardingNum++;
			}
		}
		if(boardingNum > bus->getBusCapacity()) {// abnormal, only some are boarding
			boardingNum = bus->getBusCapacity();// cut
		}
		for(j = 0; j < boardingNum; j++) {// extract person characteristics and calculate the corresponding boarding frames
			Person* p = dynamic_cast<Person*>(boarding_waitBusActivities[BoardingNum_Pos[j]]->getParent());
			if(p) {
				boarding_ms += (p->getBoardingCharacteristics()*1000);// multiplied by 1000 to transfer to ms
				accumulated_boarding_ms += (p->getBoardingCharacteristics()*1000);// multiplied by 1000 to transfer to ms
				if(boarding_ms % 5000 == 0) {// deal with a special case every 5000ms, add one frame tick in case
					boarding_ms += baseGranMS;// delay one frame tick, doesnt matter(100ms)
					accumulated_boarding_ms += baseGranMS;// advance also
				}
				WaitBusActivityRoleMovement* waitbusactivityrolemovement = dynamic_cast<WaitBusActivityRoleMovement*> (boarding_waitBusActivities[BoardingNum_Pos[j]]->Movement());
				waitbusactivityrolemovement->boarding_MS = boarding_ms;// set Boarding_MS for this WaitBusActivity
				waitbusactivityrolemovement->busDriver = parentBusDriver;// set busDriver for this WaitBusActivity
				boarding_MSs.push_back(boarding_ms);
				virtualBoarding_Persons.push_back(p);// push this person in the virtual queue
			}
		}
		if(!boarding_MSs.empty()) {
			last_boarding_ms = boarding_MSs.back();// store the last_boarding_ms
		}

	} else if(parentBusDriver->busstop_sequence_no.get() == (busStops.size() - 1)) { // last busstop, only alighting, all alighting without any choice
		for(i = 0; i < (bus->passengers_inside_bus).size(); i++) {
			AlightingNum_Pos[i] = i;
			Person* p = dynamic_cast<Person*>((bus->passengers_inside_bus)[i]);
			if(p) {
				Passenger* passenger = dynamic_cast<Passenger*>(p->getRole());
				PassengerMovement* passenger_movement = dynamic_cast<PassengerMovement*> (passenger->Movement());
				if(passenger) {
					alighting_ms += (p->getAlightingCharacteristics()*1000);// multiplied by 1000 to transfer to ms
					accumulated_alighted_ms += (p->getAlightingCharacteristics()*1000);// multiplied by 1000 to transfer to ms
					if(alighting_ms % 5000 == 0) {// deal with a special case every 50 frames, add one frame tick in case
						alighting_ms += baseGranMS;;// delay one frame tick, doesnt matter(100ms)
						accumulated_alighted_ms += baseGranMS;// advance also
					}
					if(passenger_movement) {
						passenger_movement->alighting_MS = alighting_ms;// set Alighting_MS for this Passenger
						passenger_movement->busTripRunNum = bustrip->getBusTripRun_SequenceNum();
						passenger_movement->buslineId = busline->getBusLineID();
					}
					alighting_MSs.push_back(alighting_ms);
				}
			}
		}
		if(!alighting_MSs.empty()) {
			last_alighting_ms = alighting_MSs.back();// store the last_alighting_ms
		}

	} else { // normal busstop, both boarding and alighting
		// determine the alighting frame for each possible persons
//		if(busstopAgent->getBusStop().busstopno_ == "59069") {
//			std::cout << "test here!!! " << std::endl;
//		}
		for(i = 0; i < (bus->passengers_inside_bus).size(); i++) {
			Person* p = dynamic_cast<Person*>((bus->passengers_inside_bus)[i]);
			if(p) {
				Passenger* passenger = dynamic_cast<Passenger*>(p->getRole());
				PassengerMovement* passenger_movement = dynamic_cast<PassengerMovement*> (passenger->Movement());
				if(passenger) {
					if(passenger_movement) {
						if(passenger_movement->getDestBusStop() == parentBusDriver->lastVisited_BusStop.get()) // it reached the DestBusStop and it want to alight
						{
							AlightingNum_Pos[alightingNum] = i;
							alightingNum++;
						}
					}
				}
			}
		}
		for(j = 0; j < alightingNum; j++) {// extract person characteristics and calculate the corresponding alighting frames
			Person* p = dynamic_cast<Person*>((bus->passengers_inside_bus)[AlightingNum_Pos[j]]);
			if(p) {
				Passenger* passenger = dynamic_cast<Passenger*>(p->getRole());
				PassengerMovement* passenger_movement = dynamic_cast<PassengerMovement*> (passenger->Movement());
				if(passenger) {
					alighting_ms += (p->getAlightingCharacteristics()*1000);// multiplied by 1000 to transfer to ms
					accumulated_alighted_ms += (p->getAlightingCharacteristics()*1000);// advance also
					if(alighting_ms % 5000 == 0) {// deal with a special case every 5000ms, add one frame tick in case
						alighting_ms += baseGranMS;// delay one frame tick, doesnt matter(100ms)
						accumulated_alighted_ms += baseGranMS;// advance also
					}
					if(passenger_movement) {
						passenger_movement->alighting_MS = alighting_ms;// set Alighting_MS for this Passenger
						passenger_movement->busTripRunNum = bustrip->getBusTripRun_SequenceNum();
						passenger_movement->buslineId = busline->getBusLineID();
					}
					alighting_MSs.push_back(alighting_ms);
				}
			}
		}
		if(!alighting_MSs.empty()) {
			last_alighting_ms = alighting_MSs.back();// store the last_alighting_ms
		}


		// determine the boarding frame for each possible persons
		for(i = 0; i < boarding_waitBusActivities.size(); i++) {
			WaitBusActivityRoleMovement* waitbusactivityrolemovement = dynamic_cast<WaitBusActivityRoleMovement*> (boarding_waitBusActivities[i]->Movement());
			if(waitbusactivityrolemovement->getBuslineID() == busline->getBusLineID()) // calculate how many persons want to board this bus based on the BusLineID
			{
				BoardingNum_Pos[boardingNum] = i;// record the previous pos in the boarding_WaitBusActivities
				boardingNum++;
			}
		}
		if(boardingNum > (bus->getBusCapacity() - bus->getPassengerCount() + alightingNum)) {// abnormal, only some are boarding
			boardingNum = (bus->getBusCapacity() - bus->getPassengerCount() + alightingNum);// cut
		}
		for(j = 0; j < boardingNum; j++) {// extract person characteristics and calculate the corresponding boarding frames
			Person* p = dynamic_cast<Person*>(boarding_waitBusActivities[BoardingNum_Pos[j]]->getParent());
			if(p) {
				boarding_ms += (p->getBoardingCharacteristics()*1000);// multiplied by 1000 to transfer to ms
				accumulated_boarding_ms += (p->getBoardingCharacteristics()*1000);// advance also
				if(boarding_ms % 5000 == 0) {// deal with a special case every 5000ms, add one frame tick in case
					boarding_ms += baseGranMS;// delay one frame tick, doesnt matter(100ms)
					accumulated_boarding_ms += baseGranMS;// advance also
				}
				WaitBusActivityRoleMovement* waitbusactivityrolemovement = dynamic_cast<WaitBusActivityRoleMovement*> (boarding_waitBusActivities[BoardingNum_Pos[j]]->Movement());
				waitbusactivityrolemovement->boarding_MS = boarding_ms;// set Boarding_MS for this WaitBusActivity
				waitbusactivityrolemovement->busDriver = parentBusDriver;// set busDriver for this WaitBusActivity
				boarding_MSs.push_back(boarding_ms);
				virtualBoarding_Persons.push_back(p);// push this person in the virtual queue,
				std::cout << "BoardingNum_Pos[j]: " << BoardingNum_Pos[j] << std::endl;
				//std::cout << "boarding_waitBusActivities.size(): " << boarding_waitBusActivities.size() << std::endl;
			}
		}
		if(!boarding_MSs.empty()) {
			last_boarding_ms = boarding_MSs.back();// store the last_boarding_ms
		}
	}

	// no one boarding and alighting
	if(boarding_MSs.empty() && alighting_MSs.empty()) {
		resetBoardingAlightingVariables();// allow_boarding_alighting_flag = false
		return;
	}
	last_boarding_alighting_ms = (last_boarding_ms > last_alighting_ms) ? last_boarding_ms : last_alighting_ms;// determine the last MS, may be boarding MS or alighting MS
	BUS_STOP_WAIT_BOARDING_ALIGHTING_SEC = (double)((last_boarding_alighting_ms - first_boarding_alighting_ms) / 1000.0 + 0.1f);// set the dwelltime for output, some precision corrected
	allow_boarding_alighting_flag = true;// next time allow boarding and alighting individually, will not go to this whole loop to check
	waitAtStopMS = 0;// reset waitAtStopMS after boarding alighting MS is determined
}

void sim_mob::BusDriverMovement::StartBoardingAlighting(Bus* bus)
{
	// begin alighting and boarding
	uint32_t curr_ms = parentBusDriver->getParams().now.ms();
	int i = 0;
	const RoleFactory& rf = ConfigManager::GetInstance().FullConfig().getRoleFactory();
	const Busline* busline = nullptr;
	BusStopAgent* busstopAgent = parentBusDriver->lastVisited_BusStop.get()->generatedBusStopAgent;
	std::vector<sim_mob::WaitBusActivityRole*>& boarding_waitBusActivities = busstopAgent->getBoarding_WaitBusActivities();// get the boarding queue of persons for all Buslines

	// first alighting
	if(!alighting_MSs.empty())// not empty for alighting, otherwise only boarding
	{
		for(i = 0; i < alighting_MSs.size(); i++) {// individual alighting
			if(curr_ms == alighting_MSs[i]) {
				//busstopAgent->getAlighted_Persons().push_back(bus->passengers_inside_bus[AlightingNum_Pos[i]]);// from the left-hand side
				(bus->passengers_inside_bus).erase((bus->passengers_inside_bus.begin() + AlightingNum_Pos[i]) - alightingMS_offset);// erase also from left-hand side
				bus->setPassengerCount(bus->getPassengerCount()-1);
				alightingMS_offset++;
			}
		}
	}
	// then boarding
	if(!boarding_MSs.empty())// not empty for boarding, otherwise only alighting
	{
		for(i = 0; i < boarding_MSs.size(); i++) {// individual boarding
			if(curr_ms == boarding_MSs[i]) {
				(bus->passengers_inside_bus).push_back(virtualBoarding_Persons[i]);
				std::cout << "BoardingNum_Pos[i]: " << BoardingNum_Pos[i] << std::endl;
				boarding_waitBusActivities.erase((boarding_waitBusActivities.begin() + BoardingNum_Pos[i]) - boardingMS_offset);//  erase this Person in the BusStopAgent
				bus->setPassengerCount(bus->getPassengerCount()+1);
				boardingMS_offset++;
			}
		}
	}
}

void sim_mob::BusDriverMovement::resetBoardingAlightingVariables()
{
	allow_boarding_alighting_flag = false;
	first_boarding_alighting_ms = 0;
	last_boarding_alighting_ms = 0;
	boardingMS_offset = 0;
	alightingMS_offset = 0;
//	BUS_STOP_WAIT_TIME = 2;// reset waiting time
//	BUS_STOP_HOLDING_TIME_SEC = 2;// reset holdingtime
	BUS_STOP_WAIT_BOARDING_ALIGHTING_SEC = 2;// reset dwelltime
	virtualBoarding_Persons.clear();
	BoardingNum_Pos.clear();
	AlightingNum_Pos.clear();
	boarding_MSs.clear();
	alighting_MSs.clear();
}
}
