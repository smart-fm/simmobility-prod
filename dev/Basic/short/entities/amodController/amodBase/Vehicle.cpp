/*
 * Vehicle.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#include "Vehicle.hpp"

namespace sim_mob{

namespace amod {
    
typedef Vehicle::Status VehicleStatus;

Vehicle::Vehicle(int id) : status(UNKNOWN), capacity(1), speed(0), custId(0), locId(0) {
    // TODO Auto-generated constructor stub
    Entity::setId(id);
}

Vehicle::Vehicle(int id, const std::string& name, Position pos, int capacity_, VehicleStatus status_) :
Entity(id, name, pos), capacity(capacity_), status(status_), speed(0), custId(0)
{
    return;
}

Vehicle::~Vehicle() {
    // TODO Auto-generated destructor stub
}

VehicleStatus Vehicle::getStatus() const{
    return status;
}

void Vehicle::setStatus(Vehicle::Status s) {
    status = s;
}

double Vehicle::getSpeed() const {
    return speed;
}

void Vehicle::setSpeed(double speed_) {
    speed = speed_;
}

void Vehicle::setCustomerId(int custId_) {
    custId = custId_;
}

int Vehicle::getCustomerId() const {
    return custId;
}

void Vehicle::clearCustomerId() {
    custId = 0;
}


void Vehicle::setCapacity(int capacity_) {
    capacity = capacity_;
}

int Vehicle::getCapacity() const {
    return capacity;
}

void Vehicle::setWaypoints(std::list<Position> &waypoints_) {
    wayPoints = waypoints_;
}

void Vehicle::getWaypoints(std::list<Position> *waypoints_) {
    *waypoints_ = wayPoints;
}

int Vehicle::getLocationId() {
    return locId;
}

void Vehicle::setLocationId(int locId_) {
    locId = locId_;
}

}

}
