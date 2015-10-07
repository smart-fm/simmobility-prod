#include "PathSetParam.hpp"

#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "PathSetManager.hpp"
#include "util/Profiler.hpp"


namespace{
//sim_mob::BasicLogger & logger = sim_mob::Logger::log("pathset.log");
}

sim_mob::PathSetParam *sim_mob::PathSetParam::instance_ = NULL;

sim_mob::PathSetParam* sim_mob::PathSetParam::getInstance()
{
	if(!instance_)
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
	soci::session dbSession(soci::postgresql,dbStr);
	setRTTT(cfg.getRTTT());
	sim_mob::aimsun::Loader::LoadERPData(cfg.getDatabaseConnectionString(false), ERP_SurchargePool,	ERP_Gantry_ZonePool, ERP_SectionPool);
	sim_mob::aimsun::Loader::LoadDefaultTravelTimeData(dbSession, segDefTT);
	bool res = sim_mob::aimsun::Loader::LoadRealTimeTravelTimeData(dbSession, cfg.pathSet().interval, segHistoryTT);
	if(!res) // no realtime travel time table
	{
		//create
		if(!createTravelTimeRealtimeTable(dbSession) )
		{
			throw std::runtime_error("can not create travel time table");
		}
	}
}
void sim_mob::PathSetParam::storeSinglePath(soci::session& sql,std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool,const std::string pathSetTableName)
{
	sim_mob::aimsun::Loader::storeSinglePath(sql,spPool,pathSetTableName);
}

bool sim_mob::PathSetParam::createTravelTimeRealtimeTable(soci::session& dbSession)
{
	bool res=false;
	std::string createTableStr = "create table " + RTTT +
			"("
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
			"ALTER TABLE " + RTTT +
			"  OWNER TO postgres;";
	res = sim_mob::aimsun::Loader::excuString(dbSession,createTableStr);
	return res;
}

void sim_mob::PathSetParam::setRTTT(const std::string& value)
{
	if(!value.size())
	{
		throw std::runtime_error("Missing Travel Time Table Name.\n "
				"It is either missing in the XML configuration file,\n"
				"or you are trying to access the file name before reading the Configuration file");
	}
	RTTT = value;
	//logger << "[REALTIME TABLE NAME : " << RTTT << "]\n";
}

double sim_mob::PathSetParam::getSegRangeTT(const sim_mob::RoadSegment* rs, const std::string travelMode, const sim_mob::DailyTime& startTime, const sim_mob::DailyTime& endTime) const
{
	//1. check realtime table
	double res=0.0;
	double totalTravelTime=0.0;
	int count = 0;
	sim_mob::DailyTime dailyTime(startTime);
	TT::TI endTimeRange = TravelTimeManager::getTimeInterval(endTime.getValue(), intervalMS);
	while(dailyTime.isBeforeEqual(endTime))
	{
		double tt = getSegTT(rs,travelMode, dailyTime);
		totalTravelTime += tt;
		if(tt > 0.0)
		{
			count ++;
		}
		dailyTime = sim_mob::DailyTime(dailyTime.getValue() + intervalMS) ;
	}
	return (count ? totalTravelTime / count : 0.0);
}

double sim_mob::PathSetParam::getDefSegTT(const sim_mob::RoadSegment* rs) const
{
	/*Note:
	 * this method doesn't look at the intended time range.
	 * Instead, it searches for all occurrences of the given road segment in the
	 * default travel time container, and returns an average.
	 */
	boost::unordered_map<unsigned long, sim_mob::SegmentTravelTimeVector*>::const_iterator it = segDefTT.find(rs->getId());
	if(it == segDefTT.end() || it->second->vecSegTT.empty())
	{
		std::stringstream out("");
		out <<  "[NO DTT FOR : " <<  rs->getId() << "]\n";
		//logger << out.str();
		throw std::runtime_error(out.str());
	}

	const std::vector<sim_mob::SegmentTravelTime>& e = it->second->vecSegTT;
	double totalTravelTime = 0.0;
	for(std::vector<sim_mob::SegmentTravelTime>::const_iterator lttIt=e.begin(); lttIt!=e.end(); lttIt++)
	{
		totalTravelTime = totalTravelTime + (*lttIt).travelTime;
	}
	return (totalTravelTime / e.size());
}

