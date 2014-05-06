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
#include "database/dao/LandUseZoneDao.hpp"
#include "database/dao/DevelopmentTypeTemplateDao.hpp"
#include "database/dao/TemplateUnitTypeDao.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
using std::runtime_error;

using std::string;
namespace {
    const string MODEL_NAME = "Developer Model";
}

DeveloperModel::DeveloperModel(WorkGroup& workGroup)
: Model(MODEL_NAME, workGroup), timeInterval( 30 ){ //In days (7 - weekly, 30 - Montly)
}

DeveloperModel::DeveloperModel(WorkGroup& workGroup, unsigned int timeIntervalDevModel )
: Model(MODEL_NAME, workGroup), timeInterval( timeIntervalDevModel ){
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
        loadData<ParcelDao>(conn, parcels, parcelsById, &Parcel::getId);
        //load land use zones
        loadData<LandUseZoneDao>(conn, zones, zonesById, &LandUseZone::getId);
        //load DevelopmentType-Templates
        loadData<DevelopmentTypeTemplateDao>(conn, developmentTypeTemplates);
        //load Template - UnitType
        loadData<TemplateUnitTypeDao>(conn, templateUnitTypes);
    }

    for (DeveloperList::iterator it = developers.begin(); it != developers.end();
            it++) {
        DeveloperAgent* devAgent = new DeveloperAgent(*it, this);
        AgentsLookupSingleton::getInstance().addDeveloper(devAgent);
        agents.push_back(devAgent);
        workGroup.assignAWorker(devAgent);
    }

    //Assign parcels to developers.
    unsigned int index = 0;
    for (ParcelList::iterator it = parcels.begin(); it != parcels.end();
            it++) {
        DeveloperAgent* devAgent = dynamic_cast<DeveloperAgent*> (agents[index % agents.size()]);
        if (devAgent) {
            devAgent->assignParcel((*it)->getId());
        } else {
            throw runtime_error("Developer Model: Must be a developer agent.");
        }
        index++;
    }

    addMetadata("Time Interval", timeInterval);
    addMetadata("Initial Developers", developers.size());
    addMetadata("Initial Templates", templates.size());
    addMetadata("Initial Parcels", parcels.size());
    addMetadata("Initial Zones", zones.size());
    addMetadata("Initial DevelopmentTypeTemplates", developmentTypeTemplates.size());
    addMetadata("Initial TemplateUnitTypes", templateUnitTypes.size());
}

void DeveloperModel::stopImpl() {
    parcelsById.clear();
    clear_delete_vector(developers);
    clear_delete_vector(templates);
    clear_delete_vector(parcels);
    clear_delete_vector(zones);
    clear_delete_vector(developmentTypeTemplates);
    clear_delete_vector(templateUnitTypes);
}

unsigned int DeveloperModel::getTimeInterval() const {
    return timeInterval;
}

const Parcel* DeveloperModel::getParcelById(BigSerial id) const {
    ParcelMap::const_iterator itr = parcelsById.find(id);
    if (itr != parcelsById.end()) {
        return itr->second;
    }
    return nullptr;
}

const LandUseZone* DeveloperModel::getZoneById(BigSerial id) const {
    LandUseZoneMap::const_iterator itr = zonesById.find(id);
    if (itr != zonesById.end()) {
        return itr->second;
    }
    return nullptr;
}

const DeveloperModel::DevelopmentTypeTemplateList& DeveloperModel::getDevelopmentTypeTemplates() const {
    return developmentTypeTemplates;
}

const DeveloperModel::TemplateUnitTypeList& DeveloperModel::getTemplateUnitType() const {
    return templateUnitTypes;
}
