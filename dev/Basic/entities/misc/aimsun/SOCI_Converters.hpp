/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

/// \file SOCI_Converters.hpp
///This file contains several Type converters for SOCI object-mapping
/// \author Seth N. Hetu

#include "soci.h"
#include "TripChain.hpp"


using namespace sim_mob::aimsun;
using std::string;


namespace soci
{


template<> struct type_conversion<aimsun::TripChainItem>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, aimsun::TripChainItem &res)
    {
    	res.entityID = vals.get<int>("entityid",0);
    	res.setSequenceNumber(vals.get<int>("trip_chain_sequence_number",0));
    	res.itemType = sim_mob::TripChainItem::getItemType(vals.get<std::string>("trip_chain_item_type",""));
    	if(res.itemType == sim_mob::trip){
    		aimsun::SubTrip& aSubTrip = static_cast<aimsun::SubTrip&>(res);
    		aSubTrip.tripID = vals.get<int>("trip_id", 0);
    		aSubTrip.tmp_subTripID = vals.get<int>("sub_trip_id",0);
    		aSubTrip.tmp_fromLocationNodeID = vals.get<int>("from_location_id",0);
    		aSubTrip.fromLocationType = sim_mob::TripChainItem::getLocationType(vals.get<std::string>("from_location_type",""));
    		aSubTrip.tmp_toLocationNodeID = vals.get<int>("to_location_id",0);
    		aSubTrip.toLocationType = sim_mob::TripChainItem::getLocationType(vals.get<std::string>("to_location_type",""));
    		aSubTrip.mode = vals.get<std::string>("description","");
    		aSubTrip.tmp_startTime = vals.get<std::string>("start_time","");
    	}
    	else if(res.itemType == sim_mob::activity){
    		aimsun::Activity& anActivity = static_cast<aimsun::Activity&>(res);
    		anActivity.tmp_activityID = vals.get<int>("activity_id", 0);
    		anActivity.description = vals.get<std::string>("activity_description", "");
    		anActivity.isPrimary = vals.get<bool>("primary_activity", false);
    		anActivity.isFlexible = vals.get<bool>("flexible_activity", false);
    		anActivity.tmp_locationID = vals.get<int>("location_id", 0);
    		anActivity.locationType = sim_mob::TripChainItem::getLocationType(vals.get<std::string>("location_type", ""));
    		anActivity.tmp_activityStartTime = vals.get<std::string>("activity_start_time", "");
    		anActivity.tmp_activityEndTime = vals.get<std::string>("activity_end_time", "");
    	}
    	else {
    		// error
    	}
    }

    static void to_base(const aimsun::TripChainItem& src, soci::values& vals, soci::indicator& ind)
    {
    	/*vals.set("activity_id", src.EMPTY_activityID);
    	vals.set("from_activity_desc", src.from.description);
    	vals.set("from_location", src.from.location->id);
    	vals.set("to_activity_desc", src.to.description);
    	vals.set("to_location", src.to.location->id);
    	vals.set("primary_activity", src.primary?1:0);
    	vals.set("flexible_activity", src.flexible?1:0);
    	vals.set("trip_start", src.startTime.toString());
    	vals.set("transport_mode", src.mode);
        ind = i_ok;*/
    }
};


}
