/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Individual.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 22, 2013, 5:46 PM
 */

#include "Individual.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

Individual::Individual(int id, HousingMarket* market, float income)
: LT_Agent(id, market), income(income), sex(UNKNOWN_SEX), race(UNKNOWN_RACE) {
}

Individual::~Individual() {
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

Sex Individual::GetSex() const{
    return sex;
}
            
Race Individual::GetRace() const{
    return race;
}