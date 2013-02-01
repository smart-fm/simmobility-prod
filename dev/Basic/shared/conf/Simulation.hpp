/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <list>
#include <string>
#include <vector>
#include <stdexcept>

#include "geospatial/Point2D.hpp"

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
	int ms() const { return ms_; }

private:
	int ms_;
};


/**
 * A generic "loader" class. Can be used for loading a variety of data (RoadNetworks, Agents, etc.)
 * Sub-classes will specify the actual loading to be done.
 */
class DataLoader {
public:
	virtual ~DataLoader() {}

	//TODO: Later, once we know how loadData should work.
	//virtual void loadData() = 0;
};


/**
 * Loader for Database-based data.
 */
class DbLoader : public DataLoader {
public:
	DbLoader(const std::string& connection, const std::string& mappings) : connection(connection), mappings(mappings) {}
protected:
	std::string connection;
	std::string mappings;
};

/**
 * Loader for Xml-based data.
 */
class XmlLoader : public DataLoader {
public:
	XmlLoader(const std::string& fileName, const std::string& rootNode) : fileName(fileName), rootNode(rootNode) {}
protected:
	std::string fileName;
	std::string rootNode;
};


/**
 * Load a series of manual Agent specifications
 */
template <class AgentSpec>
class AgentLoader : public DataLoader {
public:
	void addAgentSpec(const AgentSpec& ags) {
		agents.push_back(ags);
	}

protected:
	std::vector<AgentSpec> agents;
};


/**
 * Specification for explicit Driver agents.
 */
struct DriverSpec {
	DriverSpec() : startTimeMs(0) {}

	sim_mob::Point2D origin;
	sim_mob::Point2D dest;
	uint32_t startTimeMs; //Frames are converted to ms
	std::map<std::string, std::string> properties;  //Customized properties, to be used later.
};

/**
 * Specification for explicit Pedestrian agents.
 */
struct PedestrianSpec {
	sim_mob::Point2D origin;
	sim_mob::Point2D dest;
	uint32_t startTimeMs; //Frames are converted to ms
	std::map<std::string, std::string> properties;  //Customized properties, to be used later.
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

	//Loader for all items inside the "road_network" tag. For now, only the first is used.
	//This array is cleared (and its items are deleted) after the config file has been processed.
	std::list<DataLoader*> roadNetworkLoaders;

	//Loader for all items inside the "agents" tag.
	//This array is cleared (and its items are deleted) after the config file has been processed.
	std::list<DataLoader*> agentsLoaders;
};

}
