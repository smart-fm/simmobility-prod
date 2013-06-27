/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Building.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 8, 2013, 3:04 PM
 */

#include <set>

#include "Building.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

Building::Building() :
id(INVALID_ID), numberOfUnits(0), numberOfResidentialUnits(0), numberOfBusinessUnits(0),
numberOfStories(0), area(0), year(0), averageIncome(0),
mainRace(UNKNOWN_RACE), distanceToCDB(0) {
}

Building::~Building() {
}

Building& Building::operator=(const Building& source) {
    this->id = source.id;
    this->numberOfUnits = source.numberOfUnits;
    this->numberOfResidentialUnits = source.numberOfResidentialUnits;
    this->numberOfBusinessUnits = source.numberOfBusinessUnits;
    this->numberOfStories = source.numberOfStories;
    this->area = source.area;
    this->year = source.year;
    this->averageIncome = source.averageIncome;
    this->mainRace = source.mainRace;
    this->distanceToCDB = source.distanceToCDB;
    return *this;
}

BigSerial Building::GetId() const {
    return id;
}

int Building::GetNumberOfUnits() const {
    return numberOfUnits;
}

int Building::GetNumberOfResidentialUnits() const {
    return numberOfResidentialUnits;
}

int Building::GetNumberOfBusinessUnits() const {
    return numberOfBusinessUnits;
}

int Building::GetYear() const {
    return year;
}

double Building::GetArea() const {
    return area;
}

double Building::GetAverageIncome() const {
    return averageIncome;
}

Race Building::GetMainRace() const {
    return mainRace;
}

int Building::GetNumberOfStories() const {
    return numberOfStories;
}

double Building::GetDistanceToCDB() const {
    return distanceToCDB;
}
