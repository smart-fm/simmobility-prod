/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusDriver.hpp"

#include <vector>

#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"


using namespace sim_mob;
using std::vector;


namespace {

//Create a simple route via the following rules:
//   1) Every other road segment (starting with the second) has a
//      bus stop.
//   2) The first of these is located 30% down the road, the second is
//      60% down the road, and all others are 50% down the road.

BusRoute MakeSampleRoute(const vector<const RoadSegment*>& path)
{
	vector<DemoBusStop> res;
	for (size_t i=1; i<path.size(); i+=2) {
		DemoBusStop next;
		next.seg = path.at(i);
		next.percent = (i==1?0.3:i==3?0.6:0.5);
		res.push_back(next);
	}
	return BusRoute(res);
}

} //End anonymous namespace


sim_mob::BusDriver::BusDriver(Person* parent, MutexStrategy mtxStrat, unsigned int reacTime_LeadingVehicle, unsigned int reacTime_SubjectVehicle, unsigned int reacTime_Gap)
	: Driver(parent, mtxStrat, reacTime_LeadingVehicle, reacTime_SubjectVehicle, reacTime_Gap), nextStop(nullptr), waitAtStopMS(0.0)
{

}

void sim_mob::BusDriver::setRoute(const BusRoute& route)
{
	this->route = route;
	nextStop = route.getCurrentStop();
}


void sim_mob::BusDriver::frame_init(UpdateParams& p)
{
	//We're recreating the parent class's frame_init() method here.
	// Our goal is to reuse as much of Driver as possible, and then
	// refactor common code out later. We could just call the frame_init() method
	// directly, but there's some unexpected interdependencies.

	//Save the path, create a vehicle.
	initializePath();

	//initializePath() actually creates a Vehicle. We want Vehicle to be a "Bus",
	// so we need to recreate it here. This will require a new property, "route", which
	// contains the Bus route. For now, we can just generate a simple route.
	Vehicle* oldVehicle = vehicle;
	vehicle = new Bus(MakeSampleRoute(vehicle->getCompletePath()), vehicle);
	delete oldVehicle;

	//This code is used by Driver to set a few properties of the Vehicle/Bus.
	if (vehicle && vehicle->hasPath()) {
		setOrigin(params);
	} else {
		throw std::runtime_error("Vehicle could not be created for bus driver; no route!");
	}

	//Unique to BusDrivers: reset your route
	route.reset();
	nextStop = route.getCurrentStop();
	waitAtStopMS = 0.0;
}


double sim_mob::BusDriver::updatePositionOnLink(DriverUpdateParams& p)
{
	//First, have we just passed a bus stop? (You'll have to modify this to
	//   detect them in advance, but the concept's similar)
	bool atBusStop = nextStop && nextStop->atOrPastBusStop(vehicle->getCurrSegment(), vehicle->getDistanceMovedInSegment());

	//If we are moving, then (optionally) decelerate and call normal update behavior.
	if (vehicle->getVelocity()>0 || vehicle->getLatVelocity()>0) {
		if (atBusStop) {
			vehicle->setAcceleration(-300); //Force stop.
		}
		Driver::updatePositionOnLink(p);
	} else {
		//We're stopped. Is it for a bus stop? If so, set waitAtStopMS if it's not already set
		if (atBusStop && waitAtStopMS<=0.0) {
			waitAtStopMS = 10000; //Stop for 10s (temp)
		}
	}

}


//Main update functionality
void sim_mob::BusDriver::frame_tick(UpdateParams& p)
{
	//Call the parent's tick function
	Driver::frame_tick(p);

	//Driver::frame_tick() will move the Bus along its route.
	// If a Bus Stop has been reached, then a forced stop is performed,
	// and the variable "waitAtStopMS" is set to >0. This is where we react to it.
	if (waitAtStopMS>0.0) {
		waitAtStopMS = std::max(0.0, waitAtStopMS-p.currTimeMS);
		if (waitAtStopMS <= 0.0) {
			//Done waiting at the bus stop.
			Bus* bus = dynamic_cast<Bus*>(vehicle);
			if (!bus) {
				return; //TODO: Bus drivers should always have a Bus, by design...
			}

			//Pick up a semi-random number of passengers
			int pCount = reinterpret_cast<intptr_t>(bus) % 50;
			bus->setPassengerCount(pCount);

			//Advance your route. This will cause the Bus to start moving again.
			route.advance();
			nextStop = route.getCurrentStop();
		}
	}
}

void sim_mob::BusDriver::frame_tick_output(const UpdateParams& p)
{
	//Skip?
	if (vehicle->isDone() || ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

	//Vehicle should be a Bus
	const Bus* bus = dynamic_cast<const Bus*>(vehicle);

	double baseAngle = vehicle->isInIntersection() ? intModel->getCurrentAngle() : vehicle->getAngle();

#ifndef SIMMOB_DISABLE_OUTPUT
	LogOut("(\"BusDriver\""
			<<","<<p.frameNumber
			<<","<<parent->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(vehicle->getX())
			<<"\",\"yPos\":\""<<static_cast<int>(vehicle->getY())
			<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
			<<"\",\"length\":\""<<static_cast<int>(vehicle->length)
			<<"\",\"width\":\""<<static_cast<int>(vehicle->width)
			<<"\",\"passengers\":\""<<(bus?bus->getPassengerCount():0)
			<<"\"})"<<std::endl);
#endif
}

void sim_mob::BusDriver::frame_tick_output_mpi(frame_t frameNumber)
{
	//Skip output?
	if (frameNumber<parent->getStartTime() || vehicle->isDone()) {
		return;
	}

#ifndef SIMMOB_DISABLE_OUTPUT
	double baseAngle = vehicle->isInIntersection() ? intModel->getCurrentAngle() : vehicle->getAngle();

	//Vehicle should be a Bus
	const Bus* bus = dynamic_cast<const Bus*>(vehicle);

	std::stringstream logout;
	logout << "(\"Driver\"" << "," << frameNumber << "," << parent->getId() << ",{" << "\"xPos\":\""
			<< static_cast<int> (vehicle->getX()) << "\",\"yPos\":\"" << static_cast<int> (vehicle->getY())
			<< "\",\"angle\":\"" << (360 - (baseAngle * 180 / M_PI)) << "\",\"length\":\""
			<< static_cast<int> (vehicle->length) << "\",\"width\":\"" << static_cast<int> (vehicle->width)
			<<"\",\"passengers\":\""<<(bus?bus->getPassengerCount():0);

	if (this->parent->isFake) {
		logout << "\",\"fake\":\"" << "true";
	} else {
		logout << "\",\"fake\":\"" << "false";
	}

	logout << "\"})" << std::endl;

	LogOut(logout.str());
#endif
}

