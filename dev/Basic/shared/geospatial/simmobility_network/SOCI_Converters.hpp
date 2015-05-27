//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/tokenizer.hpp>
#include <string>
#include <vector>

#include "soci.h"
#include "Lane.hpp"
#include "LaneConnector.hpp"
#include "Link.hpp"
#include "Node.hpp"
#include "Point.hpp"
#include "RoadNetwork.hpp"
#include "Tag.hpp"
#include "TurningGroup.hpp"
#include "TurningPath.hpp"

using namespace simmobility_network;

//Anonymous name-space for helper functions
namespace
{
  //Helper functions
  std::vector<Tag>* ParseStringToTags(std::string strTags)
  {
    //For tokenizing the string
    boost::tokenizer<> tokens(strTags);
    boost::tokenizer<>::iterator itTokens = tokens.begin();
    
    //Vector of tags
    std::vector<Tag> *tags = new std::vector<Tag>();
    
    while(itTokens != tokens.end())
    {
      //The string representation is as follows:
      //"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"
      
      std::string key = *itTokens;
      
      ++itTokens;
      
      std::string value = *itTokens;
      
      ++itTokens;
      
      //Create a tag
      Tag tag(key, value);
      
      //Add tag to the vector
      tags->push_back(tag);
    }
    
    return tags;
  }
}

namespace soci
{    
  
