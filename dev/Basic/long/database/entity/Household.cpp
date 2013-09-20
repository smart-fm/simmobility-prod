//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Household.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 22, 2013, 5:54 PM
 */

#include "Household.hpp"
#include "util/Utils.hpp"
#include "database/dao/AbstractDao.hpp"

using namespace sim_mob::long_term;

Household::Household(BigSerial id, double income, int size, int children,
        int carOwnership, BigSerial unitId, int housingDuration)
: id(id), income(income), size(size), children(children), carOwnership(carOwnership),
unitId(unitId), housingDuration(housingDuration) {
}

Household::~Household() {
}

BigSerial Household::GetId() const {
    return id;
}

BigSerial Household::GetUnitId() const {
    return unitId;
}

int Household::GetSize() const {
    return size;
}

int Household::GetChildren() const {
    return children;
}

double Household::GetIncome() const {
    return income;
}

int Household::GetCarOwnership() const {
    return carOwnership;
}

int Household::GetHousingDuration() const {
    return housingDuration;
}

Household& Household::operator=(const Household& source) {
    this->id = source.id;
    this->unitId = source.unitId;
    this->income = source.income;
    this->size = source.size;
    this->children = source.children;
    this->carOwnership = source.carOwnership;
    this->housingDuration = source.housingDuration;
    return *this;
}
