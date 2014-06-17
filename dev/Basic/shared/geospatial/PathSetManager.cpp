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

#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/foreach.hpp>

using std::vector;
using std::string;

using namespace sim_mob;

PathSetManager *sim_mob::PathSetManager::instance_;
sim_mob::Profiler sim_mob::PathSetManager::profiler(false, "main_profiler","path_set_profiler.txt");

PathSetParam *sim_mob::PathSetParam::instance_ = NULL;

sim_mob::PathSetParam* sim_mob::PathSetParam::getInstance()
{
	if(!instance_)
	{
		instance_ = new PathSetParam();
	}
	return instance_;
}
sim_mob::RoadSegment* sim_mob::PathSetParam::getRoadSegmentByAimsunId(std::string id)
{
	std::map<std::string,sim_mob::RoadSegment*>::iterator it = segPool.find(id);
	if(it != segPool.end())
	{
		sim_mob::RoadSegment* seg = (*it).second;
		return seg;
	}
	return NULL;
}
void sim_mob::PathSetParam::getDataFromDB()
{
	setTravleTimeTmpTableName(ConfigManager::GetInstance().FullConfig().getTravelTimeTmpTableName());
		createTravelTimeTmpTable();
		sim_mob::aimsun::Loader::LoadERPData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
				ERP_Surcharge_pool,
				ERP_Gantry_Zone_pool,
				ERP_Section_pool);
		sim_mob::aimsun::Loader::LoadDefaultTravelTimeData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
				Link_default_travel_time_pool);
		bool res = sim_mob::aimsun::Loader::LoadRealTimeTravelTimeData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
				pathset_traveltime_realtime_table_name,
				Link_realtime_travel_time_pool);
		if(!res) // no realtime travel time table
		{
			//create
			if(!createTravelTimeRealtimeTable() )
			{
				throw std::runtime_error("can not create travel time table");
			}
		}
//	}
//	// test insert into tmp table
//	sim_mob::Link_travel_time data;
//	data.start_time = "00:00:00";
//	data.end_time = "08:08:08";
//	data.travel_time = 1.0;
//	data.link_id = 123;
//	insertTravelTime2TmpTable(data);
}
void sim_mob::PathSetParam::storeSinglePath(soci::session& sql,std::vector<sim_mob::SinglePath*>& spPool)
{
	sim_mob::aimsun::Loader::SaveOneSinglePathDataST(sql,spPool);
}
void sim_mob::PathSetParam::storePathSet(soci::session& sql,std::map<std::string,sim_mob::PathSet* >& psPool)
{
	sim_mob::aimsun::Loader::SaveOnePathSetDataST(sql,psPool);
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
	Print()<<"setTravleTimeTmpTableName: "<<pathset_traveltime_tmp_table_name<<std::endl;
	pathset_traveltime_realtime_table_name = value+"_travel_time";
}
double sim_mob::PathSetParam::getAverageTravelTimeBySegIdStartEndTime(std::string id,sim_mob::DailyTime startTime,sim_mob::DailyTime endTime)
{
	//1. check realtime table
	double res=0.0;
	double totalTravelTime=0.0;
	int count=0;
	std::map<std::string,std::vector<sim_mob::Link_travel_time*> >::iterator it =
			Link_realtime_travel_time_pool.find(id);
	if(it!=Link_realtime_travel_time_pool.end())
	{
		std::vector<sim_mob::Link_travel_time*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::Link_travel_time* l = e[i];
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
//	double res=0.0;
	it = Link_default_travel_time_pool.find(id);
	if(it!=Link_default_travel_time_pool.end())
	{
		std::vector<sim_mob::Link_travel_time*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::Link_travel_time* l = e[i];
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
		std::string str = "PathSetParam::getAverageTravelTimeBySegIdStartEndTime=> no travel time for segment " + id;
		Print()<<"error: "<<str<<std::endl;
	}
	return res;
}

double sim_mob::PathSetParam::getDefaultTravelTimeBySegId(std::string id)
{
	double res=0.0;
	double totalTravelTime=0.0;
	int count=0;
	std::map<std::string,std::vector<sim_mob::Link_travel_time*> >::iterator it =
			Link_default_travel_time_pool.find(id);
	if(it!=Link_default_travel_time_pool.end())
	{
		std::vector<sim_mob::Link_travel_time*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::Link_travel_time* l = e[i];
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
		Print()<<"error: "<<str<<std::endl;
	}
	return res;
}
double sim_mob::PathSetParam::getTravelTimeBySegId(std::string id,sim_mob::DailyTime startTime)
{
	//1. check realtime table
	double res=0.0;
	std::map<std::string,std::vector<sim_mob::Link_travel_time*> >::iterator it =
			Link_realtime_travel_time_pool.find(id);
	if(it!=Link_realtime_travel_time_pool.end())
	{
		std::vector<sim_mob::Link_travel_time*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::Link_travel_time* l = e[i];
			if( l->start_time_dt.isBeforeEqual(startTime) && l->end_time_dt.isAfter(startTime) )
			{
				res = l->travel_time;
				return res;
			}
		}
	}
	//2. if no , check default
//	double res=0.0;
	it = Link_default_travel_time_pool.find(id);
	if(it!=Link_default_travel_time_pool.end())
	{
		std::vector<sim_mob::Link_travel_time*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::Link_travel_time* l = e[i];
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
		Print()<<"error: "<<str<<std::endl;
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
sim_mob::Node* sim_mob::PathSetParam::getNodeByAimsunId(std::string id)
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
	isUseCache=false;

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

sim_mob::PathSetParam::PathSetParam() {
	roadNetwork = &ConfigManager::GetInstance().FullConfig().getNetwork();
	initParameters();
	multiNodesPool = roadNetwork->getNodes();
	uniNodesPool = roadNetwork->getUniNodes();
			for(std::vector<sim_mob::Link *>::const_iterator it = roadNetwork->getLinks().begin(), it_end(roadNetwork->getLinks().end()); it != it_end ; it ++)
			{
				for(std::set<sim_mob::RoadSegment *>::iterator seg_it = (*it)->getUniqueSegments().begin(), it_end((*it)->getUniqueSegments().end()); seg_it != it_end; seg_it++)
				{
					if (!(*seg_it)->originalDB_ID.getLogItem().empty())
					{
						string aimsun_id = (*seg_it)->originalDB_ID.getLogItem();
						string seg_id = getNumberFromAimsunId(aimsun_id);
						segPool.insert(std::make_pair(seg_id,*seg_it));
						WayPoint *wp = new WayPoint(*seg_it);
						wpPool.insert(std::make_pair(*seg_it,wp));
					}
				}
			}
			for(int i=0;i<multiNodesPool.size();++i)
			{
				sim_mob::Node* n = multiNodesPool.at(i);
				if (!n->originalDB_ID.getLogItem().empty())
				{
					std::string id = n->originalDB_ID.getLogItem();
					nodePool.insert(std::make_pair(id,n));
				}
			}

			for(std::set<sim_mob::UniNode*>::iterator it=uniNodesPool.begin(); it!=uniNodesPool.end(); ++it)
			{
				sim_mob::UniNode* n = (*it);
				if (!n->originalDB_ID.getLogItem().empty())
				{
					std::string id = n->originalDB_ID.getLogItem();
					nodePool.insert(std::make_pair(id,n));
				}
			}
		Print()<<"PathSetParam: nodes amount "<<multiNodesPool.size() + uniNodesPool.size()<<std::endl;
		Print()<<"PathSetParam: segments amount "<<roadNetwork->getLinks().size()<<std::endl;

		getDataFromDB();
}

sim_mob::PathSetManager::PathSetManager() {
	sql = NULL;
	psDbLoader=NULL;
	pathSetParam = PathSetParam::getInstance();
	stdir = &StreetDirectory::instance();
	roadNetwork = &ConfigManager::GetInstance().FullConfig().getNetwork();
	multiNodesPool = roadNetwork->getNodes();
	uniNodesPool = roadNetwork->getUniNodes();
//	// 1.2 get all segs
	init();
	if(!psDbLoader)
	{
		psDbLoader = new PathSetDBLoader(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
	}

	serialPathSetGroup = ConfigManager::GetInstance().FullConfig().PathSetGenerationMode();
	threadpool_ = new sim_mob::batched::ThreadPool(50);
}

sim_mob::PathSetManager::~PathSetManager()
{
	if(threadpool_){
		delete threadpool_;
	}
}

void sim_mob::PathSetManager::init()
{
	setCSVFileName();
	initParameters();
}
void sim_mob::PathSetManager::clearPools()
{

	for(std::map<std::string,sim_mob::PathSet* >::iterator it = pathSetPool.begin();
			it!=pathSetPool.end();
			++it)
	{
		sim_mob::PathSet* ps = (*it).second;
		if(ps)
			delete ps;
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
}

bool sim_mob::PathSetManager::LoadSinglePathDBwithId(
		std::string& pathset_id,
		std::vector<sim_mob::SinglePath*>& spPool)
{
	bool res=false;
	res = sim_mob::aimsun::Loader::LoadSinglePathDBwithId2(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
			waypoint_singlepathPool,pathset_id,spPool);
	return res;
}


bool sim_mob::PathSetManager::generateAllPathSetWithTripChain2()
{
	const std::map<std::string, std::vector<sim_mob::TripChainItem*> > *tripChainPool =
			&ConfigManager::GetInstance().FullConfig().getTripChains();
	Print()<<"generateAllPathSetWithTripChain: trip chain pool size "<<
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
		for(std::vector<sim_mob::TripChainItem*>::iterator it=tci.begin(); it!=tci.end(); ++it)
		{
			if((*it)->itemType == sim_mob::TripChainItem::IT_TRIP)
			{
				sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip*> ((*it));
				if(!trip)
					throw std::runtime_error("generateAllPathSetWithTripChainPool: trip error");
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
		Print()<<"generateAllPathSetWithTripChain: "<<i<<"/"<<poolsize<<" mem: "<<size()/1024/1024<<"mb"<<std::endl;
	}
	return res;
}

void sim_mob::PathSetManager::storePersonIdPathSets(std::string personId,std::map<std::string,sim_mob::PathSet*> subTripId_pathSet)
{
	personPathSetPool.insert(std::make_pair(personId,subTripId_pathSet));
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
	Print()<<"csvFileName: "<<csvFileName<<std::endl;
	csvFile.open(csvFileName.c_str());
}

bool sim_mob::PathSetManager::insertTravelTime2TmpTable(sim_mob::Link_travel_time& data)
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
		  sim_mob::aimsun::Loader::insertCSV2TableST(psDbLoader->sql,
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
	Print()<<"table name: "<<pathset_traveltime_tmp_table_name<<std::endl;
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

std::map<std::string,sim_mob::PathSet*> sim_mob::PathSetManager::generatePathSetByTripChainItemPool(std::vector<sim_mob::TripChainItem*> &tci)
{
	std::map<std::string,sim_mob::PathSet*> res;
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

void  sim_mob::PathSetManager::generatePathSetByTrip(std::map<std::string,sim_mob::PathSet*> &subTripId_pathSet,sim_mob::Trip *trip)
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
			sim_mob::PathSet *ps = generatePathSetByFromToNodes(fromNode,toNode,st);
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
bool sim_mob::PathSetManager::getCachedBestPath(std::string id, std::vector<WayPoint>& value)
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
	const std::pair <RPOD::const_iterator,RPOD::const_iterator > range = pathSegments.equal_range(segment);
	return range;
}

vector<WayPoint> sim_mob::PathSetManager::getPathByPerson(sim_mob::Person* per)
{
	std::ostringstream out("");

	if(!profiler.isStarted()){
		//happens only once
		profiler.startProfiling();
	}
	std::ostringstream id("");
	id << per << "-" << per->currWorkerProvider;
	Profiler personProfiler(true, id.str());
	out << "Profiling person " << id.str() << std::endl;
	personProfiler.addOutPut(out);
	// get person id and current subtrip id
	std::string personId = per->getDatabaseId();
	std::vector<sim_mob::SubTrip>::const_iterator currSubTripIt = per->currSubTrip;
	const sim_mob::SubTrip *subTrip = &(*currSubTripIt);
	std::string subTripId = subTrip->tripID;

	Worker *worker = (Worker*)per->currWorkerProvider;
	if(worker)
	{
		sql = &(worker->sql);
	}
	else
	{
		sql = &(psDbLoader->sql);
	}
		vector<WayPoint> res = generateBestPathChoiceMT(subTrip, profiler);
		cacheODbySegment(per, subTrip, res);
		sql = NULL;

		uint32_t elapsed = personProfiler.endProfiling();
		out.str("");
		out << "Total profiling time(ms) for person " << id.str() << " :" <<  elapsed << std::endl;
		out << "------------------------------------------------------------------------------------" << std::endl;
		personProfiler.addOutPut(out);
		//All done for this person, so add the out put and time lapse to the main profiler
		profiler.addOutPut(personProfiler.outPut(),true) ;
		profiler.addToTotalTime(personProfiler.getTotalTime());

		return res;
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
	std::string fromId_toId = fromNode->originalDB_ID.getLogItem() +"_"+ toNode->originalDB_ID.getLogItem();
	std::string mys=fromId_toId;
	//check cache to save a trouble if the path already exists
	if(getCachedBestPath(fromId_toId,res))
	{
		return res;
	}
		//
		mys = "'"+mys+"'";
		sim_mob::PathSet ps_;
#if 0
		bool hasPSinDB = sim_mob::aimsun::Loader::LoadOnePathSetDBwithId(
						ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
						ps_,mys);
#else
		bool hasPSinDB = sim_mob::aimsun::Loader::LoadOnePathSetDBwithIdST(
						*sql,
						ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
						ps_,mys);
#endif
		if(ps_.has_path == -1) //no path
		{
			return res;
		}
		if(hasPSinDB)
		{
			// init ps_
			if(!ps_.isInit)
			{
				ps_.subTrip = st;
				std::map<std::string,sim_mob::SinglePath*> id_sp;
#if 0
				bool hasSPinDB = sim_mob::aimsun::Loader::LoadSinglePathDBwithId2(
#else
				bool hasSPinDB = sim_mob::aimsun::Loader::LoadSinglePathDBwithIdST(
						*sql,
#endif
						ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
						id_sp,
						mys,
						ps_.pathChoices);
				if(hasSPinDB)
				{
					std::map<std::string,sim_mob::SinglePath*>::iterator it = id_sp.find(ps_.singlepath_id);
					if(it!=id_sp.end())
					{
						ps_.oriPath = id_sp[ps_.singlepath_id];
						bool r = getBestPathChoiceFromPathSet(ps_);
						if(r)
						{
							res = sim_mob::convertWaypointP2Wp(ps_.bestWayPointpathP);
							// delete ps,sp
							for(int i=0;i<ps_.pathChoices.size();++i)
							{
								sim_mob::SinglePath* sp_ = ps_.pathChoices[i];
								if(sp_)
									delete sp_;
							}
							ps_.pathChoices.clear();
							insertFromTo_BestPath_Pool(fromId_toId,res);
							return res;
						}
						else
							return res;
					}
					else
					{
						std::string str = "gBestPC2: oriPath not find,no path for this nodes pair ";
						Print()<<str<<std::endl;
						return res;
					}
				}// hasSPinDB
			}
		} // hasPSinDB
		else
		{
			Print()<<"gBestPC2: create data for "<<fromId_toId<<std::endl;
			// 1. generate shortest path with all segs
			// 1.1 check StreetDirectory
			if(!stdir || !roadNetwork)
			{
				throw std::runtime_error("StreetDirectory or RoadNetwork is null");
			}
			// 1.2 get all segs
			// 1.3 generate shortest path with full segs
			std::map<std::string,SinglePath*> wp_spPool;
			sim_mob::SinglePath *s = generateSinglePathByFromToNodes3(fromNode,toNode,wp_spPool);
			if(!s)
			{
				// no path
				ps_ = PathSet(fromNode,toNode);
				ps_.has_path = -1;
				ps_.isNeedSave2DB = true;
				ps_.id = fromId_toId;
				ps_.from_node_id = fromNode->originalDB_ID.getLogItem();
				ps_.to_node_id = toNode->originalDB_ID.getLogItem();
				ps_.scenario = scenarioName;
				std::map<std::string,sim_mob::PathSet* > tmp;
				tmp.insert(std::make_pair(fromId_toId,&ps_));
				sim_mob::aimsun::Loader::SaveOnePathSetData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),tmp);
				return res;
			}
			//	// 1.31 check path pool
				// 1.4 create PathSet object
			ps_ = PathSet(fromNode,toNode);
			ps_.has_path = 1;
			ps_.subTrip = st;
			ps_.isNeedSave2DB = true;
			ps_.oriPath = s;
			ps_.id = fromId_toId;
			ps_.singlepath_id = s->id;
			s->pathset_id = ps_.id;
			s->pathSet = &ps_;
			s->travel_cost = getTravelCost2(s);
			s->travle_time = getTravelTime(s);
			ps_.pathChoices.push_back(s);
				// 2. exclude each seg in shortest path, then generate new shortest path
			generatePathesByLinkElimination(s->shortestWayPointpath,wp_spPool,ps_,fromNode,toNode);
				// generate shortest travel time path (default,morning peak,evening peak, off time)
				generateTravelTimeSinglePathes(fromNode,toNode,wp_spPool,ps_);

				// generate k-shortest paths
				std::vector< std::vector<sim_mob::WayPoint> > kshortestPaths = kshortestImpl->getKShortestPaths(fromNode,toNode,ps_,wp_spPool);
				ps_.from_node_id = fromNode->originalDB_ID.getLogItem();
				ps_.to_node_id = toNode->originalDB_ID.getLogItem();
				ps_.scenario = scenarioName;
				// 3. store pathset
				sim_mob::generatePathSizeForPathSet2(&ps_);
				std::map<std::string,sim_mob::PathSet* > tmp;
				tmp.insert(std::make_pair(fromId_toId,&ps_));
				sim_mob::aimsun::Loader::SaveOnePathSetData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),tmp);
				//
				bool r = getBestPathChoiceFromPathSet(ps_);
				if(r)
				{
					res = sim_mob::convertWaypointP2Wp(ps_.bestWayPointpathP);
					// delete ps,sp
					for(int i=0;i<ps_.pathChoices.size();++i)
					{
						sim_mob::SinglePath* sp_ = ps_.pathChoices[i];
						if(sp_)
							delete sp_;
					}
					ps_.pathChoices.clear();
					return res;
				}
				else
					return res;
		}

	return res;
}
//Operations:
//check the cache
//if not found in cache, check DB
//if not found in DB, generate all 4 types of path
//choose the best path using utility function
std::vector<WayPoint> sim_mob::PathSetManager::generateBestPathChoiceMT(const sim_mob::Person * per, const sim_mob::SubTrip* st,
		const std::vector<const sim_mob::RoadSegment*> & exclude_seg, bool isUseCache, bool isUseDB){
	//you may need to double check your database connection
	Worker *worker = (Worker*)per->currWorkerProvider;
	if(worker)
	{
		sql = &(worker->sql);
	}
	else
	{
		sql = &(psDbLoader->sql);
	}
	Profiler profiler;
	//call the default method
	return generateBestPathChoiceMT(st, profiler, exclude_seg, isUseCache, isUseDB);
}

vector<WayPoint> sim_mob::PathSetManager::generateBestPathChoiceMT(const sim_mob::SubTrip* st, Profiler & personProfiler, const std::vector<const sim_mob::RoadSegment*> & exclude_seg , bool isUseCache, bool isUseDB)
{
	vector<WayPoint> res;
	std::ostringstream out("");
	if(st->mode != "Car") //only driver need path set
	{
		return res;
	}
	const sim_mob::Node* fromNode = st->fromLocation.node_;
	const sim_mob::Node* toNode = st->toLocation.node_;
	std::string fromId_toId = fromNode->originalDB_ID.getLogItem() +"_"+ toNode->originalDB_ID.getLogItem();
	std::string mys=fromId_toId;
	sim_mob::PathSet ps_;
	//check cache
	sim_mob::Profiler CBP_Profiler(true);
	if(isUseCache){
		if(getCachedBestPath(fromId_toId,res))
		{
			out << "getCachedBestPath:true:" << CBP_Profiler.endProfiling() << std::endl;
			personProfiler.addOutPut(out);
			return res;
		}
	}
	bool hasPSinDB = false;
	if(isUseDB){
		out.str("");
		out  << "getCachedBestPath:false:" << CBP_Profiler.endProfiling() << std::endl;
		personProfiler.addOutPut(out);
		//
		mys = "'"+mys+"'";
		Profiler PSDB_Profiler(true);
		hasPSinDB = sim_mob::aimsun::Loader::LoadOnePathSetDBwithIdST(
								*sql,
								ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
								ps_,mys);
		//time taken to find out if there is a path set in DB
		out.str("");
		out << "hasPSinDB:" << (hasPSinDB ? "true" : "false" ) << ":" << PSDB_Profiler.endProfiling() << std::endl;
		personProfiler.addOutPut(out);

		if(ps_.has_path == -1) //no path
		{
			return res;
		}
		if(hasPSinDB)
		{
			// init ps_
			if(!ps_.isInit)
			{
				ps_.subTrip = st;
				std::map<std::string,sim_mob::SinglePath*> id_sp;
				Profiler SP_Profiler(true);
				bool hasSPinDB = sim_mob::aimsun::Loader::LoadSinglePathDBwithIdST(
										*sql,
										ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
										id_sp,
										mys,
										ps_.pathChoices);

				out.str("");
				out << "hasSPinDB:" << (hasSPinDB ? "true" : "false" ) << ":" << SP_Profiler.endProfiling() << std::endl;
				personProfiler.addOutPut(out);

				if(hasSPinDB)
				{
					bool r = false;
					Profiler BPC_Profiler(true);
					std::map<std::string,sim_mob::SinglePath*>::iterator it = id_sp.find(ps_.singlepath_id);
					if(it!=id_sp.end())
					{
						ps_.oriPath = id_sp[ps_.singlepath_id];
						r = getBestPathChoiceFromPathSet(ps_);
						if(r)
						{
							res = sim_mob::convertWaypointP2Wp(ps_.bestWayPointpathP);
							insertFromTo_BestPath_Pool(fromId_toId,res);
							// delete ps,sp
							for(int i=0;i<ps_.pathChoices.size();++i)
							{
								sim_mob::SinglePath* sp_ = ps_.pathChoices[i];
								if(sp_)
									delete sp_;
							}
							ps_.pathChoices.clear();
						}
							return res;
					}
					else
					{
						std::string str = "gBestPC2: oriPath not find,no path for this nodes pair ";
						Print()<<str<<std::endl;
					}

					out.str("");
					out << "BPC_Profiler:" << (r ? "true" : "false" ) << ":" << BPC_Profiler.endProfiling() << std::endl;
					personProfiler.addOutPut(out);
					return res;
				}// hasSPinDB
			}
		} // hasPSinDB
	}//isUseDB
	if(!isUseDB || !hasPSinDB)
		{
			Print()<<"gBestPC2: create data for "<<fromId_toId<<std::endl;
			// 1. generate shortest path with all segs
			// 1.1 check StreetDirectory
			if(!stdir || !roadNetwork)
			{
				throw std::runtime_error("StreetDirectory or RoadNetwork is null");
			}
			// 1.2 get all segs
			// 1.3 generate shortest path with full segs
			ps_ = PathSet(fromNode,toNode);
			ps_.id = fromId_toId;
			ps_.from_node_id = fromNode->originalDB_ID.getLogItem();
			ps_.to_node_id = toNode->originalDB_ID.getLogItem();
			ps_.scenario = scenarioName;
			ps_.subTrip = st;
			ps_.psMgr = this;
			out.str("");
			out << "generateAllPathChoicesMT:start" << std::endl;
			personProfiler.addOutPut(out);

			if(!generateAllPathChoicesMT(&ps_, personProfiler,exclude_seg))
			{
				out.str("");
				out << "generateAllPathChoicesMT:false" << std::endl;
				personProfiler.addOutPut(out);
				return res;
			}
			out.str("");
			out << "generateAllPathChoicesMT:true" << std::endl;
			personProfiler.addOutPut(out);

			for(std::map<std::string,sim_mob::SinglePath*>::iterator it=ps_.SinglePathPool.begin();it!=ps_.SinglePathPool.end();++it)
			{
				sim_mob::SinglePath* sp = (*it).second;
				ps_.pathChoices.push_back(sp);
			}
			sim_mob::Profiler GENPSPS_Profiler(true);
			sim_mob::generatePathSizeForPathSet2(&ps_);
			out.str("");
			out << "GENERATE_PATH_SIZE_FOR_PATH_SET:" << GENPSPS_Profiler.endProfiling() << std::endl;
			personProfiler.addOutPut(out);

			std::map<std::string,sim_mob::PathSet* > tmp;
			tmp.insert(std::make_pair(fromId_toId,&ps_));
			sim_mob::Profiler STRP_Profiler(true);
			pathSetParam->storePathSet(*sql,tmp);
			pathSetParam->storeSinglePath(*sql,ps_.pathChoices);
			out.str("");
			out << "STORE_PATH:" << STRP_Profiler.endProfiling() << std::endl;
			personProfiler.addOutPut(out);
			//
			// save pathset to loacl container
			sim_mob::Profiler BPCFPS_Profiler(true);
			bool r = getBestPathChoiceFromPathSet(ps_);
			// generate all node to current end node
//				generatePathset2AllNode(toNode,st);
			if(r)
			{
				res = sim_mob::convertWaypointP2Wp(ps_.bestWayPointpathP);
			}
			out.str("");
			out << "GET_BEST_PATH_CHOICE_FROM_PATHSET:" << (r? "true" : "false") << ":" << 	BPCFPS_Profiler.endProfiling() << std::endl;
			personProfiler.addOutPut(out);
			return res;
		}
	return res;
}

bool sim_mob::PathSetManager::generateAllPathChoicesMT(PathSet* ps, Profiler & personProfiler, const std::vector<const sim_mob::RoadSegment*> & exclude_seg)
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
	std::ostringstream out("");
	std::map<std::string,SinglePath*> pool;
	Profiler GENSPFT3_Profiler(true);
	sim_mob::SinglePath *s = generateSinglePathByFromToNodes3(ps->fromNode,ps->toNode,pool,exclude_seg);
	if(!s)
	{
		// no path
		ps->has_path = -1;
		ps->isNeedSave2DB = true;
		std::map<std::string,sim_mob::PathSet* > tmp;
		tmp.insert(std::make_pair(ps->id,ps));
		sim_mob::aimsun::Loader::SaveOnePathSetData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),tmp);
		out.str("");
		out << "generateSinglePathByFromToNodes3:false:" << GENSPFT3_Profiler.endProfiling()  << std::endl;
		personProfiler.addOutPut(out);
		return false;
	}
	out.str("");
	out << "generateSinglePathByFromToNodes3:true:" << GENSPFT3_Profiler.endProfiling() << std::endl;
	personProfiler.addOutPut(out);

	//	// 1.31 check path pool
		// 1.4 create PathSet object
	ps->has_path = 1;
	ps->isNeedSave2DB = true;
	ps->oriPath = s;
	std::string fromId_toId = ps->fromNode->originalDB_ID.getLogItem() +"_"+ ps->toNode->originalDB_ID.getLogItem();
	ps->id = fromId_toId;
	ps->singlepath_id = s->id;
	s->pathset_id = ps->id;
	s->pathSet = ps;
	Profiler TC2_Profiler(true);
	s->travel_cost = sim_mob::getTravelCost2(s);
	out.str("");
	out << "TRAVEL_TIME_COST:" << TC2_Profiler.endProfiling() << std::endl;
	personProfiler.addOutPut(out);
	Profiler TT_Profiler(true);
	s->travle_time = getTravelTime(s);
	out.str("");
	out << "TRAVEL_TIME:" << TT_Profiler.endProfiling() << std::endl;
	personProfiler.addOutPut(out);

	// SHORTEST DISTANCE LINK ELIMINATION
	//declare the profiler  but dont start profiling. it will just accumulate the elapsed time of the profilers who are associated with the workers
	Profiler SDLE_Profiler(true);//start the profiler
	sim_mob::Link *l = NULL;
	std::vector<PathSetWorkerThread*> workPool;
	A_StarShortestPathImpl * impl = (A_StarShortestPathImpl*)stdir->getDistanceImpl();
	StreetDirectory::VertexDesc from = impl->DrivingVertex(*ps->fromNode);
	StreetDirectory::VertexDesc to = impl->DrivingVertex(*ps->toNode);
	StreetDirectory::Vertex* fromV = &from.source;
	StreetDirectory::Vertex* toV = &to.sink;
	for(int i=0;i<ps->oriPath->shortestWayPointpath.size();++i)
	{
		WayPoint *w = ps->oriPath->shortestWayPointpath[i];
		if (w->type_ == WayPoint::ROAD_SEGMENT && l != w->roadSegment_->getLink()) {
			const sim_mob::RoadSegment* seg = w->roadSegment_;
			PathSetWorkerThread * work = new PathSetWorkerThread();
			//introducing the profiling time accumulator
			//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
			if(serialPathSetGroup){
				work->parentProfiler = &SDLE_Profiler;
			}
			work->graph = &impl->drivingMap_;
			work->segmentLookup = &impl->drivingSegmentLookup_;
			work->fromVertex = fromV;
			work->toVertex = toV;
			work->fromNode = ps->fromNode;
			work->toNode = ps->toNode;
			work->excludeSeg = seg;
//			work->s = NULL;
			work->s->clear();
			work->ps = ps;
			threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
			workPool.push_back(work);
		} //ROAD_SEGMENT
	}

	if(serialPathSetGroup){
		threadpool_->wait();
	}
	//kep your own ending time
	SDLE_Profiler.addToTotalTime(SDLE_Profiler.endProfiling());


	// SHORTEST TRAVEL TIME LINK ELIMINATION
	Profiler STTLE_Profiler(true);
	l=NULL;
	A_StarShortestTravelTimePathImpl * sttpImpl = (A_StarShortestTravelTimePathImpl*)stdir->getTravelTimeImpl();
	from = sttpImpl->DrivingVertexNormalTime(*ps->fromNode);
	to = sttpImpl->DrivingVertexNormalTime(*ps->toNode);
	fromV = &from.source;
	toV = &to.sink;
	SinglePath *sinPathTravelTimeDefault = generateShortestTravelTimePath(ps->fromNode,ps->toNode,pool,sim_mob::Default);
	if(sinPathTravelTimeDefault)
	{
		for(int i=0;i<sinPathTravelTimeDefault->shortestWayPointpath.size();++i)
		{
			WayPoint *w = sinPathTravelTimeDefault->shortestWayPointpath[i];
			if (w->type_ == WayPoint::ROAD_SEGMENT && l != w->roadSegment_->getLink()) {
				const sim_mob::RoadSegment* seg = w->roadSegment_;
				PathSetWorkerThread *work = new PathSetWorkerThread();
				//introducing the profiling time accumulator
				//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
				if(serialPathSetGroup){
					work->parentProfiler = &STTLE_Profiler;
				}
				work->graph = &sttpImpl->drivingMap_Default;
				work->segmentLookup = &sttpImpl->drivingSegmentLookup_Default_;
				work->fromVertex = fromV;
				work->toVertex = toV;
				work->fromNode = ps->fromNode;
				work->toNode = ps->toNode;
				work->excludeSeg = seg;
//				work->s = NULL;
				work->s->clear();
				work->ps = ps;
				threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
				workPool.push_back(work);
			} //ROAD_SEGMENT
		}//for
	}//if sinPathTravelTimeDefault

	if(serialPathSetGroup){
		threadpool_->wait();
	}
	STTLE_Profiler.addToTotalTime(STTLE_Profiler.endProfiling());


	// TRAVEL TIME HIGHWAY BIAS
	//declare the profiler  but dont start profiling. it will just accumulate the elapsed time of the profilers who are associated with the workers
	Profiler STTLEH_Profiler(true);
	l=NULL;
	SinglePath *sinPathHightwayBias = generateShortestTravelTimePath(ps->fromNode,ps->toNode,pool,sim_mob::HighwayBias_Distance);
	from = sttpImpl->DrivingVertexHighwayBiasDistance(*ps->fromNode);
	to = sttpImpl->DrivingVertexHighwayBiasDistance(*ps->toNode);
	fromV = &from.source;
	toV = &to.sink;
	if(sinPathHightwayBias)
	{
		for(int i=0;i<sinPathHightwayBias->shortestWayPointpath.size();++i)
		{
			WayPoint *w = sinPathHightwayBias->shortestWayPointpath[i];
			if (w->type_ == WayPoint::ROAD_SEGMENT && l != w->roadSegment_->getLink()) {
				const sim_mob::RoadSegment* seg = w->roadSegment_;
				PathSetWorkerThread *work = new PathSetWorkerThread();
				//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
				//introducing the profiling time accumulator
				if(serialPathSetGroup){
					work->parentProfiler = &STTLEH_Profiler;
				}
				work->graph = &sttpImpl->drivingMap_HighwayBias_Distance;
				work->segmentLookup = &sttpImpl->drivingSegmentLookup_HighwayBias_Distance_;
				work->fromVertex = fromV;
				work->toVertex = toV;
				work->fromNode = ps->fromNode;
				work->toNode = ps->toNode;
				work->excludeSeg = seg;
				work->s->clear();
//				work->s = NULL;
				work->ps = ps;
				threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
				workPool.push_back(work);
			} //ROAD_SEGMENT
		}//for
	}//if sinPathTravelTimeDefault
	if(serialPathSetGroup){
		threadpool_->wait();
	}
	STTLEH_Profiler.addToTotalTime(STTLEH_Profiler.endProfiling());
	// generate random path
	Profiler randomPath_Profiler(true);
	for(int i=0;i<20;++i)
	{
		from = sttpImpl->DrivingVertexRandom(*ps->fromNode,i);
		to = sttpImpl->DrivingVertexRandom(*ps->toNode,i);
		fromV = &from.source;
		toV = &to.sink;
		PathSetWorkerThread *work = new PathSetWorkerThread();
		//introducing the profiling time accumulator
		//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
		if(serialPathSetGroup){
			work->parentProfiler = &randomPath_Profiler;
		}
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
		threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
		workPool.push_back(work);
	}
	//WAITING FOR THREAPOOL TO END HERE
	threadpool_->wait();
	randomPath_Profiler.addToTotalTime(randomPath_Profiler.endProfiling());
	//now that all the threads have concluded, get the total times
	out.str("");
	out << "SHORTEST_DISTANCE_LE:" << SDLE_Profiler.getTotalTime() << std::endl;
	out << "SHORTEST_TRAVEL_TIME_LE:" << STTLE_Profiler.getTotalTime() << std::endl;
	out << "SHORTEST_TRAVEL_TIME_LE_HIGHWAY_BIAS:" << STTLEH_Profiler.getTotalTime() << std::endl;
	out << "RANDOM_PATH:" << randomPath_Profiler.getTotalTime() << std::endl;
	personProfiler.addOutPut(out);
	//record
	//a.record the shortest path with all segments
	ps->SinglePathPool.insert(std::make_pair(ps->oriPath->id,ps->oriPath));
	//b.record the rest of paths
	for(int i=0;i<workPool.size();++i)
	{
		if(workPool[i]->hasPath)
		{
			std::map<std::string,sim_mob::SinglePath*>::iterator it = ps->SinglePathPool.find(workPool[i]->s->id);
			if(it==ps->SinglePathPool.end())
			{
				// this sp not store before
				ps->SinglePathPool.insert(std::make_pair(workPool[i]->s->id,workPool[i]->s));
			}
		}
	}
	return true;
}
void sim_mob::PathSetManager::generatePathesByLinkElimination(std::vector<WayPoint*>& path,
			std::map<std::string,SinglePath*>& wp_spPool,
			sim_mob::PathSet& ps_,
			const sim_mob::Node* fromNode,
			const sim_mob::Node* toNode)
{
	for(int i=0;i<path.size();++i)
	{
		WayPoint *w = path[i];
		if (w->type_ == WayPoint::ROAD_SEGMENT) {
			std::vector<const sim_mob::RoadSegment*> seg ;
			seg.push_back(w->roadSegment_);
			SinglePath *sinPath = generateSinglePathByFromToNodes3(fromNode,toNode,wp_spPool,seg);
			if(!sinPath)
			{
				continue;
			}
			sinPath->pathSet = &ps_; // set parent
			sinPath->travel_cost = getTravelCost2(sinPath);
			sinPath->travle_time = getTravelTime(sinPath);
			sinPath->pathset_id = ps_.id;
			ps_.pathChoices.push_back(sinPath);
		}
	}//end for
}
void sim_mob::PathSetManager::generatePathesByTravelTimeLinkElimination(std::vector<WayPoint*>& path,
				std::map<std::string,SinglePath*>& wp_spPool,
				sim_mob::PathSet& ps_,
				const sim_mob::Node* fromNode,
				const sim_mob::Node* toNode,
				sim_mob::TimeRange tr)
{
	for(int i=0;i<path.size();++i)
	{
		WayPoint *w = path[i];
		if (w->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w->roadSegment_;
			SinglePath *sinPath = generateShortestTravelTimePath(fromNode,toNode,wp_spPool,tr,seg);
			if(!sinPath)
			{
				continue;
			}
			sinPath->pathSet = &ps_; // set parent
			sinPath->travel_cost = getTravelCost2(sinPath);
			sinPath->travle_time = getTravelTime(sinPath);
			sinPath->pathset_id = ps_.id;
//			storePath(sinPath);
			ps_.pathChoices.push_back(sinPath);
		}
	}//end for
}
void sim_mob::PathSetManager::generateTravelTimeSinglePathes(const sim_mob::Node *fromNode,
		   const sim_mob::Node *toNode,
		   std::map<std::string,SinglePath*>& wp_spPool,sim_mob::PathSet& ps_)
{
	SinglePath *sinPath_morningPeak = generateShortestTravelTimePath(fromNode,toNode,wp_spPool,sim_mob::MorningPeak);
	if(sinPath_morningPeak)
	{
		sinPath_morningPeak->pathSet = &ps_; // set parent
		sinPath_morningPeak->travel_cost = getTravelCost2(sinPath_morningPeak);
		sinPath_morningPeak->travle_time = getTravelTime(sinPath_morningPeak);
		sinPath_morningPeak->pathset_id = ps_.id;
		ps_.pathChoices.push_back(sinPath_morningPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_morningPeak->shortestWayPointpath,wp_spPool,ps_,fromNode,toNode,sim_mob::MorningPeak);
	}
	SinglePath *sinPath_eveningPeak = generateShortestTravelTimePath(fromNode,toNode,wp_spPool,sim_mob::EveningPeak);
	if(sinPath_eveningPeak)
	{
		sinPath_eveningPeak->pathSet = &ps_; // set parent
		sinPath_eveningPeak->travel_cost = getTravelCost2(sinPath_eveningPeak);
		sinPath_eveningPeak->travle_time = getTravelTime(sinPath_eveningPeak);
		sinPath_eveningPeak->pathset_id = ps_.id;
		ps_.pathChoices.push_back(sinPath_eveningPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_eveningPeak->shortestWayPointpath,wp_spPool,ps_,fromNode,toNode,sim_mob::EveningPeak);
	}
	SinglePath *sinPath_offPeak = generateShortestTravelTimePath(fromNode,toNode,wp_spPool,sim_mob::OffPeak);
	if(sinPath_offPeak)
	{
		sinPath_offPeak->pathSet = &ps_; // set parent
		sinPath_offPeak->travel_cost = getTravelCost2(sinPath_offPeak);
		sinPath_offPeak->travle_time = getTravelTime(sinPath_offPeak);
		sinPath_offPeak->pathset_id = ps_.id;
		ps_.pathChoices.push_back(sinPath_offPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_offPeak->shortestWayPointpath,wp_spPool,ps_,fromNode,toNode,sim_mob::OffPeak);
	}
	SinglePath *sinPath_default = generateShortestTravelTimePath(fromNode,toNode,wp_spPool,sim_mob::Default);
	if(sinPath_default)
	{
		sinPath_default->pathSet = &ps_; // set parent
		sinPath_default->travel_cost = getTravelCost2(sinPath_default);
		sinPath_default->travle_time = getTravelTime(sinPath_default);
		sinPath_default->pathset_id = ps_.id;
		ps_.pathChoices.push_back(sinPath_default);
		generatePathesByTravelTimeLinkElimination(sinPath_default->shortestWayPointpath,wp_spPool,ps_,fromNode,toNode,sim_mob::Default);
	}
	// generate high way bias path
	SinglePath *sinPath = generateShortestTravelTimePath(fromNode,toNode,wp_spPool,sim_mob::HighwayBias_Distance);
	if(sinPath)
	{
		sinPath->pathSet = &ps_; // set parent
		sinPath->travel_cost = getTravelCost2(sinPath);
		sinPath->travle_time = getTravelTime(sinPath);
		sinPath->pathset_id = ps_.id;
		ps_.pathChoices.push_back(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,wp_spPool,ps_,fromNode,toNode,sim_mob::HighwayBias_Distance);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,wp_spPool,sim_mob::HighwayBias_MorningPeak);
	if(sinPath)
	{
		sinPath->pathSet = &ps_; // set parent
		sinPath->travel_cost = getTravelCost2(sinPath);
		sinPath->travle_time = getTravelTime(sinPath);
		sinPath->pathset_id = ps_.id;
		ps_.pathChoices.push_back(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,wp_spPool,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,wp_spPool,sim_mob::HighwayBias_EveningPeak);
	if(sinPath)
	{
		sinPath->pathSet = &ps_; // set parent
		sinPath->travel_cost = getTravelCost2(sinPath);
		sinPath->travle_time = getTravelTime(sinPath);
		sinPath->pathset_id = ps_.id;
		ps_.pathChoices.push_back(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,wp_spPool,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,wp_spPool,sim_mob::HighwayBias_OffPeak);
	if(sinPath)
	{
		sinPath->pathSet = &ps_; // set parent
		sinPath->travel_cost = getTravelCost2(sinPath);
		sinPath->travle_time = getTravelTime(sinPath);
		sinPath->pathset_id = ps_.id;
		ps_.pathChoices.push_back(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,wp_spPool,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,wp_spPool,sim_mob::HighwayBias_Default);
	if(sinPath)
	{
		sinPath->pathSet = &ps_; // set parent
		sinPath->travel_cost = getTravelCost2(sinPath);
		sinPath->travle_time = getTravelTime(sinPath);
		sinPath->pathset_id = ps_.id;
		ps_.pathChoices.push_back(sinPath);
		//
		generatePathesByTravelTimeLinkElimination(sinPath->shortestWayPointpath,wp_spPool,ps_,fromNode,toNode,sim_mob::HighwayBias_Default);
	}
	// generate random path
	for(int i=0;i<20;++i)
	{
		const sim_mob::RoadSegment *rs = NULL;
		sinPath = generateShortestTravelTimePath(fromNode,toNode,wp_spPool,sim_mob::Random,rs,i);
		if(sinPath)
		{
			sinPath->pathSet = &ps_; // set parent
			sinPath->travel_cost = getTravelCost2(sinPath);
			sinPath->travle_time = getTravelTime(sinPath);
			sinPath->pathset_id = ps_.id;
			ps_.pathChoices.push_back(sinPath);
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


bool sim_mob::PathSetManager::getBestPathChoiceFromPathSet(sim_mob::PathSet& ps)
{
	// path choice algorithm
	if(!ps.oriPath)
	{
		Print()<<"warning gBPCFromPS: ori path empty"<<std::endl;
		return false;
	}
	int gdata=0;
	if(gdata)
	{
		ps.bestWayPointpathP = ps.oriPath->shortestWayPointpath;
		return true;
	}
	// step 1.1 : For each path i in the path choice:
	//1. set PathSet(O, D)
	//2. travle_time
	//3. utility
	//step 1.2 : accumulate the logsum
	ps.logsum = 0.0;
	for(int i=0;i<ps.pathChoices.size();++i)
	{
		SinglePath* sp = ps.pathChoices[i];
		if(sp)
		{
			sp->pathSet = &ps;
			sp->travle_time = getTravelTime(sp);
			sp->utility = getUtilityBySinglePath(sp);
			ps.logsum += exp(sp->utility);
		}
	}
	// step 2: find the best waypoint path :
		// calculate a probability using path's utility and pathset's logsum,
		// compare the resultwith a  random number to decide whether pick the current path as the best path or not
		//if not, just chose the shortest path as the best path
	double upperProb=0;
	// 2.1 Draw a random number X between 0.0 and 1.0 for agent A.
	//double x = sim_mob::gen_random_float(0,1);
	//Below is a temporary hack. commenting the line above and replacing with
	//next temporarily for the 8-July-2012 ~melani
	double x = -0.1;
	// 2.2 For each path i in the path choice set PathSet(O, D):
	for(int i=0;i<ps.pathChoices.size();++i)
	{
		SinglePath* sp = ps.pathChoices[i];
		if(sp)
		{
			double prob = sp->utility/(ps.logsum+0.00001);
			upperProb += prob;
			if (x<upperProb)
			{
				// 2.3 agent A chooses path i from the path choice set.
				ps.bestWayPointpathP = sp->shortestWayPointpath;
				return true;
			}
		}
	}
	// have to has a path
	ps.bestWayPointpathP = ps.oriPath->shortestWayPointpath;
	return true;
}
void sim_mob::PathSetManager::initParameters()
{
	isUseCache=pathSetParam->isUseCache;

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
		   std::map<std::string,SinglePath*>& wp_spPool,
		   const std::vector<const sim_mob::RoadSegment*> & exclude_seg)
{
	/**
	 * step-1: find the shortest driving path between the given OD
	 * step-2: turn the outputted waypoint container into a string tobe used as an id
	 * step-3: create a new Single path object
	 * step-4: return the resulting singlepath object as well as add it to the container supplied through args
	 */
	sim_mob::SinglePath *s=NULL;
	std::vector<const sim_mob::RoadSegment*> blacklist;
	if(exclude_seg.size())
	{
		const sim_mob::RoadSegment* rs;
		BOOST_FOREACH(rs, exclude_seg){
			blacklist.push_back(rs);
		}
	}
	std::vector<WayPoint> wp = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*fromNode), stdir->DrivingVertex(*toNode),blacklist);
	if(wp.size()==0)
	{
		// no path
		if(exclude_seg.size())
		{
			const sim_mob::RoadSegment* rs;
			BOOST_FOREACH(rs, exclude_seg){
			Print()<<"gSPByFTNodes3: no path for nodes["<<fromNode->originalDB_ID.getLogItem()<< "] and [" <<
				toNode->originalDB_ID.getLogItem()<< " and ex seg[" <<
				rs->originalDB_ID.getLogItem()<< "]" << std::endl;
			}
		}
		else
		{
			Print()<<"gSPByFTNodes3: no path for nodes["<<fromNode->originalDB_ID.getLogItem()<< "] and [" <<
							toNode->originalDB_ID.getLogItem()<< "]" << std::endl;
		}
		return s;
	}
	// make sp id
	std::string id = sim_mob::makeWaypointsetString(wp);
	// 1.31 check path pool
	std::map<std::string,SinglePath*>::iterator it =  wp_spPool.find(id);
	// no stored path found, generate new one
	if(it==wp_spPool.end())
	{
		s = new SinglePath();
		// fill data
		s->isNeedSave2DB = true;
		s->init(wp);
		sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(s);
		s->fromNode = fromNode;
		s->toNode = toNode;
		s->pathSet = NULL;
		s->length = sim_mob::generateSinglePathLengthPT(s->shortestWayPointpath);
		s->id = id;
		s->scenario = scenarioName;
		s->pathsize=0;
		wp_spPool.insert(std::make_pair(id,s));
	}

	return s;
}
sim_mob::SinglePath* sim_mob::PathSetManager::generateShortestTravelTimePath(const sim_mob::Node *fromNode,
			   const sim_mob::Node *toNode,
			   std::map<std::string,SinglePath*>& wp_spPool,
			   sim_mob::TimeRange tr,
			   const sim_mob::RoadSegment* exclude_seg,int random_graph_idx)
{
	sim_mob::SinglePath *s=NULL;
		std::vector<const sim_mob::RoadSegment*> blacklist;
		if(exclude_seg)
		{
			blacklist.push_back(exclude_seg);
		}
		std::vector<WayPoint> wp = stdir->SearchShortestDrivingTimePath(stdir->DrivingTimeVertex(*fromNode,tr,random_graph_idx),
				stdir->DrivingTimeVertex(*toNode,tr,random_graph_idx),
				blacklist,
				tr,
				random_graph_idx);
		if(wp.size()==0)
		{
			// no path
			Print()<<"generateShortestTravelTimePath: no path for nodes"<<fromNode->originalDB_ID.getLogItem()<<
							toNode->originalDB_ID.getLogItem()<<std::endl;
			return s;
		}
		// make sp id
		std::string id = sim_mob::makeWaypointsetString(wp);
		// 1.31 check path pool
		std::map<std::string,SinglePath*>::iterator it =  wp_spPool.find(id);
		// no stored path found, generate new one
		if(it==wp_spPool.end())
		{
			s = new SinglePath();
			// fill data
			s->isNeedSave2DB = true;
			s->init(wp);
			sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(s);
			s->fromNode = fromNode;
			s->toNode = toNode;
			s->pathSet = NULL;
			s->length = sim_mob::generateSinglePathLengthPT(s->shortestWayPointpath);
			s->id = id;
			s->scenario = scenarioName;
			s->pathsize=0;
			wp_spPool.insert(std::make_pair(id,s));
		}

		return s;
}
sim_mob::SinglePath * sim_mob::PathSetManager::generateSinglePathByFromToNodes(const sim_mob::Node *fromNode,
		   const sim_mob::Node *toNode,const sim_mob::RoadSegment* exclude_seg)
{
	sim_mob::SinglePath *s=NULL;
	std::vector<const sim_mob::RoadSegment*> blacklist;
	if(exclude_seg)
	{
		blacklist.push_back(exclude_seg);
	}
	std::vector<WayPoint> wp = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*fromNode), stdir->DrivingVertex(*toNode),blacklist);
	if(wp.size()==0)
	{
		// no path
		if(exclude_seg)
		{
			Print()<<"gSPByFTNodes: no path for nodes and ex seg"<<fromNode->originalDB_ID.getLogItem()<<
				toNode->originalDB_ID.getLogItem()<<
				exclude_seg->originalDB_ID.getLogItem()<<std::endl;
		}
		else
		{
			Print()<<"gSPByFTNodes: no path for nodes"<<fromNode->originalDB_ID.getLogItem()<<
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
		s->shortestWayPointpath = convertWaypoint2Point(wp);//stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*fromNode), stdir->DrivingVertex(*toNode),blacklist);
		s->shortestSegPath = sim_mob::generateSegPathByWaypointPathP(s->shortestWayPointpath);
		sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(s);
		s->fromNode = fromNode;
		s->toNode = toNode;
		s->pathSet = NULL;
		s->length = sim_mob::generateSinglePathLengthPT(s->shortestWayPointpath);
		s->id = id;
		s->scenario = scenarioName;
		s->pathsize=0;
	}
	return s;
}
sim_mob::PathSet *sim_mob::PathSetManager::generatePathSetByFromToNodes(const sim_mob::Node *fromNode,
														   const sim_mob::Node *toNode,
														   const sim_mob::SubTrip* st,
														   bool isUseCache)
{
	// 0. check pathSetPool already calculate before with this pair
	std::string fromId_toId = fromNode->originalDB_ID.getLogItem() +"_"+ toNode->originalDB_ID.getLogItem();
	// 0.1 no data in memory, so check db
	std::string mys=fromId_toId;
	mys = "'"+mys+"'";
	PathSet *ps = NULL;
	// check cache first
	if(isUseCache)
	{
		ps = getPathSetByFromToNodeAimsunId(fromId_toId);
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
		bool res = sim_mob::aimsun::Loader::LoadPathSetDBwithId(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),
				pathSetPool,mys);
		if(res)
		{
			ps = getPathSetByFromToNodeAimsunId(fromId_toId);
			// 0.2 no data in db, so create new
			if(ps)
			{
				if(!ps->isInit)
				{
				// get all relative singlepath
					std::vector<sim_mob::SinglePath*> allChoices;
					LoadSinglePathDBwithId(mys,ps->pathChoices);
					// 2. get SinglePath
					if(!ps->oriPath)
					{
						std::string str = "PathSet: oriPath not find,no path for this nodes pair ";
										Print()<<str<<std::endl;
					}
					ps->isInit = true;
				}
				return ps;
			}
			else
			{
				Print()<<"gPSByFTNodes: data in db ,but not in mem?  "<<fromId_toId<<std::endl;
			}
		}
	}
	Print()<<"gPSByFTNodes: create data for "<<fromId_toId<<std::endl;
	// 1. generate shortest path with all segs
	// 1.1 check StreetDirectory
	if(!stdir || !roadNetwork)
	{
		throw std::runtime_error("StreetDirectory or RoadNetwork is null");
	}
	// 1.2 get all segs
	// 1.3 generate shortest path with full segs
	sim_mob::SinglePath *s = generateSinglePathByFromToNodes(fromNode,toNode);
	if(!s)
	{
		// no path
		return ps;
	}
	// 1.4 create PathSet object
	ps = new PathSet(fromNode,toNode);
	ps->subTrip = st;
	ps->isNeedSave2DB = true;
	ps->oriPath = s;
	ps->id = fromId_toId;
	ps->singlepath_id = s->id;
	s->pathset_id = ps->id;
	s->pathSet = ps;
	s->travel_cost = getTravelCost2(s);
	s->travle_time = getTravelTime(s);
	ps->pathChoices.push_back(s);
	// 2. exclude each seg in shortest path, then generate new shortest path
	for(int i=0;i<s->shortestWayPointpath.size();++i)
	{
		WayPoint *w = s->shortestWayPointpath[i];
		if (w->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w->roadSegment_;
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
			ps->pathChoices.push_back(sinPath);
		}
	}//end for
	// calculate oriPath's path size

	ps->from_node_id = fromNode->originalDB_ID.getLogItem();
	ps->to_node_id = toNode->originalDB_ID.getLogItem();
	ps->scenario = scenarioName;
	// 3. store pathset
	sim_mob::generatePathSizeForPathSet2(ps);
	if(isUseCache)
	{
		pathSetPool.insert(std::make_pair(fromId_toId,ps));
	}
	std::map<std::string,sim_mob::PathSet* > tmp;
	tmp.insert(std::make_pair(fromId_toId,ps));
	sim_mob::aimsun::Loader::SaveOnePathSetData(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false),tmp);

	return ps;
}
std::string sim_mob::makeWaypointsetString(std::vector<WayPoint>& wp)
{
	std::string str;
	if(wp.size()==0)
	{
		Print()<<"warning: makeWaypointsetString"<<std::endl;
	}
	for(std::vector<WayPoint>::iterator it = wp.begin(); it != wp.end(); it++)
	{
		int i=0;
		if (it->type_ == WayPoint::ROAD_SEGMENT) {
			std::string tmp = it->roadSegment_->originalDB_ID.getLogItem();
			if(i==0)
			{
				str += getNumberFromAimsunId(tmp) + "_";
			}
			else
			{
				str += "_"+getNumberFromAimsunId(tmp);
			}
			i++;
		} // if ROAD_SEGMENT
	}
	if(str.size()<1)
	{
		// when same f,t node, it happened
		Print()<<"warning: makeWaypointsetString id"<<std::endl;
	}
	return str;
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

void sim_mob::generatePathSizeForPathSet2(sim_mob::PathSet *ps,bool isUseCache)
{
	// Step 1: the length of each path in the path choice set
	double minL = ps->oriPath->length;

	// find MIN_TRAVEL_TIME
	double minTravelTime=99999999.0;
	int idx = 0; // record which is min
	for(int i = 0;i<ps->pathChoices.size();++i)
	{
		sim_mob::SinglePath* sp = ps->pathChoices[i];
		if(i==0)
		{
			minTravelTime = sp->travle_time;
			idx = 0;
		}
		else
		{
			if(sp->travle_time < minTravelTime)
			{
				minTravelTime = sp->travle_time;
				idx = i;
			}
		}
	}
	if(!ps->pathChoices.empty())
	{
		ps->pathChoices[idx]->isMinTravelTime = 1;
	}
	// find MIN_DISTANCE
	double minDistance=99999999.0;
	idx = 0; // record which is min
	for(int i = 0;i<ps->pathChoices.size();++i)
	{
		sim_mob::SinglePath* sp = ps->pathChoices[i];
		if(i==0)
		{
			minDistance = sp->length;
			idx = 0;
		}
		else
		{
			if(sp->length < minDistance)
			{
				minTravelTime = sp->length;
				idx = i;
			}
		}
	}
	if(!ps->pathChoices.empty())
	{
		ps->pathChoices[idx]->isMinDistance = 1;
	}
	// find MIN_SIGNAL
	int minSignal=99999999;
	idx = 0; // record which is min
	for(int i = 0;i<ps->pathChoices.size();++i)
	{
		sim_mob::SinglePath* sp = ps->pathChoices[i];
		if(i==0)
		{
			minSignal = sp->signal_number;
			idx = 0;
		}
		else
		{
			if(sp->signal_number < minSignal)
			{
				minSignal = sp->signal_number;
				idx = i;
			}
		}
	}
	if(!ps->pathChoices.empty())
	{
		ps->pathChoices[idx]->isMinSignal = 1;
	}
	// find MIN_RIGHT_TURN
	int minRightTurn=99999999;
	idx = 0; // record which is min
	for(int i = 0;i<ps->pathChoices.size();++i)
	{
		sim_mob::SinglePath* sp = ps->pathChoices[i];
		if(i==0)
		{
			minRightTurn = sp->right_turn_number;
			idx = 0;
		}
		else
		{
			if(sp->right_turn_number < minRightTurn)
			{
				minSignal = sp->right_turn_number;
				idx = i;
			}
		}
	}
	if(!ps->pathChoices.empty())
	{
		ps->pathChoices[idx]->isMinRightTurn = 1;
	}
	// find MAX_HIGH_WAY_USAGE
	double maxHighWayUsage=0.0;
	idx = 0; // record which is min
	for(int i = 0;i<ps->pathChoices.size();++i)
	{
		sim_mob::SinglePath* sp = ps->pathChoices[i];
		if(i==0)
		{
			maxHighWayUsage = sp->highWayDistance / sp->length;
			idx = 0;
		}
		else
		{
			if(sp->highWayDistance / sp->length > maxHighWayUsage)
			{
				maxHighWayUsage = sp->highWayDistance / sp->length;
				idx = i;
			}
		}
	}
	if(!ps->pathChoices.empty())
	{
		ps->pathChoices[idx]->isMaxHighWayUsage = 1;
	}
	//
	for(int i = 0;i<ps->pathChoices.size();++i)
	{
		//Set size = 0.
		double size=0;
		sim_mob::SinglePath* sp = ps->pathChoices[i];
		if(!sp)
		{
			continue;
		}
		//For each link a in the path:
		for(std::map<const RoadSegment*,WayPoint*>::iterator it1 = sp->shortestSegPath.begin();
				it1 != sp->shortestSegPath.end(); it1++)
		{
			const sim_mob::RoadSegment* seg = (*it1).first;
				double l=seg->length;
				//Set sum = 0.
				double sum=0;
				//For each path j in the path choice set PathSet(O, D):
				for(int j = 0;j<ps->pathChoices.size();++j)
				{
					sim_mob::SinglePath* spj = ps->pathChoices[j];
					std::map<const RoadSegment*,WayPoint*>::iterator itt2 = spj->shortestSegPath.find(seg);
					if(itt2!=spj->shortestSegPath.end())
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

inline std::map<const RoadSegment*,WayPoint*> sim_mob::generateSegPathByWaypointPathP(std::vector<WayPoint*>& wp)
{
	std::map<const RoadSegment*,WayPoint*> res;
	for (int i = 0;i< wp.size();++i) {
		WayPoint *w = wp[i];
		if (w->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w->roadSegment_;
			res.insert(std::make_pair(seg,w));
		}
	}
	return res;
}

inline void sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(sim_mob::SinglePath *sp)
{
	if(sp->shortestWayPointpath.size()<2)
	{
		sp->right_turn_number=0;
		sp->signal_number=0;
		return ;
	}
	int res=0;
	int signalNumber=0;
	std::map<const RoadSegment*,WayPoint*>::iterator itt=sp->shortestSegPath.begin();
	++itt;
	for(std::map<const RoadSegment*,WayPoint*>::iterator it=sp->shortestSegPath.begin();
			it!=sp->shortestSegPath.end();
			++it)
	{
		const RoadSegment* currentSeg = (*it).first;
		const RoadSegment* targetSeg = NULL;
		if(itt!=sp->shortestSegPath.end())
		{

			targetSeg = (*itt).first;
		}
		else // already last segment
		{
			break;
		}
		const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (currentSeg->getEnd());
		if(currEndNode) // intersection
		{
			signalNumber++;
			// get lane connector
			const std::set<sim_mob::LaneConnector*>& lcs = currEndNode->getOutgoingLanes(currentSeg);
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
//TODO:I think lane index should be a data member in the lane class
size_t sim_mob::getLaneIndex2(const Lane* l) {
	if (l) {
		const RoadSegment* r = l->getRoadSegment();
		for (size_t i = 0; i < r->getLanes().size(); i++) {
			if (r->getLanes().at(i) == l) {
				return i;
			}
		}
	}
	return -1; //NOTE: This might not do what you expect! ~Seth
}
sim_mob::PathSet* sim_mob::PathSetManager::getPathSetByFromToNodeAimsunId(std::string id)
{
	std::map<std::string,sim_mob::PathSet* >::iterator it = pathSetPool.find(id);
	if(it == pathSetPool.end())
	{
		Print()<<"getPSByAId: no pathset found for "<<id<<std::endl;
		return NULL;
	}
	else
	{
//		sim_mob::PathSet* p = new sim_mob::PathSet((*it).second);
		sim_mob::PathSet* p = (*it).second;
		return p;
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


double sim_mob::getTravelCost2(sim_mob::SinglePath *sp)
{
	double res=0.0;
	double ts=0.0;
	if(!sp) Print()<<"gTC: sp is empty"<<std::endl;
//	std::map<const RoadSegment*,WayPoint> shortestSegPath = generateSegPathByWaypointPath(sp->shortestWayPointpath);
	sim_mob::DailyTime trip_startTime = sp->pathSet->subTrip->startTime;
	for(std::map<const RoadSegment*,WayPoint*>::iterator it1 = sp->shortestSegPath.begin();
					it1 != sp->shortestSegPath.end(); it1++)
	{
		const sim_mob::RoadSegment* seg = (*it1).first;
		std::string seg_id = seg->originalDB_ID.getLogItem();
//		Print()<<"getTravelCost: "<<seg_id<<std::endl;
		std::map<std::string,sim_mob::ERP_Section*>::iterator it = sim_mob::PathSetParam::getInstance()->ERP_Section_pool.find(seg_id);
		//get travel time to this segment
		double t = sim_mob::PathSetParam::getInstance()->getTravelTimeBySegId(seg_id,trip_startTime);
		ts += t;
		trip_startTime = trip_startTime + sim_mob::DailyTime(ts*1000);
		if(it!=sim_mob::PathSetParam::getInstance()->ERP_Section_pool.end())
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
					if( s->start_time_dt.isBeforeEqual(trip_startTime) && s->end_time_dt.isAfter(trip_startTime) &&
							s->Vehicle_Type_Id == 1 && s->Day == "Weekdays")
					{
						res += s->Rate;
					}
				}
			}
		}
	}

	return res;
}
double sim_mob::PathSetManager::getTravelTime(sim_mob::SinglePath *sp)
{
	double ts=0.0;
	sim_mob::DailyTime startTime = sp->pathSet->subTrip->startTime;
	for(int i=0;i<sp->shortestWayPointpath.size();++i)
	{
		WayPoint *w = sp->shortestWayPointpath[i];
		if (w->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w->roadSegment_;
			std::string seg_id = seg->originalDB_ID.getLogItem();
			double t = getTravelTimeBySegId(seg_id,startTime);
			ts += t;
			startTime = startTime + sim_mob::DailyTime(ts*1000);
		}
	}
	return ts;
}
double sim_mob::PathSetManager::getTravelTimeBySegId(std::string id,sim_mob::DailyTime startTime)
{
	//testing
	return 100.0;
	//1. check realtime table
	double res=0.0;
	std::map<std::string,std::vector<sim_mob::Link_travel_time*> >::iterator it =
			Link_realtime_travel_time_pool.find(id);
	if(it!=Link_realtime_travel_time_pool.end())
	{
		std::vector<sim_mob::Link_travel_time*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::Link_travel_time* l = e[i];
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
		std::vector<sim_mob::Link_travel_time*> e = (*it).second;
		for(int i=0;i<e.size();++i)
		{
			sim_mob::Link_travel_time* l = e[i];
			if( l->start_time_dt.isBeforeEqual(startTime) && l->end_time_dt.isAfter(startTime) )
			{
				res = l->travel_time;
				return res;
			}
		}
	}
	else
	{
		std::string str = "PathSetManager::getTravelTimeBySegId=> no travel time for segment " + id;
		Print()<<"error: "<<str<<std::endl;
	}
	return res;
}


sim_mob::SinglePath::SinglePath(SinglePath& source) :
		id(source.id),
		utility(source.utility),pathsize(source.pathsize),
		travel_cost(source.travel_cost),
		signal_number(source.signal_number),
		right_turn_number(source.right_turn_number),
		length(source.length),travle_time(source.travle_time),
		pathset_id(source.pathset_id),highWayDistance(source.highWayDistance),
		isMinTravelTime(source.isMinTravelTime),isMinDistance(source.isMinDistance),isMinSignal(source.isMinSignal),
		isMinRightTurn(source.isMinRightTurn),isMaxHighWayUsage(source.isMaxHighWayUsage)
{
	isNeedSave2DB=false;

	purpose = sim_mob::work;

	//use id to build shortestWayPointpath
	std::vector<std::string> segIds;
	boost::split(segIds,source.id,boost::is_any_of("_"));
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
				Print()<<"error: "<<str<<std::endl;
			}
			sim_mob::WayPoint *w = sim_mob::PathSetParam::getInstance()->getWayPointBySeg(seg);
			this->shortestWayPointpath.push_back(w);
			shortestSegPath.insert(std::make_pair(seg,w));
		}
	}
}

void sim_mob::SinglePath::init(std::vector<WayPoint>& wpPools)
{
	int j=0;
	for(int i=0;i<wpPools.size();++i)
	{
		if (wpPools[i].type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = wpPools[i].roadSegment_;
			// find waypoint point from pool
			WayPoint *wp_ = sim_mob::PathSetParam::getInstance()->getWayPointBySeg(seg);
			//convertWaypoint2Point
			this->shortestWayPointpath.push_back(wp_);
			//generateSegPathByWaypointPathP
			this->shortestSegPath.insert(std::make_pair(seg,wp_));
		}
	}
}

void sim_mob::SinglePath::clear()
{
	shortestWayPointpath.clear();
	shortestSegPath.clear();
	id="";
	pathset_id="";
	pathSet = NULL;
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

sim_mob::PathSet::~PathSet()
{
	fromNode = NULL;
	toNode = NULL;
	subTrip = NULL;
}

sim_mob::PathSet::PathSet(const PathSet &ps) :
		logsum(ps.logsum),oriPath(ps.oriPath),
		subTrip(ps.subTrip),
		id(ps.id),
		from_node_id(ps.from_node_id),
		to_node_id(ps.to_node_id),
		scenario(ps.scenario),
		pathChoices(ps.pathChoices),
		singlepath_id(ps.singlepath_id),
		has_path(ps.has_path)
{
	isNeedSave2DB=false;
	isInit = false;
	hasBestChoice = false;
	// 1. get from to nodes
	//	can get nodes later,when insert to personPathSetPool
	this->fromNode = sim_mob::PathSetParam::getInstance()->getNodeByAimsunId(from_node_id);
	this->toNode = sim_mob::PathSetParam::getInstance()->getNodeByAimsunId(to_node_id);
//	// get all relative singlepath
}

sim_mob::ERP_Section::ERP_Section(ERP_Section &src)
	: section_id(src.section_id),ERP_Gantry_No(src.ERP_Gantry_No),
	  ERP_Gantry_No_str(boost::lexical_cast<std::string>(src.ERP_Gantry_No))
{
	originalSectionDB_ID.setProps("aimsun-id",src.section_id);
}

sim_mob::Link_travel_time::Link_travel_time(Link_travel_time& src)
	: link_id(src.link_id),
			start_time(src.start_time),end_time(src.end_time),travel_time(src.travel_time),
			start_time_dt(sim_mob::DailyTime(src.start_time)),end_time_dt(sim_mob::DailyTime(src.end_time))
{
	originalSectionDB_ID.setProps("aimsun-id",src.link_id);
}



/***********************************************************************************************************
 * **********************************  UNUSED CODE!!!! *****************************************************
************************************************************************************************************/
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
	Print()<<"table name: "<<pathset_traveltime_tmp_table_name<<std::endl;
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

bool sim_mob::PathSetManager::generateAllPathSetWithTripChain()
{
	const std::map<std::string, std::vector<sim_mob::TripChainItem*> > *tripChainPool =
			&ConfigManager::GetInstance().FullConfig().getTripChains();
	Print()<<"generateAllPathSetWithTripChain: trip chain pool size "<<
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
		std::map<std::string,sim_mob::PathSet*> subTripId_pathSet = generatePathSetByTripChainItemPool(tci);
		storePersonIdPathSets(personId,subTripId_pathSet);
		i++;
		Print()<<"generateAllPathSetWithTripChain: "<<i<<"/"<<poolsize<<" mem: "<<size()/1024/1024<<"mb"<<std::endl;
	}
	//
	return res;
}

void sim_mob::PathSetManager::generateAllPathSetWithTripChainPool(std::map<std::string, std::vector<sim_mob::TripChainItem*> > *tripChainPool)
{
	// 1. get from and to node
	std::map<std::string, std::vector<sim_mob::TripChainItem*> >::iterator it;
	for(it = tripChainPool->begin();it!=tripChainPool->end();++it)
	{
		std::string personId = (*it).first;
		std::vector<sim_mob::TripChainItem*> tci = (*it).second;
		std::map<std::string,sim_mob::PathSet*> subTripId_pathSet = generatePathSetByTripChainItemPool(tci);
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
		Print()<<"storePath: id wrong, ignore"<<std::endl;
	}
}


sim_mob::PathSet* sim_mob::PathSetManager::generatePathSetBySubTrip(const sim_mob::SubTrip* st)
{
	std::string subTripId = st->tripID;
	sim_mob::PathSet *ps = NULL;
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

vector<WayPoint> sim_mob::PathSetManager::generateBestPathChoice(sim_mob::Person* per,
		sim_mob::PathSet* ps,
		bool isReGenerate)
{
	vector<WayPoint> res;
	if(!ps)
	{
		return res;
	}
	if(ps->hasBestChoice && !isReGenerate)
	{
		return sim_mob::convertWaypointP2Wp(ps->bestWayPointpathP);
	}
	//TODO: path choice algorithm
	if(ps->oriPath)
	{
		res = sim_mob::convertWaypointP2Wp(ps->oriPath->shortestWayPointpath);
	}
	else
	{
		Print()<<"warning gBestPC: ori path empty"<<std::endl;
		return res;
	}
#if 1
	// step 1.For each path i in the path choice set PathSet(O, D):
	ps->logsum = 0.0;
	for(int i=0;i<ps->pathChoices.size();++i)
	{
		SinglePath* sp = ps->pathChoices[i];
		if(sp)
		{
			sp->utility = getUtilityBySinglePath(sp);
			sp->utility = exp(sp->utility);
			ps->logsum += sp->utility;
		}
	}
	// step 2:
	double upperProb=0;
	// 2.1 Draw a random number X between 0.0 and 1.0 for agent A.
	double x = sim_mob::gen_random_float(0,1);
	// 2.2 For each path i in the path choice set PathSet(O, D):
	for(int i=0;i<ps->pathChoices.size();++i)
	{
		SinglePath* sp = ps->pathChoices[i];
		if(sp)
		{
			double prob = sp->utility/(ps->logsum+0.00001);
			upperProb += prob;
			if (x<upperProb)
			{
				// 2.3 agent A chooses path i from the path choice set.
				ps->bestWayPointpathP = sp->shortestWayPointpath;
				res = sim_mob::convertWaypointP2Wp(sp->shortestWayPointpath);
				return res;
			}
		}
	}
#endif
	ps->hasBestChoice = true;
	return res;
}

sim_mob::PathSet* sim_mob::PathSetManager::getPathSetByPersonIdAndSubTripId(std::string personId,std::string subTripId)
{
	std::map<std::string, std::map<std::string,sim_mob::PathSet*> >::iterator it=personPathSetPool.find(personId);
	if(it != personPathSetPool.end())
	{
		std::map<std::string,sim_mob::PathSet*> subTripId_pathset = (*it).second;
		std::map<std::string,sim_mob::PathSet*>::iterator itt = subTripId_pathset.find(subTripId);
		if(itt != subTripId_pathset.end())
		{
			sim_mob::PathSet* ps = (*itt).second;
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
	return NULL;
}

void sim_mob::PathSetManager::generatePaths2Node(const sim_mob::Node *toNode)
{
	// 1. from multinode to toNode
	for(int i=0;i<multiNodesPool.size();++i)
	{
		sim_mob::MultiNode* fn = multiNodesPool.at(i);
		generateSinglePathByFromToNodes(fn,toNode);
	}
	// 2. from uninode to toNode
	for(std::set<sim_mob::UniNode*>::iterator it=uniNodesPool.begin(); it!=uniNodesPool.end(); ++it)
	{
		sim_mob::UniNode* fn = (*it);
		generateSinglePathByFromToNodes(fn,toNode);
	}
}
bool sim_mob::PathSetManager::generateSinglePathByFromToNodes2(
		   const sim_mob::Node *fromNode,
		   const sim_mob::Node *toNode,
		   sim_mob::SinglePath& sp,
		   const sim_mob::RoadSegment* exclude_seg)
{
	bool res=false;
	sim_mob::SinglePath s;
	std::vector<const sim_mob::RoadSegment*> blacklist;
	if(exclude_seg)
	{
		blacklist.push_back(exclude_seg);
	}
	std::vector<WayPoint> wp = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*fromNode), stdir->DrivingVertex(*toNode),blacklist);
	if(wp.size()==0)
	{
		// no path
		if(exclude_seg)
		{
			Print()<<"gSPByFTNodes2: no path for nodes and ex seg"<<fromNode->originalDB_ID.getLogItem()<<
				toNode->originalDB_ID.getLogItem()<<
				exclude_seg->originalDB_ID.getLogItem()<<std::endl;
		}
		else
		{
			Print()<<"gSPByFTNodes2: no path for nodes"<<fromNode->originalDB_ID.getLogItem()<<
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
		s.shortestWayPointpath = convertWaypoint2Point(wp);//stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*fromNode), stdir->DrivingVertex(*toNode),blacklist);
		s.shortestSegPath = sim_mob::generateSegPathByWaypointPathP(s.shortestWayPointpath);
		sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(&s);
		s.fromNode = fromNode;
		s.toNode = toNode;
		s.pathSet = NULL;
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
		   const sim_mob::RoadSegment* exclude_seg){

	std::vector<const sim_mob::RoadSegment*> blacklist;
	if(exclude_seg)
	{
		blacklist.push_back(exclude_seg);
	}
	std::vector<WayPoint> wp = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*fromNode), stdir->DrivingVertex(*toNode),blacklist);
	if(wp.size()==0)
	{
		// no path
		Print() << "No Path" << std::endl;
	}

}


inline std::map<const RoadSegment*,WayPoint> sim_mob::generateSegPathByWaypointPath(std::vector<WayPoint>& wp)
{
	std::map<const RoadSegment*,WayPoint> res;
	for (vector<WayPoint>::iterator it = wp.begin(); it != wp.end(); it++) {
		if (it->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = it->roadSegment_;
			res.insert(std::make_pair(seg,(*it)));
		}
	}
	return res;
}

inline int sim_mob::calculateRightTurnNumberByWaypoints(std::map<const RoadSegment*,WayPoint>& segWp)
{
	if(segWp.size()<2)
		return 0;
	int res=0;
	std::map<const RoadSegment*,WayPoint>::iterator itt=segWp.begin();
	++itt;
	for(std::map<const RoadSegment*,WayPoint>::iterator it=segWp.begin();
			it!=segWp.end();
			++it)
	{
		const RoadSegment* currentSeg = (*it).first;
		const RoadSegment* targetSeg = NULL;
		if(itt!=segWp.end())
		{

			targetSeg = (*itt).first;
		}
		else // already last segment
		{
			break;
		}
		const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (currentSeg->getEnd());
		if(currEndNode) // intersection
		{
			// get lane connector
			const std::set<sim_mob::LaneConnector*>& lcs = currEndNode->getOutgoingLanes(currentSeg);
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
				id += seg->originalDB_ID.getLogItem() + "_";
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
		Print()<<"warning getSinglePath"<<std::endl;
	}
	return NULL;
}

double sim_mob::PathSetManager::getTravelCost(sim_mob::SinglePath *sp)
{
	double res=0.0;
	sim_mob::DailyTime trip_startTime = sp->pathSet->subTrip->startTime;
//	std::map<const RoadSegment*,WayPoint*> shortestSegPath = generateSegPathByWaypointPath(sp->shortestWayPointpath);
	for(std::map<const RoadSegment*,WayPoint*>::iterator it1 = sp->shortestSegPath.begin();
					it1 != sp->shortestSegPath.end(); it1++)
	{
		const sim_mob::RoadSegment* seg = (*it1).first;
		std::string seg_id = seg->originalDB_ID.getLogItem();
//		Print()<<"getTravelCost: "<<seg_id<<std::endl;
		std::map<std::string,sim_mob::ERP_Section*>::iterator it = ERP_Section_pool.find(seg_id);
		if(it!=ERP_Section_pool.end())
		{
			sim_mob::ERP_Section* erp_section = (*it).second;
			std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> >::iterator itt =
					ERP_Surcharge_pool.find(erp_section->ERP_Gantry_No_str);
			if(itt!=ERP_Surcharge_pool.end())
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

sim_mob::SinglePath::SinglePath(SinglePath *source,const sim_mob::RoadSegment* seg) :
		shortestWayPointpath(source->shortestWayPointpath),
		shortestSegPath(source->shortestSegPath),
		pathSet(source->pathSet),fromNode(source->fromNode),
		toNode(source->toNode),id(source->id),
		utility(source->utility),pathsize(source->pathsize),travel_cost(source->travel_cost),
		signal_number(source->signal_number),right_turn_number(source->right_turn_number),
		length(source->length),travle_time(source->travle_time),
		pathset_id(source->pathset_id)
{
	isNeedSave2DB=false;
	purpose = sim_mob::work;
}

sim_mob::SinglePath::SinglePath(SinglePath *source) :
		pathSet(source->pathSet),fromNode(source->fromNode),
		toNode(source->toNode),purpose(source->purpose),
		utility(source->utility),pathsize(source->pathsize),travel_cost(source->travel_cost),
		signal_number(source->signal_number),right_turn_number(source->right_turn_number),
		length(source->length),travle_time(source->travle_time),
		pathset_id(source->pathset_id),highWayDistance(source->highWayDistance),
		isMinTravelTime(source->isMinTravelTime),isMinDistance(source->isMinDistance),isMinSignal(source->isMinSignal),
		isMinRightTurn(source->isMinRightTurn),isMaxHighWayUsage(source->isMaxHighWayUsage)
{
	isNeedSave2DB=false;

	purpose = sim_mob::work;
	if(source->shortestWayPointpath.size()>0)
	{
		this->shortestWayPointpath = source->shortestWayPointpath;
		this->shortestSegPath = generateSegPathByWaypointPathP(this->shortestWayPointpath);

	}
	else
	{
		//use id to build shortestWayPointpath
		std::vector<std::string> segIds;
		boost::split(segIds,source->id,boost::is_any_of("_"));
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
					Print()<<"error: "<<str<<std::endl;
				}
				sim_mob::WayPoint *w = sim_mob::PathSetParam::getInstance()->getWayPointBySeg(seg);
				this->shortestWayPointpath.push_back(w);
				shortestSegPath.insert(std::make_pair(seg,w));
			}
		}
	}//end of else
}


sim_mob::PathSet::PathSet(PathSet *ps) :
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
