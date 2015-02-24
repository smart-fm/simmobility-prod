#include "TravelTimeManager.hpp"
#include "PathSetManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "boost/filesystem.hpp"

sim_mob::TravelTimeManager::TravelTimeManager(unsigned int &intervalMS, unsigned int &curIntervalMS):intervalMS(intervalMS), curIntervalMS(curIntervalMS)
{
	enRouteTT.reset(new sim_mob::LastTT(*this));
}

void sim_mob::TravelTimeManager::addTravelTime(const Agent::RdSegTravelStat & stats) {
	TT::TI timeInterval = TravelTimeManager::getTimeInterval(stats.entryTime * 1000, intervalMS);//milliseconds
	{
		boost::unique_lock<boost::mutex> lock(ttMapMutex);
		TT::TimeAndCount &tc = ttMap[timeInterval][stats.travelMode][stats.rs];
		tc.totalTravelTime += stats.travelTime; //add to total travel time
		tc.travelTimeCnt += 1; //increment the total contribution
	}
}

sim_mob::TT::TI sim_mob::TravelTimeManager::getTimeInterval(const unsigned long time, const unsigned int interval)
{
	return time / interval ;/*milliseconds*/
}

double sim_mob::TravelTimeManager::getInSimulationSegTT(const std::string mode, const sim_mob::RoadSegment *rs) const
{
	return enRouteTT->getInSimulationSegTT(mode,rs);
}

double sim_mob::LastTT::getInSimulationSegTT(const std::string mode, const sim_mob::RoadSegment *rs) const
{
	boost::unique_lock<boost::mutex> lock(parent.ttMapMutex);
	//[time interval][travel mode][road segment][average travel time]
	 //<-----TI-----><-------------------MRTC----------------------->
		//start from the last recorded time interval (before the current time interval) and proceed to find a travel time for the given section.
		//if no records found, check the previous time interval and so on.
		double res = 0.0;
		sim_mob::TravelTime::reverse_iterator itTI = parent.ttMap.rbegin();
		//like I said, not the current interval
		if(itTI != parent.ttMap.rend() && itTI->first == sim_mob::PathSetManager::curIntervalMS)
		{
			itTI++;
		}
		sim_mob::TT::MRTC::iterator itMode;
		sim_mob::TT::RSTC::iterator itSeg;
		//search backwards. try to find a matching road segment in any of the previous time intervals
		while(itTI != parent.ttMap.rend())
		{
			itMode = itTI->second.find(mode);
			if(itMode != itTI->second.end())
			{
				itSeg = itMode->second.find(rs);
				if(itSeg != itMode->second.end())
				{
					return itSeg->second.totalTravelTime / itSeg->second.travelTimeCnt;
				}
			}
			++itTI;
		}
		return 0.0;


// //[road segment][travel mode][time interval][average travel time]
// //<-----RS-----><-------------------MTITC----------------------->
//	//proceed towards the time interval sub-section and at the same time perform necessary sanity checks
//	double res = 0.0;
//	sim_mob::TravelTime::iterator itSeg = ttMap.find(rs);
//	if(itSeg == ttMap.end())
//	{
//		return 0.0;
//	}
//
//	TT::MTITC::iterator itMode = itSeg->second.find(mode);
//	if(itMode == itSeg->second.end())
//	{
//		return 0.0;
//	}
//
//
//	if(itMode->second.empty())
//	{
//		return 0.0;
//	}
//
//	// the reverse iterator on a std::map, guarantees the highest time interval value
//	TT::TITC::const_reverse_iterator itTI = itMode->second.rbegin();
//	//bypass the current interval
//	if(itTI->first == TravelTimeManager::curIntervalMS)
//	{
//		itTI++;
//	}
//	if(itTI == itMode->second.rend())
//	{
//		return 0.0;
//	}
//	//the clean answer
//	const TT::TimeAndCount &tc = itTI->second;
//	return tc.totalTravelTime / tc.travelTimeCnt;
}

