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

HM_Model::HM_Model(WorkGroup& workGroup)
: Model(MODEL_NAME, workGroup) {
}

HM_Model::~HM_Model() {
    stopImpl(); //for now
}

const Unit* HM_Model::getUnitById(const BigSerial& unitId) const{
    if (unitsById.find(unitId) != unitsById.end()){
        return unitsById.at(unitId);
    }
    return nullptr;
}

void HM_Model::startImpl() {
    DB_Config dbConfig(LT_DB_CONFIG_FILE);
    dbConfig.load();
    // Connect to database and load data.
    DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
    conn.connect();
    workGroup.assignAWorker(&market);
    if (conn.isConnected()) {
        // Households
        HouseholdDao hhDao(&conn);
        hhDao.getAll(households);
        //units
        UnitDao unitDao(&conn);
        unitDao.getAll(units);
        for (vector<Unit>::iterator it = units.begin(); it != units.end(); it++) {
            Unit* unit = &(*it);
            unitsById.insert(std::make_pair(unit->getId(), unit));
        }

        for (vector<Household>::iterator it = households.begin(); it != households.end(); it++) {
            Household* household = &(*it);
            //PrintOut("Household: " << (*household) << endl);
            sim_mob::db::Parameters keys;
            keys.push_back(household->getId());
            HouseholdAgent* hhAgent = new HouseholdAgent(this, household, &market);
            UnitMap::iterator mapItr = unitsById.find(household->getUnitId());
            if (mapItr != unitsById.end()) { //Context Id does exists
                Unit* unit = (mapItr->second);
                hhAgent->addUnitId(unit->getId());
                PrintOut("Household ["<< household->getId()<<"] holds the Unit ["<< unit->getId()<<"]" << std::endl);
            }
            agents.push_back(hhAgent);
            workGroup.assignAWorker(hhAgent);
        }
    }
}

void HM_Model::stopImpl() {
    households.clear();
    units.clear();
    unitsById.clear();
}

