/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusDriver.hpp"
#include <vector>

#include "DriverUpdateParams.hpp"

#include "entities/Person.hpp"
#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "util/DebugFlags.hpp"

#include "util/PassengerDistribution.hpp"

using namespace sim_mob;
using std::vector;
using std::map;
using std::string;

namespace {
//const int BUS_STOP_WAIT_PASSENGER_TIME_SEC = 2;
} //End anonymous namespace

sim_mob::BusDriver::BusDriver(Person* parent, MutexStrategy mtxStrat)
	: Driver(parent, mtxStrat), nextStop(nullptr), waitAtStopMS(-1) , lastTickDistanceToBusStop(-1)
, lastVisited_BusStop(mtxStrat,nullptr), lastVisited_BusStopSequenceNum(mtxStrat,0), real_DepartureTime(mtxStrat,0)
, real_ArrivalTime(mtxStrat,0), DwellTime_ijk(mtxStrat,0), busstop_sequence_no(mtxStrat,0)
, first_busstop(true), last_busstop(false), no_passengers_boarding(0), no_passengers_alighting(0)
{
	BUS_STOP_WAIT_PASSENGER_TIME_SEC = 2;
	dwellTime_record = 0;
	passengerCountOld_display_flag = false;
	curr_busStopRealTimes = new Shared<BusStop_RealTimes>(mtxStrat,BusStop_RealTimes());
}


Role* sim_mob::BusDriver::clone(Person* parent) const
{
	return new BusDriver(parent, parent->getMutexStrategy());
}


Vehicle* sim_mob::BusDriver::initializePath_bus(bool allocateVehicle)
{
	Vehicle* res = nullptr;

	//Only initialize if the next path has not been planned for yet.
	if(!parent->getNextPathPlanned()) {
		vector<const RoadSegment*> path;
		Person* person = dynamic_cast<Person*>(parent);
		int vehicle_id = 0;
		if(person) {
			const BusTrip* bustrip = dynamic_cast<const BusTrip*>(person->currTripChainItem);
			if(bustrip && person->currTripChainItem->itemType==TripChainItem::IT_BUSTRIP) {
				path = bustrip->getBusRouteInfo().getRoadSegments();
				vehicle_id = bustrip->getVehicleID();
			}
		}

		//TODO: Start in lane 0?
		int startlaneID = 0;

		// Bus should be at least 1200 to be displayed on Visualizer
		const double length = dynamic_cast<BusDriver*>(this) ? 1200 : 400;
		const double width = 200;

		//A non-null vehicle means we are moving.
		if (allocateVehicle) {
			res = new Vehicle(path, startlaneID, vehicle_id, length, width);
		}
	}

	//to indicate that the path to next activity is already planned
	parent->setNextPathPlanned(true);
	return res;
}

//We're recreating the parent class's frame_init() method here.
// Our goal is to reuse as much of Driver as possible, and then
// refactor common code out later. We could just call the frame_init() method
// directly, but there's some unexpected interdependencies.
void sim_mob::BusDriver::frame_init(UpdateParams& p)
{
	//TODO: "initializePath()" in Driver mixes initialization of the path and
	//      creation of the Vehicle (e.g., its width/height). These are both
	//      very different for Cars and Buses, but until we un-tangle the code
	//      we'll need to rely on hacks like this.
	Vehicle* newVeh = nullptr;
	Person* person = dynamic_cast<Person*>(parent);
	if(person)
	{
		if(person->getAgentSrc() == "BusController") {
			newVeh = initializePath_bus(true);// no need any node information
		} else {
			newVeh = initializePath(true);// previous node to node calculation
		}
	}

	//Save the path, create a vehicle.
	if (newVeh) {
		//Use this sample vehicle to build our Bus, then delete the old vehicle.
		BusRoute nullRoute; //Buses don't use the route at the moment.
		vehicle = new Bus(nullRoute, newVeh);
		delete newVeh;

		//This code is used by Driver to set a few properties of the Vehicle/Bus.
		if (!vehicle->hasPath()) {
			throw std::runtime_error("Vehicle could not be created for bus driver; no route!");
		}

		//Set the bus's origin and set of stops.
		setOrigin(params);
		if(person)
		{
			if(person->getAgentSrc() == "BusController") {
				const BusTrip* bustrip = dynamic_cast<const BusTrip*>(person->currTripChainItem);
				if(bustrip && person->currTripChainItem->itemType==TripChainItem::IT_BUSTRIP) {
					busStops = bustrip->getBusRouteInfo().getBusStops();
					if (busStops.empty()) {
						std::cout << "Error: No BusStops assigned from BusTrips!!! " << std::endl;
						// This case can be true, so use the BusStops found by Path instead
						busStops = findBusStopInPath(vehicle->getCompletePath());
					}
				}
			} else {
				busStops = findBusStopInPath(vehicle->getCompletePath());
			}
		}

		//Unique to BusDrivers: reset your route
		waitAtStopMS = 0.0;
	}
}

