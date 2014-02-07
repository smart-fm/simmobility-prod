//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

/// \file SOCI_Converters.hpp
///This file contains several Type converters for SOCI object-mapping
/// \author Seth N. Hetu
/// \author LIM Fung Chai
/// \author Matthew Bremer Bruchon

#include "soci.h"
#include "Node.hpp"
#include "Section.hpp"
#include "Lane.hpp"
#include "Crossing.hpp"
#include "Turning.hpp"
#include "Polyline.hpp"
#include "BusStop.hpp"
#include "./Signal.hpp"
#include "Phase.hpp"
#include "geospatial/PathSetManager.hpp"

//using namespace sim_mob::aimsun;
//using std::string;








namespace soci
{

template<> struct type_conversion<sim_mob::aimsun::Node>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::aimsun::Node &res)
    {
    	res.id = vals.get<int>("node_id", 0);
    	res.xPos = vals.get<double>("xpos", 0.0);
    	res.yPos = vals.get<double>("ypos", 0.0);
    	res.isIntersection = vals.get<int>("isintersection", 0);
    }
    static void to_base(const sim_mob::aimsun::Node& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("node_id", src.id);
        vals.set("xpos", src.xPos);
        vals.set("ypos", src.yPos);
        vals.set("isintersection", src.isIntersection?1:0);
        ind = i_ok;
    }
};
template<> struct type_conversion<sim_mob::PathSet>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::PathSet &res)
    {
    	res.id = vals.get<std::string>("ID", "");
    	res.from_node_id = vals.get<std::string>("FROM_NODE_ID", "");
    	res.to_node_id = vals.get<std::string>("TO_NODE_ID", "");
//    	res.person_id = vals.get<string>("PERSON_ID", "");
//    	res.trip_id = vals.get<string>("TRIP_ID", "");
    	res.singlepath_id = vals.get<std::string>("SINGLEPATH_ID", "");
    	res.scenario = vals.get<std::string>("SCENARIO", "");
    	res.has_path = vals.get<int>("HAS_PATH", 0);
    }
    static void to_base(const sim_mob::PathSet& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("ID", src.id);
        vals.set("FROM_NODE_ID", src.from_node_id);
        vals.set("TO_NODE_ID", src.to_node_id);
//        vals.set("PERSON_ID", src.person_id);
//        vals.set("TRIP_ID", src.trip_id);
        vals.set("SINGLEPATH_ID", src.singlepath_id);
        vals.set("SCENARIO", src.scenario);
        vals.set("HAS_PATH", src.has_path);
        ind = i_ok;
    }
};
template<> struct type_conversion<sim_mob::SinglePath>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::SinglePath &res)
    {
    	res.id = vals.get<std::string>("ID", "");
//    	res.exclude_seg_id = vals.get<string>("EXCLUDE_SEG_ID", "");
    	res.pathset_id = vals.get<std::string>("PATHSET_ID", "");
//    	res.waypointset = vals.get<std::string>("WAYPOINTSET", "");
//    	res.from_node_id = vals.get<std::string>("FROM_NODE_ID", "");
//    	res.to_node_id = vals.get<std::string>("TO_NODE_ID", "");
    	res.utility = vals.get<double>("UTILITY", 0);
    	res.pathsize = vals.get<double>("PATHSIZE", 0);
    	res.travel_cost = vals.get<double>("TRAVEL_COST", 0);
    	res.signal_number = vals.get<int>("SIGNAL_NUMBER", 0);
    	res.right_turn_number = vals.get<int>("RIGHT_TURN_NUMBER", 0);
    	res.scenario = vals.get<std::string>("SCENARIO", "");
    	res.length = vals.get<double>("LENGTH",0);
    	res.travle_time = vals.get<double>("TRAVEL_TIME",0);
    }
    static void to_base(const sim_mob::SinglePath& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("ID", src.id);
//        vals.set("EXCLUDE_SEG_ID", src.exclude_seg_id);
        vals.set("PATHSET_ID", src.pathset_id);
//        vals.set("WAYPOINTSET", src.waypointset);
//        vals.set("FROM_NODE_ID", src.from_node_id);
//        vals.set("TO_NODE_ID", src.to_node_id);
        vals.set("UTILITY", src.utility);
        vals.set("PATHSIZE", src.pathsize);
        vals.set("TRAVEL_COST", src.travel_cost);
        vals.set("SIGNAL_NUMBER", src.signal_number);
        vals.set("RIGHT_TURN_NUMBER", src.right_turn_number);
        vals.set("SCENARIO", src.scenario);
        vals.set("LENGTH", src.length);
        vals.set("TRAVEL_TIME", src.travle_time);
        ind = i_ok;
    }
};
template<> struct type_conversion<sim_mob::ERP_Surcharge>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::ERP_Surcharge &res)
    {
    	res.Gantry_No = vals.get<std::string>("gantry_no", "");
    	res.Start_Time = vals.get<std::string>("start_time", "00:00:00");
    	res.End_Time = vals.get<std::string>("end_time", "00:00:00");
    	res.Rate = vals.get<double>("Rate", 0.0);
    	res.Vehicle_Type_Id = vals.get<int>("Vehicle_Type_Id", 0);
    	res.Vehicle_Type_Desc = vals.get<std::string>("Vehicle_Type_Desc", "");
    	res.Day = vals.get<std::string>("Day", "");
    }
    static void to_base(const sim_mob::ERP_Surcharge& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("Gantry_No", src.Gantry_No);
        vals.set("Start_Time", src.Start_Time);
        vals.set("End _Time", src.End_Time);
        vals.set("Rate", src.Rate);
        vals.set("Vehicle_Type_Id", src.Vehicle_Type_Id);
        vals.set("Vehicle_Type_Desc", src.Vehicle_Type_Desc);
        vals.set("Day", src.Day);
        ind = i_ok;
    }
};
template<> struct type_conversion<sim_mob::ERP_Section>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::ERP_Section &res)
    {
    	res.ERP_Gantry_No = vals.get<int>("ERP_Gantry_No", 0);
    	res.section_id = vals.get<int>("section_id", 0);
    }
    static void to_base(const sim_mob::ERP_Section& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("ERP_Gantry_No", src.ERP_Gantry_No);
        vals.set("section_id", src.section_id);
        ind = i_ok;
    }
};
template<> struct type_conversion<sim_mob::ERP_Gantry_Zone>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::ERP_Gantry_Zone &res)
    {
    	res.Gantry_no = vals.get<std::string>("Gantry_no", "");
    	res.Zone_Id = vals.get<std::string>("Zone_Id", "");
    }
    static void to_base(const sim_mob::ERP_Gantry_Zone& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("Gantry_no", src.Gantry_no);
        vals.set("Zone_Id", src.Zone_Id);
        ind = i_ok;
    }
};
template<> struct type_conversion<sim_mob::Link_travel_time>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::Link_travel_time &res)
    {
//    	res.gid = vals.get<int>("gid", 0);
    	res.link_id = vals.get<int>("link_id", 0);
    	res.start_time = vals.get<std::string>("start_time", "00:00:00");
    	res.end_time = vals.get<std::string>("end_time", "00:00:00");
    	res.travel_time = vals.get<double>("travel_time", 0.0);
    }
    static void to_base(const sim_mob::Link_travel_time& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("link_id", src.link_id);
        vals.set("start_time", src.start_time);
        vals.set("end_time", src.end_time);
        vals.set("travel_time", src.travel_time);
        ind = i_ok;
    }
};

template<> struct type_conversion<sim_mob::aimsun::Section>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::aimsun::Section &res)
    {
    	res.id = vals.get<int>("id", 0);
    	res.roadName = vals.get<std::string>("road_name", "");
    	res.numLanes = vals.get<int>("nb_lanes", 1);
    	res.speed = vals.get<double>("speed", 0);
    	res.capacity = vals.get<double>("capacity", 0);
    	res.length = vals.get<double>("length", 0);
    	res.TMP_FromNodeID = vals.get<int>("fnode", 0);
    	res.TMP_ToNodeID = vals.get<int>("tnode", 0);
    }
    static void to_base(const sim_mob::aimsun::Section& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("id", src.id);
        vals.set("road_name", src.roadName);
        vals.set("nb_lanes", src.numLanes);
        vals.set("speed", src.speed);
        vals.set("capacity", src.capacity);
        vals.set("length", src.length);
        vals.set("fnode", src.fromNode->id);
        vals.set("tnode", src.toNode->id);
        ind = i_ok;
    }
};

template<> struct type_conversion<sim_mob::aimsun::Phase>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::aimsun::Phase &res)
    {
    	res.name = vals.get<std::string>("phases","");
    	res.nodeId = vals.get<int>("node_id",0);
    	res.sectionFrom = vals.get<int>("from_section",0);
    	res.sectionTo = vals.get<int>("to_section",0);
    	res.laneFrom_A = vals.get<int>("from_lane_a", 0);
    	res.laneTo_A = vals.get<int>("to_lane_a", 0);
    	res.laneFrom_B = vals.get<int>("from_lane_b", 0);
    	res.laneTo_B = vals.get<int>("to_lane_b", 0);
    }
    static void to_base(const sim_mob::aimsun::Phase& src, soci::values& vals, soci::indicator& ind)
    {
//    	vals.set("id", src.id);
        vals.set("phases", src.name);
        vals.set("node_id", src.nodeId);
        vals.set("from_section", src.sectionFrom);
        vals.set("to_section", src.sectionTo);
        vals.set("from_lane_a", src.laneFrom_A);
        vals.set("from_lane_b", src.laneFrom_B);
        vals.set("to_lane_a", src.laneTo_A);
        vals.set("to_lane_b", src.laneTo_B);
        ind = i_ok;
    }
};

