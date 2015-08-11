/*
 * PathSetManager.hpp
 *
 *  Created on: May 6, 2013
 *      Author: Max
 *      Author: Vahid
 */

#pragma once

#include "PathSetParam.hpp"
#include "TravelTimeManager.hpp"
#include "geospatial/Link.hpp"
#include "entities/Person.hpp"
#include "util/Cache.hpp"

namespace sim_mob
{

namespace batched
{
	class ThreadPool;
}

class PathSetWorkerThread;

/**
 * Path set manager class
 *
 * \author Vahid Saber Hamishagi
 * \author Harish Loganathan
 * \author Balakumar Marimuthu
 */
class PathSetManager
{
public:
	PathSetManager();
	~PathSetManager();

	/**
	 * get the database session used for this thread
	 */
	const boost::shared_ptr<soci::session>& getSession();

	void setScenarioName(std::string& name) { scenarioName = name; }

	/**
	 * check whether a given path is black listed
	 * @param path waypoint path
	 * @param blkLst black list to check against
	 */
	bool pathInBlackList(const std::vector<WayPoint> path, const std::set<const sim_mob::RoadSegment*> & blkLst) const;

	/**
	 * calculate those part of the utility function that are always fixed(like path length)
	 * and are not going to change(like travel time)
	 * @param sp the input path
	 */
	double generatePartialUtility(const sim_mob::SinglePath* sp) const;

	/**
	 * Generates a log file to validate the computation of partial utility
	 * @param sp the given singlepath
	 * @param pUtility the already computed utility
	 * @return the generated string
	 */
	std::string logPartialUtility(const sim_mob::SinglePath* sp, double pUtility) const;

	/**
	 * basically delete all the dynamically allocated memories, in addition to some more cleanups
	 * @param ps pathset
	 */
	void clearSinglePaths(boost::shared_ptr<sim_mob::PathSet> &ps);

	/**
	 * initializes intervalMS
	 */
	static void initTimeInterval();

	/**
	 * updates curIntervalMS
	 * must be called once every tick
	 */
	static void updateCurrTimeInterval();

	/** time interval value used for processing data. */
	static unsigned int intervalMS;

	/**
	* current time interval, with respect to simulation time
	* this is used to avoid continuous calculation of the current time interval.
	* Note: Updating this happens once in one of the barriers, currently
	* Distribute messages barrier(void sim_mob::WorkGroupManager::waitAllGroups_AuraManager())
	*/
	static unsigned int curIntervalMS;

protected:
	/**	link to pathset paramaters */
	PathSetParam* pathSetParam;

	/**	stores the name of database's singlepath table//todo:doublecheck the usability */
	const std::string &pathSetTableName;

	/** contains arbitrary description usually to indicating which configuration file the generated data has originated from */
	std::string scenarioName;

	/** postgres session to query pathsets */
	boost::shared_ptr<soci::session> dbSession;
};

/**
 * class responsible for generation of private traffic pathsets
 *
 * \author Vahid Saber Hamishagi
 * \author Harish Loganathan
 * \author Balakumar Marimuthu
 */
class PrivatePathsetGenerator : boost::noncopyable, public sim_mob::PathSetManager
{
private:
	static PrivatePathsetGenerator* instance_;
	static boost::mutex instanceMutex;

	/** reference to street directory */
	StreetDirectory& stdir;

	/** pool of threads for generation of paths */
	static boost::shared_ptr<sim_mob::batched::ThreadPool> threadpool_;

	/**
	 * generate shortest distance path
	 * @param fromNode origin
	 * @param toNode destination
	 * @param duplicateChecker a set to help avoid generating duplicates
	 * @param excludedSegs set of black listed segments that must not be a part of generated path
	 * @returns shortest distance driving path
	 */
	sim_mob::SinglePath* findShortestDrivingPath(const sim_mob::Node* fromNode, const sim_mob::Node* toNode, std::set<std::string> duplicateChecker,
			const std::set<const sim_mob::RoadSegment*>& excludedSegs=std::set<const sim_mob::RoadSegment*>());

	/**
	 * generate a path based on shortest travel time
	 * @param fromNode origin
	 * @param toNode destination
	 * @param tr time period for path generation
	 * @param excludedSegs set of black listed segments that must not be part of generated path
	 * @param random_graph_idx parameter for random perturbation
	 * @returns shortest travel time path for the specified time period
	 */
	sim_mob::SinglePath* generateShortestTravelTimePath(const sim_mob::Node* fromNode, const sim_mob::Node* toNode,
			sim_mob::TimeRange tr=sim_mob::MorningPeak, const sim_mob::RoadSegment* excludedSegs=NULL, int random_graph_idx=0);

	 /**
	  * generate K-shortest path
	  * @param pathset general information
	  * @param KSP_Storage output
	  * @returns the number of paths generated
	  */
	 int genK_ShortestPath(boost::shared_ptr<sim_mob::PathSet> &ps, std::set<sim_mob::SinglePath*, sim_mob::SinglePath> &KSP_Storage);

	 /**
	  * generate path by shortest distance link elimination
	  * @param pathset general information
	  * @param SDLE_Storage output
	  * @returns the number of paths generated (0 or 1)
	  */
	 int genSDLE(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &SDLE_Storage);

	 /**
	  * generate path by shortest travel time link elimination
	  * @param pathset general information
	  * @param STTLE_Storage output
	  * @returns the number of paths generated (0 or 1)
	  */
	 int genSTTLE(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &STTLE_Storage);

	 /**
	  * generate path by shortest travel time link elimination with highway bias
	  * @param pathset general information
	  * @param STTHBLE_Storage output
	  * @returns the number of paths generated (0 or 1)
	  */
	 int genSTTHBLE(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &STTHBLE_Storage);

	 /**
	  * generate path by random perturbation
	  * @param pathset general information
	  * @param RandPertStorage output
	  * @returns the number of paths generated (0 or 1)
	  */
	 int genRandPert(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &RandPertStorage);

	 /**
	  * set some tags as a result of comparing attributes among paths in a pathset
	  * @param ps general information
	  */
	 void setPathSetTags(boost::shared_ptr<sim_mob::PathSet>& ps);

	/**
	 * post pathset generation processes
	 * @param ps the input pathset
	 */
	void onGeneratePathSet(boost::shared_ptr<PathSet> &ps);

public:
	PrivatePathsetGenerator();
	virtual ~PrivatePathsetGenerator();

	/**
	 * gets the singleton instance of pathset manager
	 */
	static PrivatePathsetGenerator* getInstance();

	/**
	 * deletes the singleton instance
	 */
	static void resetInstance();

	/**
	 * generate all the paths for a person given its subtrip(OD)
	 * @param per input agent applying to get the path
	 * @param st input subtrip
	 * @param res output path generated
	 * @param excludedSegs input list segments to be excluded from the target set
	 * @param isUseCache is using the cache allowed
	 * @return number of paths generated
	 */
	int generateAllPathChoices(boost::shared_ptr<sim_mob::PathSet> ps, std::set<OD> &recursiveODs, const std::set<const sim_mob::RoadSegment*> & excludedSegs);

	/**
	 *	offline pathset generation method.
	 *	This method collects the distinct demands from database,
	 *	creates a set of distinct demands and supplies them one by one to generateAllPathChoices.
	 *	it creates recursive paths if proper settings are configured.
	 *	The out put will be a csv file ready to be inserted into database.
	 */
	void bulkPathSetGenerator();
};

/**
 * class responsible for performing route choice for private traffic vehicles
 *
 * \author Vahid Saber Hamishagi
 * \author Harish Loganathan
 * \author Balakumar Marimuthu
 */
class PrivateTrafficRouteChoice : public sim_mob::PathSetManager
{
private:
	/**	the pathset cache */
	sim_mob::LRU_Cache<std::string, boost::shared_ptr<PathSet> > cacheLRU;

	/**
	 * list of partially excluded segments
	 * example:like segments with incidents which have to be assigned a maximum travel time
	 */
	std::set<const sim_mob::RoadSegment*> partialExclusions;

	/**	protect access to incidents list */
	boost::shared_mutex mutexExclusion;

	/**	stores the name of database's function operating on the pathset and singlepath tables */
	const std::string& psRetrieval;

	/**	stores the name of database's function operating on the pathset and singlepath tables */
	const std::string& psRetrievalWithoutRestrictedRegion;

	/**	Travel time processing */
	const TravelTimeManager& processTT;

	/**
	 * cache the generated pathset
	 * @param ps pathset general information
	 */
	void cachePathSet(boost::shared_ptr<sim_mob::PathSet> &ps);

	/**
	 * searches for a pathset in the cache.
	 * @param key indicates the input key
	 * @param value the result of the search
	 * returns true/false to indicate if the search has been successful
	 */
	bool findCachedPathSet(std::string key, boost::shared_ptr<sim_mob::PathSet> &value);

	/**
	 * calculates the travel time of a path
	 * @param sp the given path
	 * @param travelMode mode of travelling through the path
	 * @startTime when to start the path
	 * @enRoute decided whether in simulation travel time should be searched or not
	 * @returns path's travel time
	 */
	double getPathTravelTime(sim_mob::SinglePath *sp,const std::string & travelMode, const sim_mob::DailyTime & startTime, bool enRoute = false);

	/**
	 * update pathset paramenters before selecting the best path
	 * @param ps the input pathset
	 * @param enRoute decides if travel time retrieval should included in simulation travel time or not
	 */
	void onPathSetRetrieval(boost::shared_ptr<PathSet> &ps, bool enRoute);

	/**
	 * calculates utility of the given path those part of the utility function that are always fixed(like path length)
	 * and are not going to change(like travel time)
	 * @param sp the target path
	 */
	double generateUtility(const sim_mob::SinglePath* sp) const;

	/**
	 * Get best path from given pathsets
	 * @param ps input pathset choices
	 * @param partialExclusion input segments temporarily having different attributes
	 * @param blckLstSegs segments off the road network. This
	 * @param enRoute is this method called for an enroute path request
	 * Note: PathsetManager object already has containers for partially excluded and blacklisted segments. They will be
	 * the default containers throughout the simulation. but partialExcludedSegs and blckLstSegs arguments are combined
	 * with their counterparts in PathSetmanager only during the scope of this method to serve temporary purposes.
	 */
	bool getBestPathChoiceFromPathSet(boost::shared_ptr<sim_mob::PathSet> &ps,
			const std::set<const sim_mob::RoadSegment *> & partialExclusion,
			const std::set<const sim_mob::RoadSegment*> &blckLstSegs, bool enRoute);

public:
	PrivateTrafficRouteChoice();
	virtual ~PrivateTrafficRouteChoice();

	/**
	 * gets the thread specific instance of pathset manager
	 */
	static PrivateTrafficRouteChoice* getInstance();

	/**
	 * gets the average travel time of a segment experienced during the current simulation.
	 * Whether the desired travel time is coming from the last time interval
	 * or from the average of all previous time intervals is implementation dependent.
	 * @param rs input road segment
	 * @param travelMode intended mode of traversing the segment
	 * @param startTime indicates when the segment is to be traversed.
	 * @return travel time in seconds
	 */
	double getInSimulationSegTT(const sim_mob::RoadSegment* rs, const std::string &travelMode, const sim_mob::DailyTime &startTime) const;

	/**
	 * insert roadsegment into incident list
	 * @param rs road segment to insert
	 */
	void insertIncidentList(const sim_mob::RoadSegment* rs);

	/**
	 * add to exclusion list
	 */
	void addPartialExclusion(const sim_mob::RoadSegment* value);

	/**
	 * find/generate set of path choices for a given subtrip, and then return the best of them
	 * @param st input subtrip
	 * @param res output path generated
	 * @param partialExcludedSegs segments temporarily having different attributes
	 * @param blckLstSegs segments off the road network. This
	 * @param tempBlckLstSegs segments temporarily off the road network
	 * @param enRoute is this method called for an enroute path request
	 * @param approach if this is an entoute, from which segment is it permitted to enter the rerouting point to start a new path
	 * Note: PathsetManager object already has containers for partially excluded and blacklisted segments. They will be
	 * the default containers throughout the simulation. but partialExcludedSegs and blckLstSegs arguments are combined
	 * with their counterparts in PathSetmanager only during the scope of this method to serve temporary purposes.
	 */
	 bool getBestPath(std::vector<sim_mob::WayPoint>& res,
			 const sim_mob::SubTrip& st,bool useCache,
			 const std::set<const sim_mob::RoadSegment*> tempBlckLstSegs/*=std::set<const sim_mob::RoadSegment*>()*/,
			 bool usePartialExclusion ,
			 bool useBlackList ,
			 bool enRoute ,const sim_mob::RoadSegment* approach);

	/**
	 * The main entry point to the pathset manager,
	 * returns a path for the requested subtrip
	 * @param per the requesting person (todo:for logging purpose only)
	 * @subTrip the subtrip information containing OD, start time etc
	 * @enRoute indication of whether this request was made in the beginning of the trip or enRoute
	 * @return a sequence of road segments wrapped in way point structure
	 */
	std::vector<WayPoint> getPath(const sim_mob::SubTrip &subTrip, bool enRoute , const sim_mob::RoadSegment* approach);
};

}//namespace
