/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DataManager.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Feb 11, 2014, 1:32 PM
 */

#include "DataManager.hpp"
#include "database/DB_Connection.hpp"
#include "database/dao/PostcodeDao.hpp"
#include "database/dao/PostcodeAmenitiesDao.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
DataManager::DataManager() {
}

DataManager::~DataManager() {
}

void DataManager::load() {
    //load postcodes

    DB_Config dbConfig(LT_DB_CONFIG_FILE);
    dbConfig.load();
    // Connect to database and load data.
    DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
    conn.connect();
    if (conn.isConnected()) {
        // Households
        PostcodeDao postcodeDao(conn);
        postcodeDao.getAll(postcodes);
        PostcodeAmenitiesDao amenitiesDao(conn);
        amenitiesDao.getAll(amenities);

        for (PostcodeList::iterator it = postcodes.begin(); it != postcodes.end(); it++) {
            PrintOut("Postcode [" << it->getId() << "]" << std::endl);
        }
    }
}
