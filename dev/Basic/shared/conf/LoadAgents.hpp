#if 0
/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <boost/noncopyable.hpp>

#include <map>
#include <set>
#include <list>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>


namespace sim_mob {

//Forward declarations
class Config;
class StartTimePriorityQueue;
class Agent;
class Entity;


/**
 * Class used to validate, load, and initialize all Agents.
 * Typically used like a verb:
 *     Config cfg = //load cfg somehow
 *     LoadAgents load(cfg);
 */
class LoadAgents : private boost::noncopyable {
public:
	///Load all Agents for a given Config file.
	LoadAgents(sim_mob::Config& cfg, std::vector<sim_mob::Entity*>& active_agents, sim_mob::StartTimePriorityQueue& pending_agents);

	///Used to ensure that all Agent IDs are unique and < startingAutoAgentID.
	struct AgentConstraints {
		AgentConstraints() : startingAutoAgentID(0) {}

		int startingAutoAgentID;
		std::set<unsigned int> manualAgentIDs;

		void validateID(unsigned int manualID) {
			//Simple constraint check.
			if (manualID>=startingAutoAgentID) {
				throw std::runtime_error("Manual ID must be within the bounds specified in the config file.");
			}

			//Ensure agents are created with unique IDs
			if (manualAgentIDs.count(manualID)>0) {
				std::stringstream msg;
				msg <<"Duplicate manual ID: " <<manualID;
				throw std::runtime_error(msg.str().c_str());
			}

			//Mark it, save it
			manualAgentIDs.insert(manualID);
		}
	};

protected:
	///Create empty copied of each (Person) agent. These will be filled in once they are fully dispatched.
	void CreateAgentShells(AgentConstraints& constraints, std::list<sim_mob::Agent*>& res_agents);

	///Send all Agents into the simulation.
	void DispatchOrPend(const std::list<sim_mob::Agent*>& res_agents, std::vector<sim_mob::Entity*>& active_agents, sim_mob::StartTimePriorityQueue& pending_agents);

private:
	//Internal helper functions


private:
	//The config file we are currently using as a reference.
	Config& cfg;
};

}
#endif
