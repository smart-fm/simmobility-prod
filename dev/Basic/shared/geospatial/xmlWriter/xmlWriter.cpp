#include "xmlWriter.hpp"

#include "buffering/Shared.hpp"
#include "util/DailyTime.hpp"
#include "util/LangHelpers.hpp"

#include "geospatial/RoadItem.hpp"
#include "geospatial/Roundabout.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/RoadSegment.hpp"
//#include "geospatial/BusStop.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/Intersection.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/Node.hpp"
//#include "geospatial/Pavement.hpp"
#include "geospatial/Traversable.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/BusStop.hpp"
#include "conf/simpleconf.hpp"

#include "entities/misc/TripChain.hpp"
#include "entities/roles/RoleFactory.hpp"
#include "entities/signal/Signal.hpp"
#include "entities/signal/Color.hpp"
#include "entities/signal/defaults.hpp"
#include "entities/signal/SplitPlan.hpp"

#include "util/ReactionTimeDistributions.hpp"
#include <iomanip>
void sim_mob::WriteXMLInput_Location(TiXmlElement * parent,bool underLocation, unsigned int X, unsigned int Y)
{
	std::ostringstream Id;
	TiXmlElement * location;
	//should x,y be sub elements of a rudimentary "location" element or just sub elements the parent
	if(underLocation == true)
		{location = new TiXmlElement("location"); parent->LinkEndChild(location);}
	else
		location = parent;

	TiXmlElement * xPos = new TiXmlElement("xPos"); location->LinkEndChild(xPos);
	Id.str("");
	Id << X;
	xPos->LinkEndChild(new  TiXmlText(Id.str()));

	TiXmlElement * yPos = new TiXmlElement("yPos"); location->LinkEndChild(yPos);
	Id.str("");
	Id << Y;
	yPos->LinkEndChild(new  TiXmlText(Id.str()));
}

void sim_mob::WriteXMLInput_laneEdgePolylines_cached(std::vector<std::vector<sim_mob::Point2D> >& polylines,TiXmlElement * laneEdgePolylines_cached)
{
	std::ostringstream Id;
	int i = 0;
	for(std::vector<std::vector<sim_mob::Point2D> >::iterator lane_it = polylines.begin(); lane_it != polylines.end(); lane_it++, i++)
	{
		//laneEdgePolyline_cached
		TiXmlElement * laneEdgePolyline_cached = new TiXmlElement("laneEdgePolyline_cached"); laneEdgePolylines_cached->LinkEndChild(laneEdgePolyline_cached);
		//laneNumber
		TiXmlElement * laneNumber = new TiXmlElement("laneNumber"); laneEdgePolyline_cached->LinkEndChild(laneNumber);
		Id.str("");
		Id << i;
		laneNumber->LinkEndChild(new  TiXmlText(Id.str()));

		//polyline
		const std::vector<sim_mob::Point2D>& polylines_ = polylines.at(i);
		TiXmlElement * polyline = new TiXmlElement("polyline"); laneEdgePolyline_cached->LinkEndChild(polyline);
		WriteXMLInput_PolyLine(polylines_,polyline);
	}
}
void sim_mob::WriteXMLInput_PolyLine(const std::vector<sim_mob::Point2D>& polylines,TiXmlElement * PolyLine)
{
	int j = polylines.size();
	std::ostringstream Id;
	int i = 0;
	for(std::vector<sim_mob::Point2D>::const_iterator polyLineObj_it = polylines.begin(), it_end(polylines.end()); polyLineObj_it != it_end; polyLineObj_it++, i++)
	{
		//PolyPoint
		TiXmlElement * PolyPoint = new TiXmlElement("PolyPoint"); PolyLine->LinkEndChild(PolyPoint);
		TiXmlElement * pointID = new TiXmlElement("pointID"); PolyPoint->LinkEndChild(pointID);
		Id.str("");
		Id << i;
		pointID->LinkEndChild(new  TiXmlText(Id.str()));
		WriteXMLInput_Location(PolyPoint,true,polyLineObj_it->getX(), polyLineObj_it->getY());
	}
}

void sim_mob::WriteXMLInput_Lane(sim_mob::Lane *LaneObj,TiXmlElement *Lanes)
{
	std::ostringstream Id;
	TiXmlElement * Lane = new TiXmlElement("Lane"); Lanes->LinkEndChild(Lane);
	//ID
	TiXmlElement * laneID = new TiXmlElement("laneID"); Lane->LinkEndChild(laneID);
	Id << LaneObj->getLaneID();
	laneID->LinkEndChild(new  TiXmlText(Id.str()));
	//Width
	TiXmlElement * width = new TiXmlElement("width"); Lane->LinkEndChild(width);
	Id.str("");
	Id << LaneObj->getWidth();
	width->LinkEndChild(new  TiXmlText(Id.str()));
//	can_go_straight
	TiXmlElement * can_go_straight = new TiXmlElement("can_go_straight"); Lane->LinkEndChild(can_go_straight);
	can_go_straight->LinkEndChild(new  TiXmlText(LaneObj->can_go_straight() ? "true" : "false"));
//	can_turn_left
	TiXmlElement * can_turn_left = new TiXmlElement("can_turn_left"); Lane->LinkEndChild(can_turn_left);
	can_turn_left->LinkEndChild(new  TiXmlText(LaneObj->can_turn_left() ? "true" : "false"));
//	can_turn_right
	TiXmlElement * can_turn_right = new TiXmlElement("can_turn_right"); Lane->LinkEndChild(can_turn_right);
	can_turn_right->LinkEndChild(new  TiXmlText(LaneObj->can_turn_right() ? "true" : "false"));
//	can_turn_on_red_signal
	TiXmlElement * can_turn_on_red_signal = new TiXmlElement("can_turn_on_red_signal"); Lane->LinkEndChild(can_turn_on_red_signal);
	can_turn_on_red_signal->LinkEndChild(new  TiXmlText(LaneObj->can_turn_on_red_signal() ? "true" : "false"));
//	can_change_lane_left
	TiXmlElement * can_change_lane_left = new TiXmlElement("can_change_lane_left"); Lane->LinkEndChild(can_change_lane_left);
	can_change_lane_left->LinkEndChild(new  TiXmlText(LaneObj->can_change_lane_left() ? "true" : "false"));
//	can_change_lane_right
	TiXmlElement * can_change_lane_right = new TiXmlElement("can_change_lane_right"); Lane->LinkEndChild(can_change_lane_right);
	can_change_lane_right->LinkEndChild(new  TiXmlText(LaneObj->can_change_lane_right() ? "true" : "false"));
//	is_road_shoulder
	TiXmlElement * is_road_shoulder = new TiXmlElement("is_road_shoulder"); Lane->LinkEndChild(is_road_shoulder);
	is_road_shoulder->LinkEndChild(new  TiXmlText(LaneObj->is_road_shoulder() ? "true" : "false"));
//	is_bicycle_lane
	TiXmlElement * is_bicycle_lane = new TiXmlElement("is_bicycle_lane"); Lane->LinkEndChild(is_bicycle_lane);
	is_bicycle_lane->LinkEndChild(new  TiXmlText(LaneObj->is_bicycle_lane() ? "true" : "false"));
//	is_pedestrian_lane
	TiXmlElement * is_pedestrian_lane = new TiXmlElement("is_pedestrian_lane"); Lane->LinkEndChild(is_pedestrian_lane);
	is_pedestrian_lane->LinkEndChild(new  TiXmlText(LaneObj->is_pedestrian_lane() ? "true" : "false"));
//	is_vehicle_lane
	TiXmlElement * is_vehicle_lane = new TiXmlElement("is_vehicle_lane"); Lane->LinkEndChild(is_vehicle_lane);
	is_vehicle_lane->LinkEndChild(new  TiXmlText(LaneObj->is_vehicle_lane() ? "true" : "false"));
//	is_standard_bus_lane
	TiXmlElement * is_standard_bus_lane = new TiXmlElement("is_standard_bus_lane"); Lane->LinkEndChild(is_standard_bus_lane);
	is_standard_bus_lane->LinkEndChild(new  TiXmlText(LaneObj->is_standard_bus_lane() ? "true" : "false"));
//	is_whole_day_bus_lane
	TiXmlElement * is_whole_day_bus_lane = new TiXmlElement("is_whole_day_bus_lane"); Lane->LinkEndChild(is_whole_day_bus_lane);
	is_whole_day_bus_lane->LinkEndChild(new  TiXmlText(LaneObj->is_whole_day_bus_lane() ? "true" : "false"));
//	is_high_occupancy_vehicle_lane
	TiXmlElement * is_high_occupancy_vehicle_lane = new TiXmlElement("is_high_occupancy_vehicle_lane"); Lane->LinkEndChild(is_high_occupancy_vehicle_lane);
	is_high_occupancy_vehicle_lane->LinkEndChild(new  TiXmlText(LaneObj->is_high_occupancy_vehicle_lane() ? "true" : "false"));
//	can_freely_park_here
	TiXmlElement * can_freely_park_here = new TiXmlElement("can_freely_park_here"); Lane->LinkEndChild(can_freely_park_here);
	can_freely_park_here->LinkEndChild(new  TiXmlText(LaneObj->can_freely_park_here() ? "true" : "false"));
//	can_stop_here
	TiXmlElement * can_stop_here = new TiXmlElement("can_stop_here"); Lane->LinkEndChild(can_stop_here);
	can_stop_here->LinkEndChild(new  TiXmlText(LaneObj->can_stop_here() ? "true" : "false"));
//	is_u_turn_allowed
	TiXmlElement * is_u_turn_allowed = new TiXmlElement("is_u_turn_allowed"); Lane->LinkEndChild(is_u_turn_allowed);
	is_u_turn_allowed->LinkEndChild(new  TiXmlText(LaneObj->is_u_turn_allowed() ? "true" : "false"));
	//Polyline

/*	we temporarily discard ployline serialization in here as Lane->getPolyline() doesn't
 * return polylines as it should. it 'creates' the polylines from within this getter function!!!
 * so if the function is used in any place other than what was originally ment to be used in,
 * it wn't show any flexibility ands messes up iterators, creates seg faults etc.*/
	/*update: I made an exception to getPolyline by adding a boolean argument to the function getPolyline()
	 * which controls the act of creating polylines
	 */
//	if(LaneObj->getLaneID() == 1000001000)
//	{
//		std::vector<sim_mob::Point2D> PolyLine = LaneObj->polyline_;
//
//		  for(std::vector<sim_mob::Point2D>::iterator it = PolyLine.begin(); it != PolyLine.end(); it++)
//		  {
//			  std::cout << "xml-witer Lane 1000001000 polypoint " << it->getX() << "," << it->getY() << std::endl;
//		  }
//	}
  if(LaneObj->getPolyline(false).size())
  {
	  TiXmlElement * PolyLine = new TiXmlElement("PolyLine"); Lane->LinkEndChild(PolyLine);
	  WriteXMLInput_PolyLine(LaneObj->getPolyline(false),PolyLine);
  }

}

