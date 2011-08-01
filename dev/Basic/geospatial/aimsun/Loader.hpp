#pragma once

#include <vector>
#include <iostream>


namespace sim_mob
{

//Forward declarations
class RoadNetwork;

namespace aimsun
{


/**
 * Class for loading AimSun data and converting it to a format compatible with out network.
 */
class Loader {
public:
	///Load AIMSUN-generated data. Currently, this is done in three steps:
	/// 1) Load data into AIMSUN-style classes. (This will presumably be automated with
	///    some kind of persistence framework later.)
	/// 2) Transform these classes into the Sim Mobility format, logging any irregularities.
	/// 3) Discard the AIMSUN classes; return the Sim Mobility classes.
	static void LoadNetwork(sim_mob::RoadNetwork& rn);
};



}
}
