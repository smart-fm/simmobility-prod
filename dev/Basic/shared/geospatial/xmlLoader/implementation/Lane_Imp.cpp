#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::lane_t_pimpl::pre ()
{
	model = sim_mob::Lane();
}

sim_mob::Lane* sim_mob::xml::lane_t_pimpl::post_lane_t ()
{
	//Note: We only allocate memory in post() to avoid leaking memory if the parser generates an error.
	return new sim_mob::Lane(model);
}

void sim_mob::xml::lane_t_pimpl::laneID (unsigned long long value)
{
	//TODO: Another discrepancy here: setLaneID takes an unsigned int.
	model.setLaneID(static_cast<unsigned int>(value));

	//add it to the book keeper
	//geo_Lanes_[static_cast<unsigned long>(laneID)] = lane;
	//NOTE: Temporarily disabling bookkeeper. ~Seth
}

void sim_mob::xml::lane_t_pimpl::PolyLine (std::vector<sim_mob::Point2D> value)
{
	  model.setLanePolyline(value);
}

void sim_mob::xml::lane_t_pimpl::width (unsigned int value)
{
	model.setLaneWidth(value);
}

void sim_mob::xml::lane_t_pimpl::can_go_straight (bool value)
{
	model.can_go_straight(value);
}

void sim_mob::xml::lane_t_pimpl::can_turn_left (bool value)
{
	  model.can_turn_left(value);
}

void sim_mob::xml::lane_t_pimpl::can_turn_right (bool value)
{
	model.can_turn_right(value);
}

void sim_mob::xml::lane_t_pimpl::can_turn_on_red_signal (bool value)
{
	model.can_turn_on_red_signal(value);
}

void sim_mob::xml::lane_t_pimpl::can_change_lane_left (bool value)
{
	model.can_change_lane_left(value);
}

void sim_mob::xml::lane_t_pimpl::can_change_lane_right (bool value)
{
	model.can_change_lane_right(value);
}

void sim_mob::xml::lane_t_pimpl::is_road_shoulder (bool value)
{
	model.is_road_shoulder(value);
}

void sim_mob::xml::lane_t_pimpl::is_bicycle_lane (bool value)
{
	model.is_bicycle_lane(value);
}

void sim_mob::xml::lane_t_pimpl::is_pedestrian_lane (bool value)
{
	model.is_pedestrian_lane(value);
}

void sim_mob::xml::lane_t_pimpl::is_vehicle_lane (bool value)
{
	model.is_vehicle_lane(value);
}

void sim_mob::xml::lane_t_pimpl::is_standard_bus_lane (bool value)
{
	model.is_standard_bus_lane(value);
}

void sim_mob::xml::lane_t_pimpl::is_whole_day_bus_lane (bool value)
{
	model.is_whole_day_bus_lane(value);
}

void sim_mob::xml::lane_t_pimpl::is_high_occupancy_vehicle_lane (bool value)
{
	model.is_high_occupancy_vehicle_lane(value);
}

void sim_mob::xml::lane_t_pimpl::can_freely_park_here (bool value)
{
	model.can_freely_park_here(value);
}

void sim_mob::xml::lane_t_pimpl::can_stop_here (bool value)
{
	model.can_stop_here(value);
}

void sim_mob::xml::lane_t_pimpl::is_u_turn_allowed (bool value)
{
	model.is_u_turn_allowed(value);
}




