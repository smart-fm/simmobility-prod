#include "TravelTimeManager.hpp"
#include "path/PathSetManager.hpp"
#include "path/SOCI_Converters.hpp"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <map>
#include <sstream>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "path/PathSetManager.hpp"
#include "util/LangHelpers.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/TurningGroup.hpp"

using namespace sim_mob;

namespace
{
unsigned int TT_STORAGE_TIME_INTERVAL_WIDTH = 0;

/**
 * given a time of the day, fetches the time interval which must be used as index for storing travel times
 * @param dt time for which index is to be fetched
 * @return index of traveltime store for time
 */
unsigned int getTimeInterval(const DailyTime& dt)
{
	if(TT_STORAGE_TIME_INTERVAL_WIDTH == 0)
	{
		throw std::runtime_error("width of time interval for travel time storage is 0");
	}
	return (dt.getValue() / TT_STORAGE_TIME_INTERVAL_WIDTH);
}

/**
 * given a time of the day, fetches the time interval which must be used as index for storing travel times
 * @param time time in milliseconds for which index is to be fetched
 * @return index of traveltime store for time
 */
unsigned int getTimeInterval(unsigned long time)
{
	if(TT_STORAGE_TIME_INTERVAL_WIDTH == 0)
	{
		throw std::runtime_error("width of time interval for travel time storage is 0");
	}
	return time / TT_STORAGE_TIME_INTERVAL_WIDTH ;/*milliseconds*/
}

} //anonymous namespace

sim_mob::TravelTimeManager* sim_mob::TravelTimeManager::instance = nullptr;

sim_mob::LinkTravelTime::LinkTravelTime() : linkId(0), defaultTravelTime(0.0)
{
}

sim_mob::LinkTravelTime::~LinkTravelTime()
{
}

sim_mob::LinkTravelTime& sim_mob::LinkTravelTime::operator=(const sim_mob::LinkTravelTime& rhs)
{
	linkId = rhs.linkId;
	defaultTravelTime = rhs.defaultTravelTime;
	return *this;
}

void sim_mob::LinkTravelTime::addHistoricalTravelTime(const DailyTime& dt, unsigned int downstreamLinkId, double travelTime)
{
	TimeInterval timeInterval = getTimeInterval(dt);
	historicalTT_Map[timeInterval][downstreamLinkId] = travelTime;
}

void sim_mob::LinkTravelTime::addInSimulationTravelTime(const LinkTravelStats& stats)
{
	TimeInterval timeInterval = getTimeInterval(stats.entryTime * 1000); //milliseconds
	if(stats.downstreamLink)
	{
		boost::upgrade_lock<boost::shared_mutex> lock(ttMapMutex);
		boost::upgrade_to_unique_lock<boost::shared_mutex> uniquelock(lock);
		TimeAndCount& tc = currentSimulationTT_Map[timeInterval][stats.downstreamLink->getLinkId()];
		tc.totalTravelTime += stats.travelTime; //add to total travel time
		tc.travelTimeCnt += 1; //increment the total contribution
	}
	else
	{
		//since the downstream link is not specified, the travel time contribution will have to go to all downstream links of this link
		const Node* toNode = stats.link->getToNode();
		const std::map<unsigned int, TurningGroup *>& turnGroupsFromLnk = toNode->getTurningGroups(stats.link->getLinkId());

		{   //new scope for lock
			boost::upgrade_lock<boost::shared_mutex> lock(ttMapMutex);
			boost::upgrade_to_unique_lock<boost::shared_mutex> uniquelock(lock);
			DownStreamLinkSpecificTimeAndCount_Map& tcMap = currentSimulationTT_Map[timeInterval];
			for(std::map<unsigned int, TurningGroup *>::const_iterator downStrmLnkIt=turnGroupsFromLnk.begin(); downStrmLnkIt!=turnGroupsFromLnk.end(); downStrmLnkIt++)
			{
				TimeAndCount& tc = tcMap[downStrmLnkIt->first];
				tc.totalTravelTime += stats.travelTime;
				tc.travelTimeCnt += 1; //increment the total contribution
			}
		}
	}
}

