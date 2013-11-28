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
using namespace mongo;

sim_mob::db::MongoDao::MongoDao(DB_Config& dbConfig, const string& database, const string& collection)
: connection(db::MONGO_DB, dbConfig)
{
	std::stringstream ss;
	ss << database << "." << collection;
	collectionName = ss.str();
}

sim_mob::db::MongoDao::~MongoDao()
{}

mongo::BSONObj& sim_mob::db::MongoDao::insert(mongo::BSONObj& bsonObj) {
	connection.getSession<mongo::DBClientConnection>().insert(collectionName, bsonObj);
	return bsonObj; // Unnecessary return just to comply with the base interface
}

bool sim_mob::db::MongoDao::update(mongo::BSONObj& bsonObj) {
	throw std::runtime_error("MongoDao::update() - Not implemented yet");
}

bool sim_mob::db::MongoDao::erase(const Parameters& params) {
	throw std::runtime_error("MongoDao::erase() - Not implemented yet");
}

bool sim_mob::db::MongoDao::getById(const Parameters& ids, mongo::BSONObj& outParam) {
	throw std::runtime_error("MongoDao::getById() - Not implemented. Use getAll() or getOne() instead");
}

bool sim_mob::db::MongoDao::getAll(std::vector<mongo::BSONObj>& outList) {
	auto_ptr<DBClientCursor> cursor = connection.getSession<mongo::DBClientConnection>().query(collectionName, Query());
	while(cursor->more()) {
		outList.push_back(cursor->next());
	}
	return true;
}

bool sim_mob::db::MongoDao::getOne(mongo::BSONObj& bsonObj, mongo::BSONObj& outBsonObj) {
	outBsonObj = connection.getSession<mongo::DBClientConnection>().findOne(collectionName, Query(bsonObj));
	return true;
}
