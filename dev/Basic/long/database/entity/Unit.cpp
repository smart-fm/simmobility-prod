/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Property.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 3:05 PM
 */

#include "Unit.hpp"
#include "util/UnitHolder.hpp"
#include "util/Utils.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

#define WEIGHT_MIN 0.0f
#define WEIGHT_MAX 1.0f

/**
 * TODO: Refactoring with Market Entry?????
 */
float CalculateHedonicPrice(float distanceToCDB, float size, float fixedCost) {
    return (float) ((distanceToCDB * 2.0f) + (size * 3.0f) + fixedCost);
}

Unit::Unit(UnitId id, bool available, float fixedCost,
        float distanceToCDB, float size) : id(id), available(available),
fixedCost(fixedCost), distanceToCDB(distanceToCDB), size(size),
hedonicPrice(CalculateHedonicPrice(distanceToCDB, size, fixedCost)), owner(nullptr),
weightPriceQuality(Utils::GenerateFloat(WEIGHT_MIN, WEIGHT_MAX)) {
    reservationPrice = (hedonicPrice * 1.2f);
}

Unit::Unit() : id(INVALID_ID), available(false),
fixedCost(.0f), distanceToCDB(.0f), size(.0f),
hedonicPrice(.0f), reservationPrice(.0f), owner(nullptr),
weightPriceQuality(Utils::GenerateFloat(WEIGHT_MIN, WEIGHT_MAX)) {
}

Unit::Unit(const Unit& source) {
    this->id = source.id;
    this->available = source.available;
    this->fixedCost = source.fixedCost;
    this->hedonicPrice = source.hedonicPrice;
    this->reservationPrice = source.reservationPrice;
    this->owner = source.owner;
    this->weightPriceQuality = source.weightPriceQuality;
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
    this->weightPriceQuality = source.weightPriceQuality;
    return *this;
}

bool Unit::IsAvailable() const {
	boost::shared_lock<boost::shared_mutex> lock(mutex);
    return available;
}

void Unit::SetAvailable(bool avaliable) {
	boost::upgrade_lock<boost::shared_mutex> up_lock(mutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
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
	boost::shared_lock<boost::shared_mutex> lock(mutex);
    return reservationPrice;
}

void Unit::SetReservationPrice(float price) {
	boost::upgrade_lock<boost::shared_mutex> up_lock(mutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    reservationPrice = price;
}

float Unit::GetHedonicPrice() const {
	boost::shared_lock<boost::shared_mutex> lock(mutex);
    return hedonicPrice;
}

void Unit::SetHedonicPrice(float price) {
	boost::upgrade_lock<boost::shared_mutex> up_lock(mutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    hedonicPrice = price;
}

float Unit::GetFixedCost() const {
	boost::shared_lock<boost::shared_mutex> lock(mutex);
    return fixedCost;
}

void Unit::SetFixedCost(float cost) {
	boost::upgrade_lock<boost::shared_mutex> up_lock(mutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    fixedCost = cost;
}

UnitHolder* Unit::GetOwner() {
	boost::shared_lock<boost::shared_mutex> lock(mutex);
    return this->owner;
}

void Unit::SetOwner(UnitHolder* receiver) {
	boost::upgrade_lock<boost::shared_mutex> up_lock(mutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    this->owner = receiver;
}

float Unit::GetWeightPriceQuality() const {
    return weightPriceQuality;
}
