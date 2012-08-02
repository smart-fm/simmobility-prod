/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusDriver.hpp"
#include <vector>

#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "util/DebugFlags.hpp"
using namespace sim_mob;
using std::vector;
using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;

namespace {

const int BUS_STOP_WAIT_PASSENGER_TIME_SEC = 2;


//Create a simple route via the following rules:
//   1) Every other road segment (starting with the first) has a
//      bus stop.
//   2) The first of these is located 30% down the road, the second is
//      60% down the road, and all others are 50% down the road.
DemoBusStop newbs;


} //End anonymous namespace

sim_mob::BusDriver::BusDriver(Person* parent, MutexStrategy mtxStrat)
	: Driver(parent, mtxStrat), nextStop(nullptr), waitAtStopMS(-1) , lastTickDistanceToBusStop(-1)
{
	//boost::mt19937 gen;
	//myDriverUpdateParams = new DriverUpdateParams(gen);
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
	BusRoute nullRoute; //Buses don't use the route at the moment.
	Bus* bus = new Bus(nullRoute, vehicle);
	busStops = findBusStopInPath(bus->getCompletePath());
	delete vehicle;
	vehicle = bus;


	//This code is used by Driver to set a few properties of the Vehicle/Bus.
	if (bus && bus->hasPath()) {
		setOrigin(params);
	} else {
		throw std::runtime_error("Vehicle could not be created for bus driver; no route!");
	}

	//Unique to BusDrivers: reset your route
//	bus->getRoute().reset();
//	nextStop = bus->getRoute().getCurrentStop();
//	stops = bus->getRoute().getStops();
//	std::cout<<"NextStopis   "<<bus->getRoute().getCurrentStop()->percent<<std::endl;
	waitAtStopMS = 0.0;

}

