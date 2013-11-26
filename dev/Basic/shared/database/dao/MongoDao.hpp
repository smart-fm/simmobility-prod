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
#include "database/dao/AbstractDao.hpp"
#include "database/DB_Connection.hpp"
#include "mongo/client/dbclient.h"

namespace sim_mob {
namespace db {

/**
 * Data access object for MongoDB
 */
class MongoDao {
public:
	MongoDao(DB_Config& dbConfig, const std::string& database, const std::string& collection);
	virtual ~MongoDao();

	/**
	 * inserts a document into collection
	 *
	 * @param bsonObj a mongo::BSONObj containing object to insert
	 */
	void insertDocument(const mongo::BSONObj& bsonObj);

	/**
	 * Overload. Fetches a cursor to the result of the query
	 *
	 * @param bsonObj a mongo::BSONObj object containing the constructed query
	 */
	std::auto_ptr<mongo::DBClientCursor> queryDocument(const mongo::BSONObj& bsonObj);

	DB_Connection connection;
	std::string collectionName;
};

}//end namespace db
}//end namespace sim_mob
