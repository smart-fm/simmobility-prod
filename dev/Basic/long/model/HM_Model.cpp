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
#include "core/AgentsLookup.hpp"


using namespace sim_mob;
using namespace sim_mob::long_term;
using std::vector;
using boost::unordered_map;

using std::string;
namespace {
    const string MODEL_NAME = "Housing Market Model";
    const BigSerial FAKE_IDS_START = 9999900;
}

HM_Model::HM_Model(WorkGroup& workGroup)
: Model(MODEL_NAME, workGroup) {
}

HM_Model::~HM_Model() {
    stopImpl(); //for now
}

void HM_Model::startImpl() {
    workGroup.assignAWorker(&market);
    unsigned int numberOfFakeSellers = workGroup.getNumberOfWorkers();
    //create fake seller agents to sell vacant units.
    std::vector<HouseholdAgent*> fakeSellers;
    for (int i = 0; i < numberOfFakeSellers; i++) {
        HouseholdAgent* fakeSeller = new HouseholdAgent((FAKE_IDS_START + i), 
                this, nullptr, &market, true);
        AgentsLookupSingleton::getInstance().addHousehold(fakeSeller);
        agents.push_back(fakeSeller);
        workGroup.assignAWorker(fakeSeller);
        fakeSellers.push_back(fakeSeller);
    }

    DataManager& dman = DataManagerSingleton::getInstance();
    const DataManager::HouseholdList& households = dman.getHouseholds();
    const DataManager::UnitList& units = dman.getUnits();
    boost::unordered_map<BigSerial, BigSerial> assignedUnits;

    // Assign households to the units.
    for (DataManager::HouseholdList::const_iterator it = households.begin();
            it != households.end(); it++) {
        const Household* household = &(*it);
        HouseholdAgent* hhAgent = new HouseholdAgent(household->getId(), this,
                household, &market);
        const Unit* unit = dman.getUnitById(household->getUnitId());
        if (unit) {
            hhAgent->addUnitId(unit->getId());
            assignedUnits.insert(std::make_pair(unit->getId(), unit->getId()));
        }
        AgentsLookupSingleton::getInstance().addHousehold(hhAgent);
        agents.push_back(hhAgent);
        workGroup.assignAWorker(hhAgent);
    }

    unsigned int vacancies = 0;
    //assign vacancies to fake seller
    for (DataManager::UnitList::const_iterator it = units.begin();
            it != units.end(); it++) {
        //this unit is a vacancy
        if (assignedUnits.find(it->getId()) == assignedUnits.end()) {
            fakeSellers[vacancies % numberOfFakeSellers]->addUnitId(it->getId());
            vacancies++;
        }
    }
    addMetadata("Initial Units", units.size());
    addMetadata("Initial Households", households.size());
    addMetadata("Initial Vacancies", vacancies);
    addMetadata("Fake Sellers", numberOfFakeSellers);
}

void HM_Model::stopImpl() {
}

