/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   BuildingType.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 8, 2013, 11:19 AM
 */

#include "BuildingType.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

BuildingType::BuildingType()
: id(INVALID_ID), genericId(INVALID_ID), unitName(""), typeName(""),
description(""), genericDescription(""), residential(false) {
}

BuildingType::~BuildingType() {
}

BuildingType& BuildingType::operator=(const BuildingType& source) {
    this->id = source.id;
    this->genericId = source.genericId;
    this->unitName = source.unitName;
    this->typeName = source.typeName;
    this->description = source.description;
    this->genericDescription = source.genericDescription;
    this->residential = source.residential;
    return *this;
}

int BuildingType::GetId() const {
    return id;
}

int BuildingType::GetGenericId() const {
    return genericId;
}

string BuildingType::GetUnitName() const {
    return unitName;
}

string BuildingType::GetTypeName() const {
    return typeName;
}

string BuildingType::GetDescription() const {
    return description;
}

string BuildingType::GetGenericDescription() const {
    return genericDescription;
}

bool BuildingType::IsResidential() const {
    return residential;
}
