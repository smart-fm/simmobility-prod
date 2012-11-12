/* Copyright Singapore-MIT Alliance for Research and Technology */

/**
 * \file main.cpp
 * A first approximation of the basic pseudo-code in C++. The main file loads several
 * properties from data/config.xml, and attempts a simulation run. Currently, the various
 * granularities and pedestrian starting locations are loaded.
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 * \author Xu Yan
 */
#include <vector>
#include <string>
#include <ctime>

#include "GenConfig.h"

#include "workers/Worker.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "workers/WorkGroup.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "util/OutputUtil.hpp"
#include "util/DailyTime.hpp"

#ifdef SIMMOB_NEW_SIGNAL
	#include "entities/signal/Signal.hpp"
#else
	#include "entities/Signal.hpp"
#endif


#include "conf/simpleconf.hpp"
#include "entities/AuraManager.hpp"
#include "entities/TrafficWatch.hpp"
#include "entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/RoleFactory.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/roles/pedestrian/Pedestrian2.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Roundabout.hpp"
#include "geospatial/Intersection.hpp"
#include "geospatial/Route.hpp"
#include "perception/FixedDelayed.hpp"
#include "buffering/Buffered.hpp"
#include "buffering/Locked.hpp"
#include "buffering/Shared.hpp"

//add by xuyan
#include "partitions/PartitionManager.hpp"
#include "partitions/ParitionDebugOutput.hpp"

//Note: This must be the LAST include, so that other header files don't have
//      access to cout if SIMMOB_DISABLE_OUTPUT is true.
#include <iostream>
//#include "main1.hpp"
#include <tinyxml.h>
//#include "xmlWriter/xmlWriter.hpp"
using std::cout;
using std::endl;
using std::vector;
using std::string;

using namespace sim_mob;

//Start time of program
timeval start_time;

//Helper for computing differences. May be off by ~1ms
namespace {
int diff_ms(timeval t1, timeval t2) {
    return (((t1.tv_sec - t2.tv_sec) * 1000000) + (t1.tv_usec - t2.tv_usec))/1000;
}
} //End anon namespace

//Current software version.
const string SIMMOB_VERSION = string(SIMMOB_VERSION_MAJOR) + ":" + SIMMOB_VERSION_MINOR;