vector<BusStop*> sim_mob::BusDriver::findBusStopInPath(const vector<const RoadSegment*>& path) const
{
	//NOTE: Use typedefs instead of defines.
	typedef vector<BusStop*> BusStopVector;

	BusStopVector res;
	int busStopAmount = 0;
	vector<const RoadSegment*>::const_iterator it;
	for( it = path.begin(); it != path.end(); ++it)
	{
		const RoadSegment* rs = (*it);
		// get obstacles in road segment
		const std::map<centimeter_t, const RoadItem*> & obstacles = rs->obstacles;
		std::map<centimeter_t, const RoadItem*>::const_iterator ob_it;
		for(ob_it = obstacles.begin(); ob_it != obstacles.end(); ++ob_it)
		{
			RoadItem* ri = const_cast<RoadItem*>(ob_it->second);
			BusStop *bs = dynamic_cast<BusStop *>(ri);
			if(bs)
			{
				DynamicVector SegmentLength(rs->getEnd()->location.getX(),rs->getEnd()->location.getY(),rs->getStart()->location.getX(),rs->getStart()->location.getY());
				DynamicVector BusStopDistfromStart(bs->xPos,bs->yPos,rs->getStart()->location.getX(),rs->getStart()->location.getY());
				DynamicVector BusStopDistfromEnd(rs->getEnd()->location.getX(),rs->getEnd()->location.getY(),bs->xPos,bs->yPos);
				double a = BusStopDistfromStart.getMagnitude();
				double b = BusStopDistfromEnd.getMagnitude();
				double c = SegmentLength.getMagnitude();
				bs->stopPoint = (-b*b + a*a + c*c)/(2.0*c);
				std::cout<<"stopPoint: "<<bs->stopPoint/100.0<<std::endl;
				// find bus stop in this segment
				res.push_back(bs);
			}
		}
	}
	return res;
}
double sim_mob::BusDriver::linkDriving(DriverUpdateParams& p) {

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
	//
	//	if (p.nextLaneIndex >= p.currLane->getRoadSegment()->getLanes().size())
	//		p.nextLaneIndex = p.currLaneIndex;

	//when vehicle stops, don't do lane changing
	if (vehicle->getVelocity() <= 0) {
		vehicle->setLatVelocity(0);
	}
	p.turningDirection = vehicle->getTurningDirection();

	//hard code , need solution
	p.space = 50;

	//get nearest car, if not making lane changing, the nearest car should be the leading car in current lane.
	//if making lane changing, adjacent car need to be taken into account.
	NearestVehicle & nv = nearestVehicle(p);
	if (nv.distance <= 0) {
		//if (nv.driver->parent->getId() > this->parent->getId())
		if (getDriverParent(nv.driver)->getId() > this->parent->getId()) {
			nv = NearestVehicle();
		}
	}

	perceivedDataProcess(nv, p);

	//bus approaching bus stop reduce speed
	//and if its left has lane, merge to left lane
	if (isBusFarawayBusStop()) {
		busAccelerating(p);
	}

	//set lateral velocity
	//NOTE: myDriverUpdateParams simply copies p, so we should just be able to use p. ~Seth
	//p.nextLaneIndex = myDriverUpdateParams->nextLaneIndex;
	p.nextLaneIndex = p.nextLaneIndex;

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
	std::cout << "BusDriver::updatePositionOnLink:current lane: "
			  << p.currLaneIndex << " lat velo: " << newLatVel / 100.0 << "m/s"
			  << std::endl;
	if (isBusApproachingBusStop()) {
		busAccelerating(p);

		//move to most left lane
		p.nextLaneIndex = vehicle->getCurrSegment()->getLanes().back()->getLaneID();
		//		LANE_CHANGE_SIDE lcs = mitsim_lc_model->makeMandatoryLaneChangingDecision(p);
		//		bus->setTurningDirection(lcs);
		//		double newLatVel;
		//		newLatVel = lcModel->executeLaneChanging(p, bus->getAllRestRoadSegmentsLength(), bus->length, bus->getTurningDirection());
		//		bus->setLatVelocity(newLatVel);

		std::cout << "BusDriver::updatePositionOnLink: bus approaching current lane: "
				  << p.currLaneIndex << std::endl;

		// reduce speed
		if (vehicle->getVelocity() / 100.0 > 10)
			vehicle->setAcceleration(-500);

		//		//Check if we should change lanes.
		//		if (p.leftLane) //has lane in left?
		//		{
		//			//Lateral movement
		//			if (!bus->isInIntersection()) {
		//				bus->setTurningDirection(LCS_LEFT);
		//				bus->setLatVelocity(1000);
		//
		//				double newLatVel;
		//				newLatVel = lcModel->executeLaneChanging(p, vehicle->getAllRestRoadSegmentsLength(), vehicle->length,
		//							vehicle->getTurningDirection());
		//				vehicle->setLatVelocity(newLatVel);
		//				if(vehicle->getLatVelocity()>0)
		//					vehicle->setTurningDirection(LCS_LEFT);
		//				else if(vehicle->getLatVelocity()<0)
		//					vehicle->setTurningDirection(LCS_RIGHT);
		//				else
		//					vehicle->setTurningDirection(LCS_SAME);
		//
		//
		//			}
		//		} else {
		//			bus->setLatVelocity(0);
		//			bus->setTurningDirection(LCS_SAME);
		//			std::cout<<"BusDriver::updatePositionOnLink: no more left lane"<<std::endl;
		//		}
		waitAtStopMS = 0;
	}
	if (isBusArriveBusStop() && waitAtStopMS >= 0 && waitAtStopMS
			< BUS_STOP_WAIT_PASSENGER_TIME_SEC) {
		std::cout
				<< "BusDriver::updatePositionOnLink: bus isBusArriveBusStop velocity: "
				<< vehicle->getVelocity() / 100.0 << std::endl;
		//		bus->setLatVelocity(0);
		if (vehicle->getVelocity() > 0)
			vehicle->setAcceleration(-5000);
		if (vehicle->getVelocity() < 0.1 && waitAtStopMS < BUS_STOP_WAIT_PASSENGER_TIME_SEC) {
			waitAtStopMS = waitAtStopMS + p.elapsedSeconds;
			std::cout << "BusDriver::updatePositionOnLink: waitAtStopMS: " << waitAtStopMS << " p.elapsedSeconds: " << p.elapsedSeconds << std::endl;

			//Pick up a semi-random number of passengers
			Bus* bus = dynamic_cast<Bus*>(vehicle);
			if ((waitAtStopMS == p.elapsedSeconds) && bus) {
				std::cout << "BusDriver::updatePositionOnLink: pich up passengers" << std::endl;
				int pCount = reinterpret_cast<intptr_t> (vehicle) % 50;
				bus->setPassengerCount(pCount);
			}
		}
	} else if (isBusArriveBusStop()) {
		vehicle->setAcceleration(3000);
	}

	//TODO: Please check from here; the merge was not 100% clean. ~Seth
	if (isBusLeavingBusStop() || waitAtStopMS >= BUS_STOP_WAIT_PASSENGER_TIME_SEC) {
		std::cout << "BusDriver::updatePositionOnLink: bus isBusLeavingBusStop" << std::endl;
		waitAtStopMS = -1;
	}

	double fwdDistance = vehicle->getVelocity() * p.elapsedSeconds + 0.5 * vehicle->getAcceleration() * p.elapsedSeconds * p.elapsedSeconds;
	if (fwdDistance < 0)
		fwdDistance = 0;

	//double fwdDistance = vehicle->getVelocity()*p.elapsedSeconds;
	double latDistance = vehicle->getLatVelocity() * p.elapsedSeconds;

	//Increase the vehicle's velocity based on its acceleration.
	busAccelerating(p);

	double rest = updatePositionOnLink(p);
	//	myDriverUpdateParams->currLaneIndex = p.currLaneIndex;
	//	myDriverUpdateParams->nextLaneIndex = p.nextLaneIndex;
	//*myDriverUpdateParams = p;
	return rest;
}

