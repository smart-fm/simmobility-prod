/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once


namespace sim_mob
{

/**
 * Class which holds all of our Debug values, to make turning them on or off simple.
 *
 * \author Seth N. Hetu
 *
 * Intended to allow usage like "if (Debug::DriverPaths){}"
 */
class Debug {
public:
	///Debug work groups and worker semantics. Adding/staging entities, etc.
	static const bool WorkGroupSemantics = false;

	///Debug Driver-specific behavior like changing lanes.
	static const bool Drivers = true;

	///Debug the General Path Mover; all advances, polyline changes, and segment changes.
	/// It is recommended to turn off all other debugging flags when using this option, since
	/// Path debugging uses a lot of output space and doesn't end every line with a newline.
	static const bool Paths = true;
};


}


