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

using namespace sim_mob;
using namespace sim_mob::db;

namespace
{
/**
 * fetches configuration and constructs DB_Connection
 * @return the constructed DB_Connection object
 */
DB_Connection getConnection()
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	Database logsumDB = config.constructs.databases.at("fm_remote_mt");
	Credential logsumDB_Credentials = ConfigManager::GetInstance().FullConfig().constructs.credentials.at("fm_remote_mt");
	std::string username = logsumDB_Credentials.getUsername();
	std::string password = logsumDB_Credentials.getPassword(false);
	DB_Config logsumDbConfig(logsumDB.host, logsumDB.port, logsumDB.dbName, username, password);

	//connect to database and load data.
	DB_Connection conn(sim_mob::db::POSTGRES, logsumDbConfig);
	return conn;
}

/**
 * file global DB_Connection object
 */
DB_Connection dbConnection = getConnection();

} //end anonymous namespace

sim_mob::PredayLT_LogsumManager::PredayLT_LogsumManager() : dataLoadReqd(true)
{}

sim_mob::PredayLT_LogsumManager::~PredayLT_LogsumManager()
{}

void sim_mob::PredayLT_LogsumManager::loadZones()
{
	if (dbConnection.isConnected())
	{
		ZoneSqlDao zoneDao(dbConnection);
		zoneDao.getAll(zoneMap, ZoneParams::getZoneId);
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
	if(dataLoadReqd)
	{
		dbConnection.connect();
		loadZones();
		loadCosts();
		dbConnection.disconnect();
		dataLoadReqd = false;
	}
	return logsumManager;
}

double sim_mob::PredayLT_LogsumManager::computeLogsum(long individualId, int homeLocation, int workLocation) const
{
}
