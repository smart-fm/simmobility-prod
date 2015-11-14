#pragma once

#include <map>
#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>
#include "Common.hpp"
#include "Path.hpp"

namespace sim_mob
{

class ERP_Gantry_Zone
{
public:
	ERP_Gantry_Zone() {}
	std::string gantryNo;
	std::string zoneId;
};

class ERP_Section
{
public:
	ERP_Section(): sectionId(-1), ERP_Gantry_No(-1), linkId(-1) {}
	int sectionId;
	int linkId;
	int ERP_Gantry_No;
	std::string ERP_Gantry_No_str;
};

class ERP_Surcharge
{
public:
	ERP_Surcharge() : rate(-1.0), vehicleTypeId(-1) {}
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

class LinkTravelTime
{
private:
	/**	time interval */
	typedef unsigned int TimeInterval;

	/** typedef for travel time for this link for each downstream link */
	typedef std::map<unsigned int, double> LinkDownStreamLinkTT_Map;

	/**	map of [time interval][Link][downstream Link] --> [average travel time] */
	typedef std::map<TimeInterval, LinkDownStreamLinkTT_Map> TravelTimeStore;

	/** link id */
	unsigned int linkId;

	/**	travel time in seconds */
	double defaultTravelTime;

	/** map of downstream link --> tt for this link in seconds */
	TravelTimeStore downstreamLinkTT_Map;

public:
	LinkTravelTime();
	virtual ~LinkTravelTime();

	double getDefaultTravelTime() const
	{
		return defaultTravelTime;
	}

	void setDefaultTravelTime(double defaultTravelTime)
	{
		this->defaultTravelTime = defaultTravelTime;
	}

	unsigned int getLinkId() const
	{
		return linkId;
	}

	void setLinkId(unsigned int linkId)
	{
		this->linkId = linkId;
	}

	/**
	 * adds travel time of this link for specific next downstream link
	 * @param downstreamLinkId id of downstream link
	 * @param travelTime travel time in seconds for this link when next link is downstreamLinkId
	 *
	 * NOTE: this function assumes that the supplied downstream link is connected to current link. It does not check connectivity.
	 */
	void addHistoricalTravelTime(unsigned int downstreamLinkId, double travelTime);

	/**
	 * fetches tt in seconds for provided downstream link and time interval index
	 * @param downstreamLinkId id of downstream link
	 * @param timeInterval index of time interval for which tt is requested
	 * @return tt found from downstreamLinkTT_Map if available; -1 otherwise
	 */
	double getHistoricalLinkTT(unsigned int downstreamLinkId, unsigned int timeInterval) const;

	/**
	 * fetches tt in seconds for provided time interval index
	 * this function averages the traveltime for each downstream link and returns the average travel time
	 * @param timeInterval index of time interval for which tt is requested
	 * @return tt found from downstreamLinkTT_Map if available; -1 otherwise
	 */
	double getHistoricalLinkTT(unsigned int timeInterval) const;

	static TimeInterval getTimeInterval(const DailyTime& dt) const;
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

	void loadERP_Surcharge(soci::session& sql);
	void loadERP_Section(soci::session& sql);
	void loadERP_GantryZone(soci::session& sql);
	void loadLinkDefaultTravelTime(soci::session& sql);
	void loadLinkHistoricalTravelTime(soci::session& sql);

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
	 * base method to get travel time of a link in a specific time of
	 * day from different sources. This method searches for link travel
	 * time in different sources in the below order:
	 * in-simulation, previous simulations, default.
	 * the method returns the first value found.
	 * @param lnk input Link
	 * @param downstreamLink the next link which is to be taken after lnk
	 * @param startTime start of the time range
	 * @return travel time in seconds
	 */
	double getLinkTT(const sim_mob::Link* lnk, const sim_mob::Link* downstreamLink, const sim_mob::DailyTime &startTime) const;

	double getHighwayBias() const { return highwayBias; }

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
	std::vector<Node*>  multiNodesPool;

	///	store all uni nodes
	//const std::set<sim_mob::UniNode*> & uniNodesPool;

	///	ERP surcharge  information <gantryNo , value=ERP_Surcharge with same No diff time stamp>
	std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> > ERP_SurchargePool;

	///	ERP Zone information <gantryNo, ERP_Gantry_Zone>
	std::map<std::string,sim_mob::ERP_Gantry_Zone*> ERP_Gantry_ZonePool;

	///	ERP section <link id , ERP_Section>
	std::map<int,sim_mob::ERP_Section*> ERP_SectionPool;

	/**
	 * a structure to keep history of average travel time records from previous simulations
	 */
	std::map<unsigned int, sim_mob::LinkTravelTime> lnkTravelTimeMap;

	///	simmobility's road network
	const sim_mob::RoadNetwork& roadNetwork;

	/// Real Time Travel Time Table Name
	std::string RTTT;
	/// Default Travel Time Table Name
	std::string DTT;

};
}//namspace sim_mob
