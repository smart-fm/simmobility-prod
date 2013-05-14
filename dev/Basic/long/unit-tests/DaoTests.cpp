/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DaoTests.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 5:22 PM
 */

#include "DaoTests.hpp"
#include "database/dao/IndividualDao.hpp"
#include "database/dao/HouseholdDao.hpp"
#include "database/dao/BuildingTypeDao.hpp"
#include "database/dao/BuildingDao.hpp"
#include "database/dao/LandUseTypeDao.hpp"
#include "database/dao/GenericLandUseTypeDao.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace unit_tests;

//"host=localhost port=5432 user=postgres password=5M_S1mM0bility dbname=sg"
//"host=172.25.184.13 port=5432 user=umiuser password=askme4sg dbname=sg"
#define CONNECTION_STRING "host=172.25.184.13 port=5432 user=umiuser password=askme4sg dbname=sg"
#define ID_TO_GET 1

template <typename T, typename K>
void TestDao() {
    DBConnection conn(POSTGRES, CONNECTION_STRING);
    conn.Connect();
    if (conn.IsConnected()) {
        T dao(&conn);
        K valueById;
        //Get by id
        dao::Parameters keys;
        keys.push_back(ID_TO_GET);
        if (dao.GetById(keys, valueById)) {
            LogOut("Get by id: " << valueById << endl);
        }

        vector<K> values;
        dao.GetAll(values);
        LogOut("GetAll Size: " << values.size() << endl);
        for (typename vector<K>::iterator it = values.begin(); it != values.end(); it++) {
            LogOut("Value: " << (*it) << endl);
        }
    }
}

void DaoTests::TestAll() {
    TestDao<IndividualDao, Individual>();
    TestDao<HouseholdDao, Household>();
    TestDao<BuildingTypeDao, BuildingType>();
    TestDao<BuildingDao, Building>();
    TestDao<LandUseTypeDao, LandUseType>();
    TestDao<GenericLandUseTypeDao, GenericLandUseType>();
}
