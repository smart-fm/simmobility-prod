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

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Config.hpp"
#include "database/DB_Connection.hpp"
#include "database/PopulationDao.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob;
using namespace sim_mob::db;
using namespace sim_mob::medium;

void sim_mob::medium::PredayManager::loadPersons() {
	Database database = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_local_lt");
	DB_Config dbConfig(database.host, database.port, database.dbName);

    // Connect to database and load data.
    DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
    conn.connect();
    if (conn.isConnected()) {
    	PopulationDao populationDao(conn);
    	populationDao.getAll(personList);
    }
}

void sim_mob::medium::PredayManager::distributeAndProcessPersons(uint16_t numWorkers) {
//	typedef unsigned long ulong;
//	ulong numPersons = personList.size();
//	ulong numPersonsPerThread = std::ceil(numPersons * 1.0 /numWorkers); // multiplying by 1.0 just to have the result in double
//	for(uint16_t i=0; i<numWorkers; i++) {
//
//	}
	processPersons(personList);
}

void sim_mob::medium::PredayManager::processPersons(PersonList& persons) {
}
