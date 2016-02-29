/*
 * TrainController.cpp
 *
 *  Created on: Feb 11, 2016
 *      Author: fm-simmobility
 */

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "logging/Log.hpp"

#ifndef _CLASS_TRAIN_CONTROLLER_FUNCTIONS
#include "entities/TrainController.hpp"
#else

namespace sim_mob {
	template<typename PERSON>
	TrainController<PERSON>* TrainController<PERSON>::pInstance=nullptr;
	template<typename PERSON>
	boost::unordered_map<const Station*, Agent*> TrainController<PERSON>::allStationAgents;
	template<typename PERSON>
	TrainController<PERSON>::TrainController(int id, const MutexStrategy& mtxStrat):Agent(mtxStrat, id)
	{

	}
	template<typename PERSON>
	TrainController<PERSON>::~TrainController() {
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
		if(pInstance) {
			delete pInstance;
		}
	}
	template<typename PERSON>
	Entity::UpdateStatus TrainController<PERSON>::frame_tick(timeslice now)
	{
		return Entity::UpdateStatus::Continue;
	}
	template<typename PERSON>
	Entity::UpdateStatus TrainController<PERSON>::frame_init(timeslice now)
	{
		return Entity::UpdateStatus::Continue;
	}
	template<typename PERSON>
	void TrainController<PERSON>::frame_output(timeslice now)
	{
	}
	template<typename PERSON>
	bool TrainController<PERSON>::isNonspatial()
	{
		return true;
	}
	template<typename PERSON>
	bool TrainController<PERSON>::getTrainRoute(const std::string& lineId, std::vector<Block*>& route)
	{
		bool res = true;
		std::map<std::string, std::vector<TrainRoute>>::iterator it = mapOfIdvsTrainRoutes.find(lineId);
		if(it==mapOfIdvsTrainRoutes.end()) {
			res = false;
		} else {
			std::vector<TrainRoute>& trainRoute = it->second;
			for(std::vector<TrainRoute>::iterator i=trainRoute.begin();i!=trainRoute.end();i++) {
				std::map<unsigned int, Block*>::iterator iBlock = mapOfIdvsBlocks.find(i->blockId);
				if(iBlock==mapOfIdvsBlocks.end()) {
					res = false;
					break;
				} else {
					route.push_back(iBlock->second);
				}
			}
		}
		return res;
	}
	template<typename PERSON>
	bool TrainController<PERSON>::getTrainPlatforms(const std::string& lineId, std::vector<Platform*>& platforms)
	{
		bool res = true;
		std::map<std::string, std::vector<TrainPlatform>>::iterator it = mapOfIdvsTrainPlatforms.find(lineId);
		if(it==mapOfIdvsTrainPlatforms.end()) {
			res = false;
		} else {
			std::vector<TrainPlatform>& trainPlatform = it->second;
			for(std::vector<TrainPlatform>::iterator i=trainPlatform.begin();i!=trainPlatform.end();i++) {
				std::map<std::string, Platform*>::iterator iPlatform = mapOfIdvsPlatforms.find(i->platformNo);
				if(iPlatform==mapOfIdvsPlatforms.end()) {
					res = false;
					break;
				} else {
					platforms.push_back(iPlatform->second);
				}
			}
		}
		return res;
	}
	template<typename PERSON>
	void TrainController<PERSON>::initTrainController()
	{
		loadPlatforms();
		loadSchedules();
		loadBlocks();
		loadTrainRoutes();
		loadTrainPlatform();
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

		for(std::map<std::string, Platform*>::iterator it = mapOfIdvsPlatforms.begin(); it!=mapOfIdvsPlatforms.end(); it++) {
			std::map<unsigned int, Block*>::iterator itBlock=mapOfIdvsBlocks.find(it->second->getAttachedBlockId());
			if(itBlock!=mapOfIdvsBlocks.end()) {
				itBlock->second->setAttachedPlatform(it->second);
			} else {
				Print()<< "Platform not find corresponding block:"<<it->first<<std::endl;
			}
		}
	}
	template<typename PERSON>
	TrainController<PERSON>* TrainController<PERSON>::getInstance()
	{
		if(!pInstance) {
			pInstance = new TrainController<PERSON>();
		}
		return pInstance;
	}
	template<typename PERSON>
	bool TrainController<PERSON>::HasTrainController()
	{
		return (pInstance!=nullptr);
	}
	template<typename PERSON>
	void TrainController<PERSON>::loadPlatforms()
	{
		const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
		const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;
		std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("pt_platform");
		if(spIt == storedProcs.end())
		{
			Print() << "missing stored procedure for pt_platform" << std::endl;
			return;
		}
		soci::session sql_(soci::postgresql, configParams.getDatabaseConnectionString(false));
		soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + spIt->second);
		for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
		{
			const soci::row& r = (*it);
			std::string platformNo = r.get<std::string>(0);
			std::string stationNo = r.get<std::string>(1);
			Platform* platform = new Platform();
			platform->setPlatformNo(platformNo);
			platform->setStationNo(stationNo);
			platform->setLineId(r.get<std::string>(2));
			platform->setCapactiy(r.get<int>(3));
			platform->setType(r.get<int>(4));
			platform->setAttachedBlockId(r.get<int>(5));
			platform->setOffset(r.get<double>(6));
			platform->setLength(r.get<double>(7));
			mapOfIdvsPlatforms[platformNo] = platform;
			if(mapOfIdvsStations.find(stationNo)==mapOfIdvsStations.end()){
				mapOfIdvsStations[stationNo] = new Station(stationNo);
			}
			Station* station = mapOfIdvsStations[stationNo];
			station->addPlatform(platform->getLineId(), platform);
		}
	}
	template<typename PERSON>
	void TrainController<PERSON>::loadSchedules()
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
			schedule.scheduleId = r.get<std::string>(0);
			schedule.startTime = r.get<std::string>(2);
			schedule.endTime = r.get<std::string>(3);
			schedule.headwaySec = r.get<int>(4);
			if(mapOfIdvsSchedules.find(lineId)==mapOfIdvsSchedules.end()) {
				mapOfIdvsSchedules[lineId] = std::vector<TrainSchedule>();
			}
			mapOfIdvsSchedules[lineId].push_back(schedule);
		}
	}
	template<typename PERSON>
	void TrainController<PERSON>::loadBlocks()
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
			block->setAccelerateRate(10.0);
			block->setDecelerateRate(10.0);
			block->setLength(r.get<double>(4));
			mapOfIdvsBlocks[blockId] = block;
		}
	}
	template<typename PERSON>
	void TrainController<PERSON>::loadTrainRoutes()
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
			if(mapOfIdvsTrainRoutes.find(lineId)==mapOfIdvsTrainRoutes.end()) {
				mapOfIdvsTrainRoutes[lineId] = std::vector<TrainRoute>();
			}
			mapOfIdvsTrainRoutes[lineId].push_back(route);
		}
	}
	template<typename PERSON>
	void TrainController<PERSON>::loadTrainPlatform()
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
			std::string lineId = r.get<std::string>(0);
			TrainPlatform platform;
			platform.lineId = lineId;
			platform.platformNo = r.get<std::string>(1);
			platform.sequenceNo = r.get<int>(2);
			if(mapOfIdvsTrainPlatforms.find(lineId)==mapOfIdvsTrainPlatforms.end()) {
				mapOfIdvsTrainPlatforms[lineId] = std::vector<TrainPlatform>();
			}
			mapOfIdvsTrainPlatforms[lineId].push_back(platform);
		}
	}
	template<typename PERSON>
	void TrainController<PERSON>::loadTransferedTimes()
	{

	}
	template<typename PERSON>
	void TrainController<PERSON>::loadBlockPolylines()
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
			point.setSequenceNumber(r.get<int>(4));
			if(mapOfIdvsPolylines.find(polylineId)==mapOfIdvsPolylines.end()) {
				mapOfIdvsPolylines[polylineId] = new PolyLine();
				mapOfIdvsPolylines[polylineId]->setPolyLineId(polylineId);
			}
			mapOfIdvsPolylines[polylineId]->addPoint(point);
		}
	}
	template<typename PERSON>
	void TrainController<PERSON>::assignTrainTripToPerson(std::set<Entity*>& activeAgents)
	{
		const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
		PERSON* person = new PERSON("TrainController", config.mutexStategy());
		std::string lineId="NE_1";
		std::vector<Block*> route;
		std::vector<Platform*> platforms;
		getTrainRoute(lineId, route);
		getTrainPlatforms(lineId, platforms);
		TrainTrip* trainTrip = new TrainTrip();
		trainTrip->setTrainRoute(route);
		trainTrip->setTrainPlatform(platforms);
		trainTrip->setLineId(lineId);
		trainTrip->itemType = TripChainItem::IT_TRAINTRIP;
		std::vector<TripChainItem*> tripChain;
		tripChain.push_back(trainTrip);
		person->setTripChain(tripChain);
		activeAgents.insert(person);
	}
	template<typename PERSON>
	Agent* TrainController<PERSON>::getAgentFromStation(const std::string& nameStation)
	{
		std::map<std::string, Station*>& mapOfIdvsStations = getInstance()->mapOfIdvsStations;
		std::map<std::string, Station*>::const_iterator it = mapOfIdvsStations.find(nameStation);
		if(it!=mapOfIdvsStations.end()) {
			if(allStationAgents.find(it->second)!=allStationAgents.end()) {
				return allStationAgents[it->second];
			}
		}
		return nullptr;
	}
	template<typename PERSON>
	void TrainController<PERSON>::registerStationAgent(const std::string& nameStation, Agent* stationAgent)
	{
		std::map<std::string, Station*>& mapOfIdvsStations = getInstance()->mapOfIdvsStations;
		std::map<std::string, Station*>::const_iterator it = mapOfIdvsStations.find(nameStation);
		if(it!=mapOfIdvsStations.end()) {
			allStationAgents[it->second]=stationAgent;
		}
	}
} /* namespace sim_mob */
#endif
