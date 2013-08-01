//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/**
 * \file Serialize.hpp

 * This file contains a set of serialization functions (boost::serialize) for the Geospatial road
 *   network classes which are compatible with boost::xml.
 * Be careful when including this file, as it has a large number of dependencies coupled with template
 *   expansion.
 *
 * \note
 * In terms of justification for why we put all of these functions outside of their respective classes,
 * there are three. First, I don't want tiny classes like Point2D and Node having boost::xml as a
 * dependency. Second, no "friends" are needed if the proper accessors are provided. Third, a friend
 * class would have been needed anyway in boost::serialization::access, so we might as well remove
 * these functions. The downside is that changes to the RoadNetwork classes will force Serialize.hpp to
 * recompile, but that is mostly unavoidable (and the classes are mostly stable now anyway). If performance
 * really becomes a problem, we can force the template into a separate cpp file (although it's messy).
 *
 * \note
 * These serialization functions are used for *writing* only; they are untested and unintended for reading.
 *
 * \author Seth N. Hetu
 */


#pragma once

#include <set>
#include <vector>
#include <iostream>
#include <stdexcept>


#include "util/XmlWriter.hpp"
#include "metrics/Length.hpp"

#include "geospatial/RoadNetwork.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/RoadItem.hpp"
#include "geospatial/Intersection.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/RoadSegment.hpp"


namespace sim_mob {
namespace xml {

/////////////////////////////////////////////////////////////////////
// get_id()
/////////////////////////////////////////////////////////////////////

//Disallow get_id for Point2D
ERASE_GET_ID(sim_mob::Point2D);

//Simple versions of get_id for most classes.
SPECIALIZE_GET_ID(sim_mob::RoadSegment, getSegmentID);
SPECIALIZE_GET_ID(sim_mob::Lane,        getLaneID);
SPECIALIZE_GET_ID(sim_mob::Link,        getLinkId);
SPECIALIZE_GET_ID(sim_mob::MultiNode,   getID);
SPECIALIZE_GET_ID(sim_mob::UniNode,     getID);
SPECIALIZE_GET_ID(sim_mob::Node,        getID);
SPECIALIZE_GET_ID(sim_mob::RoadItem,    getRoadItemID);

//get_id for Lane Connectors is more complicated than our macro can handle.
template <>
std::string get_id(const sim_mob::LaneConnector& lc)
{
	return boost::lexical_cast<std::string>(lc.getLaneFrom()->getLaneID()) + ":" + boost::lexical_cast<std::string>(lc.getLaneTo()->getLaneID());
}




/////////////////////////////////////////////////////////////////////
// write_xml() - Dispatch
//               Provide a sensible default for pairs of lane connectors (laneFrom/laneTo by ID)
/////////////////////////////////////////////////////////////////////
template <>
void write_xml(XmlWriter& write, const std::pair<const sim_mob::Lane*, const sim_mob::Lane* >& connectors)
{
	write_xml(write, connectors, namer("<laneFrom,laneTo>"), expander("<id,id>"));
}
template <>
void write_xml(XmlWriter& write, const std::pair<const sim_mob::Lane*, sim_mob::Lane* >& connectors)
{
	write_xml(write, std::pair<const sim_mob::Lane*, const sim_mob::Lane*>(connectors.first,connectors.second));
}
template <>
void write_xml(XmlWriter& write, const std::pair<sim_mob::Lane*, const sim_mob::Lane* >& connectors)
{
	write_xml(write, std::pair<const sim_mob::Lane*, const sim_mob::Lane*>(connectors.first,connectors.second));
}
template <>
void write_xml(XmlWriter& write, const std::pair<sim_mob::Lane*, sim_mob::Lane* >& connectors)
{
	write_xml(write, std::pair<const sim_mob::Lane*, const sim_mob::Lane*>(connectors.first,connectors.second));
}


/////////////////////////////////////////////////////////////////////
// write_xml() - Dispatch
//               Treat vectors of Point2Ds as poylines.
/////////////////////////////////////////////////////////////////////
template <>
void write_xml(XmlWriter& write, const std::vector<sim_mob::Point2D>& poly)
{
	int i=0;
	for (std::vector<sim_mob::Point2D>::const_iterator it=poly.begin(); it!=poly.end(); it++) {
		write.prop_begin("PolyPoint");
		write.prop("pointID", i++);
		write.prop("location", *it);
		write.prop_end();
	}
}


/////////////////////////////////////////////////////////////////////
// WORKAROUND: Currently, the new syntax can't handle certain STL combinations
//             with enough detail, so we have to provide this fallback here.
/////////////////////////////////////////////////////////////////////
template <>
void write_xml(XmlWriter& write, const std::vector< std::pair<const sim_mob::Lane*, sim_mob::Lane* > > & connectors)
{
	write_xml(write, connectors, namer("<Connector>"));
}


/////////////////////////////////////////////////////////////////////
// write_xml()
/////////////////////////////////////////////////////////////////////

template <>
void write_xml(XmlWriter& write, const sim_mob::Link& lnk)
{
	write.prop("linkID", lnk.getLinkId());
	write.prop("roadName", lnk.roadName);

	//TODO: This is a workaround at the moment. In reality, "<id>" should do all the work, but since
	//      we can't currently handle IDs of value types, the expander is actually ignored!
	write.prop("StartingNode", lnk.getStart(), namer(), expander("<id>"), false);
	write.prop("EndingNode", lnk.getEnd(), namer(), expander("<id>"), false);

	write.prop("Segments", lnk.getSegments(), namer("<Segment>"));
}


//Another workaround needed for lane polylines.
namespace {
std::vector< std::pair<int, std::vector<Point2D> > > wrap_lanes(const std::vector< std::vector<Point2D> >& lanes)
{
	int i=0;
	std::vector< std::pair<int, std::vector<Point2D> > > res;
	for (std::vector< std::vector<Point2D> >::const_iterator it=lanes.begin(); it!=lanes.end(); it++) {
		res.push_back(std::make_pair(i++, *it));
	}
	return res;
}
} //End anon namespace


template <>
void write_xml(XmlWriter& write, const sim_mob::RoadSegment& rs)
{
	write.prop("segmentID", rs.getSegmentID());

	//TODO: Similar workaround
	write.prop("startingNode", rs.getStart(), namer(), expander("<id>"), false);
	write.prop("endingNode", rs.getEnd(), namer(), expander("<id>"), false);

	write.prop("maxSpeed", rs.maxSpeed);
	write.prop("Length", rs.length);
	write.prop("Width", rs.width);
	write.prop("originalDB_ID", rs.originalDB_ID.getLogItem());

	//NOTE: We don't pass a namer in here, since vectors<> of Point2Ds are a special case.
	write.prop("polyline", rs.polyline);

	std::vector< std::vector<Point2D> > laneLines;
	for (size_t i=0; i<=rs.getLanes().size(); i++) {
		laneLines.push_back(const_cast<sim_mob::RoadSegment&>(rs).getLaneEdgePolyline(i));
	}
    write.prop("laneEdgePolylines_cached", wrap_lanes(laneLines), namer("<laneEdgePolyline_cached,<laneNumber,polyline>>"));

	write.prop("Lanes", rs.getLanes(), namer("<Lane>"));
	write.prop("Obstacles", rs.getObstacles());

}

template <>
void write_xml(XmlWriter& write, const sim_mob::Lane& ln)
{
	write.prop("laneID", ln.getLaneID());
	write.prop("width", ln.getWidth());
	write.prop("can_go_straight", ln.can_go_straight());
	write.prop("can_turn_left", ln.can_turn_left());
	write.prop("can_turn_right", ln.can_turn_right());
	write.prop("can_turn_on_red_signal", ln.can_turn_on_red_signal());
	write.prop("can_change_lane_left", ln.can_change_lane_left());
	write.prop("can_change_lane_right", ln.can_change_lane_right());
	write.prop("is_road_shoulder", ln.is_road_shoulder());
	write.prop("is_bicycle_lane", ln.is_bicycle_lane());
	write.prop("is_pedestrian_lane", ln.is_pedestrian_lane());
	write.prop("is_vehicle_lane", ln.is_vehicle_lane());
	write.prop("is_standard_bus_lane", ln.is_standard_bus_lane());
	write.prop("is_whole_day_bus_lane", ln.is_whole_day_bus_lane());
	write.prop("is_high_occupancy_vehicle_lane", ln.is_high_occupancy_vehicle_lane());
	write.prop("can_freely_park_here", ln.can_freely_park_here());
	write.prop("can_stop_here", ln.can_stop_here());
	write.prop("is_u_turn_allowed", ln.is_u_turn_allowed());
}


template <>
void write_xml(XmlWriter& write, const sim_mob::Node& nd)
{
	write.prop("nodeID", nd.nodeId);
	write.prop("location", nd.location);
	write.prop("originalDB_ID", nd.originalDB_ID.getLogItem());
}

template <>
void write_xml(XmlWriter& write, const sim_mob::RoadItem& ri)
{
	throw std::runtime_error("RoadItems by themselves can't be serialized; try putting them in an Obstacle map.");
}


//NOTE: This is another workaround for dealing with heterogeneous types.
//NOTE: It also deals with out-of-order properties.
template <>
void write_xml(XmlWriter& write, const std::map<sim_mob::centimeter_t, const sim_mob::RoadItem*>& obstacles, namer name, expander expand)
{
	for (std::map<centimeter_t, const RoadItem*>::const_iterator it=obstacles.begin(); it!=obstacles.end(); it++) {
		if (dynamic_cast<const Crossing*>(it->second)) {
			const Crossing* cr = dynamic_cast<const Crossing*>(it->second);
			write.prop_begin("Crossing");
			write.prop("id", cr->getRoadItemID());
			write.prop("Offset", it->first);
			write.prop("start", cr->getStart());
			write.prop("end", cr->getEnd());
			write.prop("nearLine", cr->nearLine);
			write.prop("farLine", cr->farLine);
			write.prop_end();
		} else if (dynamic_cast<const BusStop*>(it->second)) {
			const BusStop* bs = dynamic_cast<const BusStop*>(it->second);
			write.prop_begin("BusStop");
			write.prop("id", bs->getRoadItemID());
			write.prop("Offset", it->first);
			write.prop("start", bs->getStart());
			write.prop("end", bs->getEnd());
			write.prop("xPos", bs->xPos);
			write.prop("yPos", bs->yPos);
			write.prop("is_terminal", bs->is_terminal);
			write.prop("is_bay", bs->is_bay);
			write.prop("has_shelter", bs->has_shelter);
			write.prop("busCapacityAsLength", bs->busCapacityAsLength);
			write.prop("busstopno", bs->getBusstopno_());
			write.prop_end();
		} else {
			throw std::runtime_error("Unidentified RoadItem subclass.");
		}
	}
}
template <>
void write_xml(XmlWriter& write, const std::map<sim_mob::centimeter_t, const sim_mob::RoadItem*>& obstacles, namer name)
{
	write_xml(write, obstacles, name, expander());
}
template <>
void write_xml(XmlWriter& write, const std::map<sim_mob::centimeter_t, const sim_mob::RoadItem*>& obstacles, expander expand)
{
	write_xml(write, obstacles, namer("<item,<key,value>>"), expand);
}
template <>
void write_xml(XmlWriter& write, const std::map<sim_mob::centimeter_t, const sim_mob::RoadItem*>& obstacles)
{
	write_xml(write, obstacles, namer("<item,<key,value>>"), expander());
}




template <>
void write_xml(XmlWriter& write, const sim_mob::Point2D& pt)
{
	write.prop("xPos", pt.getX());
	write.prop("yPos", pt.getY());
}


//Workaround:
//Out multi-node connectors are not actually represented in the format we'd expect.
namespace {
std::map<const sim_mob::RoadSegment*, std::vector< std::pair<const sim_mob::Lane*, sim_mob::Lane*> > > warp_multi_connectors(const std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> > & connectors)
{
	std::map<const sim_mob::RoadSegment*, std::vector< std::pair<const sim_mob::Lane*, sim_mob::Lane*> > > res;
	for (std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> >::const_iterator it=connectors.begin(); it!=connectors.end(); it++) {
		for (std::set<sim_mob::LaneConnector*>::const_iterator lcIt=it->second.begin(); lcIt!=it->second.end(); lcIt++) {
			res[it->first].push_back(std::make_pair((*lcIt)->getLaneFrom(), const_cast<sim_mob::Lane*>((*lcIt)->getLaneTo())));
		}
	}
	return res;
}
} //End un-named namespace

template <>
void write_xml(XmlWriter& write, const sim_mob::Intersection& in)
{
	write_xml(write, dynamic_cast<const sim_mob::Node&>(in));
	write.prop("roadSegmentsAt", in.getRoadSegments(), namer("<segmentID>"), expander("<id>"));
	write.prop("Connectors", warp_multi_connectors(in.getConnectors()), namer("<MultiConnectors,<RoadSegment,Connectors>>"), expander("<*,<id,*>>"));

}

template <>
void write_xml(XmlWriter& write, const sim_mob::MultiNode& mnd)
{
	//Try to dispatch to intersection.
	//TODO: This will fail when our list of Nodes also contains roundabouts.
	write_xml(write, dynamic_cast<const sim_mob::Intersection&>(mnd));
}

template <>
void write_xml(XmlWriter& write, const sim_mob::UniNode& und)
{
	write_xml(write, dynamic_cast<const sim_mob::Node&>(und));
	write.prop("firstPair", und.firstPair, expander("<id,id>"));
	if (und.secondPair.first && und.secondPair.second) {
		write.prop("secondPair", und.secondPair, expander("<id,id>"));
	}
	write.prop("Connectors", und.getConnectors(), namer("<Connector,<laneFrom,laneTo>>"), expander("<*,<id,id>>"));
}

template <>
void write_xml(XmlWriter& write, const sim_mob::RoadNetwork& rn)
{
	//Start writing
    write.prop_begin("RoadNetwork");

    //Nodes are also wrapped
    write.prop_begin("Nodes");
    write.prop("UniNodes", rn.getUniNodes(), namer("<UniNode>"));

    //TODO: This will fail unless getNodes() returns ONLY intersections.
    write.prop("Intersections", rn.getNodes(), namer("<Intersection>"));
    write.prop_end(); //Nodes

    write.prop("Links", rn.getLinks(), namer("<Link>"));
	write.prop_end(); //RoadNetwork
}



}}
