/*
 * Location.cpp
 *
 *  Created on: Mar 27, 2015
 *      Author: haroldsoh
 */

#include "Location.hpp"

namespace sim_mob{

namespace amod {

Location::Location() : capacity(0) {
	// nothing to do
}

Location::Location(int id, std::string name, Position pos, int capacity_):
    Entity(id, name, pos), capacity(capacity_) {
	// nothing to do here
}

Location::~Location() {
	// nothing to do here
}

void Location::addVehicleId(int vehId_) {
    vehIds.insert(vehId_);
}

void Location::removeVehicleId(int vehId_) {
    vehIds.erase(vehId_);
}

void Location::getVehicleIds(std::unordered_set<int> *vehIds_) {
    *vehIds_ = vehIds;
}

void Location::getVehicleIds(std::unordered_set<int>::const_iterator *bitr, std::unordered_set<int>::const_iterator *eitr) {
	*bitr = vehIds.begin();
	*eitr = vehIds.end();
}

void Location::clearVehicleIds() {
	vehIds.clear();
}

int Location::getNumVehicles() const {
	return (int) vehIds.size();
}


int Location::getNumCustomers() const {
	return (int) custIds.size();
}

void Location::getCustomerIds(std::unordered_set<int> *custIds_) {
    *custIds_ = custIds;
}

void Location::getCustomerIds(std::unordered_set<int>::const_iterator *bitr, std::unordered_set<int>::const_iterator *eitr) {
	*bitr = custIds.begin();
	*eitr = custIds.end();
}


void Location::addCustomerId(int custId_) {
    custIds.insert(custId_);
}

void Location::removeCustomerId(int custId_) {
    custIds.erase(custId_);
}

void Location::clearCustomerIds() {
	custIds.clear();
}

void Location::setCapacity(int capacity_) {
    capacity = capacity_;
}

int Location::getCapacity() const {
	return capacity;
}

}

}