void sim_mob::WriteXMLInput_BusStop(sim_mob::BusStop * busStop , int offset, TiXmlElement *Obstacle)
{
	std::ostringstream output;
	output.setf(std::ios::fixed);
	std::cout.setf(std::ios::fixed);
	TiXmlElement * BusStop = new TiXmlElement("BusStop"); Obstacle->LinkEndChild(BusStop);
	//id
	TiXmlElement * id = new TiXmlElement("id"); BusStop->LinkEndChild(id);
	output.str("");
	output << busStop->getRoadItemID();
	id->LinkEndChild(new  TiXmlText(output.str()));
	//offset
	TiXmlElement * Offset = new TiXmlElement("Offset"); BusStop->LinkEndChild(Offset);
	output.str("");
	output << offset;
	Offset->LinkEndChild(new  TiXmlText(output.str()));
	//start
	TiXmlElement * start = new TiXmlElement("start"); BusStop->LinkEndChild(start);
	WriteXMLInput_Location(start,false,busStop->getStart().getX(),busStop->getStart().getY());
	//end
	TiXmlElement * end = new TiXmlElement("end"); BusStop->LinkEndChild(end);
	WriteXMLInput_Location(end,false,busStop->getStart().getX(),busStop->getStart().getY());
	//xPos
	TiXmlElement * xPos = new TiXmlElement("xPos"); BusStop->LinkEndChild(xPos);
	output.str("");
	output << std::setprecision(4) <<  busStop->xPos;
	xPos->LinkEndChild(new  TiXmlText(output.str()));
	//yPos
	TiXmlElement * yPos = new TiXmlElement("yPos"); BusStop->LinkEndChild(yPos);
	output.str("");
	output << std::setprecision(4) << busStop->yPos;
	yPos->LinkEndChild(new  TiXmlText(output.str()));
	//lane_location
	if(busStop->getLaneLocation())
	{
		const sim_mob::Lane *lane =  busStop->getLaneLocation();
		TiXmlElement * lane_location = new TiXmlElement("lane_location"); BusStop->LinkEndChild(lane_location);
		output.str("");
		output << lane->getLaneID();
		lane_location->LinkEndChild(new  TiXmlText(output.str()));
	}

	//is_terminal
	TiXmlElement * is_terminal = new TiXmlElement("is_terminal"); BusStop->LinkEndChild(is_terminal);
	output.str("");
	output << (busStop->isTerminal() ? "true" : "false");
	is_terminal->LinkEndChild(new  TiXmlText(output.str()));
	//is_bay
	TiXmlElement * is_bay = new TiXmlElement("is_bay"); BusStop->LinkEndChild(is_bay);
	output.str("");
	output << (busStop->isBay() ? "true" : "false");
	is_bay->LinkEndChild(new  TiXmlText(output.str()));
	//has_shelter
	TiXmlElement * has_shelter = new TiXmlElement("has_shelter"); BusStop->LinkEndChild(has_shelter);
	output.str("");
	output << (busStop->hasShelter() ? "true" : "false");
	has_shelter->LinkEndChild(new  TiXmlText(output.str()));
	//busCapacityAsLength
	TiXmlElement * busCapacityAsLength = new TiXmlElement("busCapacityAsLength"); BusStop->LinkEndChild(busCapacityAsLength);
	output.str("");
	output << busStop->getBusCapacityAsLength();
	busCapacityAsLength->LinkEndChild(new  TiXmlText(output.str()));
	//busstopno
	TiXmlElement * busstopno = new TiXmlElement("busstopno"); BusStop->LinkEndChild(busstopno);
	output.str("");
	output << busStop->getBusstopno_();
	busstopno->LinkEndChild(new  TiXmlText(output.str()));
}

void sim_mob::WriteXMLInput_Crossing(sim_mob::Crossing * crossing , int offset, TiXmlElement *Obstacle)
{
	std::ostringstream output;
	TiXmlElement * Crossing = new TiXmlElement("Crossing"); Obstacle->LinkEndChild(Crossing);
	//id
	TiXmlElement * id = new TiXmlElement("id"); Crossing->LinkEndChild(id);
	output.str("");
	output << crossing->getCrossingID();
	id->LinkEndChild(new  TiXmlText(output.str()));
	//offset
	TiXmlElement * Offset = new TiXmlElement("Offset"); Crossing->LinkEndChild(Offset);
	output.str("");
	output << offset;
	Offset->LinkEndChild(new  TiXmlText(output.str()));
	//start
	TiXmlElement * start = new TiXmlElement("start"); Crossing->LinkEndChild(start);
	WriteXMLInput_Location(start,false,crossing->getStart().getX(),crossing->getStart().getY());
	//end
	TiXmlElement * end = new TiXmlElement("end"); Crossing->LinkEndChild(end);
	WriteXMLInput_Location(end,false,crossing->getStart().getX(),crossing->getStart().getY());
	{//nearLine
		TiXmlElement * nearLine = new TiXmlElement("nearLine"); Crossing->LinkEndChild(nearLine);
		TiXmlElement * first = new TiXmlElement("first"); nearLine->LinkEndChild(first);
		WriteXMLInput_Location(first,false,crossing->nearLine.first.getX(),crossing->nearLine.first.getY());
		TiXmlElement * second = new TiXmlElement("second"); nearLine->LinkEndChild(second);
		WriteXMLInput_Location(second,false,crossing->nearLine.second.getX(),crossing->nearLine.second.getY());
	}
	{//farLine
		TiXmlElement * farLine = new TiXmlElement("farLine"); Crossing->LinkEndChild(farLine);
		TiXmlElement * first = new TiXmlElement("first"); farLine->LinkEndChild(first);
		WriteXMLInput_Location(first,false,crossing->farLine.first.getX(),crossing->farLine.first.getY());
		TiXmlElement * second = new TiXmlElement("second"); farLine->LinkEndChild(second);
		WriteXMLInput_Location(second,false,crossing->farLine.second.getX(),crossing->farLine.second.getY());
	}
}
bool sim_mob::WriteXMLInput_Obstacle(sim_mob::RoadItemAndOffsetPair res, TiXmlElement * Obstacle)
{
	sim_mob::Crossing * crossing = dynamic_cast<sim_mob::Crossing *>(const_cast<sim_mob::RoadItem *>(res.item));
	if(crossing)
	{
//		sim_mob::RoadSegment * rs = crossing->getRoadSegment();
		WriteXMLInput_Crossing(crossing, res.offset, Obstacle);
	}else{
		if(dynamic_cast<sim_mob::BusStop *>(const_cast<sim_mob::RoadItem *>(res.item)))
		{
			sim_mob::BusStop * bs = dynamic_cast<sim_mob::BusStop *>(const_cast<sim_mob::RoadItem *>(res.item));
//			sim_mob::RoadSegment * rs = bs->getRoadSegment();
			WriteXMLInput_BusStop(bs, res.offset, Obstacle);

		}
		else
		{
			return false;
		}
	}
	return true;
}

