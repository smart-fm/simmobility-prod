#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::lane_t_pimpl::pre ()
{
	lane = new sim_mob::Lane();
}

sim_mob::Lane* sim_mob::xml::lane_t_pimpl::post_lane_t ()
{
  return lane;
}

void sim_mob::xml::lane_t_pimpl::laneID (unsigned long long laneID)
{
	lane->setLaneID(laneID);//setid_str embedded

	//add it to the book keeper
	//TODO: The "unsigned long long" passed in to this function is actually stored as an "unsigned long" in the bookkeeper.
	//      Leaving a static cast here so we don't forget it.
	//geo_Lanes_[static_cast<unsigned long>(laneID)] = lane;
	//NOTE: Temporarily disabling bookkeeper. ~Seth
}

void sim_mob::xml::lane_t_pimpl::PolyLine (std::vector<sim_mob::Point2D> PolyLine)
{
	  lane->setLanePolyline(PolyLine);
}

void sim_mob::xml::lane_t_pimpl::width (unsigned int width)
{
	lane->setLaneWidth(width);
}

void sim_mob::xml::lane_t_pimpl::can_go_straight (bool can_go_straight)
{
	lane->can_go_straight(can_go_straight);
}

void sim_mob::xml::lane_t_pimpl::can_turn_left (bool can_turn_left)
{
	  lane->can_turn_left(can_turn_left);
}

void sim_mob::xml::lane_t_pimpl::can_turn_right (bool can_turn_right)
{
	lane->can_turn_right(can_turn_right);
}

void sim_mob::xml::lane_t_pimpl::can_turn_on_red_signal (bool can_turn_on_red_signal)
{
	lane->can_turn_on_red_signal(can_turn_on_red_signal);
}

void sim_mob::xml::lane_t_pimpl::can_change_lane_left (bool can_change_lane_left)
{
	lane->can_change_lane_left(can_change_lane_left);
}

void sim_mob::xml::lane_t_pimpl::can_change_lane_right (bool can_change_lane_right)
{
	lane->can_change_lane_right(can_change_lane_right);
}

void sim_mob::xml::lane_t_pimpl::is_road_shoulder (bool is_road_shoulder)
{
	lane->is_road_shoulder(is_road_shoulder);
}

void sim_mob::xml::lane_t_pimpl::is_bicycle_lane (bool is_bicycle_lane)
{
	lane->is_bicycle_lane(is_bicycle_lane);
}

void sim_mob::xml::lane_t_pimpl::is_pedestrian_lane (bool is_pedestrian_lane)
{
	lane->is_pedestrian_lane(is_pedestrian_lane);
}

void sim_mob::xml::lane_t_pimpl::is_vehicle_lane (bool is_vehicle_lane)
{
	lane->is_vehicle_lane(is_vehicle_lane);
}

void sim_mob::xml::lane_t_pimpl::is_standard_bus_lane (bool is_standard_bus_lane)
{
	lane->is_standard_bus_lane(is_standard_bus_lane);
}

void sim_mob::xml::lane_t_pimpl::is_whole_day_bus_lane (bool is_whole_day_bus_lane)
{
	lane->is_whole_day_bus_lane(is_whole_day_bus_lane);
}

void sim_mob::xml::lane_t_pimpl::is_high_occupancy_vehicle_lane (bool is_high_occupancy_vehicle_lane)
{
	lane->is_high_occupancy_vehicle_lane(is_high_occupancy_vehicle_lane);
}

void sim_mob::xml::lane_t_pimpl::can_freely_park_here (bool can_freely_park_here)
{
	lane->can_freely_park_here(can_freely_park_here);
}

void sim_mob::xml::lane_t_pimpl::can_stop_here (bool can_stop_here)
{
	lane->can_stop_here(can_stop_here);
}

void sim_mob::xml::lane_t_pimpl::is_u_turn_allowed (bool is_u_turn_allowed)
{
	lane->is_u_turn_allowed(is_u_turn_allowed);
}