template<> struct type_conversion<sim_mob::aimsun::Turning>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::aimsun::Turning &res)
    {
    	res.id = vals.get<int>("turning_id", 0);
    	res.fromLane.first = vals.get<int>("from_lane_a", 0);
    	res.fromLane.second = vals.get<int>("from_lane_b", 0);
    	res.toLane.first = vals.get<int>("to_lane_a", 0);
    	res.toLane.second = vals.get<int>("to_lane_b", 0);
    	res.TMP_FromSection = vals.get<int>("from_section", 0);
    	res.TMP_ToSection = vals.get<int>("to_section", 0);
    }
    static void to_base(const sim_mob::aimsun::Turning& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("turning_id", src.id);
    	vals.set("from_lane_a", src.fromLane.first);
    	vals.set("from_lane_b", src.fromLane.second);
    	vals.set("to_lane_a", src.toLane.first);
    	vals.set("to_lane_b", src.toLane.second);
    	vals.set("from_section", src.fromSection->id);
    	vals.set("to_section", src.toSection->id);
        ind = i_ok;
    }
};


template<> struct type_conversion<sim_mob::aimsun::Polyline>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::aimsun::Polyline &res)
    {
    	res.xPos = vals.get<double>("xpos", 0.0);
    	res.yPos = vals.get<double>("ypos", 0.0);
    	res.TMP_SectionId = vals.get<int>("section_id", 0);
    }
    static void to_base(const sim_mob::aimsun::Polyline& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("section_id", src.section->id);
        vals.set("xpos", src.xPos);
        vals.set("ypos", src.yPos);
        ind = i_ok;
    }
};

template<> struct type_conversion<sim_mob::aimsun::Crossing>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::aimsun::Crossing &res)
    {
    	res.laneID = vals.get<int>("lane_id", 0);
    	res.laneType = vals.get<std::string>("lane_type", "");
    	res.TMP_AtSectionID = vals.get<int>("section", 0);
    	res.xPos = vals.get<double>("xpos", 0.0);
    	res.yPos = vals.get<double>("ypos", 0.0);
    }
    static void to_base(const sim_mob::aimsun::Crossing& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("lane_id", src.laneID);
    	vals.set("lane_type", src.laneType);
    	vals.set("section", src.atSection->id);
    	vals.set("xpos", src.xPos);
    	vals.set("ypos", src.yPos);
        ind = i_ok;
    }
};