vector<const BusStop*> sim_mob::BusDriver::findBusStopInPath(const vector<const RoadSegment*>& path) const
{
	//NOTE: Use typedefs instead of defines.
	typedef vector<const BusStop*> BusStopVector;

	BusStopVector res;
	int busStopAmount = 0;
	vector<const RoadSegment*>::const_iterator it;
	for( it = path.begin(); it != path.end(); ++it)
	{
		// get obstacles in road segment
		const RoadSegment* rs = (*it);
		const std::map<centimeter_t, const RoadItem*> & obstacles = rs->obstacles;

		//Check each of these.
		std::map<centimeter_t, const RoadItem*>::const_iterator ob_it;
		for(ob_it = obstacles.begin(); ob_it != obstacles.end(); ++ob_it)
		{
			RoadItem* ri = const_cast<RoadItem*>(ob_it->second);
			BusStop *bs = dynamic_cast<BusStop*>(ri);
			if(bs) {
				res.push_back(bs);
			}
		}
	}
	return res;
}
double sim_mob::BusDriver::linkDriving(DriverUpdateParams& p)
{

	if (!vehicle->hasNextSegment(true)) {
		p.dis2stop = vehicle->getAllRestRoadSegmentsLength()
				   - vehicle->getDistanceMovedInSegment() - vehicle->length / 2 - 300;
		if (p.nvFwd.distance < p.dis2stop)
			p.dis2stop = p.nvFwd.distance;
		p.dis2stop /= 100;
	} else {
		p.nextLaneIndex = std::min<int>(p.currLaneIndex, vehicle->getNextSegment()->getLanes().size() - 1);
		if (vehicle->getNextSegment()->getLanes().at(p.nextLaneIndex)->is_pedestrian_lane()) {
			p.nextLaneIndex--;
			p.dis2stop = vehicle->getCurrPolylineLength() - vehicle->getDistanceMovedInSegment() + 1000;
		} else
			p.dis2stop = 1000;//defalut 1000m
	}

	//when vehicle stops, don't do lane changing
	if (vehicle->getVelocity() <= 0) {
		vehicle->setLatVelocity(0);
	}
	p.turningDirection = vehicle->getTurningDirection();

	//hard code , need solution


	//get nearest car, if not making lane changing, the nearest car should be the leading car in current lane.
	//if making lane changing, adjacent car need to be taken into account.
	NearestVehicle & nv = nearestVehicle(p);

//	if (p.nvFwd.exists())
//		p.space = p.nvFwd.distance;
//	else
//		p.space = 50;
//	if (nv.distance <= 0) {
//		//if (nv.driver->parent->getId() > this->parent->getId())
//		if (getDriverParent(nv.driver)->getId() > this->parent->getId()) {
//			nv = NearestVehicle();
//		}
//	}

	//this function make the issue Ticket #86
	perceivedDataProcess(nv, p);

	//bus approaching bus stop reduce speed
	//and if its left has lane, merge to left lane
	//if (isBusFarawayBusStop()) {
//	double acci = busAccelerating(p)*100;
//	vehicle->setAcceleration(acci);
	p.currSpeed = vehicle->getVelocity() / 100;
	double newFwdAcc = 0;
	newFwdAcc = cfModel->makeAcceleratingDecision(p, targetSpeed, maxLaneSpeed);
	vehicle->setAcceleration(newFwdAcc * 100);
	//}

//	//Retrieve a new acceleration value.
//		double newFwdAcc = 0;
//		//Convert back to m/s
//		//TODO: Is this always m/s? We should rename the variable then...
//		p.currSpeed = vehicle->getVelocity() / 100;
//		//Call our model
//		//Return the remaining amount (obtained by calling updatePositionOnLink)
//		newFwdAcc = cfModel->makeAcceleratingDecision(p, targetSpeed, maxLaneSpeed);
//		//Update our chosen acceleration; update our position on the link.
//		vehicle->setAcceleration(newFwdAcc * 100);

	//NOTE: Driver already has a lcModel; we should be able to just use this. ~Seth
	LANE_CHANGE_SIDE lcs = LCS_SAME;
	MITSIM_LC_Model* mitsim_lc_model = dynamic_cast<MITSIM_LC_Model*> (lcModel);
	if (mitsim_lc_model) {
		lcs = mitsim_lc_model->makeMandatoryLaneChangingDecision(p);
	} else {
		throw std::runtime_error("TODO: BusDrivers currently require the MITSIM lc model.");
	}

	vehicle->setTurningDirection(lcs);
	double newLatVel;
	newLatVel = lcModel->executeLaneChanging(p, vehicle->getAllRestRoadSegmentsLength(), vehicle->length, vehicle->getTurningDirection());
	vehicle->setLatVelocity(newLatVel * 10);

	if (isBusApproachingBusStop()) {
		double acc = busAccelerating(p)*100;

		//move to most left lane
		p.nextLaneIndex = vehicle->getCurrSegment()->getLanes().back()->getLaneID();
		LANE_CHANGE_SIDE lcs = mitsim_lc_model->makeMandatoryLaneChangingDecision(p);
		vehicle->setTurningDirection(lcs);
		double newLatVel;
		newLatVel = mitsim_lc_model->executeLaneChanging(p, vehicle->getAllRestRoadSegmentsLength(), vehicle->length, vehicle->getTurningDirection());
		vehicle->setLatVelocity(newLatVel*5);

		// reduce speed
		if (vehicle->getVelocity() / 100.0 > 2.0)
		{
			if (acc<-500.0)
			{
				vehicle->setAcceleration(acc);
			} else
				vehicle->setAcceleration(-500);
		}

		waitAtStopMS = 0;
	}

	if (isBusArriveBusStop() && waitAtStopMS >= 0 && waitAtStopMS < BUS_STOP_WAIT_PASSENGER_TIME_SEC) {

		if (vehicle->getVelocity() > 0)
			vehicle->setAcceleration(-5000);
		if (vehicle->getVelocity() < 0.1 && waitAtStopMS < BUS_STOP_WAIT_PASSENGER_TIME_SEC) {
			waitAtStopMS = waitAtStopMS + p.elapsedSeconds;

			//Pick up a semi-random number of passengers
			Bus* bus = dynamic_cast<Bus*>(vehicle);
			if ((waitAtStopMS == p.elapsedSeconds) && bus) {
				std::cout << "real_ArrivalTime value: " << real_ArrivalTime.get() << "  DwellTime_ijk: " << DwellTime_ijk.get() << std::endl;
				real_ArrivalTime.set(p.now.ms());// BusDriver set RealArrival Time, set once(the first time comes in)
				dwellTime_record = passengerGeneration(bus);
				DwellTime_ijk.set(dwellTime_record);
				//int pCount = reinterpret_cast<intptr_t> (vehicle) % 50;
				//bus->setPassengerCount(pCount);
			}
			if ((waitAtStopMS == p.elapsedSeconds * 2.0) && bus) {
				// 0.2sec, return and reset BUS_STOP_WAIT_PASSENGER_TIME_SEC
				// (no control: use dwellTime;
				// has Control: use DwellTime to calculate the holding strategy and return BUS_STOP_WAIT_PASSENGER_TIME_SEC
				if(BusController::HasBusControllers()) {
					//BusController::TEMP_Get_Bc_1()->receiveBusInformation("", 0, 0, p.now.ms());
					Person* person = dynamic_cast<Person*>(parent);
					if(person) {
						const BusTrip* bustrip = dynamic_cast<const BusTrip*>(person->currTripChainItem);
						if(bustrip && person->currTripChainItem->itemType==TripChainItem::IT_BUSTRIP) {
							const Busline* busline = bustrip->getBusline();
							if(busline) {
								if(busline->getControl_TimePointNum() == busstop_sequence_no.get()) { // only use holding control at selected time points
									double waitTime = 0;
									waitTime = BusController::TEMP_Get_Bc_1()->decisionCalculation(busline->getBusLineID(),bustrip->getBusTripRun_SequenceNum(),busstop_sequence_no.get(),real_ArrivalTime.get(),DwellTime_ijk.get(),curr_busStopRealTimes,lastVisited_BusStop.get(),0);
									setWaitTime_BusStop(waitTime);
								} else {
									setWaitTime_BusStop(DwellTime_ijk.get());// ignore the other BusStops, just use DwellTime
								}
							} else {
								std::cout << "Busline is nullptr, something is wrong!!! " << std::endl;
								setWaitTime_BusStop(DwellTime_ijk.get());
							}
						}
					}
				} else {
					setWaitTime_BusStop(DwellTime_ijk.get());
				}
			}
//			if(DwellTime_ijk.get() - waitAtStopMS > p.elapsedSeconds * 1.0) {
//				passengerCountOld_display_flag = true;
//			}
			if(waitAtStopMS >= dwellTime_record) {
				passengerCountOld_display_flag = false;
			} else {
				passengerCountOld_display_flag = true;
			}
		}
	}
//	else if (isBusArriveBusStop()) {
//		vehicle->setAcceleration(3000);
//	}

	if (isBusLeavingBusStop() || waitAtStopMS >= BUS_STOP_WAIT_PASSENGER_TIME_SEC) {
		std::cout << "BusDriver::updatePositionOnLink: bus isBusLeavingBusStop" << std::endl;
		waitAtStopMS = -1;
		BUS_STOP_WAIT_PASSENGER_TIME_SEC = 2;// reset when leaving bus stop
		//passengerCountOld_display_flag = false;

		vehicle->setAcceleration(busAccelerating(p)*100);
	}

	//Update our distance
	lastTickDistanceToBusStop = distanceToNextBusStop();

	DynamicVector segmentlength(vehicle->getCurrSegment()->getStart()->location.getX(),vehicle->getCurrSegment()->getStart()->location.getY(),
			vehicle->getCurrSegment()->getEnd()->location.getX(),vehicle->getCurrSegment()->getEnd()->location.getY());

	//Return the remaining amount (obtained by calling updatePositionOnLink)
	return updatePositionOnLink(p);
}