double sim_mob::PathSetParam::getDefSegTT(const sim_mob::RoadSegment* rs, const sim_mob::DailyTime &startTime) const
{
	/*
	 *	Note:
	 *	This method searches for the target segment,
	 *	if found, it returns the first occurrence of travel time
	 *	which includes the given time
	 */
	boost::unordered_map<unsigned long, sim_mob::SegmentTravelTimeVector*>::const_iterator it = segDefTT.find(rs->getId());
	if(it == segDefTT.end()) { return 0.0; }
	const std::vector<sim_mob::SegmentTravelTime>& e = (*it).second->vecSegTT;
	for(std::vector<sim_mob::SegmentTravelTime>::const_iterator itL(e.begin());itL != e.end();++itL)
	{
		const sim_mob::SegmentTravelTime& l = *itL;
		if( l.startTime_DT.isBeforeEqual(startTime) && l.endTime_DT.isAfter(startTime) )
		{
			return l.travelTime;
		}
	}
	return 0.0;
}

double sim_mob::PathSetParam::getHistorySegTT(const sim_mob::RoadSegment* rs, const std::string &travelMode, const sim_mob::DailyTime &startTime) const
{
	std::ostringstream dbg("");
	//1. check realtime table
	double res = 0.0;
	TT::TI timeInterval = TravelTimeManager::getTimeInterval(startTime.getValue(),intervalMS);
	AverageTravelTime::const_iterator itRange = segHistoryTT.find(timeInterval);
	if(itRange != segHistoryTT.end())
	{
		const sim_mob::TT::MST& mst = itRange->second;
		sim_mob::TT::MST::const_iterator itMode = mst.find(travelMode);
		if(itMode != mst.end())
		{
			std::map<const sim_mob::RoadSegment*,double >::const_iterator itRS = itMode->second.find(rs);
			if(itRS != itMode->second.end())
			{
//				logger << startTime.getRepr_() << " [REALTT] " <<  "  " << dbg.str() << "\n";
				return itRS->second;
			}
		}
	}
	return 0.0;
}

double sim_mob::PathSetParam::getSegTT(const sim_mob::RoadSegment* rs, const std::string &travelMode, const sim_mob::DailyTime &startTime) const
{
	//check realtime table
	double res = getHistorySegTT(rs, travelMode, startTime);
	if(res <= 0.0)
	{
		//check default if travel time is not found
		res = getDefSegTT(rs, startTime);
	}
	return res;
}

void sim_mob::PathSetParam::initParameters()
{
	const PathSetConf & pathset = sim_mob::ConfigManager::GetInstance().PathSetConfig();
	bTTVOT = pathset.params.bTTVOT ;//-0.01373;//-0.0108879;
	bCommonFactor =pathset.params.bCommonFactor ;// 1.0;
	bLength = pathset.params.bLength ;//-0.001025;//0.0; //negative sign proposed by milan
	bHighway =pathset.params.bHighway ;// 0.00052;//0.0;
	bCost = pathset.params.bCost ;//0.0;
	bSigInter = pathset.params.bSigInter ;//-0.13;//0.0;
	bLeftTurns = pathset.params.bLeftTurns ;//0.0;
	bWork = pathset.params.bWork ;//0.0;
	bLeisure = pathset.params.bLeisure ;//0.0;
	highwayBias = pathset.params.highwayBias ;//0.5;
	minTravelTimeParam = pathset.params.minTravelTimeParam ;//0.879;
	minDistanceParam = pathset.params.minDistanceParam ;//0.325;
	minSignalParam = pathset.params.minSignalParam ;//0.256;
	maxHighwayParam = pathset.params.maxHighwayParam ;//0.422;
}

