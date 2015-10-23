//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

/// \file SOCI_Converters.hpp
///This file contains several Type converters for SOCI object-mapping
/// \author Seth N. Hetu

#include "boost/algorithm/string.hpp"
#include "soci/soci.h"
#include "TripChain.hpp"


using namespace sim_mob::aimsun;
using std::string;


namespace soci
{


template<> struct type_conversion<sim_mob::aimsun::TripChainItem>
{
    typedef values base_type;
    static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::aimsun::TripChainItem &res)
    {
    	res.personID = vals.get<std::string>("entityid","");
    	res.sequenceNumber = vals.get<int>("trip_chain_sequence_number",0);
    	res.itemType = sim_mob::TripChainItem::getItemType(vals.get<std::string>("trip_chain_item_type",""));
    	if(res.itemType == sim_mob::TripChainItem::IT_TRIP) {
    		res.tripID = vals.get<std::string>("trip_id", "");
    		res.tmp_tripfromLocationNodeID = vals.get<int>("trip_from_location_id",0);
    		res.tripfromLocationType = sim_mob::TripChainItem::LT_NODE; //sim_mob::TripChainItem::getLocationType(vals.get<std::string>("trip_from_location_type",""));
    		res.tmp_triptoLocationNodeID = vals.get<int>("trip_to_location_id",0);
    		res.triptoLocationType = sim_mob::TripChainItem::LT_NODE; //sim_mob::TripChainItem::getLocationType(vals.get<std::string>("trip_to_location_type",""));
    		res.tmp_subTripID = vals.get<std::string>("sub_trip_id","");
    		res.tmp_fromLocationNodeID = vals.get<int>("from_location_id",0);
    		res.fromLocationType =  sim_mob::TripChainItem::LT_NODE; //sim_mob::TripChainItem::getLocationType(vals.get<std::string>("from_location_type",""));
    		res.tmp_toLocationNodeID = vals.get<int>("to_location_id",0);
    		res.toLocationType = sim_mob::TripChainItem::LT_NODE; //sim_mob::TripChainItem::getLocationType(vals.get<std::string>("to_location_type",""));
    		res.mode = vals.get<std::string>("mode","");
    		res.isPrimaryMode = vals.get<int>("primary_mode", 0);
    		res.ptLineId = vals.get<std::string>("pt_line_id","");
    		res.tmp_startTime = vals.get<std::string>("start_time","");
    	}
    	else if(res.itemType == sim_mob::TripChainItem::IT_ACTIVITY) {
    		res.tmp_activityID = vals.get<std::string>("activity_id", "");
    		res.description = vals.get<std::string>("activity_description", "");
    		res.isPrimary = vals.get<int>("primary_activity", 0);
    		res.isFlexible = vals.get<int>("flexible_activity", 0);
    		res.isMandatory = vals.get<int>("mandatory_activity", 0);
    		res.tmp_locationID = vals.get<int>("location_id", 0);
    		res.locationType = sim_mob::TripChainItem::getLocationType(vals.get<std::string>("location_type", ""));
    		res.tmp_startTime = vals.get<std::string>("activity_start_time", "");
    		res.tmp_endTime = vals.get<std::string>("activity_end_time", "");
    	}
    	else {
    		throw std::runtime_error("Couldn't load Trip Chain; unexpected type.");
    	}
    }

    static void to_base(const sim_mob::aimsun::TripChainItem& src, soci::values& vals, soci::indicator& ind)
    {
    	throw std::runtime_error("TripChainItem::to_base() not implemented");
    }
};



template<>
struct type_conversion<sim_mob::PT_BusDispatchFreq>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::PT_BusDispatchFreq& ptBusDispatchFreq)
    {
    	ptBusDispatchFreq.frequencyId = values.get<std::string>("frequency_id", "");
    	boost::trim(ptBusDispatchFreq.frequencyId);
    	ptBusDispatchFreq.routeId = values.get<std::string>("route_id", "");
    	boost::trim(ptBusDispatchFreq.routeId);
    	ptBusDispatchFreq.startTime = sim_mob::DailyTime(values.get<std::string>("start_time", ""));
    	ptBusDispatchFreq.endTime = sim_mob::DailyTime(values.get<std::string>("end_time", ""));
    	ptBusDispatchFreq.headwaySec = values.get<int>("headway_sec", 0);
    }

    static void
    to_base(sim_mob::PT_BusDispatchFreq const & ptBusDispatchFreq, soci::values & values, soci::indicator & indicator)
    {
        values.set("frequency_id", ptBusDispatchFreq.frequencyId);
        values.set("route_id", ptBusDispatchFreq.routeId);
        values.set("start_time", ptBusDispatchFreq.startTime.getStrRepr());
        values.set("end_time", ptBusDispatchFreq.endTime.getStrRepr());
        values.set("headway_sec", ptBusDispatchFreq.headwaySec);
        indicator = i_ok;
    }
};

template<>
struct type_conversion<sim_mob::PT_BusRoutes>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::PT_BusRoutes& ptBusRoutes)
    {
    	ptBusRoutes.routeId = values.get<std::string>("route_id", "");
    	boost::trim(ptBusRoutes.routeId);
    	ptBusRoutes.linkId = values.get<std::string>("link_id", "");
    	boost::trim(ptBusRoutes.linkId);
    	ptBusRoutes.sequenceNo = values.get<int>("link_sequence_no", 0);
    }

    static void
    to_base(sim_mob::PT_BusRoutes const & ptBusRoutes, soci::values & values, soci::indicator & indicator)
    {
        values.set("route_id", ptBusRoutes.routeId);
        values.set("link_id", ptBusRoutes.linkId);
        values.set("link_sequence_no", ptBusRoutes.sequenceNo);
        indicator = i_ok;
    }
};

template<>
struct type_conversion<sim_mob::PT_BusStops>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::PT_BusStops& ptBusStops)
    {
    	ptBusStops.routeId = values.get<std::string>("route_id", "");
    	boost::trim(ptBusStops.routeId);
    	ptBusStops.stopNo = values.get<std::string>("busstop_no", "");
    	boost::trim(ptBusStops.stopNo);
    	ptBusStops.sequenceNo = values.get<int>("busstop_sequence_no", 0);
    }

    static void
    to_base(sim_mob::PT_BusStops const & ptBusStops, soci::values & values, soci::indicator & indicator)
    {
        values.set("route_id", ptBusStops.routeId);
        values.set("busstop_no", ptBusStops.stopNo);
        values.set("busstop_sequence_no", ptBusStops.sequenceNo);
        indicator = i_ok;
    }
};

template<>
struct type_conversion<sim_mob::OD_Trip>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::OD_Trip& od_trip)
    {
    	od_trip.startStop = values.get<std::string>("start_stop", "");
    	od_trip.endStop = values.get<std::string>("end_stop", "");
    	od_trip.sType = values.get<int>("start_type", -1);
    	od_trip.eType = values.get<int>("end_type", -1);
    	od_trip.tType = values.get<std::string>("r_type", "");
    	od_trip.serviceLines = values.get<std::string>("r_service_lines", "");
    	od_trip.originNode = values.get<std::string>("origin_node", "");
    	od_trip.destNode = values.get<std::string>("dest_node", "");
    	od_trip.pathset = values.get<std::string>("pathset", "");
       	od_trip.id = values.get<int>("id", 0);
       	od_trip.travelTime = values.get<double>("travel_time", 0);
       	boost::trim_right(od_trip.startStop);
       	boost::trim_right(od_trip.endStop);
       	boost::trim_right(od_trip.serviceLines);
       	boost::trim_right(od_trip.tType);
       	boost::trim_right(od_trip.pathset);
       	boost::trim_right(od_trip.originNode);
       	boost::trim_right(od_trip.destNode);
    }

    static void
    to_base(sim_mob::OD_Trip const & od_trip, soci::values & values, soci::indicator & indicator)
    {
    	values.set("start_stop", od_trip.startStop);
       	values.set("end_stop", od_trip.endStop);
       	values.set("r_type", od_trip.tType);
       	values.set("r_service_lines", od_trip.serviceLines);
       	values.set("origin_node", od_trip.originNode);
       	values.set("dest_node", od_trip.destNode);
       	values.set("pathset", od_trip.pathset);
       	values.set("id", od_trip.id);
        indicator = i_ok;
    }
};
}
