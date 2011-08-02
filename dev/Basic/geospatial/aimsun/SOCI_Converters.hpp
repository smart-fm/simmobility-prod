/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

//This file contains several Type converters for SOCI object-mapping

#include "soci/soci.h"
#include "Node.hpp"


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






}



