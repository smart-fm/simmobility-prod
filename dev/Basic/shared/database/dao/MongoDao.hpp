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

#include <cstdlib>
#include "mongo/client/dbclient.h"

#include "database/dao/AbstractDao.hpp"
#include "database/DB_Connection.hpp"

namespace sim_mob {
namespace db {
typedef boost::variant<int, std::string, long, double> ValueType;
typedef std::map<std::string, ValueType> KeyValuePairs;

/**
 * Data access object for MongoDB
 */
class MongoDao {
public:
	MongoDao(db::DB_Connection* connection, std::string& database, std::string& collection);
	virtual ~MongoDao();

	/**
	 * inserts a document into collection
	 *
	 * @param doc a map of key value pairs to construct the document to be inserted
	 */
	void insertDocument(const KeyValuePairs& doc);

	/**
	 * fetches a cursor to the result of the query
	 *
	 * @param doc a map of key value pairs to construct the document to be inserted
	 */
	std::auto_ptr<mongo::DBClientCursor> queryDocument(const KeyValuePairs& doc);

	/**
	 * Overload. Fetches a cursor to the result of the query
	 *
	 * @param doc a map of key value pairs to construct the document to be inserted
	 */
	std::auto_ptr<mongo::DBClientCursor> queryDocument(const mongo::BSONObj& bsonObj);

	DB_Connection& connection;
	std::string collectionName;

private:
	void constructBSON_Obj(const KeyValuePairs& doc, mongo::BSONObj& bsonObj);

};

}//end namespace db
}//end namespace sim_mob
