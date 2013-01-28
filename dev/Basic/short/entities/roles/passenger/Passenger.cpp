/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Passenger.cpp
 *
 *  Created on: 2012-12-20
 *      Author: Meenu
 */

#include "Passenger.hpp"

#include <vector>
#include <iostream>
#include <cmath>

#include <boost/unordered_map.hpp>

#include "entities/Person.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"
#include "entities/AuraManager.hpp"
#include "entities/signal/Signal.hpp"

#include "geospatial/Node.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"

#include "util/DebugFlags.hpp"
#include "util/OutputUtil.hpp"
#include "util/GeomHelpers.hpp"
#include "util/GeomHelpers.hpp"
#include "util/DynamicVector.hpp"

#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"

using namespace sim_mob;
using std::vector;
using std::cout;
using std::map;
using std::string;

pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;


sim_mob::Passenger::Passenger(Agent* parent, MutexStrategy mtxStrat, std::string roleName) : Role(parent,roleName),WaitingAtBusStop(mtxStrat,true),boardedBus(mtxStrat,false),alightedBus(mtxStrat,false),DestReached(mtxStrat,false),busdriver(mtxStrat,nullptr),random_x(mtxStrat,0),random_y(mtxStrat,0),WaitingTime(-1),params(parent->getGenerator())
{
	/*cStart_busstop_X=0.0;
	cStart_busstop_Y=0.0;
	cEnd_busstop_X=0.0;
	cEnd_busstop_Y=0.0;*/
	 //this->TimeofBoardingBus = 0;
	 this->TimeofReachingBusStop = 0;
}

sim_mob::Passenger::~Passenger() {
}

void sim_mob::Passenger::setParentBufferedData() {

	if((this->isAtBusStop()==false)and(this->busdriver.get()!=NULL))//if passenger inside bus,update position of the passenger agent(inside bus)every frame tick
	{
	  parent->xPos.set(this->busdriver.get()->getPositionX());//passenger x,y position equals the bus drivers x,y position as passenger is inside the bus
	  parent->yPos.set(this->busdriver.get()->getPositionY());
	}



}

void sim_mob::Passenger::frame_init(UpdateParams& p)
{
	 //passenger_inside_bus.set(false);
	 busdriver.set(nullptr);
	// bus stop no 4179,x-372228.099782,143319.818791
	 destination.setX(parent->destNode->location.getX());
	 destination.setY(parent->destNode->location.getY());

	if(parent->destNode->getID() == 75780)
	{
		destination.setX(37222809);
		destination.setY(14331981);
	}
	else if(parent->destNode->getID()==75822)
	{
		destination.setX(37290070);//75822
		destination.setY(14390218);
	}
	else if(parent->destNode->getID() == 91144)
	{
		destination.setX(37285920);
		destination.setY(14375941);

	}
	else if(parent->destNode->getID() == 106946)
	{
		destination.setX(37267223);
		destination.setY(14352090);

	}
	else if(parent->destNode->getID() == 103046)
	{
		destination.setX(37234196);
		destination.setY(14337740);

	}
	else if(parent->destNode->getID() == 95374)
	{
		destination.setX(37241994);
		destination.setY(14347188);

	}
	else if(parent->destNode->getID() == 58950)
	{
		destination.setX(37263940);
		destination.setY(14373280);

	}
	else if(parent->destNode->getID() == 75808)
	{
		destination.setX(37274363);
		destination.setY(14385509);

	}
	else if(parent->destNode->getID() == 98852)
	{
		destination.setX(37254693);
		destination.setY(14335301);

	}

	if(parent->originNode->getID() == 75780)
	{
		parent->xPos.set(37222809);
		parent->yPos.set(14331981);

	}
	else if(parent->originNode->getID() == 91144)
	{
		parent->xPos.set(37285920);
		parent->yPos.set(14375941);

	}
	else if(parent->originNode->getID() == 106946)
	{
		parent->xPos.set(37267223);
		parent->yPos.set(14352090);

	}
	else if(parent->originNode->location.getX()==37236345)//103046
	 {
	 parent->xPos.set(37234200);
	 parent->yPos.set(14337700);
	 }
	 else if(parent->originNode->location.getX()==37242841)//95374
	 {
		 parent->xPos.set(37242000);
	     parent->yPos.set(14347200);
	 }
	 else if(parent->originNode->location.getX()==37265277)//58950
	 {
		 parent->xPos.set(37263900);
		 parent->yPos.set(14373300);
	 }
	 else if(parent->originNode->location.getX()==37275470)//75808
	 {
		 parent->xPos.set(37274300);
		 parent->yPos.set(14385500);
	 }
	 else if(parent->originNode->location.getX()==37218351 and parent->originNode->location.getY()==14335255)//75780
	 {
        parent->xPos.set(37222800);//4179
        parent->yPos.set(14332000);
	 }
	 else if(parent->originNode->location.getX()==37200400 and parent->originNode->location.getY()==14349164)//68014
	 {
	      parent->xPos.set(37199539);//8069
	      parent->yPos.set(14351303);

	 }
	 else if(parent->originNode->getID()==75822)
	 {
		 parent->xPos.set(37290070);//75822
		 parent->yPos.set(14390218);
	 }
	 else
	 {
		 parent->xPos.set(parent->originNode->location.getX());
		 parent->yPos.set(parent->originNode->location.getY());
		 std::cout<<"origin-x"<<parent->originNode->location.getX()<<std::endl;
	 }

	 WaitingAtBusStop.set(true);
	 boardedBus.set(false);
	 alightedBus.set(false);
	 DestReached.set(false);
	 this->TimeofReachingBusStop=this->parent->getStartTime();
}


UpdateParams& sim_mob::Passenger::make_frame_tick_params(timeslice now)
{
	params.reset(now);
		return params;
}


//Main update method
void sim_mob::Passenger::frame_tick(UpdateParams& p)
{
	setParentBufferedData();//update passenger coordinates every frame tick
	if(this->DestReached==true)
	parent->setToBeRemoved();
}


void sim_mob::Passenger::frame_tick_output(const UpdateParams& p)
{
	if (ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

	pthread_mutex_lock(&mu);
	srand(parent->getId()*parent->getId());
	int random_num1 = rand()%(250);
	int random_num2 = rand()%(100);
	pthread_mutex_unlock(&mu);

	if((this->WaitingAtBusStop==true) && (!(this->alightedBus)))
     // LogOut("("<<"\"passenger\","<<p.now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<parent->xPos.get()<<"\"," <<"\"yPos\":\""<<this->parent->yPos.get()<<"\",})"<<std::endl);
		LogOut("("<<"\"passenger\","<<p.now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<(this->parent->xPos.get()+random_num1)<<"\"," <<"\"yPos\":\""<<(this->parent->yPos.get()+random_num2)<<"\",})"<<std::endl);

	else if(this->alightedBus)
	{

	   //this->random_x.set(random_x.get()+random_num1);//passenger x,y position equals the bus drivers x,y position as passenger is inside the bus
	   //this->random_y.set(random_y.get()+random_num2);
	   LogOut("("<<"\"passenger\","<<p.now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<(random_x.get()+random_num1)<<"\"," <<"\"yPos\":\""<<(random_y.get()+random_num2)<<"\",})"<<std::endl);

	}
}

void sim_mob::Passenger::frame_tick_output_mpi(timeslice now)
{
	if (now.frame() < 1 || now.frame() < parent->getStartTime())
			return;
	if((this->WaitingAtBusStop==true) ||(this->alightedBus==true))
	{
		if (this->parent->isFake) {
			//LogOut("("<<"\"passenger\","<<now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<parent->xPos.get()<<"\"," <<"\"yPos\":\""<<this->parent->yPos.get() <<"\"," <<"\"fake\":\""<<"true" <<"\",})"<<std::endl);
			LogOut("("<<"\"passenger\","<<now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<this->random_x.get()<<"\"," <<"\"yPos\":\""<<this->random_y.get() <<"\"," <<"\"fake\":\""<<"true" <<"\",})"<<std::endl);
		} else {
			//LogOut("("<<"\"passenger\","<<now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<parent->xPos.get()<<"\"," <<"\"yPos\":\""<<this->parent->yPos.get() <<"\"," <<"\"fake\":\""<<"false" <<"\",})"<<std::endl);
			LogOut("("<<"\"passenger\","<<now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<this->random_x.get()<<"\"," <<"\"yPos\":\""<<this->random_y.get()<<"\"," <<"\"fake\":\""<<"false" <<"\",})"<<std::endl);
		}
	}
}

 void sim_mob::Passenger::update(timeslice now)
 {

 }
bool sim_mob::Passenger::isAtBusStop() {

	if(this->WaitingAtBusStop==true)//at bus stop
	{
	  return true;
	}
	else//at bus
		return false;
}

bool sim_mob::Passenger::isDestBusStopReached() {

	if(alightedBus==true)//passenger alights when destination is reached
		 return true;
	else
	    return false;
}

Role* sim_mob::Passenger::clone(sim_mob::Person* parent) const {
		return new Passenger(parent, parent->getMutexStrategy());
}

Point2D sim_mob::Passenger::getXYPosition()
{
	return Point2D(parent->xPos.get(),parent->yPos.get());
}

Point2D sim_mob::Passenger::getDestPosition()
{
	return Point2D(parent->destNode->location.getX(),parent->destNode->location.getY());
}

bool sim_mob::Passenger::PassengerBoardBus(Bus* bus,BusDriver* busdriver,Person* p,std::vector<const BusStop*> busStops,int k)
{
	for(;k < busStops.size();k++)//chcking if destinaton node position of passenger is in the list of bus stops which the bus would stop
	{
		if((abs(( busStops[k]->xPos - this->getDestPosition().getX())/1000)<= 3 )and(abs((busStops[k]->yPos - this->getDestPosition().getY())/1000)<= 3))
	 	{
			if(bus->getPassengerCount()+1<=bus->getBusCapacity())//and(last_busstop==false))
			{
				//if passenger is to be boarded,add to passenger vector inside the bus
				bus->passengers_inside_bus.push_back(p);
				//time(&this->TimeofBoardingBus);
				bus->setPassengerCount(bus->getPassengerCount()+1);
				// this->passenger_inside_bus.set(true);//to indicate whether passenger is waiting at the bus stop or is inside the bus
				this->WaitingAtBusStop.set(false);
				this->busdriver.set(busdriver);//passenger should store the bus driver
				this->boardedBus.set(true);//to indicate passenger has boarded bus
				this->alightedBus.set(false);//to indicate whether passenger has alighted bus
				findWaitingTime(bus);
				std::cout<<"iamwaiting "<<findWaitingTime(bus)<<"at the busstop_no "<<busdriver->busstop_sequence_no.get() << "----agent id "<<this->parent->getId()<<" reached "<<this->TimeofReachingBusStop<<" boarded "<<bus->TimeOfBusreachingBusstop<<std::endl;
				return true;
	 		}
	 	}

	}
	return false;
}

bool sim_mob::Passenger::PassengerAlightBus(Bus* bus,int xpos_approachingbusstop,int ypos_approachingbusstop,BusDriver* busdriver)
{
	if((abs((xpos_approachingbusstop/1000)-(this->getDestPosition().getX()/1000))<=2)and(abs((ypos_approachingbusstop/1000)-(this->getDestPosition().getY()/1000))<=2))
	{
		//alight-delete passenger agent from list
		std::cout<<"pcount"<<bus->getPassengerCount()<<std::endl;
		bus->setPassengerCount(bus->getPassengerCount()-1);
        std::cout<<"pcount"<<bus->getPassengerCount()<<std::endl;
        //this->passenger_inside_bus.set(false);
        this->busdriver.set(nullptr);	//driver would be null as passenger has alighted
        this->alightedBus.set(true);
        this->boardedBus.set(false);
	 	//this->updateParentCoordinates(xpos_approachingbusstop,ypos_approachingbusstop);
	 	this->parent->xPos.set(xpos_approachingbusstop);
	 	this->parent->yPos.set(ypos_approachingbusstop);
	 	this->random_x.set(xpos_approachingbusstop);
	 	this->random_y.set(ypos_approachingbusstop);
        return true;
	 }
	 return false;
}

std::vector<sim_mob::BufferedBase*> sim_mob::Passenger::getSubscriptionParams()
{
 	std::vector<sim_mob::BufferedBase*> res;
 	//res.push_back(&(passenger_inside_bus));
 	res.push_back(&(WaitingAtBusStop));
 	res.push_back(&(boardedBus));
 	res.push_back(&(alightedBus));
 	res.push_back(&(DestReached));
 	res.push_back(&(busdriver));
 	res.push_back(&(random_x));
 	res.push_back(&(random_y));
 	//	res.push_back(&(estimated_boarding_passengers_no));
 	return res;
}

bool sim_mob::Passenger::isBusBoarded()
{
	if(this->boardedBus==true)
		return true;
	else
		return false;
}

double sim_mob::Passenger::findWaitingTime(Bus* bus)
{
	//this->WaitingTime=difftime(this->TimeofBoardingBus,this->TimeofReachingBusStop);
	this->WaitingTime=(bus->TimeOfBusreachingBusstop)-(this->TimeofReachingBusStop);
	return this->WaitingTime;
}
