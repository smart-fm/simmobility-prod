//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>
#include "behavioral/params/PersonParams.hpp"
#include "database/dao/MongoDao.hpp"
#include "database/DB_Config.hpp"

namespace sim_mob
{
namespace medium
{
/**
 * mongodb DAO for population data
 *
 * \author Harish Loganathan
 */
class PopulationMongoDao: public db::MongoDao
{
public:
	PopulationMongoDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection);
	virtual ~PopulationMongoDao();

	/**
	 * Gets all values from the source and put them on the given list.
	 * @param outList to put the retrieved values.
	 * @return true if some values were returned, false otherwise.
	 */
	bool getAll(std::vector<PersonParams*>& outList);

	/**
	 * Gets all ids from the source and put them on to the given list.
	 * @param outList to put the retrieved ids.
	 * @return true if some ids were returned, false otherwise.
	 */
	bool getAllIds(std::vector<std::string>& outList);

	/**
	 * Converts a given row into a PersonParams type.
	 * @param result result row.
	 * @param outParam (Out parameter) to receive data from row.
	 */
	void fromRow(mongo::BSONObj document, PersonParams& outParam);

	/**
	 * returns the _id field of the document
	 * @param document the input document
	 * @return id of the given document
	 *
	 * \note this function assumes the _id field is a string (not the default object)
	 */
	std::string getIdFromRow(mongo::BSONObj document);

	/**
	 * fetches data for one person by his id
	 * @param id the input person id
	 * @param outParam (Out parameter) to receive data from document
	 * @return true if fetch was successful; false otherwise
	 */
	bool getOneById(const std::string& id, PersonParams& outParam);
};
}
}
