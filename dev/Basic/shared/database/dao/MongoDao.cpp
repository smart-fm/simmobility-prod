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

sim_mob::db::MongoDao::MongoDao(db::DB_Connection* connection, string& database, string& collection) : connection(connection)
{
	std::stringstream ss;
	ss << database << "." << collection;
	collectionName = ss.str();
}

sim_mob::db::MongoDao::~MongoDao()
{}

void sim_mob::db::MongoDao::insertDocument(const KeyValuePairs& doc) {
	mongo::BSONObj bsonObj;
	constructBSON_Obj(doc, bsonObj);
	connection.getMongoConnection().insert(collectionName, bsonObj);
}

std::auto_ptr<mongo::DBClientCursor> sim_mob::db::MongoDao::queryDocument(const KeyValuePairs& doc) {
	mongo::BSONObj bsonObj;
	constructBSON_Obj(doc, bsonObj);
	mongo::Query queryObj(bsonObj);
	return connection.getMongoConnection().query(collectionName, queryObj);
}

std::auto_ptr<mongo::DBClientCursor> sim_mob::db::MongoDao::queryDocument(const mongo::BSONObj& bsonObj) {
	mongo::Query queryObj(bsonObj);
	return connection.getMongoConnection().query(collectionName, queryObj);
}

void sim_mob::db::MongoDao::constructBSON_Obj(const KeyValuePairs& doc, mongo::BSONObj& bsonObj) {
	mongo::BSONObjBuilder bsonObjBldr;
	for(KeyValuePairs::const_iterator i=doc.begin(); i!=doc.end(); i++) {
		bsonObjBldr << i->first << i->second;
	}
	bsonObj = bsonObjBldr.obj();
}
