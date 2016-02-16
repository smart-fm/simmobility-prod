/*
 * TrainController.cpp
 *
 *  Created on: Feb 11, 2016
 *      Author: fm-simmobility
 */

#include "entities/TrainController.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "logging/Log.hpp"

namespace sim_mob {
TrainController* TrainController::pInstance=nullptr;


TrainController::TrainController(int id, const MutexStrategy& mtxStrat):Agent(mtxStrat, id)
{

}

TrainController::~TrainController() {
	for (std::map<std::string, Platform*>::iterator it =
			mapOfIdvsPlatforms.begin(); it != mapOfIdvsPlatforms.end(); it++) {
		delete it->second;
	}
	for (std::map<unsigned int, Block*>::iterator it =
			mapOfIdvsBlocks.begin(); it != mapOfIdvsBlocks.end(); it++) {
		delete it->second;
	}
	for (std::map<unsigned int, PolyLine*>::iterator it =
			mapOfIdvsPolylines.begin(); it != mapOfIdvsPolylines.end(); it++) {
		delete it->second;
	}
	if(pInstance){
		delete pInstance;
	}
}

Entity::UpdateStatus TrainController::frame_tick(timeslice now)
{
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus TrainController::frame_init(timeslice now)
{
	return Entity::UpdateStatus::Continue;
}

void TrainController::frame_output(timeslice now)
{
}
bool TrainController::isNonspatial()
{
	return true;
}
void TrainController::initTrainController()
{
	loadPlatforms();
	loadSchedules();
	loadBlocks();
	loadRoutes();
	loadTransferedTimes();
	loadBlockPolylines();
	for (std::map<unsigned int, Block*>::iterator it = mapOfIdvsBlocks.begin();it != mapOfIdvsBlocks.end(); it++) {
		std::map<unsigned int, PolyLine*>::iterator itLine =mapOfIdvsPolylines.find(it->first);
		if (itLine != mapOfIdvsPolylines.end()) {
			it->second->setPloyLine((*itLine).second);
		} else {
			Print()<< "Block not find polyline:"<<it->first<<std::endl;
		}
	}
}

TrainController* TrainController::getInstance()
{
	if(!pInstance){
		pInstance = new TrainController();
	}
	return pInstance;
}
bool TrainController::HasTrainController()
{
	return (pInstance!=nullptr);
}

void TrainController::loadPlatforms()
{
	const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
	const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;
	std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("pt_mrt_platform");
	if(spIt == storedProcs.end())
	{
		Print() << "missing stored procedure for pt_mrt_platform" << std::endl;
		return;
	}
    soci::session sql_(soci::postgresql, configParams.getDatabaseConnectionString(false));
    soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + spIt->second);
	for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{
		const soci::row& r = (*it);
		std::string platformNo = r.get<std::string>(0);
		Platform* platform = new Platform();
		platform->setPlatformNo(platformNo);
		platform->setStationNo(r.get<std::string>(1));
		platform->setLineId(r.get<std::string>(2));
		platform->setCapactiy(r.get<int>(3));
		platform->setType(r.get<int>(4));
		platform->setAttachedBlockId(r.get<int>(5));
		platform->setOffset(r.get<double>(6));
		platform->setLength(r.get<double>(7));
		mapOfIdvsPlatforms[platformNo] = platform;
	}
}

void TrainController::loadSchedules()
{
	const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
	const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;
	std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("pt_mrt_dispatch_freq");
	if(spIt == storedProcs.end())
	{
		Print() << "missing stored procedure for pt_mrt_dispatch_freq" << std::endl;
		return;
	}
    soci::session sql_(soci::postgresql, configParams.getDatabaseConnectionString(false));
    soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + spIt->second);
	for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{
		const soci::row& r = (*it);
		std::string lineId = r.get<std::string>(1);
		TrainSchedule schedule;
		schedule.lineId = lineId;
		schedule.scheduleId = r.get<int>(0);
		schedule.startTime = r.get<std::string>(2);
		schedule.endTime = r.get<std::string>(3);
		schedule.headwaySec = r.get<int>(4);
		if(mapOfIdvsSchedules.find(lineId)==mapOfIdvsSchedules.end()){
			mapOfIdvsSchedules[lineId] = std::vector<TrainSchedule>();
		}
		mapOfIdvsSchedules[lineId].push_back(schedule);
	}
}

void TrainController::loadBlocks()
{
	const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
	const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;
	std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("pt_block");
	if(spIt == storedProcs.end())
	{
		Print() << "missing stored procedure for pt_block" << std::endl;
		return;
	}
    soci::session sql_(soci::postgresql, configParams.getDatabaseConnectionString(false));
    soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + spIt->second);
	for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{
		const soci::row& r = (*it);
		int blockId = r.get<int>(0);
		Block* block = new Block();
		block->setBlockId(blockId);
		block->setSpeedLimit(r.get<double>(1));
		block->setAccelerateRate(r.get<double>(2));
		block->setDecelerateRate(r.get<double>(3));
		block->setLength(r.get<double>(4));
		mapOfIdvsBlocks[blockId] = block;
	}
}

void TrainController::loadRoutes()
{
	const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
	const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;
	std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("pt_mrt_route");
	if(spIt == storedProcs.end())
	{
		Print() << "missing stored procedure for pt_mrt_route" << std::endl;
		return;
	}
    soci::session sql_(soci::postgresql, configParams.getDatabaseConnectionString(false));
    soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + spIt->second);
	for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{
		const soci::row& r = (*it);
		std::string lineId = r.get<std::string>(0);
		TrainRoute route;
		route.lineId = lineId;
		route.blockId = r.get<int>(1);
		route.sequenceNo = r.get<int>(2);
		if(mapOfIdvsRoutes.find(lineId)==mapOfIdvsRoutes.end()){
			mapOfIdvsRoutes[lineId] = std::vector<TrainRoute>();
		}
		mapOfIdvsRoutes[lineId].push_back(route);
	}
}

void TrainController::loadTransferedTimes()
{

}

void TrainController::loadBlockPolylines()
{
	const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
	const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;
	std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("pt_block_polyline");
	if(spIt == storedProcs.end())
	{
		Print() << "missing stored procedure for pt_block_polyline" << std::endl;
		return;
	}
    soci::session sql_(soci::postgresql, configParams.getDatabaseConnectionString(false));
    soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + spIt->second);
	for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{
		const soci::row& r = (*it);
		int polylineId = r.get<int>(0);
		PolyLine* polyline = new PolyLine();
		polyline->setPolyLineId(polylineId);
		PolyPoint point;
		point.setPolyLineId(polylineId);
		point.setX(r.get<double>(1));
		point.setY(r.get<double>(2));
		point.setZ(r.get<double>(3));
		point.setSequenceNumber(r.get<double>(4));
		if(mapOfIdvsPolylines.find(polylineId)==mapOfIdvsPolylines.end()){
			mapOfIdvsPolylines[polylineId] = new PolyLine();
			mapOfIdvsPolylines[polylineId]->setPolyLineId(polylineId);
		}
		mapOfIdvsPolylines[polylineId]->addPoint(point);
	}
}

} /* namespace sim_mob */
