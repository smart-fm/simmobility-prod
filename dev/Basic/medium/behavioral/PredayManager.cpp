//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayManager.cpp
 *
 *  Created on: Nov 18, 2013
 *      Author: harish
 */

#include "PredayManager.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "database/DB_Config.hpp"
#include "database/DB_Connection.hpp"
#include "database/PredayDao.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob;
using namespace sim_mob::db;
using namespace sim_mob::medium;

void sim_mob::medium::PredayManager::loadPersons() {
	DB_Config dbConfig(ConfigManager::GetInstance().FullConfig().constructs.databases.at(""))
}

void sim_mob::medium::PredayManager::distributeAndProcessPersons() {
}

void sim_mob::medium::PredayManager::processPersons(PersonList& persons) {
}
