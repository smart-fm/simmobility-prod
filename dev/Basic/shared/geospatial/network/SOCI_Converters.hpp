//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <map>
#include <soci/soci.h>
#include <string>

#include "entities/misc/PublicTransit.hpp"
#include "Lane.hpp"
#include "LaneConnector.hpp"
#include "Link.hpp"
#include "Node.hpp"
#include "ParkingSlot.hpp"
#include "Point.hpp"
#include "PT_Stop.hpp"
#include "RoadItem.hpp"
#include "TurningGroup.hpp"
#include "TurningPath.hpp"

using namespace sim_mob;

//Anonymous name-space for helper functions
namespace
{

//Helper functions

/**
 * This method parses the string retrieved from the tag field from the database and returns the value
 * part of the string corresponding to the requested tag.
 * Note: Currently no tags are required during simulation, and as a result this method is not used.
 * @param std::string
 * @param std::string
 * @return std::string
 */
std::string ExtractTag(std::string strTags, std::string reqdTag)
{
	//To get tokens from the string
	typedef boost::tokenizer<boost::char_separator<char> > tokeniser;
	char sep[] = {'<', '>', ':', ',', ' '};
	boost::char_separator<char> separator(sep);

	tokeniser tokens(strTags, separator);
	tokeniser::iterator itTokens = tokens.begin();

	std::string key, value;
	bool tagFound = false;

	while (itTokens != tokens.end())
	{
		//The string representation is as follows:
		//"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"

		key = *itTokens;

		++itTokens;

		value = *itTokens;

		++itTokens;

		if (key == reqdTag)
		{
			tagFound = true;
			break;
		}
	}

	if (tagFound)
	{
		return value;
	}
	else
	{
		return "";
	}
}

}

namespace soci
{

template<> struct type_conversion<sim_mob::Node>
{
	typedef values base_type;

	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::Node& res)
	{
		res.setNodeId(vals.get<unsigned int>("id", 0));
		res.setNodeType((sim_mob::NodeType)vals.get<unsigned int>("node_type", 0));
		res.setTrafficLightId(vals.get<unsigned int>("traffic_light_id", 0));

		//Create and set the node location
		double x = vals.get<double>("x", 0);
		double y = vals.get<double>("y", 0);
		double z = vals.get<double>("z", 0);
		Point location(x, y, z);
		res.setLocation(location);
	}
};

template<> struct type_conversion<sim_mob::TurningGroup>
{
	typedef values base_type;

	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::TurningGroup& res)
	{
		res.setTurningGroupId(vals.get<unsigned int>("id", 0));
		res.setFromLinkId(vals.get<unsigned int>("from_link", 0));
		res.setNodeId(vals.get<unsigned int>("node_id", 0));
		res.setPhases(vals.get<std::string>("phases", ""));
		res.setRule((sim_mob::TurningGroupRule)vals.get<unsigned int>("rules", 0));
		res.setToLinkId(vals.get<unsigned int>("to_link", 0));
		res.setVisibility(vals.get<double>("visibility", 0));
	}
};

template<> struct type_conversion<sim_mob::TurningPath>
{
	typedef values base_type;

	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::TurningPath& res)
	{
		res.setTurningPathId((unsigned int) vals.get<int>("id", 0));
		res.setFromLaneId(vals.get<unsigned int>("from_lane", 0));
		res.setMaxSpeed((double)vals.get<unsigned int>("max_speed"));
		res.setToLaneId(vals.get<unsigned int>("to_lane", 0));
		res.setTurningGroupId(vals.get<unsigned int>("group_id", 0));
	}
};

template<> struct type_conversion<sim_mob::TurningConflict>
{
	typedef values base_type;

	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::TurningConflict& res)
	{
		res.setConflictId(vals.get<unsigned int>("id", 0));
		res.setCriticalGap(vals.get<double>("gap_time", 0));
		res.setFirstConflictDistance(vals.get<double>("cd1", 0));
		res.setFirstTurningId(vals.get<unsigned int>("turning_path1", 0));
		res.setPriority(vals.get<unsigned int>("priority", 0));
		res.setSecondConflictDistance(vals.get<double>("cd2", 0));
		res.setSecondTurningId(vals.get<unsigned int>("turning_path2", 0));
	}
};

template<> struct type_conversion<sim_mob::Link>
{
	typedef values base_type;

	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::Link& res)
	{
		res.setLinkId(vals.get<unsigned int>("id", 0));
		res.setFromNodeId(vals.get<unsigned int>("from_node", 0));
		res.setLinkCategory((sim_mob::LinkCategory)vals.get<unsigned int>("category", 0));
		res.setLinkType((sim_mob::LinkType)vals.get<unsigned int>("road_type", 0));
		res.setRoadName(vals.get<std::string>("road_name", ""));
		res.setToNodeId(vals.get<unsigned int>("to_node", 0));
	}
};

template<> struct type_conversion<sim_mob::RoadSegment>
{
	typedef values base_type;

	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::RoadSegment& res)
	{
		res.setRoadSegmentId(vals.get<unsigned int>("id", 0));
		res.setCapacity(vals.get<unsigned int>("capacity", 0));
		res.setLinkId(vals.get<unsigned int>("link_id", 0));
		res.setMaxSpeed((double)vals.get<unsigned int>("max_speed", 0));
		res.setSequenceNumber(vals.get<unsigned int>("sequence_num", 0));
	}
};

template<> struct type_conversion<sim_mob::Lane>
{
	typedef values base_type;

	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::Lane& res)
	{
		res.setLaneId(vals.get<unsigned int>("id", 0));
		res.setBusLaneRules((sim_mob::BusLaneRules)vals.get<unsigned int>("bus_lane", 0));
		res.setCanVehiclePark(vals.get<unsigned int>("can_park", 0));
		res.setCanVehicleStop(vals.get<unsigned int>("can_stop", 0));
		res.setHasRoadShoulder(vals.get<unsigned int>("has_road_shoulder", 0));
		res.setHighOccupancyVehicleAllowed(vals.get<unsigned int>("high_occ_veh", 0));
		res.setRoadSegmentId(vals.get<unsigned int>("segment_id", 0));
		res.setWidth(vals.get<double>("width", 0));
	}
};

template<> struct type_conversion<sim_mob::LaneConnector>
{
	typedef values base_type;

	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::LaneConnector& res)
	{
		res.setLaneConnectionId(vals.get<unsigned int>("id", 0));
		res.setFromLaneId(vals.get<unsigned int>("from_lane", 0));
		res.setFromRoadSegmentId(vals.get<unsigned int>("from_segment", 0));
		res.setToLaneId(vals.get<unsigned int>("to_lane", 0));
		res.setToRoadSegmentId(vals.get<unsigned int>("to_segment", 0));
		res.setIsTrueConnector(vals.get<unsigned int>("is_true_connector", 0));
	}
};

template<> struct type_conversion<sim_mob::PolyPoint>
{
	typedef values base_type;

	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::PolyPoint& res)
	{
		res.setPolyLineId(vals.get<unsigned int>("polyline_id", 0));
		res.setSequenceNumber(vals.get<unsigned int>("sequence_no", 0));
		res.setX(vals.get<double>("x", 0));
		res.setY(vals.get<double>("y", 0));
		res.setZ(vals.get<double>("z", 0));
	}
};

template<> struct type_conversion<sim_mob::BusStop>
{
	typedef values base_type;

	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::BusStop& res)
	{
		res.setStopId(vals.get<unsigned int>("id", 0));
		res.setRoadItemId(vals.get<unsigned int>("id", 0));
		res.setStopCode(vals.get<std::string>("code", ""));
		res.setRoadSegmentId(vals.get<unsigned int>("section_id", 0));
		res.setStopName(vals.get<std::string>("name", ""));
		res.setStopStatus(vals.get<std::string>("status", ""));
		res.setTerminusType((sim_mob::TerminusType)vals.get<int>("terminal", 0));
		res.setLength(vals.get<double>("length", 0.0));
		res.setOffset(vals.get<double>("section_offset", 0.0));
		res.setReverseSectionId(vals.get<unsigned int>("reverse_section", 0));
		res.setTerminalNodeId(vals.get<unsigned int>("terminal_node", 0));

		//Create and set the bus stop location
		double x = vals.get<double>("x", 0);
		double y = vals.get<double>("y", 0);
		double z = vals.get<double>("z", 0);
		Point location(x, y, z);
		res.setStopLocation(location);
	}
};

template<> struct type_conversion<sim_mob::ParkingSlot>
{
	typedef values base_type;

	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::ParkingSlot& res)
	{		
		res.setRoadItemId(vals.get<unsigned int>("id", 0));
		res.setRoadSegmentId(vals.get<unsigned int>("segment_id", 0));
		res.setOffset(vals.get<double>("segment_offset", 0.0));
		res.setLength(vals.get<double>("length", 0.0));
	}
};

template<>
struct type_conversion<sim_mob::PT_BusDispatchFreq>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::PT_BusDispatchFreq& ptBusDispatchFreq)
    {
    	ptBusDispatchFreq.frequencyId = values.get<std::string>("frequency_id", "");
    	boost::trim(ptBusDispatchFreq.frequencyId);
    	ptBusDispatchFreq.routeId = values.get<std::string>("route_id", "");
    	boost::trim(ptBusDispatchFreq.routeId);
    	ptBusDispatchFreq.startTime = sim_mob::DailyTime(values.get<std::string>("start_time", ""));
    	ptBusDispatchFreq.endTime = sim_mob::DailyTime(values.get<std::string>("end_time", ""));
    	ptBusDispatchFreq.headwaySec = values.get<int>("headway_sec", 0);
    }

    static void
    to_base(sim_mob::PT_BusDispatchFreq const & ptBusDispatchFreq, soci::values & values, soci::indicator & indicator)
    {
        values.set("frequency_id", ptBusDispatchFreq.frequencyId);
        values.set("route_id", ptBusDispatchFreq.routeId);
        values.set("start_time", ptBusDispatchFreq.startTime.getStrRepr());
        values.set("end_time", ptBusDispatchFreq.endTime.getStrRepr());
        values.set("headway_sec", ptBusDispatchFreq.headwaySec);
        indicator = i_ok;
    }
};

template<>
struct type_conversion<sim_mob::PT_BusRoutes>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::PT_BusRoutes& ptBusRoutes)
    {
    	ptBusRoutes.routeId = values.get<std::string>("route_id", "");
    	boost::trim(ptBusRoutes.routeId);
    	ptBusRoutes.linkId = values.get<std::string>("section_id", "");
    	boost::trim(ptBusRoutes.linkId);
    	ptBusRoutes.sequenceNo = values.get<int>("sequence_no", 0);
    }

    static void
    to_base(sim_mob::PT_BusRoutes const & ptBusRoutes, soci::values & values, soci::indicator & indicator)
    {
        values.set("route_id", ptBusRoutes.routeId);
        values.set("link_id", ptBusRoutes.linkId);
        values.set("link_sequence_no", ptBusRoutes.sequenceNo);
        indicator = i_ok;
    }
};

template<>
struct type_conversion<sim_mob::PT_BusStops>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::PT_BusStops& ptBusStops)
    {
    	ptBusStops.routeId = values.get<std::string>("route_id", "");
    	boost::trim(ptBusStops.routeId);
    	ptBusStops.stopNo = values.get<std::string>("stop_code", "");
    	boost::trim(ptBusStops.stopNo);
    	ptBusStops.sequenceNo = values.get<int>("sequence_no", 0);
    }

    static void
    to_base(sim_mob::PT_BusStops const & ptBusStops, soci::values & values, soci::indicator & indicator)
    {
        values.set("route_id", ptBusStops.routeId);
        values.set("busstop_no", ptBusStops.stopNo);
        values.set("busstop_sequence_no", ptBusStops.sequenceNo);
        indicator = i_ok;
    }
};
} //namesace soci
