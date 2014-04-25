/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   PotentialProject.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 31, 2014, 2:51 PM
 */

#include "PotentialProject.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

PotentialUnit::PotentialUnit(BigSerial unitTypeId, double floorArea, bool freehold)
: unitTypeId(unitTypeId), floorArea(floorArea), freehold(freehold) {

}

PotentialUnit::~PotentialUnit() {
}

BigSerial PotentialUnit::getUnitTypeId() const {
    return unitTypeId;
}

double PotentialUnit::getFloorArea() const {
    return floorArea;
}

bool PotentialUnit::isFreehold() const {
    return freehold;
}

PotentialProject::PotentialProject(const DevelopmentTypeTemplate* devTemplate,
        const Parcel* parcel, const LandUseZone* zone)
: devTemplate(devTemplate), parcel(parcel), zone(zone), cost(0), revenue(0) {
}

PotentialProject::~PotentialProject() {
}

void PotentialProject::addUnit(const PotentialUnit& unit) {
    units.push_back(unit);
}

const DevelopmentTypeTemplate* PotentialProject::getDevTemplate() const {
    return devTemplate;
}

const Parcel* PotentialProject::getParcel() const {
    return parcel;
}

const LandUseZone* PotentialProject::getZone() const {
    return zone;
}

const std::vector<PotentialUnit>& PotentialProject::getUnits() const {
    return units;
}

double PotentialProject::getCost() const {
    return cost;
}

double PotentialProject::getRevenue() const {
    return revenue;
}

void PotentialProject::setCost(const double cost) {
    this->cost = cost;
}

void PotentialProject::setRevenue(const double revenue) {
    this->revenue = revenue;
}


namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const PotentialUnit& data) {
            return strm << "{"
                    << "\"unitTypeId\":\"" << data.getUnitTypeId() << "\","
                    << "\"floorArea\":\"" << data.getFloorArea() << "\","
                    << "\"freehold\":\"" << data.isFreehold() << "\""
                    << "}";
        }

        std::ostream& operator<<(std::ostream& strm, const PotentialProject& data) {
            std::stringstream unitsStr;
            unitsStr << "[";
            std::vector<PotentialUnit>::const_iterator itr;
            const std::vector<PotentialUnit>& units = data.getUnits();
            for (itr = units.begin(); itr != units.end(); itr++) {
                unitsStr << (*itr) << (((itr + 1) != units.end()) ? "," : "");
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