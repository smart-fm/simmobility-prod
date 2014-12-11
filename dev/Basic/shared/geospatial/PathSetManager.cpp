/*
 * PathSetManager.cpp
 *
 *  Created on: May 6, 2013
 *      Author: Max
 */

#include "PathSetManager.hpp"
#include "entities/PersonLoader.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/PathSet/PathSetThreadPool.h"
#include "geospatial/streetdir/KShortestPathImpl.hpp"
#include "util/threadpool/Threadpool.hpp"
#include "workers/Worker.hpp"
#include "message/MessageBus.hpp"
#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <sstream>

using std::vector;
using std::string;

using namespace sim_mob;

namespace{
sim_mob::BasicLogger & logger = sim_mob::Logger::log("path_set");
}

std::string getFromToString(const sim_mob::Node* fromNode,const sim_mob::Node* toNode ){
	std::stringstream out("");
	std::string idStrTo = toNode->originalDB_ID.getLogItem();
	std::string idStrFrom = fromNode->originalDB_ID.getLogItem();
	out << Utils::getNumberFromAimsunId(idStrFrom) << "," << Utils::getNumberFromAimsunId(idStrTo);
	//Print() << "debug: " << idStrFrom << "   " << Utils::getNumberFromAimsunId(idStrFrom) << std::endl;
	return out.str();
}

PathSetManager *sim_mob::PathSetManager::instance_;

PathSetParam *sim_mob::PathSetParam::instance_ = NULL;

std::map<boost::thread::id, boost::shared_ptr<soci::session> > sim_mob::PathSetManager::cnnRepo;
boost::shared_ptr<sim_mob::batched::ThreadPool> sim_mob::PathSetManager::threadpool_(new sim_mob::batched::ThreadPool(10));

sim_mob::PathSetParam* sim_mob::PathSetParam::getInstance()
{
	if(!instance_)
	{
		instance_ = new PathSetParam();
	}
	return instance_;
}

void sim_mob::PathSetParam::getDataFromDB()
{
	logger << "[RTT TABLE NAME : " << ConfigManager::GetInstance().FullConfig().getRTTT() << "]\n";
	std::cout << "[RTT TABLE NAME : " << ConfigManager::GetInstance().FullConfig().getRTTT() << "]\n";
	setRTTT(ConfigManager::GetInstance().FullConfig().getRTTT());

		sim_mob::aimsun::Loader::LoadERPData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
				ERP_SurchargePool,	ERP_Gantry_ZonePool,ERP_SectionPool);
		logger << "[ERP RETRIEVED " <<	ERP_SurchargePool.size() << " "  << ERP_Gantry_ZonePool.size() << " " << ERP_SectionPool.size() << "]\n";

		sim_mob::aimsun::Loader::LoadDefaultTravelTimeData(*(PathSetManager::getSession()), segmentDefaultTravelTimePool);
		logger << segmentDefaultTravelTimePool.size() << " records for Link_default_travel_time found\n";

		bool res = sim_mob::aimsun::Loader::LoadRealTimeTravelTimeData(*(PathSetManager::getSession()),
				RTTT,	segmentRealTimeTravelTimePool);
		logger << segmentRealTimeTravelTimePool.size() << " records for Link_realtime_travel_time found\n";
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
	sim_mob::aimsun::Loader::storeSinglePath(sql,spPool,singlePathTableName);
}

bool sim_mob::PathSetParam::createTravelTimeRealtimeTable()
{
	bool res=false;
	std::string createTableStr = RTTT + " ( \"link_id\" integer NOT NULL,\"start_time\" time without time zone NOT NULL,\"end_time\" time without time zone NOT NULL,\"travel_time\" double precision )";
	res = sim_mob::aimsun::Loader::createTable(*(PathSetManager::getSession()),createTableStr);
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
	logger << "[REALTIME TABLE NAME : " << RTTT << "]\n";
}

double sim_mob::PathSetParam::getAverageTravelTimeBySegIdStartEndTime(std::string id,sim_mob::DailyTime startTime,sim_mob::DailyTime endTime)
{
	//1. check realtime table
	double res=0.0;
	double totalTravelTime=0.0;
	int count=0;
	std::map<std::string,std::vector<sim_mob::LinkTravelTime> >::iterator it =
			segmentRealTimeTravelTimePool.find(id);
	if(it!=segmentRealTimeTravelTimePool.end())
	{
		//logger << "using realtime travel time \n";
		std::vector<sim_mob::LinkTravelTime> &e = (*it).second;
		for(int i = 0;i < e.size();++i)
		{
			sim_mob::LinkTravelTime& l = e[i];
			if( l.startTime_DT.isAfterEqual(startTime) && l.endTime_DT.isBeforeEqual(endTime) )
			{
				totalTravelTime += l.travelTime;
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
	it = segmentDefaultTravelTimePool.find(id);
	if(it!=segmentDefaultTravelTimePool.end())
	{
//		logger << "using default travel time \n";
		std::vector<sim_mob::LinkTravelTime> &e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::LinkTravelTime& l = e[i];
				totalTravelTime += l.travelTime;
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
		logger << "[NO TT FOR SEGMENT : " << id << "]\n";
	}
	return res;
}

double sim_mob::PathSetParam::getDefaultTravelTimeBySegId(std::string id)
{
	double res=0.0;
	double totalTravelTime=0.0;
	int count=0;
	std::map<std::string,std::vector<sim_mob::LinkTravelTime> >::iterator it = segmentDefaultTravelTimePool.find(id);
	if(it!=segmentDefaultTravelTimePool.end())
	{
		std::vector<sim_mob::LinkTravelTime> &e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::LinkTravelTime& l = e[i];
				totalTravelTime += l.travelTime;
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
		logger << "[NO DEFTT FOR : " <<  id << "]\n";
	}
	return res;
}
double sim_mob::PathSetParam::getTravelTimeBySegId(const std::string &id,sim_mob::DailyTime startTime)
{
	//1. check realtime table
	double res=0.0;
	std::map<std::string,std::vector<sim_mob::LinkTravelTime> >::iterator it;
	it = segmentRealTimeTravelTimePool.find(id);
	if(it!=segmentRealTimeTravelTimePool.end())
	{
		std::vector<sim_mob::LinkTravelTime> &e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::LinkTravelTime& l = e[i];
			if( l.startTime_DT.isBeforeEqual(startTime) && l.endTime_DT.isAfter(startTime) )
			{
				res = l.travelTime;
				logger << id << "  " << startTime.getRepr_()  << "[REALTT]\n";
				return res;
			}
		}
	}
	else
	{
		//logger << "[NO REALTT] " << id << "\n";
	}
	//2. if no , check default
	it = segmentDefaultTravelTimePool.find(id);
	if(it!=segmentDefaultTravelTimePool.end())
	{
		std::vector<sim_mob::LinkTravelTime> &e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::LinkTravelTime& l = e[i];
			if( l.startTime_DT.isBeforeEqual(startTime) && l.endTime_DT.isAfter(startTime) )
			{
				res = l.travelTime;
				logger << id << "  " << startTime.getRepr_() << " [DEFTT]\n";
				return res;
			}
		}
	}
	else
	{
		logger <<  "[NOTT] " + id + "\n";
	}
	return res;
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
	bLength = -0.001025;//0.0; //negative sign proposed by milan
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
	typedef std::map<std::string,sim_mob::RoadSegment*>::value_type SPP;
	BOOST_FOREACH(SPP& segPool_pair,segPool)
	{
		sum += segPool_pair.first.length();
	}
	sum += sizeof(sim_mob::RoadSegment*) * segPool.size();

//		std::map<const sim_mob::RoadSegment*,sim_mob::WayPoint*> wpPool;//unused for now
	//std::map<std::string,sim_mob::Node*> nodePool;
	typedef std::map<std::string,sim_mob::Node*>::value_type NPP;
	logger << "nodePool.size() " << nodePool.size() << "\n";
	BOOST_FOREACH(NPP& nodePool_pair,nodePool)
	{
		sum += nodePool_pair.first.length();
	}
	sum += sizeof(sim_mob::Node*) * nodePool.size();

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
	typedef std::map<std::string,sim_mob::ERP_Section*>::value_type  ERPSP;
	BOOST_FOREACH(ERPSP&ERP_Section_pair,ERP_SectionPool)
	{
		sum += ERP_Section_pair.first.length();
	}
	sum += sizeof(sim_mob::ERP_Section*) * ERP_SectionPool.size();

//		std::map<std::string,std::vector<sim_mob::LinkTravelTime*> > segmentDefaultTravelTimePool;
	typedef std::map<std::string,std::vector<sim_mob::LinkTravelTime> >::value_type LDTTPP;
	BOOST_FOREACH(LDTTPP & ldttpp,segmentDefaultTravelTimePool)
	{
		sum += ldttpp.first.length();
		sum += sizeof(sim_mob::LinkTravelTime) * ldttpp.second.size();
	}


//		std::map<std::string,std::vector<sim_mob::LinkTravelTime*> > segmentRealTimeTravelTimePool;
	typedef std::map<std::string,std::vector<sim_mob::LinkTravelTime> >::value_type LRTTPP;
	BOOST_FOREACH(LRTTPP &lrttp,segmentRealTimeTravelTimePool)
	{
		sum += lrttp.first.length();
		sum += sizeof(sim_mob::LinkTravelTime) * lrttp.second.size();
	}

//		const roadnetwork;
	sum += sizeof(sim_mob::RoadNetwork&);

//		std::string RTTT;
	sum += RTTT.length();
	return sum;
}

sim_mob::PathSetParam::PathSetParam() :
		roadNetwork(ConfigManager::GetInstance().FullConfig().getNetwork()),
		multiNodesPool(ConfigManager::GetInstance().FullConfig().getNetwork().getNodes()), uniNodesPool(ConfigManager::GetInstance().FullConfig().getNetwork().getUniNodes()),
		RTTT("")
{
	initParameters();
	for (std::vector<sim_mob::Link *>::const_iterator it =	ConfigManager::GetInstance().FullConfig().getNetwork().getLinks().begin(), it_end( ConfigManager::GetInstance().FullConfig().getNetwork().getLinks().end()); it != it_end; it++) {
		for (std::set<sim_mob::RoadSegment *>::iterator seg_it = (*it)->getUniqueSegments().begin(), it_end((*it)->getUniqueSegments().end()); seg_it != it_end; seg_it++) {
			if (!(*seg_it)->originalDB_ID.getLogItem().empty()) {
				string aimsun_id = (*seg_it)->originalDB_ID.getLogItem();
				string seg_id = Utils::getNumberFromAimsunId(aimsun_id);
				segPool.insert(std::make_pair(seg_id, *seg_it));
			}
		}
	}
	//we are still in constructor , so const refs like roadNetwork and multiNodesPool are not ready yet.
	BOOST_FOREACH(sim_mob::Node* n, ConfigManager::GetInstance().FullConfig().getNetwork().getNodes()){
		if (!n->originalDB_ID.getLogItem().empty()) {
			std::string t = n->originalDB_ID.getLogItem();
			std::string id = sim_mob::Utils::getNumberFromAimsunId(t);
			nodePool.insert(std::make_pair(id , n));
		}
	}

	BOOST_FOREACH(sim_mob::UniNode* n, ConfigManager::GetInstance().FullConfig().getNetwork().getUniNodes()){
		if (!n->originalDB_ID.getLogItem().empty()) {
			std::string t = n->originalDB_ID.getLogItem();
			std::string id = sim_mob::Utils::getNumberFromAimsunId(t);
			nodePool.insert(std::make_pair(id, n));
		}
	}

	logger << "PathSetParam: nodes amount " <<
			ConfigManager::GetInstance().FullConfig().getNetwork().getNodes().size() +
			ConfigManager::GetInstance().FullConfig().getNetwork().getNodes().size() << "\n";
	logger << "PathSetParam: segments amount "	<<
			segPool.size() << "\n";
	getDataFromDB();
}
uint32_t sim_mob::PathSetManager::getSize(){
	uint32_t sum = 0;
	//first get the pathset param here
	sum += pathSetParam->getSize();
	sum += sizeof(StreetDirectory&);
	sum += sizeof(PathSetParam *);
	sum += sizeof(bool); // bool isUseCache;
	sum += sizeof(const sim_mob::RoadSegment*) * partialExclusions.size(); // std::set<const sim_mob::RoadSegment*> currIncidents;
	sum += sizeof(boost::shared_mutex); // boost::shared_mutex mutexPartial;
	sum += sizeof(std::string &);  // const std::string &pathSetTableName;
	sum += sizeof(const std::string &); // const std::string &singlePathTableName;
	sum += sizeof(const std::string &); // const std::string &dbFunction;
	// static std::map<boost::thread::id, boost::shared_ptr<soci::session > > cnnRepo;
	sum += (sizeof(boost::thread::id)) + sizeof(boost::shared_ptr<soci::session >) * cnnRepo.size();
	sum += sizeof(sim_mob::batched::ThreadPool *); // sim_mob::batched::ThreadPool *threadpool_;
	sum += sizeof(boost::shared_mutex); // boost::shared_mutex cachedPathSetMutex;
//	//map<const std::string,sim_mob::PathSet > cachedPathSet;
//	for(std::map<const std::string,boost::shared_ptr<sim_mob::PathSet> >::iterator it = cachedPathSet.begin(); it !=cachedPathSet.end(); it++)
////			BOOST_FOREACH(cachedPathSet_pair,cachedPathSet)
//	{
//		sum += it->first.length();
//		sum += it->second->getSize();
//	}
	sum += scenarioName.length(); // std::string scenarioName;
	// std::map<std::string ,std::vector<WayPoint> > fromto_bestPath;
	sum += sizeof(WayPoint) * fromto_bestPath.size();
	for(std::map<std::string ,std::vector<WayPoint> >::iterator it = fromto_bestPath.begin(); it != fromto_bestPath.end(); sum += it->first.length(), it++);
	for(std::set<std::string>::iterator it = tempNoPath.begin(); it != tempNoPath.end(); sum += (*it).length(), it++); // std::set<std::string> tempNoPath;
	sum += sizeof(SGPER); // SGPER pathSegments;
	sum += pathSetParam->RTTT.length(); // std::string RTTT;
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

	return sum;
}

sim_mob::PathSetManager::PathSetManager():stdir(StreetDirectory::instance()),
//		pathSetTableName(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().pathSetTableName),
		singlePathTableName(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().singlePathTableName),
		dbFunction(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().dbFunction),cacheLRU(2500),
		blacklistSegments((sim_mob::ConfigManager::GetInstance().FullConfig().CBD() ?
				RestrictedRegion::getInstance().getZoneSegments(): std::set<const sim_mob::RoadSegment*>()))//todo placeholder
{
	pathSetParam = PathSetParam::getInstance();
	std::string dbStr(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
//	// 1.2 get all segs
	cnnRepo[boost::this_thread::get_id()].reset(new soci::session(soci::postgresql,dbStr));
}

void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message){

}

sim_mob::PathSetManager::~PathSetManager()
{
}

namespace {
int pathsCnt = 0;
int spCnt = 0;
}

bool sim_mob::PathSetManager::pathInBlackList(const std::vector<WayPoint> path, const std::set<const sim_mob::RoadSegment*> & blkLst)
{
	BOOST_FOREACH(WayPoint wp, path)
	{
		if(blkLst.find(wp.roadSegment_) != blkLst.end())
		{
			return true;
		}
	}
	return false;
}

//most of thease methods with '2' suffix are prone to memory leak due to inserting duplicate singlepaths into a std::set
bool sim_mob::PathSetManager::generateAllPathSetWithTripChain2()
{
	const std::map<std::string, std::vector<sim_mob::TripChainItem*> > *tripChainPool =
			&ConfigManager::GetInstance().FullConfig().getTripChains();
	logger<<"generateAllPathSetWithTripChain: trip chain pool size "<<  tripChainPool->size() <<  "\n";
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
	}
	return res;
}

