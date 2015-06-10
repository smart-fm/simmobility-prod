//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PredayLT_Logsum.hpp"

#include <vector>

#include "behavioral/lua/PredayLogsumLuaProvider.hpp"
#include "behavioral/params/LogsumTourModeDestinationParams.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "database/DB_Config.hpp"
#include "database/DB_Connection.hpp"
#include "database/predaydao/DatabaseHelper.hpp"
#include "database/predaydao/LT_PopulationSqlDao.hpp"
#include "database/predaydao/ZoneCostSqlDao.hpp"
#include "logging/Log.hpp"

using namespace sim_mob;
using namespace sim_mob::db;

namespace
{
std::string EMPTY_STRING = "";
const std::string LT_DB_CONFIG_FILE = "private/lt-db.ini";
/**
 * DB_Config for logsum db.
 * initialized from getConnection()
 */
DB_Config mtDbConfig(EMPTY_STRING,EMPTY_STRING,EMPTY_STRING,EMPTY_STRING,EMPTY_STRING);
DB_Config ltDbConfig(LT_DB_CONFIG_FILE);

/**
 * file global DB_Connection objects
 * re-initialized correctly from first call to getInstance
 */
DB_Connection mtDbConnection(sim_mob::db::POSTGRES, mtDbConfig);
DB_Connection ltDbConnection(sim_mob::db::POSTGRES, ltDbConfig);

/**
 * DAO for fetching individuals from db
 */
LT_PopulationSqlDao ltPopulationDao(ltDbConnection);

/**
 * fetches configuration and constructs DB_Connection
 * @return the constructed DB_Connection object
 */
DB_Connection getConnection()
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	const std::string dbId = "fm_remote_mt";
	Database logsumDB = config.constructs.databases.at(dbId);
	Credential logsumDB_Credentials = ConfigManager::GetInstance().FullConfig().constructs.credentials.at(dbId);
	std::string username = logsumDB_Credentials.getUsername();
	std::string password = logsumDB_Credentials.getPassword(false);
	mtDbConfig = DB_Config(logsumDB.host, logsumDB.port, logsumDB.dbName, username, password);

	//connect to database and load data.
	DB_Connection conn(sim_mob::db::POSTGRES, mtDbConfig);
	return conn;
}
} //end anonymous namespace

//init static member
sim_mob::PredayLT_LogsumManager sim_mob::PredayLT_LogsumManager::logsumManager;

sim_mob::PredayLT_LogsumManager::PredayLT_LogsumManager() : dataLoadReqd(true)
{}

sim_mob::PredayLT_LogsumManager::~PredayLT_LogsumManager()
{
	ltDbConnection.disconnect(); //safe only because logsumManager is a singleton and this class is noncopyable

	// clear Zones
	Print() << "Clearing zoneMap" << std::endl;
	for(ZoneMap::iterator i = zoneMap.begin(); i!=zoneMap.end(); i++) {
		delete i->second;
	}
	zoneMap.clear();

	// clear AMCosts
	Print() << "Clearing amCostMap" << std::endl;
	for(CostMap::iterator i = amCostMap.begin(); i!=amCostMap.end(); i++) {
		for(boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j!=i->second.end(); j++) {
			if(j->second) {
				delete j->second;
			}
		}
	}
	amCostMap.clear();

	// clear PMCosts
	Print() << "Clearing pmCostMap" << std::endl;
	for(CostMap::iterator i = pmCostMap.begin(); i!=pmCostMap.end(); i++) {
		for(boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j!=i->second.end(); j++) {
			if(j->second) {
				delete j->second;
			}
		}
	}
	pmCostMap.clear();

	// clear OPCosts
	Print() << "Clearing opCostMap" << std::endl;
	for(CostMap::iterator i = opCostMap.begin(); i!=opCostMap.end(); i++) {
		for(boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j!=i->second.end(); j++) {
			if(j->second) {
				delete j->second;
			}
		}
	}
	opCostMap.clear();
}

void sim_mob::PredayLT_LogsumManager::loadZones()
{
	if (mtDbConnection.isConnected())
	{
		ZoneSqlDao zoneDao(mtDbConnection);
		zoneDao.getAll(zoneMap, &ZoneParams::getZoneId);
	}
	else
	{
		throw std::runtime_error("MT database connection unavailable");
	}

	//construct zoneIdLookup
	for(ZoneMap::iterator znMapIt=zoneMap.begin(); znMapIt!=zoneMap.end(); znMapIt++)
	{
		zoneIdLookup[znMapIt->second->getZoneCode()] = znMapIt->first;
	}
	Print() << "TAZs loaded" << std::endl;
}

void sim_mob::PredayLT_LogsumManager::loadCosts()
{
	if (mtDbConnection.isConnected())
	{
		CostSqlDao amCostDao(mtDbConnection, DB_GET_ALL_AM_COSTS);
		amCostDao.getAll(amCostMap);
		Print() << "AM costs loaded" << std::endl;

		CostSqlDao pmCostDao(mtDbConnection, DB_GET_ALL_PM_COSTS);
		pmCostDao.getAll(pmCostMap);
		Print() << "PM costs loaded" << std::endl;

		CostSqlDao opCostDao(mtDbConnection, DB_GET_ALL_OP_COSTS);
		opCostDao.getAll(opCostMap);
		Print() << "OP costs loaded" << std::endl;
	}
	else
	{
		throw std::runtime_error("MT database connection unavailable");
	}
}

const PredayLT_LogsumManager& sim_mob::PredayLT_LogsumManager::getInstance()
{
	if(logsumManager.dataLoadReqd)
	{
		mtDbConnection = getConnection();
		mtDbConnection.connect();
		logsumManager.loadZones();
		logsumManager.loadCosts();
		mtDbConnection.disconnect();

		ltDbConfig.load();
		ltDbConnection = DB_Connection(sim_mob::db::POSTGRES, ltDbConfig);
		ltDbConnection.connect();
		if(!ltDbConnection.isConnected()) { throw std::runtime_error("LT database connection failure!"); }
		ltPopulationDao.getIncomeCategories(PredayPersonParams::getIncomeCategoryLowerLimits());
		ltPopulationDao.getVehicleCategories(PredayPersonParams::getVehicleCategoryLookup());
		ltPopulationDao.getAddressTAZs(PredayPersonParams::getAddressTazLookup());
		logsumManager.dataLoadReqd = false;
	}
	return logsumManager;
}

double sim_mob::PredayLT_LogsumManager::computeLogsum(long individualId, int homeLocation, int workLocation) const
{
	// construct population dao
	if(!ltDbConnection.isConnected()) { throw std::runtime_error("LT database connection failure!"); }

	PredayPersonParams personParams;
	ltPopulationDao.getOneById(individualId, personParams);
	if(personParams.getPersonId().empty()) { throw std::runtime_error("individual could not be fetched from LT db"); }
	personParams.setHomeLocation(homeLocation);
	personParams.setFixedWorkLocation(workLocation);

	LogsumTourModeDestinationParams tmdParams(zoneMap, amCostMap, pmCostMap, personParams, NULL_STOP);
	tmdParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(personParams.getHomeLocation()))->getCbdDummy());

	PredayLogsumLuaProvider::getPredayModel().computeTourModeDestinationLogsum(personParams, tmdParams);
	PredayLogsumLuaProvider::getPredayModel().computeDayPatternLogsums(personParams);
	PredayLogsumLuaProvider::getPredayModel().computeDayPatternBinaryLogsums(personParams);

	return personParams.getDpbLogsum();
}
