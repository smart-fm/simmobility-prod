#pragma once

#include <sstream>
#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "geospatial/network/WayPoint.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/params/PT_NetworkEntities.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"

namespace sim_mob
{
/*****************************************************
 ******************* Single Path *********************
 *****************************************************
 */
//forward declaration
class Link;
class RoadSegment;
class Node;
class SubTrip;
class SinglePath;
class Lane;

enum TRIP_PURPOSE
{
	work = 1,
	leisure = 2
};

/**
 * counts the number of right turns and signals in path
 * @param sp pointer to SinglePath object countaining the path
 */
void countRightTurnsAndSignals(sim_mob::SinglePath *sp);

/**
 * Returns total length of highway links in path
 * @param sp pointer to SinglePath object countaining the path
 * @return total highway length of the path in meter
 */
double calculateHighWayDistance(sim_mob::SinglePath *sp);

/**
 * Returns path length in meter
 * @param wp collection of links wrapped in waypoint
 * @return total length of the path in meter
 */
double generatePathLength(const std::vector<sim_mob::WayPoint>& wp);

/**
 * computes the default travel time for path
 * @param wp collection of links wrapped in waypoint
 */
double calculateSinglePathDefaultTT(const std::vector<sim_mob::WayPoint>& wp);

/**
 * Constructs a string of comma separated link-ids from a waypoint path
 * @param wp input waypoint path
 * @return csv of link-ids od links in wp
 */
std::string makePathString(const std::vector<WayPoint>& wp);
std::string makePT_PathString(const std::vector<PT_NetworkEdge> &path);
std::string makePT_PathSetString(const std::vector<PT_NetworkEdge> &path);

class SinglePath
{
public:
	/// path representation
	std::vector<sim_mob::WayPoint> path;

	bool isNeedSave2DB;
	std::string scenario;
	std::string id;   //id: seg1id_seg2id_seg3id
	std::string pathSetId;

	double travelCost;
	double travelTime;

	/// time independent part of utility(used for optimization purposes)
	double partialUtility;
	mutable std::stringstream partialUtilityDbg;
	double utility;
	double pathSize;

	double highWayDistance;
	int signalNumber;
	int rightTurnNumber;
	double length; //length in meter
	sim_mob::TRIP_PURPOSE purpose;

	bool isMinTravelTime;
	bool isMinDistance;
	bool isMinSignal;
	bool isMinRightTurn;
	bool isMaxHighWayUsage;
	bool isShortestPath;

	bool valid_path;


	SinglePath(const SinglePath &source);
	///	extract the segment waypoint from series og node-segments waypoints
	SinglePath();
	~SinglePath();
	void init(std::vector<WayPoint>& wpPools);
	void clear();

	 bool operator() (const SinglePath* lhs, const SinglePath* rhs) const
	 {
		 return lhs->id < rhs->id;
	 }

	///does these SinglePath include the any of given RoadSegment(s)
	 bool includesLink(const sim_mob::Link* lnk) const;
	bool includesLinks(const std::set<const sim_mob::Link*>& lnks) const;
	static void filterOutNodes(std::vector<sim_mob::WayPoint>& input, std::vector<sim_mob::WayPoint>& output);

	double getHighWayDistance() const {
		return highWayDistance;
	}

	bool isIsMaxHighWayUsage() const {
		return isMaxHighWayUsage;
	}

	bool isIsMinDistance() const {
		return isMinDistance;
	}

	bool isIsMinSignal() const {
		return isMinSignal;
	}

	double getLength() const {
		return length;
	}

	double getPathSize() const {
		return pathSize;
	}

	sim_mob::TRIP_PURPOSE getPurpose() const {
		return purpose;
	}

	int getRightTurnNumber() const {
		return rightTurnNumber;
	}

	int getSignalNumber() const {
		return signalNumber;
	}

	double getTravelCost() const {
		return travelCost;
	}

	double getTravelTime() const {
		return travelTime;
	}

	double getPartialUtility() const {
		return partialUtility;
	}
};



/*****************************************************
 ******************* Path Set ************************
 *****************************************************
 */
class PathSet
{
public:
	PathSet():  logsum(0.0),hasPath(false),bestPath(nullptr),oriPath(nullptr),isNeedSave2DB(false),id(""),nonCDB_OD(false) {pathChoices.clear();}
	~PathSet();

	short addOrDeleteSinglePath(sim_mob::SinglePath* s);
	std::vector<WayPoint> *bestPath;  //best choice
	SinglePath* oriPath;  // shortest path with all segments
	std::set<sim_mob::SinglePath*, sim_mob::SinglePath> pathChoices;
	boost::shared_mutex pathChoicesMutex;
	bool isNeedSave2DB;
	double logsum;
	// pathset use info of subtrip to get start, end, travel start time...
	sim_mob::SubTrip subTrip;
	std::string id;
	std::string excludedPaths;
	std::string scenario;
	bool hasPath;
	bool nonCDB_OD;
	PathSet(boost::shared_ptr<sim_mob::PathSet> &ps);
};

//Public Transit path

class PT_Path
{
public:

	PT_Path(const std::vector<PT_NetworkEdge>& path);
	PT_Path();
	~PT_Path();

	bool isMinDistance() const {
		return minDistance;
	}

	void setMinDistance(bool minDistance) {
		this->minDistance = minDistance;
	}

	bool isMinInVehicleTravelTimeSecs() const {
		return minInVehicleTravelTime;
	}

	void setMinInVehicleTravelTimeSecs(bool minInVehicleTravelTimeSecs) {
		this->minInVehicleTravelTime = minInVehicleTravelTimeSecs;
	}

	bool isMinNumberOfTransfers() const {
		return minNumberOfTransfers;
	}

	void setMinNumberOfTransfers(bool minNumberOfTransfers) {
		this->minNumberOfTransfers = minNumberOfTransfers;
	}

	bool isMinTravelOnBus() const {
		return minTravelOnBus;
	}

	void setMinTravelOnBus(bool minTravelOnBus) {
		this->minTravelOnBus = minTravelOnBus;
	}

	bool isMinTravelOnMrt() const {
		return minTravelOnMRT;
	}

	void setMinTravelOnMrt(bool minTravelOnMrt) {
		minTravelOnMRT = minTravelOnMrt;
	}

	bool isMinWalkingDistance() const {
		return minWalkingDistance;
	}

	void setMinWalkingDistance(bool minWalkingDistance) {
		this->minWalkingDistance = minWalkingDistance;
	}

	const std::vector<PT_NetworkEdge>& getPathEdges() const {
		return pathEdges;
	}

	void setPathEdges(const std::vector<PT_NetworkEdge>& pathEdges) {
		this->pathEdges = pathEdges;
	}

	double getPathSize() const {
		return pathSize;
	}

	void setPathSize(double pathSize) const {
		this->pathSize = pathSize;
	}

	double getPathTravelTime() const {
		return pathTravelTime;
	}

	void setPathTravelTime(double pathTravelTime) {
		this->pathTravelTime = pathTravelTime;
	}

	const std::string& getPtPathId() const {
		return ptPathId;
	}

	void setPtPathId(const std::string& ptPathId) {
		this->ptPathId = ptPathId;

	}

	const std::string& getPtPathSetId() const {
		return ptPathSetId;
	}

	void setPtPathSetId(const std::string& ptPathSetId) {
		this->ptPathSetId = ptPathSetId;
	}

	const std::string& getScenario() const {
		return scenario;
	}

	void setScenario(const std::string& scenario) {
		this->scenario = scenario;
	}

	bool isShortestPath() const {
		return shortestPath;
	}

	void setShortestPath(bool shortestPath) {
		this->shortestPath = shortestPath;
	}

	double getTotalCost() const{
		return totalCost;
	}

	void setTotalCost(double totalCost) {
		this->totalCost = totalCost;
	}

	double getTotalDistanceKms() const {
		return totalDistanceKms;
	}

	void setTotalDistanceKms(double totalDistanceKms) {
		this->totalDistanceKms = totalDistanceKms;
	}

	double getTotalInVehicleTravelTimeSecs() const {
		return totalInVehicleTravelTimeSecs;
	}

	void setTotalInVehicleTravelTimeSecs(double totalInVehicleTravelTimeSecs) {
		this->totalInVehicleTravelTimeSecs = totalInVehicleTravelTimeSecs;
	}

	int getTotalNumberOfTransfers() const {
		return totalNumberOfTransfers;
	}

	void setTotalNumberOfTransfers(int totalNumberOfTransfers) {
		this->totalNumberOfTransfers = totalNumberOfTransfers;
	}

	double getTotalWaitingTimeSecs() const {
		return totalWaitingTimeSecs;
	}

	void setTotalWaitingTimeSecs(double totalWaitingTimeSecs) {
		this->totalWaitingTimeSecs = totalWaitingTimeSecs;
	}

	double getTotalWalkingTimeSecs() const {
		return totalWalkingTimeSecs;
	}

	void setTotalWalkingTimeSecs(double totalWalkingTimeSecs) {
		this->totalWalkingTimeSecs = totalWalkingTimeSecs;
	}

	bool isValidPath() const {
		return validPath;
	}

	void setValidPath(bool validPath) {
		this->validPath = validPath;
	}

	int getPathModesType() const
	{
		return pathModesType;
	}

	/**
	 * parses comma separated list of path edge ids and builds pathEdges (vector of PT_NetworkEdge objects)
	 */
	void updatePathEdges();

private:
	std::vector<PT_NetworkEdge> pathEdges;
	std::string ptPathId;
	std::string ptPathSetId;
	std::string scenario;
	double pathTravelTime;
	double totalDistanceKms;
	mutable double pathSize;
	double totalCost;
	double totalInVehicleTravelTimeSecs;
	double totalWaitingTimeSecs;
	double totalWalkingTimeSecs;
	int totalNumberOfTransfers;
	bool minDistance;
	bool validPath;
	bool shortestPath;
	bool minInVehicleTravelTime;
	bool minNumberOfTransfers;
	bool minWalkingDistance;
	bool minTravelOnMRT;
	bool minTravelOnBus;

 	/**
	 * pathModesType is set to
	 *  0 if path involves neither bus nor MRT travel;
	 *  1 if path involves only bus travel;
	 *  2 if path involves only MRT travel;
	 *  3 if path involves both bus and MRT travel
	 */
	int pathModesType;

	double getTotalCostByDistance(double);
};

struct cmp_path_vector: public std::less<PT_Path>
{
		bool operator() (const PT_Path& lhs, const PT_Path& rhs) const;
};

class PT_PathSet
{
public:
	PT_PathSet();
	~PT_PathSet();

	std::set<PT_Path,cmp_path_vector> pathSet;
	void computeAndSetPathSize();
	void checkPathFeasibilty();
};

}//namespace