//void WriteXMLInput_Location(TiXmlElement * parent,bool underLocation, unsigned int X, unsigned int Y)
//{
//	std::ostringstream Id;
//	TiXmlElement * location;
//	//should x,y be sub elements of a rudimentary "location" element or just sub elements the parent
//	if(underLocation == true)
//		{location = new TiXmlElement("location"); parent->LinkEndChild(location);}
//	else
//		location = parent;
//
//	TiXmlElement * xPos = new TiXmlElement("xPos"); location->LinkEndChild(xPos);
//	Id.str("");
//	Id << X;
//	xPos->LinkEndChild(new  TiXmlText(Id.str()));
//
//	TiXmlElement * yPos = new TiXmlElement("yPos"); location->LinkEndChild(yPos);
//	Id.str("");
//	Id << Y;
//	yPos->LinkEndChild(new  TiXmlText(Id.str()));
//}
//
//void WriteXMLInput_PolyLine(const std::vector<sim_mob::Point2D> polylines,/*sim_mob::Lane *lane ,*/TiXmlElement * PolyLine)
//{
//	std::ostringstream Id;
//	int i = 0;
//	for(std::vector<Point2D>::const_iterator polyLineObj_it = polylines.begin(), it_end(polylines.end()); polyLineObj_it != it_end; polyLineObj_it++, i++)
//	{
//		//PolyPoint
//		TiXmlElement * PolyPoint = new TiXmlElement("PolyPoint"); PolyLine->LinkEndChild(PolyPoint);
//		TiXmlElement * pointID = new TiXmlElement("pointID"); PolyPoint->LinkEndChild(pointID);
//		Id.str("");
//		Id << i;
//		pointID->LinkEndChild(new  TiXmlText(Id.str()));
//		WriteXMLInput_Location(PolyPoint,true,polyLineObj_it->getX(), polyLineObj_it->getY());
//	}
//}
//
//void WriteXMLInput_Lane(sim_mob::Lane *LaneObj,TiXmlElement *Lanes)
//{
//	std::ostringstream Id;
//	TiXmlElement * Lane = new TiXmlElement("Lane"); Lanes->LinkEndChild(Lane);
//	//ID
//	TiXmlElement * laneID = new TiXmlElement("laneID"); Lane->LinkEndChild(laneID);
//	Id << LaneObj->getLaneID_str();
//	laneID->LinkEndChild(new  TiXmlText(Id.str()));
//	//Width
//	TiXmlElement * width = new TiXmlElement("width"); Lane->LinkEndChild(width);
//	Id.str("");
//	Id << LaneObj->getWidth();
//	width->LinkEndChild(new  TiXmlText(Id.str()));
////	can_go_straight
//	TiXmlElement * can_go_straight = new TiXmlElement("can_go_straight"); Lane->LinkEndChild(can_go_straight);
//	can_go_straight->LinkEndChild(new  TiXmlText(LaneObj->can_go_straight() ? "true" : "false"));
////	can_turn_left
//	TiXmlElement * can_turn_left = new TiXmlElement("can_turn_left"); Lane->LinkEndChild(can_turn_left);
//	can_turn_left->LinkEndChild(new  TiXmlText(LaneObj->can_turn_left() ? "true" : "false"));
////	can_turn_right
//	TiXmlElement * can_turn_right = new TiXmlElement("can_turn_right"); Lane->LinkEndChild(can_turn_right);
//	can_turn_right->LinkEndChild(new  TiXmlText(LaneObj->can_turn_right() ? "true" : "false"));
////	can_turn_on_red_signal
//	TiXmlElement * can_turn_on_red_signal = new TiXmlElement("can_turn_on_red_signal"); Lane->LinkEndChild(can_turn_on_red_signal);
//	can_turn_on_red_signal->LinkEndChild(new  TiXmlText(LaneObj->can_turn_on_red_signal() ? "true" : "false"));
////	can_change_lane_left
//	TiXmlElement * can_change_lane_left = new TiXmlElement("can_change_lane_left"); Lane->LinkEndChild(can_change_lane_left);
//	can_change_lane_left->LinkEndChild(new  TiXmlText(LaneObj->can_change_lane_left() ? "true" : "false"));
////	can_change_lane_right
//	TiXmlElement * can_change_lane_right = new TiXmlElement("can_change_lane_right"); Lane->LinkEndChild(can_change_lane_right);
//	can_change_lane_right->LinkEndChild(new  TiXmlText(LaneObj->can_change_lane_right() ? "true" : "false"));
////	is_road_shoulder
//	TiXmlElement * is_road_shoulder = new TiXmlElement("is_road_shoulder"); Lane->LinkEndChild(is_road_shoulder);
//	is_road_shoulder->LinkEndChild(new  TiXmlText(LaneObj->is_road_shoulder() ? "true" : "false"));
////	is_bicycle_lane
//	TiXmlElement * is_bicycle_lane = new TiXmlElement("is_bicycle_lane"); Lane->LinkEndChild(is_bicycle_lane);
//	is_bicycle_lane->LinkEndChild(new  TiXmlText(LaneObj->is_bicycle_lane() ? "true" : "false"));
////	is_pedestrian_lane
//	TiXmlElement * is_pedestrian_lane = new TiXmlElement("is_pedestrian_lane"); Lane->LinkEndChild(is_pedestrian_lane);
//	is_pedestrian_lane->LinkEndChild(new  TiXmlText(LaneObj->is_pedestrian_lane() ? "true" : "false"));
////	is_vehicle_lane
//	TiXmlElement * is_vehicle_lane = new TiXmlElement("is_vehicle_lane"); Lane->LinkEndChild(is_vehicle_lane);
//	is_vehicle_lane->LinkEndChild(new  TiXmlText(LaneObj->is_vehicle_lane() ? "true" : "false"));
////	is_standard_bus_lane
//	TiXmlElement * is_standard_bus_lane = new TiXmlElement("is_standard_bus_lane"); Lane->LinkEndChild(is_standard_bus_lane);
//	is_standard_bus_lane->LinkEndChild(new  TiXmlText(LaneObj->is_standard_bus_lane() ? "true" : "false"));
////	is_whole_day_bus_lane
//	TiXmlElement * is_whole_day_bus_lane = new TiXmlElement("is_whole_day_bus_lane"); Lane->LinkEndChild(is_whole_day_bus_lane);
//	is_whole_day_bus_lane->LinkEndChild(new  TiXmlText(LaneObj->is_whole_day_bus_lane() ? "true" : "false"));
////	is_high_occupancy_vehicle_lane
//	TiXmlElement * is_high_occupancy_vehicle_lane = new TiXmlElement("is_high_occupancy_vehicle_lane"); Lane->LinkEndChild(is_high_occupancy_vehicle_lane);
//	is_high_occupancy_vehicle_lane->LinkEndChild(new  TiXmlText(LaneObj->is_high_occupancy_vehicle_lane() ? "true" : "false"));
////	can_freely_park_here
//	TiXmlElement * can_freely_park_here = new TiXmlElement("can_freely_park_here"); Lane->LinkEndChild(can_freely_park_here);
//	can_freely_park_here->LinkEndChild(new  TiXmlText(LaneObj->can_freely_park_here() ? "true" : "false"));
////	can_stop_here
//	TiXmlElement * can_stop_here = new TiXmlElement("can_stop_here"); Lane->LinkEndChild(can_stop_here);
//	can_stop_here->LinkEndChild(new  TiXmlText(LaneObj->can_stop_here() ? "true" : "false"));
////	is_u_turn_allowed
//	TiXmlElement * is_u_turn_allowed = new TiXmlElement("is_u_turn_allowed"); Lane->LinkEndChild(is_u_turn_allowed);
//	is_u_turn_allowed->LinkEndChild(new  TiXmlText(LaneObj->is_u_turn_allowed() ? "true" : "false"));
//	//Polyline
//	TiXmlElement * PolyLine = new TiXmlElement("PolyLine"); Lane->LinkEndChild(PolyLine);
//	WriteXMLInput_PolyLine(LaneObj->getPolyline(),PolyLine);
//
//}
//void WriteXMLInput_Crossing(sim_mob::Crossing * crossing , int offset, TiXmlElement *Obstacle)
//{
//	std::ostringstream output;
//	TiXmlElement * Crossing = new TiXmlElement("Crossing"); Obstacle->LinkEndChild(Crossing);
//	//offset
//	TiXmlElement * Offset = new TiXmlElement("Offset"); Crossing->LinkEndChild(Offset);
//	output << offset;
//	Offset->LinkEndChild(new  TiXmlText(output.str()));
//	//start
//	TiXmlElement * start = new TiXmlElement("start"); Crossing->LinkEndChild(start);
//	WriteXMLInput_Location(start,false,crossing->getStart().getX(),crossing->getStart().getY());
//	//end
//	TiXmlElement * end = new TiXmlElement("end"); Crossing->LinkEndChild(end);
//	WriteXMLInput_Location(end,false,crossing->getStart().getX(),crossing->getStart().getY());
//	//id
//	TiXmlElement * crossingID = new TiXmlElement("crossingID"); Crossing->LinkEndChild(crossingID);
//	output << crossing->getCrossingID();
//	crossingID->LinkEndChild(new  TiXmlText(output.str()));
//	{//nearLine
//		TiXmlElement * nearLine = new TiXmlElement("nearLine"); Crossing->LinkEndChild(nearLine);
//		TiXmlElement * first = new TiXmlElement("first"); nearLine->LinkEndChild(first);
//		WriteXMLInput_Location(first,false,crossing->nearLine.first.getX(),crossing->nearLine.first.getY());
//		TiXmlElement * second = new TiXmlElement("second"); nearLine->LinkEndChild(second);
//		WriteXMLInput_Location(second,false,crossing->nearLine.second.getX(),crossing->nearLine.second.getY());
//	}
//	{//farLine
//		TiXmlElement * farLine = new TiXmlElement("farLine"); Crossing->LinkEndChild(farLine);
//		TiXmlElement * first = new TiXmlElement("first"); farLine->LinkEndChild(first);
//		WriteXMLInput_Location(first,false,crossing->farLine.first.getX(),crossing->farLine.first.getY());
//		TiXmlElement * second = new TiXmlElement("second"); farLine->LinkEndChild(second);
//		WriteXMLInput_Location(second,false,crossing->farLine.second.getX(),crossing->farLine.second.getY());
//	}
//}
//void WriteXMLInput_Obstacle(RoadItemAndOffsetPair res, TiXmlElement * Obstacle)
//{
//	if(dynamic_cast<sim_mob::Crossing *>(const_cast<sim_mob::RoadItem *>(res.item)))
//	{
//		WriteXMLInput_Crossing(dynamic_cast<sim_mob::Crossing *>(const_cast<sim_mob::RoadItem *>(res.item)), res.offset, Obstacle);
//	}else{}
//}
//
//void WriteXMLInput_Segment(sim_mob::RoadSegment* rs ,TiXmlElement * Segments)
//{
//	std::ostringstream Id;
//
//	//Segment
//	TiXmlElement * Segment = new TiXmlElement("Segment"); Segments->LinkEndChild(Segment);
//	//segmentID
//	TiXmlElement * segmentID = nullptr;
//	segmentID = new TiXmlElement("segmentID"); Segment->LinkEndChild(segmentID);
//	Id.str("");
//	Id << rs->getSegmentID();
//	segmentID->LinkEndChild(new  TiXmlText(Id.str()));
//	//start
//	TiXmlElement * startingNode = new TiXmlElement("startingNode"); Segment->LinkEndChild(startingNode);
//	Id.str("");
//	Id << rs->getStart()->getID();
//	startingNode->LinkEndChild(new  TiXmlText(Id.str()));
//	//end
//	TiXmlElement * endingNode = new TiXmlElement("endingNode"); Segment->LinkEndChild(endingNode);
//	Id.str("");
//	Id << rs->getEnd()->getID();
//	endingNode->LinkEndChild(new  TiXmlText(Id.str()));
//	//maxSpeed
//	TiXmlElement * maxSpeed = new TiXmlElement("maxSpeed"); Segment->LinkEndChild(maxSpeed);
//	Id.str("");
//	Id << rs->maxSpeed;
//	maxSpeed->LinkEndChild(new  TiXmlText(Id.str()));
//	//Length
//	TiXmlElement * Length = new TiXmlElement("Length"); Segment->LinkEndChild(Length);
//	Id.str("");
//	Id << rs->length;
//	Length->LinkEndChild(new  TiXmlText(Id.str()));
//	//Width
//	TiXmlElement * Width = new TiXmlElement("Width"); Segment->LinkEndChild(Width);
//	Id.str("");
//	Id << rs->width;
//	Width->LinkEndChild(new  TiXmlText(Id.str()));
//	//originalDB_ID
//	TiXmlElement * originalDB_ID = new TiXmlElement("originalDB_ID");  Segment->LinkEndChild(originalDB_ID);
//	originalDB_ID->LinkEndChild( new TiXmlText(rs->originalDB_ID.getLogItem()));
//	//polyline
//	TiXmlElement * polyline = new TiXmlElement("polyline"); Segment->LinkEndChild(polyline);
//	WriteXMLInput_PolyLine(const_cast<std::vector<sim_mob::Point2D>& >(rs->polyline), polyline);
//	//Lanes
//	TiXmlElement * Lanes = new TiXmlElement("Lanes"); Segment->LinkEndChild(Lanes);
//	//Lane
//	for(std::vector<sim_mob::Lane*>::const_iterator LaneObj_it = rs->getLanes().begin(), it_end(rs->getLanes().end()); LaneObj_it != it_end ; LaneObj_it++)
//		WriteXMLInput_Lane(*LaneObj_it,Lanes);
//	TiXmlElement * Obstacles = new TiXmlElement("Obstacles"); Segment->LinkEndChild(Obstacles);
//	for (int currOffset = 0;currOffset <= rs->length ;) {
//		//Get the next item, if any.
//		RoadItemAndOffsetPair res = rs->nextObstacle(currOffset, true);
//		if (!res.item) {
//			break;
//		}
//		WriteXMLInput_Obstacle(res, Obstacles);
//		currOffset += res.offset + 1;
//	}
//
//}
//
//void WriteXMLInput_Segments(sim_mob::Link* LinkObj ,TiXmlElement * Link)
//{
//	//Segments element first
//	TiXmlElement * Segments = new TiXmlElement("Segments");
//		//FWDSegments
//    TiXmlElement * FWDSegments = new TiXmlElement("FWDSegments");
//    std::vector<sim_mob::RoadSegment*>::const_iterator SegObj_it = (LinkObj)->getFwdSegments().begin();
//    //validation
//    if(SegObj_it != (LinkObj)->getFwdSegments().end())
//    	Segments->LinkEndChild(FWDSegments);
//    for(; (SegObj_it != (LinkObj)->getFwdSegments().end()) ; SegObj_it++)
//    {
//    	WriteXMLInput_Segment(*SegObj_it,FWDSegments);
//    }
//
//    //BKDSegments
//    TiXmlElement * BKDSegments = new TiXmlElement("BKDSegments");
//    SegObj_it = (LinkObj)->getRevSegments().begin();
//    //validation
//    if((LinkObj)->getRevSegments().begin() != (LinkObj)->getRevSegments().end())
//    	 Segments->LinkEndChild(BKDSegments);
//    for(; (SegObj_it != (LinkObj)->getRevSegments().end()) ; SegObj_it++)
//    {
//      	WriteXMLInput_Segment(*SegObj_it,BKDSegments);
//    }
//    //validation:if you don't have any FWDSegments and BKDSegments, don't add parent element 'Segments'
//    if(!(((LinkObj)->getRevSegments().begin() == (LinkObj)->getRevSegments().end())&&((LinkObj)->getFwdSegments().begin() == (LinkObj)->getFwdSegments().end())))
//    	Link->LinkEndChild(Segments);
//}
//
//void WriteXMLInput_Links(const std::vector<sim_mob::Link*>& link,TiXmlElement * RoadNetwork)
//{
//	std::ostringstream out;
//    TiXmlElement * Links = new TiXmlElement("Links");
//    std::vector<sim_mob::Link*>::const_iterator LinksObj_it = link.begin();
//    if(LinksObj_it != link.end())
//    	RoadNetwork->LinkEndChild(Links);
//    for( ;LinksObj_it != link.end() ; LinksObj_it++)
//    {
//    	//Link emlement
//    	TiXmlElement * Link = new TiXmlElement("Link"); Links->LinkEndChild(Link);
//    	//LinkID
//    	TiXmlElement * linkID = new TiXmlElement("linkID");
//    	out.str("");
//    	out << (*LinksObj_it)->getLinkId();
//    	if(out.str().size())
//    	{
//        	linkID->LinkEndChild( new TiXmlText((out.str())));
//    		Link->LinkEndChild(linkID);
//    	}
//    	//RoadName
//    	TiXmlElement * roadName = new TiXmlElement("roadName");  Link->LinkEndChild(roadName);
//    	roadName->LinkEndChild( new TiXmlText(((*LinksObj_it)->getRoadName())));
//    	//StartingNode
//    	TiXmlElement * StartingNode = new TiXmlElement("StartingNode");
//    	out.str("");
//    	out << (*LinksObj_it)->getStart()->getID();
//    	if(out.str().size())
//    	{
//    		StartingNode->LinkEndChild( new TiXmlText((out.str())));
//    		Link->LinkEndChild(StartingNode);
//    	}
//    	//EndingNode
//    	TiXmlElement * EndingNode = new TiXmlElement("EndingNode");
//    	out.str("");
//    	out << (*LinksObj_it)->getEnd()->getID();
//    	if(out.str().size())
//    	{
//    		EndingNode->LinkEndChild( new TiXmlText((out.str())));
//    		Link->LinkEndChild(EndingNode);
//    	}
//    	WriteXMLInput_Segments(*LinksObj_it,Link);
//    }
//}
//
//void WriteXMLInput_UniNode_Connectors(sim_mob::UniNode* uninode,TiXmlElement * UniNode_)
//{
//	std::ostringstream out;
//	TiXmlElement * Connectors = new TiXmlElement("Connectors"); UniNode_->LinkEndChild(Connectors);
//	TiXmlElement * Connector;
//	TiXmlElement * laneFrom;
//	TiXmlElement * laneTo;
//
//	for(std::map<const sim_mob::Lane*, sim_mob::Lane* >::const_iterator it = uninode->getConnectors().begin(),it_end(uninode->getConnectors().end());it != it_end; it++)
//	{
//		Connector = new TiXmlElement("Connector"); Connectors->LinkEndChild(Connector);
//		laneFrom = new TiXmlElement("laneFrom"); Connector->LinkEndChild(laneFrom);
//		laneTo = new TiXmlElement("laneTo"); Connector->LinkEndChild(laneTo);
//		out.str(""); out << (*it).first->getLaneID_str();
//		laneFrom->LinkEndChild( new TiXmlText(out.str()));
//		out.str(""); out << (*it).second->getLaneID_str();
//		laneTo->LinkEndChild( new TiXmlText(out.str()));
//	}
//}
//void WriteXMLInput_Node(sim_mob::Node *node, TiXmlElement * parent)
//{
//	std::ostringstream out;
//	//nodeId
//	TiXmlElement * nodeID = new TiXmlElement("nodeID"); parent->LinkEndChild(nodeID);
//	out.str("");
//	out << node->getID();
//	nodeID->LinkEndChild( new TiXmlText(out.str()));
//	//location
//	WriteXMLInput_Location(parent,true,node->getLocation().getX(),node->getLocation().getY());
//	//linkLoc
//	TiXmlElement * linkLoc = new TiXmlElement("linkLoc"); parent->LinkEndChild(linkLoc);
//	out.str("");
//	out << node->getLinkLoc()->linkID;
//	linkLoc->LinkEndChild( new TiXmlText(out.str()));
//	//originalDB_ID
//	TiXmlElement * originalDB_ID = new TiXmlElement("originalDB_ID");  parent->LinkEndChild(originalDB_ID);
//	originalDB_ID->LinkEndChild( new TiXmlText(node->originalDB_ID.getLogItem()));
//}
//void WriteXMLInput_UniNodes(sim_mob::RoadNetwork & roadNetwork,TiXmlElement * Nodes)
//{
//	TiXmlElement * UniNodes = new TiXmlElement("UniNodes"); Nodes->LinkEndChild(UniNodes);
//	for(std::set<sim_mob::UniNode*>::const_iterator it = roadNetwork.getUniNodes().begin(), it_end( roadNetwork.getUniNodes().end()); it != it_end ; it++ )
//	{
//		TiXmlElement * UniNode = new TiXmlElement("UniNode"); UniNodes->LinkEndChild(UniNode);
//		WriteXMLInput_Node(*it, UniNode);//basic node information
//		//connectors
//		WriteXMLInput_UniNode_Connectors(*it,UniNode);
//	}
//}
//
//void WriteXMLInput_roadSegmentsAt(sim_mob::MultiNode * mn,TiXmlElement * Intersection)
//{
//	//roadSegmentsAt
//	TiXmlElement * roadSegmentsAt = new TiXmlElement("roadSegmentsAt"); Intersection->LinkEndChild(roadSegmentsAt);
//	for(std::set<sim_mob::RoadSegment*>::const_iterator rsObj_it = (mn)->getRoadSegments().begin(),it_end((mn)->getRoadSegments().end()) ; rsObj_it != it_end; rsObj_it++)
//	{
//		TiXmlElement * segmentID = new TiXmlElement("segmentID"); roadSegmentsAt->LinkEndChild(segmentID);
//		std::ostringstream Id;
//		Id.str("");
//		Id << (*rsObj_it)->getSegmentID();
//		segmentID->LinkEndChild( new TiXmlText(Id.str()));
//	}
//}
//
//void WriteXMLInput_MultiNode_Connectors(sim_mob::MultiNode* mn,TiXmlElement * MultiNode)
//{
//	std::ostringstream out;
//	TiXmlElement * Connectors = new TiXmlElement("Connectors"); MultiNode->LinkEndChild(Connectors);
//
////	std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> >::iterator conn_it = mn->getConnectors().begin();
//	for(std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> >::const_iterator conn_it = mn->getConnectors().begin(), it_end(mn->getConnectors().end()); conn_it != it_end; conn_it++)
//	{
//		TiXmlElement * MultiConnectors = new TiXmlElement("MultiConnectors"); Connectors->LinkEndChild(MultiConnectors);
//		TiXmlElement * RoadSegment = new TiXmlElement("RoadSegment"); MultiConnectors->LinkEndChild(RoadSegment);
//
//		std::ostringstream Id;
//		Id.str("");
//		Id << (*conn_it).first->getSegmentID();
//		RoadSegment->LinkEndChild( new TiXmlText(Id.str()));
//
//		TiXmlElement * Connectors = new TiXmlElement("Connectors"); MultiConnectors->LinkEndChild(Connectors);
//		for(std::set<sim_mob::LaneConnector*> ::const_iterator l_conn_it = conn_it->second.begin(), it_end(conn_it->second.end()); l_conn_it != it_end ; l_conn_it++)
//		{
//			TiXmlElement * Connector = new TiXmlElement("Connector"); Connectors->LinkEndChild(Connector);
//			TiXmlElement * laneFrom = new TiXmlElement("laneFrom"); Connector->LinkEndChild(laneFrom);
//			TiXmlElement * laneTo = new TiXmlElement("laneTo"); Connector->LinkEndChild(laneTo);
//			out.str(""); out << (*l_conn_it)->getLaneFrom()->getLaneID_str();
//			laneFrom->LinkEndChild( new TiXmlText(out.str()));
//
//			out.str(""); out << (*l_conn_it)->getLaneTo()->getLaneID_str();
//			laneTo->LinkEndChild( new TiXmlText(out.str()));
//		}
//	}
//}
//
//TiXmlElement * WriteXMLInput_Intersection(sim_mob::Intersection *intersection,TiXmlElement * Intersections, TiXmlElement * Nodes)
//{
//	std::ostringstream out;
//	if(!Intersections) { Intersections = new TiXmlElement("Intersections"); Nodes->LinkEndChild(Intersections);}
//		out.str("");
//		TiXmlElement * Intersection = new TiXmlElement("Intersection"); Intersections->LinkEndChild(Intersection);
////    	//nodeID
////    	TiXmlElement * nodeID = new TiXmlElement("nodeID");  Intersection->LinkEndChild(nodeID);
////    	out << (intersection)->getID();
////    	nodeID->LinkEndChild( new TiXmlText((out.str())));
////    	//originalDB_ID
////    	TiXmlElement * originalDB_ID = new TiXmlElement("originalDB_ID");  Intersection->LinkEndChild(originalDB_ID);
////    	originalDB_ID->LinkEndChild( new TiXmlText(intersection->originalDB_ID.getLogItem()));
////    	//location
////    	WriteXMLInput_Location(Intersection,true,(intersection)->location.getX(),(intersection)->location.getY());
//    	WriteXMLInput_Node(intersection, Intersection);//basic node information
//    	WriteXMLInput_roadSegmentsAt(intersection,Intersection);
//    	WriteXMLInput_MultiNode_Connectors(intersection,Intersection);
//    	return Intersections;//used to decide whether create a parent "Intersections" element or not
//}
//TiXmlElement * WriteXMLInput_Roundabout(sim_mob::Roundabout *roundabout,TiXmlElement * roundabouts,TiXmlElement * Nodes)
//{
//	if(!roundabouts) { roundabouts = new TiXmlElement("roundabouts"); Nodes->LinkEndChild(roundabouts);}
//	return roundabouts;
//}
//
//void WriteXMLInput_Nodes(sim_mob::RoadNetwork roadNetwork,TiXmlElement * RoadNetwork)
//{
//    TiXmlElement * Nodes = new TiXmlElement("Nodes"); RoadNetwork->LinkEndChild(Nodes);
//	WriteXMLInput_UniNodes(roadNetwork,Nodes);
//	TiXmlElement * intersections = 0, *roundabouts = 0;
//	std::vector<sim_mob::MultiNode*>::const_iterator mNode_it = roadNetwork.getNodes().begin();
//	for(; mNode_it != roadNetwork.getNodes().end() ; mNode_it++)
//	{
//		if(dynamic_cast<sim_mob::Intersection*>(*mNode_it))
//			intersections = WriteXMLInput_Intersection(dynamic_cast<sim_mob::Intersection*>(*mNode_it),intersections, Nodes);
//		if(dynamic_cast<sim_mob::Roundabout*>(*mNode_it))
//			roundabouts = WriteXMLInput_Roundabout(dynamic_cast<sim_mob::Roundabout*>(*mNode_it),roundabouts, Nodes);
//	}
//}
//void WriteXMLInput_RoadNetwork(TiXmlElement * GeoSpatial)
//{
//	TiXmlElement * RoadNetwork = new TiXmlElement("RoadNetwork");  GeoSpatial->LinkEndChild(RoadNetwork);
//	ConfigParams& config = ConfigParams::GetInstance();
//
//
//    sim_mob::RoadNetwork RoadNetworkObj = config.getNetwork();
//    const std::vector<sim_mob::Link*>& LinksObj = RoadNetworkObj.getLinks();
//    WriteXMLInput_Nodes(RoadNetworkObj,RoadNetwork);
//    WriteXMLInput_Links(LinksObj,RoadNetwork);
//}
//
////unlikely to be used
//void WriteXMLInput_RoadItems(TiXmlElement * GeoSpatial)
//{
//	TiXmlElement * RoadItems = new TiXmlElement("RoadItems"); 	   GeoSpatial->LinkEndChild(RoadItems);
//    TiXmlElement * BusStops = new TiXmlElement("BusStops"); RoadItems->LinkEndChild(BusStops);
//    TiXmlElement * ERP_Gantries = new TiXmlElement("ERP_Gantries"); RoadItems->LinkEndChild(ERP_Gantries);
//    TiXmlElement * Crossings = new TiXmlElement("Crossings"); RoadItems->LinkEndChild(Crossings);
//    TiXmlElement * RoadBumps = new TiXmlElement("RoadBumps"); RoadItems->LinkEndChild(RoadBumps);
//}
//void WriteXMLInput_GeoSpatial(TiXmlElement * SimMobility)
//{
//	TiXmlElement * GeoSpatial;
//    GeoSpatial = new TiXmlElement( "GeoSpatial" );
//    SimMobility->LinkEndChild( GeoSpatial );
//    WriteXMLInput_RoadNetwork(GeoSpatial);
//}
//
//void WriteXMLInput_TripChainItem(TiXmlElement * TripChain, sim_mob::TripChainItem & tripChainItem)
//{
//	std::ostringstream out;
////	//entityID
////	out.str("");
////	out << tripChainItem.entityID;
////	TiXmlElement * entityID  = new TiXmlElement( "entityID" );
////	entityID->LinkEndChild( new TiXmlText(out.str()));
////	TripChain->LinkEndChild( entityID );
//	//personID
//	out.str("");
//	out << tripChainItem.personID;//TODO: sunc with tripchainitem(last decision was to replace entityID with personID
//	TiXmlElement * personID  = new TiXmlElement( "personID" );
//	personID->LinkEndChild( new TiXmlText(out.str()));
//	TripChain->LinkEndChild( personID );
//
//	//itemType
//	out.str("");
//	switch(tripChainItem.itemType)
//	{
//	case sim_mob::TripChainItem::IT_ACTIVITY:
//		out << "IT_ACTIVITY";
//		break;
//
//	case sim_mob::TripChainItem::IT_TRIP:
//		out << "IT_TRIP";
//		break;
//	}
//	TiXmlElement * itemType  = new TiXmlElement( "itemType" );
//	itemType->LinkEndChild( new TiXmlText(out.str()));
//	TripChain->LinkEndChild( itemType );
//
//	//sequenceNumber
//	out.str("");
//	out << tripChainItem.sequenceNumber;
//	TiXmlElement * sequenceNumber  = new TiXmlElement( "sequenceNumber" );
//	sequenceNumber->LinkEndChild( new TiXmlText(out.str()));
//	TripChain->LinkEndChild( sequenceNumber );
//
//	//startTime
////	out.str("");
////	out << tripChainItem.startTime.toString();
//	TiXmlElement * startTime  = new TiXmlElement( "startTime" );
//	startTime->LinkEndChild( new TiXmlText(tripChainItem.startTime.toString()));
//	TripChain->LinkEndChild( startTime );
//	//endTime
//	TiXmlElement * endTime  = new TiXmlElement( "endTime" );
//	endTime->LinkEndChild( new TiXmlText(tripChainItem.endTime.toString()));
//	TripChain->LinkEndChild( endTime );
//}
////duto some special/weird design in tripchain hierarchy, we need to copy tripchainitem and trip information the way you see
//void WriteXMLInput_TripChain_Subtrips(TiXmlElement * Trip,  sim_mob::Trip & trip)
//{
//	TiXmlElement * Subtrips  = new TiXmlElement( "subTrips" ); Trip->LinkEndChild( Subtrips );
//	std::ostringstream out;
//	for(std::vector<SubTrip>::const_iterator it = trip.getSubTrips().begin(), it_end(trip.getSubTrips().end()); it != it_end; it++)
//	{
//		TiXmlElement * Subtrip  = new TiXmlElement( "subTrip" ); Subtrips->LinkEndChild( Subtrip );
//		///////////tripchainitem part//////////////////
//		//personID
//		out.str("");
//		out << it->personID;
//		TiXmlElement * personID  = new TiXmlElement( "personID" );
//		personID->LinkEndChild( new TiXmlText(out.str()));
//		Subtrip->LinkEndChild( personID );
//		//itemType
//		out.str("");
//
//		switch(it->itemType)
//		{
//		case sim_mob::TripChainItem::IT_ACTIVITY:
//			out << "IT_ACTIVITY";
//			break;
//
//		case sim_mob::TripChainItem::IT_TRIP:
//			out << "IT_TRIP";
//			break;
//		}
//		TiXmlElement * itemType  = new TiXmlElement( "itemType" );
//		itemType->LinkEndChild( new TiXmlText(out.str()));
//		Subtrip->LinkEndChild( itemType );
//		//sequenceNumber
//		out.str("");
//		out << it->sequenceNumber;
//		TiXmlElement * sequenceNumber  = new TiXmlElement( "sequenceNumber" );
//		sequenceNumber->LinkEndChild( new TiXmlText(out.str()));
//		Subtrip->LinkEndChild( sequenceNumber );
//		//startTime
//		out.str("");
//		out << it->startTime.toString();
//		TiXmlElement * startTime  = new TiXmlElement( "startTime" );
//		startTime->LinkEndChild( new TiXmlText(out.str()));
//		Subtrip->LinkEndChild( startTime );
//		//endTime
//		out.str("");
//		out << it->endTime.toString();
//		TiXmlElement * endTime  = new TiXmlElement( "endTime" );
//		endTime->LinkEndChild( new TiXmlText(out.str()));
//		Subtrip->LinkEndChild( endTime );
//		///////////trip part//////////////////
//		//tripID
//		out.str("");
//		out << it->tripID;
//		TiXmlElement * tripID  = new TiXmlElement( "tripID" );
//		tripID->LinkEndChild( new TiXmlText(out.str()));
//		Subtrip->LinkEndChild( tripID );
//		//fromLocation
//		out.str("");
//		out << it->fromLocation->getID();
//		TiXmlElement * fromLocation  = new TiXmlElement( "fromLocation" );
//		fromLocation->LinkEndChild( new TiXmlText(out.str()));
//		Subtrip->LinkEndChild( fromLocation );
//		//fromLocationType
//		out.str("");
//		switch(it->fromLocationType)
//		{
//		case TripChainItem::LT_BUILDING:
//			out << "LT_BUILDING";
//			break;
//		case TripChainItem::LT_NODE:
//			out << "LT_NODE";
//			break;
//		case TripChainItem::LT_LINK:
//			out << "LT_LINK";
//			break;
//		case TripChainItem::LT_PUBLIC_TRANSIT_STOP:
//			out << "LT_PUBLIC_TRANSIT_STOP";
//			break;
//		}
//		TiXmlElement * fromLocationType  = new TiXmlElement( "fromLocationType" );
//		fromLocationType->LinkEndChild( new TiXmlText(out.str()));
//		Subtrip->LinkEndChild( fromLocationType );
//		//toLocation
//		out.str("");
//		out << it->toLocation->getID();
//		TiXmlElement * toLocation  = new TiXmlElement( "toLocation" );
//		toLocation->LinkEndChild( new TiXmlText(out.str()));
//		Subtrip->LinkEndChild( toLocation );
//		//toLocationType
//		out.str("");
//		switch(it->toLocationType)
//		{
//		case TripChainItem::LT_BUILDING:
//			out << "LT_BUILDING";
//			break;
//		case TripChainItem::LT_NODE:
//			out << "LT_NODE";
//			break;
//		case TripChainItem::LT_LINK:
//			out << "LT_LINK";
//			break;
//		case TripChainItem::LT_PUBLIC_TRANSIT_STOP:
//			out << "LT_PUBLIC_TRANSIT_STOP";
//			break;
//		}
//		TiXmlElement * toLocationType  = new TiXmlElement( "toLocationType" );
//		toLocationType->LinkEndChild( new TiXmlText(out.str()));
//		Subtrip->LinkEndChild( toLocationType );
//		///////////actual subtrip part//////////////////
//		//mode
//		TiXmlElement * mode  = new TiXmlElement( "mode" );
//		mode->LinkEndChild( new TiXmlText(it->mode));
//		Subtrip->LinkEndChild( mode );
//		//isPrimaryMode
//		TiXmlElement * isPrimaryMode  = new TiXmlElement( "isPrimaryMode" );
//		isPrimaryMode->LinkEndChild( new TiXmlText(it->isPrimaryMode ? "true" : "false"));
//		Subtrip->LinkEndChild( isPrimaryMode );
//		//ptLineId
//		TiXmlElement * ptLineId  = new TiXmlElement( "ptLineId" );
//		ptLineId->LinkEndChild( new TiXmlText(it->ptLineId));
//		Subtrip->LinkEndChild( ptLineId );
//	}
//}
//
//std::string locationType_toString(TripChainItem::LocationType type)
//{
//	switch(type)
//	{
//	case TripChainItem::LT_BUILDING:
//		return "LT_BUILDING";
//	case TripChainItem::LT_NODE:
//		return "LT_NODE";
//	case TripChainItem::LT_LINK:
//		return "LT_PUBLIC_TRANSIT_STOP";
//	default:
//		return "";
//	}
//}
//void WriteXMLInput_TripChain_Trip(TiXmlElement * TripChains, sim_mob::Trip & trip)
//{
//	std::ostringstream out;
//	TiXmlElement * Trip;
//	Trip = new TiXmlElement( "Trip" );
//	TripChains->LinkEndChild( Trip );
//	WriteXMLInput_TripChainItem(Trip,trip);
//	//tripID
//	out.str("");
//	out << trip.tripID;
//	TiXmlElement * tripID  = new TiXmlElement( "tripID" );
//	tripID->LinkEndChild( new TiXmlText(out.str()));
//	Trip->LinkEndChild( tripID );
//	//fromLocation
//	TiXmlElement * fromLocation  = new TiXmlElement( "fromLocation" );
//	out.str("");
//	out << trip.fromLocation->getID();
//	fromLocation->LinkEndChild( new TiXmlText(out.str()));
////	WriteXMLInput_Location(fromLocation,false,trip.fromLocation->getLocation().getX(),trip.fromLocation->getLocation().getY());
//	Trip->LinkEndChild( fromLocation );
//	//fromLocationType
////	out.str("");
////	out << locationType_toString(trip.fromLocationType);
//	TiXmlElement * fromLocationType  = new TiXmlElement( "fromLocationType" );
//	fromLocationType->LinkEndChild( new TiXmlText(locationType_toString(trip.fromLocationType)));
//	Trip->LinkEndChild( fromLocationType );
//	//toLocation
//	TiXmlElement * toLocation  = new TiXmlElement( "toLocation" );
//	out.str("");
//	out << trip.toLocation->getID();
//	toLocation->LinkEndChild( new TiXmlText(out.str()));
////	WriteXMLInput_Location(toLocation,false,trip.toLocation->getLocation().getX(),trip.toLocation->getLocation().getY());
//	Trip->LinkEndChild( toLocation );
//	//toLocationType
////	out.str("");
////	out << trip.toLocationType;
//	TiXmlElement * toLocationType  = new TiXmlElement( "toLocationType" );
//	toLocationType->LinkEndChild( new TiXmlText(locationType_toString(trip.toLocationType)));
//	Trip->LinkEndChild( toLocationType );
//	if(trip.getSubTrips().size() > 0)
//		WriteXMLInput_TripChain_Subtrips(Trip,trip);
//
//}
//
//
//	void WriteXMLInput_TripChain_Activity(TiXmlElement * TripChains, sim_mob::Activity & activity)
//{
//	std::ostringstream out;
//	TiXmlElement * Activity;
//	Activity = new TiXmlElement( "Activity" );
//	TripChains->LinkEndChild( Activity );
//	WriteXMLInput_TripChainItem(Activity,activity);
//	//description
//	TiXmlElement * description  = new TiXmlElement( "description" );
//	description->LinkEndChild( new TiXmlText(activity.description));
//	Activity->LinkEndChild( description );
//	//location
//	TiXmlElement * location  = new TiXmlElement( "location" );
//	out.str("");
//	out << activity.location->getID();
//	location->LinkEndChild( new TiXmlText(out.str()));
//	Activity->LinkEndChild( location );
//	//locationType
////	out.str("");
////	out << activity.locationType;
//	TiXmlElement * locationType  = new TiXmlElement( "locationType" );
//	locationType->LinkEndChild( new TiXmlText(locationType_toString(activity.locationType)));
//	Activity->LinkEndChild( locationType );
//	//isPrimary
//	TiXmlElement * isPrimary  = new TiXmlElement( "isPrimary" );
//	isPrimary->LinkEndChild( new TiXmlText(activity.isPrimary ? "true" : "false"));
//	Activity->LinkEndChild( isPrimary );
//	//isFlexible
//	TiXmlElement * isFlexible  = new TiXmlElement( "isFlexible" );
//	isFlexible->LinkEndChild( new TiXmlText(activity.isFlexible ? "true" : "false"));
//	Activity->LinkEndChild( isFlexible );
//	//isMandatory
//	TiXmlElement * isMandatory  = new TiXmlElement( "isMandatory" );
//	isMandatory->LinkEndChild( new TiXmlText(activity.isMandatory ? "true" : "false"));
//	Activity->LinkEndChild( isMandatory );
//}
//void WriteXMLInput_TripChains(TiXmlElement * SimMobility)
//{
//	std::ostringstream out;
//	ConfigParams& config = ConfigParams::GetInstance();
//
//	std::map<unsigned int, std::vector<sim_mob::TripChainItem*> >& TripChainsObj = config.getTripChains();
//	if(TripChainsObj.size() < 1) return;
//
//	TiXmlElement * TripChains;
//	TripChains = new TiXmlElement( "TripChains" );
//	SimMobility->LinkEndChild( TripChains );
//
//	for(std::map<unsigned int, std::vector<sim_mob::TripChainItem*> >::iterator it_map = TripChainsObj.begin(), it_end(TripChainsObj.end()); it_map != it_end ; it_map ++)
//	{
//		//tripchain
//		TiXmlElement * TripChain;
//		TripChain = new TiXmlElement( "TripChain" );
//		TripChains->LinkEndChild( TripChain );
//
//		//personID sub-element
//		TiXmlElement * personID;
//		personID = new TiXmlElement( "personID" );
//		out.str("");
//		out << it_map->first; //personID
//		personID->LinkEndChild(new TiXmlText(out.str()));
//		TripChain->LinkEndChild( personID );
//
//		for(std::vector<sim_mob::TripChainItem*>::iterator it = it_map->second.begin() , it_end(it_map->second.end()); it != it_end; it++)
//		{
//			sim_mob::TripChainItem* tripChainItem = *it;
//			if(dynamic_cast<sim_mob::Activity *>(tripChainItem))
//				WriteXMLInput_TripChain_Activity(TripChain,*dynamic_cast<sim_mob::Activity *>(tripChainItem));
//			else
//				if(dynamic_cast<sim_mob::Trip *>(tripChainItem))
//					WriteXMLInput_TripChain_Trip(TripChain,*dynamic_cast<sim_mob::Trip *>(tripChainItem));
//		}
//	}
//
//}
//void WriteXMLInput(const std::string& XML_OutPutFileName)
//{
//
//	cout <<"In WriteXMLInput" << endl;
//	TiXmlDocument doc;
//	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "utf-8", "");
//	doc.LinkEndChild( decl );
//	TiXmlElement * SimMobility;
//	SimMobility = new TiXmlElement( "geo:SimMobility" );
//	SimMobility->SetAttribute("xmlns:geo" , "http://www.smart.mit.edu/geo");
//	SimMobility->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
//	SimMobility->SetAttribute("xsi:schemaLocation", "http://www.smart.mit.edu/geo file:/home/vahid/Desktop/geo8/geo10.xsd");
//    doc.LinkEndChild( SimMobility );
//
//	WriteXMLInput_GeoSpatial(SimMobility);
//	WriteXMLInput_TripChains(SimMobility);
//    doc.SaveFile( XML_OutPutFileName );
//}

