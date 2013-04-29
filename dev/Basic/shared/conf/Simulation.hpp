/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <list>
#include <string>
#include <vector>
#include <stdexcept>

#include "geospatial/Point2D.hpp"

#include "conf/LoadAgents.hpp"
#include "entities/roles/RoleFactory.hpp"

#include "util/LangHelpers.hpp"
#include "util/DailyTime.hpp"

namespace sim_mob {

//Forward declarations
class Config;


/**
 * A granularity, which can be represented as "x hours", "x minutes", and so forth,
 * and may also have a base granularity (used to compute "ticks").
 * The ms() function returns the most often-accessed format; we may choose to add
 * hours(), minutes(), and so forth later.
 */
class Granularity {
public:
	Granularity(int amount=0, const std::string& units="ms");

	int ms() const { return ms_; }

	///Return the number of ticks this granularity represents. Fails if baseGranMS is 0.
	int ticks() const;

	//Update the BaseGran.
	void setBaseGranMS(int baseGranMS);

private:
	int ms_;
	int base_gran_ms_;
};


/**
 * A generic "loader" class. Can be used for loading a variety of data (RoadNetworks, Agents, etc.)
 * Sub-classes will specify the actual loading to be done.
 */
class AbstractAgentLoader {
public:
	virtual ~AbstractAgentLoader() {}

	///Create agent "shells" and store them into a resulting list.
	///Overrides of this function should use the AgentConstraints to check any manual IDs.
	virtual void loadAgents(std::list<sim_mob::Agent*>& res, LoadAgents::AgentConstraints& constraints, const sim_mob::Config& cfg) = 0;
};


/**
 * Loader for Database-based Agents.
 */
class DatabaseAgentLoader : public AbstractAgentLoader {
public:
	DatabaseAgentLoader(const std::string& connection, const std::string& mappings) : connection(connection), mappings(mappings) {}

	virtual void loadAgents(std::list<sim_mob::Agent*>& res, LoadAgents::AgentConstraints& constraints, const sim_mob::Config& cfg);
protected:
	std::string connection;
	std::string mappings;
};

/**
 * Loader for Xml-based Agents.
 */
class XmlAgentLoader : public AbstractAgentLoader {
public:
	XmlAgentLoader(const std::string& fileName, const std::string& rootNode) : fileName(fileName), rootNode(rootNode) {}

	virtual void loadAgents(std::list<sim_mob::Agent*>& res, LoadAgents::AgentConstraints& constraints, const sim_mob::Config& cfg);
protected:
	std::string fileName;
	std::string rootNode;
};



///A class used to specify Agents that should be loaded.
///Uses a template parameter to provide detailed parameters for a specific subclass,
///as well as specializations for loading that particular class (in the future)
///
///NOTE: This used to be a templatized class, but:
///      1) The use of "agentType" made it pointless.
///      2) The fact that it used "Conf" so much meant we had to put the implementation into
///         a separate file... so templates just got in the way.
///
struct AgentSpec {
	//Agents are still constructed based on their "agentType" for now. We can clean this up later; for now, just
	// make sure that you are constructing AgentSpecs with the correct agentType strings.
	AgentSpec(const std::string& agentType="ERR") : id(-1), agentType(agentType), startTimeMs(0) {}

	int32_t id; //Set to a positive number to "force" that id.
	std::string agentType; //Mirrors the old "agent type" in simpleconf.
	uint32_t startTimeMs; //Frames are converted to ms
	std::map<std::string, std::string> properties;  //Customized properties, to be used later.
	sim_mob::Point2D origin;
	sim_mob::Point2D dest;
};




/**
 * Load a series of manual Agent specifications
 */
class AgentLoader : public AbstractAgentLoader {
public:
	void addAgentSpec(const AgentSpec& ags);

	virtual void loadAgents(std::list<sim_mob::Agent*>& res, LoadAgents::AgentConstraints& constraints, const sim_mob::Config& cfg);

protected:
	std::vector< AgentSpec > agents;
};


/**
 * Specification for explicit Driver agents.
 */
/*struct DriverSpec {
	sim_mob::Point2D origin;
	sim_mob::Point2D dest;

};*/

/**
 * Specification for explicit Pedestrian agents.
 */
/*struct PedestrianSpec {
	sim_mob::Point2D origin;
	sim_mob::Point2D dest;
};*/




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
	std::list<AbstractAgentLoader*> roadNetworkLoaders;

	//Loader for all items inside the "agents" tag.
	//This array is cleared (and its items are deleted) after the config file has been processed.
	std::list<AbstractAgentLoader*> agentsLoaders;
};

}
