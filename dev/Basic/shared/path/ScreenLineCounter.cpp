/*
 * ScreenLineCounter.cpp
 *
 *  Created on: 8 Jun, 2015
 *      Author: balakumar
 */

#include "path/ScreenLineCounter.hpp"
#include "geospatial/RoadSegment.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include <algorithm>

namespace sim_mob
{
	ScreenLineCounter* ScreenLineCounter::instance = nullptr;
	boost::mutex ScreenLineCounter::instanceMutex;

	ScreenLineCounter::ScreenLineCounter()
	{
		if(ConfigManager::GetInstance().FullConfig().screenLineParams.outputEnabled)
		{
			screenLines.clear();
			sim_mob::aimsun::Loader::getScreenLineSegments(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
				sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().procedureMappings,screenLines);
			std::sort(screenLines.begin(), screenLines.end());
			intervalMS = ConfigManager::GetInstance().FullConfig().screenLineParams.interval * 1000;
		}
	}

	ScreenLineCounter::~ScreenLineCounter()
	{
		screenLines.clear();
		ttMap.clear();
	}

	ScreenLineCounter* ScreenLineCounter::getInstance()
	{
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex);
			if(!instance)
			{
				instance = new ScreenLineCounter();
			}
		}
		return instance;
	}

	void ScreenLineCounter::updateScreenLineCount(const Agent::RdSegTravelStat& rdSegStat)
	{
		if(!std::binary_search(screenLines.begin(), screenLines.end(), rdSegStat.rs->getId()))
		{
			return;
		}

		TT::TI timeInterval = ScreenLineCounter::getTimeInterval(rdSegStat.entryTime * 1000);
		TT::TimeAndCount &tc = ttMap[timeInterval][rdSegStat.travelMode][rdSegStat.rs];
		tc.totalTravelTime += rdSegStat.travelTime; //add to total travel time
		tc.travelTimeCnt += 1; //increment the total contribution
	}

	double ScreenLineCounter::getTimeInterval(const double time)
	{
		return time / intervalMS;
	}

	void ScreenLineCounter::exportScreenLineCount()
	{
		std::string fileName = ConfigManager::GetInstance().FullConfig().screenLineParams.fileName;

		sim_mob::BasicLogger& screenLineLogger  = sim_mob::Logger::log(fileName);

		TravelTime::iterator travelTimeIter = ttMap.begin();
		for(;travelTimeIter != ttMap.end(); travelTimeIter++)
		{
			const TT::TI & timeInterval = travelTimeIter->first;

			const DailyTime &simStartTime = sim_mob::ConfigManager::GetInstance().FullConfig().simStartTime();
			DailyTime startTime(simStartTime.getValue() +  (timeInterval* intervalMS) );
			DailyTime endTime(simStartTime.getValue() + ((timeInterval + 1) * intervalMS - 1) );

			sim_mob::TT::MRTC::iterator modeTCIter = travelTimeIter->second.begin();
			for(;modeTCIter != travelTimeIter->second.end(); modeTCIter++)
			{
				sim_mob::TT::RSTC::iterator rstcIter = modeTCIter->second.begin();
				for(;rstcIter != modeTCIter->second.end(); rstcIter++)
				{
					screenLineLogger << rstcIter->first->getId() << "\t" <<
						startTime.getStrRepr() << "\t" << endTime.getStrRepr() <<
						"\t" << modeTCIter->first <<
						"\t" << rstcIter->second.travelTimeCnt << "\n";
				}
			}
		}

		sim_mob::Logger::log(fileName).flush();
	}

}