/**
 * Main simulation loop.
 * \note
 * For doxygen, we are setting the variable JAVADOC AUTOBRIEF to "true"
 * This isn't necessary for class-level documentation, but if we want
 * documentation for a short method (like "get" or "set") then it makes sense to
 * have a few lines containing brief/full comments. (See the manual's description
 * of JAVADOC AUTOBRIEF). Of course, we can discuss this first.
 *
 * \par
 * See Buffered.hpp for an example of this in action.
 *
 * \par
 * ~Seth
 *
 * This function is separate from main() to allow for easy scoping of WorkGroup objects.
 */
bool performMain(const std::string& configFileName,const std::string& XML_OutPutFileName) {
	cout <<"Starting SimMobility, version1 " <<SIMMOB_VERSION <<endl;
	
	ProfileBuilder* prof = nullptr;
#ifdef SIMMOB_AGENT_UPDATE_PROFILE
	ProfileBuilder::InitLogFile("agent_update_trace.txt");
	ProfileBuilder prof_i;
	prof = &prof_i;
#endif

	//Register our Role types.
	//TODO: Accessing ConfigParams before loading it is technically safe, but we
	//      should really be clear about when this is not ok.
	RoleFactory& rf = ConfigParams::GetInstance().getRoleFactoryRW();
	rf.registerRole("driver", new sim_mob::Driver(nullptr, ConfigParams::GetInstance().mutexStategy));
	rf.registerRole("pedestrian", new sim_mob::Pedestrian2(nullptr));
	rf.registerRole("busdriver", new sim_mob::BusDriver(nullptr, ConfigParams::GetInstance().mutexStategy));
	rf.registerRole("activityRole", new sim_mob::ActivityPerformer(nullptr));
	//rf.registerRole("buscontroller", new sim_mob::BusController()); //Not a role!

	//Loader params for our Agents
	WorkGroup::EntityLoadParams entLoader(Agent::pending_agents, Agent::all_agents);

	//Load our user config file
	if (!ConfigParams::InitUserConf(configFileName, Agent::all_agents, Agent::pending_agents, prof)) {
		return false;
	}

	//Save a handle to the shared definition of the configuration.
	const ConfigParams& config = ConfigParams::GetInstance();
//#ifdef SIMMOB_XML_WRITER
//	/*
//	 *******************************
//	 * XML Writer
//	 *******************************
//	 */
//	WriteXMLInput(XML_OutPutFileName);
//	cout << "XML input for SimMobility Created....\n";
//	return true;
//#endif

	//Start boundaries
	if (!config.MPI_Disabled() && config.is_run_on_many_computers) {
		PartitionManager::instance().initBoundaryTrafficItems();
	}

	bool NoDynamicDispatch = config.DynamicDispatchDisabled();

	PartitionManager* partMgr = nullptr;
	if (!config.MPI_Disabled() && config.is_run_on_many_computers) {
		partMgr = &PartitionManager::instance();
	}

	{ //Begin scope: WorkGroups
	//TODO: WorkGroup scope currently does nothing. We need to re-enable WorkGroup deletion at some later point. ~Seth

	//Work Group specifications
	WorkGroup* agentWorkers = WorkGroup::NewWorkGroup(config.agentWorkGroupSize, config.totalRuntimeTicks, config.granAgentsTicks, &AuraManager::instance(), partMgr);
	WorkGroup* signalStatusWorkers = WorkGroup::NewWorkGroup(config.signalWorkGroupSize, config.totalRuntimeTicks, config.granSignalsTicks);

	//Initialize all work groups (this creates barriers, and locks down creation of new groups).
	WorkGroup::InitAllGroups();

	//Initialize each work group individually
	agentWorkers->initWorkers(NoDynamicDispatch ? nullptr :  &entLoader);
	signalStatusWorkers->initWorkers(nullptr);


	//Anything in all_agents is starting on time 0, and should be added now.
	for (vector<Entity*>::iterator it = Agent::all_agents.begin(); it != Agent::all_agents.end(); it++) {
		agentWorkers->assignAWorker(*it);
	}

	//Assign all signals too
	for (vector<Signal*>::iterator it = Signal::all_signals_.begin(); it != Signal::all_signals_.end(); it++) {
		signalStatusWorkers->assignAWorker(*it);
	}

	cout << "Initial Agents dispatched or pushed to pending." << endl;


	//Initialize the aura manager
	AuraManager::instance().init();

	///
	///  TODO: Do not delete this next line. Please read the comment in TrafficWatch.hpp
	///        ~Seth
	///
//	TrafficWatch& trafficWatch = TrafficWatch::instance();

	//Start work groups and all threads.
	WorkGroup::StartAllWorkGroups();

	//
	if (!config.MPI_Disabled() && config.is_run_on_many_computers) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.setEntityWorkGroup(agentWorkers, signalStatusWorkers);

		std::cout << "partition_solution_id in main function:" << partitionImpl.partition_config->partition_solution_id << std::endl;
	}

	/////////////////////////////////////////////////////////////////
	// NOTE: WorkGroups are able to handle skipping steps by themselves.
	//       So, we simply call "wait()" on every tick, and on non-divisible
	//       time ticks, the WorkGroups will return without performing
	//       a barrier sync.
	/////////////////////////////////////////////////////////////////
	size_t numStartAgents = Agent::all_agents.size();
	size_t numPendingAgents = Agent::pending_agents.size();
	size_t maxAgents = Agent::all_agents.size();

	timeval loop_start_time;
	gettimeofday(&loop_start_time, nullptr);
	int loop_start_offset = diff_ms(loop_start_time, start_time);

	ParitionDebugOutput debug;

	int lastTickPercent = 0; //So we have some idea how much time is left.
	for (unsigned int currTick = 0; currTick < config.totalRuntimeTicks; currTick++) {
		//Flag
		bool warmupDone = (currTick >= config.totalWarmupTicks);

		//Get a rough idea how far along we are
		int currTickPercent = (currTick*100)/config.totalRuntimeTicks;

		//Save the maximum number of agents at any given time
		maxAgents = std::max(maxAgents, Agent::all_agents.size());

		//Output
#ifndef SIMMOB_DISABLE_OUTPUT
		{
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			cout << "Approximate Tick Boundary: " << currTick << ", ";
			cout << (currTick * config.baseGranMS) << " ms   [" <<currTickPercent <<"%]" << endl;
			if (!warmupDone) {
				cout << "  Warmup; output ignored." << endl;
			}
		}
#else
		//We don't need to lock this output if general output is disabled, since Agents won't
		//  perform any output (and hence there will be no contention)
		if (currTickPercent-lastTickPercent>9) {
			lastTickPercent = currTickPercent;
			cout <<currTickPercent <<"%" <<endl;
		}
#endif

		///
		///  TODO: Do not delete this next line. Please read the comment in TrafficWatch.hpp
		///        ~Seth
		///
//		trafficWatch.update(currTick);

		//Agent-based cycle, steps 1,2,3,4 of 4
		WorkGroup::WaitAllGroups();

		//Check if the warmup period has ended.
		if (warmupDone) {
		}
	}

	//Finalize partition manager
	if (!config.MPI_Disabled() && config.is_run_on_many_computers) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.stopMPIEnvironment();
	}

	std::cout <<"Database lookup took: " <<loop_start_offset <<" ms" <<std::endl;

	cout << "Max Agents at any given time: " <<maxAgents <<std::endl;
	cout << "Starting Agents: " << numStartAgents;
	cout << ",     Pending: ";
	if (NoDynamicDispatch) {
		cout <<"<Disabled>";
	} else {
		cout <<numPendingAgents;
	}
	cout << endl;

	if (Agent::all_agents.empty()) {
		cout << "All Agents have left the simulation.\n";
	} else {
		size_t numPerson = 0;
		size_t numDriver = 0;
		size_t numPedestrian = 0;
		for (vector<Entity*>::iterator it = Agent::all_agents.begin(); it
				!= Agent::all_agents.end(); it++) {
			Person* p = dynamic_cast<Person*> (*it);
			if (p) {
				numPerson++;
				if (p->getRole() && dynamic_cast<Driver*> (p->getRole())) {
					numDriver++;
				}
				if (p->getRole() && dynamic_cast<Pedestrian*> (p->getRole())) {
					numPedestrian++;
				}
			}
		}
		cout << "Remaining Agents: " << numPerson << " (Person)   "
				<< (Agent::all_agents.size() - numPerson) << " (Other)" << endl;
		cout << "   Person Agents: " << numDriver << " (Driver)   "
				<< numPedestrian << " (Pedestrian)   " << (numPerson
				- numDriver - numPedestrian) << " (Other)" << endl;
	}

	if (ConfigParams::GetInstance().numAgentsSkipped>0) {
		cout <<"Agents SKIPPED due to invalid route assignment: " <<ConfigParams::GetInstance().numAgentsSkipped <<endl;
	}

	if (!Agent::pending_agents.empty()) {
		cout << "WARNING! There are still " << Agent::pending_agents.size()
				<< " Agents waiting to be scheduled; next start time is: "
				<< Agent::pending_agents.top()->getStartTime() << " ms\n";
		if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			throw std::runtime_error("ERROR: pending_agents shouldn't be used if Dynamic Dispatch is disabled.");
		}
	}

	//Here, we will simply scope-out the WorkGroups, and they will migrate out all remaining Agents.
	}  //End scope: WorkGroups. (Todo: should move this into its own function later)
	WorkGroup::FinalizeAllWorkGroups();

	//Test: At this point, it should be possible to delete all Signals and Agents.
	clear_delete_vector(Signal::all_signals_);
	clear_delete_vector(Agent::all_agents);

	cout << "Simulation complete; closing worker threads." << endl;
	return true;
}

