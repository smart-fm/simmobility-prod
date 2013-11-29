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

#include "behavioral/PredaySystem.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Config.hpp"
#include "database/PopulationSqlDao.hpp"
#include "database/PopulationMongoDao.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob;
using namespace sim_mob::db;
using namespace sim_mob::medium;

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

void sim_mob::medium::PredayManager::distributeAndProcessPersons(uint16_t numWorkers) {
	processPersons(personList);
}

void sim_mob::medium::PredayManager::processPersons(PersonList& persons) {
	for(PersonList::iterator i = persons.begin(); i!=persons.end(); i++) {
		PredaySystem predaySystem(*i);
		predaySystem.planDay();
	}
}
