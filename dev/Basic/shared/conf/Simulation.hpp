/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <list>
#include <string>
#include <vector>
#include <stdexcept>

#include "geospatial/Point2D.hpp"

#include "conf/LoadAgents.hpp"

#include "util/LangHelpers.hpp"
#include "util/DailyTime.hpp"

namespace sim_mob {


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
class DataLoader {
public:
	virtual ~DataLoader() {}

	///Optional: Check manually assigned IDs for all Agents. This is done in the "pre-load" phase, so
	///          certain Loader types (e.g., database, XML) cannot benefit from it and must
	///          wait for the "load" phase to validate IDs.
	virtual void checkManualIDs(LoadAgents::AgentConstraints& constraints) {}

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



///A class used to specify Agents that should be loaded.
///Uses a template parameter to provide detailed parameters for a specific subclass,
///as well as specializations for loading that particular class (in the future)
template <class Details>
struct AgentSpec {
	AgentSpec() : id(-1), startTimeMs(0) {}

	int32_t id; //Set to a positive number to "force" that id.
	uint32_t startTimeMs; //Frames are converted to ms
	std::map<std::string, std::string> properties;  //Customized properties, to be used later.
	Details specifics; //Put type-specific details here.
};




/**
 * Load a series of manual Agent specifications
 */
template <class Details>
class AgentLoader : public DataLoader {
public:
	void addAgentSpec(const AgentSpec<Details>& ags) {
		agents.push_back(ags);
	}

	virtual void checkManualIDs(LoadAgents::AgentConstraints& constraints) {
		for (typename std::vector< AgentSpec<Details> >::iterator it=agents.begin(); it!=agents.end(); it++) {
			//Agents can specify manual or automatic IDs. We only need to check manual IDs, since automatic IDs are handled
			//  by the Agent class.
			if (it->id >= 0) {
				//Manual ID
				constraints.validateID(it->id);
			}
		}
	}


protected:
	std::vector< AgentSpec<Details> > agents;
};


/**
 * Specification for explicit Driver agents.
 */
struct DriverSpec {
	sim_mob::Point2D origin;
	sim_mob::Point2D dest;

};

/**
 * Specification for explicit Pedestrian agents.
 */
struct PedestrianSpec {
	sim_mob::Point2D origin;
	sim_mob::Point2D dest;
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
