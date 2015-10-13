#pragma once

#include <boost/noncopyable.hpp>

#include "MT_Config.hpp"

namespace sim_mob
{

namespace medium
{

class ExpandMidTermConfigFile : public boost::noncopyable
{
public:
    ExpandMidTermConfigFile(MT_Config& mtCfg, ConfigParams& cfg,
    						std::set<sim_mob::Entity*>& active_agents,
    						StartTimePriorityQueue& pending_agents);

private:
    void processConfig();

    //These functions are called by ProcessConfig()
    void checkGranularities();
    void setTicks();
    bool setTickFromBaseGran(unsigned int& res, unsigned int tickLenMs);

    void printSettings();

    void loadNetworkFromDatabase();

    void loadPublicTransitNetworkFromDatabase();

    void WarnMidroadSidewalks();

    void verifyIncidents();

    void setRestrictedRegionSupport();

    MT_Config& mtCfg;

    //The config file we are currently loading
    ConfigParams& cfg;

    //Our active/pending agent lists.
    std::set<sim_mob::Entity*>& active_agents;

    StartTimePriorityQueue& pending_agents;
};

}

}
