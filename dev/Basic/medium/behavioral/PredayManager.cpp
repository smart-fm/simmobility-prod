//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayManager.cpp
 *
 *  Created on: Nov 18, 2013
 *      Author: Harish Loganathan
 */

#include "PredayManager.hpp"

#include <algorithm>
#include <boost/thread.hpp>
#include "behavioral/PredaySystem.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Config.hpp"
#include "database/PopulationSqlDao.hpp"
#include "database/PopulationMongoDao.hpp"
#include "database/ZoneCostMongoDao.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob;
using namespace sim_mob::db;
using namespace sim_mob::medium;

sim_mob::medium::PredayManager::~PredayManager() {
	Print() << "Clearing Person List" << std::endl;
	// clear Persons
	for(PersonList::iterator i = personList.begin(); i!=personList.end(); i++) {
		delete *i;
	}
	personList.clear();

	// clear Zones
	Print() << "Clearing zoneMap" << std::endl;
	for(ZoneMap::iterator i = zoneMap.begin(); i!=zoneMap.end(); i++) {
		delete i->second;
	}
	zoneMap.clear();

	// clear AMCosts
	Print() << "Clearing amCostMap" << std::endl;
	for(CostMap::iterator i = amCostMap.begin(); i!=amCostMap.end(); i++) {
		for(boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j!=i->second.end(); j++) {
			if(j->second) {
				delete j->second;
			}
		}
	}
	amCostMap.clear();

	// clear PMCosts
	Print() << "Clearing pmCostMap" << std::endl;
	for(CostMap::iterator i = pmCostMap.begin(); i!=pmCostMap.end(); i++) {
		for(boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j!=i->second.end(); j++) {
			if(j->second) {
				delete j->second;
			}
		}
	}
	pmCostMap.clear();

	// clear OPCosts
	Print() << "Clearing opCostMap" << std::endl;
	for(CostMap::iterator i = opCostMap.begin(); i!=opCostMap.end(); i++) {
		for(boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j!=i->second.end(); j++) {
			if(j->second) {
				delete j->second;
			}
		}
	}
	opCostMap.clear();
}

void sim_mob::medium::PredayManager::loadPersons(BackendType dbType) {
	switch(dbType) {
	case POSTGRES:
	{
		Database database = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_local_lt");
		std::string cred_id = ConfigManager::GetInstance().FullConfig().system.networkDatabase.credentials;
		Credential credentials = ConfigManager::GetInstance().FullConfig().constructs.credentials.at(cred_id);
		std::string username = credentials.getUsername();
		std::string password = credentials.getPassword(false);
		DB_Config dbConfig(database.host, database.port, database.dbName, username, password);

		// Connect to database and load data.
		DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
		conn.connect();
		if (conn.isConnected()) {
			PopulationSqlDao populationDao(conn);
			populationDao.getAll(personList);
		}
		break;
	}
	case MONGO_DB:
	{
		std::string populationCollectionName = ConfigManager::GetInstance().FullConfig().constructs.mongoCollectionsMap.at("preday_mongo").collectionName.at("population");
		Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
		std::string emptyString;
		db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);
		PopulationMongoDao populationDao(dbConfig, db.dbName, populationCollectionName);
		populationDao.getAll(personList);
		break;
	}
	default:
	{
		throw std::runtime_error("Unsupported backend type. Only PostgreSQL and MongoDB are currently supported.");
	}
	}
}

void sim_mob::medium::PredayManager::loadZones(db::BackendType dbType) {
	switch(dbType) {
	case POSTGRES:
	{
		throw std::runtime_error("Zone information is not available in PostgreSQL database yet");
		break;
	}
	case MONGO_DB:
	{
//		zoneMap.reserve(1092);
		std::string zoneCollectionName = ConfigManager::GetInstance().FullConfig().constructs.mongoCollectionsMap.at("preday_mongo").collectionName.at("Zone");
		Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
		std::string emptyString;
		db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);
		ZoneMongoDao zoneDao(dbConfig, db.dbName, zoneCollectionName);
		zoneDao.getAllZones(zoneMap);
		Print() << "MTZ Zones loaded" << std::endl;
		break;
	}
	default:
	{
		throw std::runtime_error("Unsupported backend type. Only PostgreSQL and MongoDB are currently supported.");
	}
	}

	for(ZoneMap::iterator i=zoneMap.begin(); i!=zoneMap.end(); i++) {
		zoneIdLookup[i->second->getZoneCode()] = i->first;
	}
}

void sim_mob::medium::PredayManager::loadCosts(db::BackendType dbType) {
	switch(dbType) {
	case POSTGRES:
	{
		throw std::runtime_error("AM, PM and off peak costs are not available in PostgreSQL database yet");
	}
	case MONGO_DB:
	{
		std::string amCostsCollName = ConfigManager::GetInstance().FullConfig().constructs.mongoCollectionsMap.at("preday_mongo").collectionName.at("AMCosts");
		std::string pmCostsCollName = ConfigManager::GetInstance().FullConfig().constructs.mongoCollectionsMap.at("preday_mongo").collectionName.at("PMCosts");
		std::string opCostsCollName = ConfigManager::GetInstance().FullConfig().constructs.mongoCollectionsMap.at("preday_mongo").collectionName.at("OPCosts");
		Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
		std::string emptyString;
		db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);

		ZoneMap::size_type nZones = zoneMap.size();
		if(nZones > 0) {
			// if the zone data was loaded already we can reserve space for costs to speed up the loading
			// Cost data will be available foe every pair (a,b) of zones where a!=b
			CostMap::size_type mapSz = nZones * nZones - nZones;
			amCostMap.rehash(mapSz / amCostMap.max_load_factor());
			pmCostMap.rehash(mapSz / pmCostMap.max_load_factor());
			opCostMap.rehash(mapSz / opCostMap.max_load_factor());
		}

		CostMongoDao amCostDao(dbConfig, db.dbName, amCostsCollName);
		amCostDao.getAll(amCostMap);
		Print() << "AM Costs Loaded" << std::endl;

		CostMongoDao pmCostDao(dbConfig, db.dbName, pmCostsCollName);
		pmCostDao.getAll(pmCostMap);
		Print() << "PM Costs Loaded" << std::endl;

		CostMongoDao opCostDao(dbConfig, db.dbName, opCostsCollName);
		opCostDao.getAll(opCostMap);
		Print() << "OP Costs Loaded" << std::endl;
	}
	}
}

void sim_mob::medium::PredayManager::distributeAndProcessPersons(unsigned numWorkers) {
	boost::thread_group threadGroup;
	if(numWorkers == 1) { // if single threaded execution was requested
		processPersons(personList.begin(), personList.end());
	}
	else {
		PersonList::size_type numPersons = personList.size();
		PersonList::size_type numPersonsPerThread = numPersons / numWorkers;
		PersonList::iterator first = personList.begin();
		PersonList::iterator last = personList.begin()+numPersonsPerThread;
		Print() << "numPersons:" << numPersons << "|numWorkers:" << numWorkers
				<< "|numPersonsPerThread:" << numPersonsPerThread << std::endl;

		/*
		 * Passing different iterators on the same list into the threaded
		 * function. So each thread will iterate mutually exclusive and
		 * exhaustive set of persons from the population.
		 *
		 * Note that each thread will iterate the same personList with different
		 * start and end iterators. It is therefore important that none of the
		 * threads change the personList.
		 */
		for(int i = 1; i<=numWorkers; i++) {
			threadGroup.create_thread( boost::bind(&PredayManager::processPersons, this, first, last) );
			first = last;
			if(i+1 == numWorkers) {
				// if the next iteration is the last take all remaining persons
				last = personList.end();
			}
			else {
				last = last + numPersonsPerThread;
			}
		}
		threadGroup.join_all();
	}

}

void sim_mob::medium::PredayManager::processPersons(
		PersonList::iterator firstPersonIt,
		PersonList::iterator oneAfterLastPersonIt)
{
	boost::unordered_map<std::string, db::MongoDao*> mongoDao;
	MongoCollectionsMap mongoColl = ConfigManager::GetInstance().FullConfig().constructs.mongoCollectionsMap.at("preday_mongo");
	Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
	std::string emptyString;
	for(std::map<std::string, std::string>::const_iterator i=mongoColl.collectionName.begin(); i!=mongoColl.collectionName.end(); i++) {
		db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);
		mongoDao[i->first]= new db::MongoDao(dbConfig, db.dbName, i->second);
	}

	// loop through all persons within the range and plan their day
	for(PersonList::iterator i = firstPersonIt; i!=oneAfterLastPersonIt; i++) {
		PredaySystem predaySystem(**i, zoneMap, zoneIdLookup, amCostMap, pmCostMap, opCostMap, mongoDao);
		predaySystem.planDay();
		predaySystem.outputPredictionsToMongo();
	}

	// destroy Dao objects
	for(boost::unordered_map<std::string, db::MongoDao*>::iterator i=mongoDao.begin(); i!=mongoDao.end(); i++) {
		safe_delete_item(i->second);
	}
	mongoDao.clear();
}
