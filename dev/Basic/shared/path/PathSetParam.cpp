#include "PathSetParam.hpp"

#include <boost/thread.hpp>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
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

void sim_mob::PathSetParam::populate()
{
	getDataFromDB();
}

void sim_mob::PathSetParam::getDataFromDB()
{
	setRTTT(ConfigManager::GetInstance().FullConfig().getRTTT());
	//logger << "[RTT TABLE NAME : " << RTTT << "]\n";
	sim_mob::aimsun::Loader::LoadERPData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
			ERP_SurchargePool,	ERP_Gantry_ZonePool,ERP_SectionPool);

	sim_mob::aimsun::Loader::LoadDefaultTravelTimeData(*(PathSetManager::getSession()), segDefTT);

	bool res = sim_mob::aimsun::Loader::LoadRealTimeTravelTimeData(*(PathSetManager::getSession()),
			sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().interval, segHistoryTT);
	if(!res) // no realtime travel time table
	{
		//create
		if(!createTravelTimeRealtimeTable() )
		{
			throw std::runtime_error("can not create travel time table");
		}
	}
}
void sim_mob::PathSetParam::storeSinglePath(soci::session& sql,std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool,const std::string pathSetTableName)
{
	sim_mob::aimsun::Loader::storeSinglePath(sql,spPool,pathSetTableName);
}

bool sim_mob::PathSetParam::createTravelTimeRealtimeTable()
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
	res = sim_mob::aimsun::Loader::excuString(*(PathSetManager::getSession()),createTableStr);
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

double sim_mob::PathSetParam::getSegRangeTT(const sim_mob::RoadSegment* rs, const std::string travelMode, const sim_mob::DailyTime& startTime, const sim_mob::DailyTime& endTime)
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
	std::map<unsigned long,std::vector<sim_mob::LinkTravelTime> >::const_iterator it = segDefTT.find(rs->getId());
	if(it == segDefTT.end() || it->second.empty())
	{
		std::stringstream out("");
		out <<  "[NO DTT FOR : " <<  rs->getId() << "]\n";
		//logger << out.str();
		throw std::runtime_error(out.str());
	}

	const std::vector<sim_mob::LinkTravelTime>& e = (*it).second;
	double totalTravelTime = 0.0;
	BOOST_FOREACH(const sim_mob::LinkTravelTime& lnkTT, e) { totalTravelTime+= lnkTT.travelTime; }
	return (totalTravelTime / e.size());
}

double sim_mob::PathSetParam::getDefSegTT(const sim_mob::RoadSegment* rs, const sim_mob::DailyTime &startTime)
{
	/*
	 *	Note:
	 *	This method searches for the target segment,
	 *	if found, it returns the first occurrence of travel time
	 *	which includes the given time
	 */
	std::map<unsigned long, std::vector<sim_mob::LinkTravelTime> >::iterator it = segDefTT.find(rs->getId());

	if(it == segDefTT.end())
	{
		//logger <<  "[NOTT] " << rs->getId() << "\n";
		return 0.0;
	}

	std::vector<sim_mob::LinkTravelTime> &e = (*it).second;
	for(std::vector<sim_mob::LinkTravelTime>::iterator itL(e.begin());itL != e.end();++itL)
	{
		sim_mob::LinkTravelTime& l = *itL;
		if( l.startTime_DT.isBeforeEqual(startTime) && l.endTime_DT.isAfter(startTime) )
		{
//			logger << rs->getId() << "  " << startTime.getRepr_() << " [DEFTT] " <<  "  " << dbg.str() << "\n";
			return l.travelTime;
		}
	}

	return 0.0;
}

double sim_mob::PathSetParam::getHistorySegTT(const sim_mob::RoadSegment* rs, const std::string &travelMode, const sim_mob::DailyTime &startTime)
{
	std::ostringstream dbg("");
	//1. check realtime table
	double res = 0.0;
	TT::TI timeInterval = TravelTimeManager::getTimeInterval(startTime.getValue(),intervalMS);
	AverageTravelTime::iterator itRange = segHistoryTT.find(timeInterval);
	if(itRange != segHistoryTT.end())
	{
		sim_mob::TT::MST & mst = itRange->second;
		sim_mob::TT::MST::iterator itMode = mst.find(travelMode);
		if(itMode != mst.end())
		{
			std::map<const sim_mob::RoadSegment*,double >::iterator itRS = itMode->second.find(rs);
			if(itRS != itMode->second.end())
			{
//				logger << startTime.getRepr_() << " [REALTT] " <<  "  " << dbg.str() << "\n";
				return itRS->second;
			}
		}
	}
	return 0.0;
}

double sim_mob::PathSetParam::getSegTT(const sim_mob::RoadSegment* rs, const std::string &travelMode, const sim_mob::DailyTime &startTime)
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
	typedef std::map<unsigned long,std::vector<sim_mob::LinkTravelTime> >::value_type LDTTPP;
	BOOST_FOREACH(LDTTPP & ldttpp,segDefTT)
	{
		sum += sizeof(unsigned long);
		sum += sizeof(sim_mob::LinkTravelTime) * ldttpp.second.size();
	}
	//todo historical avg travel time
//		const roadnetwork;
	sum += sizeof(sim_mob::RoadNetwork&);

//		std::string RTTT;
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

sim_mob::ERP_Section::ERP_Section(ERP_Section &src)
	: section_id(src.section_id),ERP_Gantry_No(src.ERP_Gantry_No)
{
	ERP_Gantry_No_str = boost::lexical_cast<std::string>(src.ERP_Gantry_No);
}

sim_mob::LinkTravelTime::LinkTravelTime(const LinkTravelTime& src)
	: linkId(src.linkId),
			startTime(src.startTime),endTime(src.endTime),travelTime(src.travelTime),interval(src.interval)
			,startTime_DT(sim_mob::DailyTime(src.startTime)),endTime_DT(sim_mob::DailyTime(src.endTime))
{
}
sim_mob::LinkTravelTime::LinkTravelTime()
	: linkId(0),
			startTime(""),endTime(""),travelTime(0.0),
			startTime_DT(0),endTime_DT(0)
{
}
