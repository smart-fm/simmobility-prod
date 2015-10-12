#pragma once

#include <boost/unordered_map.hpp>
#include "Common.hpp"
#include "Path.hpp"
#include "soci/soci.h"
#include "soci/postgresql/soci-postgresql.h"

namespace sim_mob
{

class ERP_Gantry_Zone
{
public:
	ERP_Gantry_Zone() {}
	ERP_Gantry_Zone(ERP_Gantry_Zone &src):gantryNo(src.gantryNo),zoneId(src.zoneId) {}
	std::string gantryNo;
	std::string zoneId;
};

class ERP_Section
{
public:
	ERP_Section(): section_id(-1), ERP_Gantry_No(-1) {}
	ERP_Section(ERP_Section &src);
	int section_id;
	int ERP_Gantry_No;
	std::string ERP_Gantry_No_str;
};

class ERP_Surcharge
{
public:
	ERP_Surcharge() : rate(-1.0), vehicleTypeId(-1) {}
	ERP_Surcharge(ERP_Surcharge& src):gantryNo(src.gantryNo),startTime(src.startTime),endTime(src.endTime),rate(src.rate),
			vehicleTypeId(src.vehicleTypeId),vehicleTypeDesc(src.vehicleTypeDesc),day(src.day),
			startTime_DT(sim_mob::DailyTime(src.startTime)),endTime_DT(sim_mob::DailyTime(src.endTime)){}
	std::string gantryNo;
	std::string startTime;
	std::string endTime;
	sim_mob::DailyTime startTime_DT;
	sim_mob::DailyTime endTime_DT;
	double rate;
	int vehicleTypeId;
	std::string vehicleTypeDesc;
	std::string day;
};

class SegmentTravelTime
{
public:
	SegmentTravelTime();
	SegmentTravelTime(const SegmentTravelTime& src);
	/**
	 * Common Information
	 */
	unsigned long linkId;
	///	travel time in seconds
	double travelTime;
	std::string travelMode;
	int interval;
	/**
	 * Filled during data Retrieval From DB and information usage
	 */
	std::string startTime;
	std::string endTime;
	sim_mob::DailyTime startTime_DT;
	sim_mob::DailyTime endTime_DT;
};

struct SegmentTravelTimeVector
{
public:
	std::vector<sim_mob::SegmentTravelTime> vecSegTT;
};

/**
 * This class is used to store, retrieve, cache different parameters used in the pathset generation
 */
class PathSetParam {
private:
	PathSetParam();
	~PathSetParam();
	static PathSetParam *instance_;

	/**	current real time collection/retrieval interval (in milliseconds) */
	const int intervalMS;

	/** initializes parameters*/
	void initParameters();

	/**
	 * create the table used to store realtime travel time information
	 * @param dbSession the soci session object to use for table creation
	 */
	bool createTravelTimeRealtimeTable(soci::session& dbSession);

	/** Retrieve 'ERP' and 'link travel time' information */
	void populate();

	/** Retrieve 'ERP' and 'link travel time' information from Database */
	void getDataFromDB();

	/**
	 * set the database table name used to store travel time information
	 * @param value table name
	 */
	void setRTTT(const std::string& value);

public:
	static PathSetParam* getInstance();

	/**
	 * deletes the current instance
	 */
	static void resetInstance();

	/**
	 * insert an entry into singlepath table in the database
	 * @param sql the database connection
	 * @param spPool the set of paths to be stored
	 * @param pathSetTableName pathSet Table Name
	 */
	void storeSinglePath(soci::session& sql,std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool,const std::string pathSetTableName);

	/**
	 * get the average travel time of a segment within a time range
	 * @param rs input road segment
	 * @param travelMode intended mode of traversing the segment
	 * @param startTime start of the time range
	 * @param endTime end of the time range
	 * @return travel time in seconds
	 */
	double getSegRangeTT(const sim_mob::RoadSegment* rs, const std::string travelMode, const sim_mob::DailyTime& startTime, const sim_mob::DailyTime& endTime) const;

	/**
	 * gets the average 'default' travel time of a segment.
	 * it doesn't consider time of day.
	 * @param rs the input road segment
	 * @return travel time in seconds
	 */
	double getDefSegTT(const sim_mob::RoadSegment* rs) const;

	/**
	 * gets the 'default' travel time of a segment based on the given time of day.
	 * @param rs the input road segment
	 * @return travel time in seconds
	 */
	double getDefSegTT(const sim_mob::RoadSegment* rs, const sim_mob::DailyTime &startTime) const;

	/**
	 * get historical average travel time of a segment in a specific time of day
	 * from previous simulations.
	 * @param rs input road segment
	 * @param travelMode intended mode of traversing the segment
	 * @param startTime start of the time range
	 * @return travel time in seconds
	 */
	double getHistorySegTT(const sim_mob::RoadSegment* rs, const std::string &travelMode, const sim_mob::DailyTime &startTime) const;

	/**
	 * base method to get travel time of a segment in a specific time of
	 * day from different sources. This method searches for segment travel
	 * time in different sources arranged in the specified order:
	 * in-simulation, previous simulations, default.
	 * the method will returns the first value found.
	 * @param rs input road segment
	 * @param travelMode intended mode of traversing the segment
	 * @param startTime start of the time range
	 * @return travel time in seconds
	 */
	double getSegTT(const sim_mob::RoadSegment* rs, const std::string &travelMode, const sim_mob::DailyTime &startTime) const;

//	///	return cached node given its id
//	sim_mob::Node* getCachedNode(std::string id);

	double getHighwayBias() const { return highwayBias; }

	///	return the current rough size of the class todo:obsolete
	uint32_t getSize();
	///	pathset parameters
	double bTTVOT;
	double bCommonFactor;
	double bLength;
	double bHighway;
	double bCost;
	double bSigInter;
	double bLeftTurns;
	double bWork;
	double bLeisure;
	double highwayBias;
	double minTravelTimeParam;
	double minDistanceParam;
	double minSignalParam;
	double maxHighwayParam;

	///	store all multi nodes in the map
	const std::vector<sim_mob::MultiNode*>  &multiNodesPool;

	///	store all uni nodes
	const std::set<sim_mob::UniNode*> & uniNodesPool;

	///	ERP surcharge  information <gantryNo , value=ERP_Surcharge with same No diff time stamp>
	std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> > ERP_SurchargePool;

	///	ERP Zone information <gantryNo, ERP_Gantry_Zone>
	std::map<std::string,sim_mob::ERP_Gantry_Zone*> ERP_Gantry_ZonePool;

	///	ERP section <aim-sun id , ERP_Section>
	std::map<int,sim_mob::ERP_Section*> ERP_SectionPool;

	///	information of "Segment" default travel time <segment aim-sun id ,segment_default_travel_time with diff time stamp>
	boost::unordered_map<unsigned long, sim_mob::SegmentTravelTimeVector*> segDefTT;

	///	a structure to keep history of average travel time records from previous simulations
	///	[time interval][travel mode][road segment][average travel time]
	AverageTravelTime segHistoryTT;

	///	simmobility's road network
	const sim_mob::RoadNetwork& roadNetwork;

	/// Real Time Travel Time Table Name
	std::string RTTT;
	/// Default Travel Time Table Name
	std::string DTT;

};
}//namspace sim_mob