//todo:obsolete
uint32_t sim_mob::PathSetParam::getSize()
{
	uint32_t sum = 0;
	sum += sizeof(double); //double bTTVOT;
	sum += sizeof(double); //double bCommonFactor;
	sum += sizeof(double); //double bLength;
	sum += sizeof(double); //double bHighway;
	sum += sizeof(double); //double bCost;
	sum += sizeof(double); //double bSigInter;
	sum += sizeof(double); //double bLeftTurns;
	sum += sizeof(double); //double bWork;
	sum += sizeof(double); //double bLeisure;
	sum += sizeof(double); //double highwayBias;
	sum += sizeof(double); //double minTravelTimeParam;
	sum += sizeof(double); //double minDistanceParam;
	sum += sizeof(double); //double minSignalParam;
	sum += sizeof(double); //double maxHighwayParam;

	//std::map<std::string,sim_mob::RoadSegment*> segPool;
	typedef std::map<std::string,sim_mob::RoadSegment*>::value_type SPP;

//		const std::vector<sim_mob::MultiNode*>  &multiNodesPool;
	sum += sizeof(sim_mob::MultiNode*) * multiNodesPool.size();

//		const std::set<sim_mob::UniNode*> & uniNodesPool;
	sum += sizeof(sim_mob::UniNode*) * uniNodesPool.size();

//		std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> > ERP_SurchargePool;
	typedef std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> >::value_type ERPSCP;
	BOOST_FOREACH(ERPSCP & ERP_Surcharge_pool_pair,ERP_SurchargePool)
	{
		sum += ERP_Surcharge_pool_pair.first.length();
		sum += sizeof(sim_mob::ERP_Surcharge*) * ERP_Surcharge_pool_pair.second.size();
	}

//		std::map<std::string,sim_mob::ERP_Gantry_Zone*> ERP_Gantry_ZonePool;
	typedef std::map<std::string,sim_mob::ERP_Gantry_Zone*>::value_type ERPGZP;
	BOOST_FOREACH(ERPGZP & ERP_Gantry_Zone_pool_pair,ERP_Gantry_ZonePool)
	{
		sum += ERP_Gantry_Zone_pool_pair.first.length();
	}
	sum += sizeof(sim_mob::ERP_Gantry_Zone*) * ERP_Gantry_ZonePool.size();

//		std::map<std::string,sim_mob::ERP_Section*> ERP_Section_pool;
	typedef std::map<int,sim_mob::ERP_Section*>::value_type  ERPSP;
	BOOST_FOREACH(ERPSP&ERP_Section_pair,ERP_SectionPool)
	{
		sum += sizeof(int);
	}
	sum += sizeof(sim_mob::ERP_Section*) * ERP_SectionPool.size();

//		std::map<std::string,std::vector<sim_mob::LinkTravelTime*> > segDefTT;
	typedef boost::unordered_map<unsigned long, sim_mob::SegmentTravelTimeVector*>::value_type LDTTPP;
	BOOST_FOREACH(LDTTPP& ldttpp, segDefTT)
	{
		sum += sizeof(unsigned long);
		sum += sizeof(sim_mob::SegmentTravelTime) * ldttpp.second->vecSegTT.size();
	}
	//todo historical avg travel time
	sum += sizeof(sim_mob::RoadNetwork&);

	sum += RTTT.length();
	return sum;
}

sim_mob::PathSetParam::PathSetParam() :
		roadNetwork(ConfigManager::GetInstance().FullConfig().getNetwork()),
		multiNodesPool(ConfigManager::GetInstance().FullConfig().getNetwork().getNodes()), uniNodesPool(ConfigManager::GetInstance().FullConfig().getNetwork().getUniNodes()),
		RTTT(""),intervalMS(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().interval* 1000 /*milliseconds*/)
{
	initParameters();
	populate();
}

sim_mob::PathSetParam::~PathSetParam()
{
	//clear ERP_SurchargePool
	for(std::map<std::string, std::vector<sim_mob::ERP_Surcharge*> >::iterator mapIt=ERP_SurchargePool.begin(); mapIt!=ERP_SurchargePool.end(); mapIt++)
	{
		for(std::vector<sim_mob::ERP_Surcharge*>::iterator vecIt=mapIt->second.begin(); vecIt!=mapIt->second.end(); vecIt++)
		{
			safe_delete_item(*vecIt);
		}
		mapIt->second.clear();
	}
	ERP_SurchargePool.clear();

	//clear ERP_Gantry_ZonePool
	for(std::map<std::string,sim_mob::ERP_Gantry_Zone*>::iterator mapIt=ERP_Gantry_ZonePool.begin(); mapIt!=ERP_Gantry_ZonePool.end(); mapIt++)
	{
		safe_delete_item(mapIt->second);
	}
	ERP_Gantry_ZonePool.clear();

	//clear ERP_SectionPool
	for(std::map<int,sim_mob::ERP_Section*>::iterator mapIt=ERP_SectionPool.begin(); mapIt!=ERP_SectionPool.end(); mapIt++)
	{
		safe_delete_item(mapIt->second);
	}
	ERP_SectionPool.clear();

	//clear segDefTT
	//clear ERP_SectionPool
	for(boost::unordered_map<unsigned long, sim_mob::SegmentTravelTimeVector*>::iterator mapIt=segDefTT.begin(); mapIt!=segDefTT.end(); mapIt++)
	{
		safe_delete_item(mapIt->second);
	}
	segDefTT.clear();
}

sim_mob::ERP_Section::ERP_Section(ERP_Section &src)
	: section_id(src.section_id),ERP_Gantry_No(src.ERP_Gantry_No)
{
	ERP_Gantry_No_str = boost::lexical_cast<std::string>(src.ERP_Gantry_No);
}

sim_mob::SegmentTravelTime::SegmentTravelTime(const SegmentTravelTime& src)
	: linkId(src.linkId),
			startTime(src.startTime),endTime(src.endTime),travelTime(src.travelTime),interval(src.interval)
			,startTime_DT(sim_mob::DailyTime(src.startTime)),endTime_DT(sim_mob::DailyTime(src.endTime))
{
}
sim_mob::SegmentTravelTime::SegmentTravelTime()
	: linkId(0),
			startTime(""),endTime(""),travelTime(0.0),
			startTime_DT(0),endTime_DT(0)
{
}
