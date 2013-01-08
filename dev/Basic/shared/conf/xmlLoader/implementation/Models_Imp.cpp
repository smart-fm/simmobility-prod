#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <sstream>
#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;
using std::map;

void sim_mob::conf::models_pimpl::pre ()
{
	//Only necessary if we want to "re-load" the same model.
	config->constructs().carFollowModels.clear();
	config->constructs().dbConnections.clear();
	config->constructs().intDriveModels.clear();
	config->constructs().laneChangeModels.clear();
	config->constructs().storedProcedureMaps.clear();
}

void sim_mob::conf::models_pimpl::post_models ()
{
}

void sim_mob::conf::models_pimpl::lane_changing (const pair<string, string>& value)
{
	if (value.second != "built-in") {
		throw std::runtime_error("Only built-in models supported.");
	}

	map<string, sim_mob::LaneChangeModel*>::iterator it = config->built_in_models.laneChangeModels.find(value.first);
	if (it!=config->built_in_models.laneChangeModels.end()) {
		//TODO: For now we copy the pointer; we'll need to either use "clone()" or use a Factory class later.
		config->constructs().laneChangeModels[value.first] = it->second;
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

	map<string, sim_mob::CarFollowModel*>::iterator it = config->built_in_models.carFollowModels.find(value.first);
	if (it!=config->built_in_models.carFollowModels.end()) {
		//TODO: For now we copy the pointer; we'll need to either use "clone()" or use a Factory class later.
		config->constructs().carFollowModels[value.first] = it->second;
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

	map<string, sim_mob::IntersectionDrivingModel*>::iterator it = config->built_in_models.intDrivingModels.find(value.first);
	if (it!=config->built_in_models.intDrivingModels.end()) {
		//TODO: For now we copy the pointer; we'll need to either use "clone()" or use a Factory class later.
		config->constructs().intDriveModels[value.first] = it->second;
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



