//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ConfigParams.hpp"

#include "conf/ParseConfigFile.hpp"
#include "conf/ExpandAndValidateConfigFile.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "entities/misc/BusSchedule.hpp"
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

//ConfigParams sim_mob::ConfigParams::instance;


sim_mob::ConfigParams::ConfigParams() : RawConfigParams(),
	totalRuntimeTicks(0), totalWarmupTicks(0), granPersonTicks(0), granSignalsTicks(0),
	granCommunicationTicks(0), reactDist1(nullptr), reactDist2(nullptr), numAgentsSkipped(0),
	using_MPI(false), is_run_on_many_computers(false), outNetworkFileName("out.network.txt"),
	is_simulation_repeatable(false), sealedNetwork(false), commDataMgr(nullptr), controlMgr(nullptr),
	passengerDist_busstop(nullptr), passengerDist_crowdness(nullptr)
{}


sim_mob::ConfigParams::~ConfigParams()
{
	safe_delete_item(reactDist1);
	safe_delete_item(reactDist2);
	safe_delete_item(passengerDist_busstop);
	safe_delete_item(passengerDist_crowdness);
	safe_delete_item(commDataMgr);
	safe_delete_item(controlMgr);

	//These might leak??
	//std::vector<sim_mob::BusSchedule*> busschedule;
	//std::vector<sim_mob::PT_trip*> pt_trip;
	//routeID_roadSegments
	//routeID_busStops
	//confluxes
}



/*void sim_mob::ConfigParams::reset()
{
	//TODO: This *should* work fine, but check the comment below.
	instance = sim_mob::ConfigParams();

	//TODO: This is the old code for reset(); I prefer the above code, as it is more generic, but I
	//      have not tested that it works.  ~Seth
	//sealedNetwork=false;
	//roleFact.clear();
}*/


/*const ConfigParams& sim_mob::ConfigParams::GetInstance()
{
	return ConfigParams::instance;
}


ConfigParams& sim_mob::ConfigParams::GetInstanceRW()
{
	return ConfigParams::instance;
}*/


const sim_mob::RoleFactory& sim_mob::ConfigParams::getRoleFactory() const
{
	return roleFact;
}


sim_mob::RoleFactory& sim_mob::ConfigParams::getRoleFactoryRW()
{
	return roleFact;
}


sim_mob::Factory<sim_mob::Broker>& sim_mob::ConfigParams::getBrokerFactoryRW()
{
	return brokerFact;
}


/*void sim_mob::ConfigParams::InitUserConf(const std::string& configPath, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents, ProfileBuilder* prof, const Config::BuiltInModels& builtInModels)
{
	//Load using our new config syntax.
	ParseConfigFile parse(configPath, ConfigParams::GetInstanceRW());
	ExpandAndValidateConfigFile expand(ConfigParams::GetInstanceRW(), active_agents, pending_agents);
}*/


std::string sim_mob::ConfigParams::getDatabaseConnectionString(bool maskPassword) const
{
	//The database.
	std::string dbKey = system.networkDatabase.database;
	std::map<std::string, Database>::const_iterator dbIt = constructs.databases.find(dbKey);
	if (dbIt==constructs.databases.end()) {
		throw std::runtime_error("Couldn't find default database.");
	}

	//The credentials
	std::string credKey = system.networkDatabase.credentials;
	std::map<std::string, Credential>::const_iterator credIt = constructs.credentials.find(credKey);
	if (credIt==constructs.credentials.end()) {
		Print() << "trying to find " << credKey << " among:" << std::endl;
		std::map<std::string, Credential>::const_iterator it;
		for( it = constructs.credentials.begin(); it != constructs.credentials.end() ; it++)
		{
			Print() << it->first << std::endl;
		}
		throw std::runtime_error("Couldn't find default credentials..");
	}

	//Now build the string.
	std::stringstream res;

	res <<"host="   <<dbIt->second.host   <<" "
		<<"port="   <<dbIt->second.port   <<" "
		<<"dbname=" <<dbIt->second.dbName <<" "
		<<"user="   <<credIt->second.getUsername()   <<" "
		<<"password=" <<credIt->second.getPassword(maskPassword);
//		<<"password=" <<(maskPassword?"***":sim_mob::simple_password::load(std::string())); //NOTE: This is in no way secure. ~Seth
	return res.str();
}

StoredProcedureMap sim_mob::ConfigParams::getDatabaseProcMappings() const
{
	std::string key = system.networkDatabase.procedures;
	std::map<std::string, StoredProcedureMap>::const_iterator it = constructs.procedureMaps.find(key);
	if (it==constructs.procedureMaps.end()) {
		throw std::runtime_error("Couldn't find stored procedure key.");
	}
	return it->second;
}


sim_mob::CommunicationDataManager&  sim_mob::ConfigParams::getCommDataMgr() const
{
	if (!InteractiveMode()) {
		throw std::runtime_error("ConfigParams::getCommDataMgr() not supported; SIMMOB_INTERACTIVE_MODE is off.");
	}
	if (!commDataMgr) {
		commDataMgr = new CommunicationDataManager();
	}
	return *commDataMgr;
}

sim_mob::ControlManager* sim_mob::ConfigParams::getControlMgr() const
{
	if (!InteractiveMode()) {
		throw std::runtime_error("ConfigParams::getControlMgr() not supported; SIMMOB_INTERACTIVE_MODE is off.");
	}
	//In this case, ControlManager's constructor performs some logic, so it's best to use a pointer.
	if (!controlMgr) {
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
	if (sealedNetwork) {
		throw std::runtime_error("getNetworkRW() failed; network has been sealed.");
	}
	return network;
}


std::vector<IncidentParams>& sim_mob::ConfigParams::getIncidents(){
	return incidents;
}


////////////////////////////////////////////////////////////////////////////
// Getters/setters
////////////////////////////////////////////////////////////////////////////


unsigned int& sim_mob::ConfigParams::personWorkGroupSize()
{
	return system.workers.person.count;
}
const unsigned int& sim_mob::ConfigParams::personWorkGroupSize() const
{
	return system.workers.person.count;
}

unsigned int& sim_mob::ConfigParams::signalWorkGroupSize()
{
	return system.workers.signal.count;
}
const unsigned int& sim_mob::ConfigParams::signalWorkGroupSize() const
{
	return system.workers.signal.count;
}

unsigned int& sim_mob::ConfigParams::commWorkGroupSize()
{
	return system.workers.communication.count;
}
const unsigned int& sim_mob::ConfigParams::commWorkGroupSize() const
{
	return system.workers.communication.count;
}

unsigned int& sim_mob::ConfigParams::baseGranMS()
{
	return system.simulation.baseGranMS;
}
const unsigned int& sim_mob::ConfigParams::baseGranMS() const
{
	return system.simulation.baseGranMS;
}

bool& sim_mob::ConfigParams::singleThreaded()
{
	return system.singleThreaded;
}
const bool& sim_mob::ConfigParams::singleThreaded() const
{
	return system.singleThreaded;
}

bool& sim_mob::ConfigParams::mergeLogFiles()
{
	return system.mergeLogFiles;
}
const bool& sim_mob::ConfigParams::mergeLogFiles() const
{
	return system.mergeLogFiles;
}

SystemParams::NetworkSource& sim_mob::ConfigParams::networkSource()
{
	return system.networkSource;
}
const SystemParams::NetworkSource& sim_mob::ConfigParams::networkSource() const
{
	return system.networkSource;
}

std::string& sim_mob::ConfigParams::networkXmlInputFile()
{
	return system.networkXmlInputFile;
}
const std::string& sim_mob::ConfigParams::networkXmlInputFile() const
{
	return system.networkXmlInputFile;
}

std::string& sim_mob::ConfigParams::networkXmlOutputFile()
{
	return system.networkXmlOutputFile;
}
const std::string& sim_mob::ConfigParams::networkXmlOutputFile() const
{
	return system.networkXmlOutputFile;
}

std::string& sim_mob::ConfigParams::roadNetworkXsdSchemaFile()
{
	return system.roadNetworkXsdSchemaFile;
}
void sim_mob::ConfigParams::setRoadNetworkXsdSchemaFile(std::string& name)
{
	system.roadNetworkXsdSchemaFile = name;
}
const std::string& sim_mob::ConfigParams::roadNetworkXsdSchemaFile() const
{
	return system.roadNetworkXsdSchemaFile;
}

AuraManager::AuraManagerImplementation& sim_mob::ConfigParams::aura_manager_impl()
{
	return system.simulation.auraManagerImplementation;
}
const AuraManager::AuraManagerImplementation& sim_mob::ConfigParams::aura_manager_impl() const
{
	return system.simulation.auraManagerImplementation;
}

int& sim_mob::ConfigParams::percent_boarding()
{
	return system.simulation.passenger_percent_boarding;
}
const int& sim_mob::ConfigParams::percent_boarding() const
{
	return system.simulation.passenger_percent_boarding;
}

int& sim_mob::ConfigParams::percent_alighting()
{
	return system.simulation.passenger_percent_alighting;
}
const int& sim_mob::ConfigParams::percent_alighting() const
{
	return system.simulation.passenger_percent_alighting;
}

WorkGroup::ASSIGNMENT_STRATEGY& sim_mob::ConfigParams::defaultWrkGrpAssignment()
{
	return system.simulation.workGroupAssigmentStrategy;
}
const WorkGroup::ASSIGNMENT_STRATEGY& sim_mob::ConfigParams::defaultWrkGrpAssignment() const
{
	return system.simulation.workGroupAssigmentStrategy;
}

sim_mob::MutexStrategy& sim_mob::ConfigParams::mutexStategy()
{
	return system.simulation.mutexStategy;
}
const sim_mob::MutexStrategy& sim_mob::ConfigParams::mutexStategy() const
{
	return system.simulation.mutexStategy;
}

bool& sim_mob::ConfigParams::commSimEnabled()
{
	return system.simulation.commSimEnabled;
}
const bool& sim_mob::ConfigParams::commSimEnabled() const
{
	return system.simulation.commSimEnabled;
}

const std::map<std::string,sim_mob::SimulationParams::CommsimElement> &sim_mob::ConfigParams::getCommSimElements() const{
	return system.simulation.commsimElements;
}

const std::string& sim_mob::ConfigParams::getCommSimMode(std::string name) const{
	if(system.simulation.commsimElements.find(name) != system.simulation.commsimElements.end()){
		return system.simulation.commsimElements.at(name).mode;
	}
	std::ostringstream out("");
	out << "Unknown Communication Simulator : " << name ;
	throw std::runtime_error(out.str());
}

bool sim_mob::ConfigParams::commSimmEnabled(std::string &name) {
	if(system.simulation.commsimElements.find(name) != system.simulation.commsimElements.end()){
		return system.simulation.commsimElements[name].enabled;
	}
	std::ostringstream out("");
	out << "Unknown Communication Simulator : " << name ;
	throw std::runtime_error(out.str());
}


DailyTime& sim_mob::ConfigParams::simStartTime()
{
	return system.simulation.simStartTime;
}
const DailyTime& sim_mob::ConfigParams::simStartTime() const
{
	return system.simulation.simStartTime;
}
const std::string& sim_mob::ConfigParams::getTravelTimeTmpTableName() const
{
	return system.simulation.travelTimeTmpTableName;
}

std::string sim_mob::ConfigParams::busline_control_type() const
{
	std::map<std::string,std::string>::const_iterator it = system.genericProps.find("busline_control_type");
	if (it==system.genericProps.end()) {
		throw std::runtime_error("busline_control_type property not found.");
	}
	return it->second;
}


unsigned int sim_mob::ConfigParams::totalRuntimeInMilliSeconds() const
{
	return system.simulation.totalRuntimeMS;
}

unsigned int sim_mob::ConfigParams::warmupTimeInMilliSeconds() const
{
	return system.simulation.totalWarmupMS;
}

unsigned int sim_mob::ConfigParams::personTimeStepInMilliSeconds() const
{
	return system.workers.person.granularityMs;
}

unsigned int sim_mob::ConfigParams::signalTimeStepInMilliSeconds() const
{
	return system.workers.signal.granularityMs;
}

bool sim_mob::ConfigParams::RunningMidSupply() const {
	try {
		const std::string& run_mode = system.genericProps.at("mid_term_run_mode");
		return (run_mode == "supply" || run_mode == "demand+supply");
	}
	catch (const std::out_of_range& oorx) {
		//throw std::runtime_error("generic property 'mid_term_run_mode' not found");
	}
        return false;
}

bool sim_mob::ConfigParams::RunningMidDemand() const {
	try {
		const std::string& run_mode = system.genericProps.at("mid_term_run_mode");
		return (run_mode == "demand" || run_mode == "demand+supply");
	}
	catch (const std::out_of_range& oorx) {
		//throw std::runtime_error("generic property 'mid_term_run_mode' not found");
	}
        return false;
}

unsigned int sim_mob::ConfigParams::communicationTimeStepInMilliSeconds() const
{
	return system.workers.communication.granularityMs;
}

std::map<std::string, std::vector<sim_mob::TripChainItem*> >& sim_mob::ConfigParams::getTripChains()
{
	return tripchains;
}

const std::map<std::string, std::vector<sim_mob::TripChainItem*> >& sim_mob::ConfigParams::getTripChains() const
{
	return tripchains;
}

std::vector<sim_mob::BusSchedule*>& sim_mob::ConfigParams::getBusSchedule()
{
	return busschedule;
}

std::vector<sim_mob::PT_trip*>& sim_mob::ConfigParams::getPT_trip()
{
	return pt_trip;
}

std::vector<sim_mob::PT_bus_dispatch_freq>& sim_mob::ConfigParams::getPT_bus_dispatch_freq()
{
	return pt_busdispatch_freq;
}

const std::vector<sim_mob::PT_bus_dispatch_freq>& sim_mob::ConfigParams::getPT_bus_dispatch_freq() const
{
	return pt_busdispatch_freq;
}

std::vector<sim_mob::PT_bus_routes>& sim_mob::ConfigParams::getPT_bus_routes()
{
	return pt_bus_routes;
}

std::vector<sim_mob::PT_bus_stops>& sim_mob::ConfigParams::getPT_bus_stops()
{
	return pt_bus_stops;
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

std::set<sim_mob::Conflux*>& sim_mob::ConfigParams::getConfluxes()
{
	return confluxes;
}

const std::set<sim_mob::Conflux*>& sim_mob::ConfigParams::getConfluxes() const
{
	return confluxes;
}

std::map<const sim_mob::MultiNode*, sim_mob::Conflux*>& sim_mob::ConfigParams::getConfluxNodes()
{
	return multinode_confluxes;
}
