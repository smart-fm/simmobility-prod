/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusDriver.hpp"


using namespace sim_mob;



sim_mob::BusDriver::BusDriver(Person* parent, MutexStrategy mtxStrat, unsigned int reacTime_LeadingVehicle, unsigned int reacTime_SubjectVehicle, unsigned int reacTime_Gap)
	: Driver(parent, mtxStrat, reacTime_LeadingVehicle, reacTime_SubjectVehicle, reacTime_Gap), nextStop(nullptr)
{
}

void sim_mob::BusDriver::setRoute(const BusRoute& route)
{
	route = route;
	nextStop = route.getCurrentStop();
}


void sim_mob::Driver::frame_init(UpdateParams& p)
{
/*	//Save the path from orign to destination in allRoadSegments
	initializePath();

	//Set some properties about the current path, such as the current polyline, etc.
	if (vehicle && vehicle->hasPath()) {
		setOrigin(params);
	} else {
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout << "ERROR: Vehicle could not be created for driver; no route!\n";
#endif
	}*/
}

//Main update functionality
void sim_mob::Driver::frame_tick(UpdateParams& p)
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

void sim_mob::Driver::frame_tick_output(const UpdateParams& p)
{
/*	//Skip?
	if (vehicle->isDone() || ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

	double baseAngle = vehicle->isInIntersection() ? intModel->getCurrentAngle() : vehicle->getAngle();

#ifndef SIMMOB_DISABLE_OUTPUT
	LogOut("(\"Driver\""
			<<","<<p.frameNumber
			<<","<<parent->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(vehicle->getX())
			<<"\",\"yPos\":\""<<static_cast<int>(vehicle->getY())
			<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
			<<"\",\"length\":\""<<static_cast<int>(vehicle->length)
			<<"\",\"width\":\""<<static_cast<int>(vehicle->width)
			<<"\"})"<<std::endl);
#endif*/
}

void sim_mob::Driver::frame_tick_output_mpi(frame_t frameNumber)
{
/*	if (frameNumber < parent->getStartTime())
		return;

	if (vehicle->isDone())
		return;

#ifndef SIMMOB_DISABLE_OUTPUT
	double baseAngle = vehicle->isInIntersection() ? intModel->getCurrentAngle() : vehicle->getAngle();
	std::stringstream logout;

	logout << "(\"Driver\"" << "," << frameNumber << "," << parent->getId() << ",{" << "\"xPos\":\""
			<< static_cast<int> (vehicle->getX()) << "\",\"yPos\":\"" << static_cast<int> (vehicle->getY())
			<< "\",\"angle\":\"" << (360 - (baseAngle * 180 / M_PI)) << "\",\"length\":\""
			<< static_cast<int> (vehicle->length) << "\",\"width\":\"" << static_cast<int> (vehicle->width);

	if (this->parent->isFake) {
		logout << "\",\"fake\":\"" << "true";
	} else {
		logout << "\",\"fake\":\"" << "false";
	}

	logout << "\"})" << std::endl;

	LogOut(logout.str());
#endif*/
}

