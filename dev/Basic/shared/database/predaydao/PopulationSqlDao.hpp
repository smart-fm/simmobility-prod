//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <bitset>
#include <map>
#include <vector>
#include "database/dao/SqlAbstractDao.hpp"
#include "database/DB_Connection.hpp"
#include "behavioral/params/PersonParams.hpp"

namespace sim_mob
{
/**
 * Data access object for Population tables
 *
 * \author Harish Loganathan
 */
class PopulationSqlDao: public db::SqlAbstractDao<PersonParams>
{
public:
	PopulationSqlDao(db::DB_Connection& connection);
	virtual ~PopulationSqlDao();

	/**
	 * fetches all individual ids from LT population
	 * @param outList output list of ids
	 */
	void getAllIds(std::vector<long>& outList);

	/**
	 * fetches data for individual id
	 * @param id individual id
	 * @param outParam output parameter to load individual data
	 */
	void getOneById(long long id, PersonParams& outParam);

	/**
	 * fetches the lookup table for income categories
	 * @param outArray output parameter for storing income lower limits
	 */
	void getIncomeCategories(double outArray[]);

	/**
	 * fetches lookup table for vehicle categories
	 * @param outMap output parameter for storing ehicle category ids and values
	 */
	void getVehicleCategories(std::map<int, std::bitset<4> >& outMap);

	/**
	 * fetches taz code for each address id in LT database
	 * @param addressMap output parameter for storing address_id -> TAZ code map
	 * @param zoneAddressesMap output parameter for storing list of addresses in each TAZ
	 */
	void getAddresses(std::map<long, sim_mob::Address>& addressMap, std::map<int, std::vector<long> >& zoneAddressesMap);

private:
	/**
	 * Virtual override.
	 * Fills the given outObj with all values contained on Row.
	 * @param result row with data to fill the out object.
	 * @param outObj to fill.
	 */
	void fromRow(db::Row& result, PersonParams& outObj);

	/**
	 * Virtual override.
	 * Fills the outParam with all values to insert or update on datasource.
	 * @param data to get values.
	 * @param outParams to put the data parameters.
	 * @param update tells if operation is an Update or Insert.
	 */
	void toRow(PersonParams& data, db::Parameters& outParams, bool update);
};

/**
 * Data access object for Logsum table and other dataset
 *
 * \author Harish Loganathan
 */
class SimmobSqlDao: public db::SqlAbstractDao<PersonParams>
{
public:
	SimmobSqlDao(db::DB_Connection& connection, const std::string& tableName);
	virtual ~SimmobSqlDao();

	/**
	 * fetches logsum data for individual id
	 * @param id individual id
	 * @param outParam output parameter to load logsums
	 */
	void getLogsumById(long long id, PersonParams& outObj);

	/**
	 * fetches taz code for each address id in simmobility database
	 * @param outMap output parameter for storing postcode -> simmobility node map
	 */
	void getPostcodeNodeMap(std::map<unsigned int, unsigned int>& outMap);

private:
	/**
	 * Virtual override.
	 * Fills the given outObj with all values contained on Row.
	 * @param result row with data to fill the out object.
	 * @param outObj to fill with logsums.
	 */
	void fromRow(db::Row& result, PersonParams& outObj);

	/**
	 * Virtual override.
	 * Fills the outParam with all values to insert or update on datasource.
	 * @param data to get values.
	 * @param outParams to put the data parameters.
	 * @param update tells if operation is an Update or Insert.
	 */
	void toRow(PersonParams& data, db::Parameters& outParams, bool update);
};
} // end namespace sim_mib
