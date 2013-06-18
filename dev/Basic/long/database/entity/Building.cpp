/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Building.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 8, 2013, 3:04 PM
 */

#include "Building.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

Building::Building() :
id(INVALID_ID), improvementValue(0), landArea(0), nonResidentialSqft(0),
parcelId(INVALID_ID), qualityId(INVALID_ID), residentialUnits(0), stories(0),
taxExempt(false), templateId(INVALID_ID), typeId(INVALID_ID), unitSqft(0),
year(0) {
}

Building::~Building() {
}

Building& Building::operator=(const Building& source) {
    this->id = source.id;
    this->improvementValue = source.improvementValue;
    this->landArea = source.landArea;
    this->nonResidentialSqft = source.nonResidentialSqft;
    this->parcelId = source.parcelId;
    this->qualityId = source.qualityId;
    this->residentialUnits = source.residentialUnits;
    this->stories = source.stories;
    this->taxExempt = source.taxExempt;
    this->templateId = source.templateId;
    this->typeId = source.typeId;
    this->unitSqft = source.unitSqft;
    this->year = source.year;
    return *this;
}

int Building::GetId() const {
    return id;
}

int Building::GetResidentialUnits() const {
    return residentialUnits;
}

int Building::GetYear() const {
    return year;
}

int Building::GetParcelId() const {
    return parcelId;
}

double Building::GetLandArea() const {
    return landArea;
}

int Building::GetQualityId() const {
    return landArea;
}

int Building::GetImprovementValue() const {
    return improvementValue;
}

int Building::GetTypeId() const {
    return typeId;
}

int Building::GetStories() const {
    return stories;
}

bool Building::IsTaxExempt() const {
    return taxExempt;
}

int Building::GetNonResidentialSqft() const {
    return nonResidentialSqft;
}

int Building::GetTemplateId() const {
    return templateId;
}

int Building::GetUnitSqft() const {
    return unitSqft;
}