/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusDriver.hpp"
#include <sys/syscall.h>
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

//Create a simple route via the following rules:
//   1) Every other road segment (starting with the first) has a
//      bus stop.
//   2) The first of these is located 30% down the road, the second is
//      60% down the road, and all others are 50% down the road.
DemoBusStop newbs;

#define BUS_STOP_WAIT_PASSENGER_TIME_SEC   2


/*
        double xbs= 37259818;
        double ybs= 14383558;

         double xbs= 37274610;
        double ybs= 14364177;

*/
int c=0;
BusRoute MakeSampleRoute1(const vector<const RoadSegment*>& path)
{
	vector<DemoBusStop> res;
	for (size_t i=0; i<path.size(); i+=1) {
		DemoBusStop *next = new DemoBusStop();
		next->seg = path.at(i);

		centimeter_t a = next->seg->length;
        Node* start = next->seg->getStart();
        Node* end = next->seg->getEnd();
        //std::cout<<"Hello Ji"<<start->getID()<<"       "<<end->getID()<<std::endl;
        std::cout<<i<<":  next->seg start: "<<next->seg->getStart()->location.getX()<<"  "
        		<<next->seg->getStart()->location.getY()<<" next->seg end: "<<next->seg->getEnd()->location.getX()<<"  "
        		<<next->seg->getEnd()->location.getY()<<std::endl;
        double xbs[10];
        double ybs[10];
        vector<const RoadSegment*>::const_iterator it = path.begin();
        for(it = path.begin(); it != path.end() ; it++)
        {
        	const RoadSegment* rs = (*it);
        	std::cout<<"rs start: "<<rs->getStart()->location.getX()<<"  "
        	        		<<rs->getStart()->location.getY()
        	        		<<" rs end: "<<rs->getEnd()->location.getX()<<"  "
        	        		<<rs->getEnd()->location.getY()<<std::endl;
        	const std::map<centimeter_t, const RoadItem*> & obstacles = rs->obstacles;
        	for(std::map<centimeter_t, const RoadItem*>::const_iterator o_it = obstacles.begin(); o_it != obstacles.end() ; o_it++)
        	{
        		RoadItem* ri = const_cast<RoadItem*>(o_it->second);
//
        		BusStop *bs = dynamic_cast<BusStop *>(ri);
        		if(bs)
        		{
        			DemoBusStop busStop;
        			xbs[c] = bs->xPos;
        			ybs[c] = bs->yPos;
        			std::cout << "MakeSampleRoute1: Bus stop position : " << xbs[c] << " " << ybs[c]<<"    "<<c << std::endl;
        			double remDist;
        			        /*std::cout<<"kya"<<next.seg->getId()<<std::endl;*/
        			        DynamicVector SegmentLength(end->location.getX(),end->location.getY(),start->location.getX(),start->location.getY());
        			        DynamicVector BusStopDistfromStart(xbs[c],ybs[c],start->location.getX(),start->location.getY());
        			        DynamicVector BusStopDistfromEnd(end->location.getX(),end->location.getY(),xbs[c],ybs[c]);
        					std::cout<<"Distance before 3     "<<end->location.getX()<<std::endl;
        					if(BusStopDistfromStart.getMagnitude()<=SegmentLength.getMagnitude() && BusStopDistfromEnd.getMagnitude()<=SegmentLength.getMagnitude())
        					        {
        						    remDist = BusStopDistfromStart.getMagnitude();
        					        /*std::cout<<"kya"<<next.seg->getId()<<"      "<<remDist<<"      "<<BusStopDistfromStart.getMagnitude()<<"       "<<BusStopDistfromEnd.getMagnitude()<<"      "<<SegmentLength.getMagnitude()<<"     "<<next.percent<<std::endl;*/

        						    next->finalDist = remDist;
        						    next->percent = next->finalDist/SegmentLength.getMagnitude();
        						    next->distance = next->distance + next->seg->length;
        						    std::cout<<"See here is Distance"<<next->finalDist<<"      "<<next->percent<<std::endl;
        						    /*std::cout<<"Hello path.size()    "<<path.size()<<"    "<<next.percent<<"       "<<a<<"       "<<start->location.getX()<<"       "<<end->location.getX()<<"       "<<start->location.getY()<<"       "<<end->location.getY()<<std::endl;*/
        						    busStop.seg = next->seg;
        						    busStop.finalDist = next->finalDist;
        						    double t = next->percent;
        						    busStop.percent = t;
        						    busStop.distance = next->distance;
        						    res.push_back(busStop);
        						    std::cout<<"PID<"<<syscall(SYS_gettid)<<"> "<<"MakeSampleRoute1: bus stop, percent is <"<<next->percent<<">"<<std::endl;
//        						    std::cout<<"Olay Olay"<<res.at(i).percent<<std::endl;
        					        }
        					else
        					{
        						remDist=0;
        						next->finalDist = remDist;
        						        						    next->percent = next->finalDist/SegmentLength.getMagnitude();
        						        						    next->distance = next->distance + next->seg->length;
        						        						    std::cout<<"See here is Distance"<<next->finalDist<<"      "<<next->percent<<std::endl;
        						        						    /*std::cout<<"Hello path.size()    "<<path.size()<<"    "<<next.percent<<"       "<<a<<"       "<<start->location.getX()<<"       "<<end->location.getX()<<"       "<<start->location.getY()<<"       "<<end->location.getY()<<std::endl;*/

        						        						            						    busStop.seg = next->seg;
        						        						            						    busStop.finalDist = next->finalDist;
        						        						            						    double tt = next->percent;
        						        						            						    busStop.percent = next->percent;
        						        						            						    busStop.distance = next->distance;
        						        						            						    res.push_back(busStop);
//        						        						    res.push_back(*next);
        						        						    std::cout<<"PID<"<<getpid()<<"> "<<"MakeSampleRoute1: bus stop magnitude not in segment,percent is zeo"<<std::endl;
        						        						    //std::cout<<"Olay Olay"<<ceil(res.at(i).percent)<<std::endl;
        					}


        					c++;
        			        }
                  }
        	}

delete next;
        }





	return BusRoute(res);

}

} //End anonymous namespace

