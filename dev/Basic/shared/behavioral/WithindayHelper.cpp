//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include "behavioral/params/WithindayModeParams.hpp"
#include "behavioral/WithindayHelper.hpp"
#include "boost/unordered/detail/buckets.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/Constructs.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/dao/SqlAbstractDao.hpp"
#include "database/DB_Config.hpp"
#include "database/DB_Connection.hpp"
#include "database/predaydao/ZoneCostSqlDao.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/WayPoint.hpp"
#include "path/Path.hpp"
#include "path/PathSetManager.hpp"
#include "path/PT_RouteChoiceLuaModel.hpp"
#include "path/PT_RouteChoiceLuaProvider.hpp"
#include "util/DailyTime.hpp"

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

WithindayModeParams WithindayModelsHelper::buildModeChoiceParams(const Trip& curTrip, unsigned int orgNd, const DailyTime& curTime) const
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
	wdModeParams.setDestinationShops(destZnParams->getShop());

	double carInVehicleTT = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(orgNd, destNd, curTime);
	if(carInVehicleTT <= 0) { carInVehicleTT = std::numeric_limits<double>::max(); }
	wdModeParams.setTtCarInVehicle(carInVehicleTT);

	const PT_PathSet ptPathset = PT_RouteChoiceLuaProvider::getPTRC_Model().fetchPathset(orgNd, destNd, curTime);
	unsigned int numPaths = ptPathset.pathSet.size();
	int sumTransfers = 0;
	double sumInVehicleTimeSecs = 0.0;
	double sumWaitingTimeSecs = 0.0;
	double sumWalkTimeSecs = 0.0;
	double sumDistance = 0.0;
	for(const auto& ptPath : ptPathset.pathSet)
	{
		sumTransfers = sumTransfers + ptPath.getNumTransfers();
		sumInVehicleTimeSecs = sumInVehicleTimeSecs + ptPath.getInVehicleTravelTimeSecs();
		sumWaitingTimeSecs = sumWaitingTimeSecs + ptPath.getWaitingTimeSecs();
		sumWalkTimeSecs = sumWalkTimeSecs + ptPath.getWalkingTimeSecs();
		sumDistance = sumDistance + ptPath.getPathDistanceKms();
	}
	wdModeParams.setAvgTransfer(sumTransfers/numPaths);
	wdModeParams.setTtPublicInVehicle(sumInVehicleTimeSecs/numPaths);
	wdModeParams.setTtPublicWaiting(sumWaitingTimeSecs/numPaths);
	wdModeParams.setTtPublicWalk(sumWalkTimeSecs/numPaths);
	wdModeParams.setDistance(sumDistance/numPaths);

	wdModeParams.setTripType(curTrip.purpose);

	return wdModeParams;

}
