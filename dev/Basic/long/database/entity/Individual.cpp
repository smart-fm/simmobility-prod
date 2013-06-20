/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Individual.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 22, 2013, 5:46 PM
 */

#include "Individual.hpp"
#include "Common.h"

using namespace sim_mob;
using namespace sim_mob::long_term;

Individual::Individual()
: id(INVALID_ID), income(0), householdId(INVALID_ID), sex(UNKNOWN_SEX),
race(UNKNOWN_RACE), employmentStatus(UNKNOWN_ESTATUS), transportMode(-1), hboTrips(0),
hbwTrips(0), jobId(INVALID_ID), workAtHome(false), zoneId(INVALID_ID) {
}

Individual::~Individual() {
}

Individual& Individual::operator=(const Individual& source) {
    this->id = source.id;
    this->age = source.age;
    this->employmentStatus = source.employmentStatus;
    this->hboTrips = source.hboTrips;
    this->hbwTrips = source.hbwTrips;
    this->householdId = source.householdId;
    this->income = source.income;
    this->jobId = source.jobId;
    this->numberOfCars = source.numberOfCars;
    this->race = source.race;
    this->sex = source.sex;
    this->transportMode = source.transportMode;
    this->workAtHome = source.workAtHome;
    this->zoneId = source.zoneId;
    return *this;
}

int Individual::GetId() const {
    return id;
}

int Individual::GetHouseholdId() const {
    return householdId;
}

float Individual::GetIncome() const {
    return income;
}

bool Individual::HasJob() const {
    return true;
}

int Individual::GetAge() const {
    return age;
}

int Individual::GetNumberOfCars() const {
    return numberOfCars;
}

Sex Individual::GetSex() const {
    return sex;
}

Race Individual::GetRace() const {
    return race;
}

EmploymentStatus Individual::GetEmploymentStatus() const {
    return employmentStatus;
}

int Individual::GetTransportMode() const {
    return transportMode;
}

int Individual::GetHBOTrips() const {
    return hboTrips;
}

int Individual::GetHBWTrips() const {
    return hbwTrips;
}

int Individual::GetJobId() const {
    return hbwTrips;
}

int Individual::GetZoneId() const {
    return hbwTrips;
}

bool Individual::IsWorkAtHome() const {
    return workAtHome;
}