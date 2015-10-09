//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/noncopyable.hpp>
#include <set>

#include "ST_Config.hpp"
#include "conf/ExpandAndValidateConfigFile.hpp"

namespace sim_mob
{

class StartTimePriorityQueue;

class ExpandShortTermConfigFile : public ExpandAndValidateConfigFile
{
public:
    ExpandShortTermConfigFile(ST_Config& stConfig, ConfigParams& cfg,
				      std::set<sim_mob::Entity*>& active_agents,
				      StartTimePriorityQueue& pending_agents);
private:
    void processConfig();

    //These functions are called by ProcessConfig()
    void CheckGranularities();
    void SetTicks();
    bool SetTickFromBaseGran(unsigned int& res, unsigned int tickLenMs);
    void loadNetworkFromDatabase();

    void loadFMOD_Controller();
    void loadAMOD_Controller();
    void loadAgentsInOrder(ConfigParams::AgentConstraints& constraints);
    void generateAgentsFromTripChain(ConfigParams::AgentConstraints& constraints);
    void generateXMLAgents(const std::vector<EntityTemplate>& xmlItems,
			   const std::string& roleName,
			   ConfigParams::AgentConstraints& constraints);
    void generateXMLSignals();

    void PrintSettings();

    ST_Config& stConfig;

    ConfigParams& cfg;

    //Our active/pending agent lists.
    std::set<sim_mob::Entity*>& active_agents;

    StartTimePriorityQueue& pending_agents;
};

}
