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
	vector<BusStop> res;
	for (size_t i=1; i<path.size(); i+=2) {
		BusStop next;
		next.seg = path.at(i);
		next.percent = (i==1?0.3:i==3?0.6:0.5);
		res.push_back(next);
	}
	return BusRoute(res);
}

} //End anonymous namespace


sim_mob::BusDriver::BusDriver(Person* parent, MutexStrategy mtxStrat, unsigned int reacTime_LeadingVehicle, unsigned int reacTime_SubjectVehicle, unsigned int reacTime_Gap)
	: Driver(parent, mtxStrat, reacTime_LeadingVehicle, reacTime_SubjectVehicle, reacTime_Gap), nextStop(nullptr)
{
}

void sim_mob::BusDriver::setRoute(const BusRoute& route)
{
	route = route;
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
}

//Main update functionality
void sim_mob::BusDriver::frame_tick(UpdateParams& p)
{
/*	DriverUpdateParams& p2 = dynamic_cast<DriverUpdateParams&>(p);

	//Are we done already?
	if (vehicle->isDone()) {
		parent->setToBeRemoved();
		return;
	}

	//Just a bit glitchy...
	updateAdjacentLanes(p2);

	//retrieved their current "sensed" values.
	if (perceivedVelocity.can_sense(p.currTimeMS)) {
		p2.perceivedFwdVelocity = perceivedVelocity.sense(p.currTimeMS,0)->x;
		p2.perceivedLatVelocity = perceivedVelocity.sense(p.currTimeMS,0)->y;
	}

	//General update behavior.
	//Note: For now, most updates cannot take place unless there is a Lane and vehicle.
	if (p2.currLane && vehicle) {

		if (update_sensors(p2, p.frameNumber) && update_movement(p2, p.frameNumber) && update_post_movement(p2, p.frameNumber)) {

			//Update parent data. Only works if we're not "done" for a bad reason.
			setParentBufferedData();
		}
	}


	//Update our Buffered types
	//TODO: Update parent buffered properties, or perhaps delegate this.
	//	currLane_.set(params.currLane);
	//	currLaneOffset_.set(params.currLaneOffset);
	//	currLaneLength_.set(params.currLaneLength);
	if (!vehicle->isInIntersection()) {
		currLane_.set(vehicle->getCurrLane());
		currLaneOffset_.set(vehicle->getDistanceMovedInSegment());
		currLaneLength_.set(vehicle->getCurrLinkLaneZeroLength());
	}

	isInIntersection.set(vehicle->isInIntersection());
	//Update your perceptions
	perceivedVelocity.delay(new DPoint(vehicle->getVelocity(), vehicle->getLatVelocity()), p.currTimeMS);
	//Print output for this frame.*/
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

