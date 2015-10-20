//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ConfigParams.hpp"

#include "conf/ParseConfigFile.hpp"
#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "entities/misc/PublicTransit.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/RoadSegment.hpp"
#include "network/CommunicationDataManager.hpp"
#include "network/ControlManager.hpp"
#include "password/password.hpp"
#include "util/ReactionTimeDistributions.hpp"
#include "util/PassengerDistribution.hpp"

using namespace sim_mob;

sim_mob::ConfigParams::ConfigParams() : RawConfigParams(),
    totalRuntimeTicks(0), totalWarmupTicks(0), numAgentsSkipped(0),
    using_MPI(false), outNetworkFileName("out.network.txt"),
    is_simulation_repeatable(false), sealedNetwork(false), controlMgr(nullptr),
    workerPublisherEnabled(false)
{}

sim_mob::ConfigParams::~ConfigParams()
{
    ///Delete all pointers
//	safe_delete_item(commDataMgr);
	safe_delete_item(controlMgr);

    clear_delete_map(busStopNo_busStops);
    clear_delete_map_with_vector(tripchains);
}

sim_mob::Factory<sim_mob::Broker>& sim_mob::ConfigParams::getBrokerFactoryRW()
{
	return brokerFact;
}

std::string sim_mob::ConfigParams::getDatabaseConnectionString(bool maskPassword) const
{
    ///The database.
	std::string dbKey = networkDatabase.database;
	std::map<std::string, Database>::const_iterator dbIt = constructs.databases.find(dbKey);
    if (dbIt==constructs.databases.end())
    {
		throw std::runtime_error("Couldn't find default database.");
	}

    ///The credentials
	std::string credKey = networkDatabase.credentials;
	std::map<std::string, Credential>::const_iterator credIt = constructs.credentials.find(credKey);
    if (credIt==constructs.credentials.end())
    {
		Print() << "trying to find " << credKey << " among:" << std::endl;
		std::map<std::string, Credential>::const_iterator it;
		for( it = constructs.credentials.begin(); it != constructs.credentials.end() ; ++it)
		{
			Print() << it->first << std::endl;
		}
		throw std::runtime_error("Couldn't find default credentials..");
	}

    ///Now build the string.
	std::stringstream res;

	res <<"host="   <<dbIt->second.host   <<" "
		<<"port="   <<dbIt->second.port   <<" "
		<<"dbname=" <<dbIt->second.dbName <<" "
		<<"user="   <<credIt->second.getUsername()   <<" "
		<<"password=" <<credIt->second.getPassword(maskPassword);
	return res.str();
}

StoredProcedureMap sim_mob::ConfigParams::getDatabaseProcMappings() const
{
	std::string key = networkDatabase.procedures;
	std::map<std::string, StoredProcedureMap>::const_iterator it = procedureMaps.find(key);
    if (it==procedureMaps.end())
    {
		throw std::runtime_error("Couldn't find stored procedure key.");
	}
	return it->second;
}


//sim_mob::CommunicationDataManager&  sim_mob::ConfigParams::getCommDataMgr() const
//{
//    if (!InteractiveMode())
//    {
//		throw std::runtime_error("ConfigParams::getCommDataMgr() not supported; SIMMOB_INTERACTIVE_MODE is off.");
//	}
//    /*if (!commDataMgr)
//    {
//		commDataMgr = new CommunicationDataManager();
//	}
//	return *commDataMgr;*/
//}

sim_mob::ControlManager* sim_mob::ConfigParams::getControlMgr() const
{
    if (!InteractiveMode())
    {
		throw std::runtime_error("ConfigParams::getControlMgr() not supported; SIMMOB_INTERACTIVE_MODE is off.");
	}
    ///In this case, ControlManager's constructor performs some logic, so it's best to use a pointer.
    if (!controlMgr)
    {
		controlMgr = new ControlManager();
	}
	return controlMgr;
}


void sim_mob::ConfigParams::sealNetwork()
{
	sealedNetwork = true;
}


const sim_mob::RoadNetwork& sim_mob::ConfigParams::getNetwork() const
{
	return network;
}


sim_mob::RoadNetwork& sim_mob::ConfigParams::getNetworkRW()
{
    if (sealedNetwork)
    {
        throw std::runtime_error("getNetworkRW() failed; network has been sealed.");
	}
	return network;
}

////////////////////////////////////////////////////////////////////////////
/// Getters/setters
////////////////////////////////////////////////////////////////////////////

unsigned int& sim_mob::ConfigParams::baseGranMS()
{
    return simulation.baseGranMS;
}
const unsigned int& sim_mob::ConfigParams::baseGranMS() const
{
    return simulation.baseGranMS;
}

const double& sim_mob::ConfigParams::baseGranSecond() const
{
    return simulation.baseGranSecond;
}

bool& sim_mob::ConfigParams::isMergeLogFiles()
{
    return mergeLogFiles;
}
const bool sim_mob::ConfigParams::isMergeLogFiles() const
{
    return mergeLogFiles;
}

WorkGroup::ASSIGNMENT_STRATEGY& sim_mob::ConfigParams::defaultWrkGrpAssignment()
{
    return simulation.workGroupAssigmentStrategy;
}
const WorkGroup::ASSIGNMENT_STRATEGY& sim_mob::ConfigParams::defaultWrkGrpAssignment() const
{
    return simulation.workGroupAssigmentStrategy;
}

sim_mob::MutexStrategy& sim_mob::ConfigParams::mutexStategy()
{
    return simulation.mutexStategy;
}
const sim_mob::MutexStrategy& sim_mob::ConfigParams::mutexStategy() const
{
    return simulation.mutexStategy;
}

DailyTime& sim_mob::ConfigParams::simStartTime()
{
    return simulation.simStartTime;
}
const DailyTime& sim_mob::ConfigParams::simStartTime() const
{
    return simulation.simStartTime;
}

const std::string& sim_mob::ConfigParams::getRTTT() const
{
	return pathset.RTTT_Conf;
}

const std::string& sim_mob::ConfigParams::getDTT() const
{
	return pathset.DTT_Conf;
}

unsigned int sim_mob::ConfigParams::totalRuntimeInMilliSeconds() const
{
    return simulation.totalRuntimeMS;
}

unsigned int sim_mob::ConfigParams::warmupTimeInMilliSeconds() const
{
    return simulation.totalWarmupMS;
}

std::map<std::string, std::vector<sim_mob::TripChainItem*> >& sim_mob::ConfigParams::getTripChains()
{
	return tripchains;
}

const std::map<std::string, std::vector<sim_mob::TripChainItem*> >& sim_mob::ConfigParams::getTripChains() const
{
	return tripchains;
}

std::vector<sim_mob::PT_BusDispatchFreq>& sim_mob::ConfigParams::getPT_BusDispatchFreq()
{
	return ptBusDispatchFreq;
}

const std::vector<sim_mob::PT_BusDispatchFreq>& sim_mob::ConfigParams::getPT_BusDispatchFreq() const
{
	return ptBusDispatchFreq;
}

std::vector<sim_mob::PT_BusRoutes>& sim_mob::ConfigParams::getPT_BusRoutes()
{
	return ptBusRoutes;
}

std::vector<sim_mob::PT_BusStops>& sim_mob::ConfigParams::getPT_BusStops()
{
	return ptBusStops;
}

const std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& sim_mob::ConfigParams::getRoadSegments_Map() const
{
	return routeID_roadSegments;
}

std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& sim_mob::ConfigParams::getRoadSegments_Map()
{
	return routeID_roadSegments;
}

std::map<std::string, sim_mob::BusStop*>& sim_mob::ConfigParams::getBusStopNo_BusStops()
{
	return busStopNo_busStops;
}

const std::map<std::string, sim_mob::BusStop*>& sim_mob::ConfigParams::getBusStopNo_BusStops() const
{
	return busStopNo_busStops;
}

std::map<std::string, std::vector<const sim_mob::BusStop*> >& sim_mob::ConfigParams::getBusStops_Map()
{
	return routeID_busStops;
}

const std::map<std::string, std::vector<const sim_mob::BusStop*> >& sim_mob::ConfigParams::getBusStops_Map() const
{
	return routeID_busStops;
}

bool sim_mob::ConfigParams::isGenerateBusRoutes() const
{
    return generateBusRoutes;
}

/// use pathset to generate path of driver
bool sim_mob::ConfigParams::PathSetMode() const
{
	return pathset.enabled;
}

PathSetConf& sim_mob::ConfigParams::getPathSetConf()
{
    return pathset;
}

const PathSetConf& sim_mob::ConfigParams::getPathSetConf() const
{
    return pathset;
}

bool ConfigParams::RunningMidTerm() const
{
    return (simMobRunMode == MID_TERM);
}

bool ConfigParams::RunningShortTerm() const
{
    return (simMobRunMode == SHORT_TERM);
}

bool ConfigParams::RunningLongTerm() const
{
    return (simMobRunMode == LONG_TERM);
}

const ModelScriptsMap& sim_mob::ConfigParams::getLuaScriptsMap() const
{
	return luaScriptsMap;
}

void ConfigParams::setWorkerPublisherEnabled(bool value)
{
    workerPublisherEnabled = value;
}

bool ConfigParams::isWorkerPublisherEnabled() const
{
    return workerPublisherEnabled;
}
