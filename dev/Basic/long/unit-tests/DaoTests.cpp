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

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace unit_tests;

//"host=localhost port=5432 user=postgres password=5M_S1mM0bility dbname=sg"
//"host=172.25.184.13 port=5432 user=umiuser password=askme4sg dbname=sg"
#define CONNECTION_STRING "host=172.25.184.13 port=5432 user=umiuser password=askme4sg dbname=sg"
    
void DaoTests::TestIndividualDao() {
    DBConnection conn (POSTGRES, CONNECTION_STRING);
    conn.Connect();
    if (conn.IsConnected()) {
        IndividualDao dao(&conn);
        Individual ind1;
        //Get by id
        dao::Parameters keys;
        keys.push_back(98);
        if (dao.GetById(keys, ind1)) {
            LogOut("Individual by id: " << ind1 << endl);
        }

        vector<Individual> inds;
        dao.GetAll(inds);
        LogOut("Individuals Number: " << inds.size() << endl);
        for (vector<Individual>::iterator it = inds.begin(); it != inds.end(); it++) {
            LogOut("Individual: " << (*it) << endl);
        }
    }
}

void DaoTests::TestHouseholdDao() {
    DBConnection conn (POSTGRES, CONNECTION_STRING);
    conn.Connect();
    if (conn.IsConnected()) {
        HouseholdDao dao(&conn);
        Household hh;
        //Get by id
        dao::Parameters keys;
        keys.push_back(98);
        if (dao.GetById(keys, hh)) {
            LogOut("Household by id: " << hh << endl);
        }

        vector<Household> hhs;
        dao.GetAll(hhs);
        LogOut("Households Number: " << hhs.size() << endl);
        for (vector<Household>::iterator it = hhs.begin(); it != hhs.end(); it++) {
            LogOut("Household: " << (*it) << endl);
        }
    }
}