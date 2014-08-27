/*
 * PathSetManager.cpp
 *
 *  Created on: May 6, 2013
 *      Author: Max
 */

#include "PathSetManager.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/PathSet/PathSetThreadPool.h"
#include "util/threadpool/Threadpool.hpp"
#include "workers/Worker.hpp"
#include "message/MessageBus.hpp"
#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/foreach.hpp>
#include <sstream>

using std::vector;
using std::string;

using namespace sim_mob;

std::string getFromToString(const sim_mob::Node* fromNode,const sim_mob::Node* toNode ){
	std::stringstream out("");
	std::string idStrTo = toNode->originalDB_ID.getLogItem();
	std::string idStrFrom = fromNode->originalDB_ID.getLogItem();
	out << getNumberFromAimsunId(idStrFrom) << "," << getNumberFromAimsunId(idStrTo);
	//Print() << "debug: " << idStrFrom << "   " << getNumberFromAimsunId(idStrFrom) << std::endl;
	return out.str();
}

PathSetManager *sim_mob::PathSetManager::instance_;

PathSetParam *sim_mob::PathSetParam::instance_ = NULL;

std::map<boost::thread::id, boost::shared_ptr<soci::session> > sim_mob::PathSetManager::cnnRepo;

sim_mob::PathSetParam* sim_mob::PathSetParam::getInstance()
{
	if(!instance_)
	{
		sim_mob::Logger::log["path_set"].prof("PSP_Inst").tick();
		instance_ = new PathSetParam();
		uint32_t lapse = sim_mob::Logger::log["path_set"].prof("PSP_Inst").tick(true);
		sim_mob::Logger::log["path_set"].profileMsg("Time to create a PathSetParam Instance : ",lapse);
	}
	return instance_;
}

void sim_mob::PathSetParam::getDataFromDB()
{
	setTravleTimeTmpTableName(ConfigManager::GetInstance().FullConfig().getTravelTimeTmpTableName());
		createTravelTimeTmpTable();

		sim_mob::aimsun::Loader::LoadERPData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
				ERP_Surcharge_pool,
				ERP_Gantry_Zone_pool,
				ERP_Section_pool);
		sim_mob::Logger::log["path_set"] <<
				"ERP data retrieved from database[ERP_Surcharge_pool,ERP_Gantry_Zone_pool,ERP_Section_pool]: " <<
				ERP_Surcharge_pool.size() << " "  << ERP_Gantry_Zone_pool.size() << " " << ERP_Section_pool.size() << "\n";

		sim_mob::aimsun::Loader::LoadDefaultTravelTimeData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
				Link_default_travel_time_pool);
		sim_mob::Logger::log["path_set"] << Link_default_travel_time_pool.size() << " records for Link_default_travel_time found\n";

		bool res = sim_mob::aimsun::Loader::LoadRealTimeTravelTimeData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
				pathset_traveltime_realtime_table_name,
				Link_realtime_travel_time_pool);
		sim_mob::Logger::log["path_set"] << Link_realtime_travel_time_pool.size() << " records for Link_realtime_travel_time found\n";
		if(!res) // no realtime travel time table
		{
			//create
			if(!createTravelTimeRealtimeTable() )
			{
				throw std::runtime_error("can not create travel time table");
			}
		}
}
void sim_mob::PathSetParam::storeSinglePath(soci::session& sql,std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool,const std::string singlePathTableName)
{
	sim_mob::aimsun::Loader::SaveOneSinglePathDataST(sql,spPool,singlePathTableName);
}
void sim_mob::PathSetParam::storePathSet(soci::session& sql,std::map<std::string,boost::shared_ptr<sim_mob::PathSet> >& psPool,const std::string pathSetTableName)
{
	sim_mob::aimsun::Loader::SaveOnePathSetDataST(sql,psPool,pathSetTableName);
}
bool sim_mob::PathSetParam::createTravelTimeTmpTable()
{
	bool res=false;
	dropTravelTimeTmpTable();
	// create tmp table
	std::string create_table_str = pathset_traveltime_tmp_table_name + " ( \"link_id\" integer NOT NULL,\"start_time\" time without time zone NOT NULL,\"end_time\" time without time zone NOT NULL,\"travel_time\" double precision )";
	res = sim_mob::aimsun::Loader::createTable(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),create_table_str);
	return res;
}
bool sim_mob::PathSetParam::dropTravelTimeTmpTable()
{
	bool res=false;
	//drop tmp table
	std::string drop_table_str = "drop table \""+ pathset_traveltime_tmp_table_name +"\" ";
	res = sim_mob::aimsun::Loader::excuString(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),drop_table_str);
	return res;
}
bool sim_mob::PathSetParam::createTravelTimeRealtimeTable()
{
	bool res=false;
	std::string create_table_str = pathset_traveltime_realtime_table_name + " ( \"link_id\" integer NOT NULL,\"start_time\" time without time zone NOT NULL,\"end_time\" time without time zone NOT NULL,\"travel_time\" double precision )";
	res = sim_mob::aimsun::Loader::createTable(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),create_table_str);
	return res;
}

void sim_mob::PathSetParam::setTravleTimeTmpTableName(const std::string& value)
{
	pathset_traveltime_tmp_table_name = value+"_"+"traveltime_tmp"; // each user only has fix tmp table name
	sim_mob::Logger::log["path_set"]<<"setTravleTimeTmpTableName: "<<pathset_traveltime_tmp_table_name<<std::endl;
	pathset_traveltime_realtime_table_name = value+"_travel_time";
}
double sim_mob::PathSetParam::getAverageTravelTimeBySegIdStartEndTime(std::string id,sim_mob::DailyTime startTime,sim_mob::DailyTime endTime)
{
	//1. check realtime table
	double res=0.0;
	double totalTravelTime=0.0;
	int count=0;
	std::map<std::string,std::vector<sim_mob::LinkTravelTime*> >::iterator it =
			Link_realtime_travel_time_pool.find(id);
	if(it!=Link_realtime_travel_time_pool.end())
	{
		sim_mob::Logger::log["path_set"] << "using realtime travel time \n";
		std::vector<sim_mob::LinkTravelTime*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::LinkTravelTime* l = e[i];
			if( l->start_time_dt.isAfterEqual(startTime) && l->end_time_dt.isBeforeEqual(endTime) )
			{
				totalTravelTime += l->travel_time;
				count++;
			}
		}
		if(count!=0)
		{
			res = totalTravelTime/count;
			return res;
		}
	}
	//2. if no , check default
	it = Link_default_travel_time_pool.find(id);
	if(it!=Link_default_travel_time_pool.end())
	{
//		sim_mob::Logger::log["path_set"] << "using default travel time \n";
		std::vector<sim_mob::LinkTravelTime*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::LinkTravelTime* l = e[i];
				totalTravelTime += l->travel_time;
				count++;
		}
		if(count != 0)
		{
			res = totalTravelTime/count;
		}
		return res;
	}
	else
	{
		sim_mob::Logger::log["path_set"]<<"getAverageTravelTimeBySegIdStartEndTime=> no travel time for segment " << id << "\n";
	}
	return res;
}

double sim_mob::PathSetParam::getDefaultTravelTimeBySegId(std::string id)
{
	double res=0.0;
	double totalTravelTime=0.0;
	int count=0;
	std::map<std::string,std::vector<sim_mob::LinkTravelTime*> >::iterator it =
			Link_default_travel_time_pool.find(id);
	if(it!=Link_default_travel_time_pool.end())
	{
		std::vector<sim_mob::LinkTravelTime*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::LinkTravelTime* l = e[i];
				totalTravelTime += l->travel_time;
				count++;
		}
		if(count != 0)
		{
			res = totalTravelTime/count;
		}
		return res;
	}
	else
	{
		std::string str = "getDefaultTravelTimeBySegId: no default travel time for segment " + id;
		sim_mob::Logger::log["path_set"]<<"error: "<<str<<std::endl;
	}
	return res;
}
double sim_mob::PathSetParam::getTravelTimeBySegId(std::string id,sim_mob::DailyTime startTime)
{
	//1. check realtime table
	double res=0.0;
	std::map<std::string,std::vector<sim_mob::LinkTravelTime*> >::iterator it =
			Link_realtime_travel_time_pool.find(id);
	if(it!=Link_realtime_travel_time_pool.end())
	{
		std::vector<sim_mob::LinkTravelTime*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::LinkTravelTime* l = e[i];
			if( l->start_time_dt.isBeforeEqual(startTime) && l->end_time_dt.isAfter(startTime) )
			{
				res = l->travel_time;
				return res;
			}
		}
	}
	//2. if no , check default
	it = Link_default_travel_time_pool.find(id);
	if(it!=Link_default_travel_time_pool.end())
	{
		std::vector<sim_mob::LinkTravelTime*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::LinkTravelTime* l = e[i];
			if( l->start_time_dt.isBeforeEqual(startTime) && l->end_time_dt.isAfter(startTime) )
			{
				res = l->travel_time;
				return res;
			}
		}
	}
	else
	{
		std::string str = "PathSetParam::getTravelTimeBySegId=> no travel time for segment " + id;
		sim_mob::Logger::log["path_set"]<<"error: "<<str<<std::endl;
	}
	return res;
}
sim_mob::WayPoint* sim_mob::PathSetParam::getWayPointBySeg(const sim_mob::RoadSegment* seg)
{
	std::map<const sim_mob::RoadSegment*,sim_mob::WayPoint*>::iterator it = wpPool.find(seg);
	if(it != wpPool.end() )
	{
		sim_mob::WayPoint* wp = (*it).second;
		return wp;
	}
	else
	{
		// create new wp
		sim_mob::WayPoint* wp_ = new sim_mob::WayPoint(seg);
		wpPool.insert(std::make_pair(seg,wp_));
		return wp_;
	}
}
sim_mob::Node* sim_mob::PathSetParam::getCachedNode(std::string id)
{
	std::map<std::string,sim_mob::Node*>::iterator it = nodePool.find(id);
	if(it != nodePool.end())
	{
		sim_mob::Node* node = (*it).second;
		return node;
	}
	return NULL;
}
void sim_mob::PathSetParam::initParameters()
{
	bTTVOT = -0.01373;//-0.0108879;
	bCommonFactor = 1.0;
	bLength = 0.001025;//0.0;
	bHighway = 0.00052;//0.0;
	bCost = 0.0;
	bSigInter = -0.13;//0.0;
	bLeftTurns = 0.0;
	bWork = 0.0;
	bLeisure = 0.0;
	highway_bias = 0.5;

	minTravelTimeParam = 0.879;
	minDistanceParam = 0.325;
	minSignalParam = 0.256;
	maxHighwayParam = 0.422;
}

uint32_t sim_mob::PathSetParam::getSize()
{
	if(!gotSize.check()){
		return 0;
	}
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
	sum += sizeof(double); //double highway_bias;
	sum += sizeof(double); //double minTravelTimeParam;
	sum += sizeof(double); //double minDistanceParam;
	sum += sizeof(double); //double minSignalParam;
	sum += sizeof(double); //double maxHighwayParam;

	//std::map<std::string,sim_mob::RoadSegment*> segPool;
	std::pair<std::string,sim_mob::RoadSegment*> segPool_pair;
	sim_mob::Logger::log["path_set"].prof("segPool_size", false).addUp(segPool.size());
	BOOST_FOREACH(segPool_pair,segPool)
	{
		sum += segPool_pair.first.length();
	}
	sum += sizeof(sim_mob::RoadSegment*) * segPool.size();

//		std::map<const sim_mob::RoadSegment*,sim_mob::WayPoint*> wpPool;//unused for now

	//std::map<std::string,sim_mob::Node*> nodePool;
	std::pair<std::string,sim_mob::Node*> nodePool_pair;
	std::cout << "nodePool.size() " << nodePool.size() << std::endl;
	sim_mob::Logger::log["path_set"].prof("nodePool_size", false).addUp(nodePool.size());
	BOOST_FOREACH(segPool_pair,segPool)
	{
		sum += nodePool_pair.first.length();
	}
	sum += sizeof(sim_mob::Node*) * nodePool.size();

//		const std::vector<sim_mob::MultiNode*>  &multiNodesPool;
	sim_mob::Logger::log["path_set"].prof("MnodePool_size", false).addUp(multiNodesPool.size());
	sum += sizeof(sim_mob::MultiNode*) * multiNodesPool.size();

//		const std::set<sim_mob::UniNode*> & uniNodesPool;
	sim_mob::Logger::log["path_set"].prof("UnodePool_size", false).addUp(uniNodesPool.size());
	sum += sizeof(sim_mob::UniNode*) * uniNodesPool.size();

//		std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> > ERP_Surcharge_pool;
	std::pair<std::string,std::vector<sim_mob::ERP_Surcharge*> > ERP_Surcharge_pool_pair;
	sim_mob::Logger::log["path_set"].prof("ERP_Surcharge_pool_size", false).addUp(ERP_Surcharge_pool.size());
	BOOST_FOREACH(ERP_Surcharge_pool_pair,ERP_Surcharge_pool)
	{
		sum += ERP_Surcharge_pool_pair.first.length();
		sum += sizeof(sim_mob::ERP_Surcharge*) * ERP_Surcharge_pool_pair.second.size();
	}

//		std::map<std::string,sim_mob::ERP_Gantry_Zone*> ERP_Gantry_Zone_pool;
	std::pair<std::string,sim_mob::ERP_Gantry_Zone*> ERP_Gantry_Zone_pool_pair;
	sim_mob::Logger::log["path_set"].prof("ERP_Gantry_Zone_pool_size", false).addUp(ERP_Gantry_Zone_pool.size());
	BOOST_FOREACH(ERP_Gantry_Zone_pool_pair,ERP_Gantry_Zone_pool)
	{
		sum += ERP_Gantry_Zone_pool_pair.first.length();
	}
	sum += sizeof(sim_mob::ERP_Gantry_Zone*) * ERP_Gantry_Zone_pool.size();

//		std::map<std::string,sim_mob::ERP_Section*> ERP_Section_pool;
	std::pair<std::string,sim_mob::ERP_Section*> ERP_Section_pair;
	sim_mob::Logger::log["path_set"].prof("ERP_Section_size", false).addUp(ERP_Section_pool.size());
	BOOST_FOREACH(ERP_Section_pair,ERP_Section_pool)
	{
		sum += ERP_Section_pair.first.length();
	}
	sum += sizeof(sim_mob::ERP_Section*) * ERP_Section_pool.size();

//		std::map<std::string,std::vector<sim_mob::LinkTravelTime*> > Link_default_travel_time_pool;
	std::pair<std::string,std::vector<sim_mob::LinkTravelTime*> > Link_default_travel_time_pool_pair;
	sim_mob::Logger::log["path_set"].prof("Link_default_travel_time_pool_size", false).addUp(Link_default_travel_time_pool.size());
	BOOST_FOREACH(Link_default_travel_time_pool_pair,Link_default_travel_time_pool)
	{
		sum += Link_default_travel_time_pool_pair.first.length();
		sum += sizeof(sim_mob::LinkTravelTime*) * Link_default_travel_time_pool_pair.second.size();
	}


//		std::map<std::string,std::vector<sim_mob::LinkTravelTime*> > Link_realtime_travel_time_pool;
	std::pair<std::string,std::vector<sim_mob::LinkTravelTime*> > Link_realtime_travel_time_pool_pair;
	sim_mob::Logger::log["path_set"].prof("Link_realtime_travel_time_pool_size", false).addUp(Link_realtime_travel_time_pool.size());
	BOOST_FOREACH(Link_realtime_travel_time_pool_pair,Link_realtime_travel_time_pool)
	{
		sum += Link_realtime_travel_time_pool_pair.first.length();
		sum += sizeof(sim_mob::LinkTravelTime*) * Link_realtime_travel_time_pool_pair.second.size();
	}

//		const roadnetwork;
	sum += sizeof(sim_mob::RoadNetwork&);

//		std::string pathset_traveltime_realtime_table_name;
	sum += pathset_traveltime_realtime_table_name.length();
//		std::string pathset_traveltime_tmp_table_name;
	sum += pathset_traveltime_tmp_table_name.length();
	return sum;
}

sim_mob::RoadSegment* sim_mob::PathSetParam::getRoadSegmentByAimsunId(const std::string id)
{
	std::map<const std::string,sim_mob::RoadSegment*>::iterator it = segPool.find(id);
	if(it != segPool.end())
	{
		sim_mob::RoadSegment* seg = (*it).second;
		return seg;
	}
	return NULL;
}

sim_mob::PathSetParam::PathSetParam() :
		roadNetwork(ConfigManager::GetInstance().FullConfig().getNetwork()),
		multiNodesPool(ConfigManager::GetInstance().FullConfig().getNetwork().getNodes()), uniNodesPool(ConfigManager::GetInstance().FullConfig().getNetwork().getUniNodes())
{
	initParameters();
	for (std::vector<sim_mob::Link *>::const_iterator it =	ConfigManager::GetInstance().FullConfig().getNetwork().getLinks().begin(), it_end( ConfigManager::GetInstance().FullConfig().getNetwork().getLinks().end()); it != it_end; it++) {
		for (std::set<sim_mob::RoadSegment *>::iterator seg_it = (*it)->getUniqueSegments().begin(), it_end((*it)->getUniqueSegments().end()); seg_it != it_end; seg_it++) {
			if (!(*seg_it)->originalDB_ID.getLogItem().empty()) {
				string aimsun_id = (*seg_it)->originalDB_ID.getLogItem();
				string seg_id = getNumberFromAimsunId(aimsun_id);
				segPool.insert(std::make_pair(seg_id, *seg_it));
//				WayPoint *wp = new WayPoint(*seg_it);
//				wpPool.insert(std::make_pair(*seg_it, wp));
			}
		}
	}
	//we are still in constructor , so const refs like roadNetwork and multiNodesPool are not ready yet.
	BOOST_FOREACH(sim_mob::Node* n, ConfigManager::GetInstance().FullConfig().getNetwork().getNodes()){
		if (!n->originalDB_ID.getLogItem().empty()) {
			std::string t = n->originalDB_ID.getLogItem();
			std::string id = sim_mob::getNumberFromAimsunId(t);
			nodePool.insert(std::make_pair(id , n));
		}
	}

	BOOST_FOREACH(sim_mob::UniNode* n, ConfigManager::GetInstance().FullConfig().getNetwork().getUniNodes()){
		if (!n->originalDB_ID.getLogItem().empty()) {
			std::string t = n->originalDB_ID.getLogItem();
			std::string id = sim_mob::getNumberFromAimsunId(t);
			nodePool.insert(std::make_pair(id, n));
		}
	}

	sim_mob::Logger::log["path_set"] << "PathSetParam: nodes amount " <<
			ConfigManager::GetInstance().FullConfig().getNetwork().getNodes().size() +
			ConfigManager::GetInstance().FullConfig().getNetwork().getNodes().size() << std::endl;
	sim_mob::Logger::log["path_set"] << "PathSetParam: segments amount "	<<
			segPool.size() << "\n";
	sim_mob::Logger::log["path_set"].prof("PathSetParam_getDataFromDB_time").tick();
	getDataFromDB();
	sim_mob::Logger::log["path_set"].prof("PathSetParam_getDataFromDB_time").tick(true);
	sim_mob::Logger::log["path_set"].profileMsg("Time taken for PathSetParam::getDataFromDB ", sim_mob::Logger::log["path_set"].prof("PathSetParam_getDataFromDB_time").getAddUp());
	sim_mob::Logger::log["path_set"] << "Time taken for PathSetParam::getDataFromDB   " << sim_mob::Logger::log["path_set"].prof("PathSetParam_getDataFromDB_time").getAddUp() << "\n";
}
uint32_t sim_mob::PathSetManager::getSize(){
	if(!gotSize.check()){
		return 0;
	}
		uint32_t sum = 0;
		//first get the pathset param here
		sum += sim_mob::PathSetParam::getInstance()->getSize();

			sum += sizeof(StreetDirectory&);
			sum += sizeof(PathSetParam *);
			sum += sizeof(bool); // bool isUseCache;
			sum += sizeof(const sim_mob::RoadSegment*) * currIncidents.size(); // std::set<const sim_mob::RoadSegment*> currIncidents;
			sum += sizeof(boost::shared_mutex); // boost::shared_mutex mutexIncident;
			sum += sizeof(std::string &);  // const std::string &pathSetTableName;
			sum += sizeof(const std::string &); // const std::string &singlePathTableName;
			sum += sizeof(const std::string &); // const std::string &dbFunction;
			// static std::map<boost::thread::id, boost::shared_ptr<soci::session > > cnnRepo;
			sum += (sizeof(boost::thread::id)) + sizeof(boost::shared_ptr<soci::session >) * cnnRepo.size();
			sum += sizeof(sim_mob::batched::ThreadPool *); // sim_mob::batched::ThreadPool *threadpool_;

			sum += sizeof(boost::shared_mutex); // boost::shared_mutex cachedPathSetMutex;

			// boost::unordered_map<const std::string,sim_mob::PathSet > cachedPathSet;
//			unsigned n = cachedPathSet.bucket_count();
//			float m = cachedPathSet.max_load_factor();
//			  if (m > 1.0) {
//				  sum += n * m;
//			  }
//			  else {
//				  sum += n;
//			  }
			//map<const std::string,sim_mob::PathSet > cachedPathSet;
			for(std::map<const std::string,boost::shared_ptr<sim_mob::PathSet> >::iterator it = cachedPathSet.begin(); it !=cachedPathSet.end(); it++)
//			BOOST_FOREACH(cachedPathSet_pair,cachedPathSet)
			{
				sum += it->first.length();
				sum += it->second->getSize();
			}
			sum += scenarioName.length(); // std::string scenarioName;
			// std::map<std::string ,std::vector<WayPoint> > fromto_bestPath;
			sum += sizeof(WayPoint) * fromto_bestPath.size();
			for(std::map<std::string ,std::vector<WayPoint> >::iterator it = fromto_bestPath.begin(); it != fromto_bestPath.end(); sum += it->first.length(), it++);
			for(std::set<std::string>::iterator it = tempNoPath.begin(); it != tempNoPath.end(); sum += (*it).length(), it++); // std::set<std::string> tempNoPath;
			sum += sizeof(RPOD); // RPOD pathSegments;
			sum += csvFileName.length(); // std::string csvFileName;
			sum += sizeof(std::ofstream); // std::ofstream csvFile;
			sum += pathset_traveltime_realtime_table_name.length(); // std::string pathset_traveltime_realtime_table_name;
			sum += pathset_traveltime_tmp_table_name.length(); // std::string pathset_traveltime_tmp_table_name;
			sum += sizeof(sim_mob::K_ShortestPathImpl *); // sim_mob::K_ShortestPathImpl *kshortestImpl;
			sum += sizeof(double); // double bTTVOT;
			sum += sizeof(double); // double bCommonFactor;
			sum += sizeof(double); // double bLength;
			sum += sizeof(double); // double bHighway;
			sum += sizeof(double); // double bCost;
			sum += sizeof(double); // double bSigInter;
			sum += sizeof(double); // double bLeftTurns;
			sum += sizeof(double); // sum += sizeof(); // double bWork;
			sum += sizeof(double); // double bLeisure;
			sum += sizeof(double); // double highway_bias;
			sum += sizeof(double); // double minTravelTimeParam;
			sum += sizeof(double); // double minDistanceParam;
			sum += sizeof(double); // double minSignalParam;
			sum += sizeof(double);  // double maxHighwayParam;

			//std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > pathSetPool;
			std::pair<std::string,boost::shared_ptr<sim_mob::PathSet> > pair_pathSetPool;
			BOOST_FOREACH(pair_pathSetPool,pathSetPool)
			{
				sum += pair_pathSetPool.first.length();
			}
			sum += sizeof(boost::shared_ptr<sim_mob::PathSet>) * pathSetPool.size();

			//std::map<std::string,sim_mob::ERP_Section*> ERP_Section_pool;
			std::pair<std::string,sim_mob::ERP_Section* > pair_ERP_Section_pool;
			BOOST_FOREACH(pair_ERP_Section_pool,ERP_Section_pool)
			{
				sum += pair_ERP_Section_pool.first.length();
			}
			sum += sizeof(sim_mob::ERP_Section*) * ERP_Section_pool.size() ;

			//std::map<std::string, std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > > personPathSetPool;
			std::pair<std::string, std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > > personPathSetPool_pair;
			BOOST_FOREACH(personPathSetPool_pair, personPathSetPool){
				sum += personPathSetPool_pair.first.length();
				std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > &t = personPathSetPool_pair.second;
				std::pair<std::string,boost::shared_ptr<sim_mob::PathSet> > pair_;
				BOOST_FOREACH(pair_,t)
				{
					sum += pair_.first.length();
				}
				sum += sizeof(boost::shared_ptr<sim_mob::PathSet>) * t.size();
			}

//			std::map<const sim_mob::RoadSegment*,SinglePath*> seg_pathSetNull;
			sum += sizeof(sim_mob::RoadSegment*) * seg_pathSetNull.size() ;
			sum += sizeof(sim_mob::SinglePath*) * seg_pathSetNull.size() ;
			//std::map<std::string,SinglePath*> waypoint_singlepathPool;
			std::pair<std::string,SinglePath*> waypoint_singlepathPool_pair;
			BOOST_FOREACH(waypoint_singlepathPool_pair,waypoint_singlepathPool)
			{
				sum += waypoint_singlepathPool_pair.first.length();
			}
			sum +=sizeof(sim_mob::SinglePath*) * waypoint_singlepathPool.size();

			return sum;
	}

sim_mob::PathSetManager::PathSetManager():stdir(StreetDirectory::instance()),
		pathSetTableName(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().pathSetTableName),
		singlePathTableName(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().singlePathTableName),
		dbFunction(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().dbFunction)
{
//	sql = NULL;
//	psDbLoader=NULL;
	pathSetParam = PathSetParam::getInstance();
//	sim_mob::Logger::log["path_set"].prof("PathsetParam_size_bytes",false).addUp(pathSetParam->getSize());
	std::string dbStr(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
//	// 1.2 get all segs
	init();
	cnnRepo[boost::this_thread::get_id()].reset(new soci::session(soci::postgresql,dbStr));
	threadpool_ = new sim_mob::batched::ThreadPool(10);
}

void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message){

}

sim_mob::PathSetManager::~PathSetManager()
{
	if(threadpool_){
		delete threadpool_;
	}
	clearPools();
}

void sim_mob::PathSetManager::init()
{
	setCSVFileName();
	initParameters();
}

namespace {
int paths=0;
int free_cnt = 0;
}

void sim_mob::PathSetManager::clearPools()
{

	for(std::map<std::string,boost::shared_ptr<sim_mob::PathSet> >::iterator it = pathSetPool.begin();
			it!=pathSetPool.end();
			++it)
	{
		(*it).second.reset();
	}
	pathSetPool.clear();
	for(std::map<std::string,SinglePath*>::iterator it = waypoint_singlepathPool.begin();
			it!=waypoint_singlepathPool.end();
			++it)
	{
		SinglePath* sp = (*it).second;
		if(sp)
			delete sp;
	}
	waypoint_singlepathPool.clear();
	//delete cached pathsets
	std::pair<std::string,sim_mob::PathSet>pair;
	BOOST_FOREACH(pair, cachedPathSet){
		paths++;//debug
		BOOST_FOREACH(sim_mob::SinglePath *sp, pair.second.pathChoices)
		{
			if (sp){//debug
				free_cnt ++;
			}
			safe_delete_item(sp);
		}
	}
	sim_mob::Logger::log["path_set"] << "PathSet manager freed " << free_cnt << "  from " << paths << " paths" << std::endl;
}

bool sim_mob::PathSetManager::generateAllPathSetWithTripChain2()
{
	const std::map<std::string, std::vector<sim_mob::TripChainItem*> > *tripChainPool =
			&ConfigManager::GetInstance().FullConfig().getTripChains();
	sim_mob::Logger::log["path_set"]<<"generateAllPathSetWithTripChain: trip chain pool size "<<  tripChainPool->size()<<std::endl;
	int poolsize = tripChainPool->size();
	bool res=true;
	// 1. get from and to node
	std::map<std::string, std::vector<sim_mob::TripChainItem*> >::const_iterator it;
	int i=1;
	for(it = tripChainPool->begin();it!=tripChainPool->end();++it)
	{
		std::string personId = (*it).first;
		std::vector<sim_mob::TripChainItem*> tci = (*it).second;
		for(std::vector<sim_mob::TripChainItem*>::iterator it=tci.begin(); it!=tci.end(); ++it)
		{
			if((*it)->itemType == sim_mob::TripChainItem::IT_TRIP)
			{
				sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip*> ((*it));
				if(!trip)
				{
					throw std::runtime_error("generateAllPathSetWithTripChainPool: trip error");
				}
				const std::vector<sim_mob::SubTrip> subTripPool = trip->getSubTrips();
				for(int i=0; i<subTripPool.size(); ++i)
				{
					const sim_mob::SubTrip *st = &subTripPool.at(i);
					std::string subTripId = st->tripID;
					if(st->mode == "Car") //only driver need path set
					{
						std::vector<WayPoint> res =  generateBestPathChoice2(st);
					}
				}
			}
		}
		i++;
		sim_mob::Logger::log["path_set"]<<"generateAllPathSetWithTripChain: "<<i<<"/"<<poolsize<<" mem: "<<size()/1024/1024<<"mb"<<std::endl;
	}
	return res;
}


unsigned long sim_mob::PathSetManager::size()
{
	unsigned long size=0;
	//pathSetPool
	size += pathSetPool.size()*sizeof(std::string);
	size += pathSetPool.size()*sizeof(sim_mob::PathSet);
	//
	size += waypoint_singlepathPool.size()*sizeof(std::string);
	size += waypoint_singlepathPool.size()*sizeof(sim_mob::SinglePath);

	return size;
}
void sim_mob::PathSetManager::setCSVFileName()
{
	//get current working directory
	char the_path[1024];
	getcwd(the_path, 1023);
	printf("current dir: %s \n",the_path);
	std::string currentPath(the_path);
	std::string currentPathTmp = currentPath + "/tmp_"+pathSetParam->pathset_traveltime_tmp_table_name;
	std::string cmd = "mkdir -p "+currentPathTmp;
	if (system(cmd.c_str()) == 0) // create tmp folder ok
	{
		csvFileName = currentPathTmp+"/"+pathSetParam->pathset_traveltime_tmp_table_name + ".csv";
	}
	else
	{
		csvFileName = currentPath+"/"+pathSetParam->pathset_traveltime_tmp_table_name + ".csv";
	}
	struct timeval tv;
	gettimeofday(&tv, NULL);
	csvFileName += boost::lexical_cast<string>(pthread_self()) +"_"+ boost::lexical_cast<string>(tv.tv_usec);
	sim_mob::Logger::log["path_set"]<<"csvFileName: "<<csvFileName<<std::endl;
	csvFile.open(csvFileName.c_str());
}

bool sim_mob::PathSetManager::insertTravelTime2TmpTable(sim_mob::LinkTravelTime& data)
{
	bool res=false;
	if(ConfigManager::GetInstance().FullConfig().PathSetMode()){
	 // get pointer to associated buffer object
	  std::filebuf* pbuf = csvFile.rdbuf();

	  // get file size using buffer's members
	  std::size_t size = pbuf->pubseekoff (0,csvFile.end,csvFile.in);
	  if(size>50000000)//50mb
	  {
		  csvFile.close();
		  sim_mob::aimsun::Loader::insertCSV2TableST(*getSession(),
		  			pathset_traveltime_tmp_table_name,csvFileName);
		  csvFile.open(csvFileName.c_str(),std::ios::in | std::ios::trunc);
	  }
		csvFile<<data.link_id<<";"<<data.start_time<<";"<<data.end_time<<";"<<data.travel_time<<std::endl;
	}
	return res;
}

