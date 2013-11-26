/*
 * MongoDao.cpp
 *
 *  Created on: Nov 20, 2013
 *      Author: Harish Loganathan
 */

#include "MongoDao.hpp"

using namespace std;
using namespace sim_mob;
using namespace db;

sim_mob::db::MongoDao::MongoDao(DB_Config& dbConfig, const string& database, const string& collection)
: connection(db::MONGO_DB, dbConfig)
{
	std::stringstream ss;
	ss << database << "." << collection;
	collectionName = ss.str();
}

sim_mob::db::MongoDao::~MongoDao()
{}

void sim_mob::db::MongoDao::insertDocument(const mongo::BSONObj& bsonObj) {
	connection.getMongoConnection().insert(collectionName, bsonObj);
}

std::auto_ptr<mongo::DBClientCursor> sim_mob::db::MongoDao::queryDocument(const mongo::BSONObj& bsonObj) {
	mongo::Query queryObj(bsonObj);
	return connection.getMongoConnection().query(collectionName, queryObj);
}
