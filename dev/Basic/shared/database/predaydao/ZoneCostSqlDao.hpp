//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/unordered_map.hpp>
#include "database/dao/SqlAbstractDao.hpp"
#include "database/DB_Connection.hpp"
#include "behavioral/params/ZoneCostParams.hpp"

namespace sim_mob
{
/**
 * Data access object for skim matrix tables
 *
 * \author Harish Loganathan
 */
class CostSqlDao : public db::SqlAbstractDao<CostParams>
{
public:
	typedef boost::unordered_map<int, boost::unordered_map<int, CostParams*> > CostMap;
	CostSqlDao(db::DB_Connection& connection, const std::string& getAllQuery);
	virtual ~CostSqlDao();

	/**
	 * getAll overload tailored for preday specific structure
	 * @param outMap 2D map of CostParams indexed by origin and destination zone
	 * @return true if outMap has been populated with at least 1 element; false otherwise
	 */
	bool getAll(boost::unordered_map<int, boost::unordered_map<int, CostParams*> >& outMap);

private:
    /**
     * Virtual override.
     * Fills the given outObj with all values contained on Row.
     * @param result row with data to fill the out object.
     * @param outObj to fill.
     */
    void fromRow(db::Row& result, CostParams& outObj);

    /**
     * Virtual override.
     * Fills the outParam with all values to insert or update on datasource.
     * @param data to get values.
     * @param outParams to put the data parameters.
     * @param update tells if operation is an Update or Insert.
     */
    void toRow(CostParams& data, db::Parameters& outParams, bool update);
};

/**
 * Data access object for zone table
 *
 * \author Harish Loganathan
 */
class ZoneSqlDao : public db::SqlAbstractDao<ZoneParams>
{
public:

	ZoneSqlDao(db::DB_Connection& connection);
	virtual ~ZoneSqlDao();

private:
    /**
     * Virtual override.
     * Fills the given outObj with all values contained on Row.
     * @param result row with data to fill the out object.
     * @param outObj to fill.
     */
    void fromRow(db::Row& result, ZoneParams& outObj);

    /**
     * Virtual override.
     * Fills the outParam with all values to insert or update on datasource.
     * @param data to get values.
     * @param outParams to put the data parameters.
     * @param update tells if operation is an Update or Insert.
     */
    void toRow(ZoneParams& data, db::Parameters& outParams, bool update);
};

/**
 * Data access object for zone to node mapping table
 *
 * \author Harish Loganathan
 */
class ZoneNodeSqlDao : public db::SqlAbstractDao<ZoneNodeParams>
{
public:

	ZoneNodeSqlDao(db::DB_Connection& connection);
	virtual ~ZoneNodeSqlDao();

	/**
	 * fetches TAZ code to node map from database
	 * @param outList output parameter for [TAZ code -> list of simmobility nodes] map
	 */
	void getZoneNodeMap(boost::unordered_map<int, std::vector<ZoneNodeParams*> >& outList);

private:
    /**
     * Virtual override.
     * Fills the given outObj with all values contained on Row.
     * @param result row with data to fill the out object.
     * @param outObj to fill.
     */
    void fromRow(db::Row& result, ZoneNodeParams& outObj);

    /**
     * Virtual override.
     * Fills the outParam with all values to insert or update on datasource.
     * @param data to get values.
     * @param outParams to put the data parameters.
     * @param update tells if operation is an Update or Insert.
     */
    void toRow(ZoneNodeParams& data, db::Parameters& outParams, bool update);

};
} // end namespace sim_mob
