/*
 * Customer.cpp
 *
 *  Created on: Mar 27, 2015
 *      Author: haroldsoh
 */

#include "Customer.hpp"

namespace sim_mob {

namespace amod {


Customer::~Customer() {
	return;
}

Customer::Customer(int id, const std::string& name, amod::Position pos, int locationId, int assignedVeh, bool inVeh, Customer::Status status_) :
        Entity(id, name, pos), vehId(assignedVeh), status(status_), locId(locationId) {
	return;
}

void Customer::setStatus(Status s) {
    status = s;
}

CustomerStatus Customer::getStatus() const {
    return status;
}

void Customer::setInVehicle() {
    status = IN_VEHICLE;
}

bool Customer::isInVehicle() {
    return (status == IN_VEHICLE || status == WAITING_FOR_DROPOFF);
}

void Customer::setAssignedVehicleId(int vehId_) {
    vehId = vehId_;
}

void Customer::clearAssignedVehicleId() {
    vehId = 0;
}

int Customer::getAssignedVehicleId() {
    return vehId;
}

int Customer::getLocationId() {
    return locId;
}

void Customer::setLocationId(int locId_) {
    locId = locId_;
}

}
}
