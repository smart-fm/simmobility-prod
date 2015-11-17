#include "PathSetParam.hpp"

#include <boost/foreach.hpp>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "SOCI_Converters.hpp"
#include "util/Profiler.hpp"

namespace
{

//sim_mob::BasicLogger & logger = sim_mob::Logger::log("pathset.log");
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
	loadERP_Surcharge(dbSession);
	loadERP_Section(dbSession);
	loadERP_GantryZone(dbSession);
	ttMgr->loadTravelTimes();
}

void sim_mob::PathSetParam::storeSinglePath(soci::session& sql, std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool, const std::string pathSetTableName)
{
	if(ConfigManager::GetInstance().PathSetConfig().privatePathSetMode == "generation")
	{
		sim_mob::BasicLogger & pathsetCSV = sim_mob::Logger::log(ConfigManager::GetInstance().PathSetConfig().bulkFile);
		BOOST_FOREACH(sim_mob::SinglePath* sp, spPool)
		{
			if(sp->isNeedSave2DB)
			{
				pathsetCSV << ("\"" + sp->id + "\"") << ","
						<< ("\"" + sp->pathSetId + "\"") << ","
						<< sp->partialUtility << ","
						<< sp->pathSize << ","
						<< sp->signalNumber << ","
						<< sp->rightTurnNumber << ","
						<< ("\"" + sp->scenario  + "\"") << ","
						<< sp->length << ","
						<< sp->highWayDistance << ","
						<< sp->isMinDistance << ","
						<< sp->isMinSignal << ","
						<< sp->isMinRightTurn << ","
						<< sp->isMaxHighWayUsage << ","
						<< sp->valid_path << ","
						<< sp->isShortestPath << ","
						<< sp->travelTime << ","
						<< sp->isMinTravelTime << ","
						<< sp->pathSetId << "\n";
			}
		}
	}
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
		roadNetwork(*(RoadNetwork::getInstance())),
		intervalMS(sim_mob::ConfigManager::GetInstance().FullConfig().getPathSetConf().interval * 1000 /*milliseconds*/),
		ttMgr(TravelTimeManager::getInstance())
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
