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
#include "geospatial/network/PT_Stop.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "network/CommunicationDataManager.hpp"
#include "network/ControlManager.hpp"
#include "password/password.hpp"
#include "util/ReactionTimeDistributions.hpp"
#include "util/PassengerDistribution.hpp"

using namespace sim_mob;

sim_mob::ConfigParams::ConfigParams() : RawConfigParams(),
	publicTransitEnabled(false), totalRuntimeTicks(0), totalWarmupTicks(0), numAgentsSkipped(0),
    using_MPI(false), outNetworkFileName("out.network.txt"),outTrainNetworkFilename("out.train.network.txt"),outSimInfoFileName("out.siminfo.txt"),
    is_simulation_repeatable(false), sealedNetwork(false), controlMgr(nullptr),
    workerPublisherEnabled(false), enabledEdgeTravelTime(false)
{}

sim_mob::ConfigParams::~ConfigParams()
{
	///Delete all pointers
//	safe_delete_item(commDataMgr);
	safe_delete_item(controlMgr);

	clear_delete_map(busStopNo_busStops);
	safe_delete_item(simulation.closedLoop.logger);
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

std::map<std::string, sim_mob::BusStop*>& sim_mob::ConfigParams::getBusStopNo_BusStops()
{
	return busStopNo_busStops;
}

const std::map<std::string, sim_mob::BusStop*>& sim_mob::ConfigParams::getBusStopNo_BusStops() const
{
	return busStopNo_busStops;
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

bool ConfigParams::isPublicTransitEnabled() const{
    return publicTransitEnabled;
}

void ConfigParams::setPublicTransitEnabled(bool value)
{
	publicTransitEnabled = value;
}

bool ConfigParams::isEnabledEdgeTravelTime() const
{
	return enabledEdgeTravelTime;
}

void ConfigParams::setEnabledEdgeTravelTime(bool enabledEdgeTravelTime)
{
	this->enabledEdgeTravelTime = enabledEdgeTravelTime;
}

const std::string& ConfigParams::getJourneyTimeStatsFilename() const
{
	return journeyTimeStatsFilename;
}

const std::string& ConfigParams::getWaitingTimeStatsFilename() const
{
	return waitingTimeStatsFilename;
}

void ConfigParams::setJourneyTimeStatsFilename(const std::string& str)
{
	journeyTimeStatsFilename = str;
}

void ConfigParams::setWaitingTimeStatsFilename(const std::string& str)
{
	waitingTimeStatsFilename = str;
}

const std::string& ConfigParams::getWaitingCountStatsFilename() const
{
	return waitingCountStatsFilename;
}

void ConfigParams::setWaitingCountStatsFilename(const std::string& str)
{
	waitingCountStatsFilename = str;
}

unsigned int ConfigParams::getWaitingCountStatsInterval() const
{
	return waitingCountStatsStorageInterval;
}

void ConfigParams::setWaitingCountStatsInterval(unsigned int interval)
{
	waitingCountStatsStorageInterval = interval;
}

const std::string& ConfigParams::getTravelTimeStatsFilename() const
{
	return travelTimeStatsFilename;
}

void ConfigParams::setTravelTimeStatsFilename(const std::string& str)
{
	travelTimeStatsFilename = str;
}

const std::string& ConfigParams::getPT_StopStatsFilename() const
{
	return ptStopStatsFilename;
}

void ConfigParams::setPT_StopStatsFilename(const std::string& str)
{
	ptStopStatsFilename = str;
}

