#include "PathSetParam.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "SOCI_Converters.hpp"

namespace
{

//sim_mob::BasicLogger & logger = sim_mob::Logger::log("pathset.log");

void loadERP_Surcharge(soci::session sql, std::map<std::string, std::vector<sim_mob::ERP_Surcharge*> >& pool)
{
	const std::map<std::string, std::string>& procMap = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().procedureMappings;
	const std::map<std::string, std::string>::const_iterator erpSurchargeIt = procMap.find("erp_surcharge");
	if(erpSurchargeIt == procMap.end())
	{
		throw std::runtime_error("missing stored procedure name for erp_surcharge in proc map");
	}
	soci::rowset<sim_mob::ERP_Surcharge> rs = (sql.prepare << "select * from " << erpSurchargeIt->second);
	for (soci::rowset<sim_mob::ERP_Surcharge>::const_iterator it = rs.begin(); it != rs.end(); ++it)
	{
		sim_mob::ERP_Surcharge *s = new sim_mob::ERP_Surcharge(*it);
		std::map<std::string, std::vector<sim_mob::ERP_Surcharge*> >::iterator itt = pool.find(s->gantryNo);
		if (itt != pool.end())
		{
			std::vector<sim_mob::ERP_Surcharge*> e = (*itt).second;
			e.push_back(s);
			pool[s->gantryNo] = e;
		}
		else
		{
			std::vector<sim_mob::ERP_Surcharge*> e;
			e.push_back(s);
			pool[s->gantryNo] = e;
		}
	}
}

void loadERP_Section(soci::session sql, std::map<int, sim_mob::ERP_Section*>& ERP_SectionPool)
{
	const std::map<std::string, std::string>& procMap = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().procedureMappings;
	const std::map<std::string, std::string>::const_iterator erpSectionIt = procMap.find("erp_section");
	if(erpSectionIt == procMap.end())
	{
		throw std::runtime_error("missing stored procedure name for erp_section in proc map");
	}
	soci::rowset<sim_mob::ERP_Section> rs = (sql.prepare << "select * from " << erpSectionIt->second);
	for (soci::rowset<sim_mob::ERP_Section>::const_iterator it = rs.begin(); it != rs.end(); ++it)
	{
		sim_mob::ERP_Section *s = new sim_mob::ERP_Section(*it);
		ERP_SectionPool.insert(std::make_pair(s->linkId, s));
	}
}

void loadERP_GantryZone(soci::session sql, std::map<std::string, sim_mob::ERP_Gantry_Zone*>& ERP_GantryZonePool)
{
	const std::map<std::string, std::string>& procMap = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().procedureMappings;
	const std::map<std::string, std::string>::const_iterator erpGantryZoneIt = procMap.find("erp_gantry_zone");
	if(erpGantryZoneIt == procMap.end())
	{
		throw std::runtime_error("missing stored procedure name for erp_gantry_zone in proc map");
	}
	soci::rowset<sim_mob::ERP_Gantry_Zone> rs = (sql.prepare << "select * from " << erpGantryZoneIt->second);
	for (soci::rowset<sim_mob::ERP_Gantry_Zone>::const_iterator it = rs.begin(); it != rs.end(); ++it)
	{
		sim_mob::ERP_Gantry_Zone *s = new sim_mob::ERP_Gantry_Zone(*it);
		ERP_GantryZonePool.insert(std::make_pair(s->gantryNo, s));
	}
}

void loadLinkDefaultTravelTime(soci::session& sql, std::map<unsigned long, sim_mob::LinkTravelTimeVector*>& pool)
{
	const std::string &tableName = sim_mob::ConfigManager::GetInstance().PathSetConfig().DTT_Conf;
	std::string query = "select link_id, travel_mode, to_char(start_time,'HH24:MI:SS') AS start_time, to_char(end_time,'HH24:MI:SS') AS end_time, travel_time from " + tableName;
	soci::rowset<sim_mob::LinkTravelTime> rs = sql.prepare << query;

	for (soci::rowset<sim_mob::LinkTravelTime>::const_iterator lttIt = rs.begin(); lttIt != rs.end(); ++lttIt)
	{
		std::map<unsigned long, sim_mob::LinkTravelTimeVector*>::iterator poolIt = pool.find(lttIt->linkId);
		if(poolIt == pool.end())
		{
			sim_mob::LinkTravelTimeVector* lnkTT_Vector = new sim_mob::LinkTravelTimeVector();
			lnkTT_Vector->vecSegTT.push_back(*lttIt);
			pool[lttIt->linkId] = lnkTT_Vector;
		}
		else
		{
			poolIt->second->vecSegTT.push_back(*lttIt);
		}
	}
}

void loadLinkRealTimeTravelTime(soci::session& sql, int intervalSec, sim_mob::AverageTravelTime& pool)
{
	int intervalMS = intervalSec * 1000;
	const std::string &tableName = sim_mob::ConfigManager::GetInstance().PathSetConfig().RTTT_Conf;
	std::string query = "select link_id, to_char(start_time,'HH24:MI:SS') AS start_time,"
			"to_char(end_time,'HH24:MI:SS') AS end_time,travel_time, travel_mode from " + tableName + " where interval_time = "
			+ boost::lexical_cast<std::string>(intervalSec);

	//	local cache for optimization purposes
	std::map<unsigned long, const sim_mob::RoadSegment*> rsCache;
	std::map<unsigned long, const sim_mob::RoadSegment*>::iterator rsIt;

	//main loop
	try
	{
		unsigned int timeInterval;
		soci::rowset<sim_mob::LinkTravelTime> rs = (sql.prepare << query);
		for (soci::rowset<sim_mob::LinkTravelTime>::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			timeInterval = sim_mob::TravelTimeManager::getTimeInterval(it->startTimeDT.getValue(), intervalMS);
			//	optimization
			const sim_mob::RoadSegment* rs;
			if ((rsIt = rsCache.find(it->linkId)) != rsCache.end())
			{
				rs = rsIt->second;
			}
			else
			{
				//rs = rsCache[it->linkId] = sim_mob::RoadSegment::allSegments[it->linkId];
			}
			//the main job is just one line:
			pool[timeInterval][it->travelMode][rs] = it->travelTime;
		}
	}
	catch (soci::soci_error const & err)
	{
		std::cout << "[ERROR LOADING REALTIME TRAVEL TIME]: " << err.what() << std::endl;
	}
}

} //anonymous namespace

