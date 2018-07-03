//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DaoTests.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * \author : Gishara Premarathne <gishara@smart.mit.edu>
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
#include "database/dao/DevelopmentTypeTemplateDao.hpp"
#include "database/dao/TemplateUnitTypeDao.hpp"
#include "database/dao/ProjectDao.hpp"
#include "database/dao/ParcelMatchDao.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;
using namespace unit_tests;
using std::cout;
using std::endl;

CPPUNIT_TEST_SUITE_REGISTRATION(unit_tests::DaoTests);

namespace {
const int ID_TO_GET = 1;

template <typename T, typename K>
void TestDaoGetAll(unsigned int ids = 1)
{
    PrintOutV("----------------------------- TESTING: " << typeid (T).name() << "----------------------------- " << endl);
    DB_Config config(LT_DB_CONFIG_FILE);
    config.load();
    DB_Connection conn(sim_mob::db::POSTGRES, config);
    conn.connect();
    if (conn.isConnected()) {
        T dao(conn);
        K valueById;
        std::vector<K> values;
        dao.getAll(values);
        CPPUNIT_ASSERT_MESSAGE("No values loaded", !values.empty());
    }
}

template <typename T, typename K>
void TestGetById(unsigned int ids = 1)
{
    PrintOutV("----------------------------- TESTING: " << typeid (T).name() << "----------------------------- " << endl);
    DB_Config config(LT_DB_CONFIG_FILE);
    config.load();
    DB_Connection conn(sim_mob::db::POSTGRES, config);
    conn.connect();
    if (conn.isConnected()) {
        T dao(conn);
        K valueById;
        //Get by id
        sim_mob::db::Parameters keys;
        for (unsigned int i = 0; i < ids; i++){
            keys.push_back(ID_TO_GET);
        }

        bool loadVals = dao.getById(keys, valueById);
        CPPUNIT_ASSERT_MESSAGE("No values loaded", loadVals);
        if(loadVals)
        {
        	PrintOutV("Get by id: " << valueById << endl);
        }

    }
}

} //End anonymous name space

void DaoTests::testGetAll()
{
	TestDaoGetAll<HouseholdDao, Household>();
	TestDaoGetAll<BuildingDao, Building>();
	TestDaoGetAll<UnitDao, Unit>();
	TestDaoGetAll<PostcodeDao, Postcode>();
	TestDaoGetAll<PostcodeAmenitiesDao, PostcodeAmenities>();
	TestDaoGetAll<DeveloperDao, Developer>();
	TestDaoGetAll<ParcelDao, Parcel>();
	TestDaoGetAll<TemplateDao, Template>();
	TestDaoGetAll<LandUseZoneDao, LandUseZone>();
	TestDaoGetAll<DevelopmentTypeTemplateDao, DevelopmentTypeTemplate>(2);
	TestDaoGetAll<TemplateUnitTypeDao, TemplateUnitType>(2);
	TestDaoGetAll<ProjectDao, Project>();
	TestDaoGetAll<ParcelMatchDao, ParcelMatch>();
}

void DaoTests::testGetById()
{
	TestGetById<HouseholdDao, Household>();
	TestDaoGetAll<BuildingDao, Building>();
	TestDaoGetAll<UnitDao, Unit>();
	TestDaoGetAll<PostcodeDao, Postcode>();
	TestDaoGetAll<PostcodeAmenitiesDao, PostcodeAmenities>();
	TestDaoGetAll<DeveloperDao, Developer>();
	TestDaoGetAll<ParcelDao, Parcel>();
	TestDaoGetAll<TemplateDao, Template>();
	TestDaoGetAll<LandUseZoneDao, LandUseZone>();
	TestDaoGetAll<DevelopmentTypeTemplateDao, DevelopmentTypeTemplate>(2);
	TestDaoGetAll<TemplateUnitTypeDao, TemplateUnitType>(2);
}
