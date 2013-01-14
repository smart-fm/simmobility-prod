/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <string>
#include <stdexcept>

#include "util/LangHelpers.hpp"

namespace sim_mob {

//Forward declarations
class CarFollowModel;
class LaneChangeModel;
class IntersectionDrivingModel;
class WorkGroup;


/**
 * A default model mapping
 */
template <class PrototypeClass>
struct Mapping {
	Mapping() : prototype(nullptr) {}

	std::string name;
	PrototypeClass* prototype;
};


/**
 * Default models
 */
struct DefaultModels {
	Mapping<sim_mob::CarFollowModel> carFollow;
	Mapping<sim_mob::LaneChangeModel> laneChange;
	Mapping<sim_mob::IntersectionDrivingModel> intDriving;
	//Mapping<sim_mob::SidwalkMovement> sidewalkMove; //Later
};


/**
 * Default workgroups
 */
struct DefaultWorkGroups {
	Mapping<WorkGroup> agentWG;
	Mapping<WorkGroup> signalWG;
};


/**
 * Collection of various System-level settings.
 */
struct System {
	DefaultModels defaultModels;
	DefaultWorkGroups defaultWorkGroups;

	std::map<std::string, std::string> genericProps;
};

}