void sim_mob::WriteXMLInput_Segment(sim_mob::RoadSegment* rs ,TiXmlElement * Segments)
{
	std::ostringstream Id;

	//Segment
	TiXmlElement * Segment = new TiXmlElement("Segment"); Segments->LinkEndChild(Segment);
	//segmentID
	TiXmlElement * segmentID = nullptr;
	segmentID = new TiXmlElement("segmentID"); Segment->LinkEndChild(segmentID);
	Id.str("");
	Id << rs->getSegmentID();
	segmentID->LinkEndChild(new  TiXmlText(Id.str()));
	//start
	TiXmlElement * startingNode = new TiXmlElement("startingNode"); Segment->LinkEndChild(startingNode);
	Id.str("");
	Id << rs->getStart()->getID();
	startingNode->LinkEndChild(new  TiXmlText(Id.str()));
	//end
	TiXmlElement * endingNode = new TiXmlElement("endingNode"); Segment->LinkEndChild(endingNode);
	Id.str("");
	Id << rs->getEnd()->getID();
	endingNode->LinkEndChild(new  TiXmlText(Id.str()));
	//maxSpeed
	TiXmlElement * maxSpeed = new TiXmlElement("maxSpeed"); Segment->LinkEndChild(maxSpeed);
	Id.str("");
	Id << rs->maxSpeed;
	maxSpeed->LinkEndChild(new  TiXmlText(Id.str()));
	//Length
	TiXmlElement * Length = new TiXmlElement("Length"); Segment->LinkEndChild(Length);
	Id.str("");
	Id << rs->length;
	Length->LinkEndChild(new  TiXmlText(Id.str()));
	//Width
	TiXmlElement * Width = new TiXmlElement("Width"); Segment->LinkEndChild(Width);
	Id.str("");
	Id << rs->width;
	Width->LinkEndChild(new  TiXmlText(Id.str()));
	//originalDB_ID
	TiXmlElement * originalDB_ID = new TiXmlElement("originalDB_ID");  Segment->LinkEndChild(originalDB_ID);
	originalDB_ID->LinkEndChild( new TiXmlText(rs->originalDB_ID.getLogItem()));
	//polyline
	TiXmlElement * polyline = new TiXmlElement("polyline"); Segment->LinkEndChild(polyline);
	WriteXMLInput_PolyLine(const_cast<std::vector<sim_mob::Point2D>& >(rs->polyline), polyline);
	//laneEdgePolylines_cached
	TiXmlElement * laneEdgePolylines_cached = new TiXmlElement("laneEdgePolylines_cached"); Segment->LinkEndChild(laneEdgePolylines_cached);
	WriteXMLInput_laneEdgePolylines_cached(rs->laneEdgePolylines_cached,laneEdgePolylines_cached);
	//Lanes
	TiXmlElement * Lanes = new TiXmlElement("Lanes"); Segment->LinkEndChild(Lanes);
	//Lane
	int i = 0;

	for(std::vector<sim_mob::Lane*>::const_iterator LaneObj_it = rs->getLanes().begin(), it_end(rs->getLanes().end()); LaneObj_it != it_end ; LaneObj_it++)
	{
		sim_mob::Lane* temp = *LaneObj_it;
		WriteXMLInput_Lane(*LaneObj_it,Lanes);
		i++;
	}
	TiXmlElement * Obstacles = new TiXmlElement("Obstacles"); Segment->LinkEndChild(Obstacles);
	for (std::map<centimeter_t, const RoadItem*>::iterator it = rs->getObstacles().begin(); it != rs->getObstacles().end(); it++) {
		//just for the sake of backward compatibility  as well as easier future improvements
		sim_mob::RoadItemAndOffsetPair res(it->second,it->first);
		WriteXMLInput_Obstacle(res, Obstacles);
	}
//	for (int currOffset = 0;currOffset <= rs->length ;) {
//		//Get the next item, if any.
//		RoadItemAndOffsetPair res = rs->nextObstacle(currOffset, true);
//		if (!res.item) {
//			break;
//		}
//		if(!WriteXMLInput_Obstacle(res, Obstacles))
//		{
//			if(100001500 == rs->getSegmentID())
//			{
//				std::cout << "Segment 100001500, One of the obstacles at offset " << res.offset << " just didn't tally\n";
//			}
//		}
//		else
//		{
//			if(100001500 == rs->getSegmentID())
//			{
//				std::cout << "Segment 100001500, an obstacles at offset " << res.offset << " was serialized\n";
//			}
//
//		}
//		currOffset += res.offset + 1;
//	}

}

//TODO: Not sure I fixed this correctly. ~Seth
//      (But we will switch to Xerces output anyway...)
void sim_mob::WriteXMLInput_Segments(sim_mob::Link* LinkObj ,TiXmlElement * Link)
{
	//Segments element first
	TiXmlElement * Segments = new TiXmlElement("Segments");
		//FWDSegments
    //TiXmlElement * FWDSegments = new TiXmlElement("FWDSegments");
    std::vector<sim_mob::RoadSegment*>::const_iterator SegObj_it = (LinkObj)->getSegments().begin();
    //validation
    /*if(SegObj_it != (LinkObj)->getSegments().end())
    	Segments->LinkEndChild(FWDSegments);*/
    for(; (SegObj_it != (LinkObj)->getSegments().end()) ; SegObj_it++)
    {
    	WriteXMLInput_Segment(*SegObj_it,Segments);
    }

    //BKDSegments
    /*(TiXmlElement * BKDSegments = new TiXmlElement("BKDSegments");
    SegObj_it = (LinkObj)->getRevSegments().begin();
    //validation
    if((LinkObj)->getRevSegments().begin() != (LinkObj)->getRevSegments().end())
    	 Segments->LinkEndChild(BKDSegments);
    for(; (SegObj_it != (LinkObj)->getRevSegments().end()) ; SegObj_it++)
    {
      	WriteXMLInput_Segment(*SegObj_it,BKDSegments);
    }*/
    //validation:if you don't have any FWDSegments and BKDSegments, don't add parent element 'Segments'
 //   if(!(((LinkObj)->getFwdSegments().begin() == (LinkObj)->getFwdSegments().end())))
    	Link->LinkEndChild(Segments);
}

