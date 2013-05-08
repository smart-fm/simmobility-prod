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

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace unit_tests;

//"host=localhost port=5432 user=postgres password=5M_S1mM0bility dbname=sg"
//"host=172.25.184.13 port=5432 user=umiuser password=askme4sg dbname=sg"
#define CONNECTION_STRING "host=172.25.184.13 port=5432 user=umiuser password=askme4sg dbname=sg"
#define ID_TO_GET 1
void DaoTests::TestIndividualDao() {
    DBConnection conn(POSTGRES, CONNECTION_STRING);
    conn.Connect();
    if (conn.IsConnected()) {
        IndividualDao dao(&conn);
        Individual valueById;
        //Get by id
        dao::Parameters keys;
        keys.push_back(ID_TO_GET);
        if (dao.GetById(keys, valueById)) {
            LogOut("Individual by id: " << valueById << endl);
        }

        vector<Individual> values;
        dao.GetAll(values);
        LogOut("Individuals Number: " << values.size() << endl);
        for (vector<Individual>::iterator it = values.begin(); it != values.end(); it++) {
            LogOut("Individual: " << (*it) << endl);
        }
    }
}

void DaoTests::TestHouseholdDao() {
    DBConnection conn(POSTGRES, CONNECTION_STRING);
    conn.Connect();
    if (conn.IsConnected()) {
        HouseholdDao dao(&conn);
        Household valueById;
        //Get by id
        dao::Parameters keys;
        keys.push_back(ID_TO_GET);
        if (dao.GetById(keys, valueById)) {
            LogOut("Household by id: " << valueById << endl);
        }

        vector<Household> values;
        dao.GetAll(values);
        LogOut("Households Number: " << values.size() << endl);
        for (vector<Household>::iterator it = values.begin(); it != values.end(); it++) {
            LogOut("Household: " << (*it) << endl);
        }
    }
}

void DaoTests::TestBuildingTypeDao() {
    DBConnection conn(POSTGRES, CONNECTION_STRING);
    conn.Connect();
    if (conn.IsConnected()) {
        BuildingTypeDao dao(&conn);
        BuildingType valueById;
        //Get by id
        dao::Parameters keys;
        keys.push_back(ID_TO_GET);
        if (dao.GetById(keys, valueById)) {
            LogOut("BuildingType by id: " << valueById << endl);
        }

        vector<BuildingType> values;
        dao.GetAll(values);
        LogOut("BuildingTypes Number: " << values.size() << endl);
        for (vector<BuildingType>::iterator it = values.begin(); it != values.end(); it++) {
            LogOut("BuildingType: " << (*it) << endl);
        }
    }
}

void DaoTests::TestBuildingDao() {
    DBConnection conn(POSTGRES, CONNECTION_STRING);
    conn.Connect();
    if (conn.IsConnected()) {
        BuildingDao dao(&conn);
        Building valueById;
        //Get by id
        dao::Parameters keys;
        keys.push_back(ID_TO_GET);
        if (dao.GetById(keys, valueById)) {
            LogOut("Building by id: " << valueById << endl);
        }

        vector<Building> values;
        dao.GetAll(values);
        LogOut("Building Number: " << values.size() << endl);
        for (vector<Building>::iterator it = values.begin(); it != values.end(); it++) {
            LogOut("Building: " << (*it) << endl);
        }
    }
}