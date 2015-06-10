//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PredayLT_Logsum.hpp"

#include <vector>

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
/**
 * DB_Config for logsum db.
 * initialized from getConnection()
 */
DB_Config logsumDbConfig(EMPTY_STRING,EMPTY_STRING,EMPTY_STRING,EMPTY_STRING,EMPTY_STRING);

/**
 * file global DB_Connection object
 * re-initialized correctly from first call to getInstance
 */
DB_Connection dbConnection(sim_mob::db::POSTGRES, logsumDbConfig);

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
	logsumDbConfig = DB_Config(logsumDB.host, logsumDB.port, logsumDB.dbName, username, password);

	//connect to database and load data.
	DB_Connection conn(sim_mob::db::POSTGRES, logsumDbConfig);
	return conn;
}
} //end anonymous namespace

//init static member
sim_mob::PredayLT_LogsumManager sim_mob::PredayLT_LogsumManager::logsumManager;

sim_mob::PredayLT_LogsumManager::PredayLT_LogsumManager() : dataLoadReqd(true)
{}

sim_mob::PredayLT_LogsumManager::~PredayLT_LogsumManager()
{}

void sim_mob::PredayLT_LogsumManager::loadZones()
{
	if (dbConnection.isConnected())
	{
		ZoneSqlDao zoneDao(dbConnection);
		zoneDao.getAll(zoneMap, &ZoneParams::getZoneId);
	}

	//construct zoneIdLookup
	for(ZoneMap::iterator znMapIt=zoneMap.begin(); znMapIt!=zoneMap.end(); znMapIt++)
	{
		zoneIdLookup[znMapIt->second->getZoneCode()] = znMapIt->first;
	}
}

void sim_mob::PredayLT_LogsumManager::loadCosts()
{
	if (dbConnection.isConnected())
	{
		CostSqlDao amCostDao(dbConnection, DB_GET_ALL_AM_COSTS);
		amCostDao.getAll(amCostMap);

		CostSqlDao pmCostDao(dbConnection, DB_GET_ALL_PM_COSTS);
		pmCostDao.getAll(pmCostMap);

		CostSqlDao opCostDao(dbConnection, DB_GET_ALL_OP_COSTS);
		opCostDao.getAll(opCostMap);
	}
}

const PredayLT_LogsumManager& sim_mob::PredayLT_LogsumManager::getInstance()
{
	if(logsumManager.dataLoadReqd)
	{
		dbConnection = getConnection();
		dbConnection.connect();
		logsumManager.loadZones();
		logsumManager.loadCosts();
		dbConnection.disconnect();
		logsumManager.dataLoadReqd = false;
	}
	return logsumManager;
}

double sim_mob::PredayLT_LogsumManager::computeLogsum(long individualId, int homeLocation, int workLocation) const
{

}