sim_mob::PathSetParam *sim_mob::PathSetParam::instance_ = NULL;

sim_mob::PathSetParam* sim_mob::PathSetParam::getInstance()
{
	if (!instance_)
	{
		instance_ = new PathSetParam();
	}
	return instance_;
}

void sim_mob::PathSetParam::resetInstance()
{
	delete instance_;
	instance_ = NULL;
}

void sim_mob::PathSetParam::populate()
{
	getDataFromDB();
}

void sim_mob::PathSetParam::getDataFromDB()
{
	const sim_mob::ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();
	std::string dbStr(cfg.getDatabaseConnectionString(false));
	soci::session dbSession(soci::postgresql, dbStr);
	setRTTT(cfg.getRTTT());
	loadERP_Surcharge(dbSession, ERP_SurchargePool);
	loadERP_Section(dbSession, ERP_SectionPool);
	loadERP_GantryZone(dbSession, ERP_Gantry_ZonePool);
	loadLinkDefaultTravelTime(dbSession, segDefTT);
	loadLinkRealTimeTravelTime(dbSession, cfg.getPathSetConf().interval, segHistoryTT);
}
void sim_mob::PathSetParam::storeSinglePath(soci::session& sql, std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool, const std::string pathSetTableName)
{
	sim_mob::aimsun::Loader::storeSinglePath(sql, spPool, pathSetTableName);
}

bool sim_mob::PathSetParam::createTravelTimeRealtimeTable(soci::session& dbSession)
{
	bool res = false;
	std::string createTableStr = "create table " + RTTT + "("
			"link_id integer NOT NULL,"
			"start_time time without time zone NOT NULL,"
			"end_time time without time zone NOT NULL,"
			"travel_time double precision NOT NULL,"
			"interval_time integer NOT NULL,"
			"travel_mode character varying NOT NULL,"
			"history integer NOT NULL DEFAULT 1"
			")"
			"WITH ("
			"  OIDS=FALSE"
			");"
			"ALTER TABLE " + RTTT + "  OWNER TO postgres;";
	res = sim_mob::aimsun::Loader::excuString(dbSession, createTableStr);
	return res;
}

void sim_mob::PathSetParam::setRTTT(const std::string& value)
{
	if (!value.size())
	{
		throw std::runtime_error("Missing Travel Time Table Name.\n "
				"It is either missing in the XML configuration file,\n"
				"or you are trying to access the file name before reading the Configuration file");
	}
	RTTT = value;
	//logger << "[REALTIME TABLE NAME : " << RTTT << "]\n";
}

double sim_mob::PathSetParam::getLinkRangeTT(const Link *lnk, const std::string travelMode, const sim_mob::DailyTime& startTime,
		const sim_mob::DailyTime& endTime) const
{
	//1. check realtime table
	double res = 0.0;
	double totalTravelTime = 0.0;
	int count = 0;
	sim_mob::DailyTime dailyTime(startTime);
	TT::TI endTimeRange = TravelTimeManager::getTimeInterval(endTime.getValue(), intervalMS);
	while (dailyTime.isBeforeEqual(endTime))
	{
		double tt = getLinkTT(lnk, travelMode, dailyTime);
		totalTravelTime += tt;
		if (tt > 0.0)
		{
			count++;
		}
		dailyTime = sim_mob::DailyTime(dailyTime.getValue() + intervalMS);
	}
	return (count ? totalTravelTime / count : 0.0);
}

double sim_mob::PathSetParam::getDefaultLinkTT(const sim_mob::Link* lnk) const
{
	/*Note:
	 * this method doesn't look at the intended time range.
	 * Instead, it searches for all occurrences of the given road segment in the
	 * default travel time container, and returns an average.
	 */
	std::map<unsigned long, sim_mob::LinkTravelTimeVector*>::const_iterator it = segDefTT.find(lnk->getLinkId());
	if (it == segDefTT.end() || it->second->vecSegTT.empty())
	{
		std::stringstream out("");
		out << "[NO DTT FOR : " << lnk->getLinkId() << "]\n";
		//logger << out.str();
		throw std::runtime_error(out.str());
	}

	const std::vector<sim_mob::LinkTravelTime>& e = it->second->vecSegTT;
	double totalTravelTime = 0.0;
	for (std::vector<sim_mob::LinkTravelTime>::const_iterator lttIt = e.begin(); lttIt != e.end(); lttIt++)
	{
		totalTravelTime = totalTravelTime + (*lttIt).travelTime;
	}
	return (totalTravelTime / e.size());
}

double sim_mob::PathSetParam::getDefaultLinkTT(const sim_mob::Link* lnk, const sim_mob::DailyTime &startTime) const
{
	/*
	 *	Note:
	 *	This method searches for the target segment,
	 *	if found, it returns the first occurrence of travel time
	 *	which includes the given time
	 */
	std::map<unsigned long, sim_mob::LinkTravelTimeVector*>::const_iterator it = segDefTT.find(lnk->getLinkId());
	if (it == segDefTT.end())
	{
		return 0.0;
	}
	const std::vector<sim_mob::LinkTravelTime>& e = (*it).second->vecSegTT;
	for (std::vector<sim_mob::LinkTravelTime>::const_iterator itL(e.begin()); itL != e.end(); ++itL)
	{
		const sim_mob::LinkTravelTime& l = *itL;
		if (l.startTimeDT.isBeforeEqual(startTime) && l.endTimeDT.isAfter(startTime))
		{
			return l.travelTime;
		}
	}
	return 0.0;
}

double sim_mob::PathSetParam::getHistoricalLinkTT(const sim_mob::Link* lnk, const std::string &travelMode, const sim_mob::DailyTime &startTime) const
{
	std::ostringstream dbg("");
	//1. check realtime table
	double res = 0.0;
	TT::TI timeInterval = TravelTimeManager::getTimeInterval(startTime.getValue(), intervalMS);
	AverageTravelTime::const_iterator itRange = segHistoryTT.find(timeInterval);
	if (itRange != segHistoryTT.end())
	{
		const sim_mob::TT::MLT& mst = itRange->second;
		sim_mob::TT::MLT::const_iterator itMode = mst.find(travelMode);
		if (itMode != mst.end())
		{
			std::map<const sim_mob::Link*, double>::const_iterator itLnk = itMode->second.find(lnk);
			if (itLnk != itMode->second.end())
			{
				return itLnk->second;
			}
		}
	}
	return 0.0;
}

double sim_mob::PathSetParam::getLinkTT(const sim_mob::Link* lnk, const std::string &travelMode, const sim_mob::DailyTime &startTime) const
{
	//check realtime table
	double res = getHistoricalLinkTT(lnk, travelMode, startTime);
	if (res <= 0.0)
	{
		//check default if travel time is not found
		res = getDefaultLinkTT(lnk, startTime);
	}
	return res;
}

void sim_mob::PathSetParam::initParameters()
{
	const PathSetConf & pathset = sim_mob::ConfigManager::GetInstance().PathSetConfig();
	bTTVOT = pathset.params.bTTVOT; //-0.01373;//-0.0108879;
	bCommonFactor = pathset.params.bCommonFactor; // 1.0;
	bLength = pathset.params.bLength; //-0.001025;//0.0; //negative sign proposed by milan
	bHighway = pathset.params.bHighway; // 0.00052;//0.0;
	bCost = pathset.params.bCost; //0.0;
	bSigInter = pathset.params.bSigInter; //-0.13;//0.0;
	bLeftTurns = pathset.params.bLeftTurns; //0.0;
	bWork = pathset.params.bWork; //0.0;
	bLeisure = pathset.params.bLeisure; //0.0;
	highwayBias = pathset.params.highwayBias; //0.5;
	minTravelTimeParam = pathset.params.minTravelTimeParam; //0.879;
	minDistanceParam = pathset.params.minDistanceParam; //0.325;
	minSignalParam = pathset.params.minSignalParam; //0.256;
	maxHighwayParam = pathset.params.maxHighwayParam; //0.422;
}

sim_mob::PathSetParam::PathSetParam() :
		roadNetwork(*(RoadNetwork::getInstance())), RTTT(""),
			intervalMS(sim_mob::ConfigManager::GetInstance().FullConfig().getPathSetConf().interval * 1000 /*milliseconds*/)
{
	for (std::map<unsigned int, Node *>::const_iterator it = roadNetwork.getMapOfIdvsNodes().begin(); it != roadNetwork.getMapOfIdvsNodes().end(); it++)
	{
		multiNodesPool.push_back(it->second);
	}
	initParameters();
	populate();
}

sim_mob::PathSetParam::~PathSetParam()
{
	//clear ERP_SurchargePool
	for (std::map<std::string, std::vector<sim_mob::ERP_Surcharge*> >::iterator mapIt = ERP_SurchargePool.begin(); mapIt != ERP_SurchargePool.end(); mapIt++)
	{
		for (std::vector<sim_mob::ERP_Surcharge*>::iterator vecIt = mapIt->second.begin(); vecIt != mapIt->second.end(); vecIt++)
		{
			safe_delete_item(*vecIt);
		}
		mapIt->second.clear();
	}
	ERP_SurchargePool.clear();

	//clear ERP_Gantry_ZonePool
	for (std::map<std::string, sim_mob::ERP_Gantry_Zone*>::iterator mapIt = ERP_Gantry_ZonePool.begin(); mapIt != ERP_Gantry_ZonePool.end(); mapIt++)
	{
		safe_delete_item(mapIt->second);
	}
	ERP_Gantry_ZonePool.clear();

	//clear ERP_SectionPool
	for (std::map<int, sim_mob::ERP_Section*>::iterator mapIt = ERP_SectionPool.begin(); mapIt != ERP_SectionPool.end(); mapIt++)
	{
		safe_delete_item(mapIt->second);
	}
	ERP_SectionPool.clear();

	//clear segDefTT
	//clear ERP_SectionPool
	for (std::map<unsigned long, sim_mob::LinkTravelTimeVector*>::iterator mapIt = segDefTT.begin(); mapIt != segDefTT.end(); mapIt++)
	{
		safe_delete_item(mapIt->second);
	}
	segDefTT.clear();
}

sim_mob::ERP_Section::ERP_Section(ERP_Section &src) :
		sectionId(src.sectionId), ERP_Gantry_No(src.ERP_Gantry_No)
{
	ERP_Gantry_No_str = boost::lexical_cast<std::string>(src.ERP_Gantry_No);
}

sim_mob::LinkTravelTime::LinkTravelTime(const LinkTravelTime& src) :
		linkId(src.linkId), travelTime(src.travelTime), startTimeDT(src.startTimeDT), endTimeDT(src.endTimeDT)
{
}

sim_mob::LinkTravelTime::LinkTravelTime() :
		linkId(0), travelTime(0.0), startTimeDT(0), endTimeDT(0)
{
}
