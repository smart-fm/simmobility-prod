/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Passenger.cpp
 *
 *  Created on: 2012-12-20
 *      Author: Meenu
 */

#include "Passenger.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"
#include "entities/AuraManager.hpp"
#include "geospatial/Node.hpp"
#include "util/OutputUtil.hpp"
#include "util/GeomHelpers.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "util/DebugFlags.hpp"
#include "geospatial/Lane.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/aimsun/Loader.hpp"
#ifdef SIMMOB_NEW_SIGNAL
#include "entities/signal/Signal.hpp"
#else
#include "entities/Signal.hpp"
#endif
#include "util/GeomHelpers.hpp"
#include "geospatial/Point2D.hpp"

#include <vector>
#include <iostream>
#include<cmath>

#include "util/DynamicVector.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include <boost/unordered_map.hpp>
using namespace sim_mob;
using std::vector;
using std::cout;
using std::map;
using std::string;
//int estimated_boarding_passengers_no=0;
pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;

/*std::pair<const BusStop*, double> calcNearestBusStop(const RoadSegment* rs, const DPoint& pos, double stoppingDist) {
	typedef std::map<centimeter_t, const RoadItem*>::const_iterator RoadObstIt;

	std::pair<const BusStop*, double> res(nullptr, 0);
	for(RoadObstIt o_it=rs->obstacles.begin(); o_it!=rs->obstacles.end(); o_it++) {
		const BusStop* bs = dynamic_cast<const BusStop*>(o_it->second);
		if(!bs) {
			continue;
		}

		//Check if it's closer.
		double newDist = sim_mob::dist(bs->xPos, bs->yPos, pos.x, pos.y);
		if ((!res.first) || newDist<res.second) {
			res.first = bs;
			res.second = newDist;

			//Stop early?
			if (newDist < stoppingDist) {
				break;
			}
		}
	}

	return res;
}
*/

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
	if(parent->originNode->getID() == 75780)
	{
		parent->xPos.set(37222809);
		parent->yPos.set(14331981);

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
		 parent->xPos.set(37290000);//75822
		 parent->yPos.set(14390200);
	 }
	 else
	 {
		 parent->xPos.set(parent->originNode->location.getX());
		 parent->yPos.set(parent->originNode->location.getY());
		 std::cout<<"origin-x"<<parent->originNode->location.getX()<<std::endl;
	 }
  //   std::cout<<"random_num1"<<random_num1<<std::endl;
   //  std::cout<<"random_num2"<<random_num2<<std::endl;
   //  std::cout<<"parent->originNode->location.getX()+random_num1"<<parent->originNode->location.getX()+random_num1<<std::endl;
   //  std::cout<<"parent->originNode->location.getY()+random_num2"<<parent->originNode->location.getY()+random_num2<<std::endl;
//	parent->xPos.set(parent->originNode->location.getX()+random_num1);
 //   parent->yPos.set(parent->originNode->location.getY()+random_num2);
	 WaitingAtBusStop.set(true);
	 boardedBus.set(false);
	 alightedBus.set(false);
	 DestReached.set(false);
	 this->TimeofReachingBusStop=this->parent->getStartTime();
	 //this->TimeofBoardingBus= 0;
	// StreetDirectory::LaneAndIndexPair ln=StreetDirectory::instance().getLane(parent->originNode->location);
	/* vector<WayPoint> wp_path = StreetDirectory::instance().SearchShortestWalkingPath(parent->originNode->location, parent->destNode->location);
	 for (vector<WayPoint>::iterator it = wp_path.begin(); it != wp_path.end(); it++)
	 {ge
		 if (it->type_ == WayPoint::BUS_STOP)
		 {
			     parent->xPos.set(it->busStop_->xPos);
			 	 parent->yPos.set(it->busStop_->yPos);
		 }
		 if (it->type_ == WayPoint::SIDE_WALK)
		 {
			 parent->xPos.set(it->lane_->polyline_[0].getX());
			 parent->yPos.set(it->lane_->polyline_[0].getY());
		 }
	 }*
	 /*cStart_busstop_X=parent->originNode->location.getX();
	 cStart_busstop_Y=parent->originNode->location.getY();
	 cEnd_busstop_X=parent->destNode->location.getX();
	 cEnd_busstop_Y=parent->destNode->location.getY();*/
	// const Lane* lane;

	//lane=ln.lane_;
	/*if(lane!=NULL)
	{
		 DPoint xy=DPoint(parent->originNode->location.getX(),parent->originNode->location.getY());
		RoadSegment* rs=lane->getRoadSegment();
	//	std::pair<const BusStop*, double> nearestBS = calcNearestBusStop(rs, xy, 1800);

		if(rs->Pavement()!=NULL)
		{
		parent->xPos.set(rs->Pavement().polyline[0].getX());
	    parent->yPos.set(rs->Pavement().polyline[0].getY());
		}
	}*/


//	centimeter_t side = 1;
//    const vector<RoadSegment*> rs;
//    const RoadSegment* segm;
   //int i=0;
    //bool first=true;
    //int prevx=0,prevy=0;
    //double distance1,distance2;
    //int size;
//    int i=0;
   // const Link* link= StreetDirectory::instance().getLinkLoc(parent->originNode);
  /*  vector<WayPoint> wp_path= StreetDirectory::instance().SearchShortestWalkingPath(parent->originNode->location,parent->originNode->location);
    for (vector<WayPoint>::iterator it = wp_path.begin(); it != wp_path.end(); it++) {

    				if (it->type_ == WayPoint::SIDE_WALK) {
    					//Save
    				}
    				RoadSegment* rs = it->lane_->getRoadSegment();
    				int cx1=rs->polyline[0].getX();
    				int cx2=rs->polyline[0].getY();
    				parent->xPos.set(cx1);
    				parent->xPos.set(cx2);
    }*/

  //  nk->g
   // rs=link->getPath();
   /* if(link!=NULL)
    {
    vector<RoadSegment*> segments = link->getPath();

    	int totalLen = 0;
    	for (vector<RoadSegment*>::iterator it=segments.begin(); it!=segments.end(); it++) {
    		totalLen += (*it)->length;
    		DPoint xy=DPoint(parent->originNode->location.getX(),parent->originNode->location.getY());
    		std::pair<const BusStop*, double> nearestBS = calcNearestBusStop(segments[i], xy, 1800);
    			   			 if(nearestBS.first!=NULL)
    			   			 {

    			   			  parent->xPos.set(nearestBS.first->xPos);
    			   			  parent->yPos.set(nearestBS.first->yPos);
    			   			 }
    			   			 else
    			   			 {

    			   			 }
    			   			 i++;
    	}

 //rs=link->getPath();
	//std::vector<StreetDirectory::RoadSegmentAndIndexPair> segments;
    }*/
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
	 	  if ((abs(( busStops[k]->xPos - this->getDestPosition().getX())/1000)<= 3 )  and (abs((busStops[k]->yPos - this->getDestPosition().getY())/1000)<= 3))
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
	 //	 else
	 		// return false;
	 }
	 return false;
 }
 bool sim_mob::Passenger::PassengerAlightBus(Bus* bus,int xpos_approachingbusstop,int ypos_approachingbusstop,BusDriver* busdriver)
  {

     if ((abs((xpos_approachingbusstop/1000)-(this->getDestPosition().getX()/1000))<=2)  and (abs((ypos_approachingbusstop/1000)-(this->getDestPosition().getY()/1000))<=2))
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
	/* time_t rawtime;
	 struct tm * timeinfo;
	 time ( &rawtime );
	 timeinfo = localtime ( &rawtime );
	 printf ( "Current local time and date: %s", asctime (timeinfo) );
	 int seconds=timeinfo->tm_sec;
	 int minutes=timeinfo->tm_min;*/
 }
/* void sim_mob::Passenger::EstimateBoardingAlightingPassengers(Bus* bus)
 {

	//estimated_boarding_passengers_no=0;
	 StreetDirectory::LaneAndIndexPair ln=StreetDirectory::instance().getLane(parent->originNode->location);
	 const Lane* lane=ln.lane_;
	 std::vector<const sim_mob::Agent*> nearby_agents=sim_mob::AuraManager::instance().nearbyAgents(Point2D(this->getXYPosition().getX(), this->getXYPosition().getY()),*lane,3500,3500);
	 	for (vector<const Agent*>::iterator it = nearby_agents.begin(); it != nearby_agents.end(); it++)
	 	{
	 		const Person* person = dynamic_cast<const Person *> (*it);
	 		const BusDriver* busdriver = person ? dynamic_cast< BusDriver*>(person->getRole()) : nullptr;
	 		if (!busdriver)
	 		 {
	 			continue;
	 		 }
	 		std::vector<const BusStop*> busStops=busdriver->busStops;
	 		const Vehicle* vehicle=busdriver->getVehicle();
	 		const Bus* bus = dynamic_cast<const Bus*>(vehicle);
	 	//	busdriver->GetBusstops();
	 	//	std::vector<const sim_mob::BusStop*,vector::allocator_type<const sim_mob::BusStop*>> bus=busdriver->GetBusstops();
	 		if(this->isAtBusStop()==true)
	 		{
	 			for(int k=0;k< busStops.size();k++)
	 			{
	 				if ((abs(( busStops[k]->xPos - this->getDestPosition().getX())/1000)<= 2 )  and (abs((busStops[k]->yPos - this->getDestPosition().getY())/1000)<= 2))
	 					 		{
	 					        pthread_mutex_lock(&mu);
	 					           if(bus->getPassengerCount()+ estimated_boarding_passengers_no<=bus->getBusCapacity())//and(last_busstop==false))
	 					 		     {
	 					        	   estimated_boarding_passengers_no=estimated_boarding_passengers_no+1;
	 					 				 //if passenger is to be boarded,add to passenger vector inside the bus
	 					 	             // bus->setPassengerCount(bus->getPassengerCount()+1);
	 					 				 // this->passenger_inside_bus.set(true);//to indicate whether passenger is waiting at the bus stop or is inside the bus

	 					 			}
	 					          pthread_mutex_unlock(&mu);
	 					 		}
	 			}
	 		}
	 	}

 }
 */
 /*void sim_mob::Passenger::updateParentCoordinates(int x,int y)//to update passenger agent coordinates from bus driver class
 {
 	this->parent->xPos.set(x);
     this->parent->yPos.set(y);

 }*/

 /*
 int sim_mob::Passenger::getOriginX()
  {
    return parent->originNode->location.getX();
  }
  int sim_mob::Passenger::getOriginY()
  {
    return parent->originNode->location.getY();
  }
  */
 /*int sim_mob::Passenger::getXPosition()
 {
 	return parent->xPos.get();
 }
 int sim_mob::Passenger::getYPosition()
  {
     return parent->yPos.get();
  }
  int sim_mob::Passenger::getDestX()
  {
    return parent->destNode->location.getX();
  }
  int sim_mob::Passenger::getDestY()
  {
    return parent->destNode->location.getY();
  }
 */
