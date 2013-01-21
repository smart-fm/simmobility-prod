/* Copyright Singapore-MIT Alliance for Research and Technology */

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

#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>

#include "util/XmlWriter.hpp"

#include "geospatial/RoadNetwork.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/Intersection.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Point2D.hpp"


namespace sim_mob {
namespace xml {

/////////////////////////////////////////////////////////////////////
// get_id()
/////////////////////////////////////////////////////////////////////

template <>
std::string get_id(const sim_mob::RoadSegment& rs)
{
	return boost::lexical_cast<std::string>(rs.getSegmentID());
}

template <>
std::string get_id(const sim_mob::Lane& ln)
{
	return boost::lexical_cast<std::string>(ln.getLaneID());
}


//TODO: These are not used, but need to be available to the template parser. They are harmless; it'll
//      be obvious if "id" types are used instead of "value" types accidentally.
template <>
std::string get_id(const sim_mob::Link& lnk)
{
	return boost::lexical_cast<std::string>(lnk.getLinkId());
}
template <>
std::string get_id(const sim_mob::MultiNode& mnd)
{
	return boost::lexical_cast<std::string>(mnd.getID());
}
template <>
std::string get_id(const sim_mob::UniNode& und)
{
	return boost::lexical_cast<std::string>(und.getID());
}
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
// write_xml()
/////////////////////////////////////////////////////////////////////

template <>
void write_xml(XmlWriter& write, const sim_mob::Link& lnk)
{
	//TEMP
    write.prop("TODO", 2);
}


template <>
void write_xml(XmlWriter& write, const sim_mob::RoadSegment& rs)
{
	//TEMP
    write.prop("TODO", 2);
}

template <>
void write_xml(XmlWriter& write, const sim_mob::LaneConnector& lc)
{
	//TEMP
    write.prop("TODO", 2);
}

template <>
void write_xml(XmlWriter& write, const sim_mob::Lane& lc)
{
	//TEMP
    write.prop("TODO", 2);
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
	write.prop("nodeID", in.nodeId);
	write.prop("location", in.location);
	write.prop("originalDB_ID", in.originalDB_ID.getLogItem());
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
	write.prop("nodeID", und.nodeId);
	write.prop("location", und.location);
	write.prop("originalDB_ID", und.originalDB_ID.getLogItem());
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
