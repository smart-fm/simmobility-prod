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
#include "database/dao/BuildingDao.hpp"
#include "database/dao/PostcodeDao.hpp"
#include "database/dao/PostcodeAmenitiesDao.hpp"
#include "conf/ConfigManager.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
namespace {

    template <typename T, typename M, typename K>
    inline const T* getById(const M& map, const K& key)
    {
        typename M::const_iterator itr = map.find(key);
        if (itr != map.end())
        {
            return (*itr).second;
        }
        return nullptr;
    }
}

DataManager::DataManager() : readyToLoad(true) {}

DataManager::~DataManager()
{
    reset();
}

void DataManager::reset()
{
    amenitiesById.clear();
    amenitiesByCode.clear();
    postcodesById.clear();
    postcodesByCode.clear();
    buildingsById.clear();
    clear_delete_vector(buildings);
    clear_delete_vector(amenities);
    clear_delete_vector(postcodes);
    readyToLoad = true;
}


void DataManager::load()
{
    //first resets old data if necessary.
    if (!readyToLoad)
    {
        reset();
    }

    DB_Config dbConfig(LT_DB_CONFIG_FILE);
    dbConfig.load();
    // Connect to database and load data.
    DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
    conn.connect();
    if (conn.isConnected())
    {
        ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

        conn.setSchema(config.schemas.main_schema);
        loadData<BuildingDao>(conn, buildings, buildingsById, &Building::getFmBuildingId);
        PrintOutV("Loaded " << buildings.size() << " buildings." << std::endl);
        loadData<PostcodeDao>(conn, postcodes, postcodesById, &Postcode::getAddressId);
        PrintOutV("Loaded " << postcodes.size() << " postcodes." << std::endl );
        loadData<PostcodeAmenitiesDao>(conn, amenities, amenitiesById, &PostcodeAmenities::getAddressId);
        PrintOutV("Loaded " << amenities.size() << " amenities." << std::endl);

        // (Special case) Index all postcodes.
        for (PostcodeList::iterator it = postcodes.begin(); it != postcodes.end(); it++)
        {
            postcodesByCode.insert(std::make_pair((*it)->getSlaPostcode(), *it));
        }

        PrintOutV("amenitiesById pairs " << amenitiesById.size() << std::endl);
    }
    readyToLoad = false;
}

const Building* DataManager::getBuildingById(const BigSerial buildingId) const
{
    return getById<Building>(buildingsById, buildingId);
}

const Postcode* DataManager::getPostcodeById(const BigSerial postcodeId) const
{
    return getById<Postcode>(postcodesById, postcodeId);
}

const PostcodeAmenities* DataManager::getAmenitiesById(const BigSerial postcodeId) const
{
    return getById<PostcodeAmenities>(amenitiesById, postcodeId);
}

const Postcode* DataManager::getPostcodeByCode(const std::string& code) const
{
    return getById<Postcode>(postcodesByCode, code);
}

const PostcodeAmenities* DataManager::getAmenitiesByCode(const std::string& code) const
{
    return getById<PostcodeAmenities>(amenitiesByCode, code);
}

BigSerial DataManager::getPostcodeTazId(const BigSerial postcodeId) const
{
    const Postcode* pc = getPostcodeById(postcodeId);
    if (pc)
    {
        return pc->getTazId();
    }
    return INVALID_ID;
}