double sim_mob::LinkTravelTime::getHistoricalLinkTT(unsigned int downstreamLinkId, const DailyTime& dt) const
{
	TimeInterval timeInterval = getTimeInterval(dt.getValue());
	TravelTimeStore::const_iterator ttMapIt = historicalTT_Map.find(timeInterval);
	if(ttMapIt == historicalTT_Map.end())
	{
		return -1;
	}
	const DownStreamLinkSpecificTT_Map& ttInnerMap = ttMapIt->second;
	DownStreamLinkSpecificTT_Map::const_iterator ttInnerMapIt = ttInnerMap.find(downstreamLinkId);
	if(ttInnerMapIt == ttInnerMap.end())
	{
		return -1;
	}
	return ttInnerMapIt->second;
}

double sim_mob::LinkTravelTime::getHistoricalLinkTT(const DailyTime& dt) const
{
	TimeInterval timeInterval = getTimeInterval(dt);
	TravelTimeStore::const_iterator ttMapIt = historicalTT_Map.find(timeInterval);
	if(ttMapIt == historicalTT_Map.end())
	{
		return -1;
	}
	const DownStreamLinkSpecificTT_Map& ttInnerMap = ttMapIt->second;
	if(ttInnerMap.empty())
	{
		return -1;
	}
	double totalTT = 0.0;
	for(DownStreamLinkSpecificTT_Map::const_iterator ttInnerMapIt=ttInnerMap.begin(); ttInnerMapIt!=ttInnerMap.end(); ttInnerMapIt++)
	{
		totalTT = totalTT + ttInnerMapIt->second;
	}
	return (totalTT/ttInnerMap.size());
}

void sim_mob::LinkTravelTime::dumpTravelTimesToFile(const std::string fileName) const
{
	//	destination file
	sim_mob::BasicLogger& ttLogger  = sim_mob::Logger::log(fileName);
	// config interval(in seconds)
	int intervalSec = sim_mob::ConfigManager::GetInstance().FullConfig().getPathSetConf().interval;
	const DailyTime& simStartTime = sim_mob::ConfigManager::GetInstance().FullConfig().simStartTime();
	for(TimeAndCountStore::const_iterator tcIt=currentSimulationTT_Map.begin(); tcIt!=currentSimulationTT_Map.end(); tcIt++)
	{
		const TimeInterval timeInterval = tcIt->first;
		const DownStreamLinkSpecificTimeAndCount_Map& tcMap = tcIt->second;
		for(DownStreamLinkSpecificTimeAndCount_Map::const_iterator tcMapIt=tcMap.begin(); tcMapIt!=tcMap.end(); tcMapIt++)
		{
			unsigned int downstreamLinkId = tcMapIt->first;
			const TimeAndCount& tc = tcMapIt->second;
			DailyTime startTime(simStartTime.getValue() +  (timeInterval* TT_STORAGE_TIME_INTERVAL_WIDTH) );
			DailyTime endTime(simStartTime.getValue() + ((timeInterval + 1) * TT_STORAGE_TIME_INTERVAL_WIDTH - 1000) );
			ttLogger << linkId << ";" << downstreamLinkId << ";" << startTime.getStrRepr() << ";" << endTime.getStrRepr() << ";" << tc.getTravelTime() <<  "\n";
		}
	}
}

sim_mob::TravelTimeManager::TravelTimeManager()
	: intervalMS(sim_mob::ConfigManager::GetInstance().FullConfig().getPathSetConf().interval * 1000), //conversion from seconds to milliseconds
	  enRouteTT(new sim_mob::TravelTimeManager::EnRouteTT(*this))
{}

sim_mob::TravelTimeManager::~TravelTimeManager()
{
	safe_delete_item(enRouteTT);
}

void sim_mob::TravelTimeManager::loadTravelTimes()
{
	const sim_mob::ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();
	std::string dbStr(cfg.getDatabaseConnectionString(false));
	soci::session dbSession(soci::postgresql, dbStr);
	loadLinkDefaultTravelTime(dbSession);
	loadLinkHistoricalTravelTime(dbSession);
}

void sim_mob::TravelTimeManager::loadLinkDefaultTravelTime(soci::session& sql)
{
	defaultTT_TableName = sim_mob::ConfigManager::GetInstance().PathSetConfig().DTT_Conf;
	std::string query = "select link_id, to_char(start_time,'HH24:MI:SS') AS start_time, to_char(end_time,'HH24:MI:SS') AS end_time, travel_time from " + defaultTT_TableName;
	soci::rowset<sim_mob::LinkTravelTime> rs = sql.prepare << query;

	for (soci::rowset<sim_mob::LinkTravelTime>::iterator lttIt = rs.begin(); lttIt != rs.end(); ++lttIt)
	{
		lnkTravelTimeMap[lttIt->getLinkId()] = *lttIt;
	}
}

void sim_mob::TravelTimeManager::loadLinkHistoricalTravelTime(soci::session& sql)
{
	const sim_mob::ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();
	TT_STORAGE_TIME_INTERVAL_WIDTH = cfg.getPathSetConf().interval * 1000; // initialize to proper interval from config
	historicalTT_TableName = sim_mob::ConfigManager::GetInstance().PathSetConfig().RTTT_Conf;
	std::string query = "select link_id, downstream_link_id, to_char(start_time,'HH24:MI:SS') AS start_time, to_char(end_time,'HH24:MI:SS') AS end_time,"
			"travel_time from " + historicalTT_TableName + " order by link_id, downstream_link_id";

	//main loop
	soci::rowset<soci::row> rs = (sql.prepare << query);
	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
	{
		const soci::row& rowData = *it;
		unsigned int linkId = rowData.get<unsigned int>(0);
		unsigned int downstreamLinkId = rowData.get<unsigned int>(1);
		DailyTime startTime(rowData.get<std::string>(2));
		DailyTime endTime(rowData.get<std::string>(3));
		double travelTime = rowData.get<double>(4);

		//time interval validation
		DailyTime interval = endTime - startTime;
		if (interval.getValue() != TT_STORAGE_TIME_INTERVAL_WIDTH)
		{
			throw std::runtime_error("mismatch between time interval width specified in config and link_travel_time table");
		}

		//store data
		std::map<unsigned int, sim_mob::LinkTravelTime>::iterator lttIt = lnkTravelTimeMap.find(linkId); // must have an entry for all link ids after loading default travel times
		if (lttIt == lnkTravelTimeMap.end())
		{
			throw std::runtime_error("linkId specified in historical travel time table does not have a default travel time");
		}
		LinkTravelTime& lnkTT = lttIt->second;
		lnkTT.addHistoricalTravelTime(startTime, downstreamLinkId, travelTime);
	}

}

double sim_mob::TravelTimeManager::getDefaultLinkTT(const Link* lnk) const
{
	std::map<unsigned int, sim_mob::LinkTravelTime>::const_iterator it = lnkTravelTimeMap.find(lnk->getLinkId());
	if (it == lnkTravelTimeMap.end())
	{
		std::stringstream out;
		out << "NO default TT FOR : " << lnk->getLinkId() << "\n";
		throw std::runtime_error(out.str());
	}
	const LinkTravelTime& lnkTT = it->second;
	return lnkTT.getDefaultTravelTime();
}

