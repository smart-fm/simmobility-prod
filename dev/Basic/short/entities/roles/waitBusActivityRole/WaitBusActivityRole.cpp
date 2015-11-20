//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "WaitBusActivityRole.hpp"

#include "entities/Person.hpp"
#include "entities/vehicle/Bus.hpp"
#include "entities/Person_ST.hpp"

using std::vector;
using namespace sim_mob;

WaitBusActivityRole::WaitBusActivityRole(Person_ST *parent, WaitBusActivityRoleBehavior *behavior, WaitBusActivityRoleMovement *movement,
												  Role::Type roleType_, std::string roleName)
: Role(parent, behavior, movement, roleName, roleType_), params(parent->getGenerator()), TimeOfReachingBusStop(0), waitingTimeAtBusStop(0)
{
}

WaitBusActivityRole::~WaitBusActivityRole()
{
}

void WaitBusActivityRole::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}

std::vector<BufferedBase*> WaitBusActivityRole::getSubscriptionParams()
{
	vector<BufferedBase*> res;
	return res;
}

//BusStop* WaitBusActivityRole::setBusStopXY(const Node* node)//to find the nearest busstop to a node
//{
// 	 const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (node);
// 	 double dist=0;
// 	 BusStop*bs1=0;
// 	 if(currEndNode)
// 	 {
// 		 const std::set<RoadSegment*>& segments_ = currEndNode->getRoadSegments();
// 		 BusStop* busStop_ptr = nullptr;
// 		 for(std::set<RoadSegment*>::const_iterator i = segments_.begin();i !=segments_.end();i++)
// 		 {
// 			BusStop* bustop_ = (*i)->getBusStop();
// 			busStop_ptr = getbusStop(node,(*i));
// 			if(busStop_ptr)
// 			{
// 			double newDist = dist(busStop_ptr->xPos, busStop_ptr->yPos,node->location.getX(), node->location.getY());
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
// 		 Point point = node->location;
// 		 const StreetDirectory::LaneAndIndexPair lane_index =  StreetDirectory::Instance().getLane(point);
// 		 if(lane_index.lane_)
// 		 {
// 			 Link* link_= lane_index.lane_->getRoadSegment()->getLink();
// 			 const Link* link_2 = StreetDirectory::Instance().searchLink(link_->getEnd(),link_->getStart());
// 			 BusStop* busStop_ptr = nullptr;
//
// 			 std::vector<RoadSegment*> segments_ ;
//
// 			 if(link_)
// 			 {
// 				 segments_= const_cast<Link*>(link_)->getSegments();
// 				 for(std::vector<RoadSegment*>::const_iterator i = segments_.begin();i != segments_.end();i++)
// 				 {
// 					BusStop* bustop_ = (*i)->getBusStop();
// 					busStop_ptr = getbusStop(node,(*i));
// 					if(busStop_ptr)
// 					{
// 						double newDist = dist(busStop_ptr->xPos, busStop_ptr->yPos,point.getX(), point.getY());
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
// 				 for(std::vector<RoadSegment*>::const_iterator i = segments_.begin();i != segments_.end();i++)
// 				 {
// 					BusStop* bustop_ = (*i)->getBusStop();
// 					busStop_ptr = getbusStop(node,(*i));
// 					if(busStop_ptr)
// 					{
// 						double newDist = dist(busStop_ptr->xPos, busStop_ptr->yPos,point.getX(), point.getY());
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