bool sim_mob::PathSetManager::insertTravelTime2TmpTable(sim_mob::LinkTravelTime& data)
{
	return processTT.insertTravelTime2TmpTable(data);
}

//bool sim_mob::PathSetManager::copyTravelTimeDataFromTmp2RealtimeTable()
//{
//	//1. prepare the csv file to be copied into DB
////	sim_mob::Logger::log("real_time_travel_time");
//	bool res=false;
//	//2.truncate/empty out the realtime travel time table
//	res = sim_mob::aimsun::Loader::truncateTable(*getSession(),	pathSetParam->RTTT);
//	if(!res)
//	{
//		return false;
//	}
//	//3.write into DB table
//	sim_mob::Logger::log("real_time_travel_time").flush();
//	sim_mob::aimsun::Loader::insertCSV2Table(*getSession(),	pathSetParam->RTTT, boost::filesystem::canonical("real_time_travel_time.txt").string());
//	return res;
//}

bool sim_mob::PathSetManager::copyTravelTimeDataFromTmp2RealtimeTable()
{
	sim_mob::Logger::log("real_time_travel_time").flush();
	return sim_mob::aimsun::Loader::upsertTravelTime(*getSession(), boost::filesystem::canonical("real_time_travel_time.txt").string(), sim_mob::PathSetParam::getInstance()->RTTT);
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

const std::pair <SGPER::const_iterator,SGPER::const_iterator > sim_mob::PathSetManager::getODbySegment(const sim_mob::RoadSegment* segment) const{
       logger << "pathSegments cache size =" <<  pathSegments.size() << "\n";
       const std::pair <SGPER::const_iterator,SGPER::const_iterator > range = pathSegments.equal_range(segment);
       return range;
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

void sim_mob::PathSetManager::inserIncidentList(const sim_mob::RoadSegment* rs) {
	boost::unique_lock<boost::shared_mutex> lock(mutexExclusion);
	partialExclusions.insert(rs);
}

const boost::shared_ptr<soci::session> & sim_mob::PathSetManager::getSession(){
	std::map<boost::thread::id, boost::shared_ptr<soci::session> >::iterator it = cnnRepo.find(boost::this_thread::get_id());
	if(it == cnnRepo.end())
	{
		std::string dbStr(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
		cnnRepo[boost::this_thread::get_id()].reset(new soci::session(soci::postgresql,dbStr));
		it = cnnRepo.find(boost::this_thread::get_id());
	}
	return it->second;
}

void sim_mob::PathSetManager::clearSinglePaths(boost::shared_ptr<sim_mob::PathSet>&ps){
	logger << "clearing " << ps->pathChoices.size() << " SinglePaths\n";
	BOOST_FOREACH(sim_mob::SinglePath* sp_, ps->pathChoices){
		if(sp_){
			safe_delete_item(sp_);
		}
	}
	ps->pathChoices.clear();
}

bool sim_mob::PathSetManager::cachePathSet(boost::shared_ptr<sim_mob::PathSet>&ps){
	return cachePathSet_LRU(ps);
}

bool sim_mob::PathSetManager::cachePathSet_LRU(boost::shared_ptr<sim_mob::PathSet>&ps){
	cacheLRU.insert(ps->id, ps);
}

//bool sim_mob::PathSetManager::cachePathSet_orig(boost::shared_ptr<sim_mob::PathSet>&ps){
//	logger << "caching [" << ps->id << "]\n";
//	//test
////	return false;
//	//first step caching policy:
//	// if cache size excedded 250 (upper threshold), reduce the size to 200 (lowe threshold)
//	if(cachedPathSet.size() > 2500)
//	{
//		logger << "clearing some of the cached PathSets\n";
//		int i = cachedPathSet.size() - 2000;
//		std::map<std::string, boost::shared_ptr<sim_mob::PathSet> >::iterator it(cachedPathSet.begin());
//		for(; i >0 && it != cachedPathSet.end(); --i )
//		{
//			cachedPathSet.erase(it++);
//		}
//	}
//	{
//		boost::unique_lock<boost::shared_mutex> lock(cachedPathSetMutex);
//		bool res = cachedPathSet.insert(std::make_pair(ps->id,ps)).second;
//		if(!res){
//			logger << "Failed to cache [" << ps->id << "]\n";
//		}
//		return res;
//	}
//}

bool sim_mob::PathSetManager::findCachedPathSet(std::string  key,
		boost::shared_ptr<sim_mob::PathSet> &value,
		const std::set<const sim_mob::RoadSegment*> & blcklist){
	bool res = findCachedPathSet_LRU(key,value);
	//expensive operation, use with cautions
	if(res && !blcklist.empty())
	{
		throw std::runtime_error("This demo shall NOT check cache for blacklist");
		//expensive operation, use with cautions
		std::set<sim_mob::SinglePath*, sim_mob::SinglePath>::iterator it(value->pathChoices.begin());
		for ( ; it != value->pathChoices.end(); ) {
		  if (pathInBlackList((*it)->path,blcklist)) {
		    value->pathChoices.erase(it++);
		  } else {
		    ++it;
		  }
		}
	}
	return res;
}

bool sim_mob::PathSetManager::findCachedPathSet(std::string  key, boost::shared_ptr<sim_mob::PathSet> &value){
	return findCachedPathSet_LRU(key,value);
}

bool sim_mob::PathSetManager::findCachedPathSet_LRU(std::string  key, boost::shared_ptr<sim_mob::PathSet> &value){
	return cacheLRU.find(key,value);
}


void sim_mob::PathSetManager::setPathSetTags(boost::shared_ptr<sim_mob::PathSet>&ps)
{

	double minTravelTime=99999999.0;
	// find MIN_TRAVEL_TIME
	// record which is min
	sim_mob::SinglePath *minSP = *(ps->pathChoices.begin());
	int i = 0;
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		/// todo getTravelTime will be called again in onPathSetRetrieval which will be rudimentary
		if(sp->travleTime <= 0.0 && (sp->travleTime = getTravelTime(sp,ps->subTrip->startTime)) <= 0.0 )
		{
			std::stringstream out("");
			out << i << "generatePathSize=> invalid single path travleTime :" << sp->travleTime;
			throw std::runtime_error(out.str());
		}
		if(ps->oriPath && sp->id == ps->oriPath->id){
			minTravelTime = sp->travleTime;
			minSP = sp;
		}
		else{
			if(sp->travleTime < minTravelTime)
			{
				minTravelTime = sp->travleTime;
				minSP = sp;
			}

		}
		i++;
	}
	if(!ps->pathChoices.empty() && minSP)
	{
		minSP->isMinTravelTime = 1;
	}
	// find MIN_DISTANCE
	double minDistance=99999999.0;
	minSP = *(ps->pathChoices.begin()); // record which is min
	if(ps->oriPath ){
		minDistance = ps->oriPath->length;
		minSP = ps->oriPath;
	}
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(sp->length < minDistance)
		{
			minDistance = sp->length;
			minSP = sp;
		}
	}
	minSP->isMinDistance = 1;

	// find MIN_SIGNAL
	int minSignal=99999999;
	minSP = *(ps->pathChoices.begin()); // record which is min
	if(ps->oriPath){
		minSignal = ps->oriPath->signalNumber;
		minSP = ps->oriPath;
	}
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(sp->signalNumber < minSignal)
		{
			minSignal = sp->signalNumber;
			minSP = sp;
		}
	}
	minSP->isMinSignal = 1;

	// find MIN_RIGHT_TURN
	int minRightTurn=99999999;
	minSP = *(ps->pathChoices.begin()); // record which is min
	if(ps->oriPath){
		minRightTurn = ps->oriPath->rightTurnNumber;
		minSP = ps->oriPath;
	}
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(sp->travleTime < minTravelTime)
		{
			minRightTurn = sp->rightTurnNumber;
			minSP = sp;
		}
	}
	minSP->isMinRightTurn = 1;

	// find MAX_HIGH_WAY_USAGE
	double maxHighWayUsage=0.0;
	minSP = *(ps->pathChoices.begin()); // record which is min
	if(ps->oriPath){
		maxHighWayUsage = ps->oriPath->highWayDistance / ps->oriPath->length;
		minSP = ps->oriPath;
	}
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(maxHighWayUsage < sp->highWayDistance / sp->length)
		{
			maxHighWayUsage = sp->highWayDistance / sp->length;
			minSP = sp;
		}
	}
	minSP->isMaxHighWayUsage = 1;

}