void sim_mob::TravelTimeManager::insertTravelTime2TmpTable(const std::string fileName)
{
//	//whateever declaration needed for the following 3-tire nested loop:
//	typedef sim_mob::TravelTime::value_type SegPairs;//time range pairs
//	typedef TT::MTITC::value_type TravelModes;//travel mode collections
//	typedef TT::TITC::value_type TiPairs;
//	sim_mob::BasicLogger & TTLogger  = sim_mob::Logger::log(fileName);
//	const int &intervalSec = sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().interval;//config interval(in seconds)
//	//Now the loop
//	BOOST_FOREACH(SegPairs &segPair, ttMap)
//	{
//		const unsigned long &segmentId = segPair.first->getId();
//		TT::MTITC & travelModes = segPair.second;
//		BOOST_FOREACH(TravelModes &travelMode, travelModes)
//		{
//			const std::string & travelModeStr = travelMode.first;
//			const TT::TITC & timeIntervals = travelMode.second;
//			BOOST_FOREACH(const TiPairs &tiPair, timeIntervals)
//			{
//				const TT::TI & timeInterval = tiPair.first;
//				const TT::TimeAndCount & tc = tiPair.second;
//				const double &totalTT_ForThisSeg = tc.totalTravelTime;
//				const int &totalTT_Submissions = tc.travelTimeCnt;
//				double travelTime = totalTT_ForThisSeg / totalTT_Submissions;
//				dbg_ProcessTT_cnt++;
//				TTLogger << segmentId << ";" << DailyTime(timeInterval* intervalMS).getRepr_() << ";" << DailyTime((timeInterval + 1) * intervalMS - 1).getRepr_() << ";" << travelTime << ";"  << intervalSec << ";"  << travelModeStr <<  "\n";
//			}
//		}
//	}


	//	easy reading down the line

	typedef std::map<const sim_mob::RoadSegment*,sim_mob::TT::TimeAndCount >::value_type STC;//SegmentTimeCount
	typedef TT::MRTC::value_type TravelModes;
	typedef sim_mob::TravelTime::value_type TRPs;
	//	destination file
	sim_mob::BasicLogger & TTLogger  = sim_mob::Logger::log(fileName);
	// config interval(in seconds)
	int intervalSec = sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().interval;
	//time range
	BOOST_FOREACH(TRPs &TT_Pair, ttMap)
	{
		const TT::TI & timeInterval = TT_Pair.first;
		TT::MRTC & travelModes = TT_Pair.second;
		//travel mode
		BOOST_FOREACH(TravelModes &travelMode, travelModes)
		{
			const std::string & travelModeStr = travelMode.first;
			std::map<const sim_mob::RoadSegment*,TT::TimeAndCount > & travelTimes = travelMode.second;
			//road segment
			BOOST_FOREACH(STC &RS_Pair, travelTimes)
			{

				//easy reading
				const unsigned long &segmentId = RS_Pair.first->getId();
				TT::TimeAndCount & timeAndCount = RS_Pair.second;
				double &totalTT_ForThisSeg = timeAndCount.totalTravelTime;
				int &totalTT_Submissions = timeAndCount.travelTimeCnt;
				// calculate the average travel time
				double travelTime = totalTT_ForThisSeg / totalTT_Submissions;
				//start and end time:
				const DailyTime &simStartTime = sim_mob::ConfigManager::GetInstance().FullConfig().simStartTime();
				DailyTime startTime(simStartTime.getValue() +  (timeInterval* intervalMS) );
				DailyTime endTime(simStartTime.getValue() + ((timeInterval + 1) * intervalMS - 1) );
				//now simply write it to the file
				TTLogger << segmentId << ";" << startTime.getRepr_() << ";" << endTime.getRepr_() << ";" << travelTime << ";"  << intervalSec << ";"  << travelModeStr <<  "\n";
			}
		}
	}
}

bool sim_mob::TravelTimeManager::storeRTT2DB()
{
	typedef std::map<boost::thread::id, boost::shared_ptr<TravelTimeManager> >::value_type TTs;
	std::string tempFileName = sim_mob::PathSetParam::getInstance()->RTTT;
	insertTravelTime2TmpTable(tempFileName);
	sim_mob::Logger::log(tempFileName).flush();
	tempFileName += ".txt";
	return sim_mob::aimsun::Loader::upsertTravelTime(*sim_mob::PathSetManager::getInstance()->getSession(), boost::filesystem::canonical(tempFileName).string(), sim_mob::PathSetParam::getInstance()->RTTT, sim_mob::ConfigManager::GetInstance().PathSetConfig().alpha);
}

sim_mob::TravelTimeManager::~TravelTimeManager()
{
}
