/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

//This file contains several Type converters for SOCI object-mapping

#include "soci/soci.h"
#include "Node.hpp"
#include "Section.hpp"
#include "Lane.hpp"
#include "Crossing.hpp"
#include "Turning.hpp"
#include "Polyline.hpp"
#include "TripChain.hpp"


using namespace sim_mob::aimsun;
using std::string;


namespace soci
{

template<> struct type_conversion<Node>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, Node &res)
    {
    	res.id = vals.get<int>("node_id", 0);
    	res.xPos = vals.get<double>("xpos", 0.0);
    	res.yPos = vals.get<double>("ypos", 0.0);
    	res.isIntersection = vals.get<int>("isintersection", 0);
    }
    static void to_base(const Node& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("node_id", src.id);
        vals.set("xpos", src.xPos);
        vals.set("ypos", src.yPos);
        vals.set("isintersection", src.isIntersection?1:0);
        ind = i_ok;
    }
};


template<> struct type_conversion<Section>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, Section &res)
    {
    	res.id = vals.get<int>("id", 0);
    	res.roadName = vals.get<string>("road_name", "");
    	res.numLanes = vals.get<int>("nb_lanes", 1);
    	res.speed = vals.get<double>("speed", 0);
    	res.capacity = vals.get<double>("capacity", 0);
    	res.length = vals.get<double>("length", 0);
    	res.TMP_FromNodeID = vals.get<int>("fnode", 0);
    	res.TMP_ToNodeID = vals.get<int>("tnode", 0);
    }
    static void to_base(const Section& src, soci::values& vals, soci::indicator& ind)
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



template<> struct type_conversion<Turning>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, Turning &res)
    {
    	res.id = vals.get<int>("turning_id", 0);
    	res.fromLane.first = vals.get<int>("from_lane_a", 0);
    	res.fromLane.second = vals.get<int>("from_lane_b", 0);
    	res.toLane.first = vals.get<int>("to_lane_a", 0);
    	res.toLane.second = vals.get<int>("to_lane_b", 0);
    	res.TMP_FromSection = vals.get<int>("from_section", 0);
    	res.TMP_ToSection = vals.get<int>("to_section", 0);
    }
    static void to_base(const Turning& src, soci::values& vals, soci::indicator& ind)
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


template<> struct type_conversion<Polyline>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, Polyline &res)
    {
    	res.xPos = vals.get<double>("xpos", 0.0);
    	res.yPos = vals.get<double>("ypos", 0.0);
    	res.TMP_SectionId = vals.get<int>("section_id", 0);
    }
    static void to_base(const Polyline& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("section_id", src.section->id);
        vals.set("xpos", src.xPos);
        vals.set("ypos", src.yPos);
        ind = i_ok;
    }
};

template<> struct type_conversion<Crossing>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, Crossing &res)
    {
    	res.laneID = vals.get<int>("lane_id", 0);
    	res.laneType = vals.get<std::string>("lane_type", "");
    	res.TMP_AtSectionID = vals.get<int>("section", 0);
    	res.xPos = vals.get<double>("xpos", 0.0);
    	res.yPos = vals.get<double>("ypos", 0.0);
    }
    static void to_base(const Crossing& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("lane_id", src.laneID);
    	vals.set("lane_type", src.laneType);
    	vals.set("section", src.atSection->id);
    	vals.set("xpos", src.xPos);
    	vals.set("ypos", src.yPos);
        ind = i_ok;
    }
};


template<> struct type_conversion<Lane>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, Lane &res)
    {
    	res.laneID = vals.get<int>("lane_id", 0);
    	res.laneType = vals.get<std::string>("lane_type", "");
    	res.TMP_AtSectionID = vals.get<int>("section", 0);
    	res.xPos = vals.get<double>("xpos", 0.0);
    	res.yPos = vals.get<double>("ypos", 0.0);
    }
    static void to_base(const Lane& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("lane_id", src.laneID);
    	vals.set("lane_type", src.laneType);
    	vals.set("section", src.atSection->id);
    	vals.set("xpos", src.xPos);
    	vals.set("ypos", src.yPos);
        ind = i_ok;
    }
};


template<> struct type_conversion<TripChain>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, TripChain &res)
    {
    	res.id = vals.get<int>("activity_id", 0);
    	res.from.description = vals.get<std::string>("from_activity_desc", "");
    	res.from.TMP_locationNodeID = vals.get<int>("from_location", 0);
    	res.to.description = vals.get<std::string>("to_activity_desc", "");
    	res.to.TMP_locationNodeID = vals.get<int>("to_location", 0);
    	res.primary = vals.get<int>("primary_activity", 0);
    	res.flexible = vals.get<int>("flexible_activity", 0);
    	res.startTime = vals.get<double>("trip_start", 0);
    	res.mode = vals.get<std::string>("transport_mode", "");
    }
    static void to_base(const TripChain& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("activity_id", src.id);
    	vals.set("from_activity_desc", src.from.description);
    	vals.set("from_location", src.from.location->id);
    	vals.set("to_activity_desc", src.to.description);
    	vals.set("to_location", src.to.location->id);
    	vals.set("primary_activity", src.primary?1:0);
    	vals.set("flexible_activity", src.flexible?1:0);
    	vals.set("trip_start", src.startTime);
    	vals.set("transport_mode", src.mode);
        ind = i_ok;
    }
};



}
