
#pragma once

#include <boost/shared_ptr.hpp>
#include <soci/soci.h>
#include <conf/ConfigManager.hpp>
#include "entities/Person.hpp"
#include "geospatial/network/Link.hpp"
#include "PathSetParam.hpp"
#include "entities/TravelTimeManager.hpp"
#include "util/Cache.hpp"
#include "lua/LuaModel.hpp"
#include "Path.hpp"
#include "util/OneTimeFlag.hpp"


namespace sim_mob
{

namespace batched
{
    class ThreadPool;
}

//FDs for RestrictedRegion
class Node;
class TripChainItem;
class WayPoint;

enum HasPath
{
    PSM_HASPATH,//found and valid
    PSM_NOGOODPATH,//previous attempt to build pathset failed
    PSM_NOTFOUND,//search didn't find anything
    PSM_UNKNOWN
};

/**
 * from_section, to_section pair
 * \author Vahid
 */
class CBD_Pair
{
public:
    int from_section; //to avoid confusion, code style conforms to database
    int to_section; //to avoid confusion, code style conforms to database
};

/**
 * Manager class for any restricted region used for case studies
 * \author Vahid
 */
class RestrictedRegion : private boost::noncopyable
{
private:
    /**<id, node>*/
    std::map<unsigned int, const Node*> zoneNodes;
    
    /**
     * set of all links of the restricted area(aka CBD)
     */
    std::set<const sim_mob::Link *> zoneLinks;
    sim_mob::OneTimeFlag populated;

public:
    RestrictedRegion();
    virtual ~RestrictedRegion();

    /**
     * does the given "node"(or node wrapped in a WayPoint) lie in the restricted area,
     * returns the periphery node if the target is in the restricted zone
     * returns null if Node not found in the restricted region.
     */
    bool isInRestrictedZone(const Node* target) const;
    bool isInRestrictedZone(const WayPoint& target) const;

    /**
     * does the given Path "RoadSegments" and "Nodes"(segments wrapped in WayPoints) lie in the restricted area,
     * returns true if any part of the target is in the restricted zone
     */
    bool isInRestrictedZone(const std::vector<WayPoint>& target) const;

    /**
     * does the given "Link" lie in the restricted area,
     * returns true if the target is in the restricted zone
     */
    bool isInRestrictedZone(const sim_mob::Link * target) const;

    /**
     * fill the input data into in,out,zoneSegments
     * generate data based on input for zoneNodes
     */
    void populate();

    static boost::shared_ptr<RestrictedRegion> instance;

    static RestrictedRegion & getInstance()
    {
        if(!instance)
        {
            instance.reset(new RestrictedRegion());
        }
        return *instance;
    }
};

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

    /**
     * basically delete all the dynamically allocated memories, in addition to some more cleanups
     * @param ps pathset to clear
     */
    void clearPathSet(boost::shared_ptr<sim_mob::PathSet> &ps);

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
    /** link to pathset paramaters */
    PathSetParam* pathSetParam;

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
    static PrivatePathsetGenerator* pvtPathGeneratorInstance;
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
    sim_mob::SinglePath* findShortestDrivingPath(const sim_mob::Node* fromNode, const sim_mob::Node* toNode,
            const std::set<const sim_mob::Link*>& excludedLinks=std::set<const sim_mob::Link*>());

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
            sim_mob::TimeRange tr=sim_mob::MorningPeak, std::set<const Link *> excludedLinks = std::set<const Link *>(), int random_graph_idx=0);

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
     void genSDLE(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &SDLE_Storage);

     /**
      * generate path by shortest travel time link elimination
      * @param pathset general information
      * @param STTLE_Storage output
      * @returns the number of paths generated (0 or 1)
      */
     void genSTTLE(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &STTLE_Storage);

     /**
      * generate path by shortest travel time link elimination with highway bias
      * @param pathset general information
      * @param STTHBLE_Storage output
      * @returns the number of paths generated (0 or 1)
      */
     void genSTTHBLE(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &STTHBLE_Storage);

     /**
      * generate path by random perturbation
      * @param pathset general information
      * @param RandPertStorage output
      * @returns the number of paths generated (0 or 1)
      */
     void genRandPert(boost::shared_ptr<sim_mob::PathSet> &ps,std::vector<PathSetWorkerThread*> &RandPertStorage);

     /**
      * set some tags as a result of comparing attributes among paths in a pathset
      * @param ps general information
      */
     void setPathSetTags(boost::shared_ptr<sim_mob::PathSet>& ps) const;

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
     * generate all the paths for a set of ODs
     * @param st input subtrip
     * @param res output path generated
     * @return number of paths generated
     */
    int generateAllPathChoices(boost::shared_ptr<sim_mob::PathSet> ps, std::set<OD> &recursiveODs);

    /**
     *  offline pathset generation method.
     *  This method collects the distinct demands from database,
     *  creates a set of distinct demands and supplies them one by one to generateAllPathChoices.
     *  it creates recursive paths if proper settings are configured.
     *  The out put will be a csv file ready to be inserted into database.
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
class PrivateTrafficRouteChoice : public sim_mob::PathSetManager , public lua::LuaModel
{
private:
    /** the pathset cache */
    sim_mob::LRU_Cache<std::string, boost::shared_ptr<PathSet> > cacheLRU;

    /**
     * list of partially excluded links
     * example:like links with incidents which have to be assigned a maximum travel time
     */
    std::set<const sim_mob::Link*> partialExclusions;

    /** protect access to incidents list */
    boost::shared_mutex mutexExclusion;

    /** stores the name of database's function operating on the pathset and singlepath tables */
    std::string psRetrieval;

    /** stores the name of database's function operating on the pathset and singlepath tables */
    const std::string psRetrievalWithoutRestrictedRegion;

    /** Travel time processing */
    const TravelTimeManager& ttMgr;

    /** flag to indicate whether restricted region case study is enabled*/
    bool regionRestrictonEnabled;

    std::vector<sim_mob::SinglePath*> pvtpathset;

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
     * @param useInSimulationTT indicates whether in simulation travel times are to be used
     * @returns path's travel time, in seconds
     */
    double getPathTravelTime(sim_mob::SinglePath *sp, const sim_mob::DailyTime & startTime, bool enRoute = false, bool useInSimulationTT = false);

    /**
     * update pathset paramenters before selecting the best path
     * @param ps the input pathset
     * @param enRoute decides if travel time retrieval should included in simulation travel time or not
     * @param useInSimulationTT indicates whether in simulation travel times are to be used
     */
    void onPathSetRetrieval(boost::shared_ptr<PathSet> &ps, bool enRoute, bool useInSimulationTT = false);

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
     * @param approach the starting link of the path
     * Note: PathsetManager object already has containers for partially excluded and blacklisted segments. They will be
     * the default containers throughout the simulation. but partialExcludedSegs and blckLstSegs arguments are combined
     * with their counterparts in PathSetmanager only during the scope of this method to serve temporary purposes.
     */
    bool getBestPathChoiceFromPathSet(boost::shared_ptr<sim_mob::PathSet> &ps,
            const std::set<const sim_mob::Link*>& partialExclusion,
            const std::set<const sim_mob::Link*>& blckLstLnks, bool enRoute,
            const sim_mob::Link *approach);

    void mapClasses();

    /**
     * loads set of paths pre-generated for an OD
     *
     * @param sql soci session to use for querying DB
     * @param pathsetId <origin_node>,<destination_node> in string format
     * @param spPool output set of SinglePaths
     * @param functionName name of DB stored procedure to fetch pathset for an OD
     * @param excludedLnks set of black listed links (if any)
     *
     * @return status of pathset retrieval as an enumerated value from sim_mob::HasPath
     */
    sim_mob::HasPath loadPathsetFromDB(soci::session& sql,
            std::string& pathsetId,
            std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool,
            const std::string functionName,
            const std::set<const sim_mob::Link*>& excludedRS = std::set<const sim_mob::Link*>()) const;

public:
    PrivateTrafficRouteChoice();
    virtual ~PrivateTrafficRouteChoice();

    double getTravelCost(unsigned int index);
    double getTravelTime(unsigned int index);
    double getPathSize(unsigned int index);
    double getLength(unsigned int index);
    double getPartialUtility(unsigned int index);
    double getHighwayDistance(unsigned int index);
    double getSignalNumber(unsigned int index);
    double getRightTurnNumber(unsigned int index);
    int isMinDistance(unsigned int index);
    int isMinSignal(unsigned int index);
    int isMaxHighWayUsage(unsigned int index);
    int getPurpose(unsigned int index);



    /**
     * gets the thread specific instance of pathset manager
     */
    static PrivateTrafficRouteChoice* getInstance();

    bool isRegionRestrictonEnabled() const;
    void setRegionRestrictonEnabled(bool regionRestrictonEnabled);

    /**
     * gets the average travel time for link experienced during the current simulation.
     * Whether the desired travel time is coming from the last time interval
     * or from the average of all previous time intervals is implementation dependent.
     * @param rs input road segment
     * @param travelMode intended mode of traversing the segment
     * @param startTime indicates when the segment is to be traversed.
     * @return travel time in seconds
     */
    double getInSimulationLinkTT(const sim_mob::Link* lnk, const sim_mob::DailyTime &startTime) const;

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
     * @param approach the starting link of the path
     * @param useInSimulationTT indicates whether in-simulation link travel times are to be used
     * Note: PathsetManager object already has containers for partially excluded and blacklisted segments. They will be
     * the default containers throughout the simulation. but partialExcludedSegs and blckLstSegs arguments are combined
     * with their counterparts in PathSetmanager only during the scope of this method to serve temporary purposes.
     */
    bool getBestPath(std::vector<sim_mob::WayPoint> &res,
                     const sim_mob::SubTrip &st, bool useCache,
                     const std::set<const sim_mob::Link *> tempBlckLstSegs/*=std::set<const sim_mob::RoadSegment*>()*/,
                     bool usePartialExclusion,
                     bool useBlackList,
                     bool enRoute, const sim_mob::Link *approach,
                     bool useInSimulationTT = false);

    bool getBestPathToLink(std::vector<sim_mob::WayPoint> &res,
                            const sim_mob::SubTrip &st, bool useCache,
                            const std::set<const sim_mob::Link *> tempBlckLstSegs/*=std::set<const sim_mob::RoadSegment*>()*/,
                            bool usePartialExclusion,
                            bool useBlackList,
                            bool enRoute, const sim_mob::Link *approach, boost::shared_ptr<sim_mob::PathSet> &pathset,
                            const Lane *currLane,
                            const Link *last,
                            bool useInSimulationTT = false);
    bool getBestPath(std::vector<sim_mob::WayPoint> &res,
                     const sim_mob::SubTrip &st, bool useCache,
                     const std::set<const sim_mob::Link *> tempBlckLstSegs/*=std::set<const sim_mob::RoadSegment*>()*/,
                     bool usePartialExclusion,
                     bool useBlackList,
                     bool enRoute, const sim_mob::Link *approach,
                     bool useInSimulationTT, bool IsDriverControllerStudyEnable);

    bool getBestPathToLink(std::vector<sim_mob::WayPoint> &res,
                           const sim_mob::SubTrip &st, bool useCache,
                           const std::set<const sim_mob::Link *> tempBlckLstSegs/*=std::set<const sim_mob::RoadSegment*>()*/,
                           bool usePartialExclusion,
                           bool useBlackList,
                           bool enRoute, const sim_mob::Link *approach, boost::shared_ptr<sim_mob::PathSet> &pathset,
                           const Lane *currLane,
                           const Link *last,
                           bool useInSimulationTT, bool IsDriverControllerStudyEnable);

    /**
     * The main entry point to the pathset manager,
     * returns a path for the requested subtrip per the requesting person
     * @param subTrip the subtrip information containing OD, start time etc
     * @param enRoute indication of whether this request was made in the beginning of the trip or enRoute
     * @param approach the starting road segment of the path (if null, the argument will be ignored)
     * @param useInSimulationTT indicates whether in-simulation link travel times are to be used
     * @return a sequence of road segments wrapped in way point structure
     */
    std::vector<WayPoint> getPath(const sim_mob::SubTrip &subTrip, bool enRoute , const sim_mob::Link *approach, bool useInSimulationTT = false);

    std::vector<WayPoint> getPathToLink(const sim_mob::SubTrip &subTrip, bool enRoute, const sim_mob::Link *approach,
                                        const Lane *currLane, const Link *last, bool useInSimulationTT = false);
    std::vector<WayPoint> getPath(const sim_mob::SubTrip &subTrip, bool enRoute , const sim_mob::Link *approach, bool useInSimulationTT , bool IsDriverControllerStudyEnable);

    std::vector<WayPoint> getPathToLink(const sim_mob::SubTrip &subTrip, bool enRoute, const sim_mob::Link *approach,
                                        const Lane *currLane, const Link *last, bool useInSimulationTT, bool IsDriverControllerStudyEnable);

    void filterPathsetsByLastLink(boost::shared_ptr<sim_mob::PathSet>& pathset,const Link* link);
    /**
     * calculates the travel time of the shortest path in the pathset for a given OD
     * @param origin origin node id
     * @param destination destination node id
     * @param curTime time at which route choice is to be done
     *
     * @return in vehicle travel time of shortest path in the pathset for given O and D; -1 if no pathset is available for the OD. In seconds
     */
    double getOD_TravelTime(unsigned int origin, unsigned int destination, const sim_mob::DailyTime& curTime);
    //for OD Travel Time Estimation for Study Area for On Call Controller
    double getOD_TravelTime_StudyArea(unsigned int origin, unsigned int destination, const sim_mob::DailyTime& curTime);
    double getShortestPathTravelTime(const Node* origin, const Node* destination, const sim_mob::DailyTime& curTime);
};

}//namespace
