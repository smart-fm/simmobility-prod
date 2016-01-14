/*
 * ScreenLineCounter.cpp
 *
 *  Created on: 8 Jun, 2015
 *      Author: balakumar
 */

#include <algorithm>
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

ScreenLineCounter::ScreenLineCounter()
{
    const MT_Config& mtCfg = MT_Config::getInstance();
    if(mtCfg.screenLineParams.outputEnabled)
    {
        loadScreenLines();
        INTERVAL_MS = mtCfg.screenLineParams.interval * 1000;
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
    TimeInterval timeInterval = ScreenLineCounter::getTimeInterval(entryTimeSec * 1000);
    {
		boost::unique_lock<boost::mutex> lock(instanceMutex);
		screenlineMap[timeInterval][segId][travelMode].count++; //increment count for the relevant time interval, segment and mode
    }
}

unsigned int ScreenLineCounter::getTimeInterval(double time) const
{
    return time / INTERVAL_MS;
}

void ScreenLineCounter::exportScreenLineCount() const
{
    const medium::MT_Config& mtCfg = medium::MT_Config::getInstance();
    const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
    const std::string& fileName = mtCfg.screenLineParams.fileName;

    sim_mob::BasicLogger& screenLineLogger  = sim_mob::Logger::log(fileName);

    for(ScreenLineCountCollector::const_iterator travelTimeIter = screenlineMap.begin(); travelTimeIter != screenlineMap.end(); travelTimeIter++)
    {
        const TimeInterval& timeInterval = travelTimeIter->first;
        const DailyTime& simStartTime = configParams.simStartTime();
        DailyTime startTime(simStartTime.getValue() +  (timeInterval * INTERVAL_MS) );
        DailyTime endTime(simStartTime.getValue() + ((timeInterval + 1) * INTERVAL_MS - 1) );

        for(RoadSegmentCountMap::const_iterator segCountIt = travelTimeIter->second.begin();
                segCountIt != travelTimeIter->second.end(); segCountIt++)
        {
            for(CountMap::const_iterator countIter = segCountIt->second.begin();
                    countIter != segCountIt->second.end(); countIter++)
            {
                screenLineLogger << segCountIt->first << "\t" <<
                    startTime.getStrRepr() << "\t" << endTime.getStrRepr() <<
                    "\t" << countIter->first <<
                    "\t" << countIter->second.count << "\n";
            }
        }
    }

    sim_mob::Logger::log(fileName).flush();
}

}
}
