/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <string>
#include <vector>
#include <stdexcept>

#include "util/LangHelpers.hpp"
#include "util/DailyTime.hpp"

namespace sim_mob {


/**
 * A granularity, which can be represented as "x hours", "x minutes", and so forth.
 * The ms() function returns the most often-accessed format; we may choose to add
 * hours(), minutes(), and so forth later.
 */
class Granularity {
public:
	Granularity(int amount=0, const std::string& units="ms");
	int ms() { return ms_; }

private:
	int ms_;
};


/**
 * A loader for either the database or XML. Kind of a hack for now, since we use value-types in
 *   the loaders vector.
 */
class DbOrXmlLoader {
public:
	static DbOrXmlLoader MakeXmlLoader(const std::string& file, const std::string& rootElement) {
		return DbOrXmlLoader(true, std::make_pair(file, rootElement));
	}
	static DbOrXmlLoader MakeDbLoader(const std::string& connection, const std::string& mappings) {
		return DbOrXmlLoader(false, std::make_pair(connection, mappings));
	}

	std::string getXmlFile() {
		if (!isXml) { throw std::runtime_error("Not XmlLoader"); }
		return props.first;
	}
	std::string getXmlRoot() {
		if (!isXml) { throw std::runtime_error("Not XmlLoader"); }
		return props.second;
	}

	std::string getDbConnection() {
		if (isXml) { throw std::runtime_error("Not DbLoader"); }
		return props.first;
	}
	std::string getDbMappings() {
		if (isXml) { throw std::runtime_error("Not DbLoader"); }
		return props.first;
	}

private:
	DbOrXmlLoader(bool isXml, std::pair<std::string,std::string> props) : isXml(isXml), props(props) {}
	std::pair<std::string,std::string> props;
	bool isXml;
};




/**
 * Collection of various Simulation-level settings.
 */
struct Simulation {
	Simulation() : leadingVehReactTime(nullptr), subjectVehReactTime(nullptr), vehicleGapReactTime(nullptr) {}

	Granularity baseGranularity;
	Granularity totalRuntime;
	Granularity totalWarmup;
	DailyTime startTime;

	Granularity agentGranularity;
	Granularity signalGranularity;

	ReactionTimeDist* leadingVehReactTime;
	ReactionTimeDist* subjectVehReactTime;
	ReactionTimeDist* vehicleGapReactTime;

	std::vector<DbOrXmlLoader> roadNetworkLoaders;

	//TODO: Agent loaders.

};

}
