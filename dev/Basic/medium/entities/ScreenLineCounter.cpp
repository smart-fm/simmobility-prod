/*
 * ScreenLineCounter.cpp
 *
 *  Created on: 8 Jun, 2015
 *      Author: balakumar
 */

#include <algorithm>
#include <cmath>
#include <stdlib.h>
#include "soci/soci.h"
#include "soci/postgresql/soci-postgresql.h"

#include "ScreenLineCounter.hpp"
#include "conf/ConfigManager.hpp"
#include "config/MT_Config.hpp"
#include "geospatial/aimsun/Loader.hpp"

namespace
{
	/**
	 * time interval value used for processing data.
	 * This value is based on the user configuration
	 */
	unsigned int INTERVAL_MS = 0;
}

namespace sim_mob
{
namespace medium
{

ScreenLineCounter* ScreenLineCounter::instance = nullptr;

ScreenLineCounter::ScreenLineCounter() : simStartTime( ConfigManager::GetInstance().FullConfig().simStartTime())
{
    const MT_Config& mtCfg = MT_Config::getInstance();
    if(mtCfg.screenLineParams.outputEnabled)
    {
        loadScreenLines();
        INTERVAL_MS = mtCfg.screenLineParams.interval * 1000;
    }

    int interval = 0;
    DailyTime minTime = DailyTime(0);
    const DailyTime intervalWidth = DailyTime(INTERVAL_MS);
    minTimes.push_back(minTime.getStrRepr());
	for(int i=0, j=0; i<86400; i++, j=((j+1)%mtCfg.screenLineParams.interval)) //86400 seconds in a day
	{
		if(j==0)
		{
			interval = interval+1;
			minTime+=intervalWidth;
			minTimes.push_back(minTime.getStrRepr());
		}
		timeIntervalMap[i] = interval;
	}
}

ScreenLineCounter::~ScreenLineCounter()
{
    screenLineSegments.clear();
    screenlineMap.clear();
}

void ScreenLineCounter::loadScreenLines()
{
    screenLineSegments.clear();

    const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
    const std::map<std::string, std::string>& storedProcMap = config.getDatabaseProcMappings().procedureMappings;
    std::map<std::string, std::string>::const_iterator storedProcIter = storedProcMap.find("screen_line");
    if(storedProcIter == storedProcMap.end())
    {
        throw std::runtime_error("ScreenLineCounter: Stored Procedure not specified");
    }

    soci::session sql_(soci::postgresql, config.getDatabaseConnectionString(false));
    soci::rowset<unsigned long> rs = (sql_.prepare << "select * from " + storedProcIter->second);

    soci::rowset<unsigned long>::const_iterator iter = rs.begin();
    for(; iter != rs.end(); iter++)
    {
        screenLineSegments.insert(*iter);
    }
}

ScreenLineCounter* ScreenLineCounter::getInstance()
{
	if (!instance)
	{
		instance = new ScreenLineCounter();
	}
	return instance;
}

void ScreenLineCounter::updateScreenLineCount(unsigned int segId, double entryTimeSec, const std::string& travelMode)
{
    if(screenLineSegments.find(segId) == screenLineSegments.end())
    {
        return;
    }
    TimeInterval timeInterval = ScreenLineCounter::getTimeInterval(entryTimeSec + (double)simStartTime.getValue()/1000);
    {
		boost::unique_lock<boost::mutex> lock(instanceMutex);
		screenlineMap[timeInterval][segId][travelMode].count++; //increment count for the relevant time interval, segment and mode
    }
}

unsigned int ScreenLineCounter::getTimeInterval(double time) const
{
	unsigned int timeSec = std::ceil(time);
	if(timeSec >= 86400)
	{
		throw std::runtime_error("invalid time passed to update screen line counts");
	}
    return timeIntervalMap[timeSec];
}

void ScreenLineCounter::exportScreenLineCount() const
{
    const medium::MT_Config& mtCfg = medium::MT_Config::getInstance();
    const std::string& fileName = mtCfg.screenLineParams.fileName;

    sim_mob::BasicLogger& screenLineLogger  = sim_mob::Logger::log(fileName);

    for(ScreenLineCountCollector::const_iterator travelTimeIter = screenlineMap.begin(); travelTimeIter != screenlineMap.end(); travelTimeIter++)
    {
    	div_t divresult;
        const TimeInterval& timeInterval = travelTimeIter->first;
        const std::string& startTime = minTimes[timeInterval-1];
        const std::string& endTime = minTimes[timeInterval];

        for(RoadSegmentCountMap::const_iterator segCountIt = travelTimeIter->second.begin();
                segCountIt != travelTimeIter->second.end(); segCountIt++)
        {
            for(CountMap::const_iterator countIter = segCountIt->second.begin();
                    countIter != segCountIt->second.end(); countIter++)
            {
                screenLineLogger << segCountIt->first << "\t" <<
                    startTime << "\t" << endTime <<
                    "\t" << countIter->first <<
                    "\t" << countIter->second.count << "\n";
            }
        }
    }

    sim_mob::Logger::log(fileName).flush();
}

}
}
