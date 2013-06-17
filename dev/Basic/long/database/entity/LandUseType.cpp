/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LandUseType.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 13, 2013, 6:11 PM
 */

#include "LandUseType.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

LandUseType::LandUseType() {
}

LandUseType::~LandUseType() {
}

int LandUseType::GetId() const {
    return id;
}

int LandUseType::GetGenericTypeId() const {
    return genericTypeId;
}

string LandUseType::GetUnitName() const {
    return unitName;
}

string LandUseType::GetDescription() const {
    return description;
}

string LandUseType::GetName() const {
    return name;
}

LandUseType& LandUseType::operator=(const LandUseType& source) {
    this->id = source.id;
    this->genericTypeId = source.genericTypeId;
    this->name = source.name;
    this->description = source.description;
    this->unitName = source.unitName;
    return *this;
}