sim_mob::BusDriver::BusDriver(Person* parent, MutexStrategy mtxStrat)
	: Driver(parent, mtxStrat), nextStop(nullptr), waitAtStopMS(-1) , lastTickDistanceToBusStop(-1)
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
//	Bus* bus;

	bus = new Bus(MakeSampleRoute1(vehicle->getCompletePath()), vehicle);


	//std::cout<<"Help I am a bus"<<vehicle->getCompletePath().size()<<std::endl;
	delete vehicle;
	vehicle = bus;


	//This code is used by Driver to set a few properties of the Vehicle/Bus.
	if (bus && bus->hasPath()) {
		setOrigin(params);
	} else {
		throw std::runtime_error("Vehicle could not be created for bus driver; no route!");
	}

	//Unique to BusDrivers: reset your route
	bus->getRoute().reset();
	nextStop = bus->getRoute().getCurrentStop();
	stops = bus->getRoute().getStops();
//	std::cout<<"NextStopis   "<<bus->getRoute().getCurrentStop()->percent<<std::endl;
	waitAtStopMS = 0.0;

}


double sim_mob::BusDriver::updatePositionOnLink(DriverUpdateParams& p)
{
	//First, have we just passed a bus stop? (You'll have to modify this to
	//   detect them in advance, but the concept's similar)
	bool atBusStop;

//	std::vector<DemoBusStop>::const_iterator rii;
//	for(rii=stops.begin(); rii!=stops.end(); ++rii)
//	{
//		DemoBusStop s = *rii;
//		if (std::find(arrivedStops.begin(), arrivedStops.end(), s)!=arrivedStops.end())
//		{
//			continue;
//		}
//
//		atBusStop = s.atOrPastBusStop(bus->getCurrSegment(), bus->getDistanceMovedInSegment());
//		if (atBusStop)
//		{
//			std::cout<<"BusDriver::updatePositionOnLink:find bus stop"<<std::endl;
//			arrivedStops.push_back(s);
//			break;
//		}
//	}


	//bus approaching bus stop reduce speed
	//and if its left has lane, merge to left lane
	if (isBusApproachingBusStop())
	{
		std::cout<<"BusDriver::updatePositionOnLink: bus approaching"<<std::endl;
		// reduce speed
		//...

		//change left lane
		if (p.leftLane) //has lane in left?
		{
			//Lateral movement
			if (!bus->isInIntersection()) {
				bus->setTurningDirection(LCS_LEFT);
				bus->setLatVelocity(1500);
			}
		} else {
			bus->setLatVelocity(0);
			std::cout<<"BusDriver::updatePositionOnLink: no more left lane"<<std::endl;
		}
		waitAtStopMS = 0;
	}
	if (isBusArriveBusStop() && waitAtStopMS>=0 && waitAtStopMS < BUS_STOP_WAIT_PASSENGER_TIME_SEC)
	{
		std::cout<<"BusDriver::updatePositionOnLink: bus isBusArriveBusStop velocity: "<<bus->getVelocity()/100.0<<std::endl;

		if ( bus->getVelocity()>0 )
			bus->setAcceleration(-5000);
		if ( bus->getVelocity() < 0.1 && waitAtStopMS < BUS_STOP_WAIT_PASSENGER_TIME_SEC)
		{
			waitAtStopMS = waitAtStopMS + p.elapsedSeconds;
			std::cout<<"BusDriver::updatePositionOnLink: waitAtStopMS: "<<waitAtStopMS<<" p.elapsedSeconds: "<<p.elapsedSeconds<<std::endl;
			//Pick up a semi-random number of passengers
			if (waitAtStopMS == p.elapsedSeconds)
			{
				std::cout<<"BusDriver::updatePositionOnLink: pich up passengers"<<std::endl;
				int pCount = reinterpret_cast<intptr_t>(bus) % 50;
				bus->setPassengerCount(pCount);
			}
		}
	} else if (isBusArriveBusStop()){
		bus->setAcceleration(3000);
	}
	if (isBusLeavingBusStop() || waitAtStopMS >= BUS_STOP_WAIT_PASSENGER_TIME_SEC )
	{
		std::cout<<"BusDriver::updatePositionOnLink: bus isBusLeavingBusStop"<<std::endl;
		waitAtStopMS = -1;
	}
	std::cout<<"PID<"<<syscall(SYS_gettid)<<"> "<<"BusDriver::updatePositionOnLink: bus velocity: "<<bus->getVelocity()/100.0<<std::endl;
	std::cout<<"PID<"<<syscall(SYS_gettid)<<"> "<<"BusDriver::updatePositionOnLink: bus lateral velocity: "<<bus->getLatVelocity()/100.0<<std::endl;
	std::cout<<"PID<"<<syscall(SYS_gettid)<<"> "<<"BusDriver::updatePositionOnLink: bus stop distance: "<<DistanceToNextBusStop()<<std::endl;
	//std::cout<<"bus current lane can change left: "<<bus->getCurrLane()->can_change_lane_left()<<std::endl;
	//std::cout<<"bus current lane can change right: "<<bus->getCurrLane()->can_change_lane_right()<<std::endl;

		double fwdDistance = vehicle->getVelocity() * p.elapsedSeconds + 0.5 * vehicle->getAcceleration()
				* p.elapsedSeconds * p.elapsedSeconds;
		if (fwdDistance < 0)
			fwdDistance = 0;


		//double fwdDistance = vehicle->getVelocity()*p.elapsedSeconds;
		double latDistance = vehicle->getLatVelocity() * p.elapsedSeconds;

		//Increase the vehicle's velocity based on its acceleration.

		vehicle->setVelocity(vehicle->getVelocity() + vehicle->getAcceleration() * p.elapsedSeconds);


		//TEMP: For ns3
//		Person* parentP = dynamic_cast<Person*> (this);
//		if (parentP && !parentP->specialStr.empty() && parentP->specialStr.at(5) == 'A') {
//			vehicle->setVelocity(vehicle->getVelocity() * 1.1);
//		}

		//when v_lead and a_lead is 0, space is not negative, the Car Following will generate an acceleration based on free flowing model
		//this causes problem, so i manually set acceleration and velocity to 0
		if (vehicle->getVelocity() < 0 ||(p.space<1&&p.v_lead==0&&p.a_lead==0)) {
			//Set to 0 forward velocity, no acceleration.
			vehicle->setVelocity(0.0);
			vehicle->setAcceleration(0);
		}

		//Move the vehicle forward.
		double res = 0.0;
		try {
	//		if(parent->getId() == 8)
	//		LogOut("8,88\n");

			res = bus->moveFwd(fwdDistance);
		} catch (std::exception& ex) {
			if (Debug::Drivers) {
	#ifndef SIMMOB_DISABLE_OUTPUT
				DebugStream << ">>>Exception: " << ex.what() << endl;
				boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
				std::cout << DebugStream.str();
	#endif
			}

			std::stringstream msg;
			msg << "Error moving vehicle forward for Agent ID: " << parent->getId() << "," << this->vehicle->getX() << "," << this->vehicle->getY() << "\n" << ex.what();
			throw std::runtime_error(msg.str().c_str());
		}

		//Retrieve what direction we're moving in, since it will "flip" if we cross the relative X axis.
		LANE_CHANGE_SIDE relative = getCurrLaneSideRelativeToCenter();
		//after forwarding, adjacent lanes might be changed
		updateAdjacentLanes(p);
		//there is no left lane when turning left
		//or there is no right lane when turning right
		if((vehicle->getTurningDirection()==LCS_LEFT && !p.leftLane)||
				(vehicle->getTurningDirection()==LCS_RIGHT && !p.rightLane))
		{
			latDistance = 0;
			vehicle->setLatVelocity(0);
		}


		//Lateral movement
		if (!vehicle->isInIntersection() && latDistance != 0) {
			std::cout<<"BusDriver::updatePositionOnLink : bus position before move left: "<<bus->getX()<<"  "<<bus->getY()<<std::endl;
			vehicle->moveLat(latDistance);
			updatePositionDuringLaneChange(p, relative);
			std::cout<<"BusDriver::updatePositionOnLink : bus position after move left: "<<bus->getX()<<"  "<<bus->getY()<<std::endl;
		}

		//Update our offset in the current lane.
		if (!vehicle->isInIntersection()) {
			p.currLaneOffset = vehicle->getDistanceMovedInSegment();
		}



		return res;
}
bool sim_mob::BusDriver::isBusApproachingBusStop()
{
	double distance = DistanceToNextBusStop();
	//std::cout<<"BusDriver::updatePositionOnLink: bus stop distance <"<<distance<<"> m"<<std::endl;
	//std::cout<<"BusDriver::updatePositionOnLink: lastTickDistanceToBusStop <"<<lastTickDistanceToBusStop<<"> m"<<std::endl;
	if (distance >=10 && distance < 50)
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
bool sim_mob::BusDriver::isBusArriveBusStop()
{
	double distance = DistanceToNextBusStop();
	if (distance>0 && distance <10)
	{
		return true;
	}

	return false;
}
bool sim_mob::BusDriver::isBusLeavingBusStop()
{
	double distance = DistanceToNextBusStop();
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
double sim_mob::BusDriver::DistanceToNextBusStop()
{
	double distanceToCurrentSegmentBusStop = -100;
	double distanceToNextSegmentBusStop = -100;
	const RoadSegment* rsCurrent = bus->getCurrSegment();
	const RoadSegment* rsNext = bus->getNextSegment(true);

	if (rsCurrent)
		distanceToCurrentSegmentBusStop = getDistanceToBusStopOfSegment(*rsCurrent);
	if (rsNext)
		distanceToNextSegmentBusStop = getDistanceToBusStopOfSegment(*rsNext);

	std::cout.precision(10);
//	std::cout<<"BusDriver::DistanceToNextBusStop : current segment bs distance: "<<distanceToCurrentSegmentBusStop<<std::endl;
//	std::cout<<"BusDriver::DistanceToNextBusStop : next segment bs distance: "<<distanceToNextSegmentBusStop<<std::endl;

	if (distanceToCurrentSegmentBusStop >= 0 && distanceToNextSegmentBusStop >= 0)
		return ((distanceToCurrentSegmentBusStop<=distanceToNextSegmentBusStop) ? distanceToCurrentSegmentBusStop: distanceToNextSegmentBusStop);
	else if (distanceToCurrentSegmentBusStop > 0)
		return distanceToCurrentSegmentBusStop;
	else
		return distanceToNextSegmentBusStop;
}
double sim_mob::BusDriver::getDistanceToBusStopOfSegment(const RoadSegment& roadSegment)
{
	const RoadSegment* rs = &roadSegment;

	double distance = -100;
	double currentX = bus->getX();
	double currentY = bus->getY();
	bus->getRoute().getCurrentStop();

		std::cout.precision(10);
//		std::cout<<"BusDriver::DistanceToNextBusStop : current bus position: "<<currentX<<"  "<<currentY<<std::endl;
//		std::cout<<"BusDriver::DistanceToNextBusStop : seg start: "<<rs->getStart()->location.getX()<<"  "
//		        		<<rs->getStart()->location.getY()<<" rs end: "<<rs->getEnd()->location.getX()<<"  "
//		        		<<rs->getEnd()->location.getY()<<std::endl;
		const std::map<centimeter_t, const RoadItem*> & obstacles = rs->obstacles;
		int i = 1;
		for(std::map<centimeter_t, const RoadItem*>::const_iterator o_it = obstacles.begin(); o_it != obstacles.end() ; o_it++)
		{
		   RoadItem* ri = const_cast<RoadItem*>(o_it->second);
		   BusStop *bs = dynamic_cast<BusStop *>(ri);
		   if(bs)
		   {
//			   std::cout<<"BusDriver::DistanceToNextBusStop: find bus stop <"<<i<<"> in segment"<<std::endl;
			   double busStopX = bs->xPos;
			   double busStopY = bs->yPos;
//				std::cout<<"BusDriver::DistanceToNextBusStop : bus stop position: "<<busStopX<<"  "<<busStopY<<std::endl;
			   double dis = sqrt((currentX-busStopX)*(currentX-busStopX) + (currentY-busStopY)*(currentY-busStopY));
			   if (distance < 0 || dis < distance) // in case more than one stop at the segment
				   distance = dis;
//				std::cout<<"BusDriver::DistanceToNextBusStop : distance: "<<distance/100.0<<std::endl;
			   i++;
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
	if (bus->isDone() || ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

	//Vehicle should be a Bus
	//const Bus* bus = dynamic_cast<const Bus*>(vehicle);

	double baseAngle = bus->isInIntersection() ? intModel->getCurrentAngle() : bus->getAngle();

#ifndef SIMMOB_DISABLE_OUTPUT
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
	if (frameNumber<parent->getStartTime() || bus->isDone()) {
		return;
	}

#ifndef SIMMOB_DISABLE_OUTPUT
	double baseAngle = bus->isInIntersection() ? intModel->getCurrentAngle() : bus->getAngle();

	//bus should be a Bus
	const Bus* bus = dynamic_cast<const Bus*>(bus);

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