void sim_mob::WriteXMLInput_Links(const std::vector<sim_mob::Link*>& link,TiXmlElement * RoadNetwork)
{
	std::ostringstream out;
    TiXmlElement * Links = new TiXmlElement("Links");
    std::vector<sim_mob::Link*>::const_iterator LinksObj_it = link.begin();
    if(LinksObj_it != link.end())
    	RoadNetwork->LinkEndChild(Links);
    for( ;LinksObj_it != link.end() ; LinksObj_it++)
    {
    	//Link emlement
    	TiXmlElement * Link = new TiXmlElement("Link"); Links->LinkEndChild(Link);
    	//LinkID
    	TiXmlElement * linkID = new TiXmlElement("linkID");
    	out.str("");
    	out << (*LinksObj_it)->getLinkId();
    	if(out.str().size())
    	{
        	linkID->LinkEndChild( new TiXmlText((out.str())));
    		Link->LinkEndChild(linkID);
    	}
    	//RoadName
    	TiXmlElement * roadName = new TiXmlElement("roadName");  Link->LinkEndChild(roadName);
    	roadName->LinkEndChild( new TiXmlText(((*LinksObj_it)->getRoadName())));
    	//StartingNode
    	TiXmlElement * StartingNode = new TiXmlElement("StartingNode");
    	out.str("");
    	out << (*LinksObj_it)->getStart()->getID();
    	if(out.str().size())
    	{
    		StartingNode->LinkEndChild( new TiXmlText((out.str())));
    		Link->LinkEndChild(StartingNode);
    	}
    	//EndingNode
    	TiXmlElement * EndingNode = new TiXmlElement("EndingNode");
    	out.str("");
    	out << (*LinksObj_it)->getEnd()->getID();
    	if(out.str().size())
    	{
    		EndingNode->LinkEndChild( new TiXmlText((out.str())));
    		Link->LinkEndChild(EndingNode);
    	}
    	WriteXMLInput_Segments(*LinksObj_it,Link);
    }
}

void sim_mob::WriteXMLInput_UniNode_Connectors(sim_mob::UniNode* uninode,TiXmlElement * UniNode_)
{
	std::ostringstream out;
	TiXmlElement * Connectors = new TiXmlElement("Connectors"); UniNode_->LinkEndChild(Connectors);
	TiXmlElement * Connector;
	TiXmlElement * laneFrom;
	TiXmlElement * laneTo;

	for(std::map<const sim_mob::Lane*, sim_mob::Lane* >::const_iterator it = uninode->getConnectors().begin(),it_end(uninode->getConnectors().end());it != it_end; it++)
	{
		Connector = new TiXmlElement("Connector"); Connectors->LinkEndChild(Connector);
		laneFrom = new TiXmlElement("laneFrom"); Connector->LinkEndChild(laneFrom);
		laneTo = new TiXmlElement("laneTo"); Connector->LinkEndChild(laneTo);
		out.str(""); out << (*it).first->getLaneID();
		laneFrom->LinkEndChild( new TiXmlText(out.str()));
		out.str(""); out << (*it).second->getLaneID();
		laneTo->LinkEndChild( new TiXmlText(out.str()));
	}
}
void sim_mob::WriteXMLInput_Node(sim_mob::Node *node, TiXmlElement * parent)
{
	std::ostringstream out;
	//nodeId
	TiXmlElement * nodeID = new TiXmlElement("nodeID"); parent->LinkEndChild(nodeID);
	out.str("");
	out << node->getID();
	nodeID->LinkEndChild( new TiXmlText(out.str()));
	//location
	WriteXMLInput_Location(parent,true,node->getLocation().getX(),node->getLocation().getY());
	//linkLoc
	/*TiXmlElement * linkLoc = new TiXmlElement("linkLoc"); parent->LinkEndChild(linkLoc);
	out.str("");
	out << node->getLinkLoc()->linkID;
	linkLoc->LinkEndChild( new TiXmlText(out.str()));*/
	//originalDB_ID
	TiXmlElement * originalDB_ID = new TiXmlElement("originalDB_ID");  parent->LinkEndChild(originalDB_ID);
	originalDB_ID->LinkEndChild( new TiXmlText(node->originalDB_ID.getLogItem()));
}
void sim_mob::WriteXMLInput_UniNode_SegmentPair(TiXmlElement * UniNode, std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> thePair, bool firstPair)
{
	std::ostringstream out;

	std::string thePair_ = firstPair ? "firstPair" : "secondPair";
	TiXmlElement * PairElement = 0;
	if(((thePair.first) && (thePair.first->getSegmentID() > 0)) && ((thePair.second) && (thePair.second->getSegmentID() > 0)))
	{
		PairElement = new TiXmlElement(thePair_);
		UniNode->LinkEndChild(PairElement);
	}

	if((thePair.first) && (thePair.first->getSegmentID() > 0))
	{
		out.str("");
		TiXmlElement * first = new TiXmlElement("first"); PairElement->LinkEndChild(first);
		out << thePair.first->getSegmentID();
		first->LinkEndChild( new TiXmlText(out.str()));
	}

	if((thePair.second) && (thePair.second->getSegmentID() > 0))
	{
		out.str("");
		TiXmlElement * second = new TiXmlElement("second"); PairElement->LinkEndChild(second);
		out << thePair.second->getSegmentID();
		second->LinkEndChild( new TiXmlText(out.str()));
	}
}
void sim_mob::WriteXMLInput_UniNodes(sim_mob::RoadNetwork & roadNetwork,TiXmlElement * Nodes)
{
	TiXmlElement * UniNodes = new TiXmlElement("UniNodes"); Nodes->LinkEndChild(UniNodes);
	for(std::set<sim_mob::UniNode*>::const_iterator it = roadNetwork.getUniNodes().begin(), it_end( roadNetwork.getUniNodes().end()); it != it_end ; it++ )
	{
		TiXmlElement * UniNode = new TiXmlElement("UniNode"); UniNodes->LinkEndChild(UniNode);
		WriteXMLInput_Node(*it, UniNode);//basic node information
		//firstPairSegment
		WriteXMLInput_UniNode_SegmentPair(UniNode,(*it)->getRoadSegmentPair(true), true);//firstPair
		//secondpair segment--repeatition :)
		WriteXMLInput_UniNode_SegmentPair(UniNode,(*it)->getRoadSegmentPair(false), false);//secondPair
		//connectors
		WriteXMLInput_UniNode_Connectors(*it,UniNode);
	}
}

void sim_mob::WriteXMLInput_roadSegmentsAt(sim_mob::MultiNode * mn,TiXmlElement * Intersection)
{
	//roadSegmentsAt
	TiXmlElement * roadSegmentsAt = new TiXmlElement("roadSegmentsAt"); Intersection->LinkEndChild(roadSegmentsAt);
	for(std::set<sim_mob::RoadSegment*>::const_iterator rsObj_it = (mn)->getRoadSegments().begin(),it_end((mn)->getRoadSegments().end()) ; rsObj_it != it_end; rsObj_it++)
	{
		TiXmlElement * segmentID = new TiXmlElement("segmentID"); roadSegmentsAt->LinkEndChild(segmentID);
		std::ostringstream Id;
		Id.str("");
		Id << (*rsObj_it)->getSegmentID();
		segmentID->LinkEndChild( new TiXmlText(Id.str()));
	}
}

void sim_mob::WriteXMLInput_MultiNode_Connectors(sim_mob::MultiNode* mn,TiXmlElement * MultiNode)
{
	std::ostringstream out;
	TiXmlElement * Connectors = new TiXmlElement("Connectors"); MultiNode->LinkEndChild(Connectors);

//	std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> >::iterator conn_it = mn->getConnectors().begin();
	for(std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> >::const_iterator conn_it = mn->getConnectors().begin(), it_end(mn->getConnectors().end()); conn_it != it_end; conn_it++)
	{
		TiXmlElement * MultiConnectors = new TiXmlElement("MultiConnectors"); Connectors->LinkEndChild(MultiConnectors);
		TiXmlElement * RoadSegment = new TiXmlElement("RoadSegment"); MultiConnectors->LinkEndChild(RoadSegment);

		std::ostringstream Id;
		Id.str("");
		Id << (*conn_it).first->getSegmentID();
		RoadSegment->LinkEndChild( new TiXmlText(Id.str()));

		TiXmlElement * Connectors = new TiXmlElement("Connectors"); MultiConnectors->LinkEndChild(Connectors);
		for(std::set<sim_mob::LaneConnector*> ::const_iterator l_conn_it = conn_it->second.begin(), it_end(conn_it->second.end()); l_conn_it != it_end ; l_conn_it++)
		{
			TiXmlElement * Connector = new TiXmlElement("Connector"); Connectors->LinkEndChild(Connector);
			TiXmlElement * laneFrom = new TiXmlElement("laneFrom"); Connector->LinkEndChild(laneFrom);
			TiXmlElement * laneTo = new TiXmlElement("laneTo"); Connector->LinkEndChild(laneTo);
			out.str(""); out << (*l_conn_it)->getLaneFrom()->getLaneID();
			laneFrom->LinkEndChild( new TiXmlText(out.str()));

			out.str(""); out << (*l_conn_it)->getLaneTo()->getLaneID();
			laneTo->LinkEndChild( new TiXmlText(out.str()));
		}
	}
}

TiXmlElement * sim_mob::WriteXMLInput_Intersection(sim_mob::Intersection *intersection,TiXmlElement * Intersections, TiXmlElement * Nodes)
{
	std::ostringstream out;
	if(!Intersections) { Intersections = new TiXmlElement("Intersections"); Nodes->LinkEndChild(Intersections);}
		out.str("");
		TiXmlElement * Intersection = new TiXmlElement("Intersection"); Intersections->LinkEndChild(Intersection);
//    	//nodeID
//    	TiXmlElement * nodeID = new TiXmlElement("nodeID");  Intersection->LinkEndChild(nodeID);
//    	out << (intersection)->getID();
//    	nodeID->LinkEndChild( new TiXmlText((out.str())));
//    	//originalDB_ID
//    	TiXmlElement * originalDB_ID = new TiXmlElement("originalDB_ID");  Intersection->LinkEndChild(originalDB_ID);
//    	originalDB_ID->LinkEndChild( new TiXmlText(intersection->originalDB_ID.getLogItem()));
//    	//location
//    	WriteXMLInput_Location(Intersection,true,(intersection)->location.getX(),(intersection)->location.getY());
    	WriteXMLInput_Node(intersection, Intersection);//basic node information
    	WriteXMLInput_roadSegmentsAt(intersection,Intersection);
    	WriteXMLInput_MultiNode_Connectors(intersection,Intersection);
    	return Intersections;//used to decide whether create a parent "Intersections" element or not
}
TiXmlElement * sim_mob::WriteXMLInput_Roundabout(sim_mob::Roundabout *roundabout,TiXmlElement * roundabouts,TiXmlElement * Nodes)
{
	if(!roundabouts) { roundabouts = new TiXmlElement("roundabouts"); Nodes->LinkEndChild(roundabouts);}
	return roundabouts;
}

void sim_mob::WriteXMLInput_Nodes(sim_mob::RoadNetwork roadNetwork,TiXmlElement * RoadNetwork)
{
    TiXmlElement * Nodes = new TiXmlElement("Nodes"); RoadNetwork->LinkEndChild(Nodes);
	WriteXMLInput_UniNodes(roadNetwork,Nodes);
	TiXmlElement * intersections = 0, *roundabouts = 0;
	std::vector<sim_mob::MultiNode*>::const_iterator mNode_it = roadNetwork.getNodes().begin();
	for(; mNode_it != roadNetwork.getNodes().end() ; mNode_it++)
	{
		if(dynamic_cast<sim_mob::Intersection*>(*mNode_it))
			intersections = WriteXMLInput_Intersection(dynamic_cast<sim_mob::Intersection*>(*mNode_it),intersections, Nodes);
		if(dynamic_cast<sim_mob::Roundabout*>(*mNode_it))
			roundabouts = WriteXMLInput_Roundabout(dynamic_cast<sim_mob::Roundabout*>(*mNode_it),roundabouts, Nodes);
	}
}
void sim_mob::WriteXMLInput_RoadNetwork(TiXmlElement * GeoSpatial)
{
	TiXmlElement * RoadNetwork = new TiXmlElement("RoadNetwork");  GeoSpatial->LinkEndChild(RoadNetwork);
	sim_mob::ConfigParams& config = sim_mob::ConfigParams::GetInstance();


    sim_mob::RoadNetwork RoadNetworkObj = config.getNetwork();
    const std::vector<sim_mob::Link*>& LinksObj = RoadNetworkObj.getLinks();
    WriteXMLInput_Nodes(RoadNetworkObj,RoadNetwork);
    WriteXMLInput_Links(LinksObj,RoadNetwork);
}

//unlikely to be used
void sim_mob::WriteXMLInput_RoadItems(TiXmlElement * GeoSpatial)
{
	TiXmlElement * RoadItems = new TiXmlElement("RoadItems"); 	   GeoSpatial->LinkEndChild(RoadItems);
    TiXmlElement * BusStops = new TiXmlElement("BusStops"); RoadItems->LinkEndChild(BusStops);
    TiXmlElement * ERP_Gantries = new TiXmlElement("ERP_Gantries"); RoadItems->LinkEndChild(ERP_Gantries);
    TiXmlElement * Crossings = new TiXmlElement("Crossings"); RoadItems->LinkEndChild(Crossings);
    TiXmlElement * RoadBumps = new TiXmlElement("RoadBumps"); RoadItems->LinkEndChild(RoadBumps);
}
void sim_mob::WriteXMLInput_GeoSpatial(TiXmlElement * SimMobility)
{
	TiXmlElement * GeoSpatial;
    GeoSpatial = new TiXmlElement( "GeoSpatial" );
    SimMobility->LinkEndChild( GeoSpatial );
    WriteXMLInput_RoadNetwork(GeoSpatial);
}

