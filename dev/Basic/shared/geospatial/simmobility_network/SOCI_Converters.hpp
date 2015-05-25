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
#include "TurningGroup.hpp"
#include "TurningPath.hpp"

using namespace simmobility_network;

//Anonymous name-space for helper functions
namespace
{
  //Helper functions
  std::vector<Tag>& ParseStringToTags(std::string strTags)
  {
    //The vector of tags
    std::vector<Tag> tags;
    
    //For tokenizing the string
    boost::tokenizer<> tokens(strTags);
    boost::tokenizer<>::iterator itTokens = tokens.begin();
    
    while(itTokens != tokens.end())
    {
      //Create a tag
      Tag tag;
      
      //Set the key
      tag.setKey(*itTokens);
      
      ++itTokens;
      
      //Set the value
      tag.setValue(*itTokens);
      
      ++itTokens;
      
      //Add tag to the vector
      tags.push_back(tag);
    }
    
    return tags;
  }
}

namespace soci
{    
  
  template<> struct type_conversion<Node>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, Node& res)
    {
      res.setNodeId(vals.get<unsigned int>("id", 0));
      res.setNodeType(vals.get<int>("node_type", 0));
      res.setTrafficLightId(vals.get<unsigned int>("traffic_light_id", 0));
      
      //Create and set the node location
      double x = vals.get<double>("x", 0);
      double y = vals.get<double>("y", 0);
      double z = vals.get<double>("z", 0);
      Point *location = new Point(0, x, y, z);      
      res.setLocation(location);
      
      //Read the string representing the tags.
      //The string representation is as follows:
      //"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"
      std::string strTags = vals.get<std::string>("tags", "");
      
      //Parse the string to a vector tags
      std::vector<Tag>& tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<TurningGroup>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, TurningGroup& res)
    {
      res.setTurningGroupId(vals.get<unsigned int>("id", 0));
      res.setFromLinkId(vals.get<unsigned int>("from_link", 0));
      res.setNodeId(vals.get<unsigned int>("node_id", 0));
      res.setPhases(vals.get<std::string>("phases", ""));
      res.setRules(vals.get<int>("rules", 0));
      res.setToLinkId(vals.get<unsigned int>("to_link", 0));
      res.setVisibility(vals.get<double>("visibility", 0));
      
      //Read the string representing the tags.
      //The string representation is as follows:
      //"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"
      std::string strTags = vals.get<std::string>("tags", "");
      
      //Parse the string to a vector tags
      std::vector<Tag>& tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<TurningPath>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, TurningPath& res)
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
      std::vector<Tag>& tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<TurningConflict>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, TurningConflict& res)
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
      std::vector<Tag>& tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<Link>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, Link& res)
    {
      res.setLinkId(vals.get<unsigned int>("id", 0));
      res.setFromNodeId(vals.get<unsigned int>("from_node", 0));
      res.setLinkCategory(vals.get<int>("category", 0));
      res.setRoadName(vals.get<std::string>("road_name", ""));
      res.setToNodeId(vals.get<unsigned int>("to_node", 0));
      
      //Read the string representing the tags.
      //The string representation is as follows:
      //"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"
      std::string strTags = vals.get<std::string>("tags", "");
      
      //Parse the string to a vector tags
      std::vector<Tag>& tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_converstion<RoadSegment>
  {
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, RoadSegment& res)
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
      std::vector<Tag>& tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<Lane>
  {
    typedef values base_def;
    static void from_base(const soci::values& vals, soci::indicator& ind, Lane& res)
    {
      res.setLaneId(vals.get<unsigned int>("id", 0));
      res.setBusLaneRules(vals.get<int>("bus_lane", 0));
      res.setCanVehiclePark(vals.get<int>("can_park", 0));
      res.setCanVehicleStop(vals.get<int>("can_stop", 0));
      res.setHasRoadShoulder(vals.get<int>("has_road_shoulder", 0));
      res.setHighOccupancyVehicleAllowed(vals.get<int>("high_occ_veh", 0));
      res.setWidth(vals.get<double>("width", 0));
      
      //Read the string representing the tags.
      //The string representation is as follows:
      //"<key_1>:value_1,<key_2>:value_2,...<key_n>:value_n"
      std::string strTags = vals.get<std::string>("tags", "");
      
      //Parse the string to a vector tags
      std::vector<Tag>& tags = ParseStringToTags(strTags);
      res.setTags(tags);
    }
  };
  
  template<> struct type_conversion<LaneConnector>
  {
    typedef values base_def;
    static void from_base(const soci::values& vals, soci::indicator& ind, LaneConnector& res)
    {
      res.setLaneConnectionId(vals.get<unsigned int>("id", 0));
      res.setFromLaneId(vals.get<unsigned int>("from_lane", 0));
      res.setFromRoadSegmentId(vals.get<unsigned int>("from_segment", 0));
      res.setToLaneId(vals.get<unsigned int>("to_lane", 0));
      res.setToRoadSegmentId(vals.get<unsigned int>("to_segment", 0));
    }
  };
  
  template<> struct type_conversion<Point>
  {
    typedef values base_def;
    static void from_base(const soci::values& vals, soci::indicator& ind, Point& res)
    {
      res.setSequenceNumber(vals.get<unsigned int>("sequence_no", 0));
      res.setX(vals.get<double>("x", 0));
      res.setY(vals.get<double>("y", 0));
      res.setZ(vals.get<double>("z", 0));
    }
  };
  
}