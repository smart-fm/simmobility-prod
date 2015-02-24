#pragma once
#include "geospatial/WayPoint.hpp"
#include "util/DailyTime.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"

namespace sim_mob
{
class RoadSegment;

struct RdSegTravelTimes
{
public:
	double travelTimeSum;
	unsigned int agCnt;

	RdSegTravelTimes(double rdSegTravelTime, unsigned int agentCount)
	: travelTimeSum(rdSegTravelTime), agCnt(agentCount) {}
};
namespace TT
{
	struct TimeAndCount
	{
		///	total travel time
		double totalTravelTime;

		///	number of travel times
		int travelTimeCnt;

		TimeAndCount():totalTravelTime(0.0),travelTimeCnt(0){}
	};

	///	time interval
	typedef unsigned int TI;

	/*******************************************************************************
	 * ************* In Simulation Travel Time Collection **************************
	 *******************************************************************************/
	///	the heart of the final container holding accumulated travel time
	typedef std::map<const sim_mob::RoadSegment*,TimeAndCount > RSTC;

	/// part of the data structure used in travel time collection mechanism
	typedef std::map<std::string , RSTC >  MRTC;//MTITC=> M:mode, TI:Time Interval, T:time, C:count

	///	final container for collecting in simulation data:
	///	map[time interval][travel mode][road segment][pair(total-time , number-of-records)]
	typedef std::map<TI,MRTC> TravelTimeCollector;

	/*******************************************************************************
	 * ********** Average Travel Time Collection from previous Simulations *********
	 *******************************************************************************/

	///	part of the structure used for retrieving the stored travel time (average) from database
	typedef std::map<std::string , std::map<const sim_mob::RoadSegment*,double > >  MST;//MST:M:mode, S:segment, T:time-average

	///	map[time interval][travel mode][road segment][average travel time]
	typedef std::map<TI,MST> TravelTimeStore;
}

///	typedef of TT namespace structures
/**
 * Container used for TravelTime collection during simulation     :
 * [road segment][travel mode][time interval][average travel time]
 * <-----RS-----><-------------------MTITC----------------------->
 */
typedef sim_mob::TT::TravelTimeCollector TravelTime;

/**
 * Container used to retrieve Previous travel time data stored in external source(database)
 * [time interval][travel mode][road segment][average travel time]
 * <-----TI-----> <-------------------MST------------------------>
 */
typedef sim_mob::TT::TravelTimeStore AverageTravelTime;
}


/**
 * A structure to store Origin and Destination in one pair
 * additional operator overload for assignment and comparisons
 */
class OD
{
private:
	std::string key;
public:
	OD(){}
	OD(const sim_mob::WayPoint &origin, const sim_mob::WayPoint &destination):
		origin(origin), destination(destination)
	{
		std::stringstream str("");
		str << origin.node_->getID() << "," << destination.node_->getID();
		key = str.str();
	}
	OD(const sim_mob::Node * origin, const sim_mob::Node * destination):
		origin(sim_mob::WayPoint(origin)), destination(sim_mob::WayPoint(destination))
	{
		std::stringstream str("");
		str << origin->getID() << "," << destination->getID();
		key = str.str();
	}
	sim_mob::WayPoint origin;
	sim_mob::WayPoint destination;
	bool operator==(const OD & rhs) const
	{
		return (origin == rhs.origin && destination == rhs.destination);
	}

	bool operator!=(const OD & rhs) const
	{
		return !(*this == rhs);
	}

	OD & operator=(const OD & rhs)
	{
		origin = rhs.origin;
		destination = rhs.destination;
		return *this;
	}
	bool operator<(const OD & rhs) const
	{
		// just an almost dummy operator< to preserve uniquness
		return key < rhs.key;
	}
};
