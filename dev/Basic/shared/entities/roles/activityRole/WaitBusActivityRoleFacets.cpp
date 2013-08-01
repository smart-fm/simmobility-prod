/*
 * WaitBusActivityRoleFacets.cpp
 *
 *  Created on: Jun 17th, 2013
 *      Author: Yao Jin
 */

#include "WaitBusActivityRoleFacets.hpp"

#include "entities/Person.hpp"
#include "geospatial/Link.hpp"
#include "util/GeomHelpers.hpp"

using namespace sim_mob;

namespace sim_mob {

BusStop* getbusStop(const Node* node,sim_mob::RoadSegment* segment)
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

WaitBusActivityRoleBehavior::WaitBusActivityRoleBehavior(sim_mob::Person* parentAgent):
	BehaviorFacet(parentAgent), parentWaitBusActivityRole(nullptr) {}

WaitBusActivityRoleBehavior::~WaitBusActivityRoleBehavior() {}

//void WaitBusActivityRoleBehavior::frame_init(UpdateParams& p) {
//	throw std::runtime_error("WaitBusActivityRoleBehavior::frame_init is not implemented yet");
//}
//
//void WaitBusActivityRoleBehavior::frame_tick(UpdateParams& p) {
//	throw std::runtime_error("WaitBusActivityRoleBehavior::frame_tick is not implemented yet");
//}
//
//void WaitBusActivityRoleBehavior::frame_tick_output(const UpdateParams& p) {
//	throw std::runtime_error("WaitBusActivityRoleBehavior::frame_tick_output is not implemented yet");
//}
//
//void WaitBusActivityRoleBehavior::frame_tick_output_mpi(timeslice now) {
//	throw std::runtime_error("WaitBusActivityRoleBehavior::frame_tick_output_mpi is not implemented yet");
//}

sim_mob::WaitBusActivityRoleMovement::WaitBusActivityRoleMovement(sim_mob::Person* parentAgent, std::string buslineid):
		MovementFacet(parentAgent), parentWaitBusActivityRole(nullptr), busStopAgent(nullptr), registered(false),
		buslineid(buslineid), boarding_MS(0), busDriver(nullptr)
{

}

sim_mob::WaitBusActivityRoleMovement::~WaitBusActivityRoleMovement() {

}

//void sim_mob::WaitBusActivityRoleMovement::frame_init(UpdateParams& p) {
//
//}
//
//void sim_mob::WaitBusActivityRoleMovement::frame_tick(UpdateParams& p) {
//
//}
//
//void sim_mob::WaitBusActivityRoleMovement::frame_tick_output(const UpdateParams& p) {
//
//}
//
//void sim_mob::WaitBusActivityRoleMovement::frame_tick_output_mpi(timeslice now) {
//
//}
//
//void sim_mob::WaitBusActivityRoleMovement::flowIntoNextLinkIfPossible(UpdateParams& p) {
//
//}

BusStop* sim_mob::WaitBusActivityRoleMovement::setBusStopXY(const Node* node)//to find the nearest busstop to a node
{
 	 const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (node);
 	 double dist=0;
 	 BusStop*bs1=0;
 	 if(currEndNode)
 	 {
 		 const std::set<sim_mob::RoadSegment*>& segments_ = currEndNode->getRoadSegments();
 		 BusStop* busStop_ptr = nullptr;
 		 for(std::set<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i !=segments_.end();i++)
 		 {
 			sim_mob::BusStop* bustop_ = (*i)->getBusStop();
 			busStop_ptr = getbusStop(node,(*i));
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
 					busStop_ptr = getbusStop(node,(*i));
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
 					busStop_ptr = getbusStop(node,(*i));
 					if(busStop_ptr)
 					{
 						double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,point.getX(), point.getY());
 						std::cout << "busStop_ptr->BusLines.size(): " << busStop_ptr->BusLines.size() << std::endl;
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
}
