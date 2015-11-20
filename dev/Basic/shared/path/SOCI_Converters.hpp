//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "soci/soci.h"
#include "path/Path.hpp"
#include "path/PathSetManager.hpp"
#include "path/PathSetParam.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "util/DailyTime.hpp"

namespace soci
{

template<> struct type_conversion<sim_mob::CBD_Pair>
{
	typedef values base_type;
	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::CBD_Pair &res)
	{
		res.from_section = vals.get<int>("from_section", 0);
		res.to_section = vals.get<int>("to_section", 0);
	}
};

template<> struct type_conversion<sim_mob::PT_Path>
{
	typedef values base_type;
	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::PT_Path &res)
	{
		res.setPtPathId(vals.get<std::string>("path_id", ""));
		res.setPtPathSetId(vals.get<std::string>("pathset_id", ""));
		res.setScenario(vals.get<std::string>("scenario", ""));
		res.setPathTravelTime(vals.get<double>("path_travel_time_secs", 0));
		res.setTotalDistanceKms(vals.get<double>("total_distance_kms", 0));
		res.setPathSize(vals.get<double>("path_size", 0.0));
		res.setTotalCost(vals.get<double>("total_cost", 0.0));
		res.setTotalInVehicleTravelTimeSecs(vals.get<double>("total_in_vehicle_travel_time_secs", 0.0));
		res.setTotalWaitingTimeSecs(vals.get<double>("total_waiting_time", 0));
		res.setTotalWalkingTimeSecs(vals.get<double>("total_walking_time", 0));
		res.setTotalNumberOfTransfers(vals.get<int>("total_number_of_transfers", 0));
		res.updatePathEdges();
	}
};

template<> struct type_conversion<sim_mob::SinglePath>
{
	typedef values base_type;
	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::SinglePath &res)
	{
		res.id = vals.get<std::string>("id", "");
		res.pathSetId = vals.get<std::string>("pathset_id", "");
		res.partialUtility = vals.get<double>("partial_utility", 0.0);
		res.pathSize = vals.get<double>("path_size", 0.0);
		res.signalNumber = vals.get<int>("signal_number", 0);
		res.rightTurnNumber = vals.get<int>("right_turn_number", 0);
		res.scenario = vals.get<std::string>("scenario", "");
		res.length = vals.get<double>("length", 0.0);
		res.highWayDistance = vals.get<double>("highway_distance", 0.0);
		res.isMinDistance = vals.get<int>("min_distance", 0);
		res.isMinSignal = vals.get<int>("min_signal", 0);
		res.isMinRightTurn = vals.get<int>("min_right_turn", 0);
		res.isMaxHighWayUsage = vals.get<int>("max_highway_usage", 0);
		res.valid_path = vals.get<int>("valid_path", 0);
		res.isShortestPath = vals.get<int>("shortest_path", 0);
		res.index = vals.get<long long>("serial_id", 0);
	}
	static void to_base(const sim_mob::SinglePath& src, soci::values& vals, soci::indicator& ind)
	{
		vals.set("id", src.id);
		vals.set("pathset_id", src.pathSetId);
		vals.set("partial_utility", src.partialUtility);
		vals.set("path_size", src.pathSize);
		vals.set("signal_number", src.signalNumber);
		vals.set("right_turn_number", src.rightTurnNumber);
		vals.set("scenario", src.scenario);
		vals.set("length", src.length);
		vals.set("highway_distance", src.highWayDistance);
		vals.set("min_distance", (src.isMinDistance ? 1 : 0));
		vals.set("min_signal", (src.isMinSignal ? 1 : 0));
		vals.set("min_right_turn", (src.isMinRightTurn ? 1 : 0));
		vals.set("max_highway_usage", (src.isMaxHighWayUsage ? 1 : 0));
		vals.set("valid_path", (src.valid_path ? 1 : 0));
		vals.set("shortest_path", (src.isShortestPath ? 1 : 0));
		vals.set("serial_id", src.index);
		ind = i_ok;
	}
};
template<> struct type_conversion<sim_mob::ERP_Surcharge>
{
	typedef values base_type;
	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::ERP_Surcharge &res)
	{
		res.gantryNo = vals.get<std::string>("gantry_no", "");
		res.startTime = vals.get<std::string>("start_time", "00:00:00");
		res.endTime = vals.get<std::string>("end_time", "00:00:00");
		res.rate = vals.get<double>("rate", 0.0);
		res.vehicleTypeId = vals.get<int>("vehicle_type_id", 0);
		res.vehicleTypeDesc = vals.get<std::string>("vehicle_type_desc", "");
		res.day = vals.get<std::string>("day_type", "");
	}
};
template<> struct type_conversion<sim_mob::ERP_Section>
{
	typedef values base_type;
	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::ERP_Section &res)
	{
		res.sectionId = vals.get<unsigned int>("section_id", 0);
		res.linkId = vals.get<unsigned int>("link_id", 0);
		res.ERP_Gantry_No = vals.get<int>("erp_gantry_no", 0);
	}
};

template<> struct type_conversion<sim_mob::ERP_Gantry_Zone>
{
	typedef values base_type;
	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::ERP_Gantry_Zone &res)
	{
		res.gantryNo = vals.get<std::string>("gantry_no", "");
		res.zoneId = vals.get<std::string>("zone_id", "");
	}
};

template<> struct type_conversion<sim_mob::LinkTravelTime>
{
	typedef values base_type;
	static void from_base(const soci::values& vals, soci::indicator& ind, sim_mob::LinkTravelTime &res)
	{
		res.setLinkId(vals.get<int>("link_id", 0));
		res.setDefaultTravelTime(vals.get<double>("travel_time", 0.0));
	}
	static void to_base(const sim_mob::LinkTravelTime& src, soci::values& vals, soci::indicator& ind)
	{
		vals.set("link_id", src.getLinkId());
		vals.set("travel_time", src.getDefaultTravelTime());

		ind = i_ok;
	}
};

}
