//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "WaitBusActivityRole.hpp"

#include "entities/Person.hpp"
#include "entities/vehicle/Bus.hpp"

using std::vector;
using namespace sim_mob;

sim_mob::WaitBusActivityRole::WaitBusActivityRole(Agent* parent, sim_mob::WaitBusActivityRoleBehavior* behavior, sim_mob::WaitBusActivityRoleMovement* movement, Role::type roleType_, std::string roleName) :
		Role(behavior, movement, parent, roleName, roleType_), params(parent->getGenerator()), TimeOfReachingBusStop(0), waitingTimeAtBusStop(0)
{
}

sim_mob::WaitBusActivityRole::~WaitBusActivityRole() {
}

void sim_mob::WaitBusActivityRole::make_frame_tick_params(timeslice now){
	getParams().reset(now);
}

std::vector<sim_mob::BufferedBase*> sim_mob::WaitBusActivityRole::getSubscriptionParams() {
	vector<BufferedBase*> res;
	return res;
}

//BusStop* sim_mob::WaitBusActivityRole::setBusStopXY(const Node* node)//to find the nearest busstop to a node
//{
// 	 const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (node);
// 	 double dist=0;
// 	 BusStop*bs1=0;
// 	 if(currEndNode)
// 	 {
// 		 const std::set<sim_mob::RoadSegment*>& segments_ = currEndNode->getRoadSegments();
// 		 BusStop* busStop_ptr = nullptr;
// 		 for(std::set<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i !=segments_.end();i++)
// 		 {
// 			sim_mob::BusStop* bustop_ = (*i)->getBusStop();
// 			busStop_ptr = getbusStop(node,(*i));
// 			if(busStop_ptr)
// 			{
// 			double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,node->location.getX(), node->location.getY());
// 			if((newDist<dist || dist==0)&& busStop_ptr->BusLines.size()!=0)
// 			   {
// 			     dist=newDist;
// 				 bs1=busStop_ptr;
// 			   }
// 			}
// 		 }
// 	 }
// 	 else
// 	 {
// 		 Point2D point = node->location;
// 		 const StreetDirectory::LaneAndIndexPair lane_index =  StreetDirectory::instance().getLane(point);
// 		 if(lane_index.lane_)
// 		 {
// 			 sim_mob::Link* link_= lane_index.lane_->getRoadSegment()->getLink();
// 			 const sim_mob::Link* link_2 = StreetDirectory::instance().searchLink(link_->getEnd(),link_->getStart());
// 			 BusStop* busStop_ptr = nullptr;
//
// 			 std::vector<sim_mob::RoadSegment*> segments_ ;
//
// 			 if(link_)
// 			 {
// 				 segments_= const_cast<Link*>(link_)->getSegments();
// 				 for(std::vector<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i != segments_.end();i++)
// 				 {
// 					sim_mob::BusStop* bustop_ = (*i)->getBusStop();
// 					busStop_ptr = getbusStop(node,(*i));
// 					if(busStop_ptr)
// 					{
// 						double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,point.getX(), point.getY());
// 						if((newDist<dist || dist==0)&& busStop_ptr->BusLines.size()!=0)
// 						 {
// 						 	dist=newDist;
// 						 	bs1=busStop_ptr;
// 						 }
// 					}
// 				 }
// 			 }
//
// 			 if(link_2)
// 			 {
// 				 segments_ = const_cast<Link*>(link_2)->getSegments();
// 				 for(std::vector<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i != segments_.end();i++)
// 				 {
// 					sim_mob::BusStop* bustop_ = (*i)->getBusStop();
// 					busStop_ptr = getbusStop(node,(*i));
// 					if(busStop_ptr)
// 					{
// 						double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,point.getX(), point.getY());
// 						std::cout << "busStop_ptr->BusLines.size(): " << busStop_ptr->BusLines.size() << std::endl;
// 						if((newDist<dist || dist==0)&& busStop_ptr->BusLines.size()!=0)
// 					    {
// 						   dist=newDist;
// 						   bs1=busStop_ptr;
// 						 }
// 					}
// 				 }
// 			 }
// 		 }
//
// 	 }
// 	 return bs1;
//}
