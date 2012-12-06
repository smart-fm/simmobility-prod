/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <string>

namespace sim_mob {

//Forward declarations
class CarFollowModel;
class LaneChangeModel;
class IntersectionDrivingModel;


/**
 * Simple constructs. These should be moved to their own files in locations relevant to their
 * function; use forward declarations here to refer to them.
 */
class Password {
public:
	Password(const std::string& rawPwd="") : rawPwd(rawPwd) {}

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
	DatabaseConnection(const std::string& id) : Identifiable(id) {}

	std::string host;
	std::string port;
	std::string dbName;
	std::string user;
	Password password;
};
class StoredProcedureMap : public Identifiable {
	StoredProcedureMap(const std::string& id) : Identifiable(id) {}

	std::string dbFormat; //Usually "aimsun"
	std::map<std::string, std::string> procedureMappings; //key=>value
};


/**
 * Collection of various items "construct"ed from the config file.
 */
struct Constructs {
	//Models
	std::map<std::string, CarFollowModel*> carFollowModels;
	std::map<std::string, LaneChangeModel*> laneChangeModels;
	std::map<std::string, IntersectionDrivingModel*> intDriveModels;
	//std::map<std::string, SidewalkMovementModel*> sidewalkMoveModels; //Later.

	//WorkGroups

	//Distributions

	//Database Connections
	std::map<std::string, DatabaseConnection*> dbConnections;
	std::map<std::string, StoredProcedureMap*> storedProcedureMaps;
};

}
