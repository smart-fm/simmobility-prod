//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   LandUseZone.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 21, 2014, 5:54 PM
 */

#include "LandUseZone.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

LandUseZone::LandUseZone(BigSerial id, BigSerial typeId, double gpr)
: id(id), typeId(typeId), gpr(gpr) {
}

LandUseZone::~LandUseZone() {
}

BigSerial LandUseZone::getId() const {
    return id;
}

BigSerial LandUseZone::getTypeId() const {
    return typeId;
}

double LandUseZone::getGPR() const {
    return gpr;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const LandUseZone& data) {
            return strm << "{"
                    << "\"id\":\"" << data.id << "\","
                    << "\"typeId\":\"" << data.typeId << "\","
                    << "\"gpr\":\"" << data.gpr << "\""
                    << "}";
        }
    }
}