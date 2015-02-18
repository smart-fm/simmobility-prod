/*
 * PathSetManager.cpp
 *
 *  Created on: May 6, 2013
 *      Author: Max
 */

#include "PathSetManager.hpp"
#include "Path.hpp"
#include "entities/PersonLoader.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "entities/incident/IncidentManager.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "path/PathSetThreadPool.hpp"
#include "geospatial/streetdir/KShortestPathImpl.hpp"
#include "util/threadpool/Threadpool.hpp"
#include "workers/Worker.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
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
sim_mob::BasicLogger & logger = sim_mob::Logger::log("pathset.log");
}
double getPathTravelCost(sim_mob::SinglePath *sp,const std::string & travelMode, const sim_mob::DailyTime & startTime_);
sim_mob::SinglePath* findShortestPath_LinkBased(const std::set<sim_mob::SinglePath*, sim_mob::SinglePath> &pathChoices, const sim_mob::RoadSegment *rs);
sim_mob::SinglePath* findShortestPath_LinkBased(const std::set<sim_mob::SinglePath*, sim_mob::SinglePath> &pathChoices, const sim_mob::Link *ln);


std::string getFromToString(const sim_mob::Node* fromNode,const sim_mob::Node* toNode ){
	std::stringstream out("");
	out << fromNode->getID()  << "," << toNode->getID();
	return out.str();
}

PathSetManager *sim_mob::PathSetManager::instance_;
boost::mutex sim_mob::PathSetManager::instanceMutex;

std::map<boost::thread::id, boost::shared_ptr<soci::session> > sim_mob::PathSetManager::cnnRepo;
boost::shared_mutex sim_mob::PathSetManager::cnnRepoMutex;

boost::shared_ptr<sim_mob::batched::ThreadPool> sim_mob::PathSetManager::threadpool_(new sim_mob::batched::ThreadPool(10));


unsigned int sim_mob::PathSetManager::curIntervalMS = 0;
unsigned int sim_mob::PathSetManager::intervalMS = 0;

sim_mob::PathSetManager::PathSetManager():stdir(StreetDirectory::instance()),
		pathSetTableName(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().pathSetTableName),isUseCache(true),
		psRetrieval(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().psRetrieval),cacheLRU(2500),
		processTT(intervalMS, curIntervalMS),
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

#if 0
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
#endif

void sim_mob::PathSetManager::inserIncidentList(const sim_mob::RoadSegment* rs) {
	boost::unique_lock<boost::shared_mutex> lock(mutexExclusion);
	partialExclusions.insert(rs);
}

const boost::shared_ptr<soci::session> & sim_mob::PathSetManager::getSession(){
	boost::upgrade_lock< boost::shared_mutex > lock(cnnRepoMutex);
	std::map<boost::thread::id, boost::shared_ptr<soci::session> >::iterator it;
	it = cnnRepo.find(boost::this_thread::get_id());
	if(it == cnnRepo.end())
	{
		std::string dbStr(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
		{
			boost::upgrade_to_unique_lock< boost::shared_mutex > uniqueLock(lock);
			boost::shared_ptr<soci::session> t(new soci::session(soci::postgresql,dbStr));
			it = cnnRepo.insert(std::make_pair(boost::this_thread::get_id(),t)).first;
		}
	}
	return it->second;
}

void sim_mob::PathSetManager::storeRTT()
{
	processTT.storeRTT2DB();
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

bool sim_mob::PathSetManager::findCachedPathSet(std::string  key, boost::shared_ptr<sim_mob::PathSet> &value){
	return findCachedPathSet_LRU(key,value);
}

bool sim_mob::PathSetManager::findCachedPathSet_LRU(std::string  key, boost::shared_ptr<sim_mob::PathSet> &value){
	return cacheLRU.find(key,value);
}


void sim_mob::PathSetManager::setPathSetTags(boost::shared_ptr<sim_mob::PathSet>&ps)
{

	// find MIN_DISTANCE
	double minDistance = std::numeric_limits<double>::max();
	SinglePath * minSP = *(ps->pathChoices.begin()); // record which is min
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
	int minSignal = std::numeric_limits<int>::max();
	minSP = *(ps->pathChoices.begin()); // record which is min
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
	int minRightTurn = std::numeric_limits<int>::max();
	minSP = *(ps->pathChoices.begin()); // record which is min
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(sp->rightTurnNumber < minRightTurn)
		{
			minRightTurn = sp->rightTurnNumber;
			minSP = sp;
		}
	}
	minSP->isMinRightTurn = 1;

	// find MAX_HIGH_WAY_USAGE
	double maxHighWayUsage=0.0;
	minSP = *(ps->pathChoices.begin()); // record which is min
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


vector<WayPoint> sim_mob::PathSetManager::getPath(const sim_mob::SubTrip &subTrip, bool enRoute, const sim_mob::RoadSegment* approach)
{
	// get person id and current subtrip id
	std::stringstream str("");
	str << subTrip.fromLocation.node_->getID() << "," << subTrip.toLocation.node_->getID();
	std::string fromToID(str.str());
	//todo change the subtrip signature from pointer to referencer
	vector<WayPoint> res = vector<WayPoint>();
	//CBD area logic
	bool from = sim_mob::RestrictedRegion::getInstance().isInRestrictedZone(subTrip.fromLocation);
	bool to = sim_mob::RestrictedRegion::getInstance().isInRestrictedZone(subTrip.toLocation);
	str.str("");
	str << "[" << fromToID << "]";
	if (sim_mob::ConfigManager::GetInstance().FullConfig().CBD())
	{
		// case-1: Both O and D are outside CBD
		if (to == false && from == false)
		{
			subTrip.cbdTraverseType = TravelMetric::CBD_PASS;
			str << "[BLCKLST]";
			std::stringstream outDbg("");
			getBestPath(res, subTrip, true,std::set<const sim_mob::RoadSegment*>(), false, true,enRoute, approach);//use/enforce blacklist
			if (sim_mob::RestrictedRegion::getInstance().isInRestrictedSegmentZone(res))
			{
				throw std::runtime_error("\npath inside cbd ");
			}
		}
		else
		{
			// case-2:  Either O or D is outside CBD and the other one is inside CBD
			if (!(to && from))
			{
				subTrip.cbdTraverseType = from ? TravelMetric::CBD_EXIT : TravelMetric::CBD_ENTER;
				str << (from ? " [EXIT CBD]" : "[ENTER CBD]");
			}
			else
			{
				//case-3: Both are inside CBD
				str << "[BOTH INSIDE CBD]";
			}
			getBestPath(res, subTrip, true,std::set<const sim_mob::RoadSegment*>(), false, false,enRoute, approach);
		}
	}
	else
	{
		getBestPath(res, subTrip,true,std::set<const sim_mob::RoadSegment*>(), false, false,enRoute, approach);
	}

	//subscribe person
	logger << "[SELECTED PATH FOR : " << fromToID  << "]:\n";
	if(!res.empty())
	{
		str << printWPpath(res);
	}
	else{
		str << "[NO PATH]" << "\n";
	}
	logger << str.str();
	return res;
}

///	discard those entries which have segments whith their CBD flag set to true
///	return the final number of path choices
unsigned short purgeCbdPaths(sim_mob::PathSet &ps)
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
				erase = true;
				break;
			}
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

void sim_mob::PathSetManager::onPathSetRetrieval(boost::shared_ptr<PathSet> &ps, bool enRoute)
{
	//step-1 time dependent calculations
	int i = 0;
	double minTravelTime= std::numeric_limits<double>::max();
	sim_mob::SinglePath *minSP = *(ps->pathChoices.begin());
	BOOST_FOREACH(SinglePath *sp, ps->pathChoices)
	{
		sp->travleTime = getPathTravelTime(sp,ps->subTrip.mode,ps->subTrip.startTime, enRoute);
		sp->travelCost = getPathTravelCost(sp,ps->subTrip.mode,ps->subTrip.startTime );
		//MIN_TRAVEL_TIME
		if(sp->travleTime < minTravelTime)
		{
			minTravelTime = sp->travleTime;
			minSP = sp;
		}
	}
	if(!ps->pathChoices.empty() && minSP)
	{
		minSP->isMinTravelTime = 1;
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
	//store in into the database
	logger << "[STORE PATH: " << ps->id << "]\n";
	pathSetParam->storeSinglePath(*getSession(), ps->pathChoices,pathSetTableName);
}

//Operations:
//step-0: Initial preparations
//step-1: Check the cache
//step-2: If not found in cache, check DB
//Step-3: If not found in DB, generate all 4 types of path
//step-5: Choose the best path using utility function
bool sim_mob::PathSetManager::getBestPath(
		std::vector<sim_mob::WayPoint> &res,
		const sim_mob::SubTrip& st,bool useCache,
		std::set<const sim_mob::RoadSegment*> tempBlckLstSegs,
		bool usePartialExclusion,
		bool useBlackList,
		bool enRoute,const sim_mob::RoadSegment* approach)
{
	res.clear();
//	if(!(st->mode == "Car" || st->mode == "Taxi" || st->mode == "Motorcycle")) //mode filter if required
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

	const sim_mob::Node* fromNode = st.fromLocation.node_;
	const sim_mob::Node* toNode = st.toLocation.node_;
	if(!(toNode && fromNode)){
		logger << "Error, OD null\n" ;
		return false;
	}
	if(toNode->getID() == fromNode->getID()){
		logger << "Error: same OD id from different objects discarded:" << toNode->getID() << "\n" ;
		return false;
	}

	std::string fromToID = getFromToString(fromNode, toNode);
	if(tempNoPath.find(fromToID))
	{
		logger <<  fromToID   << "[PREVIOUS RECORERD OF FAILURE.BYPASSING : " << fromToID << "]\n";
		return false;
	}
	logger << "[THREAD " << boost::this_thread::get_id() << "][SEARCHING FOR : " << fromToID << "]\n" ;
	boost::shared_ptr<sim_mob::PathSet> ps_;

	//Step-1 Check Cache
	/*
	 * supply only the temporary blacklist, because with the current implementation,
	 * cache should never be filled with paths containing permanent black listed segments
	 */
	std::set<const sim_mob::RoadSegment*> emptyBlkLst = std::set<const sim_mob::RoadSegment*>();//and sometime you dont need a black list at all!
	if(useCache && findCachedPathSet(fromToID, ps_))
	{
		logger <<  fromToID  << " : Cache Hit\n";
		ps_->subTrip = st;//at least for the travel start time, subtrip is needed
		onPathSetRetrieval(ps_,enRoute);
		//no need to supply permanent blacklist
		bool r = getBestPathChoiceFromPathSet(ps_,partial,emptyBlkLst,enRoute, approach);
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

	//	before proceeding further, check if someone else has already started this path.
	//	if yes, back off and try after sometime
	if(!pathRetrievalAttempt.tryCheck(fromToID))
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));
		return getBestPath(res,st,true,tempBlckLstSegs,usePartialExclusion,useBlackList,enRoute,approach);
	}
	//step-2:check  DB
	sim_mob::HasPath hasPath = PSM_UNKNOWN;
	ps_.reset(new sim_mob::PathSet());
	ps_->subTrip = st;
	ps_->id = fromToID;
	ps_->scenario = scenarioName;
	hasPath = sim_mob::aimsun::Loader::loadSinglePathFromDB(*getSession(),fromToID,ps_->pathChoices, psRetrieval,blckLstSegs);
	logger  <<  fromToID << " : " << (hasPath == PSM_HASPATH ? "" : "Don't " ) << "have SinglePaths in DB \n" ;
	switch (hasPath) {
	case PSM_HASPATH: {
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
		onPathSetRetrieval(ps_,enRoute);
		r = getBestPathChoiceFromPathSet(ps_, partial,emptyBlkLst,enRoute, approach);
		logger << "[" << fromToID << "]" <<  " :  number of paths before blcklist: " << psCnt << " after blacklist:" << ps_->pathChoices.size() << "\n";
		if (r) {
			res = *(ps_->bestPath);
			//cache
			if(useCache)
			{
				cachePathSet(ps_);
			}
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
		ps_.reset(new PathSet());
		ps_->id = fromToID;
		ps_->scenario = scenarioName;
		ps_->subTrip = st;
		std::set<OD> recursiveOrigins;
		bool r = generateAllPathChoices(ps_, recursiveOrigins, blckLstSegs);
		if (!r)
		{
			logger << "[PATHSET GENERATION FAILURE : " << fromToID << "]\n";
			tempNoPath.insert(fromToID);
			return false;
		}
		//this hack conforms to the CBD property added to segment and node
		if(useBlackList)
		{
			if(!purgeCbdPaths(*ps_))
			{
				logger << "[ALL PATHS IN CBD" << fromToID << "]\n" ;
				tempNoPath.insert(fromToID);
				return false;
			}
		}
		logger << "[PATHSET GENERATED : " << fromToID << "]\n" ;
		onPathSetRetrieval(ps_, enRoute);
		r = getBestPathChoiceFromPathSet(ps_, partial,emptyBlkLst,enRoute,approach);
		if (r) {
			res = *(ps_->bestPath);
			//cache
			if(useCache)
			{
				cachePathSet(ps_);
			}
			logger << ps_->id	<< "WARNING not cached, apparently, already in cache. this is NOT and expected behavior!!\n";
			logger << "[RETURN PATH OF SIZE : " << res.size() << " : " << fromToID << "]\n";
			return true;
		}
		else
		{
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
namespace
{
struct TripComp
{
	bool operator()(const Trip* lhs, const Trip* rhs)
	{
		return lhs->fromLocation.node_ < rhs->fromLocation.node_;
	}
};
}
void sim_mob::PathSetManager::bulkPathSetGenerator()
{
	std::string storedProcName = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().procedureMappings["day_activity_schedule"];
	if (!storedProcName.size()) { return; }
	//Our SQL statement
	stringstream query;
	Print() << "Reading Demand...  " ;
	query << "select * from " << storedProcName << "(0,30)";
	soci::rowset<soci::row> rs = ((*getSession()).prepare << query.str());
	std::set<Trip*, TripComp> tripchains;
	int cnt = 0;
	for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{
		cnt++;
		Trip* trip = sim_mob::PeriodicPersonLoader::makeTrip(*it,0);
		if(!tripchains.insert(trip).second)//todo, sequence number is NOT needed for now. But if needed later, make proper modifications
		{
			safe_delete_item(trip);
		}
	}
	Print() << "TRIPCHAINS: " << cnt << "][DISTINICT :" <<  tripchains.size() << "]" << std::endl;

	sim_mob::Profiler t("bulk generator details", true);
	int total = 0;
	std::set<OD> recursiveOrigins;
	BOOST_FOREACH(Trip* trip, tripchains)
	{
		const std::vector<sim_mob::SubTrip>& subTrips = trip->getSubTrips();
		BOOST_FOREACH(const SubTrip& subTrip, subTrips)
		{
			if(!recursiveOrigins.insert(OD(subTrip.fromLocation.node_, subTrip.toLocation.node_)).second)
			{
				continue;
			}
			std::set<const RoadSegment*> blrs;
			boost::shared_ptr<sim_mob::PathSet> ps_(new PathSet());
			ps_->id = getFromToString(subTrip.fromLocation.node_, subTrip.toLocation.node_);
			ps_->scenario = scenarioName;
			ps_->subTrip = subTrip;
			Print() << "[" << ps_->id << "] GROUP START" << std::endl;// group: group of recursive ODs
			int r = 0;
			total += r = generateAllPathChoices(ps_, recursiveOrigins, blrs);
			Print() << "["  << ps_->id << "] GROUP COMPLETE [PATHS GENERATED: " << r << " ,  TIME :" << t.tick().second.count() << "  Microseconds]"
					<< "[TOTAL PATHS: " << total << " ,  TIME : " << t.tick().first.count() << "  Microseconds]" << std::endl;
		}
	}
}

int sim_mob::PathSetManager::genK_ShortestPath(boost::shared_ptr<sim_mob::PathSet> &ps, std::set<sim_mob::SinglePath*, sim_mob::SinglePath> &KSP_Storage)
{
	std::string fromToID(getFromToString(ps->subTrip.fromLocation.node_, ps->subTrip.toLocation.node_));
	std::vector< std::vector<sim_mob::WayPoint> > ksp;
	int kspn = sim_mob::K_ShortestPathImpl::getInstance()->getKShortestPaths(ps->subTrip.fromLocation.node_, ps->subTrip.toLocation.node_,ksp);

	logger << "[" << fromToID << "][K-SHORTEST-PATH]\n";
	for(int i=0;i<ksp.size();++i)
	{
		std::vector<sim_mob::WayPoint> &path_ = ksp[i];
		std::string id = sim_mob::makeWaypointsetString(path_);
		std::stringstream out("");
		out << ps->scenario << "KSHP-" << i;
		sim_mob::SinglePath *s = new sim_mob::SinglePath();
		// fill data
		s->isNeedSave2DB = true;
		s->id = id;
		s->pathSetId = fromToID;
		s->init(path_);
		s->scenario = ps->scenario + out.str();
		s->pathSize=0;
		KSP_Storage.insert(s);
		logger << "[KSP:" << i << "] " << s->id << "[length: " << s->length << "]\n";
	}
	return kspn;
}

int sim_mob::PathSetManager::genSDLE(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &SDLE_Storage)
{
	sim_mob::Link *curLink = NULL;
	std::set<const RoadSegment*> blackList = std::set<const RoadSegment*>();
	std::string fromToID(getFromToString(ps->subTrip.fromLocation.node_, ps->subTrip.toLocation.node_));
	logger << "[" << fromToID << "][SHORTEST DISTANCE LINK ELIMINATION]\n";
	A_StarShortestPathImpl * impl = (A_StarShortestPathImpl*)stdir.getDistanceImpl();
	StreetDirectory::VertexDesc from = impl->DrivingVertex(*ps->subTrip.fromLocation.node_);
	StreetDirectory::VertexDesc to = impl->DrivingVertex(*ps->subTrip.toLocation.node_);
	if(ps->oriPath && !ps->oriPath->path.empty())
	{
		curLink = ps->oriPath->path.begin()->roadSegment_->getLink();
	}
	int cnt = 0;
	for(std::vector<sim_mob::WayPoint>::iterator it=ps->oriPath->path.begin();	it != ps->oriPath->path.end() ;++it)
	{
		const sim_mob::RoadSegment* currSeg = it->roadSegment_;
		if(currSeg->getLink() == curLink)
		{
			blackList.insert(currSeg);
		}
		else
		{
			curLink = currSeg->getLink();
			PathSetWorkerThread * work = new PathSetWorkerThread();
			//introducing the profiling time accumulator
			//the above declared profiler will become a profiling time accumulator of ALL workers in this loop
			work->graph = &impl->drivingMap_;
			work->segmentLookup = &impl->drivingSegmentLookup_;
			work->fromVertex = from.source;
			work->toVertex = to.sink;
			work->fromNode = ps->subTrip.fromLocation.node_;
			work->toNode =  ps->subTrip.toLocation.node_;
			work->excludeSeg = blackList;
			blackList.clear();
			work->ps = ps;
			std::stringstream out("");
			out << "SDLE-" << ++cnt;
			work->dbgStr = out.str();
			threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
			SDLE_Storage.push_back(work);
		} //ROAD_SEGMENT
	}
	if(!cnt)
	{
		std::cerr  << "[" << fromToID << "]Nothing supplied to threadpool-SDLE" << std::endl;
	}
}

int sim_mob::PathSetManager::genSTTLE(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &STTLE_Storage)
{
	sim_mob::Link *curLink = NULL;
	std::set<const RoadSegment*> blackList = std::set<const RoadSegment*>();
	std::string fromToID(getFromToString(ps->subTrip.fromLocation.node_,ps->subTrip.toLocation.node_));

	logger << "[" << fromToID << "][SHORTEST TRAVEL TIME LINK ELIMINATION]\n";
	A_StarShortestTravelTimePathImpl * sttpImpl = (A_StarShortestTravelTimePathImpl*)stdir.getTravelTimeImpl();
	StreetDirectory::VertexDesc from = sttpImpl->DrivingVertexNormalTime(*ps->subTrip.fromLocation.node_);
	StreetDirectory::VertexDesc to = sttpImpl->DrivingVertexNormalTime(*ps->subTrip.toLocation.node_);
	SinglePath *pathTT = generateShortestTravelTimePath(ps->subTrip.fromLocation.node_,ps->subTrip.toLocation.node_,sim_mob::Default);
	int cnt = 0;
	if(pathTT)
	{
		if(!pathTT->path.empty())
		{
			curLink = pathTT->path.begin()->roadSegment_->getLink();
		}
		for(std::vector<sim_mob::WayPoint>::iterator it(pathTT->path.begin()); it != pathTT->path.end() ;++it)
		{
			const sim_mob::RoadSegment* currSeg = it->roadSegment_;
			if(currSeg->getLink() == curLink)
			{
				blackList.insert(currSeg);
			}
			else
			{
				curLink = currSeg->getLink();
				PathSetWorkerThread *work = new PathSetWorkerThread();
				work->graph = &sttpImpl->drivingMap_Default;
				work->segmentLookup = &sttpImpl->drivingSegmentLookup_Default_;
				work->fromVertex = from.source;
				work->toVertex = to.sink;
				work->fromNode = ps->subTrip.fromLocation.node_;
				work->toNode = ps->subTrip.toLocation.node_;
				work->excludeSeg = blackList;
				blackList.clear();
				work->ps = ps;
				std::stringstream out("");
				out << "STTLE-" << cnt++ ;
				work->dbgStr = out.str();
				threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
				STTLE_Storage.push_back(work);
			} //ROAD_SEGMENT
		}//for
	}//if sinPathTravelTimeDefault
	if(!cnt)
	{
		std::cerr  << "[" << fromToID << "]Nothing supplied to threadpool-STTLE" << std::endl;
	}

}

int sim_mob::PathSetManager::genSTTHBLE(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &STTHBLE_Storage)
{
	sim_mob::Link *curLink = NULL;
	std::set<const RoadSegment*> blackList = std::set<const RoadSegment*>();
	std::string fromToID(getFromToString(ps->subTrip.fromLocation.node_,ps->subTrip.toLocation.node_));
	logger << "[" << fromToID << "][SHORTEST TRAVEL TIME LINK ELIMINATION HIGHWAY BIAS]\n";
	SinglePath *sinPathHightwayBias = generateShortestTravelTimePath(ps->subTrip.fromLocation.node_,ps->subTrip.toLocation.node_,sim_mob::HighwayBias_Distance);
	A_StarShortestTravelTimePathImpl * sttpImpl = (A_StarShortestTravelTimePathImpl*)stdir.getTravelTimeImpl();
	StreetDirectory::VertexDesc from = sttpImpl->DrivingVertexHighwayBiasDistance(*ps->subTrip.fromLocation.node_);
	StreetDirectory::VertexDesc to = sttpImpl->DrivingVertexHighwayBiasDistance(*ps->subTrip.toLocation.node_);
	int cnt = 0;
	if(sinPathHightwayBias)
	{
		if(!sinPathHightwayBias->path.empty())
		{
			curLink = sinPathHightwayBias->path.begin()->roadSegment_->getLink();
		}
		for(std::vector<sim_mob::WayPoint>::iterator it(sinPathHightwayBias->path.begin()); it != sinPathHightwayBias->path.end() ;++it)
		{
			const sim_mob::RoadSegment* currSeg = it->roadSegment_;
			if(currSeg->getLink() == curLink)
			{
				blackList.insert(currSeg);
			}
			else
			{
				curLink = currSeg->getLink();
				PathSetWorkerThread *work = new PathSetWorkerThread();
				//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
				//introducing the profiling time accumulator
				work->graph = &sttpImpl->drivingMap_HighwayBias_Distance;
				work->segmentLookup = &sttpImpl->drivingSegmentLookup_HighwayBias_Distance_;
				work->fromVertex = from.source;
				work->toVertex = to.sink;
				work->fromNode = ps->subTrip.fromLocation.node_;
				work->toNode = ps->subTrip.toLocation.node_;
				work->excludeSeg = blackList;
				blackList.clear();
				work->ps = ps;
				std::stringstream out("");
				out << "STTH-" << cnt++;
				work->dbgStr = out.str();
				threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
				STTHBLE_Storage.push_back(work);
			} //ROAD_SEGMENT
		}//for
	}//if sinPathTravelTimeDefault
	logger  << "waiting for TRAVEL TIME HIGHWAY BIAS" << "\n";
	if(!cnt)
	{
		std::cerr  << "[" << fromToID << "]Nothing supplied to threadpool-STTH" << std::endl;
	}
}


int sim_mob::PathSetManager::genRandPert(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &RandPertStorage)
{
	std::string fromToID(getFromToString(ps->subTrip.fromLocation.node_,ps->subTrip.toLocation.node_));
	A_StarShortestTravelTimePathImpl * sttpImpl = (A_StarShortestTravelTimePathImpl*)stdir.getTravelTimeImpl();
	// generate random path
	int randCnt = sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().perturbationIteration;
	int cnt = 0;
	for(int i=0;i < randCnt ; ++i)
	{
		StreetDirectory::VertexDesc from = sttpImpl->DrivingVertexRandom(*ps->subTrip.fromLocation.node_,i);
		StreetDirectory::VertexDesc to = sttpImpl->DrivingVertexRandom(*ps->subTrip.toLocation.node_,i);
		if(!(from.valid && to.valid))
		{
			std::cout << "Invalid VertexDesc\n";
			continue;
		}
		PathSetWorkerThread *work = new PathSetWorkerThread();
		//introducing the profiling time accumulator
		//the above declared profiler will become a profiling time accumulator of ALL workeres in this loop
		work->graph = &sttpImpl->drivingMap_Random_pool[i];
		work->segmentLookup = &sttpImpl->drivingSegmentLookup_Random_pool[i];
		work->fromVertex = from.source;
		work->toVertex = to.sink;
		work->fromNode = ps->subTrip.fromLocation.node_;
		work->toNode = ps->subTrip.toLocation.node_;
		work->ps = ps;
		std::stringstream out("");
		out << "TTRP-" << cnt++;  ;
		work->dbgStr = out.str();
		logger << work->dbgStr;
		RandPertStorage.push_back(work);
		threadpool_->enqueue(boost::bind(&PathSetWorkerThread::executeThis,work));
	}
	if(!cnt)
	{
		std::cerr  << "[" << fromToID << "]Nothing supplied to threadpool-TTRP" << std::endl;
	}
}



int sim_mob::PathSetManager::generateAllPathChoices(boost::shared_ptr<sim_mob::PathSet> &ps, std::set<OD> &recursiveODs, const std::set<const sim_mob::RoadSegment*> & excludedSegs)
{
	std::string fromToID(getFromToString(ps->subTrip.fromLocation.node_,ps->subTrip.toLocation.node_));
	logger << "generateAllPathChoices" << std::endl;
	/**
	 * step-1: find the shortest path. if not found: create an entry in the "PathSet" table and return(without adding any entry into SinglePath table)
	 * step-2: K-SHORTEST PATH
	 * step-3: SHORTEST DISTANCE LINK ELIMINATION
	 * step-4: shortest travel time link elimination
	 * step-5: TRAVEL TIME HIGHWAY BIAS
	 * step-6: Random Pertubation
	 * step-7: Some caching/bookkeeping
	 * step-8: RECURSION!!!
	 */
	std::set<std::string> duplicateChecker;//for extra optimization only(creating singlepath and discarding it later can be expensive)
	sim_mob::SinglePath *s = findShortestDrivingPath(ps->subTrip.fromLocation.node_,ps->subTrip.toLocation.node_,duplicateChecker/*,excludedSegs*/);
	if(!s)
	{
		// no path
		if(tempNoPath.tryCheck(ps->id))
		{
			ps->hasPath = false;
			ps->isNeedSave2DB = true;
		}
		return false;
	}
	s->pathSetId = fromToID;
	s->scenario += "SP";
	//some additional settings
	ps->hasPath = true;
	ps->isNeedSave2DB = true;
	ps->oriPath = s;
	ps->id = fromToID;

//	//K-SHORTEST PATH
//	//Print() << "[" << fromToID << "][K-SHORTEST PATH] " << fromToID << std::endl;
//	std::set<sim_mob::SinglePath*, sim_mob::SinglePath> KSP_Storage;//main storage for k-shortest path
//	threadpool_->enqueue(boost::bind(&PathSetManager::genK_ShortestPath, this, ps, KSP_Storage));
////	genK_ShortestPath(ps, KSP_Storage);

	std::vector<std::vector<PathSetWorkerThread*> > mainStorage = std::vector<std::vector<PathSetWorkerThread*> >();
	// SHORTEST DISTANCE LINK ELIMINATION
	//Print() << "[" << fromToID << "][SHORTEST DISTANCE LINK ELIMINATION]\n";
	std::vector<PathSetWorkerThread*> SDLE_Storage;
	genSDLE(ps, SDLE_Storage);

	//step-3: SHORTEST TRAVEL TIME LINK ELIMINATION
	//Print() << "[" << fromToID << "][SHORTEST TRAVEL TIME LINK ELIMINATION]\n";
	std::vector<PathSetWorkerThread*> STTLE_Storage;
	genSTTLE(ps,STTLE_Storage);

	// TRAVEL TIME HIGHWAY BIAS
	//Print() << "[" << fromToID << "][SHORTEST TRAVEL TIME LINK ELIMINATION HIGHWAY BIAS]\n";
	std::vector<PathSetWorkerThread*> STTHBLE_Storage;
	genSTTHBLE(ps,STTHBLE_Storage);


	//Print() << "[" << fromToID << "][RANDOM]\n";
	std::vector<PathSetWorkerThread*> randPertStorage;
	genRandPert(ps,randPertStorage);

	threadpool_->wait();


	//record
	mainStorage.push_back(SDLE_Storage);
	mainStorage.push_back(STTLE_Storage);
	mainStorage.push_back(STTHBLE_Storage);
	mainStorage.push_back(randPertStorage);
	//a.record the shortest path with all segments
	if(!ps->oriPath){
		std::string str = "path set " + ps->id + " has no shortest path\n" ;
		throw std::runtime_error(str);
	}
	if(!ps->oriPath->isShortestPath){
		std::string str = "path set " + ps->id + " is supposed to be the shortest path but it is not!\n" ;
		throw std::runtime_error(str);
	}
	int total = 0;
	//Print() << "[" << fromToID << "][RECORD]\n";
	total += ps->addOrDeleteSinglePath(ps->oriPath);
//	//b. record k-shortest paths
//	BOOST_FOREACH(sim_mob::SinglePath* sp, KSP_Storage)
//	{
//		ps->addOrDeleteSinglePath(sp);
//	}

	//c. record the rest of the paths (link eliminations and random perturbation)
	BOOST_FOREACH(std::vector<PathSetWorkerThread*> &workPool, mainStorage)
	{
		BOOST_FOREACH(PathSetWorkerThread* p, workPool){
			if(p->hasPath){
				if(p->s->isShortestPath){
					std::string str = "Single path from pathset " + ps->id + " is not supposed to be marked as a shortest path but it is!\n" ;
					throw std::runtime_error(str);
				}
				total += ps->addOrDeleteSinglePath(p->s);
			}
		}
		//cleanupworkPool
		for(std::vector<PathSetWorkerThread*>::iterator wrkPoolIt=workPool.begin(); wrkPoolIt!=workPool.end(); wrkPoolIt++) {
			safe_delete_item(*wrkPoolIt);
		}
		workPool.clear();
	}
	//step-7 PROCESS
	//Print() << "[" << fromToID << "][PROCESS]\n";
	onGeneratePathSet(ps);

	if(!ConfigManager::GetInstance().FullConfig().pathSet().recPS)
	{
		//end the method here, no need to proceed to recursive pathset generation
		return total;
	}

	//step -8 : RECURSE
	boost::shared_ptr<sim_mob::PathSet> recursionPs;
	/*
	 * a) iterate through each path to first find ALL the multinodes to destination to make a set.
	 * b) then iterate through this set to choose each of them as a new Origin(Destination is same).
	 * call generateAllPathChoices on the new OD pair
	 */
	//a)
	std::set<Node*> newOrigins = std::set<Node*>();
	BOOST_FOREACH(sim_mob::SinglePath *sp, ps->pathChoices)
	{
		if(sp->path.size() <=1)
		{
			continue;
		}
		sim_mob::Node * linkEnd = nullptr;
		//skip the origin and destination node(first and last one)
		std::vector<WayPoint>::iterator it = sp->path.begin();
		it++;
		std::vector<WayPoint>::iterator itEnd = sp->path.end();
		itEnd--;
		for(; it != itEnd; it++)
		{
			//skip uninodes
			sim_mob::Node * newFrom = it->roadSegment_->getLink()->getEnd();
			// All segments of the link have the same link end node. Skip if already chosen
			if(linkEnd == newFrom)
			{
				continue;
			}
			else
			{
				linkEnd = newFrom;
			}
			//check if the new OD you want to process is not already scheduled for processing by previous iterations(todo: or even by other threads!)
			if(recursiveODs.insert(OD(newFrom,ps->subTrip.toLocation.node_)).second == false)
			{
				continue;
			}
			//Now we have a new qualified Origin. note it down for further processing
			newOrigins.insert(newFrom);
		}
	}
	//b)
	BOOST_FOREACH(sim_mob::Node *from, newOrigins)
	{
		boost::shared_ptr<sim_mob::PathSet> recursionPs(new sim_mob::PathSet());
		recursionPs->subTrip = ps->subTrip;
		recursionPs->subTrip.fromLocation.node_ = from;
		recursionPs->id = getFromToString(recursionPs->subTrip.fromLocation.node_, recursionPs->subTrip.toLocation.node_);
		recursionPs->scenario = ps->scenario;
		total += generateAllPathChoices(recursionPs,recursiveODs,excludedSegs);
	}

	return total;
}

#if 0
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
	//if(getCachedBestPath(fromToID,cachedResult))
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
						ps_->pathChoices,psRetrieval);
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
							//insertFromTo_BestPath_Pool(fromToID,res);
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
			s->travelCost = getPathTravelCost(s,ps_->subTrip.mode, ps_->subTrip.startTime);
			s->travleTime = getPathTravelTime(s,ps_->subTrip.mode,ps_->subTrip.startTime);
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
			sinPath->travelCost = getPathTravelCost(sinPath,ps_->subTrip.startTime);
			sinPath->travleTime = getPathTravelTime(sinPath,ps_->subTrip.startTime);
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
			sinPath->travelCost = getPathTravelCost(sinPath,ps_->subTrip.startTime);
			sinPath->travleTime = getPathTravelTime(sinPath,ps_->subTrip.startTime);
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
		sinPath_morningPeak->travelCost = getPathTravelCost(sinPath_morningPeak,ps_->subTrip.startTime);
		sinPath_morningPeak->travleTime = getPathTravelTime(sinPath_morningPeak,ps_->subTrip.startTime);
		sinPath_morningPeak->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath_morningPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_morningPeak->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::MorningPeak);
	}
	SinglePath *sinPath_eveningPeak = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::EveningPeak);
	if(sinPath_eveningPeak)
	{
		sinPath_eveningPeak->travelCost = getPathTravelCost(sinPath_eveningPeak,ps_->subTrip.startTime);
		sinPath_eveningPeak->travleTime = getPathTravelTime(sinPath_eveningPeak,ps_->subTrip.startTime);
		sinPath_eveningPeak->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath_eveningPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_eveningPeak->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::EveningPeak);
	}
	SinglePath *sinPath_offPeak = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::OffPeak);
	if(sinPath_offPeak)
	{
		sinPath_offPeak->travelCost = getPathTravelCost(sinPath_offPeak,ps_->subTrip.startTime);
		sinPath_offPeak->travleTime = getPathTravelTime(sinPath_offPeak,ps_->subTrip.startTime);
		sinPath_offPeak->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath_offPeak);
		generatePathesByTravelTimeLinkElimination(sinPath_offPeak->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::OffPeak);
	}
	SinglePath *sinPath_default = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::Default);
	if(sinPath_default)
	{
		sinPath_default->travelCost = getPathTravelCost(sinPath_default,ps_->subTrip.startTime);
		sinPath_default->travleTime = getPathTravelTime(sinPath_default,ps_->subTrip.startTime);
		sinPath_default->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath_default);
		generatePathesByTravelTimeLinkElimination(sinPath_default->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::Default);
	}
	// generate high way bias path
	SinglePath *sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_Distance);
	if(sinPath)
	{
		sinPath->travelCost = getPathTravelCost(sinPath,ps_->subTrip.startTime);
		sinPath->travleTime = getPathTravelTime(sinPath,ps_->subTrip.startTime);
		sinPath->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_Distance);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_MorningPeak);
	if(sinPath)
	{
		sinPath->travelCost = getPathTravelCost(sinPath,ps_->subTrip.startTime);
		sinPath->travleTime = getPathTravelTime(sinPath,ps_->subTrip.startTime);
		sinPath->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_EveningPeak);
	if(sinPath)
	{
		sinPath->travelCost = getPathTravelCost(sinPath,ps_->subTrip.startTime);
		sinPath->travleTime = getPathTravelTime(sinPath,ps_->subTrip.startTime);
		sinPath->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_OffPeak);
	if(sinPath)
	{
		sinPath->travelCost = getPathTravelCost(sinPath,ps_->subTrip.startTime);
		sinPath->travleTime = getPathTravelTime(sinPath,ps_->subTrip.startTime);
		sinPath->pathSetId = ps_->id;
		ps_->pathChoices.insert(sinPath);
		generatePathesByTravelTimeLinkElimination(sinPath->path,duplicateChecker,ps_,fromNode,toNode,sim_mob::HighwayBias_MorningPeak);
	}
	sinPath = generateShortestTravelTimePath(fromNode,toNode,duplicateChecker,sim_mob::HighwayBias_Default);
	if(sinPath)
	{
		sinPath->travelCost = getPathTravelCost(sinPath,ps_->subTrip.startTime);
		sinPath->travleTime = getPathTravelTime(sinPath,ps_->subTrip.startTime);
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
			sinPath->travelCost = getPathTravelCost(sinPath,ps_->subTrip.startTime);
			sinPath->travleTime = getPathTravelTime(sinPath,ps_->subTrip.startTime);
			sinPath->pathSetId = ps_->id;
			ps_->pathChoices.insert(sinPath);
		}
	}
}
#endif

