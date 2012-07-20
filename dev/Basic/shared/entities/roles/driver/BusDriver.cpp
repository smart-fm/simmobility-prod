/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusDriver.hpp"

#include <vector>

#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/aimsun/Loader.hpp"
using namespace sim_mob;
using std::vector;


namespace {

//Create a simple route via the following rules:
//   1) Every other road segment (starting with the first) has a
//      bus stop.
//   2) The first of these is located 30% down the road, the second is
//      60% down the road, and all others are 50% down the road.
DemoBusStop newbs;




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
        			/*std::cout << "Bus stop position : " << xbs[c] << " " << ybs[c]<<"    "<<c << std::endl;*/
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
        						    std::cout<<"PID<"<<getpid()<<"> "<<"MakeSampleRoute1: bus stop, percent is <"<<next->percent<<">"<<std::endl;
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
	: Driver(parent, mtxStrat), nextStop(nullptr), waitAtStopMS(0.0)
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
	Bus* bus;

	bus = new Bus(MakeSampleRoute1(vehicle->getCompletePath()), vehicle);


	//std::cout<<"Help I am a bus"<<vehicle->getCompletePath().size()<<std::endl;
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
	stops = bus->getRoute().getStops();
//	std::cout<<"NextStopis   "<<bus->getRoute().getCurrentStop()->percent<<std::endl;
	waitAtStopMS = 0.0;

}


double sim_mob::BusDriver::updatePositionOnLink(DriverUpdateParams& p)
{
	//First, have we just passed a bus stop? (You'll have to modify this to
	//   detect them in advance, but the concept's similar)
	bool atBusStop;

	std::vector<DemoBusStop>::const_iterator rii;
	for(rii=stops.begin(); rii!=stops.end(); ++rii)
	{
		DemoBusStop s = *rii;
		if (std::find(arrivedStops.begin(), arrivedStops.end(), s)!=arrivedStops.end())
		{
			continue;
		}

		atBusStop = s.atOrPastBusStop(vehicle->getCurrSegment(), vehicle->getDistanceMovedInSegment());
		if (atBusStop)
		{
			std::cout<<"BusDriver::updatePositionOnLink:find bus stop"<<std::endl;
			arrivedStops.push_back(s);
			break;
		}
	}
	const RoadSegment *rseg = vehicle->getCurrSegment();
	std::cout<<"BusDriver.updatePositionOnLink: current vehicle x: "<<rseg->getStart()->location.getX()
			<<" y: "<<rseg->getStart()->location.getY()<<std::endl;
	//bool atBusStop = nextStop && nextStop->atOrPastBusStop(vehicle->getCurrSegment(), vehicle->getDistanceMovedInSegment());
	//std::cout<<"updatePositionOnLink:"<<nextStop->percent<<std::endl;
	if (!nextStop)
	{
		std::cout<<"BusDriver::updatePositionOnLink:nextStop is empty"<<std::endl;
	}
	if (atBusStop)
	{
		std::cout<<"BusDriver::updatePositionOnLink:arrive bus stop"<<std::endl;
	}
	bool updatePos = false;
	//std::cout<<"SEGMENT LENGTH    "<<vehicle->getCurrSegment()->getId()<<"        "<<vehicle->getCurrSegment()->laneEdgePolylines_cached.size()<<std::endl;
	//If we are moving, then (optionally) decelerate and call normal update behavior.
	if (vehicle->getVelocity()>0 || vehicle->getLatVelocity()>0) {
		if (atBusStop) {
			//vehicle->setAcceleration(-300); //Force stop.

			vehicle->setAcceleration(0);
			vehicle->setVelocity(0); //TEMP: Need to really force it.
			waitAtStopMS = p.currTimeMS; //TEMP: Need to force this too.
			std::cout<<"SEGMENT LENGTH    "<<vehicle->getCurrSegment()->getId()<<"        "<<vehicle->getDistanceMovedInSegment()<<std::endl;
			std::cout<<"Yay we are at bus stop"<<vehicle->getX()<<"      "<<vehicle->getY()<<std::endl;
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
			//std::cout<<"This is not a Bus Stop"<<std::endl;
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
			//<<","<<vehicle->getCurrSegment()->obstacles.size()<<"+"<<vehicle->getCurrSegment()->getId()

			//<<"distance  "<<static_cast<int>(vehicle->getDistanceMovedInSegment())/(vehicle->getCurrLinkLaneZeroLength())
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(vehicle->getX())
			<<"\",\"yPos\":\""<<static_cast<int>(vehicle->getY())
			<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
			<<"\",\"length\":\""<<static_cast<int>(3*vehicle->length)
			<<"\",\"width\":\""<<static_cast<int>(2*vehicle->width)
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
			<< "\",\"segment\":\"" << vehicle->getCurrSegment()->getId()
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