double sim_mob::BusDriver::getPositionX() const
{
	if (this->vehicle)
		return this->vehicle->getX();
	return 0;
}

double sim_mob::BusDriver::getPositionY() const
{
	if (this->vehicle)
		return this->vehicle->getY();
	return 0;
}


void sim_mob::BusDriver::busAccelerating(DriverUpdateParams& p)
{
	//Retrieve a new acceleration value.
	double newFwdAcc = 0;

	//Convert back to m/s
	//TODO: Is this always m/s? We should rename the variable then...
	p.currSpeed = vehicle->getVelocity() / 100;
	//Call our model


	newFwdAcc = cfModel->makeAcceleratingDecision(p, targetSpeed, maxLaneSpeed);

	//Update our chosen acceleration; update our position on the link.
	vehicle->setAcceleration(newFwdAcc * 100);
}
bool sim_mob::BusDriver::isBusFarawayBusStop() const
{
	bool res = false;
	double distance = distanceToNextBusStop();
	if (distance < 0 || distance > 50)
		res = true;

	return res;
}

bool sim_mob::BusDriver::isBusApproachingBusStop() const
{
	double distance = distanceToNextBusStop();
	//std::cout<<"BusDriver::updatePositionOnLink: bus stop distance <"<<distance<<"> m"<<std::endl;
	//std::cout<<"BusDriver::updatePositionOnLink: lastTickDistanceToBusStop <"<<lastTickDistanceToBusStop<<"> m"<<std::endl;
	if (distance >=10 && distance <= 50)
	{
		if (lastTickDistanceToBusStop < 0)
		{
//			lastTickDistanceToBusStop = distance;
			return true;
		}
		else if (lastTickDistanceToBusStop > distance)
		{
//			lastTickDistanceToBusStop = distance;
			return true;
		}
	}
//	lastTickDistanceToBusStop = distance;
	return false;
}

bool sim_mob::BusDriver::isBusArriveBusStop() const
{
	double distance = distanceToNextBusStop();
	if (distance>0 && distance <10)
	{
		return true;
	}

	return false;
}

bool sim_mob::BusDriver::isBusLeavingBusStop() const
{
	double distance = distanceToNextBusStop();
//	std::cout<<"BusDriver::isBusLeavingBusStop: bus stop distance <"<<distance<<"> m"<<std::endl;
//	std::cout<<"BusDriver::isBusLeavingBusStop: lastTickDistanceToBusStop <"<<lastTickDistanceToBusStop<<"> m"<<std::endl;
	if (distance >=10 && distance < 50)
	{
//		std::cout<<"BusDriver::isBusLeavingBusStop: bus stop coming "<<std::endl;
		if (distance < 0)
		{
			lastTickDistanceToBusStop = distance;
			return true;
		}
		else if (lastTickDistanceToBusStop < distance)
		{
			lastTickDistanceToBusStop = distance;
			return true;
		}
	}

	lastTickDistanceToBusStop = distance;
	return false;
}
double sim_mob::BusDriver::distanceToNextBusStop() const
{
	double distanceToCurrentSegmentBusStop = -1;
	double distanceToNextSegmentBusStop = -1;
	const RoadSegment* rsCurrent = vehicle->getCurrSegment();
	const RoadSegment* rsNext = vehicle->getNextSegment(true);

	if (rsCurrent)
		distanceToCurrentSegmentBusStop = getDistanceToBusStopOfSegment(*rsCurrent);
	if (rsNext)
		distanceToNextSegmentBusStop = getDistanceToBusStopOfSegment(*rsNext);

	if (distanceToCurrentSegmentBusStop >= 0 && distanceToNextSegmentBusStop >= 0)
		return ((distanceToCurrentSegmentBusStop<=distanceToNextSegmentBusStop) ? distanceToCurrentSegmentBusStop: distanceToNextSegmentBusStop);
	else if (distanceToCurrentSegmentBusStop > 0)
		return distanceToCurrentSegmentBusStop;
	else
		return distanceToNextSegmentBusStop;
}
double sim_mob::BusDriver::getDistanceToBusStopOfSegment(const RoadSegment& roadSegment) const
{
	const RoadSegment* rs = &roadSegment;

	double distance = -100;
	double currentX = getPositionX();
	double currentY = getPositionY();
//	bus->getRoute().getCurrentStop();

		std::cout.precision(10);
//		std::cout<<"BusDriver::DistanceToNextBusStop : current bus position: "<<currentX<<"  "<<currentY<<std::endl;
//		std::cout<<"BusDriver::DistanceToNextBusStop : seg start: "<<rs->getStart()->location.getX()<<"  "
//		        		<<rs->getStart()->location.getY()<<" rs end: "<<rs->getEnd()->location.getX()<<"  "
//		        		<<rs->getEnd()->location.getY()<<std::endl;
		const std::map<centimeter_t, const RoadItem*> & obstacles = rs->obstacles;
//		int i = 1;
		for(std::map<centimeter_t, const RoadItem*>::const_iterator o_it = obstacles.begin(); o_it != obstacles.end() ; o_it++)
		{
		   RoadItem* ri = const_cast<RoadItem*>(o_it->second);
		   BusStop *bs = dynamic_cast<BusStop *>(ri);
		   if(bs)
		   {
			   if (roadSegment == vehicle->getCurrSegment())
			   {

				   if (bs->stopPoint < 0)
				   {
					   DynamicVector SegmentLength(rs->getEnd()->location.getX(),rs->getEnd()->location.getY(),rs->getStart()->location.getX(),rs->getStart()->location.getY());
					   DynamicVector BusStopDistfromStart(bs->xPos,bs->yPos,rs->getStart()->location.getX(),rs->getStart()->location.getY());
					   DynamicVector BusStopDistfromEnd(rs->getEnd()->location.getX(),rs->getEnd()->location.getY(),bs->xPos,bs->yPos);
					   double a = BusStopDistfromStart.getMagnitude();
					   double b = BusStopDistfromEnd.getMagnitude();
					   double c = SegmentLength.getMagnitude();
					   bs->stopPoint = (-b*b + a*a + c*c)/(2.0*c);
				   }

				   if (bs->stopPoint >= 0)
				   {
						DynamicVector BusDistfromStart(getPositionX(), getPositionY(),rs->getStart()->location.getX(),rs->getStart()->location.getY());
						std::cout<<"BusDriver::DistanceToNextBusStop: bus move in segment: "<<BusDistfromStart.getMagnitude()<<std::endl;
						distance = bs->stopPoint - BusDistfromStart.getMagnitude();
					}
			   }
			   else
			   {
				   DynamicVector busToSegmentStartDistance(currentX,currentY,
						   rs->getStart()->location.getX(),rs->getStart()->location.getY());
				   distance = busToSegmentStartDistance.getMagnitude() + bs->stopPoint;
			   }

////			   std::cout<<"BusDriver::DistanceToNextBusStop: find bus stop <"<<i<<"> in segment"<<std::endl;
//			   double busStopX = bs->xPos;
//			   double busStopY = bs->yPos;
////				std::cout<<"BusDriver::DistanceToNextBusStop : bus stop position: "<<busStopX<<"  "<<busStopY<<std::endl;
//			   double dis = sqrt((currentX-busStopX)*(currentX-busStopX) + (currentY-busStopY)*(currentY-busStopY));
//			   if (distance < 0 || dis < distance) // in case more than one stop at the segment
//				   distance = dis;
////				std::cout<<"BusDriver::DistanceToNextBusStop : distance: "<<distance/100.0<<std::endl;
//			   i++;
		   }
		}

		return distance/100.0;
}
//Main update functionality
void sim_mob::BusDriver::frame_tick(UpdateParams& p)
{
	//Call the parent's tick function
	Driver::frame_tick(p);

	//Driver::frame_tick() will move the Bus along its route.
	// If a Bus Stop has been reached, then a forced stop is performed,
	// and the variable "waitAtStopMS" is set to >0. This is where we react to it.
//	if (waitAtStopMS>0.0) {
//		if (p.currTimeMS-waitAtStopMS >= 10000) { //10s
//			waitAtStopMS = 0.0;
//		}
//		if (waitAtStopMS == 0.0) {
//			//Done waiting at the bus stop.
//			//Bus* bus = dynamic_cast<Bus*>(vehicle);
//			if (!bus) {
//				return; //TODO: Bus drivers should always have a Bus, by design...
//			}
//
//			//Pick up a semi-random number of passengers
//			int pCount = reinterpret_cast<intptr_t>(bus) % 50;
//			bus->setPassengerCount(pCount);
//
//			//Advance your route. This will cause the Bus to start moving again.
//			bus->getRoute().advance();
//			nextStop = bus->getRoute().getCurrentStop();
//		}
//	}
}

void sim_mob::BusDriver::frame_tick_output(const UpdateParams& p)
{
	//Skip?
	if (vehicle->isDone() || ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

	//Vehicle should be a Bus
	//const Bus* bus = dynamic_cast<const Bus*>(vehicle);

	double baseAngle = vehicle->isInIntersection() ? intModel->getCurrentAngle() : vehicle->getAngle();

#ifndef SIMMOB_DISABLE_OUTPUT
	Bus* bus = dynamic_cast<Bus*>(vehicle);
	LogOut("(\"BusDriver\""
			<<","<<p.frameNumber
			<<","<<parent->getId()
			//<<","<<bus->getCurrSegment()->obstacles.size()<<"+"<<bus->getCurrSegment()->getId()

			//<<"distance  "<<static_cast<int>(bus->getDistanceMovedInSegment())/(bus->getCurrLinkLaneZeroLength())
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(bus->getX())
			<<"\",\"yPos\":\""<<static_cast<int>(bus->getY())
			<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
			<<"\",\"length\":\""<<static_cast<int>(3*bus->length)
			<<"\",\"width\":\""<<static_cast<int>(2*bus->width)
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

	//The BusDriver class will only maintain buses as the current vehicle.
	const Bus* bus = dynamic_cast<const Bus*>(vehicle);

	std::stringstream logout;
	logout << "(\"Driver\"" << "," << frameNumber << "," << parent->getId() << ",{" << "\"xPos\":\""
			<< static_cast<int> (bus->getX()) << "\",\"yPos\":\"" << static_cast<int> (bus->getY())
			<< "\",\"segment\":\"" << bus->getCurrSegment()->getId()
			<< "\",\"angle\":\"" << (360 - (baseAngle * 180 / M_PI)) << "\",\"length\":\""
			<< static_cast<int> (bus->length) << "\",\"width\":\"" << static_cast<int> (bus->width)
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

