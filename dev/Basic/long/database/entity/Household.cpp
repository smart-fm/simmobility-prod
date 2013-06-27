/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Household.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 22, 2013, 5:54 PM
 */

#include "Household.hpp"
#include "util/Utils.hpp"
#include "database/dao/AbstractDao.hpp"

#define WEIGHT_MIN 0.0f
#define WEIGHT_MAX 1.0f

using namespace sim_mob::long_term;

Household::Household(unsigned long id, float income, int numberOfIndividuals) : id(id),
headAge(0), income(income), numberOfCars(0), numberOfChildren(0),
numberOfWorkers(0), numberOfIndividuals(numberOfIndividuals){
}

Household& Household::operator=(const Household& source) {
    this->id = source.id;
    this->headAge = source.headAge;
    this->income = source.income;
    this->numberOfCars = source.numberOfCars;
    this->numberOfChildren = source.numberOfChildren;
    this->numberOfIndividuals = source.numberOfIndividuals;
    this->numberOfWorkers = source.numberOfWorkers;
    this->race = source.race;
    return *this;
}

Household::~Household() {
}

BigSerial Household::GetId() const {
    return id;
}

int Household::GetNumberOfIndividuals() const {
    return numberOfIndividuals;
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