#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <sstream>
#include <stdexcept>
#include <iostream>

#include "entities/models/CarFollowModel.hpp"
#include "entities/models/LaneChangeModel.hpp"
#include "entities/models/IntersectionDrivingModel.hpp"

using std::string;
using std::pair;

void sim_mob::conf::models_pimpl::pre ()
{
	//Only necessary if we want to "re-load" the same model.
	constructs->carFollowModels.clear();
	constructs->dbConnections.clear();
	constructs->intDriveModels.clear();
	constructs->laneChangeModels.clear();
	constructs->storedProcedureMaps.clear();
}

void sim_mob::conf::models_pimpl::post_models ()
{
}

void sim_mob::conf::models_pimpl::lane_changing (const pair<string, string>& value)
{
	if (value.second != "built-in") {
		throw std::runtime_error("Only built-in models supported.");
	}
	if (value.first=="mitsim") {
		//TODO: Need to import MITSIM_LC_Model correctly.
		constructs->laneChangeModels[value.first] = new sim_mob::MITSIM_LC_Model();
	} else {
		std::stringstream msg;
		msg <<"Unknown lane changing model: " <<value.first <<" of type: " <<value.second;
		throw std::runtime_error(msg.str().c_str());
	}
}

void sim_mob::conf::models_pimpl::car_following (const pair<string, string>& value)
{
	if (value.second != "built-in") {
		throw std::runtime_error("Only built-in models supported.");
	}
	if (value.first=="mitsim") {
		//TODO: Need to import MITSIM_LC_Model correctly.
		constructs->carFollowModels[value.first] = new sim_mob::MITSIM_CF_Model();
	} else {
		std::stringstream msg;
		msg <<"Unknown car following model: " <<value.first <<" of type: " <<value.second;
		throw std::runtime_error(msg.str().c_str());
	}
}

void sim_mob::conf::models_pimpl::intersection_driving (const pair<string, string>& value)
{
	if (value.second != "built-in") {
		throw std::runtime_error("Only built-in models supported.");
	}
	if (value.first=="linear") {
		//TODO: Need to import MITSIM_LC_Model correctly.
		constructs->intDriveModels[value.first] = new sim_mob::SimpleIntDrivingModel();
	} else {
		std::stringstream msg;
		msg <<"Unknown intersection driving model: " <<value.first <<" of type: " <<value.second;
		throw std::runtime_error(msg.str().c_str());
	}
}

void sim_mob::conf::models_pimpl::sidewalk_movement (const pair<string, string>& value)
{
	//Later.
	/*if (value.second != "built-in") {
		throw std::runtime_error("Only built-in models supported.");
	}
	constructs->sidewalkMovement[value.first] = new SidewalkMovementModel();*/
}



