/* Copyright Singapore-MIT Alliance for Research and Technology */
/*
 * Passenger.cpp
 * Created on: 2012-12-20
 * Author: Meenu
 */
#include "Passenger.hpp"
#include "entities/Person.hpp"
#include "entities/vehicle/Bus.hpp"
using namespace sim_mob;
using std::vector;
using std::cout;
using std::map;
using std::string;

BusStop* getBusStop(const Node* node,sim_mob::RoadSegment* segment)
  {
 	 std::map<centimeter_t, const RoadItem*>::const_iterator ob_it;
 	 const std::map<centimeter_t, const RoadItem*> & obstacles =segment->obstacles;
 	 for (ob_it = obstacles.begin(); ob_it != obstacles.end(); ++ob_it) {
 		RoadItem* ri = const_cast<RoadItem*>(ob_it->second);
 		BusStop *bs = dynamic_cast<BusStop*>(ri);
 		if (bs && ((segment->getStart() == node) || (segment->getEnd() == node) )) {
 			return bs;
 		}
 	 }

 	 return nullptr;
  }

sim_mob::Passenger::Passenger(Agent* parent, MutexStrategy mtxStrat, std::string roleName) : Role(parent,roleName),
	 BoardedBus(mtxStrat,false), AlightedBus(mtxStrat,false),
	 busdriver(mtxStrat,nullptr), displayX(0), displayY(0),skip(0),
	WaitingTime(-1), TimeOfReachingBusStop(0), params(parent->getGenerator())
{
	alighting_Frame = 0;
}
void sim_mob::Passenger::setParentBufferedData()
{
//	//if passenger inside bus,update position of the passenger agent(inside bus)every frame tick
//	if((isAtBusStop()==false)and(this->busdriver.get()!=NULL))
//	{
//		//passenger x,y position equals the bus drivers x,y position as passenger is inside the bus
//		parent->xPos.set(this->busdriver.get()->getPositionX());
//		parent->yPos.set(this->busdriver.get()->getPositionY());
//	}
	if(busdriver.get()!=nullptr)
	{
		parent->xPos.set(this->busdriver.get()->getPositionX());
		parent->yPos.set(this->busdriver.get()->getPositionY());
	}
}

void sim_mob::Passenger::frame_init(UpdateParams& p)
{
   //initialization
   WaitingTime = -1;
   OriginBusStop=setBusStopXY(parent->originNode);
   parent->xPos.set(OriginBusStop->xPos);
   parent->yPos.set(OriginBusStop->yPos);
   DestBusStop=setBusStopXY(parent->destNode);
   TimeOfReachingBusStop=p.now.ms();
   Person* person = dynamic_cast<Person*> (parent);
   if(person) {
	   person->setNextRole(nullptr);// set nextRole to be nullptr when becoming Passenger
   }
   FindBusLines();//to find which bus lines the passenger wants to board based on busline info at busstop
//   Person* person = dynamic_cast<Person*> (parent);
//   if(person && (!busdriver.get())) {
//	   const RoleFactory& rf = ConfigParams::GetInstance().getRoleFactory();
//	   sim_mob::Role* newRole = rf.createRole("waitBusActivityRole", person);
//	   person->setTempRole(newRole);
//   }
}

UpdateParams& sim_mob::Passenger::make_frame_tick_params(timeslice now)
{
	params.reset(now);
	return params;
}

//Main update method
void sim_mob::Passenger::frame_tick(UpdateParams& p)
{
	if(0 != alighting_Frame) {
		if(alighting_Frame == p.now.frame()) {
//			Person* person = dynamic_cast<Person*> (parent);
//			if(!person->findPersonNextRole())// find and assign the nextRole to this Person, when this nextRole is set to be nullptr?
//			{
//				std::cout << "End of trip chain...." << std::endl;
//			}
//			Pedestrian2* pedestrian2 = dynamic_cast<Pedestrian2*> (person->getNextRole());
//			if(pedestrian2) {
//				pedestrian2->frame_init(p);
//				person->changeRole(person->getNextRole());
//			} else {
//				parent->setToBeRemoved();//removes passenger if destination is reached
//			}
			parent->setToBeRemoved();//removes passenger if destination is reached
			busdriver.set(nullptr);// assign this busdriver to Passenger
			BoardedBus.set(false);
			AlightedBus.set(true);
		} else {
			setParentBufferedData();//update passenger coordinates every frame tick
		}
	}

//	if(AlightedBus.get()==true)
//	{
//		AlightedBus.set(false);
//		parent->setToBeRemoved();//removes passenger if destination is reached
//	}
}


