/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Household.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 22, 2013, 5:54 PM
 */

#include "Household.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

Household::Household(int id) : id(id), headAge(0), income(.0f), numberOfCars(0),
numberOfChildren(0), numberOfWorkers(0), numberOfMembers(0), buildingId(-1) {
}

Household& Household::operator=(const Household& source) {
    this->id = source.id;
    this->buildingId = source.buildingId;
    this->headAge = source.headAge;
    this->income = source.income;
    this->numberOfCars = source.numberOfCars;
    this->numberOfChildren = source.numberOfChildren;
    this->numberOfMembers = source.numberOfMembers;
    this->numberOfWorkers = source.numberOfWorkers;
    this->race = source.race;
    return *this;
}

Household::~Household() {
}

int Household::GetId() const {
    return id;
}

int Household::GetBuildingId() const {
    return buildingId;
}

int Household::GetNumberOfMembers() const {
    return numberOfMembers;
}

int Household::GetNumberOfChildren() const {
    return numberOfChildren;
}

int Household::GetNumberOfWorkers() const {
    return numberOfWorkers;
}

int Household::GetNumberOfCars() const {
    return numberOfCars;
}

int Household::GetHeadAge() const {
    return headAge;
}

float Household::GetIncome() const {
    return income;
}

Race Household::GetRace() const {
    return race;
}
