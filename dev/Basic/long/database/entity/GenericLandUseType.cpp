/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   GenericLandUseType.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 13, 2013, 6:11 PM
 */

#include "GenericLandUseType.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

GenericLandUseType::GenericLandUseType() {
}

GenericLandUseType::~GenericLandUseType() {
}

int GenericLandUseType::GetId() const {
    return id;
}

string GenericLandUseType::GetUnitName() const {
    return unitName;
}

string GenericLandUseType::GetName() const {
    return name;
}

GenericLandUseType& GenericLandUseType::operator=(const GenericLandUseType& source) {
    this->id = source.id;
    this->name = source.name;
    this->unitName = source.unitName;
    return *this;
}

