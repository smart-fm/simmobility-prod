//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   TemplateUnitType.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 25, 2014, 5:54 PM
 */

#include "TemplateUnitType.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

TemplateUnitType::TemplateUnitType(
        BigSerial templateId,
        BigSerial unitTypeId,
        int proportion)
: templateId(templateId), unitTypeId(unitTypeId), proportion(proportion) {
}

TemplateUnitType::~TemplateUnitType() {
}

BigSerial TemplateUnitType::getTemplateId() const {
    return templateId;
}

BigSerial TemplateUnitType::getUnitTypeId() const {
    return unitTypeId;
}

int TemplateUnitType::getProportion() const {
    return proportion;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const TemplateUnitType& data) {
            return strm << "{"
                    << "\"templateId\":\"" << data.templateId << "\","
                    << "\"unitTypeId\":\"" << data.unitTypeId << "\","
                    << "\" proportion\":\"" << data. proportion << "\""
                    << "}";
        }
    }
}