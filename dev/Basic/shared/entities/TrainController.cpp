/*
 * TrainController.cpp
 *
 *  Created on: Feb 11, 2016
 *      Author: zhang huai peng
 */
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string.hpp>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "logging/Log.hpp"
#include "message/MessageBus.hpp"
#include "event/SystemEvents.hpp"
#include "event/args/ReRouteEventArgs.hpp"
#include "util/Profiler.hpp"
#include "util/CSVReader.hpp"
#ifndef _CLASS_TRAIN_CONTROLLER_FUNCTIONS
#include "entities/TrainController.hpp"
#else
namespace
{
const double MILLISECS_CONVERT_UNIT = 1000.0;
}
namespace sim_mob
{
	template<typename PERSON>
	TrainController<PERSON>* TrainController<PERSON>::pInstance=nullptr;
	template<typename PERSON>
	boost::unordered_map<const Station*, Agent*> TrainController<PERSON>::allStationAgents;
	template<typename PERSON>
	TrainController<PERSON>::TrainController(int id, const MutexStrategy& mtxStrat):Agent(mtxStrat, id),disruptionParam(nullptr)
	{

	}
	template<typename PERSON>
	TrainController<PERSON>::~TrainController()
	{
		for (std::map<std::string, Platform*>::iterator it =
				mapOfIdvsPlatforms.begin(); it != mapOfIdvsPlatforms.end(); it++)
		{
			delete it->second;
		}
		for (std::map<unsigned int, Block*>::iterator it =
				mapOfIdvsBlocks.begin(); it != mapOfIdvsBlocks.end(); it++)
		{
			delete it->second;
		}
		for (std::map<unsigned int, PolyLine*>::iterator it =
				mapOfIdvsPolylines.begin(); it != mapOfIdvsPolylines.end(); it++)
		{
			delete it->second;
		}
		if(pInstance)
		{
			delete pInstance;
		}
	}


	template<typename PERSON>
	void TrainController<PERSON>::setDisruptionParams(std::string startStation,std::string endStation,std::string time)
	{
		disruptionEntity.startStation=startStation;
		disruptionEntity.endStation=endStation;
		disruptionEntity.disruptionTime=time;
	}

	template<typename PERSON>
	void TrainController<PERSON>::performDisruption(std::string startStation,std::string endStation,timeslice now,std::string disruptionTime)
	{
		DailyTime currentTime=ConfigManager::GetInstance().FullConfig().simStartTime()+DailyTime(now.ms());
		DailyTime nextInterval=ConfigManager::GetInstance().FullConfig().simStartTime()+DailyTime(now.ms()+5000);
		std::string currentTimeStr=currentTime.getStrRepr();
		std::string nextIntervalStr=nextInterval.getStrRepr();
		if((boost::iequals(currentTimeStr,disruptionTime)||nextIntervalStr>disruptionTime)&&disruptionPerformed==false)
		{
			std::vector<std::string> platforms=getPlatformsBetweenStations("NE_1",startStation,endStation);
			disruptedPlatformsNamesMap_ServiceController["NE_1"]=std::vector<std::string>();
			disruptedPlatformsNamesMap_ServiceController["NE_1"].insert(disruptedPlatformsNamesMap_ServiceController["NE_1"].end(),platforms.begin(),platforms.end());
			platforms=getPlatformsBetweenStations("NE_2",endStation,startStation);
			disruptedPlatformsNamesMap_ServiceController["NE_2"]=std::vector<std::string>();
			disruptedPlatformsNamesMap_ServiceController["NE_2"].insert(disruptedPlatformsNamesMap_ServiceController["NE_2"].end(),platforms.begin(),platforms.end());
			disruptionPerformed=true;
		}
	}

	template<typename PERSON>
	void TrainController<PERSON>::loadOppositeLines()
	{
		const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
		const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;
		std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("pt_opposite_lines");
		if(spIt == storedProcs.end())
		{
			Print() << "missing stored procedure for get_pt_opposite_lines" << std::endl;
			return;
		}
		soci::session sql_(soci::postgresql, configParams.getDatabaseConnectionString(false));
		soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + spIt->second);
		for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
		{
			const soci::row& r = (*it);
			std::string lineId = r.get<std::string>(0);
			std::string opp_lineId = r.get<std::string>(1);
			mapOfOppositeLines[lineId]=opp_lineId;
		}
	}

	template<typename PERSON>
	void TrainController<PERSON>::loadUTurnPlatforms()
	{
		const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
		const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;
		std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("get_uturn_platforms");
		if(spIt == storedProcs.end())
		{
			Print() << "missing stored procedure for get_pt_opposite_lines" << std::endl;
			return;
		}
		soci::session sql_(soci::postgresql, configParams.getDatabaseConnectionString(false));
		soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + spIt->second);
		for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
		{
			const soci::row& r = (*it);
			std::string platformNo = r.get<std::string>(0);
			std::string lineId = r.get<std::string>(1);
			mapOfUturnPlatformsLines[lineId].push_back(platformNo);
		}
	}

	template<typename PERSON>
	void TrainController<PERSON>::loadTrainAvailabilities()
	{
		const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
		const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;
		std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("get_train_fleet");
		if(spIt == storedProcs.end())
		{
			Print() << "missing stored procedure for get_train_fleet" << std::endl;
			return;
		}
		soci::session sql_(soci::postgresql, configParams.getDatabaseConnectionString(false));
		soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + spIt->second);
		for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
		{
			const soci::row& r = (*it);
			std::string lineId = r.get<std::string>(0);
			int min_id = r.get<int>(1);
			int max_id = r.get<int>(2);
			std::list<int> trainIdsRange;
			trainIdsRange.push_back(min_id);
			trainIdsRange.push_back(max_id);
			mapOfTrainMaxMinIds[lineId]=trainIdsRange;
		}
	}

	template<typename PERSON>
	void TrainController<PERSON>::loadWalkingTimeParams()
	{
		std::string filename = "walkingTimeParams.csv";
		CSV_Reader variablesReader(filename, true);
		boost::unordered_map<std::string, std::string> variableRow;
		variablesReader.getNextRow(variableRow, false);
		while (!variableRow.empty())
		{
			try
			{
				WalkingTimeParams walkingParams;
				walkingParams.stationName = variableRow.at("name");
				walkingParams.alpha = boost::lexical_cast<double>(variableRow.at("alpha"));
				walkingParams.beta = boost::lexical_cast<double>(variableRow.at("beta"));
				walkingTimeAtStation[walkingParams.stationName] = walkingParams;
			} catch (const std::out_of_range& oor) {
				throw std::runtime_error("Header mis-match while reading walking Time params csv");
			} catch (boost::bad_lexical_cast const&) {
				throw std::runtime_error("Invalid value found in walking time csv");
			}
			variableRow.clear();
			variablesReader.getNextRow(variableRow, false);
		}
	}

	template<typename PERSON>
	void TrainController<PERSON>::setDisruptedPlatforms(std::string startStation,std::string endStation,std::string lineID)
	{
		std::vector<std::string> platforms=getPlatformsBetweenStations(lineID,startStation,endStation);
		disruptedPlatformsNamesMap_ServiceController[lineID] = std::vector<std::string>();
		disruptedPlatformsNamesMap_ServiceController[lineID].insert(disruptedPlatformsNamesMap_ServiceController[lineID].end(),platforms.begin(),platforms.end());
	}

	template<typename PERSON>
	void TrainController<PERSON>::clearDisruption(std::string lineID)
	{
		disruptedPlatformsNamesMap_ServiceController[lineID].erase(disruptedPlatformsNamesMap_ServiceController[lineID].begin(),disruptedPlatformsNamesMap_ServiceController[lineID].end());
		disruptedPlatformsNamesMap_ServiceController.erase(lineID);
	}

	template<typename PERSON>
	Entity::UpdateStatus TrainController<PERSON>::frame_tick(timeslice now)
	{

		if(disruptionParam.get())
        {
			unsigned int baseGran = ConfigManager::GetInstance().FullConfig().baseGranMS();
			DailyTime duration = disruptionParam->duration;
			if(duration.getValue()>baseGran)
			{
				disruptionParam->duration = DailyTime(duration.offsetMS_From(DailyTime(baseGran)));
			}
			else
			{
				disruptionParam.reset();
			}
		}

		std::map<std::string,std::vector<TrainTrip*>>::iterator unscheduledTripItr;
		for(unscheduledTripItr = mapOfIdvsUnsheduledTrips.begin(); unscheduledTripItr != mapOfIdvsUnsheduledTrips.end(); unscheduledTripItr++)
		{
			std::vector<TrainTrip*>& trainTrips = unscheduledTripItr->second;
			std::vector<TrainTrip*>::iterator trItr = trainTrips.begin();
			int starttime = (*trItr)->getStartTime();
			while(!trainTrips.empty()&&(*trItr)->getStartTime()<=now.ms())
			{

				const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
				PERSON* person = new PERSON("TrainController", config.mutexStategy());
				std::vector<TripChainItem*> tripChain;
				TrainTrip* top = *trItr;
				std::string lineId = top->getLineId();
				if(!isServiceTerminated(lineId))
				{
					int trainId = getTrainId(lineId);
					if(trainId != -1)
					{
						if(disruptionParam.get())
						{
							changeTrainTrip(top, disruptionParam.get());
						}
						top->setTrainId(trainId);
						tripChain.push_back(top);
						person->setTripChain(tripChain);
						person->setStartTime(top->getStartTime());
						trainTrips.erase(trItr);
						this->currWorkerProvider->scheduleForBred(person);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
				trItr = trainTrips.begin();
			}
		}
		std::map<std::string, TripStartTimePriorityQueue>::iterator it;
		for(it = mapOfIdvsTrip.begin(); it != mapOfIdvsTrip.end(); it++)
		{
			TripStartTimePriorityQueue& trainTrips = it->second;
			std::string lineId = it->first;
			while(!trainTrips.empty() && trainTrips.top()->getStartTime()<=now.ms())
			{
				const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
				PERSON* person = new PERSON("TrainController", config.mutexStategy());
				std::vector<TripChainItem*> tripChain;
				TrainTrip* top = trainTrips.top();
				std::string lineId = top->getLineId();
				if(!isServiceTerminated(lineId))  //This can be checked before
				{
					int trainId = getTrainId(lineId);
					if(trainId != -1)
					{
						if(disruptionParam.get())
						{
							changeTrainTrip(top, disruptionParam.get());
						}
						top->setTrainId(trainId);
						tripChain.push_back(top);
						person->setTripChain(tripChain);
						person->setStartTime(top->getStartTime());
						trainTrips.pop();
						this->currWorkerProvider->scheduleForBred(person);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
		}
		return Entity::UpdateStatus::Continue;
	}
	template<typename PERSON>
	void TrainController<PERSON>::changeTrainTrip(sim_mob::TrainTrip* trip, sim_mob::DisruptionParams* params)
	{
		std::map<std::string, std::vector<std::string>> platformsInline;
		for(size_t i=0; i<params->platformNames.size(); i++)
		{
			platformsInline[params->platformLineIds[i]].push_back(params->platformNames[i]);
		}
		std::string lineId = trip->getLineId();
		std::map<std::string, std::vector<std::string>>::iterator it;
		it = platformsInline.find(lineId);
		if(it!=platformsInline.end())
		{
			trip->removeTrainRoute(it->second);
		}
	}
	template<typename PERSON>
	int TrainController<PERSON>::getTrainId(const std::string& lineID)
	{
		int trainId = 0;
		trainId=deleteTrainFromActivePool(lineID);
		if(trainId == -1)
	    {
			return -1;
		}
		return trainId;
	}

	template<typename PERSON>
	const WalkingTimeParams* TrainController<PERSON>::getWalkingTimeParams(const std::string& station) const
	{
		const WalkingTimeParams* res = nullptr;
		auto it = walkingTimeAtStation.find(station);
		if(it!=walkingTimeAtStation.end())
		{
			res = &it->second;
		}
		return res;
	}

	template<typename PERSON>
	void TrainController<PERSON>::addToListOfActiveTrainsInLine(std::string lineId,Role<PERSON> *driver)
	{
		activeTrainsListLock.lock();
		mapOfLineAndTrainDrivers[lineId].push_back(driver);
		activeTrainsListLock.unlock();
	}

	template<typename PERSON>
	double TrainController<PERSON>::getMaximumDwellTime(std::string lineId) const
	{
		const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
		const std::map<const std::string,TrainProperties> &trainLinePropertiesMap = config.trainController.trainLinePropertiesMap;
		const TrainProperties &trainProperties = trainLinePropertiesMap.find(lineId)->second;
		return trainProperties.dwellTimeInfo.maxDwellTime;
	}

	template<typename PERSON>
	void TrainController<PERSON>::removeFromListOfActiveTrainsInLine(std::string lineId,Role<PERSON> *driver)
	{
		activeTrainsListLock.lock();
		if(mapOfLineAndTrainDrivers.find(lineId) != mapOfLineAndTrainDrivers.end())
		{
			std::vector <Role<PERSON>*> &vect = mapOfLineAndTrainDrivers[lineId];
			typename std::vector <sim_mob::Role<PERSON>*>::iterator it = vect.begin();
			it = find(vect.begin(),vect.end(),driver);
			if(it != vect.end())
			{
				vect.erase(it);
			}
		}
		activeTrainsListLock.unlock();
	}

	template<typename PERSON>
	typename  std::vector <Role<PERSON>*> TrainController<PERSON>::getActiveTrainsForALine(std::string lineID)
	{
		activeTrainsListLock.lock();
		std::vector <Role<PERSON>*> activeTrains = mapOfLineAndTrainDrivers[lineID];
		activeTrainsListLock.unlock();
		return activeTrains;
	}

	template<typename PERSON>
	Entity::UpdateStatus TrainController<PERSON>::frame_init(timeslice now)
	{
		messaging::MessageBus::SubscribeEvent(event::EVT_CORE_MRT_DISRUPTION, this);
		return Entity::UpdateStatus::Continue;
	}

	template<typename PERSON>
	std::string TrainController<PERSON>::getOppositeLineId(std::string lineId)
	{
		std::map<std::string,std::string>::iterator it = mapOfOppositeLines.find(lineId);
		if(it != mapOfOppositeLines.end())
		{
			return it->second;
		}
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
	bool TrainController<PERSON>::getTrainRoute(const std::string& lineId, std::vector<Block*>& route) const
	{
		bool res = true;
		std::map<std::string, std::vector<TrainRoute>>::const_iterator it = mapOfIdvsTrainRoutes.find(lineId);
		if(it == mapOfIdvsTrainRoutes.end())
		{
			res = false;
		}
		else
		{
			const std::vector<TrainRoute>& trainRoute = it->second;
			for(std::vector<TrainRoute>::const_iterator i = trainRoute.begin();i!=trainRoute.end();i++)
			{
				std::map<unsigned int, Block*>::const_iterator iBlock = mapOfIdvsBlocks.find(i->blockId);
				if(iBlock == mapOfIdvsBlocks.end())
				{
					res = false;
					break;
				}
				else
				{
					route.push_back(iBlock->second);
				}
			}
		}
		return res;
	}
	template<typename PERSON>
	bool TrainController<PERSON>::getTrainPlatforms(const std::string& lineId, std::vector<Platform*>& platforms) const
	{
		bool res = true;
		std::map<std::string, std::vector<TrainPlatform>>::const_iterator it = mapOfIdvsTrainPlatforms.find(lineId);
		if(it == mapOfIdvsTrainPlatforms.end())
		{
			res = false;
		}
		else
		{
			const std::vector<TrainPlatform>& trainPlatform = it->second;
			for(std::vector<TrainPlatform>::const_iterator i = trainPlatform.begin();i!=trainPlatform.end();i++)
			{
				std::map<std::string, Platform*>::const_iterator iPlatform = mapOfIdvsPlatforms.find(i->platformNo);
				if(iPlatform == mapOfIdvsPlatforms.end())
				{
					res = false;
					break;
				}
				else
				{
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
		loadUTurnPlatforms();
		loadBlocks();
		loadOppositeLines();
		loadTrainAvailabilities();
		loadTrainRoutes();
		loadTrainPlatform();
		loadTransferedTimes();
		loadBlockPolylines();
		composeBlocksAndPolyline();
		loadSchedules();
		composeTrainTrips();
		loadTrainLineProperties();
		loadWalkingTimeParams();
	}

	//Initialize Train ids function
	template<typename PERSON>
	void TrainController<PERSON>::InitializeTrainIds(std::string lineId)
	{
		std::list<int> &listOfTrainMaxMinIds = mapOfTrainMaxMinIds[lineId];
		std::list<int>::iterator itr = listOfTrainMaxMinIds.begin();
		int minId = *itr;
		int maxId = (*(++itr));
		int count = 0;
		std::vector<int> trainIds = std::vector<int>();
		for(int i = minId; i<=maxId; i++)
		{
			trainIds.push_back(i);
			count++;
		}
		recycleTrainId[lineId] = trainIds;
		mapOfNoAvailableTrains[lineId] = count;
	}


	template<typename PERSON>
	void TrainController<PERSON>::composeBlocksAndPolyline()
	{
		for (std::map<unsigned int, Block*>::iterator it = mapOfIdvsBlocks.begin();it != mapOfIdvsBlocks.end(); it++)
		{
			std::map<unsigned int, PolyLine*>::iterator itLine = mapOfIdvsPolylines.find(it->first);
			if (itLine != mapOfIdvsPolylines.end())
			{
				it->second->setPloyLine((*itLine).second);
			}
			else
			{
				Print()<< "Block not find polyline:"<<it->first<<std::endl;
			}
		}

		for(std::map<std::string, Platform*>::iterator it = mapOfIdvsPlatforms.begin(); it != mapOfIdvsPlatforms.end(); it++)
		{
			std::map<unsigned int, Block*>::iterator itBlock = mapOfIdvsBlocks.find(it->second->getAttachedBlockId());
			if(itBlock != mapOfIdvsBlocks.end())
			{
				itBlock->second->setAttachedPlatform(it->second);
			}
			else
			{
				Print()<< "Platform not find corresponding block:"<<it->first<<std::endl;
			}
		}
	}

	template<typename PERSON>
	TrainController<PERSON>* TrainController<PERSON>::getInstance()
	{
		if(!pInstance)
		{
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
			platform->setType(PlatformType(r.get<int>(4)));
			platform->setAttachedBlockId(r.get<int>(5));
			platform->setOffset(r.get<double>(6));
			platform->setLength(r.get<double>(7));
			mapOfIdvsPlatforms[platformNo] = platform;
			if(mapOfIdvsStations.find(stationNo)==mapOfIdvsStations.end())
			{
				mapOfIdvsStations[stationNo] = new Station(stationNo);
			}
			Station* station = mapOfIdvsStations[stationNo];
			station->addPlatform(platform->getLineId(), platform);
		}
	}

	template<typename PERSON>
	double TrainController<PERSON>::getMinDwellTime(std::string stationNo,std::string lineId) const
	{
		const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
		const std::map<const std::string,TrainProperties> &trainLinePropertiesMap = config.trainController.trainLinePropertiesMap;
		const TrainProperties &trainProperties = trainLinePropertiesMap.find(lineId)->second;
		double minDwellTime = trainProperties.dwellTimeInfo.dwellTimeAtNormalStation;
		if(stationNo.find("/")!= std::string::npos)
		{
			minDwellTime = trainProperties.dwellTimeInfo.dwellTimeAtInterchanges;
		}

		std::map<std::string, std::vector<TrainPlatform>>::const_iterator it = mapOfIdvsTrainPlatforms.find(lineId);
		const std::vector<TrainPlatform> TrainPlatforms = it->second;
		std::vector<TrainPlatform>::const_iterator itTrainPlatforms = TrainPlatforms.begin();
		Platform *platform = mapOfIdvsPlatforms.find((*itTrainPlatforms).platformNo)->second;
		if(boost::iequals(platform->getStationNo(),stationNo))
		{
			minDwellTime = trainProperties.dwellTimeInfo.dwellTimeAtTerminalStaions;
		}

		else
		{
			std::vector<TrainPlatform>::const_iterator itTrainPlatforms = TrainPlatforms.end()-1;
			Platform *platform = mapOfIdvsPlatforms.find((*itTrainPlatforms).platformNo)->second;
			if(boost::iequals(platform->getStationNo(),stationNo))
			{
				minDwellTime = trainProperties.dwellTimeInfo.dwellTimeAtTerminalStaions;
			}
		}
		return minDwellTime;
	}

	template<typename PERSON>
	bool TrainController<PERSON>::isFirstStation(std::string lineId,Platform *platform) const
	{
		std::map<std::string, std::vector<TrainPlatform>>::const_iterator it = mapOfIdvsTrainPlatforms.find(lineId);
		if(it != mapOfIdvsTrainPlatforms.end())
		{
			const std::vector<TrainPlatform> TrainPlatforms = it->second;
			std::vector<TrainPlatform>::const_iterator itTrainPlatforms = TrainPlatforms.begin();
			if(boost::iequals((*itTrainPlatforms).platformNo,platform->getPlatformNo()))
			{
				return true;
			}
		}
		return false;
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
			if(mapOfIdvsSchedules.find(lineId) == mapOfIdvsSchedules.end())
			{
				mapOfIdvsSchedules[lineId] = std::vector<TrainSchedule>();
			}
			mapOfIdvsSchedules[lineId].push_back(schedule);
		}
	}

	template<typename PERSON>
	void TrainController<PERSON>::composeTrainTrips()
	{
		int tripId = 1;
		std::map<std::string, std::vector<TrainSchedule>>::const_iterator it;
		for(it = mapOfIdvsSchedules.begin(); it != mapOfIdvsSchedules.end(); it++)
		{
			std::string lineId = it->first;
			boost::algorithm::erase_all(lineId, " ");
			std::vector<TrainSchedule>::const_iterator iSchedule;
			const std::vector<TrainSchedule>& schedules = it->second;
			std::vector<Block*> route;
			std::vector<Platform*> platforms;
			getTrainRoute(lineId, route);
			getTrainPlatforms(lineId, platforms);
			for(iSchedule=schedules.begin(); iSchedule!=schedules.end(); iSchedule++)
			{
				DailyTime startTime(iSchedule->startTime);
				DailyTime endTime(iSchedule->endTime);
				DailyTime advance(iSchedule->headwaySec*MILLISECS_CONVERT_UNIT);
				for(DailyTime time = startTime; time.isBeforeEqual(endTime); time += advance)
				{
					TrainTrip* trainTrip = new TrainTrip();
					trainTrip->setTrainRoute(route);
					trainTrip->setTrainPlatform(platforms);
					trainTrip->setLineId(lineId);
					trainTrip->setTripId(tripId++);
					DailyTime start(time.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime()));
					trainTrip->setStartTime(start);
					trainTrip->itemType = TripChainItem::IT_TRAINTRIP;
					if(mapOfIdvsTrip.find(lineId) == mapOfIdvsTrip.end())
					{
						mapOfIdvsTrip[lineId] = TripStartTimePriorityQueue();
					}
					mapOfIdvsTrip[lineId].push(trainTrip);
				}
			}
		}
		maxTripId=tripId;
	}

	template<typename PERSON>
	void TrainController<PERSON>::composeTrainTripUnScheduled(std::string lineId,std::string startTime,std::string startStation)
	{
		TrainTrip* trainTrip = new TrainTrip();
		trainTrip->SetScheduledStatus(true);
		std::vector<Block*> route;
		getTrainRoute(lineId, route);
		trainTrip->setLineId(lineId);
		int tripId = ++maxTripId;
		trainTrip->setTripId(tripId);
		trainTrip->setTrainRoute(route);
		std::vector<Platform*> platforms = getPlatforms(lineId,startStation);
		trainTrip->setTrainPlatform(platforms);
		DailyTime start(startTime);
		DailyTime offset(start.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime()));
		trainTrip->setStartTime(offset);
		trainTrip->itemType = TripChainItem::IT_TRAINTRIP;
		if(mapOfIdvsUnsheduledTrips.find(lineId) == mapOfIdvsUnsheduledTrips.end())
		{
			mapOfIdvsUnsheduledTrips[lineId] = std::vector<TrainTrip*>();
	    }

		std::vector<TrainTrip*>::iterator it = mapOfIdvsUnsheduledTrips[lineId].begin();
        bool flag = false;
		while(it != mapOfIdvsUnsheduledTrips[lineId].end())
		{
			int startTime = (*it)->getStartTime();
			if(startTime > trainTrip->getStartTime())
			{
				mapOfIdvsUnsheduledTrips[lineId].insert(it,trainTrip);
				flag=true;
				break;
			}
			it++;
		}

		if(flag==false)
		{
			mapOfIdvsUnsheduledTrips[lineId].push_back(trainTrip);
		}
	}

	template<typename PERSON>
	std::vector<Platform*> TrainController<PERSON>::getPlatforms(std::string lineId,std::string startStation)
	{
		std::vector<Platform*> platforms;
		if(mapOfIdvsStations.find(startStation) != mapOfIdvsStations.end())
		{
			Station *station = mapOfIdvsStations[startStation];
			if(station)
			{
				const Platform *platform = station->getPlatform(lineId);
				if(mapOfIdvsTrainPlatforms.find(lineId) != mapOfIdvsTrainPlatforms.end() )
				{
					std::vector<TrainPlatform>& trainPlatforms = mapOfIdvsTrainPlatforms[lineId];
					std::vector<TrainPlatform>::const_iterator it = trainPlatforms.begin();
					if(it != trainPlatforms.end())
					{
						bool foundStartPlatform = false;
						while(it != trainPlatforms.end())
						{
							std::string platformNo=(*it).platformNo;
							if(boost::iequals(platformNo,platform->getPlatformNo()))
							{
								foundStartPlatform = true;
							}

							if(foundStartPlatform)
							{
								if(mapOfIdvsPlatforms.find(platformNo) != mapOfIdvsPlatforms.end())
								{
									Platform *plt = mapOfIdvsPlatforms[platformNo];
									platforms.push_back(plt);
								}
								else
								{
									platforms.erase(platforms.begin(),platforms.end());
									break;
								}
							}

							it++;
						}
					}
				}
			}
		}
        return platforms;
	}

	template<typename PERSON>
	void TrainController<PERSON>::loadBlocks()
	{
		const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
		const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;
		std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("pt_mrt_block");
		if(spIt == storedProcs.end())
		{
			Print() << "missing stored procedure for pt_mrt_block" << std::endl;
			return;
		}
		soci::session sql_(soci::postgresql, configParams.getDatabaseConnectionString(false));
		soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + spIt->second);
		for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
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

	template<typename PERSON>
	TrainPlatform TrainController<PERSON>::getNextPlatform(std::string platformNo,std::string lineID)
	{
		std::vector<TrainPlatform>& trainPlatforms = mapOfIdvsTrainPlatforms[lineID];
		typename std::vector<TrainPlatform>::const_iterator it = trainPlatforms.begin();
		while(it != trainPlatforms.end())
		{
			if(boost::iequals((*it).platformNo,platformNo))
			{
				break;
			}
			it++;
		}
        if(it!=trainPlatforms.end())
        {
			it++;
			if(it != trainPlatforms.end())
			{
				return (*it);
			}
        }
	}

	template<typename PERSON>
	bool TrainController<PERSON>::isTerminalPlatform(std::string platformNo,std::string lineID)
	{
		if(mapOfIdvsTrainPlatforms.find(lineID) == mapOfIdvsTrainPlatforms.end())
		{
			return false;
		}
		std::vector<TrainPlatform>& trainPlatforms = mapOfIdvsTrainPlatforms[lineID];
		TrainPlatform endPlt = *(trainPlatforms.end()-1);
		if(boost::iequals(platformNo,endPlt.platformNo) == true)
		{
			return true;
		}
		return false;
	}

	template<typename PERSON>
	Platform* TrainController<PERSON>::getPlatformFromId(std::string platformNo)
	{
		if(mapOfIdvsPlatforms.find(platformNo)!=mapOfIdvsPlatforms.end())
		{
			return mapOfIdvsPlatforms[platformNo];
		}
		else
		{
			return nullptr;
		}
	}

	template<typename PERSON>
	std::vector<Block*> TrainController<PERSON>::getBlocks(std::string lineId)
	{
		std::map<std::string, std::vector<TrainRoute>>::const_iterator it = mapOfIdvsTrainRoutes.find(lineId);
		std::vector<Block*> blockVector;

		if(it==mapOfIdvsTrainRoutes.end())
		{
			//res = false;
		}

		else
		{
			const std::vector<TrainRoute>& trainRoute = it->second;
			for(std::vector<TrainRoute>::const_iterator i = trainRoute.begin(); i != trainRoute.end(); i++)
			{

				std::map<unsigned int, Block*>::iterator iBlock = mapOfIdvsBlocks.find(i->blockId);
				blockVector.push_back(iBlock->second);
			}
		}
		return blockVector;
	}

	template<typename PERSON>
    Block* TrainController<PERSON>::getBlock(int blockId)
	{
		std::map<unsigned int, Block*>::iterator iBlock = mapOfIdvsBlocks.find(blockId);
		return iBlock->second;
	}


	template<typename PERSON>
	std::vector<std::string> TrainController<PERSON>::getLinesBetweenTwoStations(std::string src,std::string dest)
	{
		typename std::map<std::string, std::vector<TrainRoute>>::const_iterator it = mapOfIdvsTrainRoutes.begin();
		std::vector<std::string> lines;
		while(it != mapOfIdvsTrainRoutes.end())
		{
			const std::vector<TrainRoute>& route = it->second;
			std::vector<TrainRoute> ::const_iterator tritr = route.begin();
			bool originFound=false;
            while(tritr != route.end())
            {
            	int blockId = (*tritr).blockId;
            	typename std::map<unsigned int, Block*>::const_iterator iBlock = mapOfIdvsBlocks.find(blockId);
            	Block *block = (iBlock)->second;
            	Platform *plt = block->getAttachedPlatform();
            	if(plt)
            	{
            		std::string stationNo = plt->getStationNo();
            		if(boost::iequals(stationNo, src))
            		{
            			originFound=true;
            		}
            		else if(boost::iequals(stationNo, dest)&&originFound==true)
            		{
            			lines.push_back((*tritr).lineId);
            			break;
            		}
            	}
            }
            it++;
		}
		return lines;
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
		for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			const soci::row& r = (*it);
			std::string lineId = r.get<std::string>(0);
			TrainRoute route;
			route.lineId = lineId;
			route.blockId = r.get<int>(1);
			route.sequenceNo = r.get<int>(2);
			if(mapOfIdvsTrainRoutes.find(lineId) == mapOfIdvsTrainRoutes.end())
			{
				mapOfIdvsTrainRoutes[lineId] = std::vector<TrainRoute>();
			}
			mapOfIdvsTrainRoutes[lineId].push_back(route);
			mapOfTrainServiceTerminated[lineId]=false;
			InitializeTrainIds(lineId);
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
			if(mapOfIdvsTrainPlatforms.find(lineId)==mapOfIdvsTrainPlatforms.end())
			{
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
	std::vector<std::string> TrainController<PERSON>::getPlatformsBetweenStations(std::string lineId,std::string startStation,std::string endStation)
	{
		std::vector<std::string> platforms;
		if(mapOfIdvsStations.find(startStation) == mapOfIdvsStations.end() || mapOfIdvsStations.find(endStation) == mapOfIdvsStations.end())
		{
			return platforms;
		}
		Station *station = mapOfIdvsStations[startStation];
		Station *eStation = mapOfIdvsStations[endStation];

		if(station&&eStation)
		{
			Platform *platform = station->getPlatform(lineId);
			Platform *endPlatform = eStation->getPlatform(lineId);
			if(mapOfIdvsTrainPlatforms.find(lineId) != mapOfIdvsTrainPlatforms.end())
			{
				std::vector<TrainPlatform> &trainPlatforms = mapOfIdvsTrainPlatforms[lineId];
				std::vector<TrainPlatform>::const_iterator it = trainPlatforms.begin();
				if(it != trainPlatforms.end())
				{
					bool foundStartPlatform = false;
					while(it != trainPlatforms.end())
					{
						std::string platformNo = (*it).platformNo;
						if(boost::iequals(platformNo,platform->getPlatformNo()))
						{
							foundStartPlatform = true;
						}
						if(foundStartPlatform)
						{
							Platform *plt = mapOfIdvsPlatforms.find(platformNo)->second;
							platforms.push_back(plt->getPlatformNo());
						}

						if(boost::iequals(platformNo,endPlatform->getPlatformNo()))
						{
							break;
						}
						it++;
					}
				}
			}
		}
		return platforms;
	}

	template<typename PERSON>
	void TrainController<PERSON>::loadBlockPolylines()
	{
		const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
		const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;
		std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("pt_mrt_block_polyline");
		if(spIt == storedProcs.end())
		{
			Print() << "missing stored procedure for pt_mrt_block_polyline" << std::endl;
			return;
		}
		soci::session sql_(soci::postgresql, configParams.getDatabaseConnectionString(false));
		soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + spIt->second);
		for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
		{
			const soci::row& r = (*it);
			int polylineId = r.get<int>(0);
			PolyPoint point;
			point.setPolyLineId(polylineId);
			point.setX(r.get<double>(1));
			point.setY(r.get<double>(2));
			point.setZ(r.get<double>(3));
			point.setSequenceNumber(r.get<int>(4));
			if(mapOfIdvsPolylines.find(polylineId) == mapOfIdvsPolylines.end())
			{
				mapOfIdvsPolylines[polylineId] = new PolyLine();
				mapOfIdvsPolylines[polylineId]->setPolyLineId(polylineId);
			}
			mapOfIdvsPolylines[polylineId]->addPoint(point);
		}
	}

	template<typename PERSON>
	void TrainController<PERSON>::printBlocks(std::ofstream& out) const
	{
		std::stringstream outStream;
		outStream << std::setprecision(8);
		outStream << "Print train network(blocks)" << std::endl;
		for (std::map<unsigned int, Block*>::const_iterator it = mapOfIdvsBlocks.begin();it != mapOfIdvsBlocks.end(); it++)
		{
			outStream << "(\"block\", " << it->second->getBlockId() << ", {";
			outStream << "\"length\":\"" << it->second->getLength() << "\",";
			outStream << "\"speedlimit\":\"" << it->second->getSpeedLimit() << "\",";
			outStream << "\"acceleration\":\"" << it->second->getAccelerateRate() << "\",";
			outStream << "\"deceleration\":\"" << it->second->getDecelerateRate() << "\",";
			outStream << "\"points\":\"[";
			const PolyLine *polyLine = it->second->getPolyLine();
			for (std::vector<PolyPoint>::const_iterator itPts = polyLine->getPoints().begin(); itPts != polyLine->getPoints().end(); ++itPts)
			{
				outStream << "(" << itPts->getX() << "," << itPts->getY() << "),";
			}
			outStream << "]\",";
			outStream << "})\n";
		}
		out << outStream.str() << std::endl;
	}

	template<typename PERSON>
	void TrainController<PERSON>::printPlatforms(std::ofstream& out) const
	{
		std::stringstream outStream;
		outStream << std::setprecision(8);
		outStream << "Print train network(platforms)" << std::endl;
		for(std::map<std::string, Platform*>::const_iterator it = mapOfIdvsPlatforms.begin(); it != mapOfIdvsPlatforms.end(); it++)
		{
			const Platform *platform = it->second;
			outStream << "(\"platform\", " << platform->getPlatformNo() << ", {";
			outStream << "\"station\":\"" << platform->getStationNo() << "\",";
			outStream << "\"lineId\":\"" << platform->getLineId() << "\",";
			outStream << "\"block\":\"" << platform->getAttachedBlockId() << "\",";
			outStream << "\"length\":\"" << platform->getLength() << "\",";
			outStream << "\"offset\":\"" << platform->getOffset() << "\",";
			outStream << "})\n";
		}
		out << outStream.str() << std::endl;
	}

	template<typename PERSON>
	void TrainController<PERSON>::printTrainNetwork(const std::string& outFileName) const
	{
		std::ofstream out(outFileName.c_str());
		printBlocks(out);
		printPlatforms(out);
	}

	template<typename PERSON>
	void TrainController<PERSON>::assignTrainTripToPerson(std::set<Entity*>& activeAgents)
	{
	}

	template<typename PERSON>
	void TrainController<PERSON>::unregisterChild(Entity* child)
	{

	}

	template<typename PERSON>
	Agent* TrainController<PERSON>::getAgentFromStation(const std::string& nameStation)
	{
		const std::map<std::string, Station*>& mapOfIdvsStations = getInstance()->mapOfIdvsStations;
		std::map<std::string, Station*>::const_iterator it = mapOfIdvsStations.find(nameStation);
		if(it!=mapOfIdvsStations.end())
		{
			if(allStationAgents.find(it->second) != allStationAgents.end())
			{
				return allStationAgents[it->second];
			}
		}
		return nullptr;
	}

	template<typename PERSON>
	Platform* TrainController<PERSON>::getPlatform(const std::string& lineId, const std::string& stationName)
	{
		Platform* platform = nullptr;
		std::map<std::string, Station*>& mapOfIdvsStations = getInstance()->mapOfIdvsStations;
		std::map<std::string, Station*>::const_iterator it = mapOfIdvsStations.find(stationName);
		if(it!=mapOfIdvsStations.end())
		{
			Station* station = it->second;
			platform = station->getPlatform(lineId);
		}
		return platform;
	}

	template<typename PERSON>
	bool TrainController<PERSON>::isUturnPlatform(std::string platformName,std::string lineId)
	{
		std::vector<std::string> uTurnPlatforms = mapOfUturnPlatformsLines[lineId];
		if(disruptedPlatformsNamesMap_ServiceController.find(lineId) != disruptedPlatformsNamesMap_ServiceController.end())
		{
			std::vector<std::string> platformNames = disruptedPlatformsNamesMap_ServiceController[lineId];
			if(std::find(platformNames.begin(),platformNames.end(),platformName) != platformNames.end())
			{
				return false;
			}
		}
		if(std::find(uTurnPlatforms.begin(),uTurnPlatforms.end(),platformName) != uTurnPlatforms.end())
		{
			return true;
		}
		return false;
	}

	template<typename PERSON>
	bool TrainController<PERSON>::isDisruptedPlatform(std::string platformName,std::string lineId)
	{
		std::vector<std::string> disruptedplatforms = disruptedPlatformsNamesMap_ServiceController[lineId];
		if(std::find(disruptedplatforms.begin(),disruptedplatforms.end(),platformName) != disruptedplatforms.end())
		{
			return true;
		}
		return false;
	}

	template<typename PERSON>
	Platform* TrainController<PERSON>::getPrePlatform(const std::string& lineId, const std::string& curPlatform)
	{
		Platform* platform = nullptr;
		TrainController<PERSON>* self = TrainController<PERSON>::getInstance();
		std::vector<Platform*> platforms;
		if(self->getTrainPlatforms(lineId, platforms))
		{
			std::vector<Platform*>::const_iterator it = platforms.begin();
			Platform* prev = nullptr;
			while(it!=platforms.end())
			{
				if((*it)->getPlatformNo() == curPlatform)
				{
					platform = (*it);
					break;
				}
				prev = *it;
				it++;
			}
			if(platform)
			{
				platform = prev;
			}
		}
		return platform;
	}

	template<typename PERSON>
	bool TrainController<PERSON>::isPlatformBeforeAnother(std::string firstPlatfrom ,std::string secondPlatform,std::string lineId)
	{
		std::vector<Platform*> platforms;
		getTrainPlatforms(lineId, platforms);
		bool foundFirstPlatfrom = false;
		std::vector<Platform*>::const_iterator it = platforms.begin();
		while(it != platforms.end())
		{
			if((*it)->getPlatformNo() == firstPlatfrom)
			{
				foundFirstPlatfrom = true;
				it++;
				continue;
			}

			if((*it)->getPlatformNo() == secondPlatform)
			{
				if(foundFirstPlatfrom == true)
				{
					return true;
				}
				return false;
			}
			it++;
		}
		return false;
	}

	template<typename PERSON>
	std::map<std::string,std::vector<std::string>> TrainController<PERSON>::getDisruptedPlatforms_ServiceController()
	{
        return disruptedPlatformsNamesMap_ServiceController;
	}

	template<typename PERSON>
	std::map<std::string,std::vector<std::string>>& TrainController<PERSON>::getUturnPlatforms()
	{
		return mapOfUturnPlatformsLines;
	}

	template<typename PERSON>
	bool TrainController<PERSON>::checkPlatformIsExisted(const Agent* stationAgent, const std::string& platformNo)
	{
		bool res = false;
		boost::unordered_map<const Station*, Agent*>::const_iterator it;
		for(it = allStationAgents.begin(); it!=allStationAgents.end(); it++)
		{
			if((*it).second == stationAgent)
			{
				const std::map<std::string, Platform*>& platforms = (*it).first->getPlatforms();
				std::map<std::string, Platform*>::const_iterator ii;
				for(ii = platforms.begin(); ii!=platforms.end(); ii++)
				{
					if((*ii).second->getPlatformNo() == platformNo)
					{
						res = true;
						break;
					}
				}
				break;
			}
		}
		return res;
	}

	template<typename PERSON>
	void TrainController<PERSON>::registerStationAgent(const std::string& nameStation, Agent* stationAgent)
	{
		std::map<std::string, Station*>& mapOfIdvsStations = getInstance()->mapOfIdvsStations;
		std::map<std::string, Station*>::const_iterator it = mapOfIdvsStations.find(nameStation);
		if(it!=mapOfIdvsStations.end())
		{
			allStationAgents[it->second] = stationAgent;
		}
	}

	template<typename PERSON>
	Station *TrainController<PERSON>::getStationFromId(std::string stationId)
	{
		Station *station=nullptr;
		if(mapOfIdvsStations.find(stationId) != mapOfIdvsStations.end())
		{
			station = mapOfIdvsStations[stationId];
		}
		return station;
	}

	template<typename PERSON>
	void TrainController<PERSON>::terminateTrainService(std::string lineId)
	{
		terminatedTrainServiceLock.lock();
		mapOfTrainServiceTerminated[lineId] = true;
		terminatedTrainServiceLock.unlock();
	}

	template<typename PERSON>
	bool TrainController<PERSON>::isServiceTerminated(std::string lineId)
	{
		bool isTerminated=false;
		terminatedTrainServiceLock.lock();
		isTerminated = mapOfTrainServiceTerminated[lineId];
		terminatedTrainServiceLock.unlock();
		return isTerminated;
	}

	template<typename PERSON>
	void TrainController<PERSON>::pushTrainIntoInActivePool(int trainId,std::string lineId)
	{
		trainId = deleteTrainFromActivePool(lineId);
		if(trainId != -1)
		{
			addTrainToInActivePool(lineId, trainId);
		}
	}

	template<typename PERSON>
	int  TrainController<PERSON>::deleteTrainFromInActivePool(std::string lineID)
	{
		inActivePoolLock.lock();
		int trainId=-1;
		std::map<std::string, std::vector<int>>::iterator it;
		it = mapOfInActivePoolInLine.find(lineID);
		if(it != mapOfInActivePoolInLine.end())
		{
			std::vector<int>& trainIds = it->second;
			if(trainIds.size() >0)
			{
				//getting the train Id
				trainId = trainIds[0];
				trainIds.erase(trainIds.begin());
			}
			inActivePoolLock.unlock();
			return trainId;
		}
	}

	template<typename PERSON>
	int TrainController<PERSON>::deleteTrainFromActivePool(std::string lineID)
	{
		activePoolLock.lock();
		int trainId = -1;
		std::map<std::string, std::vector<int>>::iterator it;
		it = recycleTrainId.find(lineID);
		if(it != recycleTrainId.end())
		{
			std::vector<int>& trainIds = it->second;
			if(trainIds.size() >0)
			{
				if(mapOfNoAvailableTrains[lineID]>0)
				{
					trainId = trainIds[0];
					trainIds.erase(trainIds.begin());
					mapOfNoAvailableTrains[lineID] = mapOfNoAvailableTrains[lineID]-1;
					activePoolLock.unlock();
					sim_mob::BasicLogger& ptMRTMoveLogger  = sim_mob::Logger::log("pulledTrainId.csv");
					ptMRTMoveLogger<<trainId<<","<<lineID<<std::endl;
					return trainId;
				}
			}
		}
		activePoolLock.unlock();
		return trainId;
	}

	template<typename PERSON>
	void TrainController<PERSON>::addTrainToActivePool(std::string lineId,int trainId)
	{
		activePoolLock.lock();
		std::map<std::string, std::vector<int>>::iterator it;
		it = recycleTrainId.find(lineId);
		if(it == recycleTrainId.end())
		{
			recycleTrainId[lineId] = std::vector<int>();
		}
		recycleTrainId[lineId].push_back(trainId);
		mapOfNoAvailableTrains[lineId] = mapOfNoAvailableTrains[lineId]+1;
		activePoolLock.unlock();
		sim_mob::BasicLogger& ptMRTMoveLogger  = sim_mob::Logger::log("returnedTrainId.csv");
		ptMRTMoveLogger<<trainId<<","<<lineId<<std::endl;
	}

	template<typename PERSON>
	void TrainController<PERSON>::addTrainToInActivePool(std::string lineID,int trainId)
	{
		inActivePoolLock.lock();
		std::map<std::string, std::vector<int>>::iterator it;
		it = mapOfInActivePoolInLine.find(lineID);
		if(it == mapOfInActivePoolInLine.end())
		{
			mapOfInActivePoolInLine[lineID] = std::vector<int>();
		}
		mapOfInActivePoolInLine[lineID].push_back(trainId);
		inActivePoolLock.unlock();
	}

	template<typename PERSON>
	int TrainController<PERSON>::pullOutTrainFromInActivePool( std::string lineID)
	{
		int trainId = -1;
		trainId = deleteTrainFromInActivePool(lineID);
		if(trainId != -1)
		{
			addTrainToActivePool(lineID,trainId);
		}
        return trainId;
	}

	template<typename PERSON>
	void TrainController<PERSON>::pushToInactivePoolAfterTripCompletion(int trainId,std::string lineId)
	{
		trainsToBePushedToInactivePoolAfterTripCompletion[lineId].push_back(trainId);
	}

	template<typename PERSON>
	void TrainController<PERSON>::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
	{
		switch (type)
		{
			case MSG_TRAIN_BACK_DEPOT:
			{
				//sending the train back to depot
				const TrainMessage& msg = MSG_CAST(TrainMessage, message);
				PERSON* person = dynamic_cast<PERSON*>(msg.trainAgent);
				if(person)
				{
					const std::vector<TripChainItem *>& tripChain = person->getTripChain();
					TrainTrip* front = dynamic_cast<TrainTrip*>(tripChain.front());
					if(front)
					{
						int trainId = front->getTrainId();
						std::string lineId = front->getLineId();
						std::string oppLineId = getOppositeLineId(lineId);
						std::map<std::string,std::vector<int>>::iterator itr=trainsToBePushedToInactivePoolAfterTripCompletion.find(lineId);
						if(itr != trainsToBePushedToInactivePoolAfterTripCompletion.end())
						{
							std::vector<int> &trainIds = trainsToBePushedToInactivePoolAfterTripCompletion[lineId];
							std::vector<int>::iterator it=std::find(trainIds.begin(), trainIds.end(), trainId);
							if(it != trainIds.end())
							{
								trainIds.erase(it);
								addTrainToInActivePool(oppLineId, trainId);
							}
							else
							{
								addTrainToActivePool(oppLineId, trainId);
							}
						}
						else
						{
							addTrainToActivePool(oppLineId, trainId);
						}
					}
				}
				break;
			}
		}
	}

	template<typename PERSON>
	void TrainController<PERSON>::assignResetBlockSpeeds(ResetBlockSpeeds resetSpeedBlocksEntity)
	{
		resetSpeedBlocks.push_back(resetSpeedBlocksEntity);
	}

	template<typename PERSON>
	void TrainController<PERSON>::resetBlockSpeeds(DailyTime now)
	{
		std::vector<ResetBlockSpeeds>::iterator it;
		DailyTime currentTime = now;
		DailyTime nextFrameTickTime = now + DailyTime(5000);
		int count=-1;
		for(it = resetSpeedBlocks.begin() ; it < resetSpeedBlocks.end(); )
		{
			count++;
			if((*it).startTime>=currentTime.getStrRepr() && (*it).startTime < nextFrameTickTime.getStrRepr() && (*it).speedReset == false)
			{
				std::string startStation = (*it).startStation;
				std::string endStation = (*it).endStation;
				std::string lineId = (*it).line;
				double speedLimit = (*it).speedLimit;
				std::vector<Platform*> platforms;
				getTrainPlatforms(lineId,platforms);
				bool startSeq = false;
				std::vector<Platform *>::iterator itpl;
				for(itpl = platforms.begin() ; itpl < platforms.end(); itpl++)
				{
					if(boost::iequals((*itpl)->getStationNo(), startStation))
					{
						startSeq=true;
					}
					else if (boost::iequals((*itpl)->getStationNo(), endStation))
					{
						break;
					}
					if(startSeq == true)
					{
						int blockId = (*itpl)->getAttachedBlockId();
						std::map<unsigned int, Block*>::iterator iBlock = mapOfIdvsBlocks.find(blockId);
						Block *block = iBlock->second;
						blockIdSpeed[(*itpl)->getLineId()][blockId] = block->getSpeedLimit();
						block->setSpeedLimit(speedLimit);
						resetSpeedBlocks[count].speedReset = true;
					}
				}
			}

			else if((*it).endTime <= currentTime.getStrRepr())
			{
				std::vector<Platform *>::iterator itpl;
				std::string startStation = (*it).startStation;
				std::string endStation = (*it).endStation;
				std::string lineId = (*it).line;
				std::vector<Platform*> platforms;
				getTrainPlatforms(lineId,platforms);
				bool startSeq = false;
				for(itpl = platforms.begin() ; itpl < platforms.end(); itpl++)
				{
					if(boost::iequals((*itpl)->getStationNo(), startStation))
					{
						startSeq = true;
					}
					else if (boost::iequals((*itpl)->getStationNo(), endStation))
					{
						break;
					}
					if(startSeq == true)
					{
						int blockId = (*itpl)->getAttachedBlockId();
						std::map<unsigned int, Block*>::iterator iBlock = mapOfIdvsBlocks.find(blockId);
						Block *block = iBlock->second;
						double defaultSpeed = blockIdSpeed[(*itpl)->getLineId()][block->getBlockId()];
						block->setSpeedLimit(defaultSpeed);
					}
				}
				resetSpeedBlocks.erase(it);
				continue;
			}
			it++;
		}
	}

	template<typename PERSON>
	void TrainController<PERSON>::assignResetBlockAccelerations(ResetBlockAccelerations resetAccelerationBlocksEntity)
	{
		resetAccelerationBlocks.push_back(resetAccelerationBlocksEntity);
	}

	template<typename PERSON>
	void TrainController<PERSON>::loadTrainLineProperties()
	{
		const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
		const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;
		std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("get_train_line_properties");
		if(spIt == storedProcs.end())
		{
			Print() << "missing stored procedure for get_train_line_properties" << std::endl;
			return;
		}
		soci::session sql_(soci::postgresql, configParams.getDatabaseConnectionString(false));
		soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + spIt->second);
		for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
		{
			TrainProperties trainProperties;
			TrainDwellTimeInfo dwellTimeInfo;
			const soci::row& r = (*it);
			const std::string lineId = r.get<std::string>(0);
			dwellTimeInfo.dwellTimeAtNormalStation = r.get<double>(1);
			dwellTimeInfo.dwellTimeAtTerminalStaions = r.get<double>(2);
			dwellTimeInfo.dwellTimeAtInterchanges = r.get<double>(3);
			dwellTimeInfo.maxDwellTime = r.get<double>(4);
			dwellTimeInfo.firstCoeff = r.get<double>(5);
			dwellTimeInfo.secondCoeff = r.get<double>(6);
			dwellTimeInfo.thirdCoeff = r.get<double>(7);
			dwellTimeInfo.fourthCoeff = r.get<double>(8);
			trainProperties.safeDistance = r.get<double>(9);
			trainProperties.safeHeadway = r.get<double>(10);
			trainProperties.minDistanceTrainBehindForUnscheduledTrain = r.get<double>(11);
			trainProperties.trainLength = r.get<double>(12);
			trainProperties.maxCapacity = r.get<int>(13);
			trainProperties.dwellTimeInfo = dwellTimeInfo;
			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
			config.trainController.trainLinePropertiesMap[lineId] = trainProperties;
		}
	}

	template<typename PERSON>
	void TrainController<PERSON>::resetBlockAccelerations(DailyTime now)
	{
		std::vector<ResetBlockAccelerations>::iterator it;
		DailyTime currentTime=now;
		DailyTime nextFrameTickTime=now+DailyTime(5000);
		int count=-1;
		for(it=resetAccelerationBlocks.begin() ; it < resetAccelerationBlocks.end(); )
		{
			count++;
			if((*it).startTime>=currentTime.getStrRepr() && (*it).startTime<nextFrameTickTime.getStrRepr() && (*it).accelerationReset == false)
			{
				std::string startStation = (*it).startStation;
				std::string endStation = (*it).endStation;
				std::string lineId = (*it).line;
				double accLimit = (*it).accLimit;
				std::vector<Platform*> platforms;
				getTrainPlatforms(lineId,platforms);
				bool startSeq=false;
				std::vector<Platform *>::iterator itpl;
				for(itpl=platforms.begin() ; itpl < platforms.end(); itpl++)
				{
					if(boost::iequals((*itpl)->getStationNo(), startStation))
					{
						startSeq=true;
					}
					else if (boost::iequals((*itpl)->getStationNo(), endStation))
					{
					   break;
					}
					if(startSeq==true)
					{
						int blockId = (*itpl)->getAttachedBlockId();
						std::map<unsigned int, Block*>::iterator iBlock = mapOfIdvsBlocks.find(blockId);
						Block *block=iBlock->second;
						blockIdAcceleration[(*itpl)->getLineId()][blockId]=block->getAccelerateRate();
						block->setAccelerateRate(accLimit);
						block->setDecelerateRate(accLimit);
						resetAccelerationBlocks[count].accelerationReset = true;
					}
				}
			}

			else if((*it).endTime<=currentTime.getStrRepr())
			{
				std::vector<Platform *>::iterator itpl;
				std::string startStation = (*it).startStation;
				std::string endStation = (*it).endStation;
				std::string lineId = (*it).line;
				std::vector<Platform*> platforms;
				getTrainPlatforms(lineId,platforms);
				bool startSeq=false;
				for(itpl=platforms.begin() ; itpl < platforms.end(); itpl++)
				{
					if(boost::iequals((*itpl)->getStationNo(), startStation))
					{
						startSeq=true;
					}
					else if (boost::iequals((*itpl)->getStationNo(), endStation))
					{
						break;
					}
					if(startSeq==true)
					{
						int blockId=(*itpl)->getAttachedBlockId();
						std::map<unsigned int, Block*>::iterator iBlock = mapOfIdvsBlocks.find(blockId);
						Block *block=iBlock->second;
						double defaultAcceleration = blockIdAcceleration[(*itpl)->getLineId()][block->getBlockId()];
						block->setAccelerateRate(defaultAcceleration);
						block->setDecelerateRate(defaultAcceleration);
					}
				}
				resetAccelerationBlocks.erase(it);
				continue;
			}
			it++;
		}
	}

	template<typename PERSON>
	void TrainController<PERSON>::onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args)
	{
		switch(eventId)
		{
			case event::EVT_CORE_MRT_DISRUPTION:
			{
				const event::DisruptionEventArgs& exArgs = MSG_CAST(event::DisruptionEventArgs, args);
				const DisruptionParams& disruption = exArgs.getDisruption();
				disruptionParam.reset(new DisruptionParams(disruption));
				break;
			}
		}
	}

	template<typename PERSON>
	void TrainController<PERSON>::changeNumberOfPersonsCoefficients(std::string stationName,std::string platformName,double coefficientA,double coefficientB,double coefficientC)
	{
		std::map<std::string, Platform*>::const_iterator itr =  mapOfIdvsPlatforms.find(platformName);
		if(itr != mapOfIdvsPlatforms.end())
		{
			const Platform *platform = itr->second;
			const std::string stationNo = platform->getStationNo();
			std::map<std::string, Station*>::const_iterator it = mapOfIdvsStations.find(stationNo);
			if(it != mapOfIdvsStations.end())
			{
				mapOfCoefficientsOfNumberOfPersons[it->second][itr->second].push_back(coefficientA);
				mapOfCoefficientsOfNumberOfPersons[it->second][itr->second].push_back(coefficientB);
				mapOfCoefficientsOfNumberOfPersons[it->second][itr->second].push_back(coefficientC);
			}
		}
	}

	template<typename PERSON>
	const std::vector<double> TrainController<PERSON>::getNumberOfPersonsCoefficients(const Station *station,const Platform *platform) const
	{
		std::map<const Station*,std::map<const Platform*,std::vector<double>>>::const_iterator itr = mapOfCoefficientsOfNumberOfPersons.find(station);
		if(itr != mapOfCoefficientsOfNumberOfPersons.end())
		{
			const std::map<const Platform*,std::vector<double>> &mapOfPlatformVsCoefficients = itr->second;
			std::map<const Platform*,std::vector<double>>::const_iterator it = mapOfPlatformVsCoefficients.find(platform);
			if(it != mapOfPlatformVsCoefficients.end())
			{
				return it->second;
			}
		}
		return std::vector<double>();
	}

} /* namespace sim_mob */
#endif