void sim_mob::Passenger::frame_tick_output(const UpdateParams& p)
{
	if (ConfigParams::GetInstance().using_MPI) {
		return;
	}

	//Reset our offset if it's set to zero
	if (DisplayOffset.getX()==0 && DisplayOffset.getY()==0) {
	   boost::mt19937 gen(static_cast<unsigned int>(parent->getId()*parent->getId()));
	   boost::uniform_int<> distX(0, 249);
	   boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varX(gen, distX);
	   boost::uniform_int<> distY(0, 99);
	   boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varY(gen, distY);
	   unsigned int value = (unsigned int)varX();
	   DisplayOffset.setX(value+1);
	   value= (unsigned int)varY();
	   DisplayOffset.setY(value+1);
	}

	if((BoardedBus.get()==false) && (AlightedBus.get()==false)) {
		//output passenger on visualizer only if passenger on road
		LogOut("("<<"\"passenger\","<<p.now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<(parent->xPos.get()+DisplayOffset.getX()+DisplayOffset.getX())<<"\"," <<"\"yPos\":\""<<(parent->yPos.get()+DisplayOffset.getY()+DisplayOffset.getY())<<"\",})"<<std::endl);
	} else if((BoardedBus.get()==false) && (AlightedBus.get()==true)) {
		//output passenger on visualizer only if passenger on road
		 LogOut("("<<"\"passenger\","<<p.now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<(displayX-DisplayOffset.getX()-DisplayOffset.getX())<<"\"," <<"\"yPos\":\""<<(displayY-DisplayOffset.getY()-DisplayOffset.getY())<<"\",})"<<std::endl);
	}
}

void sim_mob::Passenger::frame_tick_output_mpi(timeslice now)
{
	//Reset our offset if it's set to zero
	if (DisplayOffset.getX()==0 && DisplayOffset.getY()==0) {
	   boost::mt19937 gen(static_cast<unsigned int>(parent->getId()*parent->getId()));
	   boost::uniform_int<> distX(0, 249);
	   boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varX(gen, distX);
	   boost::uniform_int<> distY(0, 99);
	   boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varY(gen, distY);
	   unsigned int value = (unsigned int)varX();
	   DisplayOffset.setX(value+1);
	   value= (unsigned int)varY();
	   DisplayOffset.setY(value+1);
	}
	if (now.frame() < 1 || now.frame() < parent->getStartTime())
			return;
	if((BoardedBus.get()==false) && (AlightedBus.get()==false))//output passenger on visualizer only if passenger on road
	{
		if (this->parent->isFake) {
			LogOut("("<<"\"passenger\","<<now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<(parent->xPos.get()+DisplayOffset.getX()+DisplayOffset.getX())<<"\"," <<"\"yPos\":\""<<(parent->yPos.get()+DisplayOffset.getY()+DisplayOffset.getY()) <<"\"," <<"\"fake\":\""<<"true" <<"\",})"<<std::endl);
		} else {
			LogOut("("<<"\"passenger\","<<now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<(parent->xPos.get()+DisplayOffset.getX()+DisplayOffset.getX())<<"\"," <<"\"yPos\":\""<<(parent->yPos.get()+DisplayOffset.getY()+DisplayOffset.getY())<<"\"," <<"\"fake\":\""<<"false" <<"\",})"<<std::endl);
		}
	}
	else if((BoardedBus.get()==false) && (AlightedBus.get()==true))//output passenger on visualizer only if passenger on road
	{
		if (this->parent->isFake) {
			LogOut("("<<"\"passenger\","<<now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<(displayX-DisplayOffset.getX()-DisplayOffset.getX())<<"\"," <<"\"yPos\":\""<<(displayY-DisplayOffset.getY()-DisplayOffset.getY())<<"\"," <<"\"fake\":\""<<"true" <<"\",})"<<std::endl);
		} else {
			LogOut("("<<"\"passenger\","<<now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<(displayX-DisplayOffset.getX()-DisplayOffset.getX())<<"\"," <<"\"yPos\":\""<<(displayY-DisplayOffset.getY()-DisplayOffset.getY())<<"\"," <<"\"fake\":\""<<"false" <<"\",})"<<std::endl);
		}
	}

}

/*void sim_mob::Passenger::update(timeslice now)
{

}*/

bool sim_mob::Passenger::isAtBusStop()
{
	if(BoardedBus.get()==false)
	   return true;
	else
	  return false;
}

bool sim_mob::Passenger::isDestBusStopReached() {

	if(AlightedBus.get()==true)//passenger alights when destination is reached
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
	return Point2D((DestBusStop->xPos),(DestBusStop->yPos));
}



bool sim_mob::Passenger::PassengerAlightBus(BusDriver* busdriver)
{
	Bus* bus = dynamic_cast<Bus*>(busdriver->getVehicle());
	int xpos_approachingbusstop=busdriver->xpos_approachingbusstop;
	int ypos_approachingbusstop=busdriver->ypos_approachingbusstop;
	 if (xpos_approachingbusstop-getDestPosition().getX()==0 && ypos_approachingbusstop-getDestPosition().getY()==0)
	 {
    	 //alight-delete passenger agent from list
    	 bus->setPassengerCount(bus->getPassengerCount()-1);
    	 this->busdriver.set(nullptr);	//driver would be null as passenger has alighted
    	 AlightedBus.set(true);
    	 BoardedBus.set(false);
    	 parent->xPos.set(xpos_approachingbusstop);
    	 parent->yPos.set(ypos_approachingbusstop);
    	 displayX = xpos_approachingbusstop;
    	 displayY = ypos_approachingbusstop;
    	 return true;
	 }
     return false;
}

std::vector<sim_mob::BufferedBase*> sim_mob::Passenger::getSubscriptionParams()
{
 	std::vector<sim_mob::BufferedBase*> res;
 	res.push_back(&(BoardedBus));
 	res.push_back(&(AlightedBus));
 	res.push_back(&(busdriver));
 	return res;
}

bool sim_mob::Passenger::isBusBoarded()
{
	return (BoardedBus.get()==true);
}

void sim_mob::Passenger::findWaitingTime(Bus* bus)
{
	WaitingTime=(bus->TimeOfBusreachingBusstop)-(TimeOfReachingBusStop);
}
BusStop* sim_mob::Passenger::setBusStopXY(const Node* node)//to find the nearest busstop to a node
  {
 	 const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (node);
 	 double dist=0;
 	 BusStop*bs1;
 	 if(currEndNode)
 	 {
 		 const std::set<sim_mob::RoadSegment*>& segments_ = currEndNode->getRoadSegments();
 		 BusStop* busStop_ptr = nullptr;
 		 for(std::set<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i !=segments_.end();i++)
 		 {
 			sim_mob::BusStop* bustop_ = (*i)->getBusStop();
 			busStop_ptr = getBusStop(node,(*i));
 			if(busStop_ptr)
 			{
 			double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,node->location.getX(), node->location.getY());
 			if((newDist<dist || dist==0)&& busStop_ptr->BusLines.size()!=0)
 			   {
 			     dist=newDist;
 				 bs1=busStop_ptr;
 			   }
 			}
 		 }
 	 }
 	 else
 	 {
 		 Point2D point = node->location;
 		 const StreetDirectory::LaneAndIndexPair lane_index =  StreetDirectory::instance().getLane(point);
 		 if(lane_index.lane_)
 		 {
 			 sim_mob::Link* link_= lane_index.lane_->getRoadSegment()->getLink();
 			 const sim_mob::Link* link_2 = StreetDirectory::instance().searchLink(link_->getEnd(),link_->getStart());
 			 BusStop* busStop_ptr = nullptr;

 			 std::vector<sim_mob::RoadSegment*> segments_ ;

 			 if(link_)
 			 {
 				 segments_= const_cast<Link*>(link_)->getSegments();
 				 for(std::vector<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i != segments_.end();i++)
 				 {
 					sim_mob::BusStop* bustop_ = (*i)->getBusStop();
 					busStop_ptr = getBusStop(node,(*i));
 					if(busStop_ptr)
 					{
 						double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,point.getX(), point.getY());
 						if((newDist<dist || dist==0)&& busStop_ptr->BusLines.size()!=0)
 						 {
 						 	dist=newDist;
 						 	bs1=busStop_ptr;
 						 }
 					}
 				 }
 			 }

 			 if(link_2)
 			 {
 				 segments_ = const_cast<Link*>(link_2)->getSegments();
 				 for(std::vector<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i != segments_.end();i++)
 				 {
 					sim_mob::BusStop* bustop_ = (*i)->getBusStop();
 					busStop_ptr = getBusStop(node,(*i));
 					if(busStop_ptr)
 					{
 						double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,point.getX(), point.getY());
 						if((newDist<dist || dist==0)&& busStop_ptr->BusLines.size()!=0)
 					    {
 						   dist=newDist;
 						   bs1=busStop_ptr;
 						 }
 					}
 				 }
 			 }
 		 }

 	 }

 	 return bs1;
  }

bool sim_mob::Passenger::PassengerBoardBus_Choice(BusDriver* busdriver)
 {
 	for(int i=0;i<BuslinesToTake.size();i++)
 	{
 		 Bus* bus = dynamic_cast<Bus*>(busdriver->getVehicle());
 		if(BuslinesToTake[i]->getBusLineID()==bus->getBusLineID())//boards if approaching busline of approaching busline
 		//is in the pre-decided busline list
 		{
 		  if(bus->getPassengerCount()+1<=bus->getBusCapacity())
 		 {
 			Person* p=dynamic_cast<Person*>(this->getParent());
 			bus->passengers_inside_bus.push_back(p);
 			bus->setPassengerCount(bus->getPassengerCount()+1);
 			this->busdriver.set(busdriver);//passenger should store the bus driver
 		    BoardedBus.set(true);//to indicate passenger has boarded bus
 			AlightedBus.set(false);//to indicate whether passenger has alighted bus
 	        findWaitingTime(bus);
 	        BuslinesToTake.clear();
 			return true;
 		 }
 		}
 	}
 	return false;
  }
void sim_mob::Passenger::FindBusLines() //find bus lines there and decide which line to board based on shortest path
 {
	 if(BoardedBus.get()==false)
	 {
		 vector<Busline*> buslines=OriginBusStop->BusLines;//list of available buslines at busstop
         int prev=0;
		 for(int i=0;i<buslines.size();i++)
		 {
		  /*query through the busstops for each available busline at the busstop
		  and see if it goes to the passengers destination.If more than one busline avaiable
		  choose the busline with the shortest path*/

		  const std::vector<BusTrip>& BusTrips = buslines[i]->queryBusTrips();

		  //busstops has the info about the list of bus stops a particular bus line goes to
		  std::vector<const::BusStop*> busstops=BusTrips[0].getBusRouteInfo().getBusStops();

		  std::vector<const::BusStop*>::iterator it;
		  int noOfBusstops=0;

		  //queries through list of bus stops of bus line starting from current bus stop to end bus stop
		  //this is to see if the particular bus line has the destination bus stop of passenger
		  //if yes and if its shortest available route, the bus line is added to the list of bus lines passenger is supposed to take
		  for(it=++std::find(busstops.begin(),busstops.end(),OriginBusStop);it!=busstops.end();it++)
		  {
   			  BusStop* bs=const_cast<BusStop*>(*it);

   			  //checking if the bus stop is the destination bus stop of passenger
			  if(bs==DestBusStop)//add this bus line to the list,if it is the shortest
			  {
				  if(prev==0 || prev> noOfBusstops)
				  {
					  BuslinesToTake.clear();
					  BuslinesToTake.push_back(buslines[i]);
				      prev=noOfBusstops;
				  }
				  else if(prev==noOfBusstops)
				  {
					  BuslinesToTake.push_back(buslines[i]);//update the list of bus lines passenger is supposed to take
					  prev=noOfBusstops;
				  }
			  }
			  noOfBusstops++;
		  }
		 }
	 }
 }
std::vector<Busline*> sim_mob::Passenger::ReturnBusLines()
{
	 return BuslinesToTake;
}
/*bool sim_mob::Passenger::PassengerBoardBus_Normal(BusDriver* busdriver,std::vector<const BusStop*> busStops)
{
   int busstop_sequence_no=busdriver->busstop_sequence_no.get()+1;
	//chcking if destinaton node position of passenger is in the list of bus stops which the bus would stop
	for(int k=busstop_sequence_no;k < busStops.size();k++) {
		  Point2D busStop_Pos(busStops[k]->xPos,busStops[k]->yPos);
	 	  if((abs((busStop_Pos.getX() - getDestPosition().getX())/100)<= 3) and (abs((busStop_Pos.getY() - getDestPosition().getY())/100)<= 3))
	 	  {
	 		 Bus* bus = dynamic_cast<Bus*>(busdriver->getVehicle());
	           if(bus->getPassengerCount()+1<=bus->getBusCapacity())//and(last_busstop==false))
	           {
	        	   Person* p=dynamic_cast<Person*>(this->getParent());
	        	   //if passenger is to be boarded,add to passenger vector inside the bus
	        	   bus->passengers_inside_bus.push_back(p);
	        	   bus->setPassengerCount(bus->getPassengerCount()+1);
	        	   this->busdriver.set(busdriver);//passenger should store the bus driver
	        	   BoardedBus.set(true);//to indicate passenger has boarded bus
	        	   AlightedBus.set(false);//to indicate whether passenger has alighted bus
	        	   findWaitingTime(bus);
	        	   return true;
	           }
	 	  }
	 }
	 return false;
}*/
