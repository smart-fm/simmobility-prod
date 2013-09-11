/* Copyright Singapore-MIT Alliance for Research and Technology */
#pragma once

#include <map>
#include <string>
#include <stdexcept>

#include "util/LangHelpers.hpp"

namespace sim_mob {

class Identifiable {
public:
	Identifiable(const std::string& id) : id(id) {}
	std::string getId() { return id; }

private:
	std::string id;
};


class StoredProcedureMap : public Identifiable {
public:
	StoredProcedureMap(const std::string& id="") : Identifiable(id) {}

	std::string dbFormat; //Usually "aimsun"
	std::map<std::string, std::string> procedureMappings; //key=>value
};


}
