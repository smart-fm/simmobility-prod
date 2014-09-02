//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DevelopmentTypeTemplate.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 25, 2014, 5:54 PM
 */

#include "DevelopmentTypeTemplate.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

DevelopmentTypeTemplate::DevelopmentTypeTemplate(
        BigSerial developmentTypeId,
        BigSerial templateId,
        double density)
: developmentTypeId(developmentTypeId), templateId(templateId),
density(density) {
}

DevelopmentTypeTemplate::~DevelopmentTypeTemplate() {
}

BigSerial DevelopmentTypeTemplate::getDevelopmentTypeId() const {
    return developmentTypeId;
}

BigSerial DevelopmentTypeTemplate::getTemplateId() const {
    return templateId;
}

double DevelopmentTypeTemplate::getDensity() const {
    return density;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const DevelopmentTypeTemplate& data) {
            return strm << "{"
                    << "\"developmentTypeId\":\"" << data.developmentTypeId << "\","
                    << "\"templateId\":\"" << data.templateId << "\","
                    << "\"density\":\"" << data.density << "\""
                    << "}";
        }
    }
}