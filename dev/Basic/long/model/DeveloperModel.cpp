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
    const unsigned int TIME_INTERVAL = 30; //In days (7 - weekly, 30 - Montly)
}

DeveloperModel::PotentialUnit::PotentialUnit(BigSerial unitTypeId, double floorArea, bool freehold)
: unitTypeId(unitTypeId), floorArea(floorArea), freehold(freehold) {

}

DeveloperModel::PotentialUnit::~PotentialUnit() {
}

const BigSerial DeveloperModel::PotentialUnit::getUnitTypeId() const {
    return unitTypeId;
}

double DeveloperModel::PotentialUnit::getFloorArea() const {
    return floorArea;
}

bool DeveloperModel::PotentialUnit::isFreehold() const {
    return freehold;
}

DeveloperModel::PotentialProject::PotentialProject(const DevelopmentTypeTemplate* devTemplate,
        const Parcel* parcel, const LandUseZone* zone)
: devTemplate(devTemplate), parcel(parcel), zone(zone), cost(0), revenue(0) {
}

DeveloperModel::PotentialProject::~PotentialProject() {
}

void DeveloperModel::PotentialProject::addUnit(const DeveloperModel::PotentialUnit& unit){
    units.push_back(unit);
}

const DevelopmentTypeTemplate* DeveloperModel::PotentialProject::getDevTemplate() const {
    return devTemplate;
}

const Parcel* DeveloperModel::PotentialProject::getParcel() const {
    return parcel;
}

const LandUseZone* DeveloperModel::PotentialProject::getZone() const {
    return zone;
}

const std::vector<DeveloperModel::PotentialUnit>& DeveloperModel::PotentialProject::getUnits() const {
    return units;
}

double DeveloperModel::PotentialProject::getCost() const {
    return cost;
}

double DeveloperModel::PotentialProject::getRevenue() const {
    return revenue;
}

void DeveloperModel::PotentialProject::setCost(const double cost) {
    this->cost = cost;
}

void DeveloperModel::PotentialProject::setRevenue(const double revenue) {
    this->revenue = revenue;
}

DeveloperModel::DeveloperModel(WorkGroup& workGroup)
: Model(MODEL_NAME, workGroup), timeInterval(TIME_INTERVAL) {
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

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const DeveloperModel::PotentialUnit& data) {
            return strm << "{"
                    << "\"unitTypeId\":\"" << data.getUnitTypeId() << "\","
                    << "\"floorArea\":\"" << data.getFloorArea() << "\","
                    << "\"freehold\":\"" << data.isFreehold() << "\""
                    << "}";
        }
        
        std::ostream& operator<<(std::ostream& strm, const DeveloperModel::PotentialProject& data) {
            std::stringstream unitsStr;
            unitsStr << "[";
            std::vector<DeveloperModel::PotentialUnit>::const_iterator itr;
            const std::vector<DeveloperModel::PotentialUnit>& units = data.getUnits();
            for(itr=units.begin(); itr != units.end(); itr++){
                unitsStr << (*itr) << (((itr+1) != units.end()) ? "," : "");
            }
            unitsStr << "]";
            return strm << "{"
                    << "\"templateId\":\"" << data.getDevTemplate()->getTemplateId() << "\","
                    << "\"parcelId\":\"" << data.getParcel()->getId() << "\","
                    << "\"zoneId\":\"" << data.getZone()->getId() << "\","
                    << "\"gpr\":\"" << data.getZone()->getGPR() << "\","
                    << "\"density\":\"" << data.getDevTemplate()->getDensity() << "\","
                    << "\"cost\":\"" << data.getCost() << "\","
                    << "\"revenue\":\"" << data.getRevenue() << "\","
                    << "\"units\":\"" << unitsStr.str() << "\""
                    << "}";
        }
    }
}