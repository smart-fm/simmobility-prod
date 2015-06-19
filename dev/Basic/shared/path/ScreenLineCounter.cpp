/*
 * ScreenLineCounter.cpp
 *
 *  Created on: 8 Jun, 2015
 *      Author: balakumar
 */

#include <algorithm>
#include "path/ScreenLineCounter.hpp"
#include "geospatial/RoadSegment.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/aimsun/Loader.hpp"

namespace
{
	/*
	 * Reference of user configuration
	 */
	const sim_mob::ConfigParams& configParams = sim_mob::ConfigManager::GetInstance().FullConfig();

	/**
	 * time interval value used for processing data.
	 * This value is based on its counterpart in pathset manager.
	 */
	const unsigned int INTERVAL_MS = configParams.screenLineParams.interval;
}

namespace sim_mob
{
	ScreenLineCounter* ScreenLineCounter::instance = nullptr;
	boost::mutex ScreenLineCounter::instanceMutex;

	ScreenLineCounter::ScreenLineCounter()
	{
		if(configParams.screenLineParams.outputEnabled)
		{
			screenLines.clear();
			sim_mob::aimsun::Loader::getScreenLineSegments(configParams.getDatabaseConnectionString(false),
					configParams.getDatabaseProcMappings().procedureMappings,screenLines);
			std::sort(screenLines.begin(), screenLines.end());
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
		return time / INTERVAL_MS;
	}

	void ScreenLineCounter::exportScreenLineCount()
	{
		const std::string& fileName = configParams.screenLineParams.fileName;

		sim_mob::BasicLogger& screenLineLogger  = sim_mob::Logger::log(fileName);

		for(TravelTime::const_iterator travelTimeIter = ttMap.begin(); travelTimeIter != ttMap.end(); travelTimeIter++)
		{
			const TT::TI & timeInterval = travelTimeIter->first;

			const DailyTime &simStartTime = configParams.simStartTime();
			DailyTime startTime(simStartTime.getValue() +  (timeInterval* INTERVAL_MS) );
			DailyTime endTime(simStartTime.getValue() + ((timeInterval + 1) * INTERVAL_MS - 1) );

			for(sim_mob::TT::MRTC::const_iterator modeTCIter = travelTimeIter->second.begin();
					modeTCIter != travelTimeIter->second.end(); modeTCIter++)
			{
				for(sim_mob::TT::RSTC::const_iterator rstcIter = modeTCIter->second.begin();
						rstcIter != modeTCIter->second.end(); rstcIter++)
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
