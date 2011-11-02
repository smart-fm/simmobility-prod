/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

//This file contains several Type converters for SOCI object-mapping

#include "soci/soci.h"
#include "TripChain.hpp"


using namespace sim_mob::aimsun;
using std::string;


namespace soci
{


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
    	res.TMP_startTimeStr = vals.get<std::string>("trip_start", "");
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
    	vals.set("trip_start", src.startTime.toString());
    	vals.set("transport_mode", src.mode);
        ind = i_ok;
    }
};


}
