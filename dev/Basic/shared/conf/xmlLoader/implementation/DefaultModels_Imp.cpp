#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::default_models_pimpl::pre ()
{
}

void sim_mob::conf::default_models_pimpl::post_default_models ()
{
}

void sim_mob::conf::default_models_pimpl::model (const std::pair<std::string, std::string>& value)
{
	//NOTE: The "prototype" field is initialized later, in SimMobility_Imp's post() method.
	//      It could be done here, but I like putting all "construction" after parsing, when possible.
	sim_mob::DefaultModels& defaults = config->system().defaultModels;
	if (value.first=="lane_changing") {
		defaults.laneChange.name = value.second;
	} else if (value.first=="car_following") {
		defaults.carFollow.name = value.second;
	} else if (value.first=="intersection_driving") {
		defaults.intDriving.name = value.second;
	} else if (value.first=="sidewalk_movement") {
		//defaults.sidewalkMove.name = value.second; //Later.
	} else {
		throw std::runtime_error("Unknown default model type.");
	}
}