namespace
{
	std::map<const void*,sim_mob::OneTimeFlag> utilityLogger;
}

std::string sim_mob::PathSetManager::logPartialUtility(const sim_mob::SinglePath* sp, double pUtility) const
{
	//generate log file for debugging only
	if(utilityLogger[nullptr].check())
	{
		sim_mob::Logger::log("partial_utility.txt") << "pathSetId#index#travleTime#bTTVOT#travleTime * bTTVOT#pathSize#bCommonFactor#pathSize*bCommonFactor#length#bLength#length*bLength#"
				"highWayDistance#bHighway#highWayDistance*bHighway#travelCost#bCost#travelCost*bCost#signalNumber#bSigInter#signalNumber*bSigInter#rightTurnNumber#bLeftTurns#rightTurnNumber*bLeftTurns#"
				"minTravelTimeParam#isMinTravelTime#minTravelTimeParam*isMinTravelTime#minDistanceParam#isMinDistance#minDistanceParam*isMinDistance#minSignalParam#isMinSignal#minSignalParam*isMinSignal#"
				"maxHighwayParam#isMaxHighWayUsage#maxHighwayParam*isMaxHighWayUsage#purpose#b-value#purpose*b-value#partial utility" << "\n" ;
	}

	if(utilityLogger[sp].check())
	{
		sp->partialUtilityDbg << sp->pathSetId << "#" << sp->index << "#" << sp->travleTime << "#" << pathSetParam->bTTVOT <<  "#" << sp->travleTime * pathSetParam->bTTVOT << "#"
				<< sp->pathSize << "#" << pathSetParam->bCommonFactor << "#" <<  sp->pathSize * pathSetParam->bCommonFactor << "#"
				<< sp->length << "#" << pathSetParam->bLength << "#"   <<  sp->length * pathSetParam->bLength << "#"
				<< sp->highWayDistance << "#"  <<  pathSetParam->bHighway << "#" << sp->highWayDistance * pathSetParam->bHighway << "#"
				<< sp->travelCost << "#" <<   pathSetParam->bCost << "#"  << sp->travelCost * pathSetParam->bCost << "#"
				<< sp->signalNumber << "#"  <<   pathSetParam->bSigInter << "#" << sp->signalNumber * pathSetParam->bSigInter << "#"
				<< sp->rightTurnNumber << "#" << pathSetParam->bLeftTurns << "#" << sp->rightTurnNumber * pathSetParam->bLeftTurns << "#"
				<< pathSetParam->minTravelTimeParam << "#" <<  (sp->isMinTravelTime == 1) << "#" << pathSetParam->minTravelTimeParam *  (sp->isMinTravelTime == 1 ? 1 : 0) << "#"
				<< pathSetParam->minDistanceParam << "#" << (sp->isMinDistance == 1) << "#" << pathSetParam->minDistanceParam * (sp->isMinDistance == 1 ? 1 : 0) << "#"
				<< pathSetParam->minSignalParam  << "#" << (sp->isMinSignal == 1) << "#" << pathSetParam->minSignalParam * (sp->isMinSignal == 1 ? 1 : 0) << "#"
				<< pathSetParam->maxHighwayParam << "#"  << (sp->isMaxHighWayUsage == 1) << "#"  << pathSetParam->maxHighwayParam * (sp->isMaxHighWayUsage == 1 ? 1 : 0) << "#"
				<< sp->purpose << "#"  << (sp->purpose == sim_mob::work ? pathSetParam->bWork : pathSetParam->bLeisure) << "#" << sp->purpose  * (sp->purpose == sim_mob::work ? pathSetParam->bWork : pathSetParam->bLeisure) << "#"
				<< pUtility ;
		sim_mob::Logger::log("partial_utility.txt") << sp->partialUtilityDbg.str() << "\n";
	}
	return sp->partialUtilityDbg.str();
}

double sim_mob::PathSetManager::generatePartialUtility(const sim_mob::SinglePath* sp) const
{
	double pUtility = 0;
	if(!sp)
	{
		return pUtility;
	}
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
	//for debugging purpose
	logPartialUtility(sp,pUtility);
	return pUtility;
}

double sim_mob::PathSetManager::generateUtility(const sim_mob::SinglePath* sp) const
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
	return utility;
}

bool sim_mob::PathSetManager::getBestPathChoiceFromPathSet(boost::shared_ptr<sim_mob::PathSet> &ps,
		const std::set<const sim_mob::RoadSegment *> & partialExclusion ,
		const std::set<const sim_mob::RoadSegment*> &blckLstSegs , bool enRoute,
		const sim_mob::RoadSegment* approach)
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

//		if(enRoute && approach && !sim_mob::MovementFacet::isConnectedToNextSeg(approach, sp->path.begin()->roadSegment_))
//		{
//			continue;//you can't choose this path for rerouting
//		}

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
//		sp->travleTime = getPathTravelTime(sp,ps->subTrip.startTime);
//		sp->travelCost = getPathTravelCost(sp,ps->subTrip.startTime);
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
	double random = sim_mob::genRandomFloat(0,1);
	// 2.2 For each path i in the path choice set PathSet(O, D):
	int i = -1;
	utilityDbg << "\nlogsum : " << ps->logsum << "\nX : " << random << "\n";
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
		if (random <= upperProb)
		{
			// 2.3 agent A chooses path i from the path choice set.
			ps->bestPath = &(sp->path);
			sim_mob::Logger::log("path_out") << sp->pathSetId << "#" << sp->index << "#" << sp->scenario << "#" << sp->partialUtility << "#" << sp->utility << "\n";
			logger << "[LOGIT][" << sp->pathSetId <<  "] [" << i << " out of " << ps->pathChoices.size()  << " paths chosen] [UTIL: " <<  sp->utility << "] [LOGSUM: " << ps->logsum << "][exp(sp->utility)/(ps->logsum) : " << prob << "][X:" << random << "]\n";
			utilityDbg << "upperProb reached : " << upperProb << "\n";
			utilityDbg << "***********\n";
			return true;
		}
	}
	utilityDbg << "***********\n";

	// path choice algorithm
	if(!ps->oriPath)//return upon null oriPath only if the condition is normal(excludedSegs is empty)
	{
		logger<< "NO PATH , getBestPathChoiceFromPathSet, shortest path empty" << "\n";
		return false;
	}
	//the last step resorts to selecting and returning shortest path(aka oripath).
	logger << "NO BEST PATH. select to shortest path\n" ;
	ps->bestPath = &(ps->oriPath->path);
	sim_mob::Logger::log("path_out") << ps->oriPath->pathSetId << "#" << ps->oriPath->index << "#" << ps->oriPath->scenario << "#" << ps->oriPath->partialUtility << "#" << ps->oriPath->utility << "\n";
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
	if(it == duplicatePath.end())
	{
		s = new SinglePath();
		// fill data
		s->isNeedSave2DB = true;
		s->init(wp);
		s->id = id;
		s->scenario = scenarioName;
		s->isShortestPath = true;
	}
	else{
		logger<<"gSPByFTNodes3:duplicate pathset discarded\n";
	}

	return s;
}

sim_mob::SinglePath* sim_mob::PathSetManager::generateShortestTravelTimePath(const sim_mob::Node *fromNode,
			   const sim_mob::Node *toNode,
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

		s = new SinglePath();
		// fill data
		s->isNeedSave2DB = true;
		s->init(wp);
		s->id = id;
		s->scenario = scenarioName;
		s->pathSize=0;
		return s;
}

void sim_mob::generatePathSize(boost::shared_ptr<sim_mob::PathSet>&ps)
{
	//sanity check
	if(ps->pathChoices.empty())
	{
		throw std::runtime_error("Cannot generate path size for an empty pathset");
	}
	double minL = 0;
	// Step 1: the length of each path in the path choice set

	bool uniquePath;
	//pathsize
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		uniquePath = true; //this variable checks if a path has No common segments with the rest of the pathset
		double size=0.0;

		if(sp->path.empty())
		{
			throw std::runtime_error ("unexpected empty path in singlepath object");
		}
		// For each link a in the path:
		for(std::vector<WayPoint>::iterator it1=sp->path.begin(); it1!=sp->path.end(); ++it1)
		{
			const sim_mob::RoadSegment* seg = it1->roadSegment_;
			sim_mob::SinglePath* minSp = findShortestPath_LinkBased(ps->pathChoices, seg);
			if(minSp == nullptr)
			{
				std::stringstream out("");
				out << "couldn't find a min path for segment " << seg->getId();
				throw std::runtime_error(out.str());
			}
			minL = minSp->length;
			double l = seg->getLength() / 100.0;
			double sum = 0.0;
			//For each path j in the path choice set PathSet(O, D):
			BOOST_FOREACH(sim_mob::SinglePath* spj, ps->pathChoices)
			{
				if(spj->segSet.empty())
				{
					throw std::runtime_error("segSet of singlepath object is Empty");
				}
				std::set<const sim_mob::RoadSegment*>::iterator itt2 = std::find(spj->segSet.begin(), spj->segSet.end(), seg);
				if(itt2 != spj->segSet.end())
				{
					sum += minL/(spj->length);
					if(sp->id != spj->id)
					{
						uniquePath = false;
					}
				}
			} // for j
			size += l / sp->length / sum;
		}
		//is this a unique path ?
		if(uniquePath)
		{
			sp->pathSize = 0;
		}
		else
		{
			//calculate path size
			sp->pathSize = log(size);
		}
	}// end for
}

double sim_mob::PathSetManager::getPathTravelTime(sim_mob::SinglePath *sp,const std::string & travelMode, const sim_mob::DailyTime & startTime_, bool enRoute)
{
	sim_mob::DailyTime startTime = startTime_;
	double timeSum = 0.0;
	for(int i=0;i<sp->path.size();++i)
	{
		double time = 0.0;
		const sim_mob::RoadSegment * rs = sp->path[i].roadSegment_;
		const sim_mob::IncidentManager * inc = IncidentManager::getInstance();
		if(inc->getCurrIncidents().find(rs) != inc->getCurrIncidents().end())
		{
			return std::numeric_limits<double>::max();
		}
		else
		{
			if(enRoute)
			{
				time = getInSimulationSegTT(rs,travelMode, startTime);
			}
			if(!enRoute || time == 0.0)
			{
				time = sim_mob::PathSetParam::getInstance()->getSegTT(rs,travelMode, startTime);
			}
		}
		if(time == 0.0)
		{
			Print() << "No Travel Time [iteration:" << i << "] [SEGMENT: " << rs->getId() << "] [START TIME : " << startTime.getRepr_() << "]\n";
		}
		timeSum  += time;
		startTime = startTime + sim_mob::DailyTime(time*1000);
	}
	if (timeSum  <=0.0)
	{
		std::stringstream out("");
		out << "No travel time for path " << sp->id ;
		throw std::runtime_error(out.str());
	}
	return timeSum ;
}


void sim_mob::PathSetManager::addSegTT(const Agent::RdSegTravelStat & stats) {
	processTT.addTravelTime(stats);
}

double sim_mob::PathSetManager::getInSimulationSegTT(const sim_mob::RoadSegment* rs, const std::string &travelMode, const sim_mob::DailyTime &startTime)
{
	return processTT.getInSimulationSegTT(travelMode,rs);
}

void sim_mob::PathSetManager::initTimeInterval()
{
	intervalMS = sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().interval* 1000 /*milliseconds*/;
	uint32_t startTm = ConfigManager::GetInstance().FullConfig().simStartTime().getValue();
	curIntervalMS = TravelTimeManager::getTimeInterval(startTm, intervalMS);
}

void sim_mob::PathSetManager::updateCurrTimeInterval()
{
	curIntervalMS += intervalMS;
}


/**
 * In the path set find the shortest path which includes the given segment.
 * The method uses linkPath container which covers all the links that the
 * path visits.
 * Note: A path visiting a link doesnt mean the enire link is within the path.
 * fist and last link might have segments that are not in the path.
 * @param pathChoices given path set
 * @param rs consider only the paths having the given road segment
 * @return the singlepath object containing the shortest path
 */
sim_mob::SinglePath* findShortestPath_LinkBased(const std::set<sim_mob::SinglePath*, sim_mob::SinglePath> &pathChoices, const sim_mob::Link *ln)
{
	if(pathChoices.begin() == pathChoices.end())
	{
		return nullptr;
	}
	sim_mob::SinglePath* res = nullptr;
	double min = std::numeric_limits<double>::max();
	double tmp = 0.0;
	BOOST_FOREACH(sim_mob::SinglePath*sp, pathChoices)
	{

		if(sp->linkPath.empty())
		{
			throw std::runtime_error("linkPath of singlepath object is Empty");
		}
		//filter paths not including the target link
		if(std::find(sp->linkPath.begin(),sp->linkPath.end(),ln) == sp->linkPath.end())
		{
			continue;
		}
		if(sp->length <= 0.0)
		{
			throw std::runtime_error("Invalid path length");//todo remove this after enough testing
		}
		//double tmp = generateSinglePathLength(sp->path);
		if ((sp->length*1000000 - min*1000000  ) < 0.0) //easy way to check doubles
		{
			//min = tmp;
			min = sp->length;
			res = sp;
		}
	}
	return res;
}

/**
 * Overload
 */
sim_mob::SinglePath* findShortestPath_LinkBased(const std::set<sim_mob::SinglePath*, sim_mob::SinglePath> &pathChoices, const sim_mob::RoadSegment *rs)
{
	const sim_mob::Link *ln = rs->getLink();
	return findShortestPath_LinkBased(pathChoices,ln);
}

double getPathTravelCost(sim_mob::SinglePath *sp,const std::string & travelMode, const sim_mob::DailyTime & startTime_)
{
	sim_mob::DailyTime tripStartTime(startTime_);
	double res=0.0;
	double ts=0.0;
	if(!sp || !sp->path.empty()) {
		sim_mob::Logger::log("pathset.log") << "gTC: sp is empty" << std::endl;
	}
	for(std::vector<WayPoint>::iterator it1 = sp->path.begin(); it1 != sp->path.end(); it1++)
	{
		unsigned long segId = (it1)->roadSegment_->getId();
		std::map<int,sim_mob::ERP_Section*>::iterator it = sim_mob::PathSetParam::getInstance()->ERP_SectionPool.find(segId);//todo type mismatch
		//get travel time to this segment
		double t = sim_mob::PathSetParam::getInstance()->getSegTT((it1)->roadSegment_,travelMode, tripStartTime);
		ts += t;
		tripStartTime = tripStartTime + sim_mob::DailyTime(t*1000);
		if(it!=sim_mob::PathSetParam::getInstance()->ERP_SectionPool.end())
		{
			sim_mob::ERP_Section* erp_section = (*it).second;
			std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> >::iterator itt =
					sim_mob::PathSetParam::getInstance()->ERP_SurchargePool.find(erp_section->ERP_Gantry_No_str);
			if(itt!=sim_mob::PathSetParam::getInstance()->ERP_SurchargePool.end())
			{
				std::vector<sim_mob::ERP_Surcharge*> erp_surcharges = (*itt).second;
				for(int i=0;i<erp_surcharges.size();++i)
				{
					sim_mob::ERP_Surcharge* s = erp_surcharges[i];
					if( s->startTime_DT.isBeforeEqual(tripStartTime) && s->endTime_DT.isAfter(tripStartTime) &&
							s->vehicleTypeId == 1 && s->day == "Weekdays")
					{
						res += s->rate;
					}
				}
			}
		}
	}
	return res;
}