double sim_mob::BusDriver::getPositionX() const
{
	return vehicle ? vehicle->getX() : 0;
}

double sim_mob::BusDriver::getPositionY() const
{
	return vehicle ? vehicle->getY() : 0;
}


double sim_mob::BusDriver::busAccelerating(DriverUpdateParams& p)
{
	//Retrieve a new acceleration value.
	double newFwdAcc = 0;

	//Convert back to m/s
	//TODO: Is this always m/s? We should rename the variable then...
	p.currSpeed = vehicle->getVelocity() / 100;

	//Call our model
	newFwdAcc = cfModel->makeAcceleratingDecision(p, targetSpeed, maxLaneSpeed);

	return newFwdAcc;
	//Update our chosen acceleration; update our position on the link.
	//vehicle->setAcceleration(newFwdAcc * 100);
}

bool sim_mob::BusDriver::isBusFarawayBusStop()
{
	bool res = false;
	double distance = distanceToNextBusStop();
	if (distance < 0 || distance > 50)
		res = true;

	return res;
}

bool sim_mob::BusDriver::isBusApproachingBusStop()
{
	double distance = distanceToNextBusStop();
	if (distance >=10 && distance <= 50) {
		if (lastTickDistanceToBusStop < 0) {
			return true;
		} else if (lastTickDistanceToBusStop > distance) {
			return true;
		}
	}
	return false;
}

