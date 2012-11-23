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
const int BUS_STOP_WAIT_PASSENGER_TIME_SEC = 2;
} //End anonymous namespace

sim_mob::BusDriver::BusDriver(Person* parent, MutexStrategy mtxStrat)
	: Driver(parent, mtxStrat), nextStop(nullptr), waitAtStopMS(-1) , lastTickDistanceToBusStop(-1)
, lastVisited_BusStop(mtxStrat,nullptr), lastVisited_BusStopSequenceNum(mtxStrat,0), real_DepartureTime(mtxStrat,0)
, real_ArrivalTime(mtxStrat,0), DwellTime_ijk(mtxStrat,0), first_busstop(true)
, last_busstop(false), no_passengers_boarding(0), no_passengers_alighting(0)
{
}


Role* sim_mob::BusDriver::clone(Person* parent) const
{
	return new BusDriver(parent, parent->getMutexStrategy());
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
			RoadItem* ri = const_cast<RoadItem*>(ob_it->second);
			BusStop *bs = dynamic_cast<BusStop*>(ri);
			if(bs) {
				// calculate bus stops point
				//NOTE: This is already calculated in the offset (RoadSegment obstacles list) ~Seth
				/*DynamicVector SegmentLength(rs->getEnd()->location.getX(),rs->getEnd()->location.getY(),rs->getStart()->location.getX(),rs->getStart()->location.getY());
				DynamicVector BusStopDistfromStart(bs->xPos,bs->yPos,rs->getStart()->location.getX(),rs->getStart()->location.getY());
				DynamicVector BusStopDistfromEnd(rs->getEnd()->location.getX(),rs->getEnd()->location.getY(),bs->xPos,bs->yPos);
				double a = BusStopDistfromStart.getMagnitude();
				double b = BusStopDistfromEnd.getMagnitude();
				double c = SegmentLength.getMagnitude();
				bs->stopPoint = (-b*b + a*a + c*c)/(2.0*c);*/
				//std::cout<<"stopPoint: "<<bs->stopPoint/100.0<<std::endl;
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
//	std::cout << "BusDriver::updatePositionOnLink:current lane: "
//			  << p.currLaneIndex << " lat velo: " << newLatVel / 100.0 << "m/s"
//			  << std::endl;
	if (isBusApproachingBusStop()) {
		busAccelerating(p);

		//move to most left lane
		p.nextLaneIndex = vehicle->getCurrSegment()->getLanes().back()->getLaneID();
		LANE_CHANGE_SIDE lcs = mitsim_lc_model->makeMandatoryLaneChangingDecision(p);
		vehicle->setTurningDirection(lcs);
		double newLatVel;
		newLatVel = mitsim_lc_model->executeLaneChanging(p, vehicle->getAllRestRoadSegmentsLength(), vehicle->length, vehicle->getTurningDirection());
		vehicle->setLatVelocity(newLatVel*5);


//		std::cout << "BusDriver::updatePositionOnLink: bus approaching current lane: "
//				  << p.currLaneIndex << std::endl;

		// reduce speed
		if (vehicle->getVelocity() / 100.0 > 10)
			vehicle->setAcceleration(-500);

		waitAtStopMS = 0;
	}
	if (isBusArriveBusStop() && waitAtStopMS >= 0 && waitAtStopMS
			< BUS_STOP_WAIT_PASSENGER_TIME_SEC) {
//		std::cout
//				<< "BusDriver::updatePositionOnLink: bus isBusArriveBusStop velocity: "
//				<< vehicle->getVelocity() / 100.0 << std::endl;
//		real_ArrivalTime.set(p.currTimeMS);// BusDriver set RealArrival Time, set once(the first time comes in)
//		if(!BusController::all_busctrllers_.empty()) {
//			BusController::all_busctrllers_[0]->receiveBusInformation(1, 0, 0, p.currTimeMS);
//			unsigned int departureTime = BusController::all_busctrllers_[0]->decisionCalculation(1, 0, 0, p.currTimeMS,lastVisited_BusStopSequenceNum.get());// need to be changed, only calculate once(no need every time calculation)
//			real_DepartureTime.set(departureTime);// BusDriver set RealDeparture Time
//		}
		if (vehicle->getVelocity() > 0)
			vehicle->setAcceleration(-5000);
		if (vehicle->getVelocity() < 0.1 && waitAtStopMS < BUS_STOP_WAIT_PASSENGER_TIME_SEC) {
			waitAtStopMS = waitAtStopMS + p.elapsedSeconds;
//			std::cout << "BusDriver::updatePositionOnLink: waitAtStopMS: " << waitAtStopMS << " p.elapsedSeconds: " << p.elapsedSeconds << std::endl;

			//Pick up a semi-random number of passengers
			Bus* bus = dynamic_cast<Bus*>(vehicle);
			if ((waitAtStopMS == p.elapsedSeconds) && bus) {
//				std::cout << "BusDriver::updatePositionOnLink: pich up passengers" << std::endl;
				std::cout << "real_ArrivalTime value: " << real_ArrivalTime.get() << "  DwellTime_ijk: " << DwellTime_ijk.get() << std::endl;
				real_ArrivalTime.set(p.currTimeMS);// BusDriver set RealArrival Time, set once(the first time comes in)
				DwellTime_ijk.set(passengerGeneration(bus));

				//int pCount = reinterpret_cast<intptr_t> (vehicle) % 50;
				//bus->setPassengerCount(pCount);

				if(!BusController::all_busctrllers_.empty()) {
					BusController::all_busctrllers_[0]->receiveBusInformation(1, 0, 0, p.currTimeMS);
					unsigned int departureTime = BusController::all_busctrllers_[0]->decisionCalculation(1, 0, 0, p.currTimeMS,lastVisited_BusStopSequenceNum.get());// need to be changed, only calculate once(no need every time calculation)
					real_DepartureTime.set(departureTime);// BusDriver set RealDeparture Time
				}
			}
		}
	} else if (isBusArriveBusStop()) {
		vehicle->setAcceleration(3000);
	}

	//TODO: Please check from here; the merge was not 100% clean. ~Seth
	if (isBusLeavingBusStop() || waitAtStopMS >= BUS_STOP_WAIT_PASSENGER_TIME_SEC) {
		std::cout << "BusDriver::updatePositionOnLink: bus isBusLeavingBusStop" << std::endl;
		waitAtStopMS = -1;

		busAccelerating(p);
	}

/*	std::cout<<"BusDriver::updatePositionOnLink:tick: "<<p.currTimeMS/1000.0<<std::endl;
	std::cout<<"BusDriver::updatePositionOnLink:busvelocity: "<<vehicle->getVelocity()/100.0<<std::endl;
	std::cout<<"BusDriver::updatePositionOnLink:busacceleration: "<<vehicle->getAcceleration()/100.0<<std::endl;
	std::cout<<"BusDriver::updatePositionOnLink:buslateralvelocity: "<<vehicle->getLatVelocity()/100.0<<std::endl;
	std::cout<<"BusDriver::updatePositionOnLink:busstopdistance: "<<distanceToNextBusStop()<<std::endl;
	std::cout<<"my bus distance moved in segment: "<<vehicle->getDistanceToSegmentStart()/100.0<<std::endl;
	std::cout<<"but DistanceMovedInSegment"<<vehicle->getDistanceMovedInSegment()/100.0<<std::endl;
	std::cout<<"current polyline length: "<<vehicle->getCurrPolylineLength()/100.0<<std::endl;*/
	DynamicVector segmentlength(vehicle->getCurrSegment()->getStart()->location.getX(),vehicle->getCurrSegment()->getStart()->location.getY(),
			vehicle->getCurrSegment()->getEnd()->location.getX(),vehicle->getCurrSegment()->getEnd()->location.getY());
//	std::cout<<"current segment length: "<<segmentlength.getMagnitude()/100.0<<std::endl;

//	double fwdDistance = vehicle->getVelocity() * p.elapsedSeconds + 0.5 * vehicle->getAcceleration() * p.elapsedSeconds * p.elapsedSeconds;
//	if (fwdDistance < 0)
//		fwdDistance = 0;

	//double fwdDistance = vehicle->getVelocity()*p.elapsedSeconds;
//	double latDistance = vehicle->getLatVelocity() * p.elapsedSeconds;

	//Increase the vehicle's velocity based on its acceleration.


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
	double distanceToCurrentSegmentBusStop = getDistanceToBusStopOfSegment(vehicle->getCurrSegment());
	double distanceToNextSegmentBusStop;
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
		//if first busstop,only boarding
		if(first_busstop ==true)
		{
			no_passengers_boarding = (config.percent_boarding * 0.01)*no_passengers_busstop;
			if(no_passengers_boarding > bus->getBusCapacity() - no_passengers_bus)
				no_passengers_boarding = bus->getBusCapacity() - no_passengers_bus;
			passengers_Board(bus);//board the bus
			bus->setPassengerCount(no_passengers_bus+no_passengers_boarding);
			first_busstop= false;
		}
		//if last busstop,only alighting
		else if(last_busstop==true)
		{
			no_passengers_alighting=(config.percent_alighting * 0.01)*no_passengers_bus;
			passengers_Alight(bus);//alight the bus
			no_passengers_bus=no_passengers_bus - no_passengers_alighting;
			bus->setPassengerCount(no_passengers_bus);
			last_busstop = false;
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

double sim_mob::BusDriver::getDistanceToBusStopOfSegment(const RoadSegment* rs) const
{

	double distance = -100;
	double currentX = vehicle->getX();
	double currentY = vehicle->getY();
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
		   int stopPoint = o_it->first;

		   if(bs)
		   {
			   if (rs == vehicle->getCurrSegment())
			   {

				   if (stopPoint < 0)
				   {
					   throw std::runtime_error("BusDriver offset in obstacles list should never be <0");
					   /*std::cout<<"BusDriver::DistanceToNextBusStop :stopPoint < 0"<<std::endl;
					   DynamicVector SegmentLength(rs->getEnd()->location.getX(),rs->getEnd()->location.getY(),rs->getStart()->location.getX(),rs->getStart()->location.getY());
					   DynamicVector BusStopDistfromStart(bs->xPos,bs->yPos,rs->getStart()->location.getX(),rs->getStart()->location.getY());
					   DynamicVector BusStopDistfromEnd(rs->getEnd()->location.getX(),rs->getEnd()->location.getY(),bs->xPos,bs->yPos);
					   double a = BusStopDistfromStart.getMagnitude();
					   double b = BusStopDistfromEnd.getMagnitude();
					   double c = SegmentLength.getMagnitude();
					   bs->stopPoint = (-b*b + a*a + c*c)/(2.0*c);*/
				   }

				   if (stopPoint >= 0)
				   {
						DynamicVector BusDistfromStart(vehicle->getX(),vehicle->getY(),rs->getStart()->location.getX(),rs->getStart()->location.getY());
						//std::cout<<"BusDriver::DistanceToNextBusStop: bus move in segment: "<<BusDistfromStart.getMagnitude()<<std::endl;
						//distance = bs->stopPoint - BusDistfromStart.getMagnitude();
						distance = stopPoint - vehicle->getDistanceMovedInSegment();
						//std::cout<<"BusDriver::DistanceToNextBusStop :distance: "<<distance<<std::endl;
					}
			   }
			   else
			   {
				   DynamicVector busToSegmentStartDistance(currentX,currentY,
						   rs->getStart()->location.getX(),rs->getStart()->location.getY());
//				   DynamicVector busToSegmentEndDistance(currentX,currentY,
//				   						   rs->getEnd()->location.getX(),rs->getEnd()->location.getY());
				   //distance = busToSegmentStartDistance.getMagnitude() + bs->stopPoint;
				   distance = vehicle->getCurrentSegmentLength() - vehicle->getDistanceMovedInSegment() + stopPoint;
/*				   std::cout<<"BusDriver::DistanceToNextBusStop :not current segment distance:stopPoint "<<bs->stopPoint<<std::endl;
				   std::cout<<"BusDriver::DistanceToNextBusStop :not current segment distance:busToSegmentStartDistance: "<<busToSegmentStartDistance.getMagnitude()<<std::endl;
				   std::cout<<"BusDriver::DistanceToNextBusStop :not current segment distance: "<<distance<<std::endl;*/
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
//	typedef std::map<centimeter_t, const RoadItem*>::const_iterator RoadObstIt;
//	if (!roadSegment) { return -1; }
//
//	//The obstacle offset now correctly returns the BusStop's distance down the segment.
//	for(RoadObstIt o_it = roadSegment->obstacles.begin(); o_it!=roadSegment->obstacles.end(); o_it++) {
//		const BusStop *bs = dynamic_cast<const BusStop *>(o_it->second);
//		if (bs) {
//			DynamicVector BusDistfromStart(getPositionX(), getPositionY(),roadSegment->getStart()->location.getX(),roadSegment->getStart()->location.getY());
//			std::cout<<"BusDriver::DistanceToNextBusStop: bus move in segment: "<<BusDistfromStart.getMagnitude()<<std::endl;
//			return  (o_it->first - BusDistfromStart.getMagnitude()) / 100.0;
//		}
//	}
//
//	return -1;
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

vector<BufferedBase*> sim_mob::BusDriver::getSubscriptionParams() {
	vector<BufferedBase*> res;
	res = Driver::getSubscriptionParams();

	// BusDriver's features
	res.push_back(&(lastVisited_BusStop));
	res.push_back(&(real_DepartureTime));
	res.push_back(&(real_ArrivalTime));
	res.push_back(&(DwellTime_ijk));
	return res;
}

