/* Copyright Singapore-MIT Alliance for Research and Technology */

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
//

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


}