void sim_mob::WriteXMLInput_TripChainItem(TiXmlElement * TripChain, sim_mob::TripChainItem & tripChainItem)
{
	std::ostringstream out;
//	//entityID
//	out.str("");
//	out << tripChainItem.entityID;
//	TiXmlElement * entityID  = new TiXmlElement( "entityID" );
//	entityID->LinkEndChild( new TiXmlText(out.str()));
//	TripChain->LinkEndChild( entityID );
	//personID
	out.str("");
	out << tripChainItem.personID;//TODO: sunc with tripchainitem(last decision was to replace entityID with personID
	TiXmlElement * personID  = new TiXmlElement( "personID" );
	personID->LinkEndChild( new TiXmlText(out.str()));
	TripChain->LinkEndChild( personID );

	//itemType
	out.str("");
	switch(tripChainItem.itemType)
	{
	case sim_mob::TripChainItem::IT_ACTIVITY:
		out << "IT_ACTIVITY";
		break;

	case sim_mob::TripChainItem::IT_TRIP:
		out << "IT_TRIP";
		break;
	}
	TiXmlElement * itemType  = new TiXmlElement( "itemType" );
	itemType->LinkEndChild( new TiXmlText(out.str()));
	TripChain->LinkEndChild( itemType );

	//sequenceNumber
	out.str("");
	out << tripChainItem.sequenceNumber;
	TiXmlElement * sequenceNumber  = new TiXmlElement( "sequenceNumber" );
	sequenceNumber->LinkEndChild( new TiXmlText(out.str()));
	TripChain->LinkEndChild( sequenceNumber );

	//startTime
//	out.str("");
//	out << tripChainItem.startTime.toString();
	TiXmlElement * startTime  = new TiXmlElement( "startTime" );
	startTime->LinkEndChild( new TiXmlText(tripChainItem.startTime.toString()));
	TripChain->LinkEndChild( startTime );
	//endTime
	TiXmlElement * endTime  = new TiXmlElement( "endTime" );
	endTime->LinkEndChild( new TiXmlText(tripChainItem.endTime.toString()));
	TripChain->LinkEndChild( endTime );
}
//duto some special/weird design in tripchain hierarchy, we need to copy tripchainitem and trip information the way you see
void sim_mob::WriteXMLInput_TripChain_Subtrips(TiXmlElement * Trip,  sim_mob::Trip & trip)
{
	TiXmlElement * Subtrips  = new TiXmlElement( "subTrips" ); Trip->LinkEndChild( Subtrips );
	std::ostringstream out;
	for(std::vector<SubTrip>::const_iterator it = trip.getSubTrips().begin(), it_end(trip.getSubTrips().end()); it != it_end; it++)
	{
		TiXmlElement * Subtrip  = new TiXmlElement( "subTrip" ); Subtrips->LinkEndChild( Subtrip );
		///////////tripchainitem part//////////////////
		//personID
		out.str("");
		out << it->personID;
		TiXmlElement * personID  = new TiXmlElement( "personID" );
		personID->LinkEndChild( new TiXmlText(out.str()));
		Subtrip->LinkEndChild( personID );
		//itemType
		out.str("");

		switch(it->itemType)
		{
		case sim_mob::TripChainItem::IT_ACTIVITY:
			out << "IT_ACTIVITY";
			break;

		case sim_mob::TripChainItem::IT_TRIP:
			out << "IT_TRIP";
			break;
		}
		TiXmlElement * itemType  = new TiXmlElement( "itemType" );
		itemType->LinkEndChild( new TiXmlText(out.str()));
		Subtrip->LinkEndChild( itemType );
		//sequenceNumber
		out.str("");
		out << it->sequenceNumber;
		TiXmlElement * sequenceNumber  = new TiXmlElement( "sequenceNumber" );
		sequenceNumber->LinkEndChild( new TiXmlText(out.str()));
		Subtrip->LinkEndChild( sequenceNumber );
		//startTime
		out.str("");
		out << it->startTime.toString();
		TiXmlElement * startTime  = new TiXmlElement( "startTime" );
		startTime->LinkEndChild( new TiXmlText(out.str()));
		Subtrip->LinkEndChild( startTime );
		//endTime
		out.str("");
		out << it->endTime.toString();
		TiXmlElement * endTime  = new TiXmlElement( "endTime" );
		endTime->LinkEndChild( new TiXmlText(out.str()));
		Subtrip->LinkEndChild( endTime );
		///////////trip part//////////////////
		//tripID
		out.str("");
		out << it->tripID;
		TiXmlElement * tripID  = new TiXmlElement( "tripID" );
		tripID->LinkEndChild( new TiXmlText(out.str()));
		Subtrip->LinkEndChild( tripID );
		//fromLocation
		out.str("");
		out << it->fromLocation->getID();
		TiXmlElement * fromLocation  = new TiXmlElement( "fromLocation" );
		fromLocation->LinkEndChild( new TiXmlText(out.str()));
		Subtrip->LinkEndChild( fromLocation );
		//fromLocationType
		out.str("");
		switch(it->fromLocationType)
		{
		case TripChainItem::LT_BUILDING:
			out << "LT_BUILDING";
			break;
		case TripChainItem::LT_NODE:
			out << "LT_NODE";
			break;
		case TripChainItem::LT_LINK:
			out << "LT_LINK";
			break;
		case TripChainItem::LT_PUBLIC_TRANSIT_STOP:
			out << "LT_PUBLIC_TRANSIT_STOP";
			break;
		}
		TiXmlElement * fromLocationType  = new TiXmlElement( "fromLocationType" );
		fromLocationType->LinkEndChild( new TiXmlText(out.str()));
		Subtrip->LinkEndChild( fromLocationType );
		//toLocation
		out.str("");
		out << it->toLocation->getID();
		TiXmlElement * toLocation  = new TiXmlElement( "toLocation" );
		toLocation->LinkEndChild( new TiXmlText(out.str()));
		Subtrip->LinkEndChild( toLocation );
		//toLocationType
		out.str("");
		switch(it->toLocationType)
		{
		case TripChainItem::LT_BUILDING:
			out << "LT_BUILDING";
			break;
		case TripChainItem::LT_NODE:
			out << "LT_NODE";
			break;
		case TripChainItem::LT_LINK:
			out << "LT_LINK";
			break;
		case TripChainItem::LT_PUBLIC_TRANSIT_STOP:
			out << "LT_PUBLIC_TRANSIT_STOP";
			break;
		}
		TiXmlElement * toLocationType  = new TiXmlElement( "toLocationType" );
		toLocationType->LinkEndChild( new TiXmlText(out.str()));
		Subtrip->LinkEndChild( toLocationType );
		///////////actual subtrip part//////////////////
		//mode
		TiXmlElement * mode  = new TiXmlElement( "mode" );
		mode->LinkEndChild( new TiXmlText(it->mode));
		Subtrip->LinkEndChild( mode );
		//isPrimaryMode
		TiXmlElement * isPrimaryMode  = new TiXmlElement( "isPrimaryMode" );
		isPrimaryMode->LinkEndChild( new TiXmlText(it->isPrimaryMode ? "true" : "false"));
		Subtrip->LinkEndChild( isPrimaryMode );
		//ptLineId
		TiXmlElement * ptLineId  = new TiXmlElement( "ptLineId" );
		ptLineId->LinkEndChild( new TiXmlText(it->ptLineId));
		Subtrip->LinkEndChild( ptLineId );
	}
}

std::string sim_mob::locationType_toString(TripChainItem::LocationType type)
{
	switch(type)
	{
	case TripChainItem::LT_BUILDING:
		return "LT_BUILDING";
	case TripChainItem::LT_NODE:
		return "LT_NODE";
	case TripChainItem::LT_LINK:
		return "LT_PUBLIC_TRANSIT_STOP";
	default:
		return "";
	}
}
void sim_mob::WriteXMLInput_TripChain_Trip(TiXmlElement * TripChains, sim_mob::Trip & trip)
{
	std::ostringstream out;
	TiXmlElement * Trip;
	Trip = new TiXmlElement( "Trip" );
	TripChains->LinkEndChild( Trip );
	WriteXMLInput_TripChainItem(Trip,trip);
	//tripID
	out.str("");
	out << trip.tripID;
	TiXmlElement * tripID  = new TiXmlElement( "tripID" );
	tripID->LinkEndChild( new TiXmlText(out.str()));
	Trip->LinkEndChild( tripID );
	//fromLocation
	TiXmlElement * fromLocation  = new TiXmlElement( "fromLocation" );
	out.str("");
	out << trip.fromLocation->getID();
	fromLocation->LinkEndChild( new TiXmlText(out.str()));
//	WriteXMLInput_Location(fromLocation,false,trip.fromLocation->getLocation().getX(),trip.fromLocation->getLocation().getY());
	Trip->LinkEndChild( fromLocation );
	//fromLocationType
//	out.str("");
//	out << locationType_toString(trip.fromLocationType);
	TiXmlElement * fromLocationType  = new TiXmlElement( "fromLocationType" );
	fromLocationType->LinkEndChild( new TiXmlText(locationType_toString(trip.fromLocationType)));
	Trip->LinkEndChild( fromLocationType );
	//toLocation
	TiXmlElement * toLocation  = new TiXmlElement( "toLocation" );
	out.str("");
	out << trip.toLocation->getID();
	toLocation->LinkEndChild( new TiXmlText(out.str()));
//	WriteXMLInput_Location(toLocation,false,trip.toLocation->getLocation().getX(),trip.toLocation->getLocation().getY());
	Trip->LinkEndChild( toLocation );
	//toLocationType
//	out.str("");
//	out << trip.toLocationType;
	TiXmlElement * toLocationType  = new TiXmlElement( "toLocationType" );
	toLocationType->LinkEndChild( new TiXmlText(locationType_toString(trip.toLocationType)));
	Trip->LinkEndChild( toLocationType );
	if(trip.getSubTrips().size() > 0)
		WriteXMLInput_TripChain_Subtrips(Trip,trip);

}

