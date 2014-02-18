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
#include "agent/impl/HouseholdAgent.hpp"
#include "event/SystemEvents.hpp"
#include "core/DataManager.hpp"


using namespace sim_mob;
using namespace sim_mob::long_term;
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

void HM_Model::startImpl() {
    workGroup.assignAWorker(&market);
    DataManager& dman = DataManagerSingleton::getInstance();
    const DataManager::HouseholdList& households = dman.getHouseholds();
    // Assign households to the units.
    for (DataManager::HouseholdList::const_iterator it = households.begin();
            it != households.end(); it++) {
        const Household* household = &(*it);
        HouseholdAgent* hhAgent = new HouseholdAgent(this, household, &market);
        const Unit* unit = dman.getUnitById(household->getUnitId());
        if (unit) {
            hhAgent->addUnitId(unit->getId());
        }
        agents.push_back(hhAgent);
        workGroup.assignAWorker(hhAgent);
    }
}

void HM_Model::stopImpl() {
}

