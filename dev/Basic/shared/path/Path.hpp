#pragma once
#include "geospatial/WayPoint.hpp"
#include <sstream>
#include <stdint.h>
#include <boost/shared_ptr.hpp>

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
size_t getLaneIndex2(const sim_mob::Lane* l);
void calculateRightTurnNumberAndSignalNumberByWaypoints(sim_mob::SinglePath *sp);
double calculateHighWayDistance(sim_mob::SinglePath *sp);
double generateSinglePathLength(const std::vector<sim_mob::WayPoint>& wp);
std::string makeWaypointsetString(std::vector<WayPoint>& wp);

class SinglePath
{
public:
	/// path representation
	std::vector<sim_mob::WayPoint> path;
	///	link representation of path
	std::vector<sim_mob::Link*> linkPath;
	///	segment collection of the path
	std::set<const sim_mob::RoadSegment*> segSet;

	bool isNeedSave2DB;
	std::string scenario;
	std::string id;   //id: seg1id_seg2id_seg3id
	std::string pathSetId;

	double travelCost;
	double travleTime;

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

	long long index;//unique serial number assigned by db

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
	///	returns the raugh size of object in Bytes
	uint32_t getSize();
	///does these SinglePath include the any of given RoadSegment(s)
	bool includesRoadSegment(const std::set<const sim_mob::RoadSegment*> &segs, bool dbg= false, std::stringstream *out = nullptr);
	static void filterOutNodes(std::vector<sim_mob::WayPoint>& input, std::vector<sim_mob::WayPoint>& output);

};

/*****************************************************
 ******************* Path Set ************************
 *****************************************************
 */
class PathSet
{
public:
	PathSet():/*fromNode(nullptr),toNode(nullptr),*/logsum(0),hasPath(false),bestPath(nullptr),oriPath(nullptr),subTrip(nullptr),isNeedSave2DB(false),id("")  {pathChoices.clear();}
	PathSet(const sim_mob::Node *fn,const sim_mob::Node *tn) : /*fromNode(fn),toNode(tn),*/logsum(0),hasPath(false),bestPath(nullptr),oriPath(nullptr),subTrip(nullptr),isNeedSave2DB(false),id("") {pathChoices.clear();}
	~PathSet();
	///	returns the rough size of object in Bytes
	uint32_t getSize();
	/**
	 * checks to see if any of the SinglePath s includes a segment from the given container
	 * returns true if there is any common segments between the two sets.
	 * Note: This can be a computationally very expensive operation, use it with caution
	 */
	bool includesRoadSegment(const std::set<const sim_mob::RoadSegment*> & segs);
	/**
	 * eliminate those SinglePaths which have a section in the given set
	 */
	void excludeRoadSegment(const std::set<const sim_mob::RoadSegment*> & segs);
	void addOrDeleteSinglePath(sim_mob::SinglePath* s);
	std::vector<WayPoint> *bestPath;  //best choice
//	const sim_mob::Node *fromNode;
//	const sim_mob::Node *toNode;
	SinglePath* oriPath;  // shortest path with all segments
	//std::map<std::string,sim_mob::SinglePath*> SinglePathPool;
	std::set<sim_mob::SinglePath*, sim_mob::SinglePath> pathChoices;
	bool isNeedSave2DB;
	double logsum;
	// pathset use info of subtrip to get start, end, travel start time...
	const sim_mob::SubTrip* subTrip;
	std::string id;
	std::string excludedPaths;
	std::string scenario;
	bool hasPath;

	PathSet(boost::shared_ptr<sim_mob::PathSet> &ps);
};
}//namespace
