//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

/// \file SOCI_Converters.hpp
///This file contains several Type converters for SOCI object-mapping
/// \author Seth N. Hetu
/// \author LIM Fung Chai
/// \author Matthew Bremer Bruchon

#include "soci/soci.h"
#include "Node.hpp"
#include "Section.hpp"
#include "Lane.hpp"
#include "Crossing.hpp"
#include "Turning.hpp"
#include "Polyline.hpp"
#include "Signal.hpp"
#include "Phase.hpp"
#include "path/Path.hpp"
#include "path/PathSetManager.hpp"
#include "path/PathSetParam.hpp"
#include "entities/PersonLoader.hpp"
#include "geospatial/network/TurningPath.hpp"
#include "geospatial/network/TurningConflict.hpp"
#include "geospatial/network/PolyLine.hpp"
#include "geospatial/network/PT_Stop.hpp"

//using namespace sim_mob::aimsun;
//using std::string;


namespace soci
{
class ERP_Section;
class ERP_Surcharge;
class ERP_Gantry_Zone;
class LinkTravelTime;
class PathSet;
class SinglePath;

template<> struct type_conversion<sim_mob::CBD_Pair>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::CBD_Pair &res)
    {
    	res.from_section = vals.get<int>("from_section", 0);
    	res.to_section = vals.get<int>("to_section", 0);
    }
};

template<> struct type_conversion<sim_mob::aimsun::Node>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::aimsun::Node &res)
    {
    	res.id = vals.get<int>("node_id", 0);
    	res.xPos = vals.get<double>("xpos", 0.0);
    	res.yPos = vals.get<double>("ypos", 0.0);
        //res.nodeName = vals.get<std::string>("nodename", "");
    	res.isIntersection = vals.get<int>("isintersection", 0);
    	res.hasTrafficSignal = vals.get<int>("hassignal", 0);
    }
    static void to_base(const sim_mob::aimsun::Node& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("node_id", src.id);
        vals.set("xpos", src.xPos);
        vals.set("ypos", src.yPos);
        //vals.set("nodename", src.nodeName);
        vals.set("isintersection", src.isIntersection?1:0);
        vals.set("hvTLights", src.hasTrafficSignal?1:0);
        ind = i_ok;
    }
};

template<> struct type_conversion<sim_mob::SegmentType>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::SegmentType &res)
    {
    	res.id = vals.get<std::string>("id", "");
    	res.type = vals.get<int>("type", 0);
    }
    static void to_base(const sim_mob::SegmentType& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("id", src.id);
        vals.set("type", src.type);
        ind = i_ok;
    }
};

/*
template<> struct type_conversion<sim_mob::NodeType>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::NodeType &res)
    {
    	res.id = vals.get<std::string>("id", "");
    	res.type = vals.get<int>("type", 0);
    }
    static void to_base(const sim_mob::NodeType& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("id", src.id);
        vals.set("type", src.type);
        ind = i_ok;
    }
};
*/

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
    	res.serviceCategory = vals.get<std::string>("category", "");
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
        vals.set("category", src.serviceCategory);
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

/*
template<> struct type_conversion<TurningPath>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, TurningPath &res)
    {
    	res.setDbId(vals.get<int>("id", -1));
    	res.setFrom_xpos(vals.get<double>("from_xpos", -1.0));
    	res.setFrom_ypos(vals.get<double>("from_ypos", -1.0));
    	res.setTo_xpos(vals.get<double>("to_xpos", -1.0));
    	res.setTo_ypos(vals.get<double>("to_ypos", -1.0));
    	res.setFrom_road_section(vals.get<std::string>("from_road_section", ""));
    	res.setTo_road_section(vals.get<std::string>("to_road_section", ""));
    	res.setFrom_lane_index(vals.get<int>("from_lane_index", -1));
    	res.setTo_lane_index(vals.get<int>("to_lane_index", -1));
        res.setTurningSpeed(vals.get<int>("turning_speed", 20));
        res.setHasStopSign(vals.get<int>("has_stop_sign", 0));
    }
    static void to_base(const TurningPath& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("id", src.getDbId());
    	vals.set("from_xpos", src.getFrom_xpos());
    	vals.set("from_ypos", src.getFrom_ypos());
    	vals.set("to_xpos", src.getTo_xpos());
    	vals.set("to_ypos", src.getTo_ypos());
    	vals.set("from_road_section", src.getFrom_road_section());
    	vals.set("to_road_section", src.getTo_road_section());
    	vals.set("from_lane_index", src.getFrom_lane_index());
    	vals.set("to_lane_index", src.getTo_lane_index());
        vals.set("turning_speed", src.getTurningSpeed());
        vals.set("has_stop_sign", (src.turningHasStopSign() ? 1 : 0));
        ind = i_ok;
    }
};
template<> struct type_conversion<sim_mob::TurningConflict>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::TurningConflict &res)
    {
    	res.setDbId(vals.get<int>("id", -1));
    	res.setFirst_turning(vals.get<std::string>("first_turning", ""));
    	res.setSecond_turning(vals.get<std::string>("second_turning", ""));
    	res.setFirst_cd(vals.get<double>("first_cd", -1.0));
    	res.setSecond_cd(vals.get<double>("second_cd", -1.0));
        res.setCriticalGap(vals.get<double>("critical_gap", 1.5));
        res.setPriority(vals.get<int>("priority", 0));
    }
    static void to_base(const sim_mob::TurningConflict& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("id", src.getDbId());
    	vals.set("first_turning", src.getFirst_turning());
    	vals.set("second_turning", src.getSecond_turning());
    	vals.set("first_cd", src.getFirst_cd());
    	vals.set("second_cd", src.getSecond_cd());
        vals.set("critical_gap", src.getCriticalGap());
        vals.set("priority", src.getPriority());
        ind = i_ok;
    }
};

template<> struct type_conversion<sim_mob::TurningPolyline>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::TurningPolyline &res)
    {
    	res.setId(vals.get<int>("id", -1));
    	res.setTurningId(vals.get<int>("turning", -1));
    	res.setType(vals.get<int>("type", -1));
    	res.setLength(vals.get<double>("length", -1));
    	res.setScenario(vals.get<std::string>("scenario"));
    }
    static void to_base(const sim_mob::TurningPolyline& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("id", src.getId());
    	vals.set("turning", src.getTurningId());
    	vals.set("type", src.getType());
    	vals.set("length", src.getLength());
    	vals.set("scenario", src.getScenario());
        ind = i_ok;
    }
};

template<> struct type_conversion<sim_mob::Polypoint>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::Polypoint &res)
    {
    	res.id = vals.get<int>("id", -1);
    	res.polylineId = vals.get<int>("polyline", -1);
    	res.index = vals.get<int>("index", -1);
    	res.x = vals.get<double>("x", 0.0);
    	res.y = vals.get<double>("y", 0.0);
    	//res.z = vals.get<double>("z", 0.0);
    	res.scenario = vals.get<std::string>("scenario", "");
    }
    static void to_base(const sim_mob::Polypoint& src, soci::values& vals, soci::indicator& ind)
    {
    	vals.set("id", src.id);
    	vals.set("polyline", src.polylineId);
    	vals.set("index", src.index);
    	vals.set("x", src.x);
    	vals.set("y", src.y);
    	vals.set("z", src.z);
    	vals.set("scenario", src.scenario);
        ind = i_ok;
    }
};
*/
}
