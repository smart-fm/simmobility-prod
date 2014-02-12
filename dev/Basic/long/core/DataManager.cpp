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
    amenitiesById.clear();
    amenitiesByCode.clear();
    postcodesById.clear();
    postcodesByCode.clear();
    postcodes.clear();
    amenities.clear();
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

        //Index all postcodes.
        for (PostcodeList::iterator it = postcodes.begin(); 
                it != postcodes.end(); it++) {
            postcodesById.insert(std::make_pair(it->getId(), &(*it)));
            postcodesByCode.insert(std::make_pair(it->getCode(), &(*it)));
        }
        
        //Index all amenities. 
        for (PostcodeAmenitiesList::iterator it = amenities.begin();
                it != amenities.end(); it++) {
            amenitiesByCode.insert(std::make_pair(it->getPostcode(), &(*it)));
            const Postcode* pc = getPostcodeByCode(it->getPostcode());
            if (pc) {
                amenitiesById.insert(std::make_pair(pc->getId(), &(*it)));
            }
        }
    }
}

const Postcode* DataManager::getPostcodeById(const BigSerial postcodeId) const {
    PostcodeMap::const_iterator itr = postcodesById.find(postcodeId);
    if (itr != postcodesById.end()) {
        return (*itr).second;
    }
    return nullptr;
}

const PostcodeAmenities* DataManager::getAmenitiesById(const BigSerial postcodeId) const {
    PostcodeAmenitiesMap::const_iterator itr = amenitiesById.find(postcodeId);
    if (itr != amenitiesById.end()) {
        return (*itr).second;
    }
    return nullptr;
}

const Postcode* DataManager::getPostcodeByCode(const std::string& code) const {
    PostcodeByCodeMap::const_iterator itr = postcodesByCode.find(code);
    if (itr != postcodesByCode.end()) {
        return (*itr).second;
    }
    return nullptr;
}

const PostcodeAmenities* DataManager::getAmenitiesByCode(const std::string& code) const {
    PostcodeAmenitiesByCodeMap::const_iterator itr = amenitiesByCode.find(code);
    if (itr != amenitiesByCode.end()) {
        return (*itr).second;
    }
    return nullptr;
}