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
 * Simple constructs. These should be moved to their own files in locations relevant to their
 * function; use forward declarations here to refer to them.
 */
class Password {
public:
	explicit Password(const std::string& rawPwd="") : rawPwd(rawPwd) {}

	//Provide a small amount of protection on deletion
	~Password() {
		size_t sz = rawPwd.length();
		for (size_t i=0; i<sz; i++) { rawPwd[i] = '\0'; }
	}

	///Returns masked version by default. Try not to use the masked version when possible.
	std::string toString(bool mask=true) {
		if (!mask) { return rawPwd; }

		return std::string(rawPwd.length(), '*');
	}

private:
	std::string rawPwd;
};
class Identifiable {
public:
	Identifiable(const std::string& id) : id(id) {}
	std::string getId() { return id; }

private:
	std::string id;
};
class DatabaseConnection : public Identifiable {
public:
	DatabaseConnection(const std::string& id="") : Identifiable(id) {}

	std::string host;
	std::string port;
	std::string dbName;
	std::string user;
	Password password;
};
class StoredProcedureMap : public Identifiable {
public:
	StoredProcedureMap(const std::string& id="") : Identifiable(id) {}

	std::string dbFormat; //Usually "aimsun"
	std::map<std::string, std::string> procedureMappings; //key=>value
};


/**
 * WorkGroups require lazy initialization for multiple reasons.
 */
class WorkGroupFactory {
public:
	WorkGroupFactory(int numWorkers=0, bool agentWG=false, bool signalWG=false) : item(nullptr), numWorkers(numWorkers), agentWG(agentWG), signalWG(signalWG) {}

	sim_mob::WorkGroup* getItem();
private:
	sim_mob::WorkGroup* item;
	int numWorkers;
	bool agentWG;
	bool signalWG;
};


/**
 * Collection of various items "construct"ed from the config file.
 */
/*struct Constructs {
	//Models
	std::map<std::string, CarFollowModel*> carFollowModels;
	std::map<std::string, LaneChangeModel*> laneChangeModels;
	std::map<std::string, IntersectionDrivingModel*> intDriveModels;
	//std::map<std::string, SidewalkMovementModel*> sidewalkMoveModels; //Later.

	//WorkGroups
	std::map<std::string, WorkGroupFactory> workGroups;

	//Distributions
	std::map<std::string, sim_mob::ReactionTimeDist*> distributions;

	//Database Connections
	std::map<std::string, DatabaseConnection> dbConnections;
	std::map<std::string, StoredProcedureMap> storedProcedureMaps;
};*/

}