template<> struct type_conversion<sim_mob::aimsun::Lane>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::aimsun::Lane &res)
    {
    	res.laneID = vals.get<int>("lane_id", 0);
    	res.laneType = vals.get<std::string>("lane_type", "");
    	res.TMP_AtSectionID = vals.get<int>("section", 0);
    	res.xPos = vals.get<double>("xpos", 0.0);
    	res.yPos = vals.get<double>("ypos", 0.0);
        res.rowNo = vals.get<int>("rowno", 0);
    }
    static void to_base(const sim_mob::aimsun::Lane& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("lane_id", src.laneID);
    	vals.set("lane_type", src.laneType);
    	vals.set("section", src.atSection->id);
    	vals.set("xpos", src.xPos);
    	vals.set("ypos", src.yPos);
        vals.set("rowno", src.rowNo);
        ind = i_ok;
    }
};



template<>
struct type_conversion<sim_mob::aimsun::Signal>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::aimsun::Signal& signal)
    {
        signal.id = values.get<int>("signal_id", 0);
        signal.nodeId = values.get<int>("node_id", 0);
        signal.xPos = values.get<double>("xpos", 0.0);
        signal.yPos = values.get<double>("ypos", 0.0);
        signal.typeCode = values.get<std::string>("type_cd", "");
        signal.bearing = values.get<double>("bearg", 0.0);
    }

    static void
    to_base(sim_mob::aimsun::Signal const & signal, soci::values & values, soci::indicator & indicator)
    {
        values.set("signal_id", signal.id);
        values.set("node_id", signal.nodeId);
        values.set("xpos", signal.xPos);
        values.set("ypos", signal.yPos);
        values.set("type_cd", signal.typeCode);
        values.set("bearg", signal.bearing);
        indicator = i_ok;
    }
};


template<> struct type_conversion<sim_mob::aimsun::BusStop>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::aimsun::BusStop &res)
    {
    	res.bus_stop_no = vals.get<std::string>("bus_stop_no", "");
    	res.TMP_AtSectionID= vals.get<int>("section_id", 0);
    	res.status = vals.get<std::string>("status", "");
    	res.lane_type = vals.get<std::string>("lane_type", "");
    	res.road_name = vals.get<std::string>("road_name", "");
    	res.xPos = vals.get<double>("x_pos", 0.0);
    	res.yPos = vals.get<double>("y_pos", 0.0);
    }
    static void to_base(const sim_mob::aimsun::BusStop& src, soci::values& vals, soci::indicator& ind)
    {
    	//std::cout<<"I am here"<<src.xPos<<"    "<<src.yPos<<std::endl;
    	vals.set("bus_stop_id", src.bus_stop_no);
    	vals.set("section_id", src.atSection->id);
    	vals.set("status", src.status);
    	vals.set("lane_type", src.lane_type);
    	vals.set("road_name", src.road_name);
        vals.set("x_pos", src.xPos);
        vals.set("y_pos", src.yPos);
        ind = i_ok;
    }
};

template<> struct type_conversion<sim_mob::aimsun::BusStopSG>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::aimsun::BusStopSG &res)
    {
    	res.bus_stop_no = vals.get<std::string>("stop_id", "");
    	res.stop_code= vals.get<std::string>("stop_code", "");
    	res.stop_name = vals.get<std::string>("stop_name", "");
    	res.stop_lat = vals.get<std::string>("stop_lat", "");
    	res.stop_lon = vals.get<std::string>("stop_lon", "");
    	res.section_id = vals.get<std::string>("section_id", "");
    	res.aimsun_section = vals.get<int>("aimsun_section", 0);
    }
    static void to_base(const sim_mob::aimsun::BusStopSG& src, soci::values& vals, soci::indicator& ind)
    {
    	//std::cout<<"I am here"<<src.xPos<<"    "<<src.yPos<<std::endl;
    	vals.set("stop_id", src.bus_stop_no);
    	vals.set("stop_code", src.stop_code);
    	vals.set("stop_name", src.stop_name);
    	vals.set("stop_lat", src.xPos);
    	vals.set("stop_lon", src.yPos);
        vals.set("section_id", src.section_id);
        vals.set("aimsun_section", src.aimsun_section);
        ind = i_ok;
    }
};

}
