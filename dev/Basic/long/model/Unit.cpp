/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Property.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 3:05 PM
 */

#include "Unit.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

/**
 * TODO: Refactoring with Market Entry?????
 */
float CalculateHedonicPrice(float distanceToCDB, float size, float fixedCost) {
    return (float) ((distanceToCDB * 2.0f) + (size * 3.0f) + fixedCost);
}

Unit::Unit(UnitId id, bool available, float fixedCost,
        float distanceToCDB, float size) : id(id), available(available),
fixedCost(fixedCost), distanceToCDB(distanceToCDB), size(size),
hedonicPrice(CalculateHedonicPrice(distanceToCDB, size, fixedCost)),
reservationPrice(hedonicPrice * 1.2f), owner(nullptr) {
}

Unit::Unit(const Unit& source) {
    this->id = source.id;
    this->available = source.available;
    this->fixedCost = source.fixedCost;
    this->hedonicPrice = source.hedonicPrice;
    this->reservationPrice = source.reservationPrice;
    this->owner = source.owner;
}

Unit::~Unit() {
}

Unit& Unit::operator=(const Unit& source) {
    this->id = source.id;
    this->available = source.available;
    this->fixedCost = source.fixedCost;
    this->hedonicPrice = source.hedonicPrice;
    this->reservationPrice = source.reservationPrice;
    this->owner = source.owner;
    return *this;
}

bool Unit::IsAvailable() const {
    SharedReadLock(mutex);
    return available;
}

void Unit::SetAvailable(bool avaliable) {
    SharedWriteLock(mutex);
    this->available = avaliable;
}

UnitId Unit::GetId() const {
    return id;
}

float Unit::GetSize() const {
    return size;
}

float Unit::GetDistanceToCDB() const {
    return distanceToCDB;
}

float Unit::GetReservationPrice() const {
    SharedReadLock(mutex);
    return reservationPrice;
}

void Unit::SetReservationPrice(float price) {
    SharedWriteLock(mutex);
    reservationPrice = price;
}

float Unit::GetHedonicPrice() const {
    SharedReadLock(mutex);
    return hedonicPrice;
}

void Unit::SetHedonicPrice(float price) {
    SharedWriteLock(mutex);
    hedonicPrice = price;
}

float Unit::GetFixedCost() const {
    SharedReadLock(mutex);
    return fixedCost;
}

void Unit::SetFixedCost(float cost) {
    SharedWriteLock(mutex);
    fixedCost = cost;
}

MessageReceiver* Unit::GetOwner() {
    SharedReadLock(mutex);
    return this->owner;
}

void Unit::SetOwner(MessageReceiver* receiver) {
    SharedWriteLock(mutex);
    this->owner = receiver;
}
