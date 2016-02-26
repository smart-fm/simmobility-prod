/*
 * PT_Statistics.cpp
 *
 *  Created on: 13 Jun, 2014
 *      Author: zhang huai peng
 */

#include "entities/PT_Statistics.hpp"
#include <cstdio>
#include <fstream>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "config/MT_Config.hpp"
#include "util/DailyTime.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob
{
namespace medium
{
namespace
{
unsigned int SECONDS_IN_DAY = 24 * 60 * 60;
}

PT_Statistics* PT_Statistics::instance(nullptr);

PT_Statistics* PT_Statistics::getInstance()
{
	if (!instance)
	{
		instance = new PT_Statistics();
	}
	return instance;
}

void PT_Statistics::resetInstance()
{
	safe_delete_item(instance);
	instance = nullptr;
}

PT_Statistics::PT_Statistics() : MessageHandler(0) {}

PT_Statistics::~PT_Statistics() {}

void PT_Statistics::HandleMessage(Message::MessageType type, const Message& message)
{
	switch (type)
	{
	case STORE_BUS_ARRIVAL:
	{
		const BusArrivalTimeMessage& msg = MSG_CAST(BusArrivalTimeMessage, message);
		busJourneyTimes.push_back(msg.busArrivalInfo);
		stopStatsMgr.addStopStats(msg.busArrivalInfo);
		break;
	}
	case STORE_PERSON_WAITING:
	{
		const PersonWaitingTimeMessage& msg = MSG_CAST(PersonWaitingTimeMessage, message);
		char key[50];
		sprintf(key, "%u,%s", msg.personWaitingTime.personId, msg.personWaitingTime.busStopNo.c_str());
		personWaitingTimes[std::string(key)] = msg.personWaitingTime;
		stopStatsMgr.addStopStats(msg.personWaitingTime);
		break;
	}
	case STORE_PERSON_TRAVEL_TIME:
	{
		const PersonTravelTimeMessage& msg = MSG_CAST(PersonTravelTimeMessage, message);
		personTravelTimes.push_back(msg.personTravelTime);
		break;
	}
	case STORE_WAITING_PERSON_COUNT:
	{
		const WaitingCountMessage& msg = MSG_CAST(WaitingCountMessage, message);
		waitingCounts.push_back(msg.waitingCnt);
		break;
	}
	default:
	{
		break;
	}
	}
}

void PT_Statistics::storeStatistics()
{
	const MT_Config& mtcfg = MT_Config::getInstance();
	std::string journeyStatsFilename = mtcfg.getJourneyTimeStatsFilename();
	if (!journeyStatsFilename.empty())
	{
		std::ofstream outputFile(journeyStatsFilename.c_str());
		if (outputFile.is_open())
		{
			std::vector<BusArrivalTime>::const_iterator it = busJourneyTimes.begin();
			for (; it != busJourneyTimes.end(); it++)
			{
				outputFile << it->getCSV();
			}
			outputFile.close();
		}
	}
	busJourneyTimes.clear();

	std::string waitingTimeStatsFilename = mtcfg.getWaitingTimeStatsFilename();
	if (!waitingTimeStatsFilename.empty())
	{
		std::ofstream outputFile(waitingTimeStatsFilename.c_str());
		if (outputFile.is_open())
		{
			std::map<std::string, PersonWaitingTime>::const_iterator itWaitingTime = personWaitingTimes.begin();
			for (; itWaitingTime != personWaitingTimes.end(); itWaitingTime++)
			{
				outputFile << itWaitingTime->second.getCSV();
			}
			outputFile.close();
		}
	}
	personWaitingTimes.clear();

	std::string waitingCountsFilename = mtcfg.getWaitingCountStatsFilename();
	if (waitingCountsFilename.size() > 0)
	{
		std::ofstream outputFile(waitingCountsFilename.c_str());
		if (outputFile.is_open())
		{
			std::vector<WaitingCount>::const_iterator itWaitingCnt = waitingCounts.begin();
			for (; itWaitingCnt!=waitingCounts.end(); itWaitingCnt++)
			{
				outputFile << itWaitingCnt->getCSV();
			}
			outputFile.close();
		}
	}
	waitingCounts.clear();

	std::string travelTimeFilename = mtcfg.getTravelTimeStatsFilename();
	if (travelTimeFilename.size() > 0)
	{
		std::ofstream outputFile(travelTimeFilename.c_str());
		if (outputFile.is_open())
		{
			std::vector<PersonTravelTime>::const_iterator itPersonTravel = personTravelTimes.begin();
			for (; itPersonTravel != personTravelTimes.end(); itPersonTravel++)
			{
				outputFile << itPersonTravel->getCSV();
			}
			outputFile.close();
		}
	}
	personTravelTimes.clear();

	stopStatsMgr.exportStopStats();
}

std::string PersonWaitingTime::getCSV() const
{
	char csvArray[500];
	sprintf(csvArray, "%u,%s,%s,%s,%.2f,%u\n",
			personId,
			busStopNo.c_str(),
			busLine.c_str(),
			currentTime.c_str(),
			waitingTime,
			deniedBoardingCount);
	return std::string(csvArray);
}

std::string BusArrivalTime::getCSV() const
{
	char csvArray[200];
	sprintf(csvArray, "%s,%s,%s,%3.2f,%u,%s,%s\n",
			busStopNo.c_str(),
			busLine.c_str(),
			tripId.c_str(),
			pctOccupancy,
			sequenceNo,
			arrivalTime.c_str(),
			dwellTime.c_str());
	return std::string(csvArray);
}

std::string PersonTravelTime::getCSV() const
{
	char csvArray[200];
	sprintf(csvArray, "%u,%s,%s,%s,%s,%s,%s,%s,%s,%.2f\n",
			personId,
			tripStartPoint.c_str(),
			tripEndPoint.c_str(),
			subStartPoint.c_str(),
			subEndPoint.c_str(),
			subStartType.c_str(),
			subEndType.c_str(),
			mode.c_str(),
			arrivalTime.c_str(),
			travelTime);
	return std::string(csvArray);
}

std::string WaitingCount::getCSV() const
{
	char csvArray[200];
	sprintf(csvArray, "%s,%s,%u\n",
			busStopNo.c_str(),
			currTime.c_str(),
			count);
	return std::string(csvArray);
}

std::string StopStats::getCSV() const
{
	char csvArray[100];
	sprintf(csvArray, "%u,%s,%s,%.2f,%.2f,%.2f\n",
			interval,
			stopCode.c_str(),
			serviceLine.c_str(),
			((waitingCount==0)? 0 : (waitingTime/waitingCount)),
			((numArrivals==0)? 0 : (dwellTime/numArrivals)),
			numArrivals);
	return std::string(csvArray);
}

StopStatsManager::StopStatsManager() : intervalWidth(sim_mob::ConfigManager::GetInstance().FullConfig().getPathSetConf().interval)
{
}

unsigned int StopStatsManager::getTimeInSecs(const std::string& time) const
{
	return (DailyTime(time).getValue() / 1000);
}

void StopStatsManager::addStopStats(const BusArrivalTime& busArrival)
{
	unsigned int interval = getTimeInSecs(busArrival.arrivalTime) / intervalWidth;
	StopStats& stats = stopStatsMap[interval][busArrival.busStopNo][busArrival.busLine]; //an entry to be created if not in the map already
	if(stats.needsInitialization)
	{
		stats.interval = interval;
		stats.stopCode = busArrival.busStopNo;
		stats.serviceLine = busArrival.busLine;
		stats.needsInitialization = false;
	}
	stats.numArrivals++;
	stats.dwellTime = stats.dwellTime + busArrival.dwellTimeSecs;
}

void StopStatsManager::addStopStats(const PersonWaitingTime& personWaiting)
{
	unsigned int personArrivalTime = getTimeInSecs(personWaiting.currentTime) - personWaiting.waitingTime;
	if(personArrivalTime > SECONDS_IN_DAY) // personWaiting.waitingTime > personWaiting.currentTime(from start of day)
	{
		throw std::runtime_error("invalid currentTime or waiting time passed with person waiting message");
	}
	unsigned int interval = personArrivalTime / intervalWidth;
	StopStats& stats = stopStatsMap[interval][personWaiting.busStopNo][personWaiting.busLine]; //an entry to be created if not in the map already
	if(stats.needsInitialization)
	{
		stats.interval = interval;
		stats.stopCode = personWaiting.busStopNo;
		stats.serviceLine = personWaiting.busLine;
		stats.needsInitialization = false;
	}
	stats.waitingCount++;
	stats.waitingTime = stats.waitingTime + personWaiting.waitingTime;
}

void StopStatsManager::exportStopStats()
{
	const MT_Config& mtcfg = MT_Config::getInstance();
	std::string stopStatsFilename = mtcfg.getPT_StopStatsFilename();
	if (!stopStatsFilename.empty())
	{
		std::ofstream outputFile(stopStatsFilename.c_str());
		if (outputFile.is_open())
		{
			std::map<unsigned int, std::map<std::string, std::map<std::string, StopStats> > >::const_iterator stopStatsMapIt = stopStatsMap.begin();
			for (; stopStatsMapIt!=stopStatsMap.end(); stopStatsMapIt++)
			{
				const std::map<std::string, std::map<std::string, StopStats> >& stopLineStatsMap = stopStatsMapIt->second;
				std::map<std::string, std::map<std::string, StopStats> >::const_iterator stopLineStatsMapIt = stopLineStatsMap.begin();
				for(; stopLineStatsMapIt!=stopLineStatsMap.end(); stopLineStatsMapIt++)
				{
					const std::map<std::string, StopStats>& lineStatsMap = stopLineStatsMapIt->second;
					std::map<std::string, StopStats>::const_iterator lineStatsMapIt = lineStatsMap.begin();
					for(; lineStatsMapIt!=lineStatsMap.end(); lineStatsMapIt++)
					{
						outputFile << lineStatsMapIt->second.getCSV();
					}
				}
			}
			outputFile.close();
		}
	}
	stopStatsMap.clear();
}
} //end namespace medium
} //end namespace simmob