//bool sim_mob::PathSetManager::findCachedPathSet_orig(std::string  key, boost::shared_ptr<sim_mob::PathSet> &value){
////	//test
////	return false;
//	std::map<std::string, boost::shared_ptr<sim_mob::PathSet> >::iterator it ;
//	{
//		boost::unique_lock<boost::shared_mutex> lock(cachedPathSetMutex);
//		it = cachedPathSet.find(key);
//		if (it == cachedPathSet.end()) {
//			//debug
//			std::stringstream out("");
//			out << "Failed finding [" << key << "] in" << cachedPathSet.size() << " entries\n" ;
//			typedef std::map<std::string, boost::shared_ptr<sim_mob::PathSet> >::value_type ITEM;
//			BOOST_FOREACH(ITEM & item,cachedPathSet){
//				out << item.first << ",";
//			}
//			logger << out.str() << "\n";
//			return false;
//		}
//		value = it->second;
//	}
//	return true;
//}

std::string sim_mob::printWPpath(const std::vector<WayPoint> &wps , const sim_mob::Node* startingNode ){
	std::ostringstream out("wp path--");
	if(startingNode){
		out << startingNode->getID() << ":";
	}
	BOOST_FOREACH(WayPoint wp, wps){
		if(wp.type_ == sim_mob::WayPoint::ROAD_SEGMENT)
		{
			out << wp.roadSegment_->getSegmentAimsunId() << ",";
		}
		else if(wp.type_ == sim_mob::WayPoint::NODE)
		{
			out << wp.node_->getID() << ",";
		}
	}
	out << "\n";

	logger << out.str();
	return out.str();
}

namespace
{
sim_mob::BasicLogger & cbdLogger = sim_mob::Logger::log("CBD");
}


vector<WayPoint> sim_mob::PathSetManager::getPath(const sim_mob::Person* per,const sim_mob::SubTrip &subTrip)
{
	// get person id and current subtrip id
	std::stringstream str("");
	str << subTrip.fromLocation.node_->getID() << "," << subTrip.toLocation.node_->getID();
	std::string fromToID(str.str());
	//todo. change the subtrip signature from pointer to referencer
	logger << "+++++++++++++++++++++++++" << "\n";
	vector<WayPoint> res = vector<WayPoint>();
	//CBD area logic
	bool from = sim_mob::RestrictedRegion::getInstance().isInRestrictedZone(subTrip.fromLocation);
	bool to = sim_mob::RestrictedRegion::getInstance().isInRestrictedZone(subTrip.toLocation);
	str.str("");
	str << "[" << fromToID << "]";
	if (sim_mob::ConfigManager::GetInstance().FullConfig().CBD()) {
		if (to == false && from == false) {
			subTrip.cbdTraverseType = TravelMetric::CBD_PASS;
			str << "[BLCKLST]";
			std::stringstream outDbg("");
			getBestPath(res, &subTrip, &outDbg,
					std::set<const sim_mob::RoadSegment*>(), false, true);//use/enforce blacklist
			if (sim_mob::RestrictedRegion::getInstance().isInRestrictedSegmentZone(res)) {
				logger << "[" << fromToID << "]" << " [PERSON: "	<< per->getId() << "] " << "[PATH : " << res.size() << "]" << std::endl;
				printWPpath(res);
				logger << outDbg.str() << std::endl;
				throw std::runtime_error("\npath inside cbd ");
			}
		} else {

			if (!(to && from)) {
				subTrip.cbdTraverseType =
						(from ? TravelMetric::CBD_EXIT : TravelMetric::CBD_ENTER);
				str << (from ? " [EXIT CBD]" : "[ENTER CBD]");
			} else {
				str << "[BOTH INSIDE CBD]";
			}
			getBestPath(res, &subTrip);
		}
	} else {
		getBestPath(res, &subTrip);
	}

	//subscribe person
	logger << fromToID  << ": Path chosen for person[" << per->getId() << "]"  << "\n";
	str <<  " [PERSON: " << per->getId() << "]"  ;
	if(!res.empty())
	{
		logger << "[" << fromToID << "]" <<  " : was assigned path of size " << res.size()  << "\n";
		str << "[PATH : " << res.size()  << "]\n";
		//expensive due to call to getSegmentAimsunId()
		if(fromToID == "111502,79350" || fromToID == "93122,114990" || fromToID == "112768,93896")
		{
			printWPpath(res);
		}
	}
	else{
		logger << "[" << fromToID << "]" << " : NO PATH" << "\n";
		str << "[NO PATH]" << "\n";
	}
	logger << str.str();
	return res;
}

///	discard those entries which have segments whith their CBD flag set to true
///	return the final number of path choices
unsigned short purgeBlacklist(sim_mob::PathSet &ps)
{
	std::set<sim_mob::SinglePath*>::iterator it = ps.pathChoices.begin();
	for(;it != ps.pathChoices.end();)
	{
		bool erase = false;
		std::vector<sim_mob::WayPoint>::iterator itWp = (*it)->path.begin();
		for(; itWp != (*it)->path.end(); itWp++)
		{
			if(itWp->roadSegment_->CBD)
			{
//				logger << itWp->roadSegment_->getId() << " is in CBD" << std::endl;
				erase = true;
				break;
			}
//			else
//			{
//				logger << itWp->roadSegment_->getId() << " NOT in CBD" << std::endl;
//			}
		}
		if(erase)
		{
			if(ps.oriPath == *it)
			{
				ps.oriPath = nullptr;
			}
			SinglePath * sp = *it;
			delete sp;
			ps.pathChoices.erase(it++);
		}
		else
		{
			it++;
		}
	}
	return ps.pathChoices.size();
}

void sim_mob::PathSetManager::onPathSetRetrieval(boost::shared_ptr<PathSet> &ps, bool travelTime)
{
	//step-1 time dependent calculations
	//logger << "onPathSetRetrieval:\nGenerating Travel Time and Travel Cost for " << ps->pathChoices.size() << " paths\n" ;
	int i = 0;
	BOOST_FOREACH(SinglePath *sp, ps->pathChoices)
	{
		sp->travleTime = getTravelTime(sp,ps->subTrip->startTime);
		sp->travelCost = getTravelCost2(sp,ps->subTrip->startTime);
		//logger << i++ << " [TRAVELTIME: " << sp->travleTime << "][TRAVELCOST: " << sp->travelCost << "]\n";
	}

	//step-2 utility calculation
	BOOST_FOREACH(SinglePath *sp, ps->pathChoices)
	{
		sp->utility = generateUtility(sp);
	}
}
void sim_mob::PathSetManager::onGeneratePathSet(boost::shared_ptr<PathSet> &ps)
{
	setPathSetTags(ps);
	generatePathSize(ps);
	//partial utility calculation to save some time
	BOOST_FOREACH(SinglePath *sp, ps->pathChoices)
	{
		sp->partialUtility = generatePartialUtility(sp);
	}
}
//Operations:
//step-0: Initial preparations
//step-1: Check the cache
//step-2: If not found in cache, check DB
//Step-3: If not found in DB, generate all 4 types of path
//step-5: Choose the best path using utility function
bool sim_mob::PathSetManager::getBestPath(
		std::vector<sim_mob::WayPoint> &res,
		const sim_mob::SubTrip* st,std::stringstream *outDbg,
		std::set<const sim_mob::RoadSegment*> tempBlckLstSegs,
		 bool usePartialExclusion,
		 bool useBlackList,
		bool isUseCache)
{
	res.clear();
//	if(st->mode == "Car" || st->mode == "Taxi" || st->mode == "Motorcycle") //only driver need path set
//	{
//		return false;
//	}
	//take care of partially excluded and blacklisted segments here
	std::set<const sim_mob::RoadSegment*> blckLstSegs(tempBlckLstSegs);
	if(useBlackList && this->blacklistSegments.size())
	{
		blckLstSegs.insert(this->blacklistSegments.begin(), this->blacklistSegments.end()); //temporary + permanent
	}
	const std::set<const sim_mob::RoadSegment*> &partial = (usePartialExclusion ? this->partialExclusions : std::set<const sim_mob::RoadSegment*>());

	const sim_mob::Node* fromNode = st->fromLocation.node_;
	const sim_mob::Node* toNode = st->toLocation.node_;
	if(!(toNode && fromNode)){
		logger << "Error, OD null\n" ;
		return false;
	}
	if(toNode->getID() == fromNode->getID()){
		logger << "Error: same OD id from different objects discarded:" << toNode->getID() << "\n" ;
		return false;
	}
	std::stringstream out("");
	std::string idStrTo = toNode->originalDB_ID.getLogItem();
	std::string idStrFrom = fromNode->originalDB_ID.getLogItem();
	out << Utils::getNumberFromAimsunId(idStrFrom) << "," << Utils::getNumberFromAimsunId(idStrTo);
	std::string fromToID(out.str());
	if(tempNoPath.find(fromToID) != tempNoPath.end())
	{
		logger <<  fromToID   << "[RECORERD OF FAILURE.BYPASSING : " << fromToID << "]\n";
		return false;
	}
	logger << "[THREAD " << boost::this_thread::get_id() << "][SEARCHING FOR : " << fromToID << "]\n" ;
	boost::shared_ptr<sim_mob::PathSet> ps_;

	//Step-1 Check Cache
	sim_mob::Logger::log("ODs") << "[" << fromToID << "]" <<  "\n";
	/*
	 * supply only the temporary blacklist, because with the current implementation,
	 * cache should never be filled with paths containing permanent black listed segments
	 */
	if(isUseCache && findCachedPathSet(fromToID,ps_,(useBlackList && tempBlckLstSegs.size() ? tempBlckLstSegs : std::set<const sim_mob::RoadSegment*>())))
	{
		logger <<  fromToID  << " : Cache Hit\n";
		onPathSetRetrieval(ps_);
		//no need to supply permanent blacklist
		bool r = getBestPathChoiceFromPathSet(ps_,partial);
		logger <<  fromToID << " : getBestPathChoiceFromPathSet returned best path of size : " << ps_->bestPath->size() << "\n";
		if(r)
		{
			res = *(ps_->bestPath);
			logger <<  fromToID << " : returning a path " << res.size() << "\n";
			return true;

		}
		else{
				logger <<  fromToID  << "UNUSED Cache Hit" <<  "\n";
		}
	}
	else
	{
		logger <<  fromToID << " : Cache Miss " << "\n";
	}
	//step-2:check  DB
	sim_mob::HasPath hasPath = PSM_UNKNOWN;
	ps_.reset(new sim_mob::PathSet());
	ps_->subTrip = st;
	ps_->id = fromToID;
	ps_->scenario = scenarioName;
	hasPath = sim_mob::aimsun::Loader::loadSinglePathFromDB(*getSession(),fromToID,ps_->pathChoices, dbFunction,outDbg,blckLstSegs);
	logger  <<  fromToID << " : " << (hasPath == PSM_HASPATH ? "" : "Don't " ) << "have SinglePaths in DB \n" ;
	switch (hasPath) {
	case PSM_HASPATH: {
		cout << "[" << fromToID << "]" <<  " : DB Hit\n";
		logger << "[" << fromToID << "]" <<  " : DB Hit\n";
		bool r = false;
		ps_->oriPath = 0;
		BOOST_FOREACH(sim_mob::SinglePath* sp, ps_->pathChoices) {
			if (sp->isShortestPath) {
				ps_->oriPath = sp;
				break;
			}
		}
		if (ps_->oriPath == 0) {
			std::string str = "Warning => SP: oriPath(shortest path) for "
					+ ps_->id + " not valid anymore\n";
			logger << str;
		}
		//	no need of processing and storing blacklisted paths
		short psCnt = ps_->pathChoices.size();
		onPathSetRetrieval(ps_);
		r = getBestPathChoiceFromPathSet(ps_, partial);
		logger << "[" << fromToID << "]" <<  " :  number of paths before blcklist: " << psCnt << " after blacklist:" << ps_->pathChoices.size() << "\n";
		if (r) {
			res = *(ps_->bestPath);
			//cache
			if (isUseCache) {
				cachePathSet(ps_);
			}
			//test
//			clearSinglePaths(ps_);
			logger << "returning a path " << res.size() << "\n";
			return true;
		} else {
			logger << "UNUSED DB hit\n";
		}
		break;
	}
	case PSM_NOTFOUND: {
		logger << "[" << fromToID << "]" <<  " : DB Miss\n";
		// Step-3 : If not found in DB, generate all 4 types of path
		logger << "[GENERATING PATHSET : " << fromToID << "]\n";
		// 1. generate shortest path with all segs
		// 1.2 get all segs
		// 1.3 generate shortest path with full segs

		//just to avoid
		ps_.reset(new PathSet(fromNode, toNode));
		ps_->id = fromToID;
		ps_->scenario = scenarioName;
		ps_->subTrip = st;

		bool r = generateAllPathChoices(ps_, blckLstSegs);
		if (r)
		{
			onGeneratePathSet(ps_);
		}
		else
		{
			logger << "[PATHSET GENERATION FAILURE : " << fromToID << "]\n";
			tempNoPath.insert(fromToID);
			return false;
		}
		//this hack conforms to the CBD property added to segment and node
		if(useBlackList)
		{
			if(!purgeBlacklist(*ps_))
			{
				logger << "[ALL PATHS IN CBD" << fromToID << "]\n" ;
				tempNoPath.insert(fromToID);
				return false;
			}
		}
		logger << "[PATHSET GENERATED : " << fromToID << "]\n" ;
		onPathSetRetrieval(ps_, false);
		r = getBestPathChoiceFromPathSet(ps_, partial);
		if (r) {
			res = *(ps_->bestPath);
			//cache
			if (isUseCache)
			{
				cachePathSet(ps_);
				logger << ps_->id	<< "WARNING not cached, apparently, already in cache. this is NOT and expected behavior!!\n";
			}
			//store in into the database
			logger << "[STORE PATH: " << fromToID << "]\n";
			pathSetParam->storeSinglePath(*getSession(), ps_->pathChoices,singlePathTableName);
			logger << "[RETURN PATH OF SIZE : " << res.size() << " : " << fromToID << "]\n";
			return true;
		} else {
			logger << "[NO PATH RETURNED EVEN AFTER GENERATING A PATHSET : " << fromToID << "]\n";
			return false;
		}
		break;
	}
	case PSM_NOGOODPATH:
	default: {
		tempNoPath.insert(fromToID);
		break;
	}
	};

	logger << "[FINALLY NO RESULT :  " << fromToID << "]\n";
	return false;
}

bool sim_mob::PathSetManager::generateAllPathChoices(boost::shared_ptr<sim_mob::PathSet> &ps, const std::set<const sim_mob::RoadSegment*> & excludedSegs)
{
	logger << "generateAllPathChoices" << std::endl;
	/**
	 * step-1: find the shortest path. if not found: create an entry in the "PathSet" table and return(without adding any entry into SinglePath table)
	 * step-2: K-SHORTEST PATH
	 * step-3: SHORTEST DISTANCE LINK ELIMINATION
	 * step-4: shortest travel time link elimination
	 * step-5: TRAVEL TIME HIGHWAY BIAS
	 * step-6: Random Pertubation
	 * step-7: Some caching/bookkeeping
	 */
	std::set<std::string> duplicateChecker;//for extra optimization only(creating singlepath and discarding it later can be expensive)
	std::string fromToID(getFromToString(ps->fromNode, ps->toNode));
	sim_mob::SinglePath *s = findShortestDrivingPath(ps->fromNode,ps->toNode,duplicateChecker/*,excludedSegs*/);
	if(!s)
	{
		// no path
		if(tempNoPath.find(ps->id) == tempNoPath.end())
		{
			ps->hasPath = false;
			ps->isNeedSave2DB = true;
			tempNoPath.insert(ps->id);
		}
		return false;
	}
	s->pathSetId = fromToID;
	//some additional settings
	ps->hasPath = true;
	ps->isNeedSave2DB = true;
	ps->oriPath = s;
	ps->id = fromToID;

	//K-SHORTEST PATH
	//TODO:CONSIDER MERING THE PREVIOUS OPERATION(findShortestDrivingPath) IN THE FOLLOWING OPERATION
	std::vector< std::vector<sim_mob::WayPoint> > ksp;
	std::set<sim_mob::SinglePath*, sim_mob::SinglePath> kspTemp;
	int kspn = sim_mob::K_ShortestPathImpl::getInstance()->getKShortestPaths(ps->fromNode,ps->toNode,ksp);

	std::stringstream kspLog("");
	kspLog << "ksp-" << fromToID ;
	sim_mob::BasicLogger & kspLogger = sim_mob::Logger::log(kspLog.str());
	std::cout << "[" << fromToID << "][K-SHORTEST :" << kspn << "]\n";
	for(int i=0;i<ksp.size();++i)
	{
		std::vector<sim_mob::WayPoint> path_ = ksp[i];
		std::string id = sim_mob::makeWaypointsetString(path_);
		std::set<std::string>::iterator it_id =  duplicateChecker.find(id);
		if(it_id == duplicateChecker.end())
		{
			sim_mob::SinglePath *s = new sim_mob::SinglePath();
			// fill data
			s->isNeedSave2DB = true;
			s->id = id;
			std::cout << "[KSP:" << i << "] " << s->id << "\n";
			s->init(path_);
			s->scenario = ps->scenario;
			s->pathSize=0;
			duplicateChecker.insert(id);
			kspTemp.insert(s);
		}
	}
	// SHORTEST DISTANCE LINK ELIMINATION
	//declare the profiler  but dont start profiling. it will just accumulate the elapsed time of the profilers who are associated with the workers
	sim_mob::Link *l = NULL;
	std::vector<PathSetWorkerThread*> workPool;
	A_StarShortestPathImpl * impl = (A_StarShortestPathImpl*)stdir.getDistanceImpl();
	StreetDirectory::VertexDesc from = impl->DrivingVertex(*ps->fromNode);
	StreetDirectory::VertexDesc to = impl->DrivingVertex(*ps->toNode);
	StreetDirectory::Vertex* fromV = &from.source;
	StreetDirectory::Vertex* toV = &to.sink;
	//TODO: remove comment later
	for(int i=0;i<ps->oriPath->path.size();++i)
	{
		WayPoint *w = &(ps->oriPath->path[i]);
		if (w->type_ == WayPoint::ROAD_SEGMENT && l != w->roadSegment_->getLink()) {
			const sim_mob::RoadSegment* seg = w->roadSegment_;
			PathSetWorkerThread * work = new PathSetWorkerThread();
			//introducing the profiling time accumulator
			//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
			work->graph = &impl->drivingMap_;
			work->segmentLookup = &impl->drivingSegmentLookup_;
			work->fromVertex = fromV;
			work->toVertex = toV;
			work->fromNode = ps->fromNode;
			work->toNode = ps->toNode;
			work->excludeSeg.insert(seg);
			work->ps = ps;
			std::stringstream out("");
			out << "sdle," << i << "," << ps->fromNode->getID() << "," << ps->toNode->getID() ;
			work->dbgStr = out.str();
			threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
			workPool.push_back(work);
		} //ROAD_SEGMENT
	}

	logger  << "waiting for SHORTEST DISTANCE LINK ELIMINATION" << "\n";
	threadpool_->wait();

	// SHORTEST TRAVEL TIME LINK ELIMINATION
	l=NULL;

	//keep this line uncommented
	A_StarShortestTravelTimePathImpl * sttpImpl = (A_StarShortestTravelTimePathImpl*)stdir.getTravelTimeImpl();

	from = sttpImpl->DrivingVertexNormalTime(*ps->fromNode);
	to = sttpImpl->DrivingVertexNormalTime(*ps->toNode);
	fromV = &from.source;file:///home/fm-simmobility/vahid/simmobility/dev/Basic/tempIncident/private/simrun_basic-1.xml
	toV = &to.sink;
	SinglePath *sinPathTravelTimeDefault = generateShortestTravelTimePath(ps->fromNode,ps->toNode,duplicateChecker,sim_mob::Default);
	if(sinPathTravelTimeDefault)
	{
		for(int i=0;i<sinPathTravelTimeDefault->path.size();++i)
		{
			WayPoint *w = &(sinPathTravelTimeDefault->path[i]);
			if (w->type_ == WayPoint::ROAD_SEGMENT && l != w->roadSegment_->getLink()) {
				const sim_mob::RoadSegment* seg = w->roadSegment_;
				PathSetWorkerThread *work = new PathSetWorkerThread();
				//introducing the profiling time accumulator
				//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
				work->graph = &sttpImpl->drivingMap_Default;
				work->segmentLookup = &sttpImpl->drivingSegmentLookup_Default_;
				work->fromVertex = fromV;
				work->toVertex = toV;
				work->fromNode = ps->fromNode;
				work->toNode = ps->toNode;
				work->excludeSeg.insert(seg);
				work->ps = ps;
				std::stringstream out("");
				out << "ttle," << i << "," << ps->fromNode->getID() << "," << ps->toNode->getID() ;
				work->dbgStr = out.str();
				threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
				workPool.push_back(work);
			} //ROAD_SEGMENT
		}//for
	}//if sinPathTravelTimeDefault

	logger  << "waiting for SHORTEST TRAVEL TIME LINK ELIMINATION" << "\n";
	threadpool_->wait();


	// TRAVEL TIME HIGHWAY BIAS
	//declare the profiler  but dont start profiling. it will just accumulate the elapsed time of the profilers who are associated with the workers
	l=NULL;
	SinglePath *sinPathHightwayBias = generateShortestTravelTimePath(ps->fromNode,ps->toNode,duplicateChecker,sim_mob::HighwayBias_Distance);
	from = sttpImpl->DrivingVertexHighwayBiasDistance(*ps->fromNode);
	to = sttpImpl->DrivingVertexHighwayBiasDistance(*ps->toNode);
	fromV = &from.source;
	toV = &to.sink;
	if(sinPathHightwayBias)
	{
		for(int i=0;i<sinPathHightwayBias->path.size();++i)
		{
			WayPoint *w = &(sinPathHightwayBias->path[i]);
			if (w->type_ == WayPoint::ROAD_SEGMENT && l != w->roadSegment_->getLink()) {
				const sim_mob::RoadSegment* seg = w->roadSegment_;
				PathSetWorkerThread *work = new PathSetWorkerThread();
				//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
				//introducing the profiling time accumulator
				work->graph = &sttpImpl->drivingMap_HighwayBias_Distance;
				work->segmentLookup = &sttpImpl->drivingSegmentLookup_HighwayBias_Distance_;
				work->fromVertex = fromV;
				work->toVertex = toV;
				work->fromNode = ps->fromNode;
				work->toNode = ps->toNode;
				work->excludeSeg.insert(seg);
				work->ps = ps;
				std::stringstream out("");
				out << "highway," << i << "," << ps->fromNode->getID() << "," << ps->toNode->getID() ;
				work->dbgStr = out.str();
				threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
				workPool.push_back(work);
			} //ROAD_SEGMENT
		}//for
	}//if sinPathTravelTimeDefault
	logger  << "waiting for TRAVEL TIME HIGHWAY BIAS" << "\n";
	threadpool_->wait();

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
		work->ps = ps;
		std::stringstream out("");
		out << "random pertubation[" << i << "] '" << ps->fromNode->getID() << "," << ps->toNode->getID() << "'\n";
		work->dbgStr = out.str();
		logger << work->dbgStr;
		threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
		workPool.push_back(work);
	}
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
	ps->addOrDeleteSinglePath(ps->oriPath);
	//b. record k-shortest paths
	BOOST_FOREACH(sim_mob::SinglePath* sp, kspTemp)
	{
		ps->addOrDeleteSinglePath(sp);
	}
	//c. record the rest of the paths (link eliminations and random perturbation)
	BOOST_FOREACH(PathSetWorkerThread* p, workPool){
		if(p->hasPath){
			if(p->s->isShortestPath){
				std::string str = "Single path from pathset " + ps->id + " is not supposed to be marked as a shortest path but it is!\n" ;
				throw std::runtime_error(str);
			}
			ps->addOrDeleteSinglePath(p->s);
		}
	}

	//cleanupworkPool
	for(std::vector<PathSetWorkerThread*>::iterator wrkPoolIt=workPool.begin(); wrkPoolIt!=workPool.end(); wrkPoolIt++) {
		safe_delete_item(*wrkPoolIt);
	}
	workPool.clear();
	return true;
}

