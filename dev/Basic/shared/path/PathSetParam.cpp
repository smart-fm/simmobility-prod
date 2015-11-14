#include "PathSetParam.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "SOCI_Converters.hpp"

namespace
{

//sim_mob::BasicLogger & logger = sim_mob::Logger::log("pathset.log");
unsigned int TT_STORAGE_TIME_INTERVAL_WIDTH = 0;
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
	loadERP_Surcharge(dbSession);
	loadERP_Section(dbSession);
	loadERP_GantryZone(dbSession);
	loadLinkDefaultTravelTime(dbSession);
	loadLinkHistoricalTravelTime(dbSession);
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

double sim_mob::PathSetParam::getLinkTT(const sim_mob::Link* lnk, const sim_mob::Link* downstreamLink, const sim_mob::DailyTime &startTime) const
{
	LinkTravelTime::TimeInterval timeInterval = LinkTravelTime::getTimeInterval(startTime);
	std::map<unsigned long, sim_mob::LinkTravelTime>::const_iterator it = lnkTravelTimeMap.find(lnk->getLinkId());
	if (it == lnkTravelTimeMap.end())
	{
		std::stringstream out;
		out << "NO DTT FOR : " << lnk->getLinkId() << "\n";
		throw std::runtime_error(out.str());
	}
	const LinkTravelTime& lnkTT = it->second;
	double res = 0;
	if(downstreamLink)
	{
		res = lnkTT.getHistoricalLinkTT(downstreamLink->getLinkId(), timeInterval);
	}
	else
	{
		res = lnkTT.getHistoricalLinkTT(timeInterval);
	}

	if (res <= 0.0)
	{
		//check default if travel time is not found
		res = lnkTT.getDefaultTravelTime();
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
}

void sim_mob::PathSetParam::loadERP_Surcharge(soci::session& sql)
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
		std::map<std::string, std::vector<sim_mob::ERP_Surcharge*> >::iterator itt = ERP_SurchargePool.find(s->gantryNo);
		if (itt != ERP_SurchargePool.end())
		{
			std::vector<sim_mob::ERP_Surcharge*> e = (*itt).second;
			e.push_back(s);
			ERP_SurchargePool[s->gantryNo] = e;
		}
		else
		{
			std::vector<sim_mob::ERP_Surcharge*> e;
			e.push_back(s);
			ERP_SurchargePool[s->gantryNo] = e;
		}
	}
}

void sim_mob::PathSetParam::loadERP_Section(soci::session& sql)
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

void sim_mob::PathSetParam::loadERP_GantryZone(soci::session& sql)
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
		ERP_Gantry_ZonePool.insert(std::make_pair(s->gantryNo, s));
	}
}

void sim_mob::PathSetParam::loadLinkDefaultTravelTime(soci::session& sql)
{
	const std::string &tableName = sim_mob::ConfigManager::GetInstance().PathSetConfig().DTT_Conf;
	std::string query = "select link_id, to_char(start_time,'HH24:MI:SS') AS start_time, to_char(end_time,'HH24:MI:SS') AS end_time, travel_time from " + tableName;
	soci::rowset<sim_mob::LinkTravelTime> rs = sql.prepare << query;

	for (soci::rowset<sim_mob::LinkTravelTime>::iterator lttIt = rs.begin(); lttIt != rs.end(); ++lttIt)
	{
		lnkTravelTimeMap[lttIt->getLinkId()] = *lttIt;
	}
}

void sim_mob::PathSetParam::loadLinkHistoricalTravelTime(soci::session& sql)
{
	const sim_mob::ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();
	TT_STORAGE_TIME_INTERVAL_WIDTH = cfg.getPathSetConf().interval * 1000; // initialize to proper interval from config
	const std::string &tableName = sim_mob::ConfigManager::GetInstance().PathSetConfig().RTTT_Conf;
	std::string query = "select link_id, downstream_link_id, to_char(start_time,'HH24:MI:SS') AS start_time, to_char(end_time,'HH24:MI:SS') AS end_time,"
			"travel_time from " + tableName + " order by link_id, downstream_link_id";

	//main loop
		unsigned int timeInterval;
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
			if(interval.getValue() != TT_STORAGE_TIME_INTERVAL_WIDTH)
			{
				throw std::runtime_error("mismatch between time interval width specified in config and that found from link_travel_time table");
			}
			timeInterval = sim_mob::LinkTravelTime::getTimeInterval(startTime);

			//store data
			std::map<unsigned int, sim_mob::LinkTravelTime>::iterator lttIt = lnkTravelTimeMap.find(linkId); // must have an entry for all link ids after loading default travel times
			if(lttIt == lnkTravelTimeMap.end())
			{
				throw std::runtime_error("linkId specified in historical travel time table does not have a default travel time");
			}
			LinkTravelTime& lnkTT = lttIt->second;
			lnkTT.addHistoricalTravelTime(downstreamLinkId, travelTime);
		}

}

sim_mob::LinkTravelTime::LinkTravelTime() : linkId(0), defaultTravelTime(0.0)
{
}

sim_mob::LinkTravelTime::~LinkTravelTime()
{
}

sim_mob::LinkTravelTime::TimeInterval sim_mob::LinkTravelTime::getTimeInterval(const DailyTime& dt) const
{
	if(TT_STORAGE_TIME_INTERVAL_WIDTH == 0)
	{
		throw std::runtime_error("width of time interval for travel time storage is 0");
	}
	return (dt.getValue() / TT_STORAGE_TIME_INTERVAL_WIDTH);
}

void sim_mob::LinkTravelTime::addHistoricalTravelTime(unsigned int downstreamLinkId, double travelTime)
{
	downstreamLinkTT_Map[downstreamLinkId] = travelTime;
}

double sim_mob::LinkTravelTime::getHistoricalLinkTT(unsigned int downstreamLinkId, unsigned int timeInterval) const
{
	TravelTimeStore::const_iterator ttMapIt = downstreamLinkTT_Map.find(timeInterval);
	if(ttMapIt == downstreamLinkTT_Map.end())
	{
		return -1;
	}
	const LinkDownStreamLinkTT_Map& ttInnerMap = ttMapIt->second;
	LinkDownStreamLinkTT_Map::const_iterator ttInnerMapIt = ttInnerMap.find(downstreamLinkId);
	if(ttInnerMapIt == ttInnerMap.end())
	{
		return -1;
	}
	return ttInnerMapIt->second;
}

double sim_mob::LinkTravelTime::getHistoricalLinkTT(unsigned int timeInterval) const
{
	TravelTimeStore::const_iterator ttMapIt = downstreamLinkTT_Map.find(timeInterval);
	if(ttMapIt == downstreamLinkTT_Map.end())
	{
		return -1;
	}
	const LinkDownStreamLinkTT_Map& ttInnerMap = ttMapIt->second;
	if(ttInnerMap.empty())
	{
		return -1;
	}
	double totalTT = 0.0;
	for(LinkDownStreamLinkTT_Map::const_iterator ttInnerMapIt=ttInnerMap.begin(); ttInnerMapIt!=ttInnerMap.end(); ttInnerMapIt++)
	{
		totalTT = totalTT + ttInnerMapIt->second;
	}
	return (totalTT/ttInnerMap.size());
}
