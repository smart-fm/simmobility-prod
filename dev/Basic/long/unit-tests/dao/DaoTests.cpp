//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DaoTests.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 5:22 PM
 */

#include "DaoTests.hpp"
#include <typeinfo>
#include <string>
#include "database/dao/HouseholdDao.hpp"
#include "database/dao/BuildingDao.hpp"
#include "database/dao/UnitDao.hpp"
#include "database/dao/PostcodeDao.hpp"
#include "database/dao/PostcodeAmenitiesDao.hpp"
#include "database/dao/ParcelDao.hpp"
#include "database/dao/DeveloperDao.hpp"
#include "database/dao/TemplateDao.hpp"
#include "database/dao/LandUseZoneDao.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;
using namespace unit_tests;
using std::cout;
using std::endl;

namespace {
    const int ID_TO_GET = 1;
}

template <typename T, typename K>
void TestDao() {
    PrintOut("----------------------------- TESTING: " << typeid (T).name() << "----------------------------- " << endl);
    DB_Config config(LT_DB_CONFIG_FILE);
    config.load();
    DB_Connection conn(sim_mob::db::POSTGRES, config);
    conn.connect();
    if (conn.isConnected()) {
        T dao(conn);
        K valueById;
        //Get by id
        sim_mob::db::Parameters keys;
        keys.push_back(ID_TO_GET);
        if (dao.getById(keys, valueById)) {
            PrintOut("Get by id: " << valueById << endl);
        }

        std::vector<K> values;
        dao.getAll(values);
        PrintOut("GetAll Size: " << values.size() << endl);
        for (typename std::vector<K>::iterator it = values.begin(); it != values.end(); it++) {
            PrintOut("Value: " << (*it) << endl);
        }
    }
}

void DaoTests::testAll() {
    TestDao<HouseholdDao, Household>();
    TestDao<BuildingDao, Building>();
    TestDao<UnitDao, Unit>();
    TestDao<PostcodeDao, Postcode>();
    TestDao<PostcodeAmenitiesDao, PostcodeAmenities>();
    TestDao<DeveloperDao, Developer>();
    TestDao<ParcelDao, Parcel>();
    TestDao<TemplateDao, Template>();
    TestDao<LandUseZoneDao, LandUseZone>();
}
