/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DeveloperModel.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2014, 3:08 PM
 */

#include "DeveloperModel.hpp"
#include "util/LangHelpers.hpp"
#include "util/HelperFunctions.hpp"
#include "agent/impl/DeveloperAgent.hpp"
#include "core/AgentsLookup.hpp"
#include "database/DB_Connection.hpp"
#include "database/dao/DeveloperDao.hpp"
#include "database/dao/ParcelDao.hpp"
#include "database/dao/TemplateDao.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;

using std::string;
namespace {
    const string MODEL_NAME = "Developer Model";
}

DeveloperModel::DeveloperModel(WorkGroup& workGroup)
: Model(MODEL_NAME, workGroup) {
}

DeveloperModel::~DeveloperModel() {
}

void DeveloperModel::startImpl() {
    DB_Config dbConfig(LT_DB_CONFIG_FILE);
    dbConfig.load();

    // Connect to database and load data for this model.
    DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
    conn.connect();
    if (conn.isConnected()) {
        //Load developers
        loadData<DeveloperDao>(conn, developers);
        //Load templates
        loadData<TemplateDao>(conn, templates);
        //Load parcels
        loadData<ParcelDao>(conn, parcels);
    }

    for (DeveloperList::iterator it = developers.begin(); it != developers.end();
            it++) {
        DeveloperAgent* devAgent = new DeveloperAgent(*it);
        AgentsLookupSingleton::getInstance().addDeveloper(devAgent);
        agents.push_back(devAgent);
        workGroup.assignAWorker(devAgent);
    }

    addMetadata("Initial Developers", developers.size());
    addMetadata("Initial Templates", templates.size());
    addMetadata("Initial Parcels", parcels.size());
}

void DeveloperModel::stopImpl() {
    clear_delete_vector(developers);
    clear_delete_vector(templates);
    clear_delete_vector(parcels);
}