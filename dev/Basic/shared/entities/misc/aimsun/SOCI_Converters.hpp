/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

/// \file SOCI_Converters.hpp
///This file contains several Type converters for SOCI object-mapping
/// \author Seth N. Hetu

#include "soci.h"
#include "TripChain.hpp"
#include "entities/misc/BusSchedule.hpp"


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
    		res.tripfromLocationType = sim_mob::TripChainItem::getLocationType(vals.get<std::string>("trip_from_location_type",""));
    		res.tmp_triptoLocationNodeID = vals.get<int>("trip_to_location_id",0);
    		res.triptoLocationType = sim_mob::TripChainItem::getLocationType(vals.get<std::string>("trip_to_location_type",""));
    		res.tmp_subTripID = vals.get<std::string>("sub_trip_id","");
    		res.tmp_fromLocationNodeID = vals.get<int>("from_location_id",0);
    		res.fromLocationType = sim_mob::TripChainItem::getLocationType(vals.get<std::string>("from_location_type",""));
    		res.tmp_toLocationNodeID = vals.get<int>("to_location_id",0);
    		res.toLocationType = sim_mob::TripChainItem::getLocationType(vals.get<std::string>("to_location_type",""));
    		res.mode = vals.get<std::string>("mode","");
    		res.isPrimaryMode = vals.get<int>("primary_mode", 0);
    		res.ptLineId = vals.get<std::string>("public_transit_line_id","");
    		res.tmp_startTime = vals.get<std::string>("start_time","");
    	}
    	else if(res.itemType == sim_mob::TripChainItem::IT_ACTIVITY) {
    		res.tmp_activityID = vals.get<int>("activity_id", 0);
    		res.description = vals.get<std::string>("activity_description", "");
    		std::cout << "Activity description: " << res.description << std::endl;
    		res.isPrimary = vals.get<int>("primary_activity", 0);
    		res.isFlexible = vals.get<int>("flexible_activity", 0);
    		res.isMandatory = vals.get<int>("mandatory_activity", 0);
    		res.tmp_locationID = vals.get<int>("location_id", 0);
    		std::cout << "Activity location_type: " << vals.get<std::string>("location_type", "") << std::endl;
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
struct type_conversion<sim_mob::BusSchedule>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::BusSchedule& bus_schedule)
    {
    	bus_schedule.tripid = values.get<std::string>("trip_id", "");
    	bus_schedule.startTime = sim_mob::DailyTime(values.get<std::string>("start_time", ""));
    }

    static void
    to_base(sim_mob::BusSchedule const & bus_schedule, soci::values & values, soci::indicator & indicator)
    {
        values.set("trip_id", bus_schedule.tripid);
        values.set("start_time", bus_schedule.startTime.toString());
        indicator = i_ok;
    }
};

template<>
struct type_conversion<sim_mob::PT_trip>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::PT_trip& pt_trip)
    {
    	pt_trip.trip_id = values.get<std::string>("trip_id", "");
    	pt_trip.service_id = values.get<std::string>("service_id", "");
    	pt_trip.route_id = values.get<std::string>("route_id", "");
    	pt_trip.start_time = sim_mob::DailyTime(values.get<std::string>("start_time", ""));
    	pt_trip.end_time = sim_mob::DailyTime(values.get<std::string>("end_time", ""));
    }

    static void
    to_base(sim_mob::PT_trip const & pt_trip, soci::values & values, soci::indicator & indicator)
    {
        values.set("trip_id", pt_trip.trip_id);
        values.set("service_id", pt_trip.service_id);
        values.set("route_id", pt_trip.route_id);
        values.set("start_time", pt_trip.start_time.toString());
        values.set("end_time", pt_trip.end_time.toString());
        indicator = i_ok;
    }
};

template<>
struct type_conversion<sim_mob::PT_bus_dispatch_freq>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::PT_bus_dispatch_freq& pt_busdispatch_freq)
    {
    	pt_busdispatch_freq.frequency_id = values.get<std::string>("frequency_id", "");
    	pt_busdispatch_freq.route_id = values.get<std::string>("route_id", "");
    	pt_busdispatch_freq.start_time = sim_mob::DailyTime(values.get<std::string>("start_time", ""));
    	pt_busdispatch_freq.end_time = sim_mob::DailyTime(values.get<std::string>("end_time", ""));
    	pt_busdispatch_freq.headway_sec = values.get<int>("headway_sec", 0);
    }

    static void
    to_base(sim_mob::PT_bus_dispatch_freq const & pt_busdispatch_freq, soci::values & values, soci::indicator & indicator)
    {
        values.set("frequency_id", pt_busdispatch_freq.frequency_id);
        values.set("route_id", pt_busdispatch_freq.route_id);
        values.set("start_time", pt_busdispatch_freq.start_time.toString());
        values.set("end_time", pt_busdispatch_freq.end_time.toString());
        values.set("headway_sec", pt_busdispatch_freq.headway_sec);
        indicator = i_ok;
    }
};

template<>
struct type_conversion<sim_mob::PT_bus_routes>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::PT_bus_routes& pt_bus_routes)
    {
    	pt_bus_routes.route_id = values.get<std::string>("route_id", "");
    	pt_bus_routes.link_id = values.get<std::string>("link_id", "");
    	pt_bus_routes.link_sequence_no = values.get<int>("link_sequence_no", 0);
    }

    static void
    to_base(sim_mob::PT_bus_routes const & pt_bus_routes, soci::values & values, soci::indicator & indicator)
    {
        values.set("route_id", pt_bus_routes.route_id);
        values.set("link_id", pt_bus_routes.link_id);
        values.set("link_sequence_no", pt_bus_routes.link_sequence_no);
        indicator = i_ok;
    }
};

template<>
struct type_conversion<sim_mob::PT_bus_stops>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::PT_bus_stops& pt_bus_stops)
    {
    	pt_bus_stops.route_id = values.get<std::string>("route_id", "");
    	pt_bus_stops.busstop_no = values.get<std::string>("busstop_no", "");
    	pt_bus_stops.busstop_sequence_no = values.get<int>("busstop_sequence_no", 0);
    }

    static void
    to_base(sim_mob::PT_bus_stops const & pt_bus_stops, soci::values & values, soci::indicator & indicator)
    {
        values.set("route_id", pt_bus_stops.route_id);
        values.set("busstop_no", pt_bus_stops.busstop_no);
        values.set("busstop_sequence_no", pt_bus_stops.busstop_sequence_no);
        indicator = i_ok;
    }
};


}
