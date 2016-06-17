#pragma once

#include <map>
#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>
#include "Common.hpp"
#include "entities/TravelTimeManager.hpp"
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
	ERP_Section(): sectionId(0), ERP_Gantry_No(0), linkId(0) {}
	unsigned int sectionId;
	unsigned int linkId;
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

	/** Retrieve 'ERP' and 'link travel time' information */
	void populate();

	/** Retrieve 'ERP' and 'link travel time' information from Database */
	void getDataFromDB();

	/**
	 * loads ERP surcharge data from db
	 * @param sql db session object
	 */
	void loadERP_Surcharge(soci::session& sql);

	/**
	 * loads ERP sections data from db
	 * @param sql db session object
	 */
	void loadERP_Section(soci::session& sql);

	/**
	 * loads ERP gantry data from db
	 * @param sql db session object
	 */
	void loadERP_GantryZone(soci::session& sql);

public:
	static PathSetParam* getInstance();

	/**
	 * deletes the current instance
	 */
	static void resetInstance();

	/**
	 * insert an entry into singlepath table in the database
	 * @param spPool the set of paths to be stored
	 */
	void storeSinglePath(std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool);

	double getHighwayBias() const
	{
		return highwayBias;
	}

	///	pathset parameters
	double highwayBias;

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
	 * local pointer to singleton travel time manager instance
	 */
	TravelTimeManager* ttMgr;

	///	simmobility's road network
	const sim_mob::RoadNetwork& roadNetwork;
};
}//namspace sim_mob
