//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include "behavioral/WithindayHelper.hpp"
#include "boost/unordered/detail/buckets.hpp"
#include "boost/unordered/unordered_map.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/Constructs.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/dao/SqlAbstractDao.hpp"
#include "database/DB_Connection.hpp"
#include "database/predaydao/ZoneCostSqlDao.hpp"
#include "entities/misc/TripChain.hpp"
#include "logging/Log.hpp"


using namespace sim_mob;
using namespace sim_mob::db;

WithindayModelsHelper::ZoneMap WithindayModelsHelper::zoneMap;
bool WithindayModelsHelper::initialized = false;

void WithindayModelsHelper::loadZones()
{
	if(initialized) { return; }
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	const DatabaseDetails& dbInfo = cfg.networkDatabase;
	const std::string& dbId = dbInfo.database;
	Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at(dbId);
	const std::string& cred_id = dbInfo.credentials;
	Credential cred = ConfigManager::GetInstance().FullConfig().constructs.credentials.at(cred_id);
	std::string username = cred.getUsername();
	std::string password = cred.getPassword(false);
	DB_Config dbConfig(db.host, db.port, db.dbName, username, password);
	DB_Connection znDbConnection(sim_mob::db::POSTGRES, dbConfig);
	znDbConnection.connect();
	if (znDbConnection.isConnected())
	{
		ZoneSqlDao zoneDao(znDbConnection);
		zoneDao.getAll(zoneMap, &ZoneParams::getZoneCode);
		initialized = true;
	}
	else
	{
		throw std::runtime_error("MT database connection unavailable");
	}
	Print() << "TAZs loaded" << std::endl;
}

WithindayModelsHelper::WithindayModelsHelper()
{
}

WithindayModelsHelper::~WithindayModelsHelper()
{
}

const ZoneParams* sim_mob::WithindayModelsHelper::findZone(int zoneCode) const
{
	ZoneMap::const_iterator znIt = zoneMap.find(zoneCode);
	if(znIt == zoneMap.end())
	{
		throw std::runtime_error("invalid zone code passed to fetch Zone data: " + std::to_string(zoneCode));
	}
	return znIt->second;
}

WithindayModeParams WithindayModelsHelper::buildModeChoiceParams(const Trip& curTrip, unsigned int orgNd) const
{
	WithindayModeParams wdModeParams;
	int originZn = curTrip.originZoneCode;
	int destinZn = curTrip.destinationZoneCode;
	unsigned int destNd = curTrip.destination.node->getNodeId();

	const ZoneParams* orgZnParams = findZone(originZn);
	wdModeParams.setOriginArea(orgZnParams->getArea());
	wdModeParams.setOriginResidentSize(orgZnParams->getResidentWorkers());

	const ZoneParams* destZnParams = findZone(destinZn);
	wdModeParams.setDestinationArea(destZnParams->getArea());
	wdModeParams.setDestinationStudentsSize(destZnParams->getTotalEnrollment());
	wdModeParams.setDestinationWorkerSize(destZnParams->getEmployment());
	wdModeParams.setCostCarParking(destZnParams->getParkingRate());
	wdModeParams.setCentralZone(destZnParams->getCentralDummy());

	return wdModeParams;
//	wdModeParams.setAvgTransfer(double avgTransfer)
//	wdModeParams.setTtCarInVehicle(double ttCarInVehicle)
//	wdModeParams.setTtPublicInVehicle(double ttPublicInVehicle)
//	wdModeParams.setTtPublicWaiting(double ttPublicWaiting)
//	wdModeParams.setTtPublicWalk(double ttPublicWalk)
//	wdModeParams.setWalkDistance(double walkDistance)

}
