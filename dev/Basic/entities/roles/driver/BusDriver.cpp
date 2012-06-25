/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusDriver.hpp"

#include <vector>

#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"


using namespace sim_mob;
using std::vector;


namespace {

//Create a simple route via the following rules:
//   1) Every other road segment (starting with the first) has a
//      bus stop.
//   2) The first of these is located 30% down the road, the second is
//      60% down the road, and all others are 50% down the road.

BusRoute MakeSampleRoute(const vector<const RoadSegment*>& path)
{
	vector<DemoBusStop> res;
	for (size_t i=0; i<path.size(); i+=2) {
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

/*void sim_mob::BusDriver::setRoute(const BusRoute& route)
{
	this->route = route;
	nextStop = this->route.getCurrentStop();
}*/


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
	Bus* bus = new Bus(MakeSampleRoute(vehicle->getCompletePath()), vehicle);
	delete vehicle;
	vehicle = bus;

	//This code is used by Driver to set a few properties of the Vehicle/Bus.
	if (vehicle && vehicle->hasPath()) {
		setOrigin(params);
	} else {
		throw std::runtime_error("Vehicle could not be created for bus driver; no route!");
	}

	//Unique to BusDrivers: reset your route
	bus->getRoute().reset();
	nextStop = bus->getRoute().getCurrentStop();
	waitAtStopMS = 0.0;
}


double sim_mob::BusDriver::updatePositionOnLink(DriverUpdateParams& p)
{
	//First, have we just passed a bus stop? (You'll have to modify this to
	//   detect them in advance, but the concept's similar)
	bool atBusStop = nextStop && nextStop->atOrPastBusStop(vehicle->getCurrSegment(), vehicle->getDistanceMovedInSegment());
	bool updatePos = false;

	//If we are moving, then (optionally) decelerate and call normal update behavior.
	if (vehicle->getVelocity()>0 || vehicle->getLatVelocity()>0) {
		if (atBusStop) {
			//vehicle->setAcceleration(-300); //Force stop.
			vehicle->setAcceleration(0);
			vehicle->setVelocity(0); //TEMP: Need to really force it.
			waitAtStopMS = p.currTimeMS; //TEMP: Need to force this too.
		}
		updatePos = true;
	} else {
		//We're stopped. Is it for a bus stop? If so, set waitAtStopMS if it's not already set
		if (atBusStop) {
			if (waitAtStopMS==0.0) {
				waitAtStopMS = p.currTimeMS;
			}
		} else {
			updatePos = true;
		}
	}

	//Move
	double res = 0.0;
	if (updatePos) {
		res = Driver::updatePositionOnLink(p);
	}
	
	return res;
}


//Main update functionality
void sim_mob::BusDriver::frame_tick(UpdateParams& p)
{
	//Call the parent's tick function
	Driver::frame_tick(p);

	std::cout<<"BusDriver ID:-->"<<this->getParent()->getId()<<std::endl;
	std::cout<<"===========  ("<<(this->getVehicle()->getPosition().x)/1000<<","<<(this->getVehicle()->getPosition().y)/1000<<"=========== "<<std::endl;
	DPoint pt = Driver::getVehicle()->getPosition();
	DPoint ptCheck(37223035, 14331504);
	double distance = dist(pt.x, pt.y, ptCheck.x, ptCheck.y);
	BusController& busctrller = BusController::getInstance();
	if(this->getParent()->getId() == 3 && distance < 1)// check id and distance
	{
		std::cout<<"distance == "<<distance<<std::endl;
		busctrller.updateBusInformation(pt);//communication and updateupdateBusInformation
	}

	//Driver::frame_tick() will move the Bus along its route.
	// If a Bus Stop has been reached, then a forced stop is performed,
	// and the variable "waitAtStopMS" is set to >0. This is where we react to it.
	if (waitAtStopMS>0.0) {
		if (p.currTimeMS-waitAtStopMS >= 10000) { //10s
			waitAtStopMS = 0.0;
		}
		if (waitAtStopMS == 0.0) {
			//Done waiting at the bus stop.
			Bus* bus = dynamic_cast<Bus*>(vehicle);
			if (!bus) {
				return; //TODO: Bus drivers should always have a Bus, by design...
			}

			//Pick up a semi-random number of passengers
			int pCount = reinterpret_cast<intptr_t>(bus) % 50;
			bus->setPassengerCount(pCount);

			//Advance your route. This will cause the Bus to start moving again.
			bus->getRoute().advance();
			nextStop = bus->getRoute().getCurrentStop();
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

