//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "MultiNode.hpp"

#include <cstdio>
#include <cmath>

#include "conf/simpleconf.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"

using namespace sim_mob;

using std::pair;
using std::vector;
using std::set;
using std::map;



namespace {

//Compute the clockwise angle between two vectors with a common center point.
// Returns the angle from "first" to "second", in the range 0 <= res < 2PI (NOTE: That might not be right)
double AngleBetween(const Node* const center, const Node* const first, const Node* const second, bool readClockwise)
{
	//Quick check: Force 0 if the two points are the same (we wouldn't want them to appear as 2PI)
	if (first->location == second->location) {
		return 0.0;
	}

	//Subtract the vectors
	double v1x = first->location.getX() - center->location.getX();
	double v1y = first->location.getY() - center->location.getY();
	double v2x = second->location.getX() - center->location.getX();
	double v2y = second->location.getY() - center->location.getY();

	//NOTE: I'm pretty sure atan2 already handles this... but do we have to bound it to 0<=res,+2PI?
	double res = atan2(v2x, v2y) - atan2(v1x, v1y);
	return res;
}


//Helper to manage bi-directional segments
void InsertIntoVector(vector< pair<RoadSegment*, bool> >& vec, vector< pair<RoadSegment*, bool> >::iterator pos, RoadSegment* item, const Node* const centerNode)
{
	//Insert it
	bool isFwd = item->getEnd()==centerNode;
	vector< pair<RoadSegment*, bool> >::iterator resIt = vec.insert(pos, std::make_pair(item, isFwd));

	//If it's bi-directional, insert the reverse segment.
	if (item->isBiDirectional()) {
		if (isFwd) {
			vec.insert(resIt, std::make_pair(item, !isFwd));
		} else {
			vec.insert(resIt+1, std::make_pair(item, !isFwd));
		}
	}
}




} //End unnamed namespace

bool sim_mob::MultiNode::canFindRoadSegment(sim_mob::RoadSegment* rs) const
{
	std::set<sim_mob::RoadSegment*>::const_iterator it = roadSegmentsAt.find(rs);
	if(it != roadSegmentsAt.end())
		return true;
	return false;
}

bool sim_mob::MultiNode::hasOutgoingLanes(const RoadSegment * from) const
{
	return connectors.count(from) > 0;
}


const set<LaneConnector*>& sim_mob::MultiNode::getOutgoingLanes(const RoadSegment * from) const
{
	if (!hasOutgoingLanes(from)) {
//			  if(this->getID() == 45666)
//			  {
////				  const std::set<sim_mob::RoadSegment*>& getRoadSegments() const { return roadSegmentsAt; }
//				  std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> >::const_iterator it = this->getConnectors().begin();
//				  for(; it != this->getConnectors().end(); it++)
//				  {
//					  std::cout << "(45666 : " << this << ") -->connectors[link-seg(" << from << "):" << (*it).first->getLink()->getLinkId() << ":" << (*it).first->getSegmentID() << "] size  is : " << (*it).second.size() << std::endl;
//				  }
//			  }
//			  std::vector<sim_mob::MultiNode*> mNodes = sim_mob::ConfigParams::GetInstance().getNetwork().getNodes();
//			  for(std::vector<sim_mob::MultiNode*>::iterator node_it = mNodes.begin(); node_it != mNodes.end(); node_it ++)
//			  {
//				  std::cout << "- ";
//				  if((*node_it)->getID() == 45666)
//				  {
//					  for(std::set<sim_mob::RoadSegment*>::const_iterator rs_it =  (*node_it)->getRoadSegments().begin(); rs_it != (*node_it)->getRoadSegments().end(); rs_it++)
//					  {
//						  sim_mob::RoadSegment * temp = (*rs_it);
//						  std::cout << "\n=>(45666 : " << (*node_it) << ") -->connectors[link-seg(" << temp << ") :" << (*rs_it)->getLink()->getLinkId() << ":" << (*rs_it)->getSegmentID() << "] size  is : " << (*node_it)->connectors[(*rs_it)].size() << std::endl;
//
//					  }
//				  }
//			  }


		//TODO: How are we handling logical errors?
		std::stringstream msg;
		msg <<"No outgoing Lane for Road Segments(" <<  from->getSegmentID() << ")  at node(" << this << "): " <<originalDB_ID.getLogItem();
		msg <<"   from node: " <<(from->getStart()==this?from->getEnd()->originalDB_ID.getLogItem():from->getStart()->originalDB_ID.getLogItem());
		msg <<"\nExisting connectors:";
		for (map<const RoadSegment*, set<LaneConnector*> >::const_iterator it=connectors.begin(); it!=connectors.end(); it++) {
			msg <<"\n" <<it->first->getStart()->originalDB_ID.getLogItem() <<" => " <<it->first->getEnd()->originalDB_ID.getLogItem();
		}
		throw std::runtime_error(msg.str().c_str());
	}

	return connectors.find(from)->second;
}




pair< vector< pair<RoadSegment*, bool> >, vector< pair<RoadSegment*, bool> > >
	sim_mob::MultiNode::getPedestrianPaths(const Node* const nodeBefore, const Node* const nodeAfter) const
{
	//TODO: Alone, this information is not sufficient: Pedestrians can (probably) walk "backwards"
	//      down the sidewalk on a single-directional RoadSegment. Perhaps we can use RoadSegment*, bool
	//      input/outputs?

	//TODO: Scan the circular array, build two result arrays. The Agent can then decide which
	//      of these two paths to take, and can then request Crossing information for each
	//      RoadSegment he needs to cross.
	throw std::runtime_error("Not implemented yet.");
}







