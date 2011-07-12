#pragma once

#include <bitset>


namespace sim_mob
{

/**
 * Lane movement rules.
 *
 * \todo
 * We need a utility function for checking bitflags easily.
 */
enum LANE_MOVEMENT_RULES {
	//Who can drive here.
	LANE_ALLOW_VEHICLES     = 0x0001,
	LANE_ALLOW_BICYCLES     = 0x0002,
	LANE_ALLOW_PEDESTRIANS  = 0x0004,

	//Will likely be moved to Intersection or Link
	LANE_MAKE_U_TURN        = 0x0008,

	//Lane changing rules, traffic light turning
	LANE_LEFT_LC            = 0x0010,
	LANE_RIGHT_LC           = 0x0020,
	LANE_CAN_TURN_ON_RED    = 0x0040,

	//Parking rules
	LANE_STOP_ALLOWED       = 0x0080,
	LANE_PARK_ALLOWED       = 0x0100,

	//General driving restrictions
	LANE_STANDARD_BUS_LANE           = 0x0200,
	LANE_WHOLE_DAY_BUS_LANE          = 0x0400,
	LANE_IS_SHOULDER                 = 0x0800,
	LANE_ALLOW_HIGH_OCC_VEHICLES     = 0x1000,
};


/**
 * A lane for motorized vehicles.
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 */
class Lane {
public:
	//Derived
	bool can_go_straight() { return false; }
	bool can_turn_left() { return false; }
	bool can_turn_right() { return false; }

	//From the bitflags
	bool can_turn_on_red_signal() { return false; }
	bool is_standard_bus_lane() { return false; }
	bool is_whole_day_bus_lane() { return false; }
	bool can_change_lane_left() { return false; }
	bool can_change_lane_right() { return false; }
	bool is_road_shoulder() { return false; }
	bool is_bicycle_lane() { return false; }
	bool is_pedestrian_lane() { return false; }
	bool is_vehicles_lane() { return false; }
	bool is_high_occ_vehicle_lane() { return false; }
	bool can_park_here() { return false; }
	bool can_stop_here() { return false; }

	//Should probably not be part of the "Lane" class...
	bool is_u_turn_allowed() { return false; }


private:
	std::bitset<0xFF> movementRules;



};





}
