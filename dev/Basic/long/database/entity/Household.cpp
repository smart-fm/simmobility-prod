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
#include "database/dao/SqlAbstractDao.hpp"

using namespace sim_mob::long_term;

Household::Household() {
}

Household::~Household() {
}

Household& Household::operator=(const Household& source) {
    this->id = source.id;
    this->lifestyleId = source.lifestyleId;
    this->unitId = source.unitId;
    this->ethnicityId = source.ethnicityId;
    this->vehicleCategoryId = source.vehicleCategoryId;
    this->income = source.income;
    this->size = source.size;
    this->children = source.children;
    this->housingDuration = source.housingDuration;
    this->workers = source.workers;
    this->ageOfHead = source.ageOfHead;
    return *this;
}

void Household::setAgeOfHead(int ageOfHead) {
    this->ageOfHead = ageOfHead;
}

int Household::getAgeOfHead() const {
    return ageOfHead;
}

void Household::setWorkers(int workers) {
    this->workers = workers;
}

int Household::getWorkers() const {
    return workers;
}

void Household::setHousingDuration(int housingDuration) {
    this->housingDuration = housingDuration;
}

int Household::getHousingDuration() const {
    return housingDuration;
}

void Household::setIncome(double income) {
    this->income = income;
}

double Household::getIncome() const {
    return income;
}

void Household::setChildren(int children) {
    this->children = children;
}

int Household::getChildren() const {
    return children;
}

void Household::setSize(int size) {
    this->size = size;
}

int Household::getSize() const {
    return size;
}

void Household::setVehicleCategoryId(BigSerial vehicleCategoryId) {
    this->vehicleCategoryId = vehicleCategoryId;
}

BigSerial Household::getVehicleCategoryId() const {
    return vehicleCategoryId;
}

void Household::setEthnicityId(BigSerial ethnicityId) {
    this->ethnicityId = ethnicityId;
}

BigSerial Household::getEthnicityId() const {
    return ethnicityId;
}

void Household::setUnitId(BigSerial unitId) {
    this->unitId = unitId;
}

BigSerial Household::getUnitId() const {
    return unitId;
}

void Household::setLifestyleId(BigSerial lifestyleId) {
    this->lifestyleId = lifestyleId;
}

BigSerial Household::getLifestyleId() const {
    return lifestyleId;
}

void Household::setId(BigSerial id) {
    this->id = id;
}

BigSerial Household::getId() const {
    return id;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const Household& data) {
            return strm << "{"
                    << "\"id\":\"" << data.id << "\","
                    << "\"lifestyleId\":\"" << data.lifestyleId << "\","
                    << "\"unitId\":\"" << data.unitId << "\","
                    << "\"ethnicityId\":\"" << data.ethnicityId << "\","
                    << "\"vehicleCategoryId\":\"" << data.vehicleCategoryId << "\","
                    << "\"size\":\"" << data.size << "\","
                    << "\"children\":\"" << data.children << "\","
                    << "\"income\":\"" << data.income << "\","
                    << "\"housingDuration\":\"" << data.housingDuration << "\","
                    << "\"workers\":\"" << data.workers << "\","
                    << "\"ageOfHead\":\"" << data.ageOfHead << "\""
                    << "}";
        }
    }
}