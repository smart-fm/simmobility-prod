#pragma once


namespace sim_mob
{

/**
 * Lane movement rules.
 *
 * \todo
 * We need a utility function for checking bitflags easily.
 */
enum LANE_MOVEMENT_RULES {
	//Turning rules
	LANE_MOVE_STRAIGHT      = 0x0001,
	LANE_TURN_LEFT          = 0x0002,
	LANE_TURN_RIGHT         = 0x0004,
	LANE_MAKE_U_TURN        = 0x0008,

	//Lane changing rules
	LANE_LEFT_LC            = 0x0010,
	LANE_RIGHT_LC           = 0x0020,

	//Parking rules
	LANE_STOP_ALLOWED       = 0x0040,
	LANE_PARK_ALLOWED       = 0x0080,

	//General driving restrictions
	LANE_BUS_LANE           = 0x0100,
	LANE_HAS_SHOULDER       = 0x0200,
	LANE_BICYCLE_LANE       = 0x0800,
	LANE_ZIGZAG_LINE        = 0x1000,
};


/**
 * A lane for motorized vehicles.
 */
class Lane {
public:


private:
	unsigned int movementRuleBitflag;



};





}