  template<> struct type_conversion<simmobility_network::Node>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, simmobility_network::Node& res)
    {
      res.setNodeId(vals.get<unsigned int>("id", 0));
      res.setNodeType(vals.get<simmobility_network::NodeType>("node_type", simmobility_network::DEFAULT_NODE));
      res.setTrafficLightId(vals.get<unsigned int>("traffic_light_id", 0));
      
      //Create and set the node location
      double x = vals.get<double>("x", 0);
      double y = vals.get<double>("y", 0);
      double z = vals.get<double>("z", 0);
      Point *location = new Point(0, 0, x, y, z);      
      res.setLocation(location);
      
      //Read the string representing the tags.
      //The string representation is as follows:
      //"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"
      std::string strTags = vals.get<std::string>("tags", "");
      
      //Parse the string to a vector tags
      std::vector<Tag> *tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<simmobility_network::TurningGroup>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, simmobility_network::TurningGroup& res)
    {
      res.setTurningGroupId(vals.get<unsigned int>("id", 0));
      res.setFromLinkId(vals.get<unsigned int>("from_link", 0));
      res.setNodeId(vals.get<unsigned int>("node_id", 0));
      res.setPhases(vals.get<std::string>("phases", ""));
      res.setRules(vals.get<simmobility_network::TurningGroupRules>("rules", simmobility_network::TURNING_GROUP_RULE_NO_STOP_SIGN));
      res.setToLinkId(vals.get<unsigned int>("to_link", 0));
      res.setVisibility(vals.get<double>("visibility", 0));
      
      //Read the string representing the tags.
      //The string representation is as follows:
      //"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"
      std::string strTags = vals.get<std::string>("tags", "");
      
      //Parse the string to a vector tags
      std::vector<Tag> *tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<simmobility_network::TurningPath>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, simmobility_network::TurningPath& res)
    {
      res.setTurningPathId(vals.get<unsigned int>("id", 0));
      res.setFromLaneId(vals.get<unsigned int>("from_lane", 0));
      res.setToLaneId(vals.get<unsigned int>("to_lane", 0));
      res.setTurningGroupId(vals.get<unsigned int>("group_id", 0));
      
      //Read the string representing the tags.
      //The string representation is as follows:
      //"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"
      std::string strTags = vals.get<std::string>("tags", "");
      
      //Parse the string to a vector tags
      std::vector<Tag> *tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<simmobility_network::TurningConflict>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, simmobility_network::TurningConflict& res)
    {
      res.setConflictId(vals.get<unsigned int>("id", 0));
      res.setCriticalGap(vals.get<double>("gap_time", 0));
      res.setFirstConflictDistance(vals.get<double>("cd1", 0));
      res.setFirstTurningId(vals.get<unsigned int>("turning_path1", 0));
      res.setPriority(vals.get<int>("priority", 0));
      res.setSecondConflictDistance(vals.get<double>("cd2", 0));
      res.setSecondTurningId(vals.get<unsigned int>("turning_path2", 0));
      
      //Read the string representing the tags.
      //The string representation is as follows:
      //"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"
      std::string strTags = vals.get<std::string>("tags", "");
      
      //Parse the string to a vector tags
      std::vector<Tag> *tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<simmobility_network::Link>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, simmobility_network::Link& res)
    {
      res.setLinkId(vals.get<unsigned int>("id", 0));
      res.setFromNodeId(vals.get<unsigned int>("from_node", 0));
      res.setLinkCategory(vals.get<simmobility_network::LinkCategory>("category", simmobility_network::LINK_CATEGORY_DEFAULT));
      res.setRoadName(vals.get<std::string>("road_name", ""));
      res.setToNodeId(vals.get<unsigned int>("to_node", 0));
      
      //Read the string representing the tags.
      //The string representation is as follows:
      //"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"
      std::string strTags = vals.get<std::string>("tags", "");
      
      //Parse the string to a vector tags
      std::vector<Tag> *tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<simmobility_network::RoadSegment>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, simmobility_network::RoadSegment& res)
    {
      res.setRoadSegmentId(vals.get<unsigned int>("id", 0));
      res.setCapacity(vals.get<unsigned int>("capacity", 0));
      res.setLinkId(vals.get<unsigned int>("link_id", 0));
      res.setMaxSpeed(vals.get<unsigned int>("max_speed", 0));
      res.setSequenceNumber(vals.get<unsigned int>("sequence_no", 0));
      
      //Read the string representing the tags.
      //The string representation is as follows:
      //"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"
      std::string strTags = vals.get<std::string>("tags", "");
      
      //Parse the string to a vector tags
      std::vector<Tag> *tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<simmobility_network::Lane>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, simmobility_network::Lane& res)
    {
      res.setLaneId(vals.get<unsigned int>("id", 0));
      res.setBusLaneRules(vals.get<simmobility_network::BusLaneRules>("bus_lane", simmobility_network::BUS_LANE_RULES_CAR_AND_BUS));
      res.setCanVehiclePark(vals.get<int>("can_park", 0));
      res.setCanVehicleStop(vals.get<int>("can_stop", 0));
      res.setHasRoadShoulder(vals.get<int>("has_road_shoulder", 0));
      res.setHighOccupancyVehicleAllowed(vals.get<int>("high_occ_veh", 0));
      res.setRoadSegmentId(vals.get<unsigned int>("segment_id", 0));
      res.setWidth(vals.get<double>("width", 0));
      
      //Read the string representing the tags.
      //The string representation is as follows:
      //"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"
      std::string strTags = vals.get<std::string>("tags", "");
      
      //Parse the string to a vector tags
      std::vector<Tag> *tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<simmobility_network::LaneConnector>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, simmobility_network::LaneConnector& res)
    {
      res.setLaneConnectionId(vals.get<unsigned int>("id", 0));
      res.setFromLaneId(vals.get<unsigned int>("from_lane", 0));
      res.setFromRoadSegmentId(vals.get<unsigned int>("from_segment", 0));
      res.setToLaneId(vals.get<unsigned int>("to_lane", 0));
      res.setToRoadSegmentId(vals.get<unsigned int>("to_segment", 0));
    }
  };
  
  template<> struct type_conversion<simmobility_network::Point>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, simmobility_network::Point& res)
    {
      res.setPolyLineId(vals.get<unsigned int>("polyline_id", 0));
      res.setSequenceNumber(vals.get<unsigned int>("sequence_no", 0));
      res.setX(vals.get<double>("x", 0));
      res.setY(vals.get<double>("y", 0));
      res.setZ(vals.get<double>("z", 0));
    }
  };
  
}