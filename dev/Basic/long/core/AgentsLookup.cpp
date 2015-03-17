/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   AgentsLookup.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Feb 18, 2014, 1:32 PM
 */

#include "AgentsLookup.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
namespace
{
    /**
     * Gets object for the given map testing if the key exists or not.
     * @param map should be a map like map<K, T*> or map<K, const T*>.
     * @param key to search
     * @return object pointer or nullptr if key does not exists.
     */
    template <typename T, typename M, typename K>
    inline const T* getById(const M& map, const K& key)
    {
        typename M::const_iterator itr = map.find(key);
        if (itr != map.end())
        {
            return (*itr).second;
        }
        return nullptr;
    }
}

AgentsLookup::AgentsLookup() {}

AgentsLookup::~AgentsLookup()
{
    reset();
}

void AgentsLookup::reset()
{
    householdAgentsById.clear();
    developerAgentsById.clear();
    realEstateAgentsById.clear();
}

void AgentsLookup::addHouseholdAgent(const HouseholdAgent* agent)
{
    if (agent && !getById<HouseholdAgent>(householdAgentsById, agent->GetId()))
    {
        householdAgentsById.insert(std::make_pair(agent->GetId(), agent));
    }
}

void AgentsLookup::addDeveloperAgent(const DeveloperAgent* agent)
{
    if (agent && !getById<DeveloperAgent>(developerAgentsById, agent->GetId()))
    {
        developerAgentsById.insert(std::make_pair(agent->GetId(), agent));
    }
}

void AgentsLookup::addRealEstateAgent(const RealEstateAgent* agent)
{
    if (agent && !getById<RealEstateAgent>(realEstateAgentsById, agent->GetId()))
    {
        realEstateAgentsById.insert(std::make_pair(agent->GetId(), agent));
    }
}

const HouseholdAgent* AgentsLookup::getHouseholdAgentById(const BigSerial id) const
{
    return getById<HouseholdAgent>(householdAgentsById, id);
}

const DeveloperAgent* AgentsLookup::getDeveloperAgentById(const BigSerial id) const
{
    return getById<DeveloperAgent>(developerAgentsById, id);
}

const RealEstateAgent* AgentsLookup::getRealEstateAgentById(const BigSerial id) const
{
    return getById<RealEstateAgent>(realEstateAgentsById, id);
}


LoggerAgent& AgentsLookup::getLogger()
{
    return logger;
}

EventsInjector& AgentsLookup::getEventsInjector()
{
    return injector;
}
