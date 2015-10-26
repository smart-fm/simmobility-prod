//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <boost/unordered_map.hpp>
#include <map>
#include <string>
#include <vector>
#include "behavioral/params/ZoneCostParams.hpp"
#include "database/dao/MongoDao.hpp"
#include "database/DB_Config.hpp"
#include "logging/Log.hpp"

namespace sim_mob
{
namespace medium
{

/**
 * mongodb dao for Zone collection
 *
 * \author Harish Loganathan
 */
class ZoneMongoDao: db::MongoDao
{
public:
	ZoneMongoDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection);
	virtual ~ZoneMongoDao();

	/**
	 * Gets all values from the source and put them on the given list.
	 * @param outList to put the retrieved values.
	 * @return true if some values were returned, false otherwise.
	 */
	bool getAllZones(boost::unordered_map<int, ZoneParams*>& outList);

	/**
	 * Converts a given row into a PersonParams type.
	 * @param result result row.
	 * @param outParam (Out parameter) to receive data from row.
	 */
	void fromRow(mongo::BSONObj document, ZoneParams& outParam);
};

/**
 * mongodb dao for AM, PM and OP cost collections
 *
 * \author Harish Loganathan
 */
class CostMongoDao: db::MongoDao
{
public:
	CostMongoDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection);
	virtual ~CostMongoDao();

	/**
	 * Gets all values from the source and put them on the given list.
	 * @param outList to put the retrieved values.
	 * @return true if some values were returned, false otherwise.
	 */
	bool getAll(boost::unordered_map<int, boost::unordered_map<int, CostParams*> >& outList);

	/**
	 * Converts a given row into a PersonParams type.
	 * @param result result row.
	 * @param outParam (Out parameter) to receive data from row.
	 */
	void fromRow(mongo::BSONObj document, CostParams& outParam);
};

/**
 * mongodb dao for Zone to node mapping collection
 *
 * \author Harish Loganathan
 */
class ZoneNodeMappingDao: db::MongoDao
{
public:
	ZoneNodeMappingDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection);
	virtual ~ZoneNodeMappingDao();

	/**
	 * Gets all values from the source and put them on the given list.
	 * @param outList to put the retrieved values.
	 * @return true if some values were returned, false otherwise.
	 */
	bool getAll(boost::unordered_map<int, std::vector<ZoneNodeParams*> >& outList);

	/**
	 * Converts a given row into a ZoneNodeParams type.
	 * @param result result row.
	 * @param outParam (Out parameter) to receive data from row.
	 */
	void fromRow(mongo::BSONObj document, ZoneNodeParams& outParam);
};

/**
 * mongodb dao for MTZ 2008 to MTZ 2012 mapping collection
 *
 * \author Harish Loganathan
 */
class MTZ12_MTZ08_MappingDao: db::MongoDao
{
public:
	MTZ12_MTZ08_MappingDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection);
	virtual ~MTZ12_MTZ08_MappingDao();

	/**
	 * Gets all values from the source and put them on the given list.
	 * @param outList to put the retrieved values.
	 * @return true if some values were returned, false otherwise.
	 */
	bool getAll(std::map<int, int>& outList);
};

class MTZ12_MTZ08_MappingDao : db::MongoDao {
public:
	MTZ12_MTZ08_MappingDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection);
	virtual ~MTZ12_MTZ08_MappingDao();

    /**
     * Gets all values from the source and put them on the given list.
     * @param outList to put the retrieved values.
     * @return true if some values were returned, false otherwise.
     */
    bool getAll(std::map<int,int>& outList);
};
} // end namespace medium
} // end namespace sim_mob
