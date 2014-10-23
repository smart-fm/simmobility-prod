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
		logger.prof("PSP_Inst").tick();
		instance_ = new PathSetParam();
		logger.prof("PSP_Inst").tick(true);
	}
	return instance_;
}

void sim_mob::PathSetParam::getDataFromDB()
{
	setTravleTimeTmpTableName(ConfigManager::GetInstance().FullConfig().getTravelTimeTmpTableName());
		//createTravelTimeTmpTable();

		sim_mob::aimsun::Loader::LoadERPData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
				ERP_SurchargePool,	ERP_Gantry_ZonePool,ERP_SectionPool);
		logger <<
				"ERP data retrieved from database[ERP_SurchargePool,ERP_Gantry_ZonePool,ERP_Section_pool]: " <<
				ERP_SurchargePool.size() << " "  << ERP_Gantry_ZonePool.size() << " " << ERP_SectionPool.size() << "\n";

		sim_mob::aimsun::Loader::LoadDefaultTravelTimeData(*(PathSetManager::getSession()), segmentDefaultTravelTimePool);
		logger << segmentDefaultTravelTimePool.size() << " records for Link_default_travel_time found\n";

		bool res = sim_mob::aimsun::Loader::LoadRealTimeTravelTimeData(*(PathSetManager::getSession()),
				pathSetTravelTimeRealTimeTableName,	segmentRealTimeTravelTimePool);
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
	std::string createTableStr = pathSetTravelTimeTmpTableName + " ( \"link_id\" integer NOT NULL,\"start_time\" time without time zone NOT NULL,\"end_time\" time without time zone NOT NULL,\"travel_time\" double precision )";
	res = sim_mob::aimsun::Loader::createTable(*(PathSetManager::getSession()),createTableStr);
	return res;
}

bool sim_mob::PathSetParam::dropTravelTimeTmpTable()
{
	bool res=false;
	//drop tmp table
	std::string dropTableStr = "drop table \""+ pathSetTravelTimeTmpTableName +"\" ";
	res = sim_mob::aimsun::Loader::excuString(*(PathSetManager::getSession()),dropTableStr);
	return res;
}

bool sim_mob::PathSetParam::createTravelTimeRealtimeTable()
{
	bool res=false;
	std::string createTableStr = pathSetTravelTimeRealTimeTableName + " ( \"link_id\" integer NOT NULL,\"start_time\" time without time zone NOT NULL,\"end_time\" time without time zone NOT NULL,\"travel_time\" double precision )";
	res = sim_mob::aimsun::Loader::createTable(*(PathSetManager::getSession()),createTableStr);
	return res;
}

void sim_mob::PathSetParam::setTravleTimeTmpTableName(const std::string& value)
{
	if(!value.size())
	{
		throw std::runtime_error("Missing Travel Time Table Name.\n "
				"It is either missing in the XML configuration file,\n"
				"or you are trying to access the file name before reading the Configuration file");
	}
	pathSetTravelTimeTmpTableName = value + "_" + "traveltime_tmp"; // each user only has fix tmp table name
	logger << "setTravleTimeTmpTableName: " << pathSetTravelTimeTmpTableName << "\n";
	pathSetTravelTimeRealTimeTableName = value+"_travel_time";
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
		logger << "getAverageTravelTimeBySegIdStartEndTime=> no travel time for segment " << id << "\n";
	}
	return res;
}

double sim_mob::PathSetParam::getDefaultTravelTimeBySegId(std::string id)
{
	double res=0.0;
	double totalTravelTime=0.0;
	int count=0;
	std::map<std::string,std::vector<sim_mob::LinkTravelTime> >::iterator it =
			segmentDefaultTravelTimePool.find(id);
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
		std::string str = "getDefaultTravelTimeBySegId: no default travel time for segment " + id;
		logger << "error: "<< str << "\n";
	}
	return res;
}
double sim_mob::PathSetParam::getTravelTimeBySegId(std::string id,sim_mob::DailyTime startTime)
{
	//1. check realtime table
	double res=0.0;
	std::map<std::string,std::vector<sim_mob::LinkTravelTime> >::iterator it =
			segmentRealTimeTravelTimePool.find(id);
	if(it!=segmentRealTimeTravelTimePool.end())
	{
		std::vector<sim_mob::LinkTravelTime> &e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::LinkTravelTime& l = e[i];
			if( l.startTime_DT.isBeforeEqual(startTime) && l.endTime_DT.isAfter(startTime) )
			{
				res = l.travelTime;
				return res;
			}
		}
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
				return res;
			}
		}
	}
	else
	{
		logger <<  "Error :PathSetParam::getTravelTimeBySegId=> no travel time for segment " + id + "\n";
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
	logger.prof("segPool_size", false).addUp(segPool.size());
	BOOST_FOREACH(SPP& segPool_pair,segPool)
	{
		sum += segPool_pair.first.length();
	}
	sum += sizeof(sim_mob::RoadSegment*) * segPool.size();

//		std::map<const sim_mob::RoadSegment*,sim_mob::WayPoint*> wpPool;//unused for now
	//std::map<std::string,sim_mob::Node*> nodePool;
	typedef std::map<std::string,sim_mob::Node*>::value_type NPP;
	logger << "nodePool.size() " << nodePool.size() << "\n";
	logger.prof("nodePool_size", false).addUp(nodePool.size());
	BOOST_FOREACH(NPP& nodePool_pair,nodePool)
	{
		sum += nodePool_pair.first.length();
	}
	sum += sizeof(sim_mob::Node*) * nodePool.size();

//		const std::vector<sim_mob::MultiNode*>  &multiNodesPool;
	logger.prof("MnodePool_size", false).addUp(multiNodesPool.size());
	sum += sizeof(sim_mob::MultiNode*) * multiNodesPool.size();

//		const std::set<sim_mob::UniNode*> & uniNodesPool;
	logger.prof("UnodePool_size", false).addUp(uniNodesPool.size());
	sum += sizeof(sim_mob::UniNode*) * uniNodesPool.size();

//		std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> > ERP_SurchargePool;
	typedef std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> >::value_type ERPSCP;
	logger.prof("ERP_Surcharge_pool_size", false).addUp(ERP_SurchargePool.size());
	BOOST_FOREACH(ERPSCP & ERP_Surcharge_pool_pair,ERP_SurchargePool)
	{
		sum += ERP_Surcharge_pool_pair.first.length();
		sum += sizeof(sim_mob::ERP_Surcharge*) * ERP_Surcharge_pool_pair.second.size();
	}

//		std::map<std::string,sim_mob::ERP_Gantry_Zone*> ERP_Gantry_ZonePool;
	typedef std::map<std::string,sim_mob::ERP_Gantry_Zone*>::value_type ERPGZP;
	logger.prof("ERP_Gantry_Zone_pool_size", false).addUp(ERP_Gantry_ZonePool.size());
	BOOST_FOREACH(ERPGZP & ERP_Gantry_Zone_pool_pair,ERP_Gantry_ZonePool)
	{
		sum += ERP_Gantry_Zone_pool_pair.first.length();
	}
	sum += sizeof(sim_mob::ERP_Gantry_Zone*) * ERP_Gantry_ZonePool.size();

//		std::map<std::string,sim_mob::ERP_Section*> ERP_Section_pool;
	typedef std::map<std::string,sim_mob::ERP_Section*>::value_type  ERPSP;
	logger.prof("ERP_Section_size", false).addUp(ERP_SectionPool.size());
	BOOST_FOREACH(ERPSP&ERP_Section_pair,ERP_SectionPool)
	{
		sum += ERP_Section_pair.first.length();
	}
	sum += sizeof(sim_mob::ERP_Section*) * ERP_SectionPool.size();

//		std::map<std::string,std::vector<sim_mob::LinkTravelTime*> > segmentDefaultTravelTimePool;
	typedef std::map<std::string,std::vector<sim_mob::LinkTravelTime> >::value_type LDTTPP;
	logger.prof("Link_default_travel_time_pool_size", false).addUp(segmentDefaultTravelTimePool.size());
	BOOST_FOREACH(LDTTPP & ldttpp,segmentDefaultTravelTimePool)
	{
		sum += ldttpp.first.length();
		sum += sizeof(sim_mob::LinkTravelTime) * ldttpp.second.size();
	}


//		std::map<std::string,std::vector<sim_mob::LinkTravelTime*> > segmentRealTimeTravelTimePool;
	typedef std::map<std::string,std::vector<sim_mob::LinkTravelTime> >::value_type LRTTPP;
	logger.prof("Link_realtime_travel_time_pool_size", false).addUp(segmentRealTimeTravelTimePool.size());
	BOOST_FOREACH(LRTTPP &lrttp,segmentRealTimeTravelTimePool)
	{
		sum += lrttp.first.length();
		sum += sizeof(sim_mob::LinkTravelTime) * lrttp.second.size();
	}

//		const roadnetwork;
	sum += sizeof(sim_mob::RoadNetwork&);

//		std::string pathSetTravelTimeRealTimeTableName;
	sum += pathSetTravelTimeRealTimeTableName.length();
//		std::string pathSetTravelTimeTmpTableName;
	sum += pathSetTravelTimeTmpTableName.length();
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
	logger.prof("PathSetParam_getDataFromDB_time").tick();
	getDataFromDB();
	logger.prof("PathSetParam_getDataFromDB_time").tick(true);
	logger.profileMsg("Time taken for PathSetParam::getDataFromDB ", logger.prof("PathSetParam_getDataFromDB_time").getAddUp());
	logger << "Time taken for PathSetParam::getDataFromDB   " << logger.prof("PathSetParam_getDataFromDB_time").getAddUp() << "\n";
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
	sum += sizeof(SGPER); // SGPER pathSegments;
	//todo statics added separately
//	sum += csvFileName.length(); // std::string csvFileName;
	sum += sizeof(std::ofstream); // std::ofstream csvFile;
	sum += pathSetParam->pathSetTravelTimeRealTimeTableName.length(); // std::string pathSetTravelTimeRealTimeTableName;
	sum += pathSetParam->pathSetTravelTimeTmpTableName.length(); // std::string pathSetTravelTimeTmpTableName;
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
		pathSetTableName(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().pathSetTableName),
		singlePathTableName(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().singlePathTableName),
		dbFunction(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().dbFunction),cacheLRU(2500)
{
	pathSetParam = PathSetParam::getInstance();
	std::string dbStr(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
//	// 1.2 get all segs
	init();
	cnnRepo[boost::this_thread::get_id()].reset(new soci::session(soci::postgresql,dbStr));
}

void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message){

}

sim_mob::PathSetManager::~PathSetManager()
{
}

void sim_mob::PathSetManager::init()
{
	//setCSVFileName();
	initParameters();
}

namespace {
int pathsCnt = 0;
int spCnt = 0;
}

void sim_mob::PathSetManager::clearCachedPathSet()
{
	cachedPathSet.clear();
}

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

void sim_mob::PathSetManager::setCSVFileName()
{
	//get current working directory
	char the_path[1024];
	getcwd(the_path, 1023);
	printf("current dir: %s \n",the_path);
	std::string currentPath(the_path);
	std::string currentPathTmp = currentPath + "/tmp_"+pathSetParam->pathSetTravelTimeTmpTableName;
	std::string cmd = "mkdir -p "+currentPathTmp;
	boost::filesystem::path dir(currentPathTmp);
	if (boost::filesystem::create_directory(dir))
	{
		csvFileName = currentPathTmp+"/"+pathSetParam->pathSetTravelTimeTmpTableName + ".csv";
	}
	else
	{
		csvFileName = currentPath+"/"+pathSetParam->pathSetTravelTimeTmpTableName + ".csv";
	}
	struct timeval tv;
	gettimeofday(&tv, NULL);
	csvFileName += boost::lexical_cast<string>(pthread_self()) +"_"+ boost::lexical_cast<string>(tv.tv_usec);
	logger<<"csvFileName: " << csvFileName << "\n";
	csvFile.open(csvFileName.c_str());
}

bool sim_mob::PathSetManager::insertTravelTime2TmpTable(sim_mob::LinkTravelTime& data)
{
	bool res=false;
	if(ConfigManager::GetInstance().FullConfig().PathSetMode()){
		sim_mob::Logger::log("real_time_travel_time") << data.linkId << ";" << data.startTime << ";" << data.endTime << ";" << data.travelTime << "\n";
	}
	return res;
}

bool sim_mob::PathSetManager::copyTravelTimeDataFromTmp2RealtimeTable()
{
	//1. prepare the csv file to be copied into DB
	sim_mob::Logger::log("real_time_travel_time").flushLog();
	bool res=false;
	//2.truncate/empty out the realtime travel time table
	res = sim_mob::aimsun::Loader::truncateTable(*getSession(),	pathSetParam->pathSetTravelTimeRealTimeTableName);
	if(!res)
	{
		return false;
	}
	//3.write into DB table
	sim_mob::aimsun::Loader::insertCSV2Table(*getSession(),	pathSetParam->pathSetTravelTimeRealTimeTableName, boost::filesystem::canonical("real_time_travel_time.txt").string());
	return res;
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
	BOOST_FOREACH(WayPoint &wp, wps){
		pathSegments.insert(std::make_pair(wp.roadSegment_, per));
	}
}

const std::pair <SGPER::const_iterator,SGPER::const_iterator > sim_mob::PathSetManager::getODbySegment(const sim_mob::RoadSegment* segment) const{
	logger << "pathSegments cache size =" <<  pathSegments.size() << "\n";
	const std::pair <SGPER::const_iterator,SGPER::const_iterator > range = pathSegments.equal_range(segment);
	return range;
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

bool sim_mob::PathSetManager::cachePathSet_orig(boost::shared_ptr<sim_mob::PathSet>&ps){
	logger << "caching [" << ps->id << "]\n";
	//test
//	return false;
	//first step caching policy:
	// if cache size excedded 250 (upper threshold), reduce the size to 200 (lowe threshold)
	if(cachedPathSet.size() > 2500)
	{
		logger << "clearing some of the cached PathSets\n";
		int i = cachedPathSet.size() - 2000;
		std::map<std::string, boost::shared_ptr<sim_mob::PathSet> >::iterator it(cachedPathSet.begin());
		for(; i >0 && it != cachedPathSet.end(); --i )
		{
			cachedPathSet.erase(it++);
		}
	}
	{
		boost::unique_lock<boost::shared_mutex> lock(cachedPathSetMutex);
		bool res = cachedPathSet.insert(std::make_pair(ps->id,ps)).second;
		if(!res){
			logger << "Failed to cache [" << ps->id << "]\n";
		}
		return res;
	}
}

bool sim_mob::PathSetManager::findCachedPathSet(std::string  key, boost::shared_ptr<sim_mob::PathSet> &value){
	return findCachedPathSet_LRU(key,value);
}

bool sim_mob::PathSetManager::findCachedPathSet_LRU(std::string  key, boost::shared_ptr<sim_mob::PathSet> &value){
	return cacheLRU.find(key,value);
}

bool sim_mob::PathSetManager::findCachedPathSet_orig(std::string  key, boost::shared_ptr<sim_mob::PathSet> &value){
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
			typedef std::map<std::string, boost::shared_ptr<sim_mob::PathSet> >::value_type ITEM;
			BOOST_FOREACH(ITEM & item,cachedPathSet){
				out << item.first << ",";
			}
			logger << out.str() << "\n";
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
	out << "\n";

	logger << out.str();
}

vector<WayPoint> sim_mob::PathSetManager::getPath(const sim_mob::Person* per,const sim_mob::SubTrip &subTrip)
{
	// get person id and current subtrip id
	std::string fromToID = subTrip.fromLocation.node_->getID() + "," + subTrip.toLocation.node_->getID();
	//todo. change the subtrip signature from pointer to referencer
	logger << "+++++++++++++++++++++++++" << "\n";
	vector<WayPoint> res;
	//CBD area logic
	const Node * from = sim_mob::RestrictedRegion::getInstance().isInRestrictedZone(subTrip.fromLocation);
	const Node * to = sim_mob::RestrictedRegion::getInstance().isInRestrictedZone(subTrip.toLocation);
	if(to == nullptr && from ==nullptr){
		logger << fromToID  << ": Enforce blacklist for " << "\n";
		getBestPath(res, &subTrip,std::set<const sim_mob::RoadSegment*>(),false,true);//use/enforce blacklist
	}
	else if(to == nullptr || from ==nullptr){
		getBestPath(res, &subTrip);
	}
	//subscribe person
	logger << fromToID  << ": Path chosen for person[" << per->getId() << "]" << per->GetId() << "\n";
	if(!res.empty())
	{
		logger << fromToID << " : was assigned path of size " << res.size()  << "\n";
		//expensive due to call to getSegmentAimsunId()
		//printWPpath(res);
	}
	else{
		logger << fromToID <<" : NO PATH" << "\n";
	}
	return res;
}

//Operations:
//step-0: Initial preparations
//step-1: Check the cache
//step-2: If not found in cache, check DB
//Step-3: If not found in DB, generate all 4 types of path
//step-5: Choose the best path using utility function
bool sim_mob::PathSetManager::getBestPath(
		std::vector<sim_mob::WayPoint> &res,
		const sim_mob::SubTrip* st,
		std::set<const sim_mob::RoadSegment*> tempBlckLstSegs,
		 bool usePartialExclusion,
		 bool useBlackList,
		bool isUseCache)
{
	res.clear();
	if(st->mode != "Car") //only driver need path set
	{
		return false;
	}
	//take care of partially excluded and blacklisted segments here
	std::set<const sim_mob::RoadSegment*> blckLstSegs(tempBlckLstSegs);
	if(useBlackList & blckLstSegs.size() & this->blacklistSegments.size())
	{
		blckLstSegs.insert(this->blacklistSegments.begin(), this->blacklistSegments.end()); //temporary + permanent
	}
	const std::set<const sim_mob::RoadSegment*> &partial = (usePartialExclusion ? this->partialExclusions : std::set<const sim_mob::RoadSegment*>());

	const sim_mob::Node* fromNode = st->fromLocation.node_;
	const sim_mob::Node* toNode = st->toLocation.node_;
	if(toNode == fromNode){
		logger << "same OD objects discarded:" << toNode->getID() << "\n" ;
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
	logger << "[" << boost::this_thread::get_id() << "]searching for OD[" << fromToID << "]\n" ;
	boost::shared_ptr<sim_mob::PathSet> ps_;

	//Step-1 Check Cache
	sim_mob::Logger::log("ODs") << fromToID << "\n";
	if(isUseCache && findCachedPathSet(fromToID,ps_))
	{
		logger <<  fromToID  << " : Cache Hit" <<  "\n";
		bool r = getBestPathChoiceFromPathSet(ps_,partial,blckLstSegs);
		logger <<  fromToID << " : getBestPathChoiceFromPathSet returned best path of size : " << ps_->bestWayPointpath->size() << "\n";
		if(r)
		{
			res = *(ps_->bestWayPointpath);
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
	bool hasPSinDB = false;
	ps_.reset(new sim_mob::PathSet());
	ps_->subTrip = st;
	ps_->id = fromToID;
	bool hasSPinDB = sim_mob::aimsun::Loader::LoadSinglePathDBwithIdST(*getSession(),fromToID,ps_->pathChoices, dbFunction);
	logger  <<  fromToID << " : " << (hasSPinDB ? "" : "Don't " ) << "have SinglePaths in DB \n" ;
	if(hasSPinDB)
	{
		logger  <<  fromToID << " : DB Hit\n";
		bool r = false;
		ps_->oriPath = 0;
		BOOST_FOREACH(sim_mob::SinglePath* sp, ps_->pathChoices)
		{
			if(sp->isShortestPath){
				ps_->oriPath = sp;
				break;
			}
		}
		if(ps_->oriPath == 0)
		{
			std::string str = "Warning => SP: oriPath(shortest path) for "  + ps_->id + " not valid anymore\n";
			logger<< str ;
		}
		//	no need of processing and storing blacklisted paths
		short psCnt = ps_->pathChoices.size();
		r = getBestPathChoiceFromPathSet(ps_, partial, blckLstSegs);
		logger << fromToID << " :  number of paths before blcklist: " << psCnt << " after blacklist:" << ps_->pathChoices.size() << "\n" ;
		if(r)
		{
			res = *(ps_->bestWayPointpath);
			//cache
			if(isUseCache){
				r = cachePathSet(ps_);
			}
			else{
				clearSinglePaths(ps_);
			}
			//test
//			clearSinglePaths(ps_);
			logger << "returning a path " << res.size() << "\n";
			return true;
		}
		else
		{
				logger << "UNUSED DB hit\n";
		}
	}
	else // !hasSPinDB
	{
		logger  <<  fromToID << " : DB Miss\n";
	// Step-3 : If not found in DB, generate all 4 types of path
		logger<<"generate All PathChoices for "<<fromToID << "\n" ;
		// 1. generate shortest path with all segs
		// 1.2 get all segs
		// 1.3 generate shortest path with full segs
		ps_.reset(new PathSet(fromNode,toNode));
		ps_->id = fromToID;
		std:string temp = fromNode->originalDB_ID.getLogItem();
		ps_->fromNodeId = sim_mob::Utils::getNumberFromAimsunId(temp);
		temp = toNode->originalDB_ID.getLogItem();
		ps_->toNodeId = sim_mob::Utils::getNumberFromAimsunId(temp);
		ps_->scenario = scenarioName;
		ps_->subTrip = st;
		ps_->psMgr = this;

		bool r = generateAllPathChoicesMT(ps_);
		if(!r)
		{
			return false;
		}
		logger<<"generate All Done for "<<fromToID << "\n" ;
		logger.prof("utility_path_size").tick();
		sim_mob::generatePathSizeForPathSet2(ps_);
		logger.prof("utility_path_size").tick(true);
		r = getBestPathChoiceFromPathSet(ps_,partial,blckLstSegs);
		logger << "getBestPathChoiceFromPathSet returned best path of size : " << ps_->bestWayPointpath->size() << "\n";
		if(r)
		{
			res = *(ps_->bestWayPointpath);
			//cache
			if(isUseCache){
				r = cachePathSet(ps_);
				if(r)
				{
					logger << "------------------\n";
					uint32_t t = ps_->getSize();
					logger.prof("cached_pathset_bytes",false).addUp(t);
					logger.prof("cached_pathset_size",false).addUp(1);
					logger << "------------------\n";
				}
				else
				{
					logger << ps_->id << " not cached, apparently, already in cache.\n";
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
			logger << "returning a path " << res.size() << "\n";
			return true;
		}
		else
		{
			logger << "No best path, even after regenerating pathset " << "\n";
		}
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
	sim_mob::SinglePath *s = findShortestDrivingPath(ps->fromNode,ps->toNode,duplicateChecker,excludedSegs);
	if(!s)
	{
		// no path
		if(tempNoPath.find(ps->id) == tempNoPath.end())
		{
			ps->hasPath = -1;
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
	ps->hasPath = 1;
	ps->isNeedSave2DB = true;
	ps->oriPath = s;
	std::string fromToID(getFromToString(ps->fromNode, ps->toNode));
	ps->id = fromToID;
	ps->singlepath_id = s->id;
	s->pathset_id = ps->id;
	s->pathSet = ps;
	s->travelCost = sim_mob::getTravelCost2(s);
	s->travleTime = getTravelTime(s);

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
//	logger << "waiting for SHORTEST DISTANCE LINK ELIMINATION" << "\n";
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
//	logger << "waiting for SHORTEST TRAVEL TIME LINK ELIMINATION" << "\n";
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
//	logger << "waiting for TRAVEL TIME HIGHWAY BIAS" << "\n";
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
		work->excludeSeg = excludedSegs;
//		work->s = NULL;
		work->s->clear();
		work->ps = ps;
		std::stringstream out("");
		out << "random pertubation[" << i << "] '" << ps->fromNode->getID() << "," << ps->toNode->getID() << "'\n";
		work->dbgStr = out.str();
		logger << work->dbgStr;
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
				bool hasSPinDB = sim_mob::aimsun::Loader::LoadSinglePathDBwithId2(
#else
				bool hasSPinDB = sim_mob::aimsun::Loader::LoadSinglePathDBwithIdST(
						*getSession(),
#endif
						pathSetID,
						ps_->pathChoices,dbFunction);
				if(hasSPinDB)
				{
					std::map<std::string,sim_mob::SinglePath*>::iterator it = id_sp.find(ps_->singlepath_id);
					if(it!=id_sp.end())
					{
						ps_->oriPath = id_sp[ps_->singlepath_id];
						bool r = getBestPathChoiceFromPathSet(ps_);
						if(r)
						{
							res = *ps_->bestWayPointpath;// copy better than constant twisting
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
			logger<< "gBestPC2: create data for "<< fromToID << "\n";
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
				std:string temp = fromNode->originalDB_ID.getLogItem();
				ps_->fromNodeId = sim_mob::Utils::getNumberFromAimsunId(temp);
				temp = toNode->originalDB_ID.getLogItem();
				ps_->toNodeId = sim_mob::Utils::getNumberFromAimsunId(temp);
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
			ps_->hasPath = 1;
			ps_->subTrip = st;
			ps_->isNeedSave2DB = true;
			ps_->oriPath = s;
			ps_->id = fromToID;
			ps_->singlepath_id = s->id;
			s->pathset_id = ps_->id;
			s->pathSet = ps_;
			s->travelCost = getTravelCost2(s);
			s->travleTime = getTravelTime(s);
			ps_->pathChoices.insert(s);
				// 2. exclude each seg in shortest path, then generate new shortest path
			generatePathesByLinkElimination(s->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode);
				// generate shortest travel time path (default,morning peak,evening peak, off time)
				generateTravelTimeSinglePathes(fromNode,toNode,duplicateChecker,ps_);

				// generate k-shortest paths
				std::vector< std::vector<sim_mob::WayPoint> > kshortestPaths = kshortestImpl->getKShortestPaths(fromNode,toNode,ps_,duplicateChecker);
//				ps_->fromNodeId = fromNode->originalDB_ID.getLogItem();
//				ps_->toNodeId = toNode->originalDB_ID.getLogItem();
				std::string temp = fromNode->originalDB_ID.getLogItem();
				ps_->fromNodeId = sim_mob::Utils::getNumberFromAimsunId(temp);
				temp = toNode->originalDB_ID.getLogItem();
				ps_->toNodeId = sim_mob::Utils::getNumberFromAimsunId(temp);
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
					res = *ps_->bestWayPointpath;// copy better than constant twisting
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
			SinglePath *sinPath = findShortestDrivingPath(fromNode,toNode,duplicateChecker,seg);
			if(!sinPath)
			{
				continue;
			}
			sinPath->pathSet = ps_; // set parent
			sinPath->travelCost = getTravelCost2(sinPath);
			sinPath->travleTime = getTravelTime(sinPath);
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
			sinPath->travelCost = getTravelCost2(sinPath);
			sinPath->travleTime = getTravelTime(sinPath);
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
		sinPath_morningPeak->travelCost = getTravelCost2(sinPath_morningPeak);
		sinPath_morningPeak->travleTime = getTravelTime(sinPath_morningPeak);
		sinPath_morningPeak->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath_morningPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_morningPeak->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::MorningPeak);
	}
	SinglePath *sinPath_eveningPeak = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::EveningPeak);
	if(sinPath_eveningPeak)
	{
		sinPath_eveningPeak->pathSet = ps_; // set parent
		sinPath_eveningPeak->travelCost = getTravelCost2(sinPath_eveningPeak);
		sinPath_eveningPeak->travleTime = getTravelTime(sinPath_eveningPeak);
		sinPath_eveningPeak->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath_eveningPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_eveningPeak->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::EveningPeak);
	}
	SinglePath *sinPath_offPeak = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::OffPeak);
	if(sinPath_offPeak)
	{
		sinPath_offPeak->pathSet = ps_; // set parent
		sinPath_offPeak->travelCost = getTravelCost2(sinPath_offPeak);
		sinPath_offPeak->travleTime = getTravelTime(sinPath_offPeak);
		sinPath_offPeak->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath_offPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_offPeak->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::OffPeak);
	}
	SinglePath *sinPath_default = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::Default);
	if(sinPath_default)
	{
		sinPath_default->pathSet = ps_; // set parent
		sinPath_default->travelCost = getTravelCost2(sinPath_default);
		sinPath_default->travleTime = getTravelTime(sinPath_default);
		sinPath_default->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath_default);
		generatePathesByTravelTimeLinkElimination(sinPath_default->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::Default);
	}
	// generate high way bias path
	SinglePath *sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_Distance);
	if(sinPath)
	{
		sinPath->pathSet = ps_; // set parent
		sinPath->travelCost = getTravelCost2(sinPath);
		sinPath->travleTime = getTravelTime(sinPath);
		sinPath->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_Distance);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_MorningPeak);
	if(sinPath)
	{
		sinPath->pathSet = ps_; // set parent
		sinPath->travelCost = getTravelCost2(sinPath);
		sinPath->travleTime = getTravelTime(sinPath);
		sinPath->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_EveningPeak);
	if(sinPath)
	{
		sinPath->pathSet = ps_; // set parent
		sinPath->travelCost = getTravelCost2(sinPath);
		sinPath->travleTime = getTravelTime(sinPath);
		sinPath->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_OffPeak);
	if(sinPath)
	{
		sinPath->pathSet = ps_; // set parent
		sinPath->travelCost = getTravelCost2(sinPath);
		sinPath->travleTime = getTravelTime(sinPath);
		sinPath->pathset_id = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_Default);
	if(sinPath)
	{
		sinPath->pathSet = ps_; // set parent
		sinPath->travelCost = getTravelCost2(sinPath);
		sinPath->travleTime = getTravelTime(sinPath);
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
			sinPath->travelCost = getTravelCost2(sinPath);
			sinPath->travleTime = getTravelTime(sinPath);
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
	utility += sp->travleTime * bTTVOT;
	//2.0
	//Obtain the path size PS of the path.
	utility += sp->pathSize * bCommonFactor;
	//3.0
	//Obtain the travel distance l and the highway distance w of the path.
	utility += sp->length * bLength + sp->highWayDistance * bHighway;
	//4.0
	//Obtain the travel cost c of the path.
	utility += sp->travelCost * bCost;
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


bool sim_mob::PathSetManager::getBestPathChoiceFromPathSet(boost::shared_ptr<sim_mob::PathSet> &ps,
		const std::set<const sim_mob::RoadSegment *> & partialExclusion ,
		const std::set<const sim_mob::RoadSegment*> &blckLstSegs )
{
	bool computeUtility = false;
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
		if(blckLstSegs.size() && sp->includesRoadSegment(blckLstSegs))
		{
			continue;//do the same thing while measuring the probability in the loop below
		}
		if(sp->shortestWayPointpath.empty())
		{
			std::string str = temp + " Singlepath empty";
			throw std::runtime_error (str);
		}
		sp->pathSet = ps;
		//	trying to save some computation like getTravelTime(), getUtilityBySinglePath()
		computeUtility = false;
		//	state of the network changed
		if (partialExclusion.size() && sp->includesRoadSegment(partialExclusion) ) {
			sp->travleTime = maxTravelTime;//some large value like infinity
			computeUtility = true;
		}
		//	not previously calculated
		if(sp->travleTime == 0.0)
		{
			sp->travleTime = getTravelTime(sp);
			computeUtility = true;
		}
		//	not previously calculated
		if(sp->utility == 0.0)
		{
			computeUtility = true;
		}
		//	calculate utility
		if(computeUtility)
		{
			sp->utility = getUtilityBySinglePath(sp);
		}

		ps->logsum += exp(sp->utility);
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
		if(blckLstSegs.size() && sp->includesRoadSegment(blckLstSegs))
		{
			continue;//do the same thing while processing the single path in the loop above
		}
		i++;
		double prob = exp(sp->utility)/(ps->logsum);
		upperProb += prob;
		if (x <= upperProb)
		{
			// 2.3 agent A chooses path i from the path choice set.
			ps->bestWayPointpath = &(sp->shortestWayPointpath);
			logger << "OD:" << sp->pathset_id <<  ", path no: " << i << "out of " << ps->pathChoices.size()  << "paths chosen\n" ;
			return true;
		}
	}

	// path choice algorithm
	if(!ps->oriPath)//return upon null oriPath only if the condition is normal(excludedSegs is empty)
	{
		logger<< "NO PATH , getBestPathChoiceFromPathSet, shortest path empty" << "\n";
		return false;
	}
	//the last step resorts to selecting and returning shortest path(aka oripath).
	logger << "NO BEST PATH. select to shortest path\n" ;
	ps->bestWayPointpath = &(ps->oriPath->shortestWayPointpath);
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
		sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(s);
		s->fromNode = fromNode;
		s->toNode = toNode;
		s->pathSet.reset();
		s->length = sim_mob::generateSinglePathLengthPT(s->shortestWayPointpath);
		s->id = id;
		s->scenario = scenarioName;
		s->pathSize=0;
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
			sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(s);
			s->fromNode = fromNode;
			s->toNode = toNode;
			s->pathSet.reset();
			s->length = sim_mob::generateSinglePathLengthPT(s->shortestWayPointpath);
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
	if(sp->shortestWayPointpath.size()<2)
	{
		sp->right_turn_number=0;
		sp->signal_number=0;
		return ;
	}
	int res=0;
	int signalNumber=0;
	std::vector<WayPoint>::iterator itt=sp->shortestWayPointpath.begin();
	++itt;
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
			if(sp->travleTime < minTravelTime)
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
			if(sp->travleTime < minTravelTime)
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
			if(sp->travleTime < minTravelTime)
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
			if(sp->travleTime < minTravelTime)
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
		sp->pathSize = log(size);
	}// end for
}

double sim_mob::PathSetManager::getTravelTime(sim_mob::SinglePath *sp)
{
//	logger.prof("getTravelTime").tick();
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
//	logger.prof("getTravelTime").tick(true);
	return ts;
}
double sim_mob::PathSetManager::getTravelTimeBySegId(std::string id,sim_mob::DailyTime startTime)
{
	std::map<std::string,std::vector<sim_mob::LinkTravelTime> >::iterator it;
	double res=0.0;
	//2. if no , check default
	it = pathSetParam->segmentDefaultTravelTimePool.find(id);
	if(it!= pathSetParam->segmentDefaultTravelTimePool.end())
	{
		std::vector<sim_mob::LinkTravelTime> &e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::LinkTravelTime& l = e[i];
			if( l.startTime_DT.isBeforeEqual(startTime) && l.endTime_DT.isAfter(startTime) )
			{
				res = l.travelTime;
				return res;
			}
		}
	}
	else
	{
		std::string str = "PathSetManager::getTravelTimeBySegId=> no travel time for segment " + id + "  ";
		logger<< "error: " << str << pathSetParam->segmentDefaultTravelTimePool.size() << "\n";
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
	pathSize = 0.0;
	travelCost=0.0;
	signal_number=0.0;
	right_turn_number=0.0;
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
	logger << "SinglePath size bytes:" << sum << "\n" ;
	return sum;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
sim_mob::PathSet::~PathSet()
{
	fromNode = NULL;
	toNode = NULL;
	subTrip = NULL;
	logger << "Deleting PathSet " << id << " and its " << pathChoices.size() << "  singlepaths\n";
	BOOST_FOREACH(sim_mob::SinglePath*sp,pathChoices)
	{
		safe_delete_item(sp);
	}
}

sim_mob::PathSet::PathSet(const boost::shared_ptr<sim_mob::PathSet> & ps) :
		logsum(ps->logsum),oriPath(ps->oriPath),
		subTrip(ps->subTrip),
		id(ps->id),
		fromNodeId(ps->fromNodeId),
		toNodeId(ps->toNodeId),
		scenario(ps->scenario),
		pathChoices(ps->pathChoices),
		singlepath_id(ps->singlepath_id),
		hasPath(ps->hasPath),
		bestWayPointpath(nullptr)
{
	isNeedSave2DB=false;
	isInit = false;
	hasBestChoice = false;
	// 1. get from to nodes
	//	can get nodes later,when insert to personPathSetPool
	this->fromNode = sim_mob::PathSetParam::getInstance()->getCachedNode(fromNodeId);
	this->toNode = sim_mob::PathSetParam::getInstance()->getCachedNode(toNodeId);
//	// get all relative singlepath
}


uint32_t sim_mob::PathSet::getSize(){
	uint32_t sum = 0;
		sum += sizeof(bool);// isInit;
		sum += sizeof(bool);//bool hasBestChoice;
		sum += sizeof(WayPoint) * (bestWayPointpath ? bestWayPointpath->size() : 0);//std::vector<WayPoint> bestWayPointpath;  //best choice
		sum += sizeof(const sim_mob::Node *);//const sim_mob::Node *fromNode;
		sum += sizeof(const sim_mob::Node *);//const sim_mob::Node *toNode;
		sum += personId.length();//std::string personId; //person id
		sum += tripId.length();//std::string tripId; // trip item id
		sum += sizeof(SinglePath*);//SinglePath* oriPath;  // shortest path with all segments
		//std::map<std::string,sim_mob::SinglePath*> SinglePathPool;//unused so far
		typedef std::map<std::string,sim_mob::SinglePath*>::value_type tt;
		BOOST_FOREACH(tt & pair_,SinglePathPool)
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
		sum += fromNodeId.length();//std::string fromNodeId;
		sum += toNodeId.length();//std::string toNodeId;
		sum += singlepath_id.length();//std::string singlepath_id;
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
		BOOST_FOREACH(sim_mob::WayPoint &wp, sp->shortestWayPointpath)
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

sim_mob::SinglePath::SinglePath(const SinglePath& source) :
		id(source.id),
		utility(source.utility),pathSize(source.pathSize),
		travelCost(source.travelCost),
		signal_number(source.signal_number),
		right_turn_number(source.right_turn_number),
		length(source.length),travleTime(source.travleTime),
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
				logger << "error: " << str << "\n";
			}
			this->shortestWayPointpath.push_back(WayPoint(seg));//copy better than this twist
			shortestSegPath.insert(seg);
		}
	}
}

sim_mob::SinglePath::~SinglePath(){
	clear();
}

bool sim_mob::SinglePath::includesRoadSegment(const std::set<const sim_mob::RoadSegment*> & segs){
	BOOST_FOREACH(sim_mob::WayPoint &wp, this->shortestWayPointpath){
		BOOST_FOREACH(const sim_mob::RoadSegment* seg, segs){
			if(wp.roadSegment_ == seg){
				return true;
			}
		}
	}
	return false;
}

/***********************************************************************************************************
 * **********************************  UNUSED CODE!!!! *****************************************************
************************************************************************************************************/

sim_mob::PathSet::PathSet(boost::shared_ptr<sim_mob::PathSet> &ps) :
		fromNode(ps->fromNode),toNode(ps->toNode),
		logsum(ps->logsum),oriPath(ps->oriPath),
		subTrip(ps->subTrip),
		id(ps->id),
		fromNodeId(ps->fromNodeId),
		toNodeId(ps->toNodeId),
		scenario(ps->scenario),
		pathChoices(ps->pathChoices),
		hasPath(ps->hasPath),
		bestWayPointpath(ps->bestWayPointpath)
{
	if(!ps)
		throw std::runtime_error("PathSet error");

	isNeedSave2DB=false;
}
