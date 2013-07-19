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
class ReactionTimeDist;


/**
 * Busline control strategies.
 */
enum BusControlStrategy {
	BusCtrl_None,
	BusCtrl_Schedule,
	BusCtrl_Headway,
	BusCtrl_EvenHeadway,
	BusCtrl_Hybrid
};



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
	System() : startingAgentAutoID(0), signalTimingMode(0), passengerPercentBoarding(100), passengerPercentAlighting(100),
			passengerBusStopDistribution(nullptr), busControlStrategy(BusCtrl_None)
	{}

	int startingAgentAutoID; //How to start counting.

	DefaultModels defaultModels;
	DefaultWorkGroups defaultWorkGroups;

	///Note: The "signalTimingMode" seemed to be missing from the config file (i.e., not used).
	///      If you need it back, just add it to "generic_props" in the new Config file.
	int signalTimingMode;

	///Percent of passengers boarding and alighting at each bus stop.
	/// TODO: This should *definitely* be moved out of the config file eventually.
	int passengerPercentBoarding;
	int passengerPercentAlighting;
	ReactionTimeDist* passengerBusStopDistribution;  //<NOTE: We call it "ReactionTimeDist", but it's really just a distribution.

	BusControlStrategy busControlStrategy;

	std::map<std::string, std::string> genericProps;
};

}