bool sim_mob::PathSetManager::copyTravelTimeDataFromTmp2RealtimeTable()
{
	//0. copy csv to travel time table
	csvFile.close();
	sim_mob::Logger::log["path_set"]<<"table name: "<<pathset_traveltime_tmp_table_name<<std::endl;
	sim_mob::aimsun::Loader::insertCSV2Table(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
			pathset_traveltime_tmp_table_name,csvFileName);
	//1. truncate realtime table
	bool res=false;
	res = sim_mob::aimsun::Loader::truncateTable(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
			pathset_traveltime_realtime_table_name);
	if(!res)
		return res;
	//2. insert into "max_link_realtime_travel_time" (select * from "link_default_travel_time");
	std::string str = "insert into " + pathset_traveltime_realtime_table_name +
			"(select * from " + pathset_traveltime_tmp_table_name +")";
	res = sim_mob::aimsun::Loader::excuString(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),str);

	return res;
}

void  sim_mob::PathSetManager::generatePathSetByTrip(std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > &subTripId_pathSet,sim_mob::Trip *trip)
{
	const std::vector<sim_mob::SubTrip> subTripPool = trip->getSubTrips();
	for(int i=0; i<subTripPool.size(); ++i)
	{
		const sim_mob::SubTrip *st = &subTripPool.at(i);
		std::string subTripId = st->tripID;
		if(st->mode == "Car") //only driver need path set
		{
			const sim_mob::Node* fromNode = st->fromLocation.node_;
			const sim_mob::Node* toNode = st->toLocation.node_;
			boost::shared_ptr<sim_mob::PathSet>ps = generatePathSetByFromToNodes(fromNode,toNode,st);
			subTripId_pathSet.insert(std::make_pair(subTripId,ps));
		}
	}
}

void sim_mob::PathSetManager::insertFromTo_BestPath_Pool(std::string& id ,vector<WayPoint>& values)
{
	if(fromto_bestPath.size()>10000)
	{
		std::map<std::string ,std::vector<WayPoint> >::iterator it = fromto_bestPath.begin();
		fromto_bestPath.erase(it);
	}
	fromto_bestPath.insert(std::make_pair(id,values));
}

//todo: replacing bool and std::vector<WayPoint>& can save a copy
bool sim_mob::PathSetManager::getCachedBestPath(std::string id, std::vector<WayPoint> &value)
{
	std::map<std::string ,std::vector<WayPoint> >::iterator it = fromto_bestPath.find(id);
	if(it == fromto_bestPath.end())
	{
		return false;
	}
	value = (*it).second;
	return true;
}

void sim_mob::PathSetManager::cacheODbySegment(const sim_mob::Person* per, const sim_mob::SubTrip * subTrip, std::vector<WayPoint> & wps){
	BOOST_FOREACH(WayPoint wp, wps){
		pathSegments.insert(std::make_pair(wp.roadSegment_, personOD(per, subTrip)));
	}
}

const std::pair <RPOD::const_iterator,RPOD::const_iterator > sim_mob::PathSetManager::getODbySegment(const sim_mob::RoadSegment* segment) const{
	sim_mob::Logger::log["path_set"] << "pathSegments cache size =" <<  pathSegments.size() << std::endl;
	const std::pair <RPOD::const_iterator,RPOD::const_iterator > range = pathSegments.equal_range(segment);
	return range;
}


std::set<const sim_mob::RoadSegment*> &sim_mob::PathSetManager::getIncidents(){
	boost::unique_lock<boost::shared_mutex> lock(mutexIncident);
	return currIncidents;
}

void sim_mob::PathSetManager::inserIncidentList(const sim_mob::RoadSegment* rs) {
	boost::unique_lock<boost::shared_mutex> lock(mutexIncident);
	currIncidents.insert(rs);
}

