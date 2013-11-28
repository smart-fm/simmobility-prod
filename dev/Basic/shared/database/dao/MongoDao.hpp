//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MongoDao.hpp
 *
 *  Created on: Nov 20, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include "database/dao/I_Dao.h"
#include "database/DB_Connection.hpp"
#include "mongo/client/dbclient.h"

namespace sim_mob {
namespace db {

/**
 * Data access object for MongoDB - a NoSQL database
 */
class MongoDao : public I_Dao<mongo::BSONObj> {
public:
	MongoDao(DB_Config& dbConfig, const std::string& database, const std::string& collection);
	virtual ~MongoDao();

	/**
	 * inserts a document into collection
	 *
	 * @param bsonObj a mongo::BSONObj containing object to insert
	 * @return true if the transaction was committed with success; false otherwise.
	 */
	mongo::BSONObj& insert(mongo::BSONObj& bsonObj);

    /**
     * Updates the given entity into the data source.
     * @param entity to update.
     * @return true if the transaction was committed with success,
     *         false otherwise.
     */
    bool update(mongo::BSONObj& bsonObj);

    /**
     * Deletes all objects filtered by given params.
     * @param params to filter.
     * @return true if the transaction was committed with success,
     *         false otherwise.
     */
    bool erase(const Parameters& params);

    /**
     * Gets a single value filtered by given ids.
     * @param ids to filter.
     * @param outParam to put the value
     * @return true if a value was returned, false otherwise.
     */
    bool getById(const Parameters& ids, mongo::BSONObj& outParam);

    /**
     * Gets all values from the source and put them on the given list.
     * @param outList to put the retrieved values.
     * @return true if some values were returned, false otherwise.
     */
    bool getAll(std::vector<mongo::BSONObj>& outList);

	/**
	 * Overload. Fetches a cursor to the result of the query
	 *
	 * @param bsonObj a mongo::BSONObj object containing the constructed query
	 */
	bool getOne(mongo::BSONObj& bsonObj, mongo::BSONObj& outBsonObj);

	DB_Connection connection;
	std::string collectionName;
};

}//end namespace db
}//end namespace sim_mob