double sim_mob::TravelTimeManager::getLinkTT(const sim_mob::Link* lnk, const sim_mob::DailyTime& startTime, const sim_mob::Link* downstreamLink) const
{
	std::map<unsigned int, sim_mob::LinkTravelTime>::const_iterator it = lnkTravelTimeMap.find(lnk->getLinkId());
	if (it == lnkTravelTimeMap.end())
	{
		std::stringstream out;
		out << "NO TT FOR : " << lnk->getLinkId() << "\n";
		throw std::runtime_error(out.str());
	}
	const LinkTravelTime& lnkTT = it->second;
	double res = 0;
	if(downstreamLink)
	{
		res = lnkTT.getHistoricalLinkTT(downstreamLink->getLinkId(), startTime);
	}
	else
	{
		res = lnkTT.getHistoricalLinkTT(startTime);
	}

	if (res <= 0.0)
	{
		//check default if travel time is not found
		res = lnkTT.getDefaultTravelTime();
	}
	return res;
}

void sim_mob::TravelTimeManager::addTravelTime(const LinkTravelStats& stats)
{
	std::map<unsigned int, sim_mob::LinkTravelTime>::iterator ttMapIt = lnkTravelTimeMap.find(stats.link->getLinkId());
	if(ttMapIt == lnkTravelTimeMap.end())
	{
		std::stringstream errStrm;
		errStrm << "Link " << stats.link->getLinkId() << " has no entry in lnkTravelTimeMap\n";
		throw std::runtime_error(errStrm.str());
	}
	ttMapIt->second.addInSimulationTravelTime(stats);
}

double sim_mob::TravelTimeManager::getInSimulationLinkTT(const sim_mob::Link *lnk) const
{
	return enRouteTT->getInSimulationLinkTT(lnk);
}

double sim_mob::TravelTimeManager::EnRouteTT::getInSimulationLinkTT(const sim_mob::Link *lnk) const
{
	throw std::runtime_error("EnRouteTT::getInSimulationLinkTT not implemented");
//	boost::shared_lock<boost::shared_mutex> lock(parent.ttMapMutex);
//	//[time interval][travel mode][road segment][average travel time]
//	//<-----TI-----><-------------------MRTC----------------------->
//	//start from the last recorded time interval (before the current time interval) and proceed to find a travel time for the given section.
//	//if no records found, check the previous time interval and so on.
//	sim_mob::TravelTime::reverse_iterator itTI = parent.ttMap.rbegin();
//	//like I said, not the current interval
//	if(itTI != parent.ttMap.rend() && itTI->first == sim_mob::PathSetManager::curIntervalMS) { itTI++; }
//	sim_mob::TT::MRTC::iterator itMode;
//	sim_mob::TT::RSTC::iterator itSeg;
//	//search backwards. try to find a matching road segment in any of the previous time intervals
//	while(itTI != parent.ttMap.rend())
//	{
//		itMode = itTI->second.find(mode);
//		if(itMode != itTI->second.end())
//		{
//			itSeg = itMode->second.find(lnk);
//			if(itSeg != itMode->second.end())
//			{
//				return itSeg->second.totalTravelTime / itSeg->second.travelTimeCnt;
//			}
//		}
//		++itTI;
//	}
}

void sim_mob::TravelTimeManager::dumpTravelTimesToFile(const std::string fileName) const
{
	for(std::map<unsigned int, sim_mob::LinkTravelTime>::const_iterator lnkTravelTimeIt=lnkTravelTimeMap.begin(); lnkTravelTimeIt!=lnkTravelTimeMap.end(); lnkTravelTimeIt++)
	{
		lnkTravelTimeIt->second.dumpTravelTimesToFile(fileName);
	}
	sim_mob::Logger::log(fileName).flush();
}

bool sim_mob::TravelTimeManager::storeCurrentSimulationTT()
{
	dumpTravelTimesToFile(historicalTT_TableName);
	sim_mob::Logger::log(historicalTT_TableName).flush();
	return true;
}

sim_mob::TravelTimeManager* sim_mob::TravelTimeManager::getInstance()
{
	if(!instance)
	{
		instance = new TravelTimeManager();
	}
	return instance;
}