const boost::shared_ptr<soci::session> & sim_mob::PathSetManager::getSession(){
	std::map<boost::thread::id, boost::shared_ptr<soci::session> >::iterator it = cnnRepo.find(boost::this_thread::get_id());
	if(it == cnnRepo.end())
	{
//		std::stringstream ss;
//		ss << "error finding the right connection to the database. cnnRepo.size(): " << cnnRepo.size() << "\n";
//		throw std::runtime_error(ss.str());
		std::string dbStr(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
		cnnRepo[boost::this_thread::get_id()].reset(new soci::session(soci::postgresql,dbStr));
		it = cnnRepo.find(boost::this_thread::get_id());
	}
	return it->second;
}


bool sim_mob::PathSetManager::cachePathSet(boost::shared_ptr<sim_mob::PathSet>&ps){
	Logger::log["path_set"] << "caching [" << ps->id << "]\n";
	//test
//	return false;
	//first step caching policy:
	// if cache size excedded 250 (upper threshold), reduce the size to 200 (lowe threshold)
	if(cachedPathSet.size() > 2500)
	{
		Logger::log["path_set"] << "clearing some of the cached PathSets\n";
		int i = cachedPathSet.size() - 2000;
		std::map<std::string, boost::shared_ptr<sim_mob::PathSet> >::iterator it(cachedPathSet.begin());
		for(; i >0 && it != cachedPathSet.end(); --i )
		{
			cachedPathSet.erase(it++);
		}
	}
	ps->bestWayPointpath.clear(); //to be calculated later
	{
		boost::unique_lock<boost::shared_mutex> lock(cachedPathSetMutex);
		bool res = cachedPathSet.insert(std::make_pair(ps->id,ps)).second;
		if(!res){
			Logger::log["path_set"] << "Failed to cache [" << ps->id << "]\n";
		}
		return res;
	}
}

void sim_mob::PathSetManager::clearSinglePaths(boost::shared_ptr<sim_mob::PathSet>&ps){
	Logger::log["path_set"] << "clearing " << ps->pathChoices.size() << " SinglePaths\n";
	BOOST_FOREACH(sim_mob::SinglePath* sp_, ps->pathChoices){
		if(sp_){
			safe_delete_item(sp_);
		}
	}
	ps->pathChoices.clear();
}

bool sim_mob::PathSetManager::findCachedPathSet(std::string  key, boost::shared_ptr<sim_mob::PathSet> &value){
//	//test
//	return false;
	std::map<std::string, boost::shared_ptr<sim_mob::PathSet> >::iterator it ;
	{
		boost::unique_lock<boost::shared_mutex> lock(cachedPathSetMutex);
		it = cachedPathSet.find(key);
		if (it == cachedPathSet.end()) {
			//debug
			std::stringstream out("");
			out << "Failed finding [" << key << "] in" << cachedPathSet.size() << " entries\n" ;
			std::pair<std::string, boost::shared_ptr<sim_mob::PathSet> >pair_;
			BOOST_FOREACH(pair_,cachedPathSet){
				out << pair_.first << ",";
			}
			Logger::log["path_set"] << out.str() << "\n";
			return false;
		}
		value = it->second;
	}
	return true;
}

void sim_mob::printWPpath(const std::vector<WayPoint> &wps , const sim_mob::Node* startingNode ){
	std::ostringstream out("wp path--");
	if(startingNode){
		out << startingNode->getID() << ":";
	}
	BOOST_FOREACH(WayPoint wp, wps){
		out << wp.roadSegment_->getSegmentAimsunId() << ",";
	}
	out << std::endl;

	sim_mob::Logger::log["path_set"] << out.str();
}

vector<WayPoint> sim_mob::PathSetManager::getPathByPerson(const sim_mob::Person* per,const sim_mob::SubTrip &subTrip)
{
	// get person id and current subtrip id
	std::string personId = per->getDatabaseId();
	std::vector<sim_mob::SubTrip>::const_iterator currSubTripIt = per->currSubTrip;
	std::string subTripId = subTrip.tripID;
	//todo. change the subtrip signature from pointer to referencer
	sim_mob::Logger::log["path_set"] << "+++++++++++++++++++++++++" << std::endl;
	vector<WayPoint> res;
	generateBestPathChoiceMT(res, &subTrip);
	sim_mob::Logger::log["path_set"] << "Path chosen for this person: " << std::endl;
	if(!res.empty())
	{
		printWPpath(res);
	}
	else{
		sim_mob::Logger::log["path_set"] << "NO PATH" << std::endl;
	}
//	cacheODbySegment(per, &subTrip, res);
	sim_mob::Logger::log["path_set"] << "===========================" << std::endl;
	return res;
}

//Operations:
//check the cache
//if not found in cache, check DB
//if not found in DB, generate all 4 types of path
//choose the best path using utility function
bool sim_mob::PathSetManager::generateBestPathChoiceMT(std::vector<sim_mob::WayPoint> &res,const sim_mob::SubTrip* st,
		const std::set<const sim_mob::RoadSegment*> & exclude_seg_ , bool isUseCache)
{
	res.clear();
	if(st->mode != "Car") //only driver need path set
	{

		return false;
	}
	//combine the excluded segments
	std::set<const sim_mob::RoadSegment*> excludedSegs(exclude_seg_);
	if(!currIncidents.empty())
	{
		excludedSegs.insert(currIncidents.begin(), currIncidents.end());
	}

	const sim_mob::Node* fromNode = st->fromLocation.node_;
	const sim_mob::Node* toNode = st->toLocation.node_;
	if(toNode == fromNode){
		sim_mob::Logger::log["path_set"] << "same OD objects discarded:" << toNode->getID() << "\n" ;
		return false;
	}
	if(toNode->getID() == fromNode->getID()){
		sim_mob::Logger::log["path_set"] << "Error: same OD id from different objects discarded:" << toNode->getID() << "\n" ;
		return false;
	}
	std::stringstream out("");
	std::string idStrTo = toNode->originalDB_ID.getLogItem();
	std::string idStrFrom = fromNode->originalDB_ID.getLogItem();
	out << getNumberFromAimsunId(idStrFrom) << "," << getNumberFromAimsunId(idStrTo);
	std::string fromToID(out.str());
	sim_mob::Logger::log["path_set"] << "[" << boost::this_thread::get_id() << "]searching for OD[" << fromToID << "]\n" ;
	boost::shared_ptr<sim_mob::PathSet> ps_;
	//check cache
	sim_mob::Logger::log["path_set"].prof("find_cache_time").tick();
	sim_mob::Logger::log["ODs"] << fromToID << "\n";
	if(isUseCache && findCachedPathSet(fromToID,ps_))
	{
		sim_mob::Logger::log["path_set"].prof("find_cache_time").tick(true);
		sim_mob::Logger::log["path_set"] << "Cache Hit" << std::endl;
		sim_mob::Logger::log["path_set"].prof("utility_cache").tick();
		bool r = getBestPathChoiceFromPathSet(ps_);
		sim_mob::Logger::log["path_set"] << "getBestPathChoiceFromPathSet returned best path of size : " << ps_->bestWayPointpath.size() << "\n";
		sim_mob::Logger::log["path_set"].prof("utility_cache").tick(true);
		if(r)
		{
			res = boost::move(ps_->bestWayPointpath);
			sim_mob::Logger::log["path_set"] << "returning a path " << res.size() << "\n";
			return true;

		}
		else{
				sim_mob::Logger::log["path_set"] << "UNUSED cache hit" << std::endl;
		}
	}
	else{
		sim_mob::Logger::log["path_set"].prof("find_cache_time").tick(true);
		sim_mob::Logger::log["path_set"] << "Cache miss" << std::endl;
	}
	//check db
	bool hasPSinDB = false;
	ps_.reset(new sim_mob::PathSet());
	ps_->subTrip = st;
	ps_->id = fromToID;
	std::map<std::string,sim_mob::SinglePath*> id_sp;
	bool hasSPinDB = sim_mob::aimsun::Loader::LoadSinglePathDBwithIdST(
							*getSession(),
							ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
							id_sp,
//									pathSetID,
							ps_->id,
							ps_->pathChoices, dbFunction, singlePathTableName);
	sim_mob::Logger::log["path_set"]<< "hasSPinDB:" << hasSPinDB << "\n" ;
	if(hasSPinDB)
	{
		sim_mob::Logger::log["path_set"] << "DB hit for " << ps_->id << "\n";
		bool r = false;
//				std::map<std::string,sim_mob::SinglePath*>::iterator it = id_sp.find(ps_->singlepath_id);
		ps_->oriPath = 0;
		BOOST_FOREACH(sim_mob::SinglePath* sp, ps_->pathChoices)
		{
			if(sp && sp->isShortestPath){
				ps_->oriPath = sp;
				break;
			}
		}
//				if(it!=id_sp.end())
//				{
//					ps_->oriPath = id_sp[ps_->singlepath_id]; //todo ps_->oriPath = (*it)->second
//				}
//				else
		if(ps_->oriPath == 0)
		{
			std::string str = "Warning => SP: oriPath(shortest path) for "  + ps_->id + " not valid anymore\n";
			sim_mob::Logger::log["path_set"]<< str ;
		}
		sim_mob::Logger::log["path_set"].prof("utility_db").tick();
		r = getBestPathChoiceFromPathSet(ps_, excludedSegs);
//		sim_mob::Logger::log["path_set"] << "getBestPathChoiceFromPathSet returned best path of size : " << ps_->bestWayPointpath.size() << "\n";

		sim_mob::Logger::log["path_set"].prof("utility_db").tick(true);
		if(r)
		{
			res = boost::move(ps_->bestWayPointpath);
//			res = ps_->bestWayPointpath;
			//cache
			if(isUseCache){
				r = cachePathSet(ps_);
				if(r)
				{
					sim_mob::Logger::log["path_set"] << "------------------\n";
					uint32_t t = ps_->getSize();
					sim_mob::Logger::log["path_set"].prof("cached_pathset_bytes",false).addUp(t);
					sim_mob::Logger::log["path_set"].prof("cached_pathset_size",false).addUp(1);
					sim_mob::Logger::log["path_set"] << "------------------\n";
				}
			}
			else{
				clearSinglePaths(ps_);
			}
			//test
//			clearSinglePaths(ps_);
			sim_mob::Logger::log["path_set"] << "returning a path " << res.size() << "\n";
			return true;
		}
		else
		{
				sim_mob::Logger::log["path_set"] << "UNUSED DB hit\n";
		}
	}// hasSPinDB
	else
	{
		sim_mob::Logger::log["path_set"] << "DB Miss for " << fromToID << " in SinglePath, Pathset says otherwise!\n";
	}

	if(!hasSPinDB/*hasPSinDB*/)
	{
		sim_mob::Logger::log["path_set"]<<"generate All PathChoices for "<<fromToID << "\n" ;
		// 1. generate shortest path with all segs
		// 1.2 get all segs
		// 1.3 generate shortest path with full segs
		ps_.reset(new PathSet(fromNode,toNode));
		ps_->id = fromToID;
//		ps_->from_node_id = fromNode->originalDB_ID.getLogItem();
//		ps_->to_node_id = toNode->originalDB_ID.getLogItem();
		std:string temp = fromNode->originalDB_ID.getLogItem();
		ps_->from_node_id = sim_mob::getNumberFromAimsunId(temp);
		temp = toNode->originalDB_ID.getLogItem();
		ps_->to_node_id = sim_mob::getNumberFromAimsunId(temp);
		ps_->scenario = scenarioName;
		ps_->subTrip = st;
		ps_->psMgr = this;

		if(!generateAllPathChoicesMT(ps_))
		{
			return false;
		}
		sim_mob::Logger::log["path_set"]<<"generate All Done for "<<fromToID << "\n" ;
		sim_mob::generatePathSizeForPathSet2(ps_);
		//
		// save pathset to loacl container
		sim_mob::Logger::log["path_set"].prof("utility_db").tick();
		bool r = getBestPathChoiceFromPathSet(ps_, excludedSegs);
		sim_mob::Logger::log["path_set"] << "getBestPathChoiceFromPathSet returned best path of size : " << ps_->bestWayPointpath.size() << "\n";

		sim_mob::Logger::log["path_set"].prof("utility_db").tick(true);
		// generate all node to current end node
//				generatePathset2AllNode(toNode,st);
		if(r)
		{
			res = boost::move(ps_->bestWayPointpath);
			//cache
			if(isUseCache){
				r = cachePathSet(ps_);
				if(r)
				{
					sim_mob::Logger::log["path_set"] << "------------------\n";
					uint32_t t = ps_->getSize();
					sim_mob::Logger::log["path_set"].prof("cached_pathset_bytes",false).addUp(t);
					sim_mob::Logger::log["path_set"].prof("cached_pathset_size",false).addUp(1);
					sim_mob::Logger::log["path_set"] << "------------------\n";
				}
				else
				{
					sim_mob::Logger::log["path_set"] << ps_->id << " not cached, apparently, already in cache.\n";
				}
			}
			else{
				clearSinglePaths(ps_);
			}
			//test...
			//store in into the database
			std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > tmp;
			tmp.insert(std::make_pair(fromToID,ps_));
			pathSetParam->storePathSet(*getSession(),tmp,pathSetTableName);
			pathSetParam->storeSinglePath(*getSession(),ps_->pathChoices,singlePathTableName);
			//test
//			clearSinglePaths(ps_);
			sim_mob::Logger::log["path_set"] << "returning a path " << res.size() << "\n";
			return true;
		}
		else
		{
			sim_mob::Logger::log["path_set"] << "No best path, even after regenerating pathset " << std::endl;
		}
	}
	else
	{
		sim_mob::Logger::log["path_set"] << "Didn't (re)generate pathset(hasPSinDB:"<< hasPSinDB << ")" << std::endl;
	}

	return false;
}


bool sim_mob::PathSetManager::generateAllPathChoicesMT(boost::shared_ptr<sim_mob::PathSet> &ps, const std::set<const sim_mob::RoadSegment*> & excludedSegs)
{

	/**
	 * step-1: find the shortest path. if not found: create an entry in the "PathSet" table and return(without adding any entry into SinglePath table)
	 * step-2: from the Singlepath's waypoints collection, compute the "travel cost" and "travel time"
	 * step-3: SHORTEST DISTANCE LINK ELIMINATION
	 * step-4: shortest travel time link elimination
	 * step-5: TRAVEL TIME HIGHWAY BIAS
	 * step-6: Random Pertubation
	 * step-7: Some caching/bookkeeping
	 */
	std::set<std::string> duplicateChecker;
	sim_mob::SinglePath *s = generateSinglePathByFromToNodes3(ps->fromNode,ps->toNode,duplicateChecker,excludedSegs);
	if(!s)
	{
		// no path
		if(tempNoPath.find(ps->id) == tempNoPath.end())
		{
			ps->has_path = -1;
			ps->isNeedSave2DB = true;
			std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > tmp;
			tmp.insert(std::make_pair(ps->id,ps));
			std::string cnn(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
			sim_mob::aimsun::Loader::SaveOnePathSetData(cnn,tmp, pathSetTableName);
			tempNoPath.insert(ps->id);
		}
		return false;
	}

	//	// 1.31 check path pool
		// 1.4 create PathSet object
	ps->has_path = 1;
	ps->isNeedSave2DB = true;
	ps->oriPath = s;
	std::string fromToID(getFromToString(ps->fromNode, ps->toNode));
	ps->id = fromToID;
	ps->singlepath_id = s->id;
	s->pathset_id = ps->id;
	s->pathSet = ps;
	s->travel_cost = sim_mob::getTravelCost2(s);
	s->travle_time = getTravelTime(s);

	// SHORTEST DISTANCE LINK ELIMINATION
	//declare the profiler  but dont start profiling. it will just accumulate the elapsed time of the profilers who are associated with the workers
	sim_mob::Link *l = NULL;
	std::vector<PathSetWorkerThread*> workPool;
	A_StarShortestPathImpl * impl = (A_StarShortestPathImpl*)stdir.getDistanceImpl();
	StreetDirectory::VertexDesc from = impl->DrivingVertex(*ps->fromNode);
	StreetDirectory::VertexDesc to = impl->DrivingVertex(*ps->toNode);
	StreetDirectory::Vertex* fromV = &from.source;
	StreetDirectory::Vertex* toV = &to.sink;
//	for(int i=0;i<ps->oriPath->shortestWayPointpath.size();++i)
//	{
//		WayPoint *w = ps->oriPath->shortestWayPointpath[i];
//		if (w->type_ == WayPoint::ROAD_SEGMENT && l != w->roadSegment_->getLink()) {
//			const sim_mob::RoadSegment* seg = w->roadSegment_;
//			PathSetWorkerThread * work = new PathSetWorkerThread();
//			//introducing the profiling time accumulator
//			//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
//			work->graph = &impl->drivingMap_;
//			work->segmentLookup = &impl->drivingSegmentLookup_;
//			work->fromVertex = fromV;
//			work->toVertex = toV;
//			work->fromNode = ps->fromNode;
//			work->toNode = ps->toNode;
//			work->excludeSeg = seg;
//			work->s->clear();
//			work->ps = ps;
//			std::stringstream out("");
//			out << "sdle," << i << "," << ps->fromNode->getID() << "," << ps->toNode->getID() ;
//			work->dbgStr = out.str();
//			threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
//			workPool.push_back(work);
//		} //ROAD_SEGMENT
//	}
//
//	sim_mob::Logger::log["path_set"] << "waiting for SHORTEST DISTANCE LINK ELIMINATION" << std::endl;
//	threadpool_->wait();
//	//kep your own ending time
//
//
//	// SHORTEST TRAVEL TIME LINK ELIMINATION
//	l=NULL;
	A_StarShortestTravelTimePathImpl * sttpImpl = (A_StarShortestTravelTimePathImpl*)stdir.getTravelTimeImpl();
//	from = sttpImpl->DrivingVertexNormalTime(*ps->fromNode);
//	to = sttpImpl->DrivingVertexNormalTime(*ps->toNode);
//	fromV = &from.source;file:///home/fm-simmobility/vahid/simmobility/dev/Basic/tempIncident/private/simrun_basic-1.xml
//	toV = &to.sink;
//	SinglePath *sinPathTravelTimeDefault = generateShortestTravelTimePath(ps->fromNode,ps->toNode,duplicateChecker,sim_mob::Default);
//	if(sinPathTravelTimeDefault)
//	{
//		for(int i=0;i<sinPathTravelTimeDefault->shortestWayPointpath.size();++i)
//		{
//			WayPoint *w = sinPathTravelTimeDefault->shortestWayPointpath[i];
//			if (w->type_ == WayPoint::ROAD_SEGMENT && l != w->roadSegment_->getLink()) {
//				const sim_mob::RoadSegment* seg = w->roadSegment_;
//				PathSetWorkerThread *work = new PathSetWorkerThread();
//				//introducing the profiling time accumulator
//				//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
//				work->graph = &sttpImpl->drivingMap_Default;
//				work->segmentLookup = &sttpImpl->drivingSegmentLookup_Default_;
//				work->fromVertex = fromV;
//				work->toVertex = toV;
//				work->fromNode = ps->fromNode;
//				work->toNode = ps->toNode;
//				work->excludeSeg = seg;
//				work->s->clear();
//				work->ps = ps;
//				std::stringstream out("");
//				out << "ttle," << i << "," << ps->fromNode->getID() << "," << ps->toNode->getID() ;
//				work->dbgStr = out.str();
//				threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
//				workPool.push_back(work);
//			} //ROAD_SEGMENT
//		}//for
//	}//if sinPathTravelTimeDefault
//
//	sim_mob::Logger::log["path_set"] << "waiting for SHORTEST TRAVEL TIME LINK ELIMINATION" << std::endl;
//	threadpool_->wait();
//
//
//	// TRAVEL TIME HIGHWAY BIAS
//	//declare the profiler  but dont start profiling. it will just accumulate the elapsed time of the profilers who are associated with the workers
//	l=NULL;
//	SinglePath *sinPathHightwayBias = generateShortestTravelTimePath(ps->fromNode,ps->toNode,duplicateChecker,sim_mob::HighwayBias_Distance);
//	from = sttpImpl->DrivingVertexHighwayBiasDistance(*ps->fromNode);
//	to = sttpImpl->DrivingVertexHighwayBiasDistance(*ps->toNode);
//	fromV = &from.source;
//	toV = &to.sink;
//	if(sinPathHightwayBias)
//	{
//		for(int i=0;i<sinPathHightwayBias->shortestWayPointpath.size();++i)
//		{
//			WayPoint *w = sinPathHightwayBias->shortestWayPointpath[i];
//			if (w->type_ == WayPoint::ROAD_SEGMENT && l != w->roadSegment_->getLink()) {
//				const sim_mob::RoadSegment* seg = w->roadSegment_;
//				PathSetWorkerThread *work = new PathSetWorkerThread();
//				//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
//				//introducing the profiling time accumulator
//				work->graph = &sttpImpl->drivingMap_HighwayBias_Distance;
//				work->segmentLookup = &sttpImpl->drivingSegmentLookup_HighwayBias_Distance_;
//				work->fromVertex = fromV;
//				work->toVertex = toV;
//				work->fromNode = ps->fromNode;
//				work->toNode = ps->toNode;
//				work->excludeSeg = seg;
//				work->s->clear();
//				work->ps = ps;
//				std::stringstream out("");
//				out << "highway," << i << "," << ps->fromNode->getID() << "," << ps->toNode->getID() ;
//				work->dbgStr = out.str();
//				threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
//				workPool.push_back(work);
//			} //ROAD_SEGMENT
//		}//for
//	}//if sinPathTravelTimeDefault
//	sim_mob::Logger::log["path_set"] << "waiting for TRAVEL TIME HIGHWAY BIAS" << std::endl;
//	threadpool_->wait();
	// generate random path
	for(int i=0;i<20;++i)//todo flexible number
	{
		from = sttpImpl->DrivingVertexRandom(*ps->fromNode,i);
		to = sttpImpl->DrivingVertexRandom(*ps->toNode,i);
		fromV = &from.source;
		toV = &to.sink;
		PathSetWorkerThread *work = new PathSetWorkerThread();
		//introducing the profiling time accumulator
		//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
		work->graph = &sttpImpl->drivingMap_Random_pool[i];
		work->segmentLookup = &sttpImpl->drivingSegmentLookup_Random_pool[i];
		work->fromVertex = fromV;
		work->toVertex = toV;
		work->fromNode = ps->fromNode;
		work->toNode = ps->toNode;
		work->excludeSeg = NULL;
//		work->s = NULL;
		work->s->clear();
		work->ps = ps;
		std::stringstream out("");
		out << "random," << i << "," << ps->fromNode->getID() << "," << ps->toNode->getID() ;
		work->dbgStr = out.str();
		threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
		workPool.push_back(work);
	}
	//WAITING FOR THREAPOOL TO END HERE
	threadpool_->wait();
	//now that all the threads have concluded, get the total times
	//record
	//a.record the shortest path with all segments
	if(!ps->oriPath){
		std::string str = "path set " + ps->id + " has no shortest path\n" ;
		throw std::runtime_error(str);
	}

	if(!ps->oriPath->isShortestPath){
		std::string str = "path set " + ps->id + " is supposed to be the shortest path but it is not!\n" ;
		throw std::runtime_error(str);
	}
	ps->pathChoices.insert(ps->oriPath);
	BOOST_FOREACH(PathSetWorkerThread* p, workPool){
		if(p->hasPath){
			if(p->s->isShortestPath){
				std::string str = "Single path from pathset " + ps->id + " is not supposed to be marked as a shortest path but it is!\n" ;
				throw std::runtime_error(str);
			}
			ps->pathChoices.insert(p->s);
		}
	}

	return true;
}

vector<WayPoint> sim_mob::PathSetManager::generateBestPathChoice2(const sim_mob::SubTrip* st)
{
	vector<WayPoint> res;
	std::string subTripId = st->tripID;
	if(st->mode != "Car") //only driver need path set
	{
		return res;
	}
	//find the node id
	const sim_mob::Node* fromNode = st->fromLocation.node_;
	const sim_mob::Node* toNode = st->toLocation.node_;
	std::string fromToID(getFromToString(fromNode,toNode));
	std::string pathSetID=fromToID;
	//check cache to save a trouble if the path already exists
	vector<WayPoint> cachedResult;
	if(getCachedBestPath(fromToID,cachedResult))
	{
		return cachedResult;
	}
		//
		pathSetID = "'"+pathSetID+"'";
		boost::shared_ptr<PathSet> ps_;
#if 0
		bool hasPSinDB = sim_mob::aimsun::Loader::LoadOnePathSetDBwithId(
						ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
						ps_,pathSetID);
#else
		bool hasPSinDB = sim_mob::aimsun::Loader::LoadOnePathSetDBwithIdST(
						*getSession(),
						ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
						ps_,pathSetID, pathSetTableName);
#endif
		if(ps_->has_path == -1) //no path
		{
			return res;
		}
		if(hasPSinDB)
		{
			// init ps_
			if(!ps_->isInit)
			{
				ps_->subTrip = st;
				std::map<std::string,sim_mob::SinglePath*> id_sp;
#if 0
				bool hasSPinDB = sim_mob::aimsun::Loader::LoadSinglePathDBwithId2(
#else
				bool hasSPinDB = sim_mob::aimsun::Loader::LoadSinglePathDBwithIdST(
						*getSession(),
#endif
						ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
						id_sp,
						pathSetID,
						ps_->pathChoices,dbFunction, singlePathTableName);
				if(hasSPinDB)
				{
					std::map<std::string,sim_mob::SinglePath*>::iterator it = id_sp.find(ps_->singlepath_id);
					if(it!=id_sp.end())
					{
						ps_->oriPath = id_sp[ps_->singlepath_id];
						bool r = getBestPathChoiceFromPathSet(ps_);
						if(r)
						{
//							res = sim_mob::convertWaypointP2Wp(ps_->bestWayPointpath);
							res = ps_->bestWayPointpath;// copy better than constant twisting
							// delete ps,sp
							BOOST_FOREACH(sim_mob::SinglePath* sp_, ps_->pathChoices)
							{
								if(sp_){
									delete sp_;
								}
							}
							ps_->pathChoices.clear();
							insertFromTo_BestPath_Pool(fromToID,res);
							return res;
						}
						else
							return res;
					}
					else
					{
						std::string str = "gBestPC2: oriPath(shortest path) for "  + ps_->id + " not found in single path";
						sim_mob::Logger::log["path_set"]<<str<<std::endl;
						return res;
					}
				}// hasSPinDB
			}
		} // hasPSinDB
		else
		{
			sim_mob::Logger::log["path_set"]<<"gBestPC2: create data for "<<fromToID<<std::endl;
			// 1. generate shortest path with all segs
			// 1.2 get all segs
			// 1.3 generate shortest path with full segs
			std::set<std::string> duplicateChecker;
			sim_mob::SinglePath *s = generateSinglePathByFromToNodes3(fromNode,toNode,duplicateChecker);
			if(!s)
			{
				// no path
				ps_.reset(new PathSet(fromNode,toNode));
				ps_->has_path = -1;
				ps_->isNeedSave2DB = true;
				ps_->id = fromToID;
//				ps_->from_node_id = fromNode->originalDB_ID.getLogItem();
//				ps_->to_node_id = toNode->originalDB_ID.getLogItem();
				std:string temp = fromNode->originalDB_ID.getLogItem();
				ps_->from_node_id = sim_mob::getNumberFromAimsunId(temp);
				temp = toNode->originalDB_ID.getLogItem();
				ps_->to_node_id = sim_mob::getNumberFromAimsunId(temp);
				ps_->scenario = scenarioName;
				std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > tmp;
				tmp.insert(std::make_pair(fromToID,ps_));
				std::string cnn(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
				sim_mob::aimsun::Loader::SaveOnePathSetData(cnn,tmp, pathSetTableName);
				return res;
			}
			//	// 1.31 check path pool
				// 1.4 create PathSet object
			ps_.reset(new PathSet(fromNode,toNode));
			ps_->has_path = 1;
			ps_->subTrip = st;
			ps_->isNeedSave2DB = true;
			ps_->oriPath = s;
			ps_->id = fromToID;
			ps_->singlepath_id = s->id;
			s->pathset_id = ps_->id;
			s->pathSet = ps_;
			s->travel_cost = getTravelCost2(s);
			s->travle_time = getTravelTime(s);
			ps_->pathChoices.insert(s);
				// 2. exclude each seg in shortest path, then generate new shortest path
			generatePathesByLinkElimination(s->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode);
				// generate shortest travel time path (default,morning peak,evening peak, off time)
				generateTravelTimeSinglePathes(fromNode,toNode,duplicateChecker,ps_);

				// generate k-shortest paths
				std::vector< std::vector<sim_mob::WayPoint> > kshortestPaths = kshortestImpl->getKShortestPaths(fromNode,toNode,ps_,duplicateChecker);
//				ps_->from_node_id = fromNode->originalDB_ID.getLogItem();
//				ps_->to_node_id = toNode->originalDB_ID.getLogItem();
				std::string temp = fromNode->originalDB_ID.getLogItem();
				ps_->from_node_id = sim_mob::getNumberFromAimsunId(temp);
				temp = toNode->originalDB_ID.getLogItem();
				ps_->to_node_id = sim_mob::getNumberFromAimsunId(temp);
				ps_->scenario = scenarioName;
				// 3. store pathset
				sim_mob::generatePathSizeForPathSet2(ps_);
				std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > tmp;
				tmp.insert(std::make_pair(fromToID,ps_));
				std::string cnn(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
				sim_mob::aimsun::Loader::SaveOnePathSetData(cnn,tmp, pathSetTableName);
				//
				bool r = getBestPathChoiceFromPathSet(ps_);
				if(r)
				{
//					res = sim_mob::convertWaypointP2Wp(ps_->bestWayPointpath);
					res = ps_->bestWayPointpath;// copy better than constant twisting
					// delete ps,sp
					BOOST_FOREACH(sim_mob::SinglePath* sp_, ps_->pathChoices)
					{
						if(sp_){
							delete sp_;
						}
					}
					ps_->pathChoices.clear();
					return res;
				}
				else
					return res;
		}

	return res;
}

void sim_mob::PathSetManager::generatePathesByLinkElimination(std::vector<WayPoint>& path,
			std::set<std::string>& duplicateChecker,
			boost::shared_ptr<sim_mob::PathSet> &ps_,
			const sim_mob::Node* fromNode,
			const sim_mob::Node* toNode)
{
	for(int i=0;i<path.size();++i)
	{
		WayPoint &w = path[i];
		if (w.type_ == WayPoint::ROAD_SEGMENT) {
			std::set<const sim_mob::RoadSegment*> seg ;
			seg.insert(w.roadSegment_);
			SinglePath *sinPath = generateSinglePathByFromToNodes3(fromNode,toNode,duplicateChecker,seg);
			if(!sinPath)
			{
				continue;
			}
			sinPath->pathSet = ps_; // set parent
			sinPath->travel_cost = getTravelCost2(sinPath);
			sinPath->travle_time = getTravelTime(sinPath);
			sinPath->pathset_id = ps_->id;
			ps_->pathChoices.insert(sinPath);
		}
	}//end for
}
void sim_mob::PathSetManager::generatePathesByTravelTimeLinkElimination(std::vector<WayPoint>& path,
		std::set<std::string>& duplicateChecker,
				boost::shared_ptr<sim_mob::PathSet> &ps_,
				const sim_mob::Node* fromNode,
				const sim_mob::Node* toNode,
				sim_mob::TimeRange tr)
{
	for(int i=0;i<path.size();++i)
	{
		WayPoint &w = path[i];
		if (w.type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w.roadSegment_;
			SinglePath *sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,tr,seg);
			if(!sinPath)
			{
				continue;
			}
			sinPath->pathSet = ps_; // set parent
			sinPath->travel_cost = getTravelCost2(sinPath);
			sinPath->travle_time = getTravelTime(sinPath);
			sinPath->pathset_id = ps_->id;
//			storePath(sinPath);
			ps_->pathChoices.insert(sinPath);
		}
	}//end for
}
void sim_mob::PathSetManager::generateTravelTimeSinglePathes(const sim_mob::Node *fromNode,
		   const sim_mob::Node *toNode,
		   std::set<std::string>& duplicateChecker,boost::shared_ptr<sim_mob::PathSet> &ps_)
{
	SinglePath *sinPath_morningPeak = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::MorningPeak);
	if(sinPath_morningPeak)
	{
		sinPath_morningPeak->pathSet = ps_; // set parent
		sinPath_morningPeak->travel_cost = getTravelCost2(sinPath_morningPeak);
		sinPath_morningPeak->travle_time = getTravelTime(sinPath_morningPeak);
		sinPath_morningPeak->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath_morningPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_morningPeak->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::MorningPeak);
	}
	SinglePath *sinPath_eveningPeak = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::EveningPeak);
	if(sinPath_eveningPeak)
	{
		sinPath_eveningPeak->pathSet = ps_; // set parent
		sinPath_eveningPeak->travel_cost = getTravelCost2(sinPath_eveningPeak);
		sinPath_eveningPeak->travle_time = getTravelTime(sinPath_eveningPeak);
		sinPath_eveningPeak->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath_eveningPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_eveningPeak->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::EveningPeak);
	}
	SinglePath *sinPath_offPeak = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::OffPeak);
	if(sinPath_offPeak)
	{
		sinPath_offPeak->pathSet = ps_; // set parent
		sinPath_offPeak->travel_cost = getTravelCost2(sinPath_offPeak);
		sinPath_offPeak->travle_time = getTravelTime(sinPath_offPeak);
		sinPath_offPeak->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath_offPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_offPeak->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::OffPeak);
	}
	SinglePath *sinPath_default = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::Default);
	if(sinPath_default)
	{
		sinPath_default->pathSet = ps_; // set parent
		sinPath_default->travel_cost = getTravelCost2(sinPath_default);
		sinPath_default->travle_time = getTravelTime(sinPath_default);
		sinPath_default->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath_default);
		generatePathesByTravelTimeLinkElimination(sinPath_default->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::Default);
	}
	// generate high way bias path
	SinglePath *sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_Distance);
	if(sinPath)
	{
		sinPath->pathSet = ps_; // set parent
		sinPath->travel_cost = getTravelCost2(sinPath);
		sinPath->travle_time = getTravelTime(sinPath);
		sinPath->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_Distance);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_MorningPeak);
	if(sinPath)
	{
		sinPath->pathSet = ps_; // set parent
		sinPath->travel_cost = getTravelCost2(sinPath);
		sinPath->travle_time = getTravelTime(sinPath);
		sinPath->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_EveningPeak);
	if(sinPath)
	{
		sinPath->pathSet = ps_; // set parent
		sinPath->travel_cost = getTravelCost2(sinPath);
		sinPath->travle_time = getTravelTime(sinPath);
		sinPath->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_OffPeak);
	if(sinPath)
	{
		sinPath->pathSet = ps_; // set parent
		sinPath->travel_cost = getTravelCost2(sinPath);
		sinPath->travle_time = getTravelTime(sinPath);
		sinPath->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_Default);
	if(sinPath)
	{
		sinPath->pathSet = ps_; // set parent
		sinPath->travel_cost = getTravelCost2(sinPath);
		sinPath->travle_time = getTravelTime(sinPath);
		sinPath->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath);
		//
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_Default);
	}
	// generate random path
	for(int i=0;i<20;++i)
	{
		const sim_mob::RoadSegment *rs = NULL;
		sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::Random,rs,i);
		if(sinPath)
		{
			sinPath->pathSet = ps_; // set parent
			sinPath->travel_cost = getTravelCost2(sinPath);
			sinPath->travle_time = getTravelTime(sinPath);
			sinPath->pathset_id = ps_->id;
			ps_->pathChoices.insert(sinPath);
		}
	}

}
double sim_mob::PathSetManager::getUtilityBySinglePath(sim_mob::SinglePath* sp)
{
	double utility=0;
	if(!sp)
		return utility;
	// calculate utility
	//1.0
	//Obtain the travel time tt of the path.
	//Obtain value of time for the agent A: bTTlowVOT/bTTmedVOT/bTThiVOT.
	utility += sp->travle_time * bTTVOT;
	//2.0
	//Obtain the path size PS of the path.
	utility += sp->pathsize * bCommonFactor;
	//3.0
	//Obtain the travel distance l and the highway distance w of the path.
	utility += sp->length * bLength + sp->highWayDistance * bHighway;
	//4.0
	//Obtain the travel cost c of the path.
	utility += sp->travel_cost * bCost;
	//5.0
	//Obtain the number of signalized intersections s of the path.
	utility += sp->signal_number * bSigInter;
	//6.0
	//Obtain the number of right turns f of the path.
	utility += sp->right_turn_number * bLeftTurns;
	//7.0
	//min travel time param
	if(sp->isMinTravelTime == 1)
	{
		utility += minTravelTimeParam;
	}
	//8.0
	//min distance param
	if(sp->isMinDistance == 1)
	{
		utility += minDistanceParam;
	}
	//9.0
	//min signal param
	if(sp->isMinSignal == 1)
	{
		utility += minSignalParam;
	}
	//10.0
	//min highway param
	if(sp->isMaxHighWayUsage == 1)
	{
		utility += maxHighwayParam;
	}
	//Obtain the trip purpose.
	if(sp->purpose == sim_mob::work)
	{
		utility += sp->purpose * bWork;
	}
	else if(sp->purpose == sim_mob::leisure)
	{
		utility += sp->purpose * bLeisure;
	}

	return utility;
}


bool sim_mob::PathSetManager::getBestPathChoiceFromPathSet(boost::shared_ptr<sim_mob::PathSet> &ps, const std::set<const sim_mob::RoadSegment *> & excludedSegs)
{
	bool tempIsInclude = false;
	// path choice algorithm
	if(!ps->oriPath && excludedSegs.empty())//return upon null oriPath only if the condition is normal(excludedSegs is empty)
	{
		sim_mob::Logger::log["path_set"]<<"warning gBPCFromPS: ori path empty"<<std::endl;
		return false;
	}
	// step 1.1 : For each path i in the path choice:
	//1. set PathSet(O, D)
	//2. travle_time
	//3. utility
	//step 1.2 : accumulate the logsum
	double maxTravelTime = std::numeric_limits<double>::max();
	ps->logsum = 0.0;
	int temp = 0;
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(sp)
		{
			if(sp->shortestWayPointpath.empty())
			{
				std::string str = temp + " Singlepath empty";
				throw std::runtime_error (str);
			}
			sp->pathSet = ps;
			if (sp->includesRoadSegment(excludedSegs) ) {
				sp->travle_time = maxTravelTime;//some large value like infinity
			}
			else{
				sp->travle_time = getTravelTime(sp);
			}
			sp->utility = getUtilityBySinglePath(sp);
			ps->logsum += exp(sp->utility);
		}
		else
		{
			throw std::runtime_error ("Singlepath null");
		}
		temp++;
	}
	// step 2: find the best waypoint path :
		// calculate a probability using path's utility and pathset's logsum,
		// compare the resultwith a  random number to decide whether pick the current path as the best path or not
		//if not, just chose the shortest path as the best path
	double upperProb=0;
	// 2.1 Draw a random number X between 0.0 and 1.0 for agent A.
	double x = sim_mob::gen_random_float(0,1);
	// 2.2 For each path i in the path choice set PathSet(O, D):
	int i = -1;
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		i++;
		if(sp || sp->shortestWayPointpath.empty())
		{
			double prob = exp(sp->utility)/(ps->logsum);
			upperProb += prob;
			if (x <= upperProb)
			{
				// 2.3 agent A chooses path i from the path choice set.
				ps->bestWayPointpath = sp->shortestWayPointpath;
				sim_mob::Logger::log["path_set"] << "handed sp->shortestWayPointpath[size:" << sp->shortestWayPointpath.size() << "] to ps->bestWayPointpath[size:"<< ps->bestWayPointpath.size() << "]\n" ;
				return true;
			}
		}
		else
		{
			std::string str = i + " Invalid Singlepath";
			throw std::runtime_error (str);
		}

	}
	//the last step resorts to selecting and returning oripath.
	//oriPath is the shortest path ususally generatd from a graph with not exclusion(free flow)
	// there is a good chance the excluded segment is in it. in that case, you cannot return it as a valid path-vahid
	if((!ps->oriPath) || (ps->oriPath->includesRoadSegment(excludedSegs))){
		//currently, the set of excluded segments contain only 1 segment
		//so at least one of the paths resulted from link elimination should be selected by now.
		//if not, either throw an error, or return false(just like what I did now) or obtain path fom graph 'providing the exclusions'
		//throw std::runtime_error("Could not find a path NOT containing the excluded segments");
		sim_mob::Logger::log["path_set"] << "NO BEST PATH. oriPathEmpty, other paths dont seem to have anything as best path\n";
		return false;
	}
	// have to has a path
	sim_mob::Logger::log["path_set"] << "NO BEST PATH. resorted to shortest path\n" ;
	ps->bestWayPointpath = ps->oriPath->shortestWayPointpath;
	return true;
}
void sim_mob::PathSetManager::initParameters()
{
	bTTVOT = pathSetParam->bTTVOT;//-0.0108879;
	bCommonFactor = pathSetParam->bCommonFactor;
	bLength = pathSetParam->bLength;//0.0;
	bHighway = pathSetParam->bHighway;//0.0;
	bCost = pathSetParam->bCost;
	bSigInter = pathSetParam->bSigInter;//0.0;
	bLeftTurns = pathSetParam->bLeftTurns;
	bWork = pathSetParam->bWork;
	bLeisure = pathSetParam->bLeisure;
	highway_bias = pathSetParam->highway_bias;

	minTravelTimeParam = pathSetParam->minTravelTimeParam;
	minDistanceParam = pathSetParam->minDistanceParam;
	minSignalParam = pathSetParam->minSignalParam;
	maxHighwayParam = pathSetParam->maxHighwayParam;
}



sim_mob::SinglePath *  sim_mob::PathSetManager::generateSinglePathByFromToNodes3(
		   const sim_mob::Node *fromNode,
		   const sim_mob::Node *toNode,
		   std::set<std::string> & duplicatePath,
		   const std::set<const sim_mob::RoadSegment*> & excludedSegs)
{
	/**
	 * step-1: find the shortest driving path between the given OD
	 * step-2: turn the outputted waypoint container into a string to be used as an id
	 * step-3: create a new Single path object
	 * step-4: return the resulting singlepath object as well as add it to the container supplied through args
	 */
	sim_mob::SinglePath *s=NULL;
	std::vector<const sim_mob::RoadSegment*> blacklist;
	if(excludedSegs.size())
	{
		const sim_mob::RoadSegment* rs;
		BOOST_FOREACH(rs, excludedSegs){
			blacklist.push_back(rs);
		}
	}
	std::vector<WayPoint> wp = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*fromNode), stdir.DrivingVertex(*toNode),blacklist);
	if(wp.empty())
	{
		// no path debug message
		if(excludedSegs.size())
		{
			const sim_mob::RoadSegment* rs;
			BOOST_FOREACH(rs, excludedSegs){
			sim_mob::Logger::log["path_set"]<<"gSPByFTNodes3: no path for nodes["<<fromNode->originalDB_ID.getLogItem()<< "] and [" <<
				toNode->originalDB_ID.getLogItem()<< " and ex seg[" <<
				rs->originalDB_ID.getLogItem()<< "]" << std::endl;
			}
		}
		else
		{
			sim_mob::Logger::log["path_set"]<<"gSPByFTNodes3: no path for nodes["<<fromNode->originalDB_ID.getLogItem()<< "] and [" <<
							toNode->originalDB_ID.getLogItem()<< "]" << std::endl;
		}
		return s;
	}
	// make sp id
	std::string id = sim_mob::makeWaypointsetString(wp);
	if(!id.size()){
		sim_mob::Logger::log["path_set"] << "Error: Empty shortest path for OD:" <<  fromNode->getID() << "," << toNode->getID() << "\n" ;
	}
	// 1.31 check path pool
	std::set<std::string>::iterator it =  duplicatePath.find(id);
	// no stored path found, generate new one
	if(it==duplicatePath.end())
	{
		s = new SinglePath();
		// fill data
		s->isNeedSave2DB = true;
		s->init(wp);
		sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(s);
		s->fromNode = fromNode;
		s->toNode = toNode;
		s->pathSet.reset();
		s->length = sim_mob::generateSinglePathLengthPT(s->shortestWayPointpath);
		s->id = id;
		s->scenario = scenarioName;
		s->pathsize=0;
		s->isShortestPath = true;
		duplicatePath.insert(id);
	}
	else{
		sim_mob::Logger::log["path_set"]<<"gSPByFTNodes3:duplicate pathset discarded\n";
	}

	return s;
}
sim_mob::SinglePath* sim_mob::PathSetManager::generateShortestTravelTimePath(const sim_mob::Node *fromNode,
			   const sim_mob::Node *toNode,
			   std::set<std::string>& duplicateChecker,
			   sim_mob::TimeRange tr,
			   const sim_mob::RoadSegment* excludedSegs,int random_graph_idx)
{
	sim_mob::SinglePath *s=NULL;
		std::vector<const sim_mob::RoadSegment*> blacklist;
		if(excludedSegs)
		{
			blacklist.push_back(excludedSegs);
		}
		std::vector<WayPoint> wp = stdir.SearchShortestDrivingTimePath(stdir.DrivingTimeVertex(*fromNode,tr,random_graph_idx),
				stdir.DrivingTimeVertex(*toNode,tr,random_graph_idx),
				blacklist,
				tr,
				random_graph_idx);
		if(wp.size()==0)
		{
			// no path
			sim_mob::Logger::log["path_set"]<<"generateShortestTravelTimePath: no path for nodes"<<fromNode->originalDB_ID.getLogItem()<<
							toNode->originalDB_ID.getLogItem()<<std::endl;
			return s;
		}
		// make sp id
		std::string id = sim_mob::makeWaypointsetString(wp);
		// 1.31 check path pool
		std::set<std::string>::iterator it =  duplicateChecker.find(id);
		// no stored path found, generate new one
		if(it==duplicateChecker.end())
		{
			s = new SinglePath();
			// fill data
			s->isNeedSave2DB = true;
			s->init(wp);
			sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(s);
			s->fromNode = fromNode;
			s->toNode = toNode;
			s->pathSet.reset();
			s->length = sim_mob::generateSinglePathLengthPT(s->shortestWayPointpath);
			s->id = id;
			s->scenario = scenarioName;
			s->pathsize=0;
			duplicateChecker.insert(id);
		}
		else{
			sim_mob::Logger::log["path_set"]<<"generateShortestTravelTimePath:duplicate pathset discarded\n";
		}

		return s;
}