void sim_mob::WriteXMLInput_TripChain_Activity(TiXmlElement * TripChains, sim_mob::Activity & activity)
{
	std::ostringstream out;
	TiXmlElement * Activity;
	Activity = new TiXmlElement( "Activity" );
	TripChains->LinkEndChild( Activity );
	WriteXMLInput_TripChainItem(Activity,activity);
	//description
	TiXmlElement * description  = new TiXmlElement( "description" );
	description->LinkEndChild( new TiXmlText(activity.description));
	Activity->LinkEndChild( description );
	//location
	TiXmlElement * location  = new TiXmlElement( "location" );
	out.str("");
	out << activity.location->getID();
	location->LinkEndChild( new TiXmlText(out.str()));
	Activity->LinkEndChild( location );
	//locationType
//	out.str("");
//	out << activity.locationType;
	TiXmlElement * locationType  = new TiXmlElement( "locationType" );
	locationType->LinkEndChild( new TiXmlText(locationType_toString(activity.locationType)));
	Activity->LinkEndChild( locationType );
	//isPrimary
	TiXmlElement * isPrimary  = new TiXmlElement( "isPrimary" );
	isPrimary->LinkEndChild( new TiXmlText(activity.isPrimary ? "true" : "false"));
	Activity->LinkEndChild( isPrimary );
	//isFlexible
	TiXmlElement * isFlexible  = new TiXmlElement( "isFlexible" );
	isFlexible->LinkEndChild( new TiXmlText(activity.isFlexible ? "true" : "false"));
	Activity->LinkEndChild( isFlexible );
	//isMandatory
	TiXmlElement * isMandatory  = new TiXmlElement( "isMandatory" );
	isMandatory->LinkEndChild( new TiXmlText(activity.isMandatory ? "true" : "false"));
	Activity->LinkEndChild( isMandatory );
}
void sim_mob::WriteXMLInput_TripChains(TiXmlElement * SimMobility)
{
	std::ostringstream out;
	ConfigParams& config = ConfigParams::GetInstance();

	std::map<std::string, std::vector<sim_mob::TripChainItem*> >& TripChainsObj = config.getTripChains();
	if(TripChainsObj.size() < 1) return;

	TiXmlElement * TripChains;
	TripChains = new TiXmlElement( "TripChains" );
	SimMobility->LinkEndChild( TripChains );

	for(std::map<std::string, std::vector<sim_mob::TripChainItem*> >::iterator it_map = TripChainsObj.begin(), it_end(TripChainsObj.end()); it_map != it_end ; it_map ++)
	{
		//tripchain
		TiXmlElement * TripChain;
		TripChain = new TiXmlElement( "TripChain" );
		TripChains->LinkEndChild( TripChain );

		//personID sub-element
		TiXmlElement * personID;
		personID = new TiXmlElement( "personID" );
		out.str("");
		out << it_map->first; //personID
		personID->LinkEndChild(new TiXmlText(out.str()));
		TripChain->LinkEndChild( personID );

		for(std::vector<sim_mob::TripChainItem*>::iterator it = it_map->second.begin() , it_end(it_map->second.end()); it != it_end; it++)
		{
			sim_mob::TripChainItem* tripChainItem = *it;
			if(dynamic_cast<sim_mob::Activity *>(tripChainItem))
				WriteXMLInput_TripChain_Activity(TripChain,*dynamic_cast<sim_mob::Activity *>(tripChainItem));
			else
				if(dynamic_cast<sim_mob::Trip *>(tripChainItem))
					WriteXMLInput_TripChain_Trip(TripChain,*dynamic_cast<sim_mob::Trip *>(tripChainItem));
		}
	}

}

void sim_mob::WriteXMLInput_TrafficSignal_LinkAndCrossings(TiXmlElement * linkAndCrossings,const LinkAndCrossingByLink & LAC)
{
	std::ostringstream out;
	//linkAndCrossings
	for(LinkAndCrossingByLink::iterator it = LAC.begin(); it != LAC.end(); it++)
	{
		//linkAndCrossing
		TiXmlElement * linkAndCrossing  = new TiXmlElement( "linkAndCrossing" );
		linkAndCrossings->LinkEndChild( linkAndCrossing);
		{
			//linkID
			TiXmlElement * linkID  = new TiXmlElement( "linkID" );
			out.str("");
			out << it->link->getLinkId();
			linkID->LinkEndChild( new TiXmlText(out.str()));
			linkAndCrossing->LinkEndChild(linkID);
			//crossingID
			TiXmlElement * crossingID  = new TiXmlElement( "crossingID" );
			out.str("");
			out << it->crossing->getCrossingID();
			crossingID->LinkEndChild( new TiXmlText(out.str()));
			linkAndCrossing->LinkEndChild(crossingID);
		}
	}
}
void sim_mob::WriteXMLInput_TrafficSignal_ColorSequence(TiXmlElement * parent, sim_mob::ColorSequence &colorSequence)
{
	std::ostringstream out;
	//ColorSequence
	TiXmlElement * ColorSequence  = new TiXmlElement( "ColorSequence" );
	parent->LinkEndChild( ColorSequence);

	//TrafficLightType
	TiXmlElement * TrafficLightType  = new TiXmlElement( "TrafficLightType" );
	ColorSequence->LinkEndChild( TrafficLightType);
	out.str("");
	out << (colorSequence.getTrafficLightType() == sim_mob::Driver_Light ? "Driver_Light" : (colorSequence.getTrafficLightType() == sim_mob::Pedestrian_Light ? "Pedestrian_Light": "Unknown Traffic Light type"));
	TrafficLightType->LinkEndChild(new TiXmlText(out.str()));

	std::vector< std::pair<TrafficColor,std::size_t> > & cd = colorSequence.getColorDuration();
	for(std::vector< std::pair<TrafficColor,std::size_t> >::iterator it_cd = cd.begin(); it_cd != cd.end(); it_cd++)
	{
		//ColorDuration
		TiXmlElement * ColorDuration  = new TiXmlElement( "ColorDuration" );
		ColorSequence->LinkEndChild( ColorDuration);
		{
			//TrafficColor
			TiXmlElement * TrafficColor__  = new TiXmlElement( "TrafficColor" );
			ColorDuration->LinkEndChild( TrafficColor__);
			out.str("");
			switch(it_cd->first)
			{
			case Red:
				out << "Red";
				break;
			case Green:
				out << "Green";
				break;
			case Amber:
				out << "Amber";
				break;
			case FlashingRed:
				out << "FlashingRed";
				break;
			case FlashingGreen:
				out << "FlashingGreen";
				break;
			case FlashingAmber:
				out << "FlashingAmber";
				break;
			}
			TrafficColor__->LinkEndChild(new  TiXmlText(out.str()));

			//Duration
			TiXmlElement * Duration  = new TiXmlElement( "Duration" );
			ColorDuration->LinkEndChild( Duration);
			out.str("");
			out << it_cd->second;
			Duration->LinkEndChild(new TiXmlText(out.str()));
		}
	}

}
void sim_mob::WriteXMLInput_TrafficSignal_Phases(TiXmlElement * phases,  std::vector<sim_mob::Phase> &phases_)
{
	std::ostringstream out;
	for(sim_mob::Signal::phases::const_iterator it_phases = phases_.begin(); it_phases != phases_.end(); it_phases++)
	{
		//phase
		TiXmlElement * phase  = new TiXmlElement( "phase" );
		phases->LinkEndChild( phase);
		{
//			//phaseID
//			TiXmlElement * phaseID  = new TiXmlElement( "phaseID" );
//			phase->LinkEndChild( phaseID);//to be added later
//			out.str("");
//			out << it_phases->phaseID;
//			phase->LinkEndChild(new TiXmlText(out.str()));
			//name
			TiXmlElement * name  = new TiXmlElement( "name" );
			phase->LinkEndChild( name);
			out.str("");
			out << it_phases->getName();
			name->LinkEndChild(new TiXmlText(out.str()));
			//links_maps
			TiXmlElement * links_maps  = new TiXmlElement( "links_maps" );
			phase->LinkEndChild( links_maps);
			const sim_mob::links_map & lms_ = it_phases->getLinkMaps();
			for(sim_mob::links_map::const_iterator it_lm = lms_.begin(); it_lm != lms_.end(); it_lm++)
			{

				sim_mob::Link * linkFrom_ = it_lm->first;
				sim_mob::linkToLink lm_= it_lm->second;
				sim_mob::Link *linkTo_ = lm_.LinkTo;
				sim_mob::RoadSegment *RS_From_ = lm_.RS_From;
				sim_mob::RoadSegment *RS_To_ = lm_.RS_To;
				//links_map
				TiXmlElement * links_map  = new TiXmlElement( "links_map" );
				links_maps->LinkEndChild( links_map);

				//linkFrom
				TiXmlElement * linkFrom  = new TiXmlElement( "linkFrom" );
				links_map->LinkEndChild( linkFrom);
				out.str("");
				out << linkFrom_->getLinkId();
				linkFrom->LinkEndChild(new TiXmlText(out.str()));
				//linkTo
				TiXmlElement * linkTo  = new TiXmlElement( "linkTo" );
				links_map->LinkEndChild( linkTo);
				out.str("");
				out << linkTo_->getLinkId();
				linkTo->LinkEndChild(new TiXmlText(out.str()));
				//SegmentFrom
				TiXmlElement * SegmentFrom  = new TiXmlElement( "SegmentFrom" );
				links_map->LinkEndChild( SegmentFrom);
				out.str("");
				out << RS_From_->getSegmentID();
				SegmentFrom->LinkEndChild(new TiXmlText(out.str()));
				//SegmentTo
				TiXmlElement * SegmentTo  = new TiXmlElement( "SegmentTo" );
				links_map->LinkEndChild( SegmentTo);
				out.str("");
				out << RS_To_->getSegmentID();
				SegmentTo->LinkEndChild(new TiXmlText(out.str()));
				//ColorSequence
				WriteXMLInput_TrafficSignal_ColorSequence(links_map,lm_.colorSequence);

			}
			//crossings_maps
			TiXmlElement * crossings_maps  = new TiXmlElement( "crossings_maps" );
			phase->LinkEndChild( crossings_maps);
			const sim_mob::crossings_map & cms_ = it_phases->getCrossingMaps();
			for(sim_mob::crossings_map::const_iterator it_cm = cms_.begin(); it_cm != cms_.end(); it_cm++)
			{
				//crossings_map
				TiXmlElement * crossings_map  = new TiXmlElement( "crossings_map" );
				crossings_maps->LinkEndChild(crossings_map);
				//link
				out.str("");
				TiXmlElement * linkID  = new TiXmlElement( "linkID" );
				crossings_map->LinkEndChild(linkID);
				out << it_cm ->second.link->getLinkId();
				linkID->LinkEndChild(new TiXmlText(out.str()));
				//crossing
				out.str("");
				TiXmlElement * crossingID  = new TiXmlElement( "crossingID" );
				crossings_map->LinkEndChild(crossingID);
				out << it_cm ->second.crossig->getCrossingID();
				crossingID->LinkEndChild(new TiXmlText(out.str()));
				sim_mob::ColorSequence & colorSequence = const_cast<sim_mob::ColorSequence &>(it_cm->second.colorSequence);
				//ColorSequence
				WriteXMLInput_TrafficSignal_ColorSequence(crossings_map,colorSequence);
			}

		}
	}
}

void sim_mob::WriteXMLInput_TrafficSignal_common(TiXmlElement * Signal,sim_mob::Signal *signal_)
{
	std::ostringstream out;
	//signalID
	TiXmlElement * signalID  = new TiXmlElement( "signalID" );
	out << signal_->getSignalId();
	signalID->LinkEndChild( new TiXmlText(out.str()));
	Signal->LinkEndChild(signalID);
	//nodeID
	TiXmlElement * nodeID  = new TiXmlElement( "nodeID" );
	out.str("");
	out << signal_->getNode().getID();
	nodeID->LinkEndChild( new TiXmlText(out.str()));
	Signal->LinkEndChild(nodeID);

	//linkAndCrossings
	TiXmlElement * linkAndCrossings  = new TiXmlElement( "linkAndCrossings" );
	Signal->LinkEndChild( linkAndCrossings);
	const LinkAndCrossingByLink & LAC = signal_->getLinkAndCrossingsByLink();
	WriteXMLInput_TrafficSignal_LinkAndCrossings(linkAndCrossings,LAC);

	//phases
	TiXmlElement * phases  = new TiXmlElement( "phases" );
	Signal->LinkEndChild( phases);
	/*const std::vector<sim_mob::Phase>*/ sim_mob::Signal::phases & phases_ = signal_->getPhases();
	WriteXMLInput_TrafficSignal_Phases(phases,phases_);
}
void sim_mob::WriteXMLInput_TrafficSignal_SCATS_SplitPlan(TiXmlElement * SCATS,sim_mob::Signal_SCATS *signal_)
{
	std::ostringstream out;
	sim_mob::SplitPlan &plan = signal_->getPlan();
	//SplitPlan
	TiXmlElement * SplitPlan  = new TiXmlElement( "SplitPlan" );
	SCATS->LinkEndChild(SplitPlan);
	//splitplanID
	TiXmlElement * splitplanID  = new TiXmlElement( "splitplanID" );
	SplitPlan->LinkEndChild(splitplanID);
	out.str("");
	out << plan.TMP_PlanID;
	splitplanID->LinkEndChild(new TiXmlText(out.str()));
	//cycleLength
	TiXmlElement * cycleLength  = new TiXmlElement( "cycleLength" );
	SplitPlan->LinkEndChild(cycleLength);
	out.str("");
	out << plan.getCycleLength();
	cycleLength->LinkEndChild(new TiXmlText(out.str()));
	//offset
	TiXmlElement * offset  = new TiXmlElement( "offset" );
	SplitPlan->LinkEndChild(offset);
	out.str("");
	out << plan.getOffset();
	offset->LinkEndChild(new TiXmlText(out.str()));

	//ChoiceSet
	TiXmlElement * ChoiceSet  = new TiXmlElement( "ChoiceSet" );
	SplitPlan->LinkEndChild(ChoiceSet);
	int i = 0;
	for( std::vector< std::vector<double> >::iterator it_pl = plan.getChoiceSet().begin(), it_end(plan.getChoiceSet().end()); it_pl != it_end; it_pl++, i++)
	{
		//plan
		TiXmlElement * plan  = new TiXmlElement( "plan" );
		ChoiceSet->LinkEndChild(plan);

		//planID
		TiXmlElement * planID  = new TiXmlElement( "planID" );
		plan->LinkEndChild(planID);
		out.str("");
		out << i;
		planID->LinkEndChild(new TiXmlText(out.str()));
		for(std::vector<double>::iterator it_ph = it_pl->begin(), it_end(it_pl->end()); it_ph != it_end; it_ph++)//Choke on it :)
		{
			//PhasePercentage
			TiXmlElement * PhasePercentage  = new TiXmlElement( "PhasePercentage" );
			plan->LinkEndChild(PhasePercentage);
			out.str("");
			out << *it_ph;
			PhasePercentage->LinkEndChild(new TiXmlText(out.str()));
		}
	}
}
void sim_mob::WriteXMLInput_TrafficSignal_SCATS(TiXmlElement * Signal,sim_mob::Signal *signal_)
{
	//the common part
	WriteXMLInput_TrafficSignal_common(Signal, signal_);
	//SCATS specific part

	//SCATS
	sim_mob::Signal_SCATS* signal_scats = dynamic_cast<sim_mob::Signal_SCATS*>(signal_);
	TiXmlElement * SCATS  = new TiXmlElement( "SCATS" );
	Signal->LinkEndChild( SCATS);
	//	signalTimingMode
	TiXmlElement * signalTimingMode = new TiXmlElement("signalTimingMode");
	if (signal_scats->getSignalTimingMode() == 0) {
		signalTimingMode->LinkEndChild(new TiXmlText("STM_FIXED"));
	} else if (signal_scats->getSignalTimingMode() == 1) {
		signalTimingMode->LinkEndChild(new TiXmlText("STM_ADAPTIVE"));
	} else {
		std::cout << "signalTimingMode ( " << signal_scats->getSignalTimingMode()
				<< ") unknown, press any key ...\n";
	}
	SCATS->LinkEndChild(signalTimingMode);
	//splitplan
	WriteXMLInput_TrafficSignal_SCATS_SplitPlan(SCATS, signal_scats);


}

void sim_mob::WriteXMLInput_TrafficSignal(TiXmlElement * Signals,sim_mob::Signal *signal_)
{

	TiXmlElement * Signal;
	Signal = new TiXmlElement( "Signal" );
	Signals->LinkEndChild( Signal );
	//if scats:
	WriteXMLInput_TrafficSignal_SCATS(Signal,signal_);
	//else
	//....
}

void sim_mob::WriteXMLInput_TrafficSignals(TiXmlElement * SimMobility)
{
	TiXmlElement * Signals;
	Signals = new TiXmlElement( "Signals" );
	SimMobility->LinkEndChild( Signals );

	sim_mob::Signal::All_Signals &Signals_ = sim_mob::Signal::all_signals_;
	for(sim_mob::Signal::all_signals_const_Iterator it = Signals_.begin(); it != Signals_.end(); it++)
	{
		WriteXMLInput_TrafficSignal(Signals, (*it));
	}
}
void sim_mob::WriteXMLInput(const std::string& XML_OutPutFileName)
{

	std::cout <<"In WriteXMLInput" << std::endl;
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "utf-8", "");
	doc.LinkEndChild( decl );
	TiXmlElement * SimMobility;
	SimMobility = new TiXmlElement( "geo:SimMobility" );
	SimMobility->SetAttribute("xmlns:geo" , "http://www.smart.mit.edu/geo");
	SimMobility->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	SimMobility->SetAttribute("xsi:schemaLocation", "http://www.smart.mit.edu/geo file:/home/vahid/Desktop/geo8/geo10.xsd");
    doc.LinkEndChild( SimMobility );

	WriteXMLInput_GeoSpatial(SimMobility);
	WriteXMLInput_TripChains(SimMobility);
	WriteXMLInput_TrafficSignals(SimMobility);
    doc.SaveFile( XML_OutPutFileName );
}