int main(int argc, char* argv[])
{
#ifdef SIMMOB_NEW_SIGNAL
	std::cout << "Using New Signal Model" << std::endl;
#else
	std::cout << "Not Using New Signal Model" << std::endl;
#endif
	//Save start time
	gettimeofday(&start_time, nullptr);

	/**
	 * Check whether to run SimMobility or SimMobility-MPI
	 * TODO: Retrieving ConfigParams before actually loading the config file is dangerous.
	 */
	ConfigParams& config = ConfigParams::GetInstance();
	config.is_run_on_many_computers = false;
#ifndef SIMMOB_DISABLE_MPI
	if (argc > 3 && strcmp(argv[3], "mpi") == 0) {
		config.is_run_on_many_computers = true;
	}
#endif

	/**
	 * set random be repeatable
	 * TODO: Retrieving ConfigParams before actually loading the config file is dangerous.
	 */
	config.is_simulation_repeatable = true;

	/**
	 * Start MPI if is_run_on_many_computers is true
	 */
#ifndef SIMMOB_DISABLE_MPI
	if (config.is_run_on_many_computers)
	{
		PartitionManager& partitionImpl = PartitionManager::instance();
		std::string mpi_result = partitionImpl.startMPIEnvironment(argc, argv);
		if (mpi_result.compare("") != 0)
		{
			cout << "Error:" << mpi_result << endl;
			exit(1);
		}
	}
#endif

	//Argument 1: Config file
	//Note: Don't chnage this here; change it by supplying an argument on the
	//      command line, or through Eclipse's "Run Configurations" dialog.
	std::string configFileName = "data/config.xml";
	std::string XML_OutPutFileName = "data/SimMobilityInput.xml";
	if (argc > 1)
	{
		configFileName = argv[1];
	}
	else
	{
		cout << "No config file specified; using default." << endl;
	}
	cout << "Using config file: " << configFileName << endl;

	//Argument 2: Log file
#ifndef SIMMOB_DISABLE_OUTPUT
	if (argc > 2) {
		if (!Logger::log_init(argv[2]))
		{
			cout << "Loading output file failed; using cout" << endl;
			cout << argv[2] << endl;
		}
	} else {
		Logger::log_init("");
		cout << "No output file specified; using cout." << endl;
	}


#endif

	//Perform main loop
	int returnVal = performMain(configFileName,"XML_OutPut.xml") ? 0 : 1;

	//Close log file, return.
#ifndef SIMMOB_DISABLE_OUTPUT
	Logger::log_done();
#endif
	cout << "Done" << endl;
	return returnVal;
}

