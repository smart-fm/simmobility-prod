//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PredayLT_Logsum.hpp"

#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
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

const std::string LT_DB_CONFIG_FILE = "private/lt-db.ini";


/**
 * wrapper struct for thread local storage
 */
struct LT_PopulationSqlDaoContext
{
public:
	/**
	 * DAO for fetching individuals from db
	 */
	LT_PopulationSqlDao ltPopulationDao;

	LT_PopulationSqlDaoContext(const DB_Config& ltDbConfig) : ltDbConnection(sim_mob::db::POSTGRES, ltDbConfig), ltPopulationDao(ltDbConnection)
	{
		ltDbConnection.connect();
		if(!ltDbConnection.isConnected()) { throw std::runtime_error("LT database connection failure!"); }
	}

private:
	/**
	 * DB_Connection object for LT db
	 */
	DB_Connection ltDbConnection;
};

boost::thread_specific_ptr<LT_PopulationSqlDaoContext> threadContext;

/**
 * constructs the SQL dao for calling thread, if not already constructed
 */
void ensureContext()
{
	if (!threadContext.get())
	{
		DB_Config ltDbConfig(LT_DB_CONFIG_FILE);
		ltDbConfig.load();
		LT_PopulationSqlDaoContext* ltPopulationSqlDaoCtx = new LT_PopulationSqlDaoContext(ltDbConfig);
		threadContext.reset(ltPopulationSqlDaoCtx);
	}
}
} //end anonymous namespace

//init static member
sim_mob::PredayLT_LogsumManager sim_mob::PredayLT_LogsumManager::logsumManager;

sim_mob::PredayLT_LogsumManager::PredayLT_LogsumManager() : dataLoadReqd(true)
{}

sim_mob::PredayLT_LogsumManager::~PredayLT_LogsumManager()
{
	if(!logsumManager.dataLoadReqd)
	{
		// clear Zones
		//Print() << "Clearing zoneMap" << std::endl;
		for(ZoneMap::iterator i = zoneMap.begin(); i!=zoneMap.end(); i++) {
			delete i->second;
		}
		zoneMap.clear();

		// clear AMCosts
		//Print() << "Clearing amCostMap" << std::endl;
		for(CostMap::iterator i = amCostMap.begin(); i!=amCostMap.end(); i++) {
			for(boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j!=i->second.end(); j++) {
				if(j->second) {
					delete j->second;
				}
			}
		}
		amCostMap.clear();

		// clear PMCosts
		//Print() << "Clearing pmCostMap" << std::endl;
		for(CostMap::iterator i = pmCostMap.begin(); i!=pmCostMap.end(); i++) {
			for(boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j!=i->second.end(); j++) {
				if(j->second) {
					delete j->second;
				}
			}
		}
		pmCostMap.clear();

		// clear OPCosts
		//Print() << "Clearing opCostMap" << std::endl;
		for(CostMap::iterator i = opCostMap.begin(); i!=opCostMap.end(); i++) {
			for(boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j!=i->second.end(); j++) {
				if(j->second) {
					delete j->second;
				}
			}
		}
		opCostMap.clear();
	}
}

void sim_mob::PredayLT_LogsumManager::loadZones()
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	const std::string dbId = "fm_remote_mt";
	Database logsumDB = config.constructs.databases.at(dbId);
	Credential logsumDB_Credentials = ConfigManager::GetInstance().FullConfig().constructs.credentials.at(dbId);
	std::string username = logsumDB_Credentials.getUsername();
	std::string password = logsumDB_Credentials.getPassword(false);
	DB_Config mtDbConfig(logsumDB.host, logsumDB.port, logsumDB.dbName, username, password);

	//connect to database and load data.
	DB_Connection mtDbConnection(sim_mob::db::POSTGRES, mtDbConfig);
	mtDbConnection.connect();
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
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	const std::string dbId = "fm_remote_mt";
	Database logsumDB = config.constructs.databases.at(dbId);
	Credential logsumDB_Credentials = ConfigManager::GetInstance().FullConfig().constructs.credentials.at(dbId);
	std::string username = logsumDB_Credentials.getUsername();
	std::string password = logsumDB_Credentials.getPassword(false);
	DB_Config mtDbConfig(logsumDB.host, logsumDB.port, logsumDB.dbName, username, password);

	//connect to database and load data.
	DB_Connection mtDbConnection(sim_mob::db::POSTGRES, mtDbConfig);
	mtDbConnection.connect();
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
		logsumManager.loadZones();
		logsumManager.loadCosts();

		ensureContext();
		LT_PopulationSqlDao& ltPopulationDao = threadContext.get()->ltPopulationDao;
		ltPopulationDao.getIncomeCategories(PredayPersonParams::getIncomeCategoryLowerLimits());
		ltPopulationDao.getVehicleCategories(PredayPersonParams::getVehicleCategoryLookup());
		ltPopulationDao.getAddressTAZs(PredayPersonParams::getAddressTazLookup());
		logsumManager.dataLoadReqd = false;
	}
	return logsumManager;
}

PredayPersonParams sim_mob::PredayLT_LogsumManager::computeLogsum(long individualId, int homeLocation, int workLocation, int vehicleOwnership) const
{
	ensureContext();
	PredayPersonParams personParams;
	LT_PopulationSqlDao& ltPopulationDao = threadContext.get()->ltPopulationDao;
	ltPopulationDao.getOneById(individualId, personParams);
	if(personParams.getPersonId().empty())
	{
		return PredayPersonParams();
	}

	if(homeLocation > 0) { personParams.setHomeLocation(homeLocation); }
	if(workLocation > 0)
	{
		personParams.setHasWorkplace(true);
		personParams.setFixedWorkLocation(workLocation);
	}

	if( vehicleOwnership == 1)
	{
		personParams.setCarOwnNormal(1);
	}
	else if( vehicleOwnership == 0 )
	{
		personParams.setCarOwnNormal(0);
	}

	int homeLoc = personParams.getHomeLocation();
	boost::unordered_map<int,int>::const_iterator zoneLookupItr = zoneIdLookup.find(homeLoc);
	if( zoneLookupItr == zoneIdLookup.end())
	{
		return PredayPersonParams();
	}

	bool printedError = false;

	if(personParams.hasFixedWorkPlace())
	{
		int workLoc = workLoc = personParams.getFixedWorkLocation();
		ZoneParams* orgZnParams = nullptr;
		ZoneParams* destZnParams = nullptr;
		CostParams* amCostParams = nullptr;
		CostParams* pmCostParams = nullptr;

		try
		{
			orgZnParams = zoneMap.at(zoneIdLookup.at(homeLoc));
		}
		catch(...)
		{
			orgZnParams = new ZoneParams();

			//std::cout << "individualId: " << individualId << " " << homeLoc << " taz cannot be found" << std::endl;
			//printedError = true;
		}

		try
		{
			destZnParams = zoneMap.at(zoneIdLookup.at(workLoc));
		}
		catch(...)
		{
			destZnParams = new ZoneParams();

			//if( !printedError )
			//	std::cout << "individualId: " << individualId << " " << workLoc  << " taz cannot be found. destznparam" << std::endl;

			printedError = true;
		}

		if(homeLoc != workLoc)
		{
			try
			{
				amCostParams = amCostMap.at(homeLoc).at(workLoc);
			}
			catch(...)
			{
				amCostParams = new CostParams();

				//if( !printedError )
				//	std::cout << "individualId: " << individualId << " " << workLoc << " or " << homeLoc << " taz cannot be found. amcostparam" << std::endl;

				printedError = true;
			}

			try
			{
				pmCostParams = pmCostMap.at(workLoc).at(homeLoc);
			}
			catch(...)
			{
				pmCostParams = new CostParams();

				//if( !printedError )
				//	std::cout << "individualId: " << individualId << " " << workLoc << " or " << homeLoc << " taz cannot be found. pmcostparam" << std::endl;

				printedError = true;
			}

		}
		LogsumTourModeParams tmParams(orgZnParams, destZnParams, amCostParams, pmCostParams, personParams, WORK);
		PredayLogsumLuaProvider::getPredayModel().computeTourModeLogsum(personParams, tmParams);

	}

	LogsumTourModeDestinationParams tmdParams(zoneMap, amCostMap, pmCostMap, personParams, NULL_STOP);

	try
	{
		tmdParams.setCbdOrgZone(zoneMap.at(zoneLookupItr->second)->getCbdDummy());
	}
	catch(...)
	{
		//if( !printedError )
		//	std::cout << "individualId: " << individualId << " " << zoneLookupItr->second << " taz cannot be found. cbdorgzone" << std::endl;

		printedError = true;
	}


	PredayLogsumLuaProvider::getPredayModel().computeTourModeDestinationLogsum(personParams, tmdParams, amCostMap.size());
	PredayLogsumLuaProvider::getPredayModel().computeDayPatternLogsums(personParams);
	PredayLogsumLuaProvider::getPredayModel().computeDayPatternBinaryLogsums(personParams);

	return personParams;
}