std::string sim_mob::getNumberFromAimsunId(std::string &aimsunid)
{
	//"aimsun-id":"69324",
	std::string number;
	boost::regex expr (".*\"aimsun-id\":\"([0-9]+)\".*$");
	boost::smatch matches;
	if (boost::regex_match(aimsunid, matches, expr))
	{
		number  = std::string(matches[1].first, matches[1].second);
	}
	else
	{
		Warn()<<"aimsun id not correct "+aimsunid<<std::endl;
	}

	return number;
}
std::vector<WayPoint*> sim_mob::convertWaypoint2Point(std::vector<WayPoint> wp)
{
	std::vector<WayPoint*> res;
	for(int i=0;i<wp.size();++i)
	{
		if (wp[i].type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = wp[i].roadSegment_;
			// find waypoint point from pool
			WayPoint *wp_ = sim_mob::PathSetParam::getInstance()->getWayPointBySeg(seg);
			res.push_back(wp_);
		}
	}
	return res;
}
std::vector<WayPoint> sim_mob::convertWaypointP2Wp(std::vector<WayPoint*> wp)
{
	std::vector<WayPoint> res;
	for(int i=0;i<wp.size();++i)
	{
		if (wp[i]->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = wp[i]->roadSegment_;
			// find waypoint point from pool
			WayPoint wp_ = WayPoint(seg);
			res.push_back(wp_);
		}
	}
	return res;
}


bool sim_mob::convertWaypointP2Wp_NOCOPY(std::vector<WayPoint*> wp, std::vector<WayPoint> &res)
{
	for(int i=0;i<wp.size();++i)
	{
		if (wp[i]->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = wp[i]->roadSegment_;
			// find waypoint point from pool
			WayPoint wp_ = WayPoint(seg);
			res.push_back(wp_);
		}
	}
	return true;
}

void sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(sim_mob::SinglePath *sp)
{
	if(sp->shortestWayPointpath.size()<2)
	{
		sp->right_turn_number=0;
		sp->signal_number=0;
		return ;
	}
	int res=0;
	int signalNumber=0;
//	std::set<const RoadSegment*>::iterator itt=sp->shortestSegPath.begin();
	std::vector<WayPoint>::iterator itt=sp->shortestWayPointpath.begin();
	++itt;
//	for(std::set<const RoadSegment*>::iterator it=sp->shortestSegPath.begin();it!=sp->shortestSegPath.end();++it)
	for(std::vector<WayPoint>::iterator it=sp->shortestWayPointpath.begin();it!=sp->shortestWayPointpath.end();++it)
	{
		const RoadSegment* currentSeg = it->roadSegment_;
		const RoadSegment* targetSeg = NULL;
		if(itt!=sp->shortestWayPointpath.end())
		{

			targetSeg = itt->roadSegment_;
		}
		else // already last segment
		{
			break;
		}

		if(currentSeg->getEnd() == currentSeg->getLink()->getEnd()) // intersection
		{
			signalNumber++;
			// get lane connector
			const std::set<sim_mob::LaneConnector*>& lcs = dynamic_cast<const MultiNode*> (currentSeg->getEnd())->getOutgoingLanes(currentSeg);
			for (std::set<LaneConnector*>::const_iterator it2 = lcs.begin(); it2 != lcs.end(); it2++) {
				if((*it2)->getLaneTo()->getRoadSegment() == targetSeg)
				{
					int laneIndex = sim_mob::getLaneIndex2((*it2)->getLaneFrom());
					if(laneIndex<2)//most left lane
					{
						res++;
						break;
					}
				}//end if targetSeg
			}// end for lcs
//			}// end if lcs
		}//end currEndNode
		++itt;
	}//end for
	sp->right_turn_number=res;
	sp->signal_number=signalNumber;
}

void sim_mob::generatePathSizeForPathSet2(boost::shared_ptr<sim_mob::PathSet>&ps,bool isUseCache)
{
	// Step 1: the length of each path in the path choice set
	double minL = ps->oriPath->length;

	// find MIN_TRAVEL_TIME
	double minTravelTime=99999999.0;
	// record which is min
	sim_mob::SinglePath *minSP = nullptr;
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(ps->oriPath && sp->id == ps->oriPath->id){
			minTravelTime = sp->travle_time;
			minSP = sp;
		}
		else{
			if(sp->travle_time < minTravelTime)
			{
				minTravelTime = sp->travle_time;
				minSP = sp;
			}

		}
	}
	if(!ps->pathChoices.empty() && minSP)
	{
		minSP->isMinTravelTime = 1;
	}
	// find MIN_DISTANCE
	double minDistance=99999999.0;
	minSP = 0; // record which is min

	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(ps->oriPath && sp->id == ps->oriPath->id){
			minDistance = sp->length;
			minSP = sp;
		}
		else{
			if(sp->travle_time < minTravelTime)
			{
				minTravelTime = sp->length;
				minSP = sp;
			}

		}

	}
	if(!ps->pathChoices.empty() && minSP)
	{
		minSP->isMinDistance = 1;
	}
	// find MIN_SIGNAL
	int minSignal=99999999;
	minSP = 0; // record which is min

	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(ps->oriPath && sp->id == ps->oriPath->id){
			minSignal = sp->signal_number;
			minSP = sp;
		}
		else{
			if(sp->travle_time < minTravelTime)
			{
				minSignal = sp->signal_number;
				minSP = sp;
			}

		}

	}
	if(!ps->pathChoices.empty())
	{
		minSP->isMinSignal = 1;
	}
	// find MIN_RIGHT_TURN
	int minRightTurn=99999999;
	minSP = 0; // record which is min
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(ps->oriPath && sp->id == ps->oriPath->id){
			minRightTurn = sp->right_turn_number;
			minSP = sp;
		}
		else{
			if(sp->travle_time < minTravelTime)
			{
				minRightTurn = sp->right_turn_number;
				minSP = sp;
			}

		}

	}
	if(!ps->pathChoices.empty())
	{
		minSP->isMinRightTurn = 1;
	}
	// find MAX_HIGH_WAY_USAGE
	double maxHighWayUsage=0.0;
	minSP = 0; // record which is min
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(ps->oriPath && sp->id == ps->oriPath->id){
			maxHighWayUsage = sp->highWayDistance / sp->length;
			minSP = sp;
		}
		else{
			if(sp->travle_time < minTravelTime)
			{
				maxHighWayUsage = sp->highWayDistance / sp->length;
				minSP = sp;
			}

		}
	}
	if(!ps->pathChoices.empty())
	{
		minSP->isMaxHighWayUsage = 1;
	}

	//pathsize
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		//Set size = 0.
		double size=0;
		if(!sp)
		{
			continue;
		}
		// For each link a in the path:
//		for(std::set<const RoadSegment*>::iterator it1 = sp->shortestSegPath.begin();it1 != sp->shortestSegPath.end(); it1++)
		for(std::vector<WayPoint>::iterator it1=sp->shortestWayPointpath.begin(); it1!=sp->shortestWayPointpath.end(); ++it1)
		{
			const sim_mob::RoadSegment* seg = it1->roadSegment_;
				double l=seg->length;
				//Set sum = 0.
				double sum=0;
				//For each path j in the path choice set PathSet(O, D):
				BOOST_FOREACH(sim_mob::SinglePath* spj, ps->pathChoices)
				{
//					std::set<const RoadSegment*>::iterator itt2 = spj->shortestSegPath.find(seg);
					std::vector<WayPoint>::iterator itt2;
					for(itt2 = sp->shortestWayPointpath.begin(); itt2 != sp->shortestWayPointpath.end() && itt2->roadSegment_ != seg; ++itt2);
					if(itt2!=spj->shortestWayPointpath.end())
					{
						//Set sum += minL / L(j)
						sum += minL/(spj->length+0.000001);
					} // itt2!=shortestSegPathj
				} // for j
				size += l/sp->length/(sum+0.000001);
		}
		sp->pathsize = log(size);
	}// end for
}

//bool isSegFound(WayPoint &wp, RoadSegment *rs){
//	return wp.roadSegment_ == rs;
//}

boost::shared_ptr<sim_mob::PathSet> sim_mob::PathSetManager::getPathSetByFromToNodeAimsunId(std::string id)
{
	std::map<std::string,boost::shared_ptr<sim_mob::PathSet> >::iterator it = pathSetPool.find(id);
	if(it == pathSetPool.end())
	{
		sim_mob::Logger::log["path_set"]<<"getPSByAId: no pathset found for "<<id<<std::endl;
		return boost::shared_ptr<sim_mob::PathSet>();
	}
	else
	{
		return (*it).second;
	}
}

bool sim_mob::PathSetManager::getSinglePathById(std::string &id,sim_mob::SinglePath** s)
{
	bool res = false;

	std::map<std::string,SinglePath*>::iterator it =  waypoint_singlepathPool.find(id);
	if(it!=waypoint_singlepathPool.end())
	{
		//find path
		*s = (*it).second;
		res = true;
	}

	return res;
}


double sim_mob::PathSetManager::getTravelTime(sim_mob::SinglePath *sp)
{
//	sim_mob::Logger::log["path_set"].prof("getTravelTime").tick();
	double ts=0.0;
	sim_mob::DailyTime startTime = sp->pathSet->subTrip->startTime;
	for(int i=0;i<sp->shortestWayPointpath.size();++i)
	{
		if(sp->shortestWayPointpath[i].type_ == WayPoint::ROAD_SEGMENT){
			std::string seg_id = sp->shortestWayPointpath[i].roadSegment_->originalDB_ID.getLogItem();
			double t = getTravelTimeBySegId(seg_id,startTime);
			ts += t;
			startTime = startTime + sim_mob::DailyTime(ts*1000);
		}
	}
//	sim_mob::Logger::log["path_set"].prof("getTravelTime").tick(true);
	return ts;
}
double sim_mob::PathSetManager::getTravelTimeBySegId(std::string id,sim_mob::DailyTime startTime)
{
	std::map<std::string,std::vector<sim_mob::LinkTravelTime*> >::iterator it;
	double res=0.0;
	//2. if no , check default
	it = pathSetParam->Link_default_travel_time_pool.find(id);
	if(it!= pathSetParam->Link_default_travel_time_pool.end())
	{
		std::vector<sim_mob::LinkTravelTime*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::LinkTravelTime* l = e[i];
			if( l->start_time_dt.isBeforeEqual(startTime) && l->end_time_dt.isAfter(startTime) )
			{
				res = l->travel_time;
				return res;
			}
		}
	}
	else
	{
		std::string str = "PathSetManager::getTravelTimeBySegId=> no travel time for segment " + id + "  ";
		sim_mob::Logger::log["path_set"]<< "error: " << str << pathSetParam->Link_default_travel_time_pool.size() << std::endl;
	}
	return res;
}

namespace{
struct segFilter{
		bool operator()(const WayPoint value){
			return value.type_ == WayPoint::ROAD_SEGMENT;
		}
	};
}
void sim_mob::SinglePath::init(std::vector<WayPoint>& wpPools)
{

	typedef boost::filter_iterator<segFilter,std::vector<WayPoint>::iterator> FilterIterator;
	std::copy(FilterIterator(wpPools.begin(), wpPools.end()),FilterIterator(wpPools.end(), wpPools.end()),std::back_inserter(this->shortestWayPointpath));
}

void sim_mob::SinglePath::clear()
{
	shortestWayPointpath.clear();
	shortestSegPath.clear();
	id="";
	pathset_id="";
	pathSet.reset();
	fromNode = NULL;
	toNode = NULL;
	utility = 0.0;
	pathsize = 0.0;
	travel_cost=0.0;
	signal_number=0.0;
	right_turn_number=0.0;
	length=0.0;
	travle_time=0.0;
	highWayDistance=0.0;
	isMinTravelTime=0;
	isMinDistance=0;
	isMinSignal=0;
	isMinRightTurn=0;
	isMaxHighWayUsage=0;
}
uint32_t sim_mob::SinglePath::getSize(){

	if(!gotSize.check()){
		return 0;
	}
	uint32_t sum = 0;
	sum += sizeof(WayPoint) * shortestWayPointpath.size(); // std::vector<WayPoint> shortestWayPointpath;
	sum += sizeof(const RoadSegment*) * shortestSegPath.size(); // std::set<const RoadSegment*> shortestSegPath;
	sum += sizeof(boost::shared_ptr<sim_mob::PathSet>); // boost::shared_ptr<sim_mob::PathSet>pathSet; // parent
	sum += sizeof(const sim_mob::RoadSegment*); // const sim_mob::RoadSegment* excludeSeg; // can be null
	sum += sizeof(const sim_mob::Node *); // const sim_mob::Node *fromNode;
	sum += sizeof(const sim_mob::Node *); // const sim_mob::Node *toNode;

	sum += sizeof(double); // double highWayDistance;
	sum += sizeof(bool); // bool isMinTravelTime;
	sum += sizeof(bool); // bool isMinDistance;
	sum += sizeof(bool); // bool isMinSignal;
	sum += sizeof(bool); // bool isMinRightTurn;
	sum += sizeof(bool); // bool isMaxHighWayUsage;
	sum += sizeof(bool); // bool isShortestPath;

	sum += sizeof(bool); // bool isNeedSave2DB;
	sum += id.length(); // std::string id;   //id: seg1id_seg2id_seg3id
	sum += pathset_id.length(); // std::string pathset_id;
	sum += sizeof(double); // double utility;
	sum += sizeof(double); // double pathsize;
	sum += sizeof(double); // double travel_cost;
	sum += sizeof(int); // int signal_number;
	sum += sizeof(int); // int right_turn_number;
	sum += scenario.length(); // std::string scenario;
	sum += sizeof(double); // double length;
	sum += sizeof(double); // double travle_time;
	sum += sizeof(sim_mob::TRIP_PURPOSE); // sim_mob::TRIP_PURPOSE purpose;
	sim_mob::Logger::log["path_set"] << "SinglePath size bytes:" << sum << "\n" ;
	return sum;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
sim_mob::PathSet::~PathSet()
{
	fromNode = NULL;
	toNode = NULL;
	subTrip = NULL;
	sim_mob::Logger::log["path_set"] << "Deleting PathSet " << id << " and its " << pathChoices.size() << "  singlepaths\n";
	BOOST_FOREACH(sim_mob::SinglePath*sp,pathChoices)
	{
		safe_delete_item(sp);
	}
}

sim_mob::PathSet::PathSet(const boost::shared_ptr<sim_mob::PathSet> & ps) :
		logsum(ps->logsum),oriPath(ps->oriPath),
		subTrip(ps->subTrip),
		id(ps->id),
		from_node_id(ps->from_node_id),
		to_node_id(ps->to_node_id),
		scenario(ps->scenario),
		pathChoices(ps->pathChoices),
		singlepath_id(ps->singlepath_id),
		has_path(ps->has_path)
{
	isNeedSave2DB=false;
	isInit = false;
	hasBestChoice = false;
	// 1. get from to nodes
	//	can get nodes later,when insert to personPathSetPool
	this->fromNode = sim_mob::PathSetParam::getInstance()->getCachedNode(from_node_id);
	this->toNode = sim_mob::PathSetParam::getInstance()->getCachedNode(to_node_id);
//	// get all relative singlepath
}


uint32_t sim_mob::PathSet::getSize(){
	if(!gotSize.check()){
		return 0;
	}
	uint32_t sum = 0;
		sum += sizeof(bool);// isInit;
		sum += sizeof(bool);//bool hasBestChoice;
		sum += sizeof(WayPoint) * bestWayPointpath.size();//std::vector<WayPoint> bestWayPointpath;  //best choice
		sum += sizeof(const sim_mob::Node *);//const sim_mob::Node *fromNode;
		sum += sizeof(const sim_mob::Node *);//const sim_mob::Node *toNode;
		sum += personId.length();//std::string personId; //person id
		sum += tripId.length();//std::string tripId; // trip item id
		sum += sizeof(SinglePath*);//SinglePath* oriPath;  // shortest path with all segments
		//std::map<std::string,sim_mob::SinglePath*> SinglePathPool;//unused so far
		std::pair<std::string,sim_mob::SinglePath*> pair_;
		BOOST_FOREACH(pair_,SinglePathPool)
		{
			sum += pair_.first.length();
			sum += sizeof(pair_.second);
		}
		//std::set<sim_mob::SinglePath*, sim_mob::SinglePath> pathChoices;
		sim_mob::SinglePath* sp;
		BOOST_FOREACH(sp,pathChoices)
		{
			uint32_t t = sp->getSize();
			sum += t;//real singlepath size
		}
		sum += sizeof(bool);//bool isNeedSave2DB;
		sum += sizeof(double);//double logsum;
		sum += sizeof(const sim_mob::SubTrip*);//const sim_mob::SubTrip* subTrip;
		sum += id.length();//std::string id;
		sum += from_node_id.length();//std::string from_node_id;
		sum += to_node_id.length();//std::string to_node_id;
		sum += singlepath_id.length();//std::string singlepath_id;
		sum += excludedPaths.length();//std::string excludedPaths;
		sum += scenario.length();//std::string scenario;
		sum += sizeof(int);//int has_path;
		sum += sizeof(PathSetManager *);//PathSetManager *psMgr;
		sim_mob::Logger::log["path_set"] << "pathset_cached_bytes :" << sum << "\n" ;
		return sum;
}

sim_mob::ERP_Section::ERP_Section(ERP_Section &src)
	: section_id(src.section_id),ERP_Gantry_No(src.ERP_Gantry_No),
	  ERP_Gantry_No_str(boost::lexical_cast<std::string>(src.ERP_Gantry_No))
{
	originalSectionDB_ID.setProps("aimsun-id",src.section_id);
}

sim_mob::LinkTravelTime::LinkTravelTime(LinkTravelTime& src)
	: link_id(src.link_id),
			start_time(src.start_time),end_time(src.end_time),travel_time(src.travel_time),
			start_time_dt(sim_mob::DailyTime(src.start_time)),end_time_dt(sim_mob::DailyTime(src.end_time))
{
	originalSectionDB_ID.setProps("aimsun-id",src.link_id);
}



/***********************************************************************************************************
 * **********************************  UNUSED CODE!!!! *****************************************************
************************************************************************************************************/

//todo: if discarding this method, check if sim_mob::RoadSegment::getRoadSegmentByAimsunId is needed anymore
sim_mob::SinglePath::SinglePath(const SinglePath& source) :
		id(source.id),
		utility(source.utility),pathsize(source.pathsize),
		travel_cost(source.travel_cost),
		signal_number(source.signal_number),
		right_turn_number(source.right_turn_number),
		length(source.length),travle_time(source.travle_time),
		pathset_id(source.pathset_id),highWayDistance(source.highWayDistance),
		isMinTravelTime(source.isMinTravelTime),isMinDistance(source.isMinDistance),isMinSignal(source.isMinSignal),
		isMinRightTurn(source.isMinRightTurn),isMaxHighWayUsage(source.isMaxHighWayUsage),isShortestPath(source.isShortestPath)
{
	isNeedSave2DB=false;

	purpose = sim_mob::work;

	//use id to build shortestWayPointpath
	std::vector<std::string> segIds;
	boost::split(segIds,source.id,boost::is_any_of(","));
	// no path is correct
	for(int i=0;i<segIds.size();++i)
	{
		std::string id = segIds.at(i);
		if(id.size()>1)
		{
			sim_mob::RoadSegment* seg = sim_mob::PathSetParam::getInstance()->getRoadSegmentByAimsunId(id);
			if(!seg)
			{
				std::string str = "SinglePath: seg not find " + id;
				sim_mob::Logger::log["path_set"] << "error: " << str << "\n";
			}
//			sim_mob::WayPoint *w = sim_mob::PathSetParam::getInstance()->getWayPointBySeg(seg);
//			this->shortestWayPointpath.push_back(w);
			this->shortestWayPointpath.push_back(WayPoint(seg));//copy better than this twist
			shortestSegPath.insert(seg);
		}
	}
}

bool sim_mob::PathSetManager::LoadSinglePathDBwithId(
		std::string& pathset_id,
		std::set<sim_mob::SinglePath*,sim_mob::SinglePath>& spPool)
{
	bool res=false;
	res = sim_mob::aimsun::Loader::LoadSinglePathDBwithId2(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
			waypoint_singlepathPool,pathset_id,spPool);
	return res;
}

bool sim_mob::PathSetParam::insertTravelTimeCSV2TmpTable(std::ofstream& csvFile,std::string& csvFileName)
{
	boost::mutex::scoped_lock local_lock(soci_mutex);
		  csvFile.close();
		  bool res = sim_mob::aimsun::Loader::insertCSV2Table(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
		  			pathset_traveltime_tmp_table_name,csvFileName);
		  csvFile.open(csvFileName.c_str(),std::ios::in | std::ios::trunc);
	return res;
}


bool sim_mob::PathSetParam::copyTravelTimeDataFromTmp2RealtimeTable(std::ofstream& csvFile,std::string& csvFileName)
{
	boost::mutex::scoped_lock local_lock(soci_mutex);
	//0. copy csv to travel time table
	csvFile.close();
	sim_mob::Logger::log["path_set"]<<"table name: "<<pathset_traveltime_tmp_table_name<<std::endl;
	sim_mob::aimsun::Loader::insertCSV2Table(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
			pathset_traveltime_tmp_table_name,csvFileName);
	//1. truncate realtime table
	bool res=false;
	res = sim_mob::aimsun::Loader::truncateTable(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
			pathset_traveltime_realtime_table_name);
	if(!res)
		return res;
	//2. insert into "max_link_realtime_travel_time" (select * from "link_default_travel_time");
	std::string str = "insert into " + pathset_traveltime_realtime_table_name +
			"(select * from " + pathset_traveltime_tmp_table_name +")";
	res = sim_mob::aimsun::Loader::excuString(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),str);

	return res;
}

sim_mob::SinglePath * sim_mob::PathSetManager::generateSinglePathByFromToNodes(const sim_mob::Node *fromNode,
		   const sim_mob::Node *toNode,const sim_mob::RoadSegment* excludedSegs)
{
	sim_mob::SinglePath *s=NULL;
	std::vector<const sim_mob::RoadSegment*> blacklist;
	if(excludedSegs)
	{
		blacklist.push_back(excludedSegs);
	}
	std::vector<WayPoint> wp = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*fromNode), stdir.DrivingVertex(*toNode),blacklist);
	if(wp.size()==0)
	{
		// no path
		if(excludedSegs)
		{
			sim_mob::Logger::log["path_set"]<<"gSPByFTNodes: no path for nodes and ex seg"<<fromNode->originalDB_ID.getLogItem()<<
				toNode->originalDB_ID.getLogItem()<<
				excludedSegs->originalDB_ID.getLogItem()<<std::endl;
		}
		else
		{
			sim_mob::Logger::log["path_set"]<<"gSPByFTNodes: no path for nodes"<<fromNode->originalDB_ID.getLogItem()<<
							toNode->originalDB_ID.getLogItem()<<std::endl;
		}
		return s;
	}
	// make sp id
	std::string id = sim_mob::makeWaypointsetString(wp);
	// 1.31 check path pool
	bool bb = false;
	bb = getSinglePathById(id,&s);  //getWayPointPath2(wp,&s);
	if(!bb) // no stored path found, generate new one
	{
		s = new SinglePath();
		// fill data
		s->isNeedSave2DB = true;
		s->shortestWayPointpath = wp;//convertWaypoint2Point(wp);//stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*fromNode), stdir.DrivingVertex(*toNode),blacklist);
		s->shortestSegPath = sim_mob::generateSegPathByWaypointPath(s->shortestWayPointpath);
		sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(s);
		s->fromNode = fromNode;
		s->toNode = toNode;
		s->pathSet.reset();
		s->length = sim_mob::generateSinglePathLengthPT(s->shortestWayPointpath);
		s->id = id;
		s->scenario = scenarioName;
		s->pathsize=0;
	}
	return s;
}
bool sim_mob::PathSetManager::generateAllPathSetWithTripChain()
{
	const std::map<std::string, std::vector<sim_mob::TripChainItem*> > *tripChainPool =
			&ConfigManager::GetInstance().FullConfig().getTripChains();
	sim_mob::Logger::log["path_set"]<<"generateAllPathSetWithTripChain: trip chain pool size "<<
			tripChainPool->size()<<std::endl;
	int poolsize = tripChainPool->size();
	bool res=true;
	// 1. get from and to node
	std::map<std::string, std::vector<sim_mob::TripChainItem*> >::const_iterator it;
	int i=1;
	for(it = tripChainPool->begin();it!=tripChainPool->end();++it)
	{
		std::string personId = (*it).first;
		std::vector<sim_mob::TripChainItem*> tci = (*it).second;
		std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > subTripId_pathSet = generatePathSetByTripChainItemPool(tci);
		storePersonIdPathSets(personId,subTripId_pathSet);
		i++;
		sim_mob::Logger::log["path_set"]<<"generateAllPathSetWithTripChain: "<<i<<"/"<<poolsize<<" mem: "<<size()/1024/1024<<"mb"<<std::endl;
	}
	//
	return res;
}

boost::shared_ptr<sim_mob::PathSet>sim_mob::PathSetManager::generatePathSetByFromToNodes(const sim_mob::Node *fromNode,
														   const sim_mob::Node *toNode,
														   const sim_mob::SubTrip* st,
														   bool isUseCache)
{
	// 0. check pathSetPool already calculate before with this pair
	std::string fromToID = fromNode->getID() +","+ toNode->getID();
	// 0.1 no data in memory, so check db
	std::string pathSetID=fromToID;
	pathSetID = "'"+pathSetID+"'";
	boost::shared_ptr<sim_mob::PathSet>ps;
	// check cache first
	if(isUseCache)
	{
		ps = getPathSetByFromToNodeAimsunId(fromToID);
		if(ps)
		{
			return ps;
		}
	}
	else
	{
		clearPools();
	}
	if(!isUseCache)
	{
	}
	else
	{
		bool res = sim_mob::aimsun::Loader::LoadPathSetDBwithId(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false), pathSetPool,pathSetID);
		if(res)
		{
			ps = getPathSetByFromToNodeAimsunId(fromToID);
			// 0.2 no data in db, so create new
			if(ps)
			{
				if(!ps->isInit)
				{
				// get all relative singlepath
					std::vector<sim_mob::SinglePath*> allChoices;
					LoadSinglePathDBwithId(pathSetID,ps->pathChoices);
					// 2. get SinglePath
					if(!ps->oriPath)
					{
						std::string str = "PathSet: oriPath not find,no path for this nodes pair " + (ps ? ps->id : " unknown");
										sim_mob::Logger::log["path_set"]<<str<<std::endl;
					}
					ps->isInit = true;
				}
				return ps;
			}
			else
			{
				sim_mob::Logger::log["path_set"]<<"gPSByFTNodes: data in db ,but not in mem?  "<<fromToID<<std::endl;
			}
		}
	}
	sim_mob::Logger::log["path_set"]<<"gPSByFTNodes: create data for "<<fromToID<<std::endl;
	// 1. generate shortest path with all segsstdir
	// 1.2 get all segs
	// 1.3 generate shortest path with full segs
	sim_mob::SinglePath *s = generateSinglePathByFromToNodes(fromNode,toNode);
	if(!s)
	{
		// no path
		return ps;
	}
	// 1.4 create PathSet object
	ps.reset(new PathSet(fromNode,toNode));
	ps->subTrip = st;
	ps->isNeedSave2DB = true;
	ps->oriPath = s;
	ps->id = fromToID;
	ps->singlepath_id = s->id;
	s->pathset_id = ps->id;
	s->pathSet = ps;
	s->travel_cost = getTravelCost2(s);
	s->travle_time = getTravelTime(s);
	ps->pathChoices.insert(s);
	// 2. exclude each seg in shortest path, then generate new shortest path
	for(int i=0;i<s->shortestWayPointpath.size();++i)
	{
		WayPoint &w = s->shortestWayPointpath[i];
		if (w.type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w.roadSegment_;
			SinglePath *sinPath = generateSinglePathByFromToNodes(fromNode,toNode,seg);

			if(!sinPath)
			{
				continue;
			}
			sinPath->pathSet = ps; // set parent
			sinPath->travel_cost = getTravelCost2(sinPath);
			sinPath->travle_time = getTravelTime(sinPath);
			sinPath->pathset_id = ps->id;
//			storePath(sinPath);
			ps->pathChoices.insert(sinPath);
		}
	}//end for
	// calculate oriPath's path size

//	ps->from_node_id = fromNode->originalDB_ID.getLogItem();
//	ps->to_node_id = toNode->originalDB_ID.getLogItem();
//	ps->from_node_id = fromNode->getID();
//	ps->to_node_id = toNode->getID();
	std:string temp = fromNode->originalDB_ID.getLogItem();
	ps->from_node_id = sim_mob::getNumberFromAimsunId(temp);
	temp = toNode->originalDB_ID.getLogItem();
	ps->to_node_id = sim_mob::getNumberFromAimsunId(temp);
	ps->scenario = scenarioName;
	// 3. store pathset
	sim_mob::generatePathSizeForPathSet2(ps);
	if(isUseCache)
	{
		pathSetPool.insert(std::make_pair(fromToID,ps));
	}
	std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > tmp;
	tmp.insert(std::make_pair(fromToID,ps));
	std::string cnn(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
	sim_mob::aimsun::Loader::SaveOnePathSetData(cnn,tmp, pathSetTableName);

	return ps;
}

std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > sim_mob::PathSetManager::generatePathSetByTripChainItemPool(std::vector<sim_mob::TripChainItem*> &tci)
{
	std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > res;
	//
	for(std::vector<sim_mob::TripChainItem*>::iterator it=tci.begin(); it!=tci.end(); ++it)
	{
		if((*it)->itemType == sim_mob::TripChainItem::IT_TRIP)
		{
			sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip*> ((*it));
			if(!trip)
				throw std::runtime_error("generateAllPathSetWithTripChainPool: trip error");
			//
			generatePathSetByTrip(res,trip);

		}
	}

	return res;
}

void sim_mob::PathSetManager::storePersonIdPathSets(std::string personId,std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > &subTripId_pathSet)
{
	personPathSetPool.insert(std::make_pair(personId,subTripId_pathSet));
}