//todo this method has already been reimplemented. delete this buggy function if you learnt how to use its sub functions
vector<WayPoint> sim_mob::PathSetManager::generateBestPathChoice2(const sim_mob::SubTrip* st)
{
	vector<WayPoint> res;
	std::set<std::string> duplicateChecker;
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
#if 1
		bool hasPSinDB /*= sim_mob::aimsun::Loader::LoadOnePathSetDBwithId(
						ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
						ps_,pathSetID)*/;
#else
		bool hasPSinDB = sim_mob::aimsun::Loader::LoadOnePathSetDBwithIdST(
						*getSession(),
						ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
						ps_,pathSetID);
#endif
		if(ps_->hasPath == -1) //no path
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
				//bool hasSPinDB = sim_mob::aimsun::Loader::LoadSinglePathDBwithId2(
#else
				bool hasSPinDB = sim_mob::aimsun::Loader::loadSinglePathFromDB(
						*getSession(),
#endif
						pathSetID,
						ps_->pathChoices,dbFunction);
				if(hasSPinDB)
				{
					std::map<std::string,sim_mob::SinglePath*>::iterator it ;//= id_sp.find(ps_->singlepath_id);
					if(it!=id_sp.end())
					{
//						ps_->oriPath = id_sp[ps_->singlepath_id];
						bool r = getBestPathChoiceFromPathSet(ps_);
						if(r)
						{
							res = *ps_->bestPath;// copy better than constant twisting
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
						logger << str <<  "\n";
						return res;
					}
				}// hasSPinDB
			}
		} // hasPSinDB
		else
		{
			logger<< "gBestPC2: create data for "<< "[" << fromToID << "]" <<  "\n";
			// 1. generate shortest path with all segs
			// 1.2 get all segs
			// 1.3 generate shortest path with full segs
			sim_mob::SinglePath *s = findShortestDrivingPath(fromNode,toNode,duplicateChecker);
			if(!s)
			{
				// no path
				ps_.reset(new PathSet(fromNode,toNode));
				ps_->hasPath = -1;
				ps_->isNeedSave2DB = true;
				ps_->id = fromToID;
//				ps_->fromNodeId = fromNode->originalDB_ID.getLogItem();
//				ps_->toNodeId = toNode->originalDB_ID.getLogItem();

//				std:string temp = fromNode->originalDB_ID.getLogItem();
//				ps_->fromNodeId = sim_mob::Utils::getNumberFromAimsunId(temp);
//				temp = toNode->originalDB_ID.getLogItem();
//				ps_->toNodeId = sim_mob::Utils::getNumberFromAimsunId(temp);

				ps_->scenario = scenarioName;
				std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > tmp;
				tmp.insert(std::make_pair(fromToID,ps_));
//				sim_mob::aimsun::Loader::SaveOnePathSetData(*getSession(),tmp, pathSetTableName);
				return res;
			}
			//	// 1.31 check path pool
				// 1.4 create PathSet object
			ps_.reset(new PathSet(fromNode,toNode));
			ps_->hasPath = 1;
			ps_->subTrip = st;
			ps_->isNeedSave2DB = true;
			ps_->oriPath = s;
			ps_->id = fromToID;
			s->pathSetId = ps_->id;
			s->travelCost = getTravelCost2(s,ps_->subTrip->startTime);
			s->travleTime = getTravelTime(s,ps_->subTrip->startTime);
			ps_->pathChoices.insert(s);
				// 2. exclude each seg in shortest path, then generate new shortest path
			generatePathesByLinkElimination(s->path,duplicateChecker,ps_,fromNode,toNode);
				// generate shortest travel time path (default,morning peak,evening peak, off time)
				generateTravelTimeSinglePathes(fromNode,toNode,duplicateChecker,ps_);

				// generate k-shortest paths
				std::vector< std::vector<sim_mob::WayPoint> > kshortestPaths;
				sim_mob::K_ShortestPathImpl::getInstance()->getKShortestPaths(ps_->fromNode,ps_->toNode,kshortestPaths);
//				ps_->fromNodeId = fromNode->originalDB_ID.getLogItem();
//				ps_->toNodeId = toNode->originalDB_ID.getLogItem();

//				std::string temp = fromNode->originalDB_ID.getLogItem();
//				ps_->fromNodeId = sim_mob::Utils::getNumberFromAimsunId(temp);
//				temp = toNode->originalDB_ID.getLogItem();
//				ps_->toNodeId = sim_mob::Utils::getNumberFromAimsunId(temp);

				ps_->scenario = scenarioName;
				// 3. store pathset
				sim_mob::generatePathSize(ps_);
				std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > tmp;
				tmp.insert(std::make_pair(fromToID,ps_));
//				sim_mob::aimsun::Loader::SaveOnePathSetData(*getSession(),tmp, pathSetTableName);
				//
				bool r = getBestPathChoiceFromPathSet(ps_);
				if(r)
				{
					res = *ps_->bestPath;// copy better than constant twisting
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
//todo ps_->pathChoices.insert(s) prone to memory leak
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
			SinglePath *sinPath = findShortestDrivingPath(fromNode,toNode,duplicateChecker,seg);
			if(!sinPath)
			{
				continue;
			}
			sinPath->travelCost = getTravelCost2(sinPath,ps_->subTrip->startTime);
			sinPath->travleTime = getTravelTime(sinPath,ps_->subTrip->startTime);
			sinPath->pathSetId = ps_->id;
			ps_->pathChoices.insert(sinPath);
		}
	}//end for
}

//todo ps_->pathChoices.insert(s) prone to memory leak
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
			sinPath->travelCost = getTravelCost2(sinPath,ps_->subTrip->startTime);
			sinPath->travleTime = getTravelTime(sinPath,ps_->subTrip->startTime);
			sinPath->pathSetId = ps_->id;
//			storePath(sinPath);
			ps_->pathChoices.insert(sinPath);
		}
	}//end for
}