bool sim_mob::BusDriver::isBusArriveBusStop()
{
	double distance = distanceToNextBusStop();
	return  (distance>0 && distance <10);
}

bool sim_mob::BusDriver::isBusLeavingBusStop()
{
	double distance = distanceToNextBusStop();
	if (distance >=10 && distance < 50) {
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

double sim_mob::BusDriver::distanceToNextBusStop()
{
//	if (!vehicle->getCurrSegment() || !vehicle->hasNextSegment(true)) {
//		return -1;
//	}

	double distanceToCurrentSegmentBusStop = getDistanceToBusStopOfSegment(vehicle->getCurrSegment());
	double distanceToNextSegmentBusStop = -1;
	if (vehicle->hasNextSegment(true))
		distanceToNextSegmentBusStop = getDistanceToBusStopOfSegment(vehicle->getNextSegment(true));

	if (distanceToCurrentSegmentBusStop >= 0 && distanceToNextSegmentBusStop >= 0) {
		return ((distanceToCurrentSegmentBusStop<=distanceToNextSegmentBusStop) ? distanceToCurrentSegmentBusStop: distanceToNextSegmentBusStop);
	} else if (distanceToCurrentSegmentBusStop > 0) {
		return distanceToCurrentSegmentBusStop;
	}

	return distanceToNextSegmentBusStop;
}

void sim_mob::BusDriver::passengers_Board(Bus* bus)
{
	ConfigParams& config = ConfigParams::GetInstance();
	if(bus) {
		int manualID= -1;
		map<string, string> props;
		props["#mode"]="travel";
		props["#time"]="0";
		for (int no=0;no<no_passengers_boarding;no++)
		{
			//create passenger objects in the bus,bus has a list of passenger objects
			//Create the Person agent with that given ID (or an auto-generated one)
			Person* agent = new Person("XML_Def", config.mutexStategy, manualID);
			agent->setConfigProperties(props);
			agent->setStartTime(0);
			bus->passengers.push_back(agent);
		}
	}

}

void sim_mob::BusDriver::passengers_Alight(Bus* bus)
{
	//delete passenger objects in the bus
	if(bus) {
		for (int no=0;no<no_passengers_alighting;no++)
		{
			//delete passenger objects from the bus
			bus->passengers.pop_back();
		}
	}
}

double sim_mob::BusDriver::passengerGeneration(Bus* bus)
{
	double DTijk = 0.0;
	size_t no_passengers_bus = 0;
	size_t no_passengers_busstop = 0;
	ConfigParams& config = ConfigParams::GetInstance();
	PassengerDist* passenger_dist = config.passengerDist_busstop;
	//  PassengerDist* r2 = ConfigParams::GetInstance().passengerDist_crowdness;
	 //create the passenger objects at the bus stop=random no-boarding passengers
	if (bus && passenger_dist) {
		no_passengers_busstop = passenger_dist->getnopassengers();
		no_passengers_bus = bus->getPassengerCount();
		bus->setPassengerCountOld(no_passengers_bus);// record the old passenger number
		//if last busstop,only alighting
		if(last_busstop==true)
		{
			//no_passengers_alighting=(config.percent_alighting * 0.01)*no_passengers_bus;
			no_passengers_alighting=no_passengers_bus;// last bus stop, all alighting
			no_passengers_boarding=0;// reset boarding passengers to be zero at the last bus stop(for dwell time)
			passengers_Alight(bus);//alight the bus
			no_passengers_bus=no_passengers_bus - no_passengers_alighting;
			bus->setPassengerCount(no_passengers_bus);
			last_busstop = false;
		}
		//if first busstop,only boarding
		else if(first_busstop ==true)
		{
			no_passengers_boarding = (config.percent_boarding * 0.01)*no_passengers_busstop;
			if(no_passengers_boarding > bus->getBusCapacity() - no_passengers_bus)
				no_passengers_boarding = bus->getBusCapacity() - no_passengers_bus;
			passengers_Board(bus);//board the bus
			bus->setPassengerCount(no_passengers_bus+no_passengers_boarding);
			first_busstop= false;
		}

		//normal busstop,both boarding and alighting
		else
		{
			no_passengers_alighting=(config.percent_alighting * 0.01)*no_passengers_bus;
			passengers_Alight(bus);//alight the bus
			no_passengers_bus=no_passengers_bus - no_passengers_alighting;
			bus->setPassengerCount(no_passengers_bus);
			no_passengers_boarding=(config.percent_boarding * 0.01)*no_passengers_busstop;
			if(no_passengers_boarding> bus->getBusCapacity() - no_passengers_bus)
				no_passengers_boarding=bus->getBusCapacity() - no_passengers_bus;
			passengers_Board(bus);//board the bus
			bus->setPassengerCount(no_passengers_bus+no_passengers_boarding);
		}
		DTijk = dwellTimeCalculation(0,0,0,no_passengers_alighting,no_passengers_boarding,0,0,0,bus->getPassengerCount());
		return DTijk;
	} else {
		throw std::runtime_error("Passenger distributions have not been initialized yet.");
		return -1;
	}
}

double sim_mob::BusDriver::dwellTimeCalculation(int busline_i, int trip_k, int busstopSequence_j,int A,int B,int delta_bay,int delta_full,int Pfront,int no_of_passengers)
{
	double alpha1 = 0.5;
	double alpha2 = 0.5;
	double alpha3 = 0.5;
	double alpha4 = 0.5;

	double beta1 = 0.5;
	double beta2 = 0.5;
	double beta3 = 0.5;
	double DTijk = 0.0;
	//int Pfront = 1;
	bool bus_crowdness_factor;
	if(no_of_passengers>50)
		bus_crowdness_factor=1;
	else
		bus_crowdness_factor=0;
	double PTijk_front = alpha1 *Pfront*A + alpha2*B + alpha3*bus_crowdness_factor*B;
	double PTijk_rear = alpha4*(1-Pfront)*A;
	double PT;
	PT = std::max(PTijk_front,PTijk_rear);
	DTijk = beta1+PT+beta2*delta_bay+beta3*delta_full;
	return DTijk;
}

double sim_mob::BusDriver::getDistanceToBusStopOfSegment(const RoadSegment* rs)
{
	double distance = -100;
	double currentX = vehicle->getX();
	double currentY = vehicle->getY();

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
			bool isFound=false;
			for(i=0;i < busStops.size();++i)
			{
				if (bs->getBusstopno_() == busStops[i]->getBusstopno_())
				{
					isFound=true;
					busstop_sequence_no.set(i);
					lastVisited_BusStop.set(busStops[i]);
					break;
				}
			}
			if(isFound)
			{
				if (busstop_sequence_no.get() == (busStops.size() - 1)) // check whether it is the last bus stop in the busstop list
				{
					last_busstop = true;
				}
				if (rs == vehicle->getCurrSegment()) {

					if (stopPoint < 0) {
						throw std::runtime_error("BusDriver offset in obstacles list should never be <0");
					}

					if (stopPoint >= 0) {
						DynamicVector BusDistfromStart(vehicle->getX(),vehicle->getY(), rs->getStart()->location.getX(),rs->getStart()->location.getY());
						distance = stopPoint - vehicle->getDistanceMovedInSegment();
						break;
					}
				} else {
					DynamicVector busToSegmentStartDistance(currentX, currentY,rs->getStart()->location.getX(),rs->getStart()->location.getY());
					distance = vehicle->getCurrentSegmentLength() - vehicle->getDistanceMovedInSegment() + stopPoint;
				}
			} // end of if isFound
		}
	}

	return distance / 100.0;
}