void sim_mob::PathSetManager::generateAllPathSetWithTripChainPool(std::map<std::string, std::vector<sim_mob::TripChainItem*> > *tripChainPool)
{
	// 1. get from and to node
	std::map<std::string, std::vector<sim_mob::TripChainItem*> >::iterator it;
	for(it = tripChainPool->begin();it!=tripChainPool->end();++it)
	{
		std::string personId = (*it).first;
		std::vector<sim_mob::TripChainItem*> tci = (*it).second;
		std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > subTripId_pathSet = generatePathSetByTripChainItemPool(tci);
		storePersonIdPathSets(personId,subTripId_pathSet);
	}
}

void sim_mob::PathSetManager::storePath(sim_mob::SinglePath* singlePath)
{
	if(singlePath && singlePath->id.size()>1)
	{
		waypoint_singlepathPool.insert(std::make_pair(singlePath->id,singlePath));
	}
	else
	{
		sim_mob::Logger::log["path_set"]<<"storePath: id wrong, ignore"<<std::endl;
	}
}


boost::shared_ptr<sim_mob::PathSet> sim_mob::PathSetManager::generatePathSetBySubTrip(const sim_mob::SubTrip* st)
{
	std::string subTripId = st->tripID;
	boost::shared_ptr<sim_mob::PathSet> ps;
	if(st->mode == "Car") //only driver need path set
	{
		const sim_mob::Node* fromNode = st->fromLocation.node_;
		const sim_mob::Node* toNode = st->toLocation.node_;
		ps = generatePathSetByFromToNodes(fromNode,toNode,st,false);
	}
	return ps;
}


const sim_mob::Node* sim_mob::PathSetManager::getFromNodefromTripChainItems(std::vector<sim_mob::TripChainItem*> &tci)
{
	// get first trip
	for(int i=0;i<tci.size();++i)
	{
		sim_mob::TripChainItem* f = tci[i];
		if(f->itemType == sim_mob::TripChainItem::IT_TRIP)
		{
			sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip*> (f);
			return trip->fromLocation.node_;
		}
	}
	return NULL;
}

const sim_mob::Node* sim_mob::PathSetManager::getToNodefromTripChainItems(std::vector<sim_mob::TripChainItem*> &tci)
{
	// get first trip
	for(int i=tci.size()-1;i>0;--i)
	{
		sim_mob::TripChainItem* f = tci[i];
		if(f->itemType == sim_mob::TripChainItem::IT_TRIP)
		{
			sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip*> (f);
			return trip->toLocation.node_;
		}
	}
	return NULL;
}


boost::shared_ptr<sim_mob::PathSet> sim_mob::PathSetManager::getPathSetByPersonIdAndSubTripId(std::string personId,std::string subTripId)
{
	std::map<std::string, std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > >::iterator it=personPathSetPool.find(personId);
	if(it != personPathSetPool.end())
	{
		std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > subTripId_pathset = (*it).second;
		std::map<std::string,boost::shared_ptr<sim_mob::PathSet> >::iterator itt = subTripId_pathset.find(subTripId);
		if(itt != subTripId_pathset.end())
		{
			boost::shared_ptr<sim_mob::PathSet> ps = (*itt).second;
			return ps;
		}
		else
		{
			throw std::runtime_error("getPathSetByPersonIdAndSubTripId: find person ,but not find pathset for subtrip");
		}
	}
	else
	{
		throw std::runtime_error("getPathSetByPersonIdAndSubTripId: not find person");
	}
	return boost::shared_ptr<sim_mob::PathSet>();
}

void sim_mob::PathSetManager::generatePaths2Node(const sim_mob::Node *toNode)
{
	// 1. from multinode to toNode
	for(int i=0;i< pathSetParam->multiNodesPool.size();++i)
	{
		sim_mob::MultiNode* fn = pathSetParam->multiNodesPool.at(i);
		generateSinglePathByFromToNodes(fn,toNode);
	}
	// 2. from uninode to toNode
	for(std::set<sim_mob::UniNode*>::iterator it= pathSetParam->uniNodesPool.begin(); it!= pathSetParam->uniNodesPool.end(); ++it)
	{
		sim_mob::UniNode* fn = (*it);
		generateSinglePathByFromToNodes(fn,toNode);
	}
}
bool sim_mob::PathSetManager::generateSinglePathByFromToNodes2(
		   const sim_mob::Node *fromNode,
		   const sim_mob::Node *toNode,
		   sim_mob::SinglePath& sp,
		   const sim_mob::RoadSegment* excludedSegs)
{
	bool res=false;
	sim_mob::SinglePath s;
	std::vector<const sim_mob::RoadSegment*> blacklist;
	if(excludedSegs)
	{
		blacklist.push_back(excludedSegs);
	}
	std::vector<WayPoint> wp = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*fromNode), stdir.DrivingVertex(*toNode),blacklist);
	if(wp.size()==0)
	{
		// no path
		if(excludedSegs)
		{
			sim_mob::Logger::log["path_set"]<<"gSPByFTNodes2: no path for nodes and ex seg"<<fromNode->originalDB_ID.getLogItem()<<
				toNode->originalDB_ID.getLogItem()<<
				excludedSegs->originalDB_ID.getLogItem()<<std::endl;
		}
		else
		{
			sim_mob::Logger::log["path_set"]<<"gSPByFTNodes2: no path for nodes"<<fromNode->originalDB_ID.getLogItem()<<
							toNode->originalDB_ID.getLogItem()<<std::endl;
		}
		return res;
	}
	res=true;
	// make sp id
	std::string id = sim_mob::makeWaypointsetString(wp);
	// 1.31 check path pool
		// fill data
		s.isNeedSave2DB = true;
		s.shortestWayPointpath = wp;//convertWaypoint2Point(wp);//stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*fromNode), stdir.DrivingVertex(*toNode),blacklist);
		s.shortestSegPath = sim_mob::generateSegPathByWaypointPath(s.shortestWayPointpath);
		sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(&s);
		s.fromNode = fromNode;
		s.toNode = toNode;
		s.pathSet.reset();
		s.length = sim_mob::generateSinglePathLengthPT(s.shortestWayPointpath);
		// file db object data
		s.id = id;
		s.scenario = scenarioName;
		s.pathsize=0;
	return res;
}

int sim_mob::PathSetManager::generateSinglePathByFromToNodes_(
		   const sim_mob::Node *fromNode,
		   const sim_mob::Node *toNode,
		   std::map<std::string,SinglePath*>& wp_spPool,
		   const sim_mob::RoadSegment* excludedSegs){

	std::vector<const sim_mob::RoadSegment*> blacklist;
	if(excludedSegs)
	{
		blacklist.push_back(excludedSegs);
	}
	std::vector<WayPoint> wp = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*fromNode), stdir.DrivingVertex(*toNode),blacklist);
	if(wp.size()==0)
	{
		// no path
		sim_mob::Logger::log["path_set"] << "No Path" << std::endl;
	}

}


inline std::set<const RoadSegment*> sim_mob::generateSegPathByWaypointPath(std::vector<WayPoint>& wp)
{
	sim_mob::Logger::log["path_set"] << "WARNING: generateSegPathByWaypointPath and shortestSegPath are disabled and to be discarded soon\n";
	std::set<const RoadSegment*> res;
	for (vector<WayPoint>::iterator it = wp.begin(); it != wp.end(); it++) {
		if (it->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = it->roadSegment_;
			res.insert(seg);
		}
	}
	return res;
}

inline int sim_mob::calculateRightTurnNumberByWaypoints(std::set<const RoadSegment*>& segWp)
{
	if(segWp.size()<2)
		return 0;
	int res=0;
	std::set<const RoadSegment*>::iterator itt=segWp.begin();
	++itt;
	for(std::set<const RoadSegment*>::iterator it=segWp.begin();
			it!=segWp.end();
			++it)
	{
		const RoadSegment* currentSeg = (*it);
		const RoadSegment* targetSeg = NULL;
		if(itt!=segWp.end())
		{

			targetSeg = (*itt);
		}
		else // already last segment
		{
			break;
		}
//		const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (currentSeg->getEnd());//why too many dynamic_cast?
		if(currentSeg->getEnd() == currentSeg->getLink()->getEnd()) // intersection
		{
			// get lane connector
			const std::set<sim_mob::LaneConnector*>& lcs = dynamic_cast<const MultiNode*> (currentSeg->getEnd())->getOutgoingLanes(currentSeg);//todo, change the lane connectors to avoid dynamic_cast
				for (std::set<LaneConnector*>::const_iterator it2 = lcs.begin(); it2 != lcs.end(); it2++) {
					if((*it2)->getLaneTo()->getRoadSegment() == targetSeg)
					{
						int laneIndex = sim_mob::getLaneIndex2((*it2)->getLaneFrom());
						if(laneIndex<2)//most left lane
						{
							res++;
							break;
						}
					}//end if targetSeg
				}// end for lcs
		}//end currEndNode
		++itt;
	}//end for
	return res;
}

bool sim_mob::PathSetManager::getWayPointPath2(std::vector<WayPoint> &wp,sim_mob::SinglePath** s)
{
	bool res = false;
	std::string id;
	for (vector<WayPoint>::iterator it = wp.begin(); it != wp.end(); it++) {
			if (it->type_ == WayPoint::ROAD_SEGMENT) {
				const sim_mob::RoadSegment* seg = it->roadSegment_;
				id += seg->originalDB_ID.getLogItem() + ",";
			}
	}

	std::map<std::string,SinglePath*>::iterator it =  waypoint_singlepathPool.find(id);
	if(it!=waypoint_singlepathPool.end())
	{
		//find path
		*s = (*it).second;
		res = true;
	}
	return res;
}

sim_mob::SinglePath* sim_mob::PathSetManager::getSinglePath(std::string id)
{
	std::map<std::string,sim_mob::SinglePath*>::iterator it = waypoint_singlepathPool.find(id);
	if(it != waypoint_singlepathPool.end())
	{
		//find path
		// as utility is diff for each path,person, generate separate object
//		sim_mob::SinglePath* p = new sim_mob::SinglePath((*it).second);
		sim_mob::SinglePath* p = (*it).second;
		return p;
	}
	else
	{
		sim_mob::Logger::log["path_set"]<<"warning getSinglePath"<<std::endl;
	}
	return NULL;
}

double sim_mob::PathSetManager::getTravelCost(sim_mob::SinglePath *sp)
{
	double res=0.0;
	sim_mob::DailyTime trip_startTime = sp->pathSet->subTrip->startTime;
//	std::set<const RoadSegment*> shortestSegPath = generateSegPathByWaypointPath(sp->shortestWayPointpath);
	for(std::set<const RoadSegment*>::iterator it1 = sp->shortestSegPath.begin();it1 != sp->shortestSegPath.end(); it1++)
	{
		std::string seg_id = (*it1)->originalDB_ID.getLogItem();
//		sim_mob::Logger::log["path_set"]<<"getTravelCost: "<<seg_id<<std::endl;
		std::map<std::string,sim_mob::ERP_Section*>::iterator it = ERP_Section_pool.find(seg_id);
		if(it!=ERP_Section_pool.end())
		{
			sim_mob::ERP_Section* erp_section = (*it).second;
			std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> >::iterator itt =
					sim_mob::PathSetParam::getInstance()->ERP_Surcharge_pool.find(erp_section->ERP_Gantry_No_str);
			if(itt!=sim_mob::PathSetParam::getInstance()->ERP_Surcharge_pool.end())
			{
				std::vector<sim_mob::ERP_Surcharge*> erp_surcharges = (*itt).second;
				for(int i=0;i<erp_surcharges.size();++i)
				{
					sim_mob::ERP_Surcharge* s = erp_surcharges[i];
					if( s->start_time_dt.isBeforeEqual(trip_startTime) && s->end_time_dt.isAfter(trip_startTime) )
					{
						res += s->Rate;
					}
				}
			}
		}
	}

	return res;
}

sim_mob::SinglePath::~SinglePath(){
	clear();
}
//TODO: obsolete?
sim_mob::SinglePath::SinglePath(SinglePath *source,const sim_mob::RoadSegment* seg) :
		shortestWayPointpath(source->shortestWayPointpath),
		shortestSegPath(source->shortestSegPath),
		pathSet(source->pathSet),fromNode(source->fromNode),
		toNode(source->toNode),id(source->id),
		utility(source->utility),pathsize(source->pathsize),travel_cost(source->travel_cost),
		signal_number(source->signal_number),right_turn_number(source->right_turn_number),
		length(source->length),travle_time(source->travle_time),
		pathset_id(source->pathset_id),isShortestPath(source->isShortestPath)
{
	isNeedSave2DB=false;
	purpose = sim_mob::work;
}

//TODO: obsolete?
//todo: if discarding this method, check if sim_mob::RoadSegment::getRoadSegmentByAimsunId is needed anymore
sim_mob::SinglePath::SinglePath(SinglePath *source) :
		pathSet(source->pathSet),fromNode(source->fromNode),
		toNode(source->toNode),purpose(source->purpose),
		utility(source->utility),pathsize(source->pathsize),travel_cost(source->travel_cost),
		signal_number(source->signal_number),right_turn_number(source->right_turn_number),
		length(source->length),travle_time(source->travle_time),
		pathset_id(source->pathset_id),highWayDistance(source->highWayDistance),
		isMinTravelTime(source->isMinTravelTime),isMinDistance(source->isMinDistance),isMinSignal(source->isMinSignal),
		isMinRightTurn(source->isMinRightTurn),isMaxHighWayUsage(source->isMaxHighWayUsage),isShortestPath(source->isShortestPath)
{
	isNeedSave2DB=false;

	purpose = sim_mob::work;
	if(source->shortestWayPointpath.size()>0)
	{
		this->shortestWayPointpath = source->shortestWayPointpath;
		this->shortestSegPath = generateSegPathByWaypointPath(this->shortestWayPointpath);

	}
	else
	{
		//use id to build shortestWayPointpath
		std::vector<std::string> segIds;
		boost::split(segIds,source->id,boost::is_any_of(","));
		// no path is correct
		for(int i=0;i<segIds.size();++i)
		{
			std::string id = segIds.at(i);
			if(id.size()>1)
			{
				sim_mob::RoadSegment* seg = sim_mob::PathSetParam::getInstance()->getRoadSegmentByAimsunId(id);
				if(!seg)
				{
					std::string str = "error: SinglePath: seg not find. " + id + "\n";
					sim_mob::Logger::log["path_set"] << str ;
				}
//				sim_mob::WayPoint *w = sim_mob::PathSetParam::getInstance()->getWayPointBySeg(seg);
				this->shortestWayPointpath.push_back(WayPoint(seg));
//				shortestSegPath.insert(seg);
			}
		}
	}//end of else
}


bool sim_mob::SinglePath::includesRoadSegment(const sim_mob::RoadSegment* seg){
	BOOST_FOREACH(sim_mob::WayPoint wp, this->shortestWayPointpath){
		if(!(wp.roadSegment_)){
			continue;
		}
		if(seg == wp.roadSegment_) {
			return true;
		}
	}
	return false;
}
bool sim_mob::SinglePath::includesRoadSegment(const std::set<const sim_mob::RoadSegment*> & segs){
	BOOST_FOREACH(sim_mob::WayPoint wp, this->shortestWayPointpath){
		BOOST_FOREACH(const sim_mob::RoadSegment* seg, segs){
			if(wp.roadSegment_ == seg){
				//sim_mob::Logger::log["path_set"] << "sp will be maximized due to segment " << seg->getSegmentAimsunId() << std::endl;
				return true;
			}
		}
	}
	return false;
}

sim_mob::PathSet::PathSet(boost::shared_ptr<sim_mob::PathSet> &ps) :
		fromNode(ps->fromNode),toNode(ps->toNode),
		logsum(ps->logsum),oriPath(ps->oriPath),
		subTrip(ps->subTrip),
		id(ps->id),
		from_node_id(ps->from_node_id),
		to_node_id(ps->to_node_id),
		scenario(ps->scenario),
		pathChoices(ps->pathChoices),
		has_path(ps->has_path)
{
	if(!ps)
		throw std::runtime_error("PathSet error");

	isNeedSave2DB=false;
}