//todo ps_->pathChoices.insert(s) prone to memory leak
void sim_mob::PathSetManager::generateTravelTimeSinglePathes(const sim_mob::Node *fromNode,
		   const sim_mob::Node *toNode,
		   std::set<std::string>& duplicateChecker,boost::shared_ptr<sim_mob::PathSet> &ps_)
{
	SinglePath *sinPath_morningPeak = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::MorningPeak);
	if(sinPath_morningPeak)
	{
		sinPath_morningPeak->travelCost = getTravelCost2(sinPath_morningPeak,ps_->subTrip->startTime);
		sinPath_morningPeak->travleTime = getTravelTime(sinPath_morningPeak,ps_->subTrip->startTime);
		sinPath_morningPeak->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath_morningPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_morningPeak->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::MorningPeak);
	}
	SinglePath *sinPath_eveningPeak = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::EveningPeak);
	if(sinPath_eveningPeak)
	{
		sinPath_eveningPeak->travelCost = getTravelCost2(sinPath_eveningPeak,ps_->subTrip->startTime);
		sinPath_eveningPeak->travleTime = getTravelTime(sinPath_eveningPeak,ps_->subTrip->startTime);
		sinPath_eveningPeak->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath_eveningPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_eveningPeak->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::EveningPeak);
	}
	SinglePath *sinPath_offPeak = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::OffPeak);
	if(sinPath_offPeak)
	{
		sinPath_offPeak->travelCost = getTravelCost2(sinPath_offPeak,ps_->subTrip->startTime);
		sinPath_offPeak->travleTime = getTravelTime(sinPath_offPeak,ps_->subTrip->startTime);
		sinPath_offPeak->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath_offPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_offPeak->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::OffPeak);
	}
	SinglePath *sinPath_default = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::Default);
	if(sinPath_default)
	{
		sinPath_default->travelCost = getTravelCost2(sinPath_default,ps_->subTrip->startTime);
		sinPath_default->travleTime = getTravelTime(sinPath_default,ps_->subTrip->startTime);
		sinPath_default->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath_default);
		generatePathesByTravelTimeLinkElimination(sinPath_default->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::Default);
	}
	// generate high way bias path
	SinglePath *sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_Distance);
	if(sinPath)
	{
		sinPath->travelCost = getTravelCost2(sinPath,ps_->subTrip->startTime);
		sinPath->travleTime = getTravelTime(sinPath,ps_->subTrip->startTime);
		sinPath->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_Distance);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_MorningPeak);
	if(sinPath)
	{
		sinPath->travelCost = getTravelCost2(sinPath,ps_->subTrip->startTime);
		sinPath->travleTime = getTravelTime(sinPath,ps_->subTrip->startTime);
		sinPath->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_EveningPeak);
	if(sinPath)
	{
		sinPath->travelCost = getTravelCost2(sinPath,ps_->subTrip->startTime);
		sinPath->travleTime = getTravelTime(sinPath,ps_->subTrip->startTime);
		sinPath->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_OffPeak);
	if(sinPath)
	{
		sinPath->travelCost = getTravelCost2(sinPath,ps_->subTrip->startTime);
		sinPath->travleTime = getTravelTime(sinPath,ps_->subTrip->startTime);
		sinPath->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_Default);
	if(sinPath)
	{
		sinPath->travelCost = getTravelCost2(sinPath,ps_->subTrip->startTime);
		sinPath->travleTime = getTravelTime(sinPath,ps_->subTrip->startTime);
		sinPath->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath);
		//
		generatePathesByTravelTimeLinkElimination(sinPath->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_Default);
	}
	// generate random path
	for(int i=0;i<20;++i)
	{
		const sim_mob::RoadSegment *rs = NULL;
		sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::Random,rs,i);
		if(sinPath)
		{
			sinPath->travelCost = getTravelCost2(sinPath,ps_->subTrip->startTime);
			sinPath->travleTime = getTravelTime(sinPath,ps_->subTrip->startTime);
			sinPath->pathSetId = ps_->id;
			ps_->pathChoices.insert(sinPath);
		}
	}
}


std::map<long long,sim_mob::OneTimeFlag> utilityLogger;
double sim_mob::PathSetManager::generatePartialUtility(sim_mob::SinglePath* sp)
{
	double pUtility = 0;
	if(!sp)
	{
		return pUtility;
	}
	// calculate pUtility
	//1.0
	//Obtain the travel time tt of the path.
	//Obtain value of time for the agent A: bTTlowVOT/bTTmedVOT/bTThiVOT.
//	pUtility += sp->travleTime * pathSetParam->bTTVOT;
	//2.0
	//Obtain the path size PS of the path.
	//todo uncomment sanity check later
//	if(sp->pathSize > 0.0)
//	{
//		std::stringstream out("");
//		out << "generateUtility: invalid pathsize :" << sp->pathSize ;
//		throw std::runtime_error(out.str());
//	}
	pUtility += sp->pathSize * pathSetParam->bCommonFactor;
	//3.0
	//Obtain the travel distance l and the highway distance w of the path.
	pUtility += sp->length * pathSetParam->bLength + sp->highWayDistance * pathSetParam->bHighway;
	//4.0
	//Obtain the travel cost c of the path.
//	pUtility += sp->travelCost * pathSetParam->bCost;
	//5.0
	//Obtain the number of signalized intersections s of the path.
	pUtility += sp->signalNumber * pathSetParam->bSigInter;
	//6.0
	//Obtain the number of right turns f of the path.
	pUtility += sp->rightTurnNumber * pathSetParam->bLeftTurns;
	//7.0
	//min travel time param
	if(sp->isMinTravelTime == 1)
	{
		pUtility += pathSetParam->minTravelTimeParam;
	}
	//8.0
	//min distance param
	if(sp->isMinDistance == 1)
	{
		pUtility += pathSetParam->minDistanceParam;
	}
	//9.0
	//min signal param
	if(sp->isMinSignal == 1)
	{
		pUtility += pathSetParam->minSignalParam;
	}
	//10.0
	//min highway param
	if(sp->isMaxHighWayUsage == 1)
	{
		pUtility += pathSetParam->maxHighwayParam;
	}
	//Obtain the trip purpose.
	if(sp->purpose == sim_mob::work)
	{
		pUtility += sp->purpose * pathSetParam->bWork;
	}
	else if(sp->purpose == sim_mob::leisure)
	{
		pUtility += sp->purpose * pathSetParam->bLeisure;
	}

//	std::stringstream out("");
//	out << "utility_logger_" << sp->pathset_id;
//	sim_mob::Logger::log(out.str()) << "-------------------------------------------------------------------------------------------\n" <<
//			sp->pathset_id << " : UTILITY CALCULATION for pathset :" << sp->index << "\n"
//			<< "travleTime * pathSetParam->bTTVOT : " << sp->travleTime << " " << pathSetParam->bTTVOT <<  " " << sp->travleTime * pathSetParam->bTTVOT << "\n"
//			<< "sp->pathSize * pathSetParam->bCommonFactor : " << sp->pathSize << " " <<  pathSetParam->bCommonFactor <<  " " << sp->pathSize * pathSetParam->bCommonFactor<<"\n"
//			<< "sp->length * pathSetParam->bLength + sp->highWayDistance * pathSetParam->bHighway : " << sp->length << " " <<  pathSetParam->bLength  << " " <<  sp->highWayDistance  << " " <<  pathSetParam->bHighway <<  " " <<sp->length * pathSetParam->bLength + sp->highWayDistance * pathSetParam->bHighway << "\n"
//			<< "sp->travelCost * pathSetParam->bCost : " << sp->travelCost  << " " <<   pathSetParam->bCost  <<  " " <<sp->travelCost * pathSetParam->bCost << "\n"
//			<< "sp->signalNumber * pathSetParam->bSigInter : "  << sp->signalNumber  << " " <<   pathSetParam->bSigInter <<  " " << sp->signalNumber * pathSetParam->bSigInter << "\n"
//			<< "sp->rightTurnNumber * pathSetParam->bLeftTurns : " << sp->rightTurnNumber << " " << pathSetParam->bLeftTurns <<  " " << sp->rightTurnNumber * pathSetParam->bLeftTurns << "\n"
//			<< "pathSetParam->minTravelTimeParam : " << pathSetParam->minTravelTimeParam << " " <<  (sp->isMinTravelTime == 1) <<  " " <<"\n"
//			<< "pathSetParam->minDistanceParam : " <<  pathSetParam->minDistanceParam << " " <<   (sp->isMinDistance == 1) <<  " " <<"\n"
//			<< "pathSetParam->minSignalParam : " << pathSetParam->minSignalParam  << " " <<  (sp->isMinSignal == 1) <<  " " <<"\n"
//			<< "pathSetParam->maxHighwayParam  : " << pathSetParam->maxHighwayParam  << " " << (sp->isMaxHighWayUsage == 1)  <<  " " <<"\n"
//			<< "sp->purpose  * bWork/bLeisure  : " << sp->purpose  << " " <<   (sp->purpose == sim_mob::work ? pathSetParam->bWork : pathSetParam->bLeisure)  <<  " " << sp->purpose * (sp->purpose == sim_mob::work ? pathSetParam->bWork : pathSetParam->bLeisure) << "\n"
//			<< "Utility : " << pUtility << "\n";

	if(utilityLogger[sp->index].check())
	{
		sp->partialUtilityDbg << sp->pathSetId << "," << "," << sp->index << sp->travleTime << "," << pathSetParam->bTTVOT <<  ", " << sp->travleTime * pathSetParam->bTTVOT
				<< sp->pathSize << "," << pathSetParam->bCommonFactor << "," <<  sp->pathSize * pathSetParam->bCommonFactor
				<< sp->length << "," << pathSetParam->bLength << ","   <<  sp->length * pathSetParam->bLength
				<< sp->highWayDistance << ","  <<  pathSetParam->bHighway << "," << sp->highWayDistance * pathSetParam->bHighway
				<< sp->travelCost << "," <<   pathSetParam->bCost << ","  << sp->travelCost * pathSetParam->bCost
				<< sp->signalNumber << ","  <<   pathSetParam->bSigInter << "," << sp->signalNumber * pathSetParam->bSigInter
				<< sp->rightTurnNumber << "," << pathSetParam->bLeftTurns << "," << sp->rightTurnNumber * pathSetParam->bLeftTurns
				<< pathSetParam->minTravelTimeParam << "," <<  (sp->isMinTravelTime == 1) << pathSetParam->minTravelTimeParam *  (sp->isMinTravelTime == 1 ? 1 : 0)
				<< pathSetParam->minDistanceParam << "," << (sp->isMinDistance == 1) << "," << pathSetParam->minDistanceParam * (sp->isMinDistance == 1 ? 1 : 0)
				<< pathSetParam->minSignalParam  << "," << (sp->isMinSignal == 1) << "," << pathSetParam->minSignalParam * (sp->isMinSignal == 1 ? 1 : 0)
				<< pathSetParam->maxHighwayParam  << (sp->isMaxHighWayUsage == 1)  << pathSetParam->maxHighwayParam * (sp->isMaxHighWayUsage == 1 ? 1 : 0)
				<< sp->purpose << ","  << (sp->purpose == sim_mob::work ? pathSetParam->bWork : pathSetParam->bLeisure) << "," << sp->purpose  * (sp->purpose == sim_mob::work ? pathSetParam->bWork : pathSetParam->bLeisure)
				 << "," << pUtility ;
	}

	return pUtility;

}

double sim_mob::PathSetManager::generateUtility(sim_mob::SinglePath* sp)
{
	double utility=0;
	if(!sp)
	{
		return utility;
	}

	if(sp->travleTime <= 0.0)
	{
		throw std::runtime_error("generateUtility: invalid single path travleTime :");
	}

	utility = (sp->partialUtility > 0.0 ? sp->partialUtility : generatePartialUtility(sp)) ;
	// calculate utility
	//Obtain value of time for the agent A: bTTlowVOT/bTTmedVOT/bTThiVOT.
	utility += sp->travleTime * pathSetParam->bTTVOT;
	//obtain travel cost part of utility
	utility += sp->travelCost * pathSetParam->bCost;
	std::stringstream out("");
	out << "utility_csv_logger_" << sp->pathSetId << ".csv";
	sim_mob::Logger::log(out.str()) << sp->partialUtilityDbg.str() << "," << sp->travleTime << "," << pathSetParam->bTTVOT << "," << sp->travleTime * pathSetParam->bTTVOT
			<< sp->travelCost << "," <<  pathSetParam->bCost << "," <<  sp->travelCost * pathSetParam->bCost << "," << utility << "\n";
	return utility;
}

bool sim_mob::PathSetManager::getBestPathChoiceFromPathSet(boost::shared_ptr<sim_mob::PathSet> &ps,
		const std::set<const sim_mob::RoadSegment *> & partialExclusion ,
		const std::set<const sim_mob::RoadSegment*> &blckLstSegs )
{
	std::stringstream out("");
	out << "path_selection_logger" << ps->id << ".csv";
	bool computeUtility = false;
	// step 1.1 : For each path i in the path choice:
	//1. set PathSet(O, D)
	//2. travle_time
	//3. utility
	//step 1.2 : accumulate the logsum
	double maxTravelTime = std::numeric_limits<double>::max();
	ps->logsum = 0.0;
	std:ostringstream utilityDbg("");
	utilityDbg << "***********\nPATH Selection for :" << ps->id << " : \n" ;
	int iteration = 0;
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(blckLstSegs.size() && sp->includesRoadSegment(blckLstSegs))
		{
			continue;//do the same thing while measuring the probability in the loop below
		}
		if(sp->path.empty())
		{
			std::string str = iteration + " Singlepath empty";
			throw std::runtime_error (str);
		}
		//	state of the network changed
		//debug
		if(sp->travleTime <= 0.0 )
		{
			std::stringstream out("");
			out << "getBestPathChoiceFromPathSet=>invalid single path travleTime :" << sp->travleTime;
			throw std::runtime_error(out.str());
		}
		//debug..
		if (partialExclusion.size() && sp->includesRoadSegment(partialExclusion) ) {
			sp->travleTime = maxTravelTime;//some large value like infinity
			//	RE-calculate utility
			sp->utility = generateUtility(sp);
		}
		//this is done in onPathSetRetrieval so no need to repeat for now
//		sp->travleTime = getTravelTime(sp,ps->subTrip->startTime);
//		sp->travelCost = getTravelCost2(sp,ps->subTrip->startTime);
		utilityDbg << "[" << sp->utility << "," << exp(sp->utility) << "]";
		ps->logsum += exp(sp->utility);
		iteration++;
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
	utilityDbg << "\nlogsum : " << ps->logsum << "\nX : " << x << "\n";
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(blckLstSegs.size() && sp->includesRoadSegment(blckLstSegs))
		{
			continue;//do the same thing while processing the single path in the loop above
		}
		i++;
		double prob = exp(sp->utility)/(ps->logsum);
		utilityDbg << prob << " , " ;
		upperProb += prob;
		if (x <= upperProb)
		{
			// 2.3 agent A chooses path i from the path choice set.
			ps->bestPath = &(sp->path);
			logger << "[LOGIT][" << sp->pathSetId <<  "] [" << i << " out of " << ps->pathChoices.size()  << " paths chosen] [UTIL: " <<  sp->utility << "] [LOGSUM: " << ps->logsum << "][exp(sp->utility)/(ps->logsum) : " << prob << "][X:" << x << "]\n";
			utilityDbg << "upperProb reached : " << upperProb << "\n";
			utilityDbg << "***********\n";
			sim_mob::Logger::log(out.str()) << utilityDbg.str();
			return true;
		}
	}
	utilityDbg << "***********\n";
	sim_mob::Logger::log(out.str()) << utilityDbg.str();

	// path choice algorithm
	if(!ps->oriPath)//return upon null oriPath only if the condition is normal(excludedSegs is empty)
	{
		logger<< "NO PATH , getBestPathChoiceFromPathSet, shortest path empty" << "\n";
		return false;
	}
	//the last step resorts to selecting and returning shortest path(aka oripath).
	logger << "NO BEST PATH. select to shortest path\n" ;
	ps->bestPath = &(ps->oriPath->path);
	return true;
}

sim_mob::SinglePath *  sim_mob::PathSetManager::findShortestDrivingPath(
		   const sim_mob::Node *fromNode,
		   const sim_mob::Node *toNode,std::set<std::string> duplicatePath,
		   const std::set<const sim_mob::RoadSegment*> & excludedSegs)
{
	/**
	 * step-1: find the shortest driving path between the given OD
	 * step-2: turn the outputted waypoint container into a string to be used as an id
	 * step-3: create a new Single path object
	 * step-4: return the resulting singlepath object as well as add it to the container supplied through args
	 */
	sim_mob::SinglePath *s=NULL;
	std::vector<WayPoint> wp = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*fromNode), stdir.DrivingVertex(*toNode));
	if(wp.empty())
	{
		// no path debug message
		std::stringstream out("");
		if(excludedSegs.size())
		{

			const sim_mob::RoadSegment* rs;
			out << "\nWith Excluded Segments Present: \n";
			BOOST_FOREACH(rs, excludedSegs)
			{
				out <<	rs->originalDB_ID.getLogItem() << "]" << ",";
			}
		}
		logger<< "No shortest driving path for nodes[" << fromNode->originalDB_ID.getLogItem() << "] and [" <<
		toNode->originalDB_ID.getLogItem() << "]" << out.str() << "\n";
		return s;
	}
	// make sp id
	std::string id = sim_mob::makeWaypointsetString(wp);
	if(!id.size()){
		logger << "Error: Empty shortest path for OD:" <<  fromNode->getID() << "," << toNode->getID() << "\n" ;
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
		s->id = id;
		s->scenario = scenarioName;
		s->isShortestPath = true;
		duplicatePath.insert(id);
	}
	else{
		logger<<"gSPByFTNodes3:duplicate pathset discarded\n";
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
			logger<<"generateShortestTravelTimePath: no path for nodes"<<fromNode->originalDB_ID.getLogItem()<<
							toNode->originalDB_ID.getLogItem() << "\n";
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
			s->id = id;
			s->scenario = scenarioName;
			s->pathSize=0;
			duplicateChecker.insert(id);
		}
		else{
			logger<<"generateShortestTravelTimePath:duplicate pathset discarded\n";
		}

		return s;
}
void sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(sim_mob::SinglePath *sp)
{
	if(sp->path.size()<2)
	{
		sp->rightTurnNumber=0;
		sp->signalNumber=0;
		return ;
	}
	int res=0;
	int signalNumber=0;
	std::vector<WayPoint>::iterator itt=sp->path.begin();
	++itt;
	for(std::vector<WayPoint>::iterator it=sp->path.begin();it!=sp->path.end();++it)
	{
		const RoadSegment* currentSeg = it->roadSegment_;
		const RoadSegment* targetSeg = NULL;
		if(itt!=sp->path.end())
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
	sp->rightTurnNumber=res;
	sp->signalNumber=signalNumber;
}

void sim_mob::generatePathSize(boost::shared_ptr<sim_mob::PathSet>&ps,bool isUseCache)
{
	//sanity check
	if(ps->pathChoices.empty())
	{
		throw std::runtime_error("Cannot generate path size for an empty pathset");
	}
	double minL = 0;
	// Step 1: the length of each path in the path choice set
//	if(ps->oriPath)
//	{
//		minL = ps->oriPath->length;
//	}
//	else
//	{
//		sim_mob::SinglePath *sp = findShortestPath(ps->pathChoices);
//		if(sp)
//		{
//			minL = sp->length;
//		}
//		else
//		{
//			throw std::runtime_error("No Path Found for Length extraction");
//		}
//	}
//	std::cout << "min path:" << minL << std::endl;

	int dbg_ind = -1;
	bool uniquePath;
	//pathsize
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
//		std::cout << "..................................................\n";
//		std::cout << "Analyzing path: " << sp->id << std::endl;
//		std::cout << "Path length: " << sp->length << std::endl;
		dbg_ind ++;
		uniquePath = true; //this variable checks if a path has No common segments with the rest of the pathset
		//Set size = 0.
		double size=0.0;

		double pathLengthChecker=0.0;

		if(!sp)
		{
			throw std::runtime_error ("unexpected null singlepath");
		}
		if(sp->path.empty())
		{
			throw std::runtime_error ("unexpected empty shortestWayPointpath in singlepath");
		}
		// For each link a in the path:
//		for(std::set<const RoadSegment*>::iterator it1 = sp->shortestSegPath.begin();it1 != sp->shortestSegPath.end(); it1++)
		for(std::vector<WayPoint>::iterator it1=sp->path.begin(); it1!=sp->path.end(); ++it1)
		{
			const sim_mob::RoadSegment* seg = it1->roadSegment_;
			sim_mob::SinglePath* minSp = findShortestPath(ps->pathChoices, seg);
			if(minSp == nullptr)
			{
				std::stringstream out("");
				out << "couldn't find a min path for segment " << seg->getId();
				throw std::runtime_error(out.str());
			}
			minL = minSp->length;
//			std::cout << "target segment : " << seg->getId() << " minL:" << minL << std::endl;
			double l=seg->length/100.0;

			pathLengthChecker += l;
			double sum=0.0;
			//For each path j in the path choice set PathSet(O, D):
			BOOST_FOREACH(sim_mob::SinglePath* spj, ps->pathChoices)
			{
				std::vector<WayPoint>::iterator itt2;
				for(itt2 = spj->path.begin(); itt2 != spj->path.end() && itt2->roadSegment_ != seg; ++itt2);
				if(itt2 != spj->path.end())
				{
					//found a match
					//sum += minL/(spj->length);

					sum += minL/(spj->length);
					//std::cout << "\nupdate sum by minL/(spj->length) : " << minL << " / " << spj->length << " = " << minL/(spj->length) << std::endl;

					// Using formula from Ramming's thesis: Li/Lj
					//sum += sp->length/spj->length;
					//uniquness: if any part of the path is common with any other path, it is not a unique path
					if(sp->id != spj->id)
					{
						uniquePath = false;
//						std::cout << "Path i is not a unique path because we found an overlap with path j" << std::endl;
					}
					//printWPpath(spj->path);
//					std::cout << "New sum = " << sum << " ";
				}
				else
				{
					//std::cout << " O ";
				}

			} // for j
			size += l/sp->length/sum; //
//			std::cout << "\nupdate size by l/sp->length/sum : " << l << " / " << sp->length << " / " << sum << "=>" << l/sp->length/sum << std::endl;
//			std::cout << "New size = " << size << std::endl;
		}
		//is this a unique path ?
		if(uniquePath)
		{
//			std::cout << "Unique path\n";
			sp->pathSize = 0;
		}
		else
		{
			//calculate path size
			sp->pathSize = log(size);
		}
//		std::cout << "size: " << size << "  sp->pathSize : " << sp->pathSize << "\n";

//		std::cout << "pathLengthChecker = " << pathLengthChecker << " path length = " << sp->length << std::endl;
		//todo temporarily disabling the sanity check
//		if(sp->pathSize > 0.0 || (sp->pathSize == 0.0 && !uniquePath))
//		{
//			throw std::runtime_error("sp->pathSize(log(size)) is > 0 ");
//		}
	}// end for
	std::cout << "-----------------------------------\n";
}


double sim_mob::PathSetManager::getTravelTime(sim_mob::SinglePath *sp,sim_mob::DailyTime startTime)
{
	std::stringstream out("");
	out << "\n";
	double ts=0.0;
	for(int i=0;i<sp->path.size();++i)
	{
		out << i << " " ;
		if(sp->path[i].type_ == WayPoint::ROAD_SEGMENT){
			std::string seg_id = sp->path[i].roadSegment_->originalDB_ID.getLogItem();
			double t = sim_mob::PathSetParam::getInstance()->getTravelTimeBySegId(seg_id,startTime);
			ts += t;
			startTime = startTime + sim_mob::DailyTime(t*1000);
			out << t << " " << ts << startTime.getRepr_() ;
		}
	}
	if (ts <=0.0)
	{
		throw std::runtime_error(out.str());
	}
	//logger << "Retrurn TT = " << ts << out.str() << "\n" ;
	return ts;
}

void sim_mob::SinglePath::init(std::vector<WayPoint>& wpPools)
{
	//step-1 fill in the path
	filterOutNodes(wpPools, this->path);
	if(this->path.empty())
	{
	   std::string err = "empty path for OD:" + this->pathSetId + "--"  + this->id;
	   throw std::runtime_error(err);
	}

	//step-2 right/left turn
	sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(this);
	//step-3 highway distance
	highWayDistance = sim_mob::calculateHighWayDistance(this);
	//step-4 length
	length = sim_mob::generateSinglePathLength(path);
}

void sim_mob::SinglePath::clear()
{
	path.clear();
//	shortestSegPath.clear();
	id="";
	pathSetId="";
	utility = 0.0;
	pathSize = 0.0;
	travelCost=0.0;
	signalNumber=0.0;
	rightTurnNumber=0.0;
	length=0.0;
	travleTime=0.0;
	highWayDistance=0.0;
	isMinTravelTime=0;
	isMinDistance=0;
	isMinSignal=0;
	isMinRightTurn=0;
	isMaxHighWayUsage=0;
}
uint32_t sim_mob::SinglePath::getSize(){

	uint32_t sum = 0;
	sum += sizeof(WayPoint) * path.size(); // std::vector<WayPoint> shortestWayPointpath;
//	sum += sizeof(const RoadSegment*) * shortestSegPath.size(); // std::set<const RoadSegment*> shortestSegPath;
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
	sum += pathSetId.length(); // std::string pathset_id;
	sum += sizeof(double); // double utility;
	sum += sizeof(double); // double pathsize;
	sum += sizeof(double); // double travel_cost;
	sum += sizeof(int); // int signalNumber;
	sum += sizeof(int); // int rightTurnNumber;
	sum += scenario.length(); // std::string scenario;
	sum += sizeof(double); // double length;
	sum += sizeof(double); // double travle_time;
	sum += sizeof(sim_mob::TRIP_PURPOSE); // sim_mob::TRIP_PURPOSE purpose;
	logger << "SinglePath size bytes:" << sum << "\n" ;
	return sum;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
sim_mob::PathSet::~PathSet()
{
	fromNode = NULL;
	toNode = NULL;
	subTrip = NULL;
	//logger << "[DELET PATHSET " << id << "] [" << pathChoices.size() << "  SINGLEPATH]" << std::endl;
	BOOST_FOREACH(sim_mob::SinglePath*sp,pathChoices)
	{
		safe_delete_item(sp);
	}
}

uint32_t sim_mob::PathSet::getSize(){
	uint32_t sum = 0;
		sum += sizeof(bool);// isInit;
		sum += sizeof(bool);//bool hasBestChoice;
		sum += sizeof(WayPoint) * (bestPath ? bestPath->size() : 0);//std::vector<WayPoint> bestWayPointpath;  //best choice
		sum += sizeof(const sim_mob::Node *);//const sim_mob::Node *fromNode;
		sum += sizeof(const sim_mob::Node *);//const sim_mob::Node *toNode;
		sum += sizeof(SinglePath*);//SinglePath* oriPath;  // shortest path with all segments
		//std::map<std::string,sim_mob::SinglePath*> SinglePathPool;//unused so far
//		typedef std::map<std::string,sim_mob::SinglePath*>::value_type tt;
//		BOOST_FOREACH(tt & pair_,SinglePathPool)
//		{
//			sum += pair_.first.length();
//			sum += sizeof(pair_.second);
//		}
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
//		sum += fromNodeId.length();//std::string fromNodeId;
//		sum += toNodeId.length();//std::string toNodeId;
		sum += excludedPaths.length();//std::string excludedPaths;
		sum += scenario.length();//std::string scenario;
		sum += sizeof(int);//int hasPath;
		sum += sizeof(PathSetManager *);//PathSetManager *psMgr;
		logger << "pathset_cached_bytes :" << sum << "\n" ;
		return sum;
}

bool sim_mob::PathSet::includesRoadSegment(const std::set<const sim_mob::RoadSegment*> & segs)
{
	BOOST_FOREACH(sim_mob::SinglePath *sp, pathChoices)
	{
		BOOST_FOREACH(sim_mob::WayPoint &wp, sp->path)
		{
			BOOST_FOREACH(const sim_mob::RoadSegment* seg, segs)
			{
				if(wp.roadSegment_ == seg)
				{
					return true;
				}
			}
		}
	}
	return false;
}


void sim_mob::PathSet::excludeRoadSegment(const std::set<const sim_mob::RoadSegment*> & segs)
{
	std::set<sim_mob::SinglePath*>::iterator it(pathChoices.begin());
	for(; it != pathChoices.end();)
	{
		if((*it)->includesRoadSegment(segs))
		{
			pathChoices.erase(it++);
		}
		else
		{
			++it;
		}
	}
}

void sim_mob::PathSet::addOrDeleteSinglePath(sim_mob::SinglePath* s)
{
	if(!s)
	{
		return;
	}
	if(!pathChoices.insert(s).second)
	{
		safe_delete_item(s);
	}
}

sim_mob::ERP_Section::ERP_Section(ERP_Section &src)
	: section_id(src.section_id),ERP_Gantry_No(src.ERP_Gantry_No),
	  ERP_GantryNoStr(boost::lexical_cast<std::string>(src.ERP_Gantry_No))
{
	originalSectionDB_ID.setProps("aimsun-id",src.section_id);
}

sim_mob::LinkTravelTime::LinkTravelTime(const LinkTravelTime& src)
	: linkId(src.linkId),
			startTime(src.startTime),endTime(src.endTime),travelTime(src.travelTime),
			startTime_DT(sim_mob::DailyTime(src.startTime)),endTime_DT(sim_mob::DailyTime(src.endTime))
{
	originalSectionDB_ID.setProps("aimsun-id",src.linkId);
}

sim_mob::SinglePath::SinglePath() : purpose(work),utility(0.0),pathSize(0.0),travelCost(0.0),
signalNumber(0.0),rightTurnNumber(0.0),length(0.0),travleTime(0.0),highWayDistance(0.0),valid_path(true),
isMinTravelTime(0),isMinDistance(0),isMinSignal(0),isMinRightTurn(0),isMaxHighWayUsage(0),
isShortestPath(0), excludeSeg(nullptr),index(-1),path(std::vector<WayPoint>()),isNeedSave2DB(false){
}

sim_mob::SinglePath::SinglePath(const SinglePath& source) :
		id(source.id),
		utility(source.utility),pathSize(source.pathSize),
		travelCost(source.travelCost),valid_path(source.valid_path),
		signalNumber(source.signalNumber),
		rightTurnNumber(source.rightTurnNumber),
		length(source.length),travleTime(source.travleTime),
		pathSetId(source.pathSetId),highWayDistance(source.highWayDistance),
		isMinTravelTime(source.isMinTravelTime),isMinDistance(source.isMinDistance),isMinSignal(source.isMinSignal),
		isMinRightTurn(source.isMinRightTurn),isMaxHighWayUsage(source.isMaxHighWayUsage),isShortestPath(source.isShortestPath)
{
	isNeedSave2DB=false;

	purpose = sim_mob::work;
}

sim_mob::SinglePath::~SinglePath(){
	clear();
}

bool sim_mob::SinglePath::includesRoadSegment(const std::set<const sim_mob::RoadSegment*> & segs, bool dbg, std::stringstream *out){
	if(!this->path.size())
	{
		int i = 0;
	}
	BOOST_FOREACH(sim_mob::WayPoint &wp, this->path){
		BOOST_FOREACH(const sim_mob::RoadSegment* seg, segs){
			if(dbg){
				*out << "checking " << wp.roadSegment_->getId() << " against " <<  seg->getId() << "\n";
			}
			std::stringstream hack1(""),hack2("");
			hack1 << wp.roadSegment_->getId();
			hack2 << seg->getId();
			if(hack1.str() == hack2.str() ){
				return true;
			}
		}
	}
	return false;
}
int ProcessTT::dbg_ProcessTT_cnt = 0;
ProcessTT::ProcessTT():interval(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().interval * 1000)
,currRTTT(RTTT_Map.end()){}

std::map<ProcessTT::TR,ProcessTT::TT >::iterator & ProcessTT::getCurrRTTT(const DailyTime & recordTime)
{
	/*
	 * check to see if recordTime is within the current time range or it has gone past the current range.
	 * if it lies within the current range, just return the current range.
	 * else(the latter case), just create a new time range and set currRTTT to it.
	 */

	/*
	 * create a key which is nothing but two string representations of DailyTime objects
	 * these DailyTime objects denote start and end of the range
	 */
	const DailyTime & simStart = ConfigManager::GetInstance().FullConfig().simStartTime();
	TR keyBegin(recordTime.getValue() - (recordTime.getValue() % interval));//lower bound of the range
	TR key(keyBegin + interval);											//upper bound of the range
	currRTTT = RTTT_Map.find(key);
	//new Key?
	if(RTTT_Map.end() == currRTTT)
	{
		//now insert this key into the RTTT_Map. the value of this key is empty(default) as nothing has been received for this time range yet.
		currRTTT = RTTT_Map.insert(std::make_pair(key, TT())).first;
	}
	return currRTTT;
}

bool ProcessTT::insertTravelTime2TmpTable(std::map<TR,TT>::iterator prevRTTT)
{
	const TR & timeRange = prevRTTT->first;
	TT & travelTimes = prevRTTT->second;
	typedef TT::value_type TTPs;//travel time pairs
	BOOST_FOREACH(TTPs &pair, travelTimes)
	{
		int segmentId = pair.first;
		double totalTT_ForThisSeg = pair.second.first;
		int totalTT_Submissions = pair.second.second;
		double travelTime = totalTT_ForThisSeg / totalTT_Submissions;
		//now simply write it to the file
		dbg_ProcessTT_cnt++;
		sim_mob::Logger::log("real_time_travel_time") << segmentId << ";" << DailyTime(timeRange - interval).getRepr_() << ";" << DailyTime(timeRange).getRepr_() << ";" << travelTime << "\n";
	}
}

bool ProcessTT::insertTravelTime2TmpTable(sim_mob::LinkTravelTime& data)
{
	bool res=false;
	std::map<TR,TT>::iterator prevRTTT = currRTTT;
	currRTTT = getCurrRTTT(data.recordTime_DT);
	//strep-1 : Add into currRTTT
	TT & currTravelTimes = currRTTT->second;
	std::pair<double,int> &segTT_Info = currTravelTimes[data.linkId];//specific segment
	segTT_Info.first += data.travelTime;//ad to total travel times collected for this segment
	segTT_Info.second ++;//increment number of submissions

	//step-2: If a new time range has reached, write the previous range to the file
	//since the data structure is a bit complex, we break it to several variables
	if(prevRTTT != currRTTT && prevRTTT != RTTT_Map.end() )
	{
		insertTravelTime2TmpTable(prevRTTT);
	}
	return res;
}

ProcessTT::~ProcessTT()
{
	//the aggregated travel time for the last time range
	//was never wriiten. do it now:
	insertTravelTime2TmpTable(currRTTT);
}
