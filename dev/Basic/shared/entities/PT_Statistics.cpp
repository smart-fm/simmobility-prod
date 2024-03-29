/*
 * PT_Statistics.cpp
 *
 *  Created on: 13 Jun, 2014
 *      Author: zhang huai peng
 */

#include "entities/PT_Statistics.hpp"
#include <cstdio>
#include <fstream>
#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>
#include "boost/algorithm/string.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "util/DailyTime.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob
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

PT_Statistics::PT_Statistics() : MessageHandler(0)
{
    stopStatsMgr.loadHistoricalStopStats();
}

PT_Statistics::~PT_Statistics() {}

void PT_Statistics::HandleMessage(Message::MessageType type, const Message& message)
{
    switch (type)
    {
    case STORE_BUS_ARRIVAL:
    {
        const PT_ArrivalTimeMessage& msg = MSG_CAST(PT_ArrivalTimeMessage, message);
        journeyTimes.push_back(msg.arrivalInfo);
        stopStatsMgr.addStopStats(msg.arrivalInfo);
        break;
    }
    case STORE_PERSON_WAITING:
    {
        const PersonWaitingTimeMessage& msg = MSG_CAST(PersonWaitingTimeMessage, message);
        char key[50];
        sprintf(key, "%s,%s", msg.personWaitingTime.personIddb.c_str(), msg.personWaitingTime.busStopNo.c_str());
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
    case STORE_PERSON_REROUTE:
    {
        const PT_RerouteInfoMessage& msg = MSG_CAST(PT_RerouteInfoMessage, message);
        personsReroutes.push_back(msg.rerouteInfo);
        break;
    }
    case STORE_WAITING_PERSON_COUNT:
    {
        const WaitingCountMessage& msg = MSG_CAST(WaitingCountMessage, message);
        waitingCounts.push_back(msg.waitingCnt);
        break;
    }
    case STORE_PERSON_ALIGHTING:
    {
        const PT_PassengerAlightInfoMessage& msg = MSG_CAST(PT_PassengerAlightInfoMessage, message);
        stopStatsMgr.addStopStats(msg.personAlightTimeInfo);
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
    const sim_mob::ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();
    std::string journeyStatsFilename = cfg.getJourneyTimeStatsFilename();
    if (!journeyStatsFilename.empty())
    {
        std::ofstream outputFile(journeyStatsFilename.c_str());
        if (outputFile.is_open())
        {
            std::vector<PT_ArrivalTime>::const_iterator it = journeyTimes.begin();
            for (; it != journeyTimes.end(); it++)
            {
                outputFile << it->getCSV();
            }
            outputFile.close();
        }
    }
    journeyTimes.clear();

    std::string waitingTimeStatsFilename = cfg.getWaitingTimeStatsFilename();
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

    std::string waitingCountsFilename = cfg.getWaitingCountStatsFilename();
    if (!waitingCountsFilename.empty())
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

    std::string travelTimeFilename = cfg.getTravelTimeStatsFilename();
    if (!travelTimeFilename.empty())
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

    std::string personRerouteFilename = cfg.getPT_PersonRerouteFilename();
    if (!personRerouteFilename.empty())
    {
        std::ofstream outputFile(personRerouteFilename.c_str());
        if (outputFile.is_open())
        {
            std::vector<PT_RerouteInfo>::const_iterator itPerson = personsReroutes.begin();
            for (; itPerson != personsReroutes.end(); itPerson++)
            {
                outputFile << itPerson->getCSV();
            }
            outputFile.close();
        }
    }
    personsReroutes.clear();

    stopStatsMgr.exportStopStats();
}

double PT_Statistics::getDwellTime(unsigned int time, const std::string& stopCode, const std::string& serviceLine) const
{
    return stopStatsMgr.getDwellTime(time, stopCode, serviceLine);
}

double PT_Statistics::getWaitingTime(unsigned int time, const std::string& stopCode, const std::string& serviceLine) const
{
    return stopStatsMgr.getWaitingTime(time, stopCode, serviceLine);
}

std::string PersonWaitingTime::getCSV() const
{
    char csvArray[600];
    sprintf(csvArray, "%s,%u,%u,%s,%s,%s,%s,%s,%.2f,%u\n",
            personIddb.c_str(),
            originNode,
            destNode,
            busStopNo.c_str(),
            endstop.c_str(),
            busLines.c_str(),
            busLineBoarded.c_str(),
            currentTime.c_str(),
            waitingTime,
            deniedBoardingCount);
    return std::string(csvArray);
}

std::string PT_ArrivalTime::getCSV() const
{
    char csvArray[200];
    sprintf(csvArray, "%s,%s,%s,%3.2f,%u,%s,%s\n",
            stopNo.c_str(),
            serviceLine.c_str(),
            tripId.c_str(),
            pctOccupancy,
            sequenceNo,
            arrivalTime.c_str(),
            dwellTime.c_str());
    return std::string(csvArray);
}
std::string PT_RerouteInfo::getCSV() const
{
    char csvArray[200];
    sprintf(csvArray, "%s,%s,%u,%s,%u,%u,%u,%u,%s\n",
            personId.c_str(),
            stopNo.c_str(),
            lastRoleType,
            travelMode.c_str(),
            originNodeId,
            startNodeId,
            destNodeId,
            isPT_loaded,
            currentTime.c_str());
    return std::string(csvArray);
}
std::string PersonTravelTime::getCSV() const
{
    std::stringstream sst;
    sst << personId << ",";
    sst << tripStartPoint << ",";
    sst << tripEndPoint << ",";
    sst << subStartPoint << ",";
    sst << subEndPoint << ",";
    sst << subStartType << ",";
    sst << subEndType << ",";
    sst << mode << ",";
    sst << arrivalTime << ",";
    sst << travelTime << ",";
    sst << service << std::endl;
    return sst.str();
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
    sprintf(csvArray, "%u,%s,%s,%.2f,%.2f,%.2f,%.2f,%.2f\n",
            interval,
            stopCode.c_str(),
            serviceLine.c_str(),
            ((waitingCount<=0)? 0 : (waitingTime / waitingCount)),
            ((numArrivals<=0)? 0 : (dwellTime / numArrivals)),
            numArrivals,
            numBoarding,
            numAlighting);
    return std::string(csvArray);
}

StopStatsManager::StopStatsManager() : intervalWidth(sim_mob::ConfigManager::GetInstance().FullConfig().getPathSetConf().interval)
{
}

unsigned int StopStatsManager::getTimeInSecs(const std::string& time) const
{
    return (DailyTime(time).getValue() / 1000);
}

void StopStatsManager::addStopStats(const PT_ArrivalTime& arrivalInfo)
{
    unsigned int interval = 0;
    if(intervalWidth!=0)
    {
        interval = getTimeInSecs(arrivalInfo.arrivalTime) / intervalWidth;
    }
    StopStats& stats = stopStatsMap[interval][arrivalInfo.stopNo][arrivalInfo.serviceLine]; //an entry to be created if not in the map already
    if(stats.needsInitialization)
    {
        stats.interval = interval;
        stats.stopCode = arrivalInfo.stopNo;
        stats.serviceLine = arrivalInfo.serviceLine;
        stats.needsInitialization = false;
    }
    stats.numArrivals++;
    stats.dwellTime = stats.dwellTime + arrivalInfo.dwellTimeSecs;
}


void StopStatsManager::addStopStats(const PT_PassengerAlightInfo& personAlightTimeInfo)
{
    unsigned int interval = 0;
    if(intervalWidth!=0)
    {
        interval = getTimeInSecs(personAlightTimeInfo.alightTime) / intervalWidth;
    }
    StopStats& stats = stopStatsMap[interval][personAlightTimeInfo.stopNo][personAlightTimeInfo.serviceLine]; //an entry to be created if not in the map already
    if(stats.needsInitialization)
    {
        stats.interval = interval;
        stats.stopCode = personAlightTimeInfo.stopNo;
        stats.serviceLine = personAlightTimeInfo.serviceLine;
        stats.needsInitialization = false;
    }
    stats.numAlighting++;
}

void StopStatsManager::addStopStats(const PersonWaitingTime& personWaiting)
{
    unsigned int personBoardingTime = getTimeInSecs(personWaiting.currentTime);
    if(personBoardingTime > SECONDS_IN_DAY) // personWaiting.waitingTime > personWaiting.currentTime(from start of day)
    {
        throw std::runtime_error("invalid currentTime passed with person waiting message");
    }
    unsigned int boardingInterval = 0;
    if(intervalWidth!=0)
    {
        boardingInterval = personBoardingTime / intervalWidth;
    }
    StopStats& boardingStats = stopStatsMap[boardingInterval][personWaiting.busStopNo][personWaiting.busLineBoarded]; //an entry to be created if not in the map already
    if(boardingStats.needsInitialization)
    {
        boardingStats.interval = boardingInterval;
        boardingStats.stopCode = personWaiting.busStopNo;
        boardingStats.serviceLine = personWaiting.busLineBoarded;
        boardingStats.needsInitialization = false;
    }
    boardingStats.numBoarding++;

    std::vector<std::string> lines;
    boost::split(lines, personWaiting.busLines, boost::is_any_of("/"));
    unsigned int personArrivalTime;
    if (personWaiting.waitingTime >personBoardingTime) // This is the case when current tick crosses mid night time 00:00:00
    {
        personArrivalTime = (unsigned int )SECONDS_IN_DAY -(personWaiting.waitingTime - personBoardingTime);
    }
    else
    {
        personArrivalTime = personBoardingTime - personWaiting.waitingTime;
    }

    if(personArrivalTime > SECONDS_IN_DAY) // personWaiting.waitingTime > personWaiting.currentTime(from start of day)
    {
        std::stringstream msg;
        msg << "Invalid currentTime or waiting time passed with person waiting message! "
            << "\npersonBoardingTime: " << personBoardingTime << ", waitingTime: " << personWaiting.waitingTime
            << "\npersonArrivalTime: " << personArrivalTime;
        throw std::runtime_error(msg.str());
    }
    unsigned int interval = 0;
    if(intervalWidth!=0)
    {
        interval= personArrivalTime / intervalWidth;
    }
    for(const std::string& line : lines)
    {
        StopStats& stats = stopStatsMap[interval][personWaiting.busStopNo][line]; //an entry to be created if not in the map already
        if(stats.needsInitialization)
        {
            stats.interval = interval;
            stats.stopCode = personWaiting.busStopNo;
            stats.serviceLine = line;
            stats.needsInitialization = false;
        }
        stats.waitingCount++;
        stats.waitingTime = stats.waitingTime + personWaiting.waitingTime;
    }
}

void StopStatsManager::loadHistoricalStopStats()
{
    const sim_mob::ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();

    if(!cfg.isPublicTransitEnabled())
    {
        return;
    }

    std::string dbStr(cfg.getDatabaseConnectionString(false));
    soci::session dbSession(soci::postgresql, dbStr);

    historicalStopStatsMap.clear();
    std::string historicalStopStatsProc = ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().procedureMappings["pt_stop_stats"];
    if(historicalStopStatsProc.empty())
    {
        return;
    }

    std::string query = "select * from " + historicalStopStatsProc;
    soci::rowset<soci::row> rs = (dbSession.prepare << query);
    for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
    {
        const soci::row& r = (*it);
        StopStats stats;
        stats.interval = r.get<int>(0);
        stats.stopCode = r.get<std::string>(1);
        stats.serviceLine = r.get<std::string>(2);
        stats.waitingTime = r.get<double>(3);
        stats.dwellTime = r.get<double>(4);
        stats.numArrivals = r.get<double>(5);
        stats.needsInitialization = false;
        historicalStopStatsMap[stats.interval][stats.stopCode][stats.serviceLine] = stats;
    }
}

double StopStatsManager::getDwellTime(unsigned int time, const std::string& stopCode, const std::string& serviceLine) const
{
    unsigned int interval = 0;
    if(intervalWidth!=0)
    {
        interval = time / intervalWidth;
    }
    std::map<unsigned int, std::map<std::string, std::map<std::string, StopStats> > >::const_iterator histMapIt = historicalStopStatsMap.find(interval);
    if(histMapIt == historicalStopStatsMap.end())
    {
        return -1;
    }
    const std::map<std::string, std::map<std::string, StopStats> >& stopLineStatsMap = histMapIt->second;
    std::map<std::string, std::map<std::string, StopStats> >::const_iterator stopLineStatsMapIt = stopLineStatsMap.find(stopCode);
    if(stopLineStatsMapIt == stopLineStatsMap.end())
    {
        return -1;
    }
    const std::map<std::string, StopStats>& lineStatsMap = stopLineStatsMapIt->second;
    std::map<std::string, StopStats>::const_iterator lineStatsMapIt = lineStatsMap.find(serviceLine);
    if(lineStatsMapIt == lineStatsMap.end())
    {
        return -1;
    }
    return lineStatsMapIt->second.dwellTime;
}

double StopStatsManager::getWaitingTime(unsigned int time, const std::string& stopCode, const std::string& serviceLine) const
{
    unsigned int interval = 0;
    if(intervalWidth!=0)
    {
        interval = time / intervalWidth;
    }
    std::map<unsigned int, std::map<std::string, std::map<std::string, StopStats> > >::const_iterator histMapIt = historicalStopStatsMap.find(interval);
    if(histMapIt == historicalStopStatsMap.end())
    {
        return -1;
    }
    const std::map<std::string, std::map<std::string, StopStats> >& stopLineStatsMap = histMapIt->second;
    std::map<std::string, std::map<std::string, StopStats> >::const_iterator stopLineStatsMapIt = stopLineStatsMap.find(stopCode);
    if(stopLineStatsMapIt == stopLineStatsMap.end())
    {
        return -1;
    }
    const std::map<std::string, StopStats>& lineStatsMap = stopLineStatsMapIt->second;
    std::map<std::string, StopStats>::const_iterator lineStatsMapIt = lineStatsMap.find(serviceLine);
    if(lineStatsMapIt == lineStatsMap.end())
    {
        return -1;
    }
    return lineStatsMapIt->second.waitingTime;
}

void StopStatsManager::exportStopStats()
{
    const sim_mob::ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();
    std::string stopStatsFilename = cfg.getPT_StopStatsFilename();
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
} //end namespace medium
} //end namespace simmob
