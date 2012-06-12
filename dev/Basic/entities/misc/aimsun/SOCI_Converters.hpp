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
    	res.tmp_personID = vals.get<int>("PersonId",0);
    	res.setSequenceNumber(vals.get<int>("Trip_Chain_Sequence_Number",0));
    	res.tmp_itemType = vals.get<std::string>("Trip_Chain_Item_Type","");
    	if(res.tmp_itemType.compare("Trip") == 0){
    		aimsun::SubTrip *aSubTrip = dynamic_cast<aimsun::SubTrip>(res);
    		aSubTrip->tripID = vals.get<int>("Trip_ID", 0);
    		aSubTrip->tmp_subTripID = vals.get<int>("Sub_Trip_Id",0);
    		aSubTrip->tmp_fromLocationNodeID = vals.get<int>("From_Location_Id",0);
    		aSubTrip->fromLocationType = aimsun::TripChainItem::getLocationType(vals.get<std::string>("From_Location_Type",""));
    		aSubTrip->tmp_toLocationNodeID = vals.get<int>("To_Location_Id",0);
    		aSubTrip->toLocationType = aimsun::TripChainItem::getLocationType(vals.get<std::string>("To_Location_Type",""));
    		aSubTrip->mode = vals.get<int>("Description",0);
    		aSubTrip->startTime = vals.get<std::string>("Start_Time",0);
    	}
    	else if(res.tmp_itemType.compare("Activity") == 0){
    		aimsun::Activity *anActivity = dynamic_cast<aimsun::Activity>(res);
    		anActivity->tmp_activityID = vals.get<int>("Activity_Id", 0);
    		anActivity->description = vals.get<std::string>("Activity_Description", "");
    		anActivity->isPrimary = vals.get<bool>("Primary_Activity", false);
    		anActivity->isFlexible = vals.get<bool>("Flexible_Activity", false);
    		anActivity->tmp_locationID = vals.get<int>("Location_ID", 0);
    		anActivity->locationType = aimsun::TripChainItem::getLocationType(vals.get<std::string>("Location_Type", ""));
    		anActivity->tmp_activityStartTime = vals.get<std::string>("Activity_Start_Time", "");
    		anActivity->tmp_activityEndTime = vals.get<std::string>("Activity_End_Time", "");
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
