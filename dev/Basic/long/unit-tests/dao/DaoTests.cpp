/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DaoTests.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 5:22 PM
 */

#include "DaoTests.hpp"
#include "database/dao/HouseholdDao.hpp"
#include "database/dao/BuildingDao.hpp"
#include "database/dao/UnitDao.hpp"

using namespace sim_mob::long_term;
using namespace unit_tests;
using std::cout;
using std::endl;
using sim_mob::DBConnection;

//"host=localhost port=5432 user=postgres password=5M_S1mM0bility dbname=sg"
//"host=172.25.184.13 port=5432 user=umiuser password=askme4sg dbname=sg"
//"host=localhost port=5432 user=postgres password=5M_S1mM0bility dbname=lt-db"
#define CONNECTION_STRING "host=localhost port=5432 user=postgres password=5M_S1mM0bility dbname=lt-db"
#define ID_TO_GET 1

template <typename T, typename K>
void TestDao() {
    DBConnection conn(sim_mob::POSTGRES, CONNECTION_STRING);
    conn.Connect();
    if (conn.IsConnected()) {
        T dao(&conn);
        K valueById;
        //Get by id
        sim_mob::dao::Parameters keys;
        keys.push_back(ID_TO_GET);
        if (dao.GetById(keys, valueById)) {
            LogOut("Get by id: " << valueById << endl);
        }

        std::vector<K> values;
        dao.GetAll(values);
        LogOut("GetAll Size: " << values.size() << endl);
        for (typename std::vector<K>::iterator it = values.begin(); it != values.end(); it++) {
            LogOut("Value: " << (*it) << endl);
        }
    }
}

void DaoTests::TestAll() {
   // TestDao<HouseholdDao, Household>();
   // TestDao<BuildingDao, Building>();
    TestDao<UnitDao, Unit>();
}
