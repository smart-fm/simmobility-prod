/*
 * PathSetManager.cpp
 *
 *  Created on: May 6, 2013
 *      Author: Max
 */

#include "PathSetManager.hpp"

#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <sstream>
#include "entities/incident/IncidentManager.hpp"
#include "entities/PersonLoader.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/streetdir/KShortestPathImpl.hpp"
#include "message/MessageBus.hpp"
#include "Path.hpp"
#include "path/PathSetThreadPool.hpp"
#include "soci/postgresql/soci-postgresql.h"
#include "util/threadpool/Threadpool.hpp"
#include "util/Utils.hpp"
#include "workers/Worker.hpp"



using std::vector;
using std::string;

using namespace sim_mob;

namespace
{

	struct ModelContext
	{
		ModelContext() { pvtRouteChoiceModel = new PrivateTrafficRouteChoice(); }
		~ModelContext() { delete pvtRouteChoiceModel; }

		PrivateTrafficRouteChoice* pvtRouteChoiceModel;
	};

	boost::thread_specific_ptr<ModelContext> threadContext;

	void ensureContext()
	{
		if (!threadContext.get())
		{
			ModelContext* modelCtx = new ModelContext();
			threadContext.reset(modelCtx);
		}
	}

	class PrivateRouteChoiceProvider
	{
	public:
		/**
		 * Gets the private traffic route choice model.
		 *
		 * Attention: you should not hold this instance.
		 * This provider will give you an instance based on
		 *  current thread context.
		 *
		 * @return Lua preday model reference.
		 */
		static PrivateTrafficRouteChoice* getPvtRouteChoiceModel()
		{
		    ensureContext();
		    return threadContext.get()->pvtRouteChoiceModel;
		}
	};

	//sim_mob::BasicLogger & logger = sim_mob::Logger::log("pathset.log");

	unsigned int seed = 0;
	inline double genRandomDouble(double min, double max)
	{
		boost::mt19937 rng;
		rng.seed(static_cast<unsigned int>(std::time(0) + (++seed)));
		boost::uniform_real<double> u(min, max);
		boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > gen(rng, u);
		return gen();
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

	// Overload
	sim_mob::SinglePath* findShortestPath_LinkBased(const std::set<sim_mob::SinglePath*, sim_mob::SinglePath> &pathChoices, const sim_mob::RoadSegment *rs)
	{
		const sim_mob::Link *ln = rs->getLink();
		return findShortestPath_LinkBased(pathChoices,ln);
	}

	/**
	 * Generate pathsize of paths. PathSize values are stored in the corresponding SinglePath object
	 * @param ps the given pathset
	 */
	void generatePathSize(boost::shared_ptr<sim_mob::PathSet>&ps)
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

	/**
	 * returns string representation of WayPoint based paths
	 * @param wps a WayPoint based path
	 * @param startingNode starting node
	 * @returns string of comma separated node/segment ids contained in the path
	 */
	std::string printWPpath(const std::vector<WayPoint> &wps , const sim_mob::Node* startingNode ){
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

		return out.str();
	}

	/**
	 * constructs a template for subtrip to use for bulk generation of pathsets
	 */
	sim_mob::SubTrip makeTemplateSubTrip()
	{
		sim_mob::SubTrip subTrip;
		subTrip.setPersonID(std::string());
		subTrip.itemType = sim_mob::TripChainItem::IT_TRIP;
		subTrip.tripID = "1";
		subTrip.fromLocationType = sim_mob::TripChainItem::LT_NODE;
		subTrip.toLocationType = sim_mob::TripChainItem::LT_NODE;
		subTrip.mode = "Car";
		subTrip.isPrimaryMode = true;
		subTrip.startTime = sim_mob::DailyTime("00:00:00");
		return subTrip;
	}

	/**
	 * structure to help avoiding simultaneous pathset generation by multiple threads for identical OD
	 */
	struct SimpleCollector
	{
	private:
		boost::mutex mutex_;
		std::set<std::string> collection;
	public:
		bool tryCheck(const std::string &od)
		{
			boost::unique_lock<boost::mutex> lock(mutex_);
			if(collection.find(od) != collection.end())
			{
				return false;
			}
			collection.insert(od);
			return true;
		}

		bool insert(const std::string &od)
		{
			boost::unique_lock<boost::mutex> lock(mutex_);
			return collection.insert(od).second;
		}

		void erase(const std::string &od)
		{
			boost::unique_lock<boost::mutex> lock(mutex_);
			collection.erase(od);
		}

		bool find(const std::string &od)
		{
			boost::unique_lock<boost::mutex> lock(mutex_);
			return collection.find(od) != collection.end();
		}
	};

	double getPathTravelCost(sim_mob::SinglePath *sp,const std::string & travelMode, const sim_mob::DailyTime & startTime_)
	{
		sim_mob::DailyTime tripStartTime(startTime_);
		double res=0.0;
		double ts=0.0;
		for(std::vector<WayPoint>::iterator it1 = sp->path.begin(); it1 != sp->path.end(); it1++)
		{
			unsigned long segId = (it1)->roadSegment_->getId();
			std::map<int,sim_mob::ERP_Section*>::iterator it = sim_mob::PathSetParam::getInstance()->ERP_SectionPool.find(segId);//todo type mismatch
			//get travel time to this segment
			double t = sim_mob::PathSetParam::getInstance()->getSegTT((it1)->roadSegment_,travelMode, tripStartTime);
			ts += t;
			tripStartTime = (tripStartTime + sim_mob::DailyTime(t*1000)).getTimeFromMidNight();
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
						if( s->startTime_DT.isBeforeEqual(tripStartTime) && s->endTime_DT.isAfterEqual(tripStartTime) &&
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

	/**
	 * used to avoid entering duplicate "HAS_PATH=-1" pathset entries into PathSet.
	 * It will be removed once the cache and/or proper DB functions are in place
	 */
	SimpleCollector tempNoPath;
}

PrivatePathsetGenerator* sim_mob::PrivatePathsetGenerator::instance_ = nullptr;

std::string getFromToString(const sim_mob::Node* fromNode,const sim_mob::Node* toNode ){
	std::stringstream out("");
	out << fromNode->getID()  << "," << toNode->getID();
	return out.str();
}

boost::shared_ptr<sim_mob::batched::ThreadPool> sim_mob::PrivatePathsetGenerator::threadpool_;


unsigned int sim_mob::PathSetManager::curIntervalMS = 0;
unsigned int sim_mob::PathSetManager::intervalMS = 0;

sim_mob::PathSetManager::PathSetManager(): pathSetTableName(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().pathSetTableName)
{
	pathSetParam = PathSetParam::getInstance();
	std::string dbStr(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
	dbSession = boost::shared_ptr<soci::session>(new soci::session(soci::postgresql,dbStr));
}

sim_mob::PathSetManager::~PathSetManager()
{}

bool sim_mob::PathSetManager::pathInBlackList(const std::vector<WayPoint> path, const std::set<const sim_mob::RoadSegment*> & blkLst) const
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

void sim_mob::PrivateTrafficRouteChoice::insertIncidentList(const sim_mob::RoadSegment* rs) {
	boost::unique_lock<boost::shared_mutex> lock(mutexExclusion);
	partialExclusions.insert(rs);
}

const boost::shared_ptr<soci::session>& sim_mob::PathSetManager::getSession()
{
	return dbSession;
}

void sim_mob::PathSetManager::clearSinglePaths(boost::shared_ptr<sim_mob::PathSet>& ps){
	BOOST_FOREACH(sim_mob::SinglePath* sp_, ps->pathChoices)
	{
		if(sp_)
		{
			safe_delete_item(sp_);
		}
	}
	ps->pathChoices.clear();
}

void sim_mob::PrivateTrafficRouteChoice::cachePathSet(boost::shared_ptr<sim_mob::PathSet>& ps){
	cacheLRU.insert(ps->id, ps);
}

bool sim_mob::PrivateTrafficRouteChoice::findCachedPathSet(std::string key, boost::shared_ptr<sim_mob::PathSet> &value){
	return cacheLRU.find(key,value);
}

void sim_mob::PrivatePathsetGenerator::setPathSetTags(boost::shared_ptr<sim_mob::PathSet>&ps)
{
	double minDistance = std::numeric_limits<double>::max();
	double maxHighWayUsage = std::numeric_limits<double>::min();
	int minSignal = std::numeric_limits<int>::max();
	int minRightTurn = std::numeric_limits<int>::max();
	double minTravelTime = std::numeric_limits<int>::max();

	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		// find MIN_DISTANCE
		if(sp->length < minDistance) { minDistance = sp->length; }

		// find MIN_SIGNAL
		if(sp->signalNumber < minSignal) { minSignal = sp->signalNumber; }

		// find MIN_RIGHT_TURN
		if(sp->rightTurnNumber < minRightTurn) { minRightTurn = sp->rightTurnNumber; }

		// find MAX_HIGH_WAY_USAGE
		if(maxHighWayUsage < sp->highWayDistance / sp->length) { maxHighWayUsage = sp->highWayDistance / sp->length; }

		//find MIN_TRAVEL_TIME
		if(sp->travelTime < minTravelTime) { minTravelTime = sp->travelTime; }
	}

	//set all minima maximas to true (more than one path may have same minima/maxima)
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		sp->isMinDistance = (sp->length == minDistance);
		sp->isMinSignal = (minSignal == sp->signalNumber);
		sp->isMinRightTurn = (sp->rightTurnNumber == minRightTurn);
		sp->isMaxHighWayUsage = (maxHighWayUsage == sp->highWayDistance / sp->length);
		sp->isMinTravelTime = (minTravelTime == sp->travelTime);
	}
}

vector<WayPoint> sim_mob::PrivateTrafficRouteChoice::getPath(const sim_mob::SubTrip &subTrip, bool enRoute, const sim_mob::RoadSegment* approach)
{
	std::stringstream str("");
	str << subTrip.fromLocation.node_->getID() << "," << subTrip.toLocation.node_->getID();
	std::string fromToID(str.str());

	vector<WayPoint> res = vector<WayPoint>();

	//Restricted area logic
	bool fromLocationInRestrictedRegion = sim_mob::RestrictedRegion::getInstance().isInRestrictedZone(subTrip.fromLocation);
	bool toLocationInRestrictedRegion = sim_mob::RestrictedRegion::getInstance().isInRestrictedZone(subTrip.toLocation);
	str.str("");
	str << "[" << fromToID << "]";
	if (sim_mob::ConfigManager::GetInstance().FullConfig().CBD())
	{
		// case-1: Both O and D are outside CBD
		if (!toLocationInRestrictedRegion && !fromLocationInRestrictedRegion)
		{
			str << "[BLCKLST]";
			std::stringstream outDbg("");
			getBestPath(res, subTrip, true, std::set<const sim_mob::RoadSegment*>(), false, true, enRoute, approach);
		}
		else
		{
			// case-2:  Either O or D is inside CBD
			if (!(toLocationInRestrictedRegion && fromLocationInRestrictedRegion))
			{
				str << (fromLocationInRestrictedRegion ? " [EXIT CBD]" : "[ENTER CBD]");
			}
			else //case-3: Both O & D are inside CBD
			{
				str << "[BOTH INSIDE CBD]";
			}
			getBestPath(res, subTrip, true,std::set<const sim_mob::RoadSegment*>(), false, false,enRoute, approach);
		}
	}
	else
	{
		getBestPath(res, subTrip, true, std::set<const sim_mob::RoadSegment*>(), false, false, enRoute, approach);
	}

//	logger << "[SELECTED PATH FOR : " << fromToID  << "]:\n";
//	if(!res.empty())
//	{
//		str << printWPpath(res);
//	}
//	else
//	{
//		str << "[NO PATH]" << "\n";
//	}
//	logger << str.str();
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

void sim_mob::PrivateTrafficRouteChoice::onPathSetRetrieval(boost::shared_ptr<PathSet> &ps, bool enRoute)
{
	//step-1 time dependent calculations
	double minTravelTime= std::numeric_limits<double>::max();
	sim_mob::SinglePath *minSP = *(ps->pathChoices.begin());
	BOOST_FOREACH(SinglePath *sp, ps->pathChoices)
	{
		sp->travelTime = getPathTravelTime(sp,ps->subTrip.mode,ps->subTrip.startTime, enRoute);
		sp->travelCost = getPathTravelCost(sp,ps->subTrip.mode,ps->subTrip.startTime );
		//MIN_TRAVEL_TIME
		if(sp->travelTime < minTravelTime)
		{
			minTravelTime = sp->travelTime;
			minSP = sp;
		}
	}

	BOOST_FOREACH(SinglePath *sp, ps->pathChoices)
	{
		if(minTravelTime == sp->travelTime)
		{
			sp->isMinTravelTime = 1;
		}
	}

	//step-2 utility calculation
	BOOST_FOREACH(SinglePath *sp, ps->pathChoices)
	{
		sp->utility = generateUtility(sp);
	}
}


void sim_mob::PrivatePathsetGenerator::onGeneratePathSet(boost::shared_ptr<PathSet> &ps)
{
	setPathSetTags(ps);
	generatePathSize(ps);
	//partial utility calculation to save some time
	BOOST_FOREACH(SinglePath *sp, ps->pathChoices)
	{
		sp->partialUtility = generatePartialUtility(sp);
	}
	//store in into the database
	//logger << "[STORE PATH: " << ps->id << "]\n";
	if(!ps->nonCDB_OD)
	{
		pathSetParam->storeSinglePath(*getSession(), ps->pathChoices, pathSetTableName);
	}
}

//Operations:
//step-0: Initial preparations
//step-1: Check the cache
//step-2: If not found in cache, check DB
//Step-3: If not found in DB, generate all 4 types of path
//step-5: Choose the best path using utility function
bool sim_mob::PrivateTrafficRouteChoice::getBestPath(
		std::vector<sim_mob::WayPoint>& res,
		const sim_mob::SubTrip& st,
		bool useCache,
		std::set<const sim_mob::RoadSegment*> tempBlckLstSegs,
		bool usePartialExclusion,
		bool nonCBD_OD,
		bool enRoute,
		const sim_mob::RoadSegment* approach)
{
	res.clear();

	//take care of partially excluded and blacklisted segments here
	std::set<const sim_mob::RoadSegment*> blckLstSegs(tempBlckLstSegs);
	const std::set<const sim_mob::RoadSegment*>& partial = (usePartialExclusion ? this->partialExclusions : std::set<const sim_mob::RoadSegment*>());

	const sim_mob::Node* fromNode = st.fromLocation.node_;
	const sim_mob::Node* toNode = st.toLocation.node_;

	if(!toNode || !fromNode)
	{
		//logger << "Error, OD null\n" ;
		return false;
	}

	if(toNode->getID() == fromNode->getID())
	{
		//logger << "Error: same O and D:" << toNode->getID() << "\n" ;
		return false;
	}

	std::string fromToID = getFromToString(fromNode, toNode);
	if(tempNoPath.find(fromToID))
	{
		//logger <<  fromToID   << "[PREVIOUS RECORD OF FAILURE. EARLY EXIT : " << fromToID << "]\n";
		return false;
	}

	//logger << "[THREAD " << boost::this_thread::get_id() << "][SEARCHING FOR : " << fromToID << "]\n" ;
	boost::shared_ptr<sim_mob::PathSet> pathset;

	//Step-1 Check Cache
	/*
	 * supply only the temporary blacklist, because with the current implementation,
	 * cache should never be filled with paths containing permanent black listed segments
	 */
	std::set<const sim_mob::RoadSegment*> emptyBlkLst = std::set<const sim_mob::RoadSegment*>(); //sometimes you don't need a black list at all!
	if(useCache && findCachedPathSet(fromToID, pathset))
	{
		//logger <<  fromToID  << " : Cache Hit\n";
		pathset->subTrip = st;//at least for the travel start time, subtrip is needed
		onPathSetRetrieval(pathset,enRoute);
		//no need to supply permanent blacklist
		bool pathChosen = getBestPathChoiceFromPathSet(pathset, partial, emptyBlkLst, enRoute);
		//logger <<  fromToID << " : getBestPathChoiceFromPathSet returned best path of size : " << pathset->bestPath->size() << "\n";
		if(pathChosen)
		{
			res = *(pathset->bestPath);
			//logger <<  fromToID << " : returning a path " << res.size() << "\n";
			return true;
		}
		//else { logger <<  fromToID  << "UNUSED Cache Hit" <<  "\n"; }
	}
	//else { logger <<  fromToID << " : Cache Miss " << "\n"; }

	//step-2:check  DB
	sim_mob::HasPath hasPath = PSM_UNKNOWN;
	pathset.reset(new sim_mob::PathSet());
	pathset->subTrip = st;
	pathset->id = fromToID;
	pathset->scenario = scenarioName;
	pathset->nonCDB_OD = nonCBD_OD;
	if(nonCBD_OD)
	{
		hasPath = sim_mob::aimsun::Loader::loadSinglePathFromDB(*getSession(), fromToID, pathset->pathChoices, psRetrievalWithoutRestrictedRegion, blckLstSegs);
	}
	else
	{
		hasPath = sim_mob::aimsun::Loader::loadSinglePathFromDB(*getSession(), fromToID, pathset->pathChoices, psRetrieval, blckLstSegs);
	}
	//logger  <<  fromToID << " : " << (hasPath == PSM_HASPATH ? "" : "Don't " ) << "have SinglePaths in DB \n" ;
	switch (hasPath)
	{
	case PSM_HASPATH:
	{
		//logger << "[" << fromToID << "]" <<  " : DB Hit\n";
		pathset->oriPath = nullptr;
		BOOST_FOREACH(sim_mob::SinglePath* sp, pathset->pathChoices)
		{
			if (sp->isShortestPath)
			{
				pathset->oriPath = sp;
				break;
			}
		}
//		if (!pathset->oriPath)
//		{
//			std::string str = "Warning => SP: oriPath(shortest path) for " + pathset->id + " not valid anymore\n";
//			logger << str;
//		}
		//	no need of processing and storing blacklisted paths
		short psCnt = pathset->pathChoices.size();
		onPathSetRetrieval(pathset,enRoute);
		bool pathChosen = getBestPathChoiceFromPathSet(pathset, partial, emptyBlkLst, enRoute);
		//logger << "[" << fromToID << "]" <<  " :  number of paths before blcklist: " << psCnt << " after blacklist:" << pathset->pathChoices.size() << "\n";
		if(pathChosen)
		{
			res = *(pathset->bestPath);
			//cache
			if(useCache) { cachePathSet(pathset); }
			//logger << "returning a path " << res.size() << "\n";
			return true;
		}
		break;
	}
	case PSM_NOTFOUND: //if not found,
	case PSM_NOGOODPATH: // or if no good path available
	default: // or if anything else
	{
		tempNoPath.insert(fromToID); //note pathset unavailability
		break;
	}
	};

	//logger << "[FINALLY NO RESULT :  " << fromToID << "]\n";
	return false;
}

namespace
{
	struct TripComp
	{
		bool operator()(const Trip* lhs, const Trip* rhs)
		{
			return getFromToString(lhs->fromLocation.node_ , lhs->toLocation.node_) <
					getFromToString(rhs->fromLocation.node_ , rhs->toLocation.node_);
		}
	};
}

void sim_mob::PrivatePathsetGenerator::bulkPathSetGenerator()
{
	const std::string odSourceTableName = sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().odSourceTableName;
	sim_mob::RoadNetwork& rn = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW();
	if (odSourceTableName.empty()) { return; }
	//Our SQL statement
	stringstream query;
	Print() << "Reading Demand...  " ;
	query << "select * from " << odSourceTableName;
	soci::rowset<soci::row> rs = ((*getSession()).prepare << query.str());
	std::set<OD> odPairs;
	int cnt = 0;
	sim_mob::Node* originNode = nullptr;
	sim_mob::Node* destinationNode = nullptr;
	for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{
		const soci::row& r = (*it);
		cnt++;
		originNode = rn.getNodeById(r.get<int>(0));
		destinationNode = rn.getNodeById(r.get<int>(1));
		odPairs.insert(OD(originNode, destinationNode));
	}
	Print() << "[DISTINICT ODs:" <<  odPairs.size() << "]" << std::endl;

	sim_mob::Profiler t("bulk generator details", true);
	int total = 0, iterCnt1 = 0, iterCnt2 = 0;
	std::set<OD> recursiveOrigins;
	std::set<const RoadSegment*> tempBlackList;
	sim_mob::SubTrip templateSubTrip = makeTemplateSubTrip();
	BOOST_FOREACH(const OD& od, odPairs)
	{
		templateSubTrip.fromLocation = od.origin;
		templateSubTrip.toLocation = od.destination;
		if(!recursiveOrigins.insert(od).second) { continue; }

		boost::shared_ptr<sim_mob::PathSet> ps_(new PathSet());
		ps_->id = od.getOD_Str();
		ps_->scenario = scenarioName;
		ps_->subTrip = templateSubTrip;
		threadpool_->enqueue(boost::bind(&sim_mob::PrivatePathsetGenerator::generateAllPathChoices, this, ps_, boost::ref(recursiveOrigins), boost::ref(tempBlackList)));
	}
	threadpool_->wait();
}

int sim_mob::PrivatePathsetGenerator::genK_ShortestPath(boost::shared_ptr<sim_mob::PathSet> &ps, std::set<sim_mob::SinglePath*, sim_mob::SinglePath> &KSP_Storage)
{
	std::string fromToID(getFromToString(ps->subTrip.fromLocation.node_, ps->subTrip.toLocation.node_));
	std::vector< std::vector<sim_mob::WayPoint> > ksp;
	int kspn = sim_mob::K_ShortestPathImpl::getInstance()->getKShortestPaths(ps->subTrip.fromLocation.node_, ps->subTrip.toLocation.node_,ksp);

	//logger << "[" << fromToID << "][K-SHORTEST-PATH]\n";
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
		//logger << "[KSP:" << i << "] " << s->id << "[length: " << s->length << "]\n";
	}
	return kspn;
}

int sim_mob::PrivatePathsetGenerator::genSDLE(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &SDLE_Storage)
{
	sim_mob::Link *curLink = nullptr;
	std::set<const RoadSegment*> blackList = std::set<const RoadSegment*>();
	std::string fromToID(getFromToString(ps->subTrip.fromLocation.node_, ps->subTrip.toLocation.node_));
	//logger << "[" << fromToID << "][SHORTEST DISTANCE LINK ELIMINATION]\n";
	A_StarShortestPathImpl * impl = (A_StarShortestPathImpl*)stdir.getDistanceImpl();
	StreetDirectory::VertexDesc from = impl->DrivingVertex(*ps->subTrip.fromLocation.node_);
	StreetDirectory::VertexDesc to = impl->DrivingVertex(*ps->subTrip.toLocation.node_);
	int cnt = 0;
	if(ps->oriPath && !ps->oriPath->path.empty())
	{
		for(std::vector<sim_mob::WayPoint>::iterator it=ps->oriPath->path.begin();	it != ps->oriPath->path.end() ;++it)
		{
			const sim_mob::RoadSegment* currSeg = it->roadSegment_;
			if(currSeg->getLink() != curLink)
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
				blackList.clear();
				blackList.insert(currSeg);
				work->excludeSeg = blackList;
				work->ps = ps;
				std::stringstream out("");
				out << "SDLE-" << ++cnt;
				work->dbgStr = out.str();
				work->timeBased = false;

				if(ConfigManager::GetInstance().PathSetConfig().privatePathSetMode == "generation")
				{
					/*
					 * NOTE:
					 * when pathset runs in "normal" mode, during pathset generation for requested ODs, each method
					 * of pathset generation(link elimination, random perturbation, etc)  will use threadpool for its operation.
					 * Whereas in "generation" mode,  each pathset generation task(as a whole) is assigned to a dedicated thread in threadpool.
					 */
					work->run();
				}
				else
				{
					threadpool_->enqueue(boost::bind(&PathSetWorkerThread::run,work));
				}

				SDLE_Storage.push_back(work);
			} //ROAD_SEGMENT
		} //for
	}
//	if(!cnt)
//	{
//		logger  << "[" << fromToID << "]Nothing supplied to threadpool-SDLE" << std::endl;
//	}
}

int sim_mob::PrivatePathsetGenerator::genSTTLE(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &STTLE_Storage)
{
	sim_mob::Link *curLink = nullptr;
	std::set<const RoadSegment*> blackList = std::set<const RoadSegment*>();
	std::string fromToID(getFromToString(ps->subTrip.fromLocation.node_,ps->subTrip.toLocation.node_));

	//logger << "[" << fromToID << "][SHORTEST TRAVEL TIME LINK ELIMINATION]\n";
	A_StarShortestTravelTimePathImpl * sttpImpl = (A_StarShortestTravelTimePathImpl*)stdir.getTravelTimeImpl();
	StreetDirectory::VertexDesc from = sttpImpl->DrivingVertexDefault(*ps->subTrip.fromLocation.node_);
	StreetDirectory::VertexDesc to = sttpImpl->DrivingVertexDefault(*ps->subTrip.toLocation.node_);
	SinglePath* pathTT = generateShortestTravelTimePath(ps->subTrip.fromLocation.node_,ps->subTrip.toLocation.node_,sim_mob::Default);

	int cnt = 0;
	if(pathTT && !pathTT->path.empty())
	{
		pathTT->scenario = "STTLE-SP";
		pathTT->pathSetId = ps->id;
		PathSetWorkerThread* work = new PathSetWorkerThread();
		work->s = pathTT;
		work->hasPath = true;
		work->ps = ps;
		STTLE_Storage.push_back(work); //store STT path as well

		for(std::vector<sim_mob::WayPoint>::iterator it(pathTT->path.begin()); it != pathTT->path.end() ;++it)
		{
			const sim_mob::RoadSegment* currSeg = it->roadSegment_;
			if(currSeg->getLink() != curLink)
			{
				curLink = currSeg->getLink();
				PathSetWorkerThread *work = new PathSetWorkerThread();
				work->graph = &sttpImpl->drivingMap_Default;
				work->segmentLookup = &sttpImpl->drivingSegmentLookup_Default_;
				work->fromVertex = from.source;
				work->toVertex = to.sink;
				work->fromNode = ps->subTrip.fromLocation.node_;
				work->toNode = ps->subTrip.toLocation.node_;
				blackList.clear();
				blackList.insert(currSeg);
				work->excludeSeg = blackList;
				work->ps = ps;
				std::stringstream out("");
				out << "STTLE-" << ++cnt ;
				work->dbgStr = out.str();
				work->timeBased = true;

				if(ConfigManager::GetInstance().PathSetConfig().privatePathSetMode == "generation")
				{
					/*
					 * NOTE:
					 * when pathset runs in "normal" mode, during pathset generation for requested ODs, each method
					 * of pathset generation(link elimination, random perturbation, etc)  will use threadpool for its operation.
					 * Whereas in "generation" mode,  each pathset generation task(as a whole) is assigned to a dedicated thread in threadpool.
					 */
					work->run();
				}
				else
				{
					threadpool_->enqueue(boost::bind(&PathSetWorkerThread::run,work));
				}
				STTLE_Storage.push_back(work);
			} //ROAD_SEGMENT
		}//for
	}//if sinPathTravelTimeDefault
//	if(!cnt)
//	{
//		logger  << "[" << fromToID << "]Nothing supplied to threadpool-STTLE" << std::endl;
//	}
}

int sim_mob::PrivatePathsetGenerator::genSTTHBLE(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &STTHBLE_Storage)
{
	sim_mob::Link *curLink = nullptr;
	std::set<const RoadSegment*> blackList = std::set<const RoadSegment*>();
	std::string fromToID(getFromToString(ps->subTrip.fromLocation.node_,ps->subTrip.toLocation.node_));
	//logger << "[" << fromToID << "][SHORTEST TRAVEL TIME LINK ELIMINATION HIGHWAY BIAS]\n";
	SinglePath *sinPathHighwayBias = generateShortestTravelTimePath(ps->subTrip.fromLocation.node_,ps->subTrip.toLocation.node_,sim_mob::HighwayBias_Default);
	A_StarShortestTravelTimePathImpl * sttpImpl = (A_StarShortestTravelTimePathImpl*)stdir.getTravelTimeImpl();
	StreetDirectory::VertexDesc from = sttpImpl->DrivingVertexHighwayBiasDefault(*ps->subTrip.fromLocation.node_);
	StreetDirectory::VertexDesc to = sttpImpl->DrivingVertexHighwayBiasDefault(*ps->subTrip.toLocation.node_);
	int cnt = 0;
	if(sinPathHighwayBias && !sinPathHighwayBias->path.empty())
	{
		sinPathHighwayBias->scenario = "STTHLE-SP";
		sinPathHighwayBias->pathSetId = ps->id;
		PathSetWorkerThread* work = new PathSetWorkerThread();
		work->s = sinPathHighwayBias;
		work->hasPath = true;
		work->ps = ps;
		STTHBLE_Storage.push_back(work); //store STTHB path as well

		for(std::vector<sim_mob::WayPoint>::iterator it(sinPathHighwayBias->path.begin()); it != sinPathHighwayBias->path.end() ;++it)
		{
			const sim_mob::RoadSegment* currSeg = it->roadSegment_;
			if(currSeg->getLink() != curLink)
			{
				curLink = currSeg->getLink();
				PathSetWorkerThread* work = new PathSetWorkerThread();
				work->graph = &sttpImpl->drivingMap_HighwayBias_Default;
				work->segmentLookup = &sttpImpl->drivingSegmentLookup_HighwayBias_Default_;
				work->fromVertex = from.source;
				work->toVertex = to.sink;
				work->fromNode = ps->subTrip.fromLocation.node_;
				work->toNode = ps->subTrip.toLocation.node_;
				blackList.clear();
				blackList.insert(currSeg);
				work->excludeSeg = blackList;
				work->ps = ps;
				std::stringstream out("");
				out << "STTHLE-" << cnt++;
				work->dbgStr = out.str();
				work->timeBased = true;

				if(ConfigManager::GetInstance().PathSetConfig().privatePathSetMode == "generation")
				{
					/*
					 * NOTE:
					 * when pathset runs in "normal" mode, during pathset generation for requested ODs, each method
					 * of pathset generation(link elimination, random perturbation, etc)  will use threadpool for its operation.
					 * Whereas in "generation" mode, each pathset generation task (as a whole) is assigned to a dedicated thread in threadpool.
					 */
					work->run();
				}
				else
				{
					threadpool_->enqueue(boost::bind(&PathSetWorkerThread::run,work));
				}
				STTHBLE_Storage.push_back(work);
			} //ROAD_SEGMENT
		}//for
	} //if sinPathTravelTimeDefault

	//logger  << "waiting for TRAVEL TIME HIGHWAY BIAS" << "\n";
//	if(!cnt) {
//		logger  << "[" << fromToID << "]Nothing supplied to threadpool-STTH" << std::endl;
//	}
}


int sim_mob::PrivatePathsetGenerator::genRandPert(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &RandPertStorage)
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
		//logger << work->dbgStr;
		RandPertStorage.push_back(work);
		work->timeBased = true;

		if(ConfigManager::GetInstance().PathSetConfig().privatePathSetMode == "generation")
		{
			/*
			 * NOTE:
			 * when pathset runs in "normal" mode, during pathset generation for requested ODs, each method
			 * of pathset generation(link elimination, random perturbation, etc)  will use threadpool for its operation.
			 * Whereas in "generation" mode,  each pathset generation task(as a whole) is assigned to a dedicated thread in threadpool.
			 */
			work->run();
		}
		else
		{
			threadpool_->enqueue(boost::bind(&PathSetWorkerThread::run,work));
		}
	}
//	if(!cnt)
//	{
//		logger  << "[" << fromToID << "]Nothing supplied to threadpool-TTRP" << std::endl;
//	}
}

int sim_mob::PrivatePathsetGenerator::generateAllPathChoices(boost::shared_ptr<sim_mob::PathSet> ps, std::set<OD> &recursiveODs, const std::set<const sim_mob::RoadSegment*> & excludedSegs)
{
	Profiler gen("generateAllPathChoices", true);
	//small sanity check
	if(!ps || ps->subTrip.fromLocation.node_ == ps->subTrip.toLocation.node_ || !ps->subTrip.fromLocation.node_ || !ps->subTrip.toLocation.node_)
	{
		return 0;
	}

	std::string fromToID(getFromToString(ps->subTrip.fromLocation.node_,ps->subTrip.toLocation.node_));
	//logger << "generateAllPathChoices" << std::endl;
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

	//K-SHORTEST PATH
	std::set<sim_mob::SinglePath*, sim_mob::SinglePath> KSP_Storage;//main storage for k-shortest path
	if(ConfigManager::GetInstance().PathSetConfig().privatePathSetMode == "generation")
	{
		genK_ShortestPath(ps, KSP_Storage);
	}
	else
	{
		threadpool_->enqueue(boost::bind(&PrivatePathsetGenerator::genK_ShortestPath, this, ps, boost::ref(KSP_Storage)));
	}

	std::vector<std::vector<PathSetWorkerThread*> > mainStorage = std::vector<std::vector<PathSetWorkerThread*> >();
	// SHORTEST DISTANCE LINK ELIMINATION
	std::vector<PathSetWorkerThread*> SDLE_Storage;
	genSDLE(ps, SDLE_Storage);

	//step-3: SHORTEST TRAVEL TIME LINK ELIMINATION
	std::vector<PathSetWorkerThread*> STTLE_Storage;

	genSTTLE(ps,STTLE_Storage);

	// TRAVEL TIME HIGHWAY BIAS
	std::vector<PathSetWorkerThread*> STTHBLE_Storage;
	genSTTHBLE(ps,STTHBLE_Storage);

	//	RANDOM;
	std::vector<PathSetWorkerThread*> randPertStorage;
	genRandPert(ps,randPertStorage);

	if(!(ConfigManager::GetInstance().PathSetConfig().privatePathSetMode == "generation"))
	{
		/*
		 * NOTE:
		 * when pathset runs in "normal" mode, at the time of generating a pathset for requested ODs, each method
		 * of pathset generation(link elimination, random perturbation, etc)  will use threadpool for its operation.
		 * Whereas in "generation" mode,  each pathset generation task(as a whole) is assigned to a dedicated thread in threadpool.
		 * (check the corresponding pathset generation class member methods like genSTTHBLE, genRandPert etc)
		 * Therefore, the following wait is not necessary in case of "generation" mode because "one" thread will take care of all types of
		 * path generations.
		 */
		threadpool_->wait();
	}

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
	BOOST_FOREACH(sim_mob::SinglePath* sp, KSP_Storage)
	{
		ps->addOrDeleteSinglePath(sp);
	}

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
	std::pair <boost::chrono::microseconds,	boost::chrono::microseconds> tick = gen.tick();
	Print() << "[" << fromToID << " PATHSET SIZE: " << ps->pathChoices.size() << " , TIME:" << tick.first.count()/1000000 << " seconds]\n";
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

namespace
{
	std::map<const void*,sim_mob::OneTimeFlag> utilityLogger;
}

std::string sim_mob::PathSetManager::logPartialUtility(const sim_mob::SinglePath* sp, double pUtility) const
{
	//generate log file for debugging only
	if(utilityLogger[nullptr].check())
	{
		sim_mob::Logger::log("partial_utility.txt") << "pathSetId#algorithm#index#travleTime#bTTVOT#travleTime * bTTVOT#pathSize#bCommonFactor#pathSize*bCommonFactor#length#bLength#length*bLength#"
				"highWayDistance#bHighway#highWayDistance*bHighway#travelCost#bCost#travelCost*bCost#signalNumber#bSigInter#signalNumber*bSigInter#rightTurnNumber#bLeftTurns#rightTurnNumber*bLeftTurns#"
				"minTravelTimeParam#isMinTravelTime#minTravelTimeParam*isMinTravelTime#minDistanceParam#isMinDistance#minDistanceParam*isMinDistance#minSignalParam#isMinSignal#minSignalParam*isMinSignal#"
				"maxHighwayParam#isMaxHighWayUsage#maxHighwayParam*isMaxHighWayUsage#purpose#b-value#purpose*b-value#partial-utility" << "\n" ;
	}

	if(utilityLogger[sp].check())
	{
		sp->partialUtilityDbg << sp->pathSetId << "#" << sp->scenario << "#" << sp->index << "#" << sp->travelTime << "#" << pathSetParam->bTTVOT <<  "#" << sp->travelTime * pathSetParam->bTTVOT << "#"
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
	pUtility += sp->length * pathSetParam->bLength ;
	pUtility += sp->highWayDistance * pathSetParam->bHighway;
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
	//comment logging if not needed
	//logPartialUtility(sp,pUtility);
	return pUtility;
}

double sim_mob::PrivateTrafficRouteChoice::generateUtility(const sim_mob::SinglePath* sp) const
{
	if(!sp) { return 0; }
	if(sp->travelTime <= 0.0) { throw std::runtime_error("generateUtility: invalid single path travleTime :"); }
	double partialUtility = (sp->partialUtility > 0.0 ? sp->partialUtility : generatePartialUtility(sp));
	double utility = partialUtility;
	// calculate utility
	//obtain value of time for the agent A: bTTlowVOT/bTTmedVOT/bTThiVOT.
	utility += sp->travelTime * pathSetParam->bTTVOT;
	//obtain travel cost part of utility
	utility += sp->travelCost * pathSetParam->bCost;
	//OD,partialUtility,travleTime,travelCost,utility
	//sim_mob::Logger::log("final_utility.csv") << sp->pathSetId << "," << sp->scenario << "," << partialUtility << "," << sp->travelTime << "," << sp->travelCost << "," << utility << "\n";
	return utility;
}

bool sim_mob::PrivateTrafficRouteChoice::getBestPathChoiceFromPathSet(boost::shared_ptr<sim_mob::PathSet> &ps,
		const std::set<const sim_mob::RoadSegment *> & partialExclusion ,
		const std::set<const sim_mob::RoadSegment*> &blckLstSegs , bool enRoute)
{
	bool computeUtility = false;
	// step 1.1 : For each path i in the path choice:
	//1. set PathSet(O, D)
	//2. travle_time
	//3. utility
	//step 1.2 : accumulate the logsum
	ps->logsum = 0.0;
	//std:ostringstream utilityDbg("");
	//utilityDbg << ps->id << "\nutility:\n";

	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(sp->path.empty()) { throw std::runtime_error ("Empty Path"); }
		if(sp->includesRoadSegment(blckLstSegs)) { continue; } //do the same thing while measuring the probability in the loop below

		if(sp->travelTime <= 0.0 )
		{
			std::stringstream out("");
			out << sp->pathSetId << " getBestPathChoiceFromPathSet=>invalid single path travleTime :" << sp->travelTime;
			throw std::runtime_error(out.str());
		}

		if (sp->includesRoadSegment(partialExclusion) )
		{
			sp->travelTime = std::numeric_limits<double>::max();//some large value like infinity
			sp->utility = generateUtility(sp); //re-calculate utility
		}

		//utilityDbg << "[" << sp->utility << "," << exp(sp->utility) << "]";
		ps->logsum += exp(sp->utility);
	}
	//utilityDbg << "\n\nlogsum: " << ps->logsum;
	// step 2: find the best waypoint path :
	// calculate a probability using path's utility and pathset's logsum,
	// compare the resultwith a  random number to decide whether pick the current path as the best path or not
	//if not, just chose the shortest path as the best path
	double upperProb=0;
	// 2.1 Draw a random number X between 0.0 and 1.0 for agent A.
	double random = genRandomDouble(0,1);
	//utilityDbg << "\nrandom number:" << random << "\n";
	// 2.2 For each path i in the path choice set PathSet(O, D):
	int i = -1;
	BOOST_FOREACH(sim_mob::SinglePath* sp, ps->pathChoices)
	{
		if(sp->includesRoadSegment(blckLstSegs)) { continue; } //do the same thing while processing the single path in the loop above
		i++;
		double prob = exp(sp->utility)/(ps->logsum);
		upperProb += prob;
		//utilityDbg << "[" << sp->scenario << "," << sp->utility << "," << prob << "," << upperProb << "]";
		if (random <= upperProb)
		{
			// 2.3 agent A chooses path i from the path choice set.
			ps->bestPath = &(sp->path);
			//logger << "[LOGIT][" << sp->pathSetId <<  "] [" << i << " out of " << ps->pathChoices.size()  << " paths chosen] [UTIL: " <<  sp->utility << "] [LOGSUM: " << ps->logsum << "][exp(sp->utility)/(ps->logsum) : " << prob << "][X:" << random << "]\n";
			//utilityDbg << "\nselect: " << sp->pathSetId  << "|" << sp->scenario << "|[" << sp->scenario << "," << sp->utility << "," << prob << "," << upperProb << "]" << "\n";
			//sim_mob::Logger::log("path_selection") << utilityDbg.str() << "\n-------------------------------------------------------\n";
			return true;
		}
	}
	//sim_mob::Logger::log("path_selection") << utilityDbg.str() << "\n-------------------------------------------------------\n";

	// path choice algorithm
	if(!ps->oriPath)//return upon null oriPath only if the condition is normal(excludedSegs is empty)
	{
		//logger<< "NO PATH , getBestPathChoiceFromPathSet, shortest path empty" << "\n";
		//sim_mob::Logger::log("path_selection") << utilityDbg.str() << "\n-------------------------------------------------------\n";
		return false;
	}
	//the last step resorts to selecting and returning shortest path(aka oripath).
	//logger << "NO BEST PATH. select to shortest path\n" ;
	ps->bestPath = &(ps->oriPath->path);
	//sim_mob::Logger::log("path_selection") << utilityDbg.str() << "\n-------------------------------------------------------\n";
	return true;
}

sim_mob::SinglePath *  sim_mob::PrivatePathsetGenerator::findShortestDrivingPath(
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
		//std::stringstream out("");
//		if(excludedSegs.size())
//		{
//
//			const sim_mob::RoadSegment* rs;
//			out << "\nWith Excluded Segments Present: \n";
//			BOOST_FOREACH(rs, excludedSegs)
//			{
//				out <<	rs->originalDB_ID.getLogItem() << "]" << ",";
//			}
//		}
//		logger<< "No shortest driving path for nodes[" << fromNode->originalDB_ID.getLogItem() << "] and ["
//				<< toNode->originalDB_ID.getLogItem() << "]" << out.str() << "\n";
		return s;
	}
	// make sp id
	std::string id = sim_mob::makeWaypointsetString(wp);
//	if(!id.size()){
//		logger << "Error: Empty shortest path for OD:" <<  fromNode->getID() << "," << toNode->getID() << "\n" ;
//	}
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
//	else{
//		logger<<"gSPByFTNodes3:duplicate pathset discarded\n";
//	}

	return s;
}

sim_mob::SinglePath* sim_mob::PrivatePathsetGenerator::generateShortestTravelTimePath(const sim_mob::Node *fromNode, const sim_mob::Node *toNode,
		sim_mob::TimeRange tr, const sim_mob::RoadSegment* excludedSegs, int random_graph_idx)
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
//		logger<<"generateShortestTravelTimePath: no path for nodes"<<fromNode->originalDB_ID.getLogItem()<<
//				toNode->originalDB_ID.getLogItem() << "\n";
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



double sim_mob::PrivateTrafficRouteChoice::getPathTravelTime(sim_mob::SinglePath *sp,const std::string & travelMode, const sim_mob::DailyTime & startTime_, bool enRoute)
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
			std::stringstream ss;
			ss << "No Travel Time [SEGMENT: " << rs->getId() << "] [START TIME : " << startTime.getStrRepr() << "]\n";
			throw std::runtime_error(ss.str());
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

double sim_mob::PrivateTrafficRouteChoice::getInSimulationSegTT(const sim_mob::RoadSegment* rs, const std::string &travelMode, const sim_mob::DailyTime &startTime) const
{
	return processTT.getInSimulationSegTT(travelMode,rs);
}

void sim_mob::PathSetManager::initTimeInterval()
{
	intervalMS = sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().interval * 1000 /*milliseconds*/;
	if(intervalMS <= 0) { throw runtime_error("invalid interval specified in config file"); }
	uint32_t startTm = ConfigManager::GetInstance().FullConfig().simStartTime().getValue();
	curIntervalMS = TravelTimeManager::getTimeInterval(startTm, intervalMS);
}

void sim_mob::PathSetManager::updateCurrTimeInterval()
{
	curIntervalMS += intervalMS;
}

sim_mob::PrivatePathsetGenerator::PrivatePathsetGenerator() : PathSetManager(), stdir(StreetDirectory::instance())
{
	if(!threadpool_)
	{
		threadpool_.reset(new sim_mob::batched::ThreadPool(sim_mob::ConfigManager::GetInstance().PathSetConfig().threadPoolSize));
	}
}

sim_mob::PrivatePathsetGenerator::~PrivatePathsetGenerator()
{}

sim_mob::PrivateTrafficRouteChoice::PrivateTrafficRouteChoice() : PathSetManager(),
		psRetrieval(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().psRetrieval),
		psRetrievalWithoutRestrictedRegion(sim_mob::ConfigManager::GetInstance().FullConfig().pathSet().psRetrievalWithoutBannedRegion),
		cacheLRU(2500), processTT(*(sim_mob::TravelTimeManager::getInstance()))
{}

sim_mob::PrivateTrafficRouteChoice::~PrivateTrafficRouteChoice()
{}

PrivateTrafficRouteChoice* sim_mob::PrivateTrafficRouteChoice::getInstance()
{
	return PrivateRouteChoiceProvider::getPvtRouteChoiceModel();
}

PrivatePathsetGenerator* sim_mob::PrivatePathsetGenerator::getInstance()
{
	if(!instance_)
	{
		instance_ = new PrivatePathsetGenerator();
	}
	return instance_;
}

void sim_mob::PrivatePathsetGenerator::resetInstance()
{
	delete instance_;
	instance_ = nullptr;
}

void sim_mob::PrivateTrafficRouteChoice::addPartialExclusion(const sim_mob::RoadSegment* value)
{
	partialExclusions.insert(value);
}