//Main update functionality
void sim_mob::BusDriver::frame_tick(UpdateParams& p)
{
	//NOTE: If this is all that is doen, we can simply delete this function and
	//      let its parent handle it automatically. ~Seth
	Driver::frame_tick(p);
}

void sim_mob::BusDriver::frame_tick_output(const UpdateParams& p)
{
	//Skip?
	if (vehicle->isDone() || ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

#ifndef SIMMOB_DISABLE_OUTPUT
	double baseAngle = vehicle->isInIntersection() ? intModel->getCurrentAngle() : vehicle->getAngle();
	Bus* bus = dynamic_cast<Bus*>(vehicle);
	if(!passengerCountOld_display_flag) {
		LogOut("(\"BusDriver\""
				<<","<<p.now.frame()
				<<","<<parent->getId()
				<<",{"
				<<"\"xPos\":\""<<static_cast<int>(bus->getX())
				<<"\",\"yPos\":\""<<static_cast<int>(bus->getY())
				<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
				<<"\",\"length\":\""<<static_cast<int>(3*bus->length)
				<<"\",\"width\":\""<<static_cast<int>(2*bus->width)
				<<"\",\"passengers\":\""<<(bus?bus->getPassengerCount():0)
				<<"\",\"real_ArrivalTime\":\""<<(bus?real_ArrivalTime.get():0)
				<<"\",\"DwellTime_ijk\":\""<<(bus?DwellTime_ijk.get():0)
				<<"\"})"<<std::endl);
	} else {
		LogOut("(\"BusDriver\""
				<<","<<p.now.frame()
				<<","<<parent->getId()
				<<",{"
				<<"\"xPos\":\""<<static_cast<int>(bus->getX())
				<<"\",\"yPos\":\""<<static_cast<int>(bus->getY())
				<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
				<<"\",\"length\":\""<<static_cast<int>(3*bus->length)
				<<"\",\"width\":\""<<static_cast<int>(2*bus->width)
				<<"\",\"passengers\":\""<<(bus?bus->getPassengerCountOld():0)
				<<"\",\"real_ArrivalTime\":\""<<(bus?real_ArrivalTime.get():0)
				<<"\",\"DwellTime_ijk\":\""<<(bus?DwellTime_ijk.get():0)
				<<"\"})"<<std::endl);
	}

#endif
}

void sim_mob::BusDriver::frame_tick_output_mpi(timeslice now)
{
	//Skip output?
	if (now.frame()<parent->getStartTime() || vehicle->isDone()) {
		return;
	}

#ifndef SIMMOB_DISABLE_OUTPUT
	double baseAngle = vehicle->isInIntersection() ? intModel->getCurrentAngle() : vehicle->getAngle();

	//The BusDriver class will only maintain buses as the current vehicle.
	const Bus* bus = dynamic_cast<const Bus*>(vehicle);

	std::stringstream logout;
	if(!passengerCountOld_display_flag) {
		logout << "(\"Driver\"" << "," << now.frame() << "," << parent->getId() << ",{" << "\"xPos\":\""
				<< static_cast<int> (bus->getX()) << "\",\"yPos\":\"" << static_cast<int> (bus->getY())
				<< "\",\"segment\":\"" << bus->getCurrSegment()->getId()
				<< "\",\"angle\":\"" << (360 - (baseAngle * 180 / M_PI)) << "\",\"length\":\""
				<< static_cast<int> (bus->length) << "\",\"width\":\"" << static_cast<int> (bus->width)
				<<"\",\"passengers\":\""<<(bus?bus->getPassengerCount():0);
	} else {
		logout << "(\"Driver\"" << "," << now.frame() << "," << parent->getId() << ",{" << "\"xPos\":\""
				<< static_cast<int> (bus->getX()) << "\",\"yPos\":\"" << static_cast<int> (bus->getY())
				<< "\",\"segment\":\"" << bus->getCurrSegment()->getId()
				<< "\",\"angle\":\"" << (360 - (baseAngle * 180 / M_PI)) << "\",\"length\":\""
				<< static_cast<int> (bus->length) << "\",\"width\":\"" << static_cast<int> (bus->width)
				<<"\",\"passengers\":\""<<(bus?bus->getPassengerCountOld():0);
	}


	if (this->parent->isFake) {
		logout << "\",\"fake\":\"" << "true";
	} else {
		logout << "\",\"fake\":\"" << "false";
	}

	logout << "\"})" << std::endl;

	LogOut(logout.str());
#endif
}

vector<BufferedBase*> sim_mob::BusDriver::getSubscriptionParams() {
	vector<BufferedBase*> res;
	res = Driver::getSubscriptionParams();

	// BusDriver's features
	res.push_back(&(lastVisited_BusStop));
	res.push_back(&(real_DepartureTime));
	res.push_back(&(real_ArrivalTime));
	res.push_back(&(DwellTime_ijk));
	res.push_back(&(busstop_sequence_no));
	res.push_back(curr_busStopRealTimes);
	return res;
}

