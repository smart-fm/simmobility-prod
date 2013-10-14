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

void Household::SetAgeOfHead(int ageOfHead) {
    this->ageOfHead = ageOfHead;
}

int Household::GetAgeOfHead() const {
    return ageOfHead;
}

void Household::SetWorkers(int workers) {
    this->workers = workers;
}

int Household::GetWorkers() const {
    return workers;
}

void Household::SetHousingDuration(int housingDuration) {
    this->housingDuration = housingDuration;
}

int Household::GetHousingDuration() const {
    return housingDuration;
}

void Household::SetIncome(double income) {
    this->income = income;
}

double Household::GetIncome() const {
    return income;
}

void Household::SetChildren(int children) {
    this->children = children;
}

int Household::GetChildren() const {
    return children;
}

void Household::SetSize(int size) {
    this->size = size;
}

int Household::GetSize() const {
    return size;
}

void Household::SetVehicleCategoryId(BigSerial vehicleCategoryId) {
    this->vehicleCategoryId = vehicleCategoryId;
}

BigSerial Household::GetVehicleCategoryId() const {
    return vehicleCategoryId;
}

void Household::SetEthnicityId(BigSerial ethnicityId) {
    this->ethnicityId = ethnicityId;
}

BigSerial Household::GetEthnicityId() const {
    return ethnicityId;
}

void Household::SetUnitId(BigSerial unitId) {
    this->unitId = unitId;
}

BigSerial Household::GetUnitId() const {
    return unitId;
}

void Household::SetLifestyleId(BigSerial lifestyleId) {
    this->lifestyleId = lifestyleId;
}

BigSerial Household::GetLifestyleId() const {
    return lifestyleId;
}

void Household::SetId(BigSerial id) {
    this->id = id;
}

BigSerial Household::GetId() const {
    return id;
}
