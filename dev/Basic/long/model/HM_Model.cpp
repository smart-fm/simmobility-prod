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


using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
using std::vector;
using boost::unordered_map;

namespace {

    void deleteAgents(vector<Agent*>& agents) {
        while (!agents.empty()) {
            safe_delete_item(agents[0]);
        }
        agents.clear();
    }
}

HM_Model::HM_Model(DatabaseConfig& dbConfig, WorkGroup& workGroup)
: workGroup(workGroup), dbConfig(dbConfig) {
}

HM_Model::~HM_Model() {
    deleteAgents(agents);
}

void HM_Model::Start() {
    if (!started) {
        DatabaseConfig dbConfig(LT_DB_CONFIG_FILE);
        // Connect to database and load data.
        DBConnection conn(sim_mob::db::POSTGRES, dbConfig);
        conn.Connect();
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
                }
                agents.push_back(hhAgent);
                workGroup.assignAWorker(hhAgent);
            }
            unitsById.clear();
        }
        started = true;
    }
}

void HM_Model::Stop() {
    if (started) {
        started = false;
        deleteAgents(agents);
        households.clear();
        units.clear();
    }
}

