/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusDriver.hpp"
#include <vector>

#include "DriverUpdateParams.hpp"

#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "util/DebugFlags.hpp"

using namespace sim_mob;
using std::vector;
using std::map;
using std::string;

namespace {
const int BUS_STOP_WAIT_PASSENGER_TIME_SEC = 2;
} //End anonymous namespace

sim_mob::BusDriver::BusDriver(Person* parent, MutexStrategy mtxStrat)
	: Driver(parent, mtxStrat), nextStop(nullptr), waitAtStopMS(-1) , lastTickDistanceToBusStop(-1)
{
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
	Vehicle* newVeh = initializePath(true);

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
		busStops = findBusStopInPath(vehicle->getCompletePath());

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
			const BusStop *bs = dynamic_cast<const BusStop*>(ob_it->second);
			if(bs) {
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
		std::cout << "BusDriver::updatePositionOnLink: bus approaching current lane: "
				  << p.currLaneIndex << std::endl;

		// reduce speed
		if (vehicle->getVelocity() / 100.0 > 10)
			vehicle->setAcceleration(-500);

		waitAtStopMS = 0;
	}
	if (isBusArriveBusStop() && waitAtStopMS >= 0 && waitAtStopMS
			< BUS_STOP_WAIT_PASSENGER_TIME_SEC) {
		std::cout
				<< "BusDriver::updatePositionOnLink: bus isBusArriveBusStop velocity: "
				<< vehicle->getVelocity() / 100.0 << std::endl;

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
	if (distance >=10 && distance <= 50) {
		if (lastTickDistanceToBusStop < 0) {
			return true;
		} else if (lastTickDistanceToBusStop > distance) {
			return true;
		}
	}
	return false;
}

bool sim_mob::BusDriver::isBusArriveBusStop() const
{
	double distance = distanceToNextBusStop();
	return  (distance>0 && distance <10);
}

bool sim_mob::BusDriver::isBusLeavingBusStop() const
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

	const std::map<centimeter_t, const RoadItem*> & obstacles = rs->obstacles;
	for(std::map<centimeter_t, const RoadItem*>::const_iterator o_it = obstacles.begin(); o_it != obstacles.end() ; o_it++) {
	   RoadItem* ri = const_cast<RoadItem*>(o_it->second);
	   BusStop *bs = dynamic_cast<BusStop *>(ri);
	   if(bs) {
		   if (roadSegment == vehicle->getCurrSegment()) {
			   if (bs->stopPoint < 0) {
				   throw std::runtime_error("Bus stop point should have already been set.");
				   /*DynamicVector SegmentLength(rs->getEnd()->location.getX(),rs->getEnd()->location.getY(),rs->getStart()->location.getX(),rs->getStart()->location.getY());
				   DynamicVector BusStopDistfromStart(bs->xPos,bs->yPos,rs->getStart()->location.getX(),rs->getStart()->location.getY());
				   DynamicVector BusStopDistfromEnd(rs->getEnd()->location.getX(),rs->getEnd()->location.getY(),bs->xPos,bs->yPos);
				   double a = BusStopDistfromStart.getMagnitude();
				   double b = BusStopDistfromEnd.getMagnitude();
				   double c = SegmentLength.getMagnitude();
				   bs->stopPoint = (-b*b + a*a + c*c)/(2.0*c);*/
			   }

			   if (bs->stopPoint >= 0) {
					DynamicVector BusDistfromStart(getPositionX(), getPositionY(),rs->getStart()->location.getX(),rs->getStart()->location.getY());
					std::cout<<"BusDriver::DistanceToNextBusStop: bus move in segment: "<<BusDistfromStart.getMagnitude()<<std::endl;
					distance = bs->stopPoint - BusDistfromStart.getMagnitude();
				}
		   } else {
			   DynamicVector busToSegmentStartDistance(currentX,currentY, rs->getStart()->location.getX(),rs->getStart()->location.getY());
			   distance = busToSegmentStartDistance.getMagnitude() + bs->stopPoint;
		   }
	   }
	}
	return distance/100.0;
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

