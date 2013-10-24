/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HM_Model.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on October 21, 2013, 3:08 PM
 */

#include "HM_Model.hpp"
#include <boost/unordered_map.hpp>
#include "util/LangHelpers.hpp"
#include "database/DBConnection.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "database/dao/UnitDao.hpp"
#include "database/dao/HouseholdDao.hpp"
#include "event/SystemEvents.hpp"


using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
using std::vector;
using boost::unordered_map;

using std::string;
namespace {
    const string MODEL_NAME = "Housing Market Model";
}

HM_Model::HM_Model(DatabaseConfig& dbConfig, WorkGroup& workGroup)
: Model(MODEL_NAME, dbConfig, workGroup) {
}

HM_Model::~HM_Model() {
}

void HM_Model::startImpl() {
    DatabaseConfig dbConfig(LT_DB_CONFIG_FILE);
    // Connect to database and load data.
    DBConnection conn(sim_mob::db::POSTGRES, dbConfig);
    conn.Connect();
    workGroup.assignAWorker(&market);
    if (conn.IsConnected()) {
        // Households
        HouseholdDao hhDao(&conn);
        hhDao.GetAll(households);
        //units
        UnitDao unitDao(&conn);
        unitDao.GetAll(units);
        unordered_map<BigSerial, Unit*> unitsById;
        for (vector<Unit>::iterator it = units.begin(); it != units.end(); it++) {
            Unit* unit = &(*it);
            unitsById.insert(std::make_pair(unit->GetId(), unit));
        }

        for (vector<Household>::iterator it = households.begin(); it != households.end(); it++) {
            Household* household = &(*it);
            //PrintOut("Household: " << (*household) << endl);
            sim_mob::db::Parameters keys;
            keys.push_back(household->GetId());
            HouseholdAgent* hhAgent = new HouseholdAgent(household->GetId(), household, &market);
            unordered_map<BigSerial, Unit*>::iterator mapItr = unitsById.find(household->GetUnitId());
            if (mapItr != unitsById.end()) { //Context Id does exists
                Unit* unit = new Unit(*(mapItr->second));
                unit->SetAvailable(true);
                hhAgent->AddUnit(unit);
                PrintOut("Household ["<< household->GetId()<<"] holds the Unit ["<< unit->GetId()<<"]" << std::endl);
            }
            agents.push_back(hhAgent);
            workGroup.assignAWorker(hhAgent);
        }
        unitsById.clear();
    }
}

void HM_Model::stopImpl() {
    households.clear();
    units.clear();
}

