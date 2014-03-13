/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DataManager.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Feb 11, 2014, 1:32 PM
 */

#include "DataManager.hpp"
#include "util/HelperFunctions.hpp"
#include "database/DB_Connection.hpp"
#include "database/dao/HouseholdDao.hpp"
#include "database/dao/UnitDao.hpp"
#include "database/dao/BuildingDao.hpp"
#include "database/dao/PostcodeDao.hpp"
#include "database/dao/PostcodeAmenitiesDao.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
namespace {

    template <typename T, typename M, typename K>
    inline const T* getById(const M& map, const K& key) {
        typename M::const_iterator itr = map.find(key);
        if (itr != map.end()) {
            return (*itr).second;
        }
        return nullptr;
    }
}

DataManager::DataManager() : readyToLoad(true) {
}

DataManager::~DataManager() {
    reset();
}

void DataManager::reset() {
    amenitiesById.clear();
    amenitiesByCode.clear();
    postcodesById.clear();
    postcodesByCode.clear();
    buildingsById.clear();
    unitsById.clear();
    householdsById.clear();
    clear_delete_vector(units);
    clear_delete_vector(buildings);
    clear_delete_vector(households);
    clear_delete_vector(amenities);
    clear_delete_vector(postcodes);
    readyToLoad = true;
}

void DataManager::load() {
    //first resets old data if necessary.
    if (!readyToLoad) {
        reset();
    }

    DB_Config dbConfig(LT_DB_CONFIG_FILE);
    dbConfig.load();
    // Connect to database and load data.
    DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
    conn.connect();
    if (conn.isConnected()) {
        loadData<HouseholdDao>(conn, households, householdsById, &Household::getId);
        loadData<UnitDao>(conn, units, unitsById, &Unit::getId);
        loadData<BuildingDao>(conn, buildings, buildingsById, &Building::getId);
        loadData<PostcodeDao>(conn, postcodes, postcodesById, &Postcode::getId);
        loadData<PostcodeAmenitiesDao>(conn, amenities, amenitiesByCode, 
                &PostcodeAmenities::getPostcode);

        // (Special case) Index all postcodes.
        for (PostcodeList::iterator it = postcodes.begin();
                it != postcodes.end(); it++) {
            postcodesByCode.insert(std::make_pair((*it)->getCode(), *it));
        }

        //Index all amenities. 
        for (PostcodeAmenitiesList::iterator it = amenities.begin();
                it != amenities.end(); it++) {
            const Postcode* pc = getPostcodeByCode((*it)->getPostcode());
            if (pc) {
                amenitiesById.insert(std::make_pair(pc->getId(), *it));
            }
        }
    }
    readyToLoad = false;
}

const Building* DataManager::getBuildingById(const BigSerial buildingId) const {
    return getById<Building>(buildingsById, buildingId);
}

const Postcode* DataManager::getPostcodeById(const BigSerial postcodeId) const {
    return getById<Postcode>(postcodesById, postcodeId);
}

const PostcodeAmenities* DataManager::getAmenitiesById(const BigSerial postcodeId) const {
    return getById<PostcodeAmenities>(amenitiesById, postcodeId);
}

const Postcode* DataManager::getPostcodeByCode(const std::string& code) const {
    return getById<Postcode>(postcodesByCode, code);
}

const PostcodeAmenities* DataManager::getAmenitiesByCode(const std::string& code) const {
    return getById<PostcodeAmenities>(amenitiesByCode, code);
}

const Unit* DataManager::getUnitById(const BigSerial unitId) const {
    return getById<Unit>(unitsById, unitId);
}

const Household* DataManager::getHouseholdById(const BigSerial householdId) const {
    return getById<Household>(householdsById, householdId);
}

const BigSerial DataManager::getUnitTazId(const BigSerial unitId) const {
    const Unit* unit = getUnitById(unitId);
    if (unit) {
        const Postcode* pc = getPostcodeById(unit->getPostcodeId());
        if (pc) {
            return pc->getTazId();
        }
    }
    return INVALID_ID;
}

const DataManager::HouseholdList& DataManager::getHouseholds() const {
    return households;
}

const DataManager::UnitList& DataManager::getUnits() const {
    return units;
}