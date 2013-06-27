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

double CalculateHedonicPrice(const Unit& unit) {
    return (double) unit.GetFixedPrice() +
            unit.GetArea() * unit.GetWeightArea() +
            (((int) unit.GetType()) + 1) * unit.GetWeightType() +
            unit.GetStorey() * unit.GetWeightStorey() +
            unit.GetLastRemodulationYear() * unit.GetWeightYearLastRemodulation() +
            unit.GetTaxExempt() * unit.GetWeightTaxExempt() +
            unit.GetDistanceToCBD() * unit.GetWeightDistanceToCBD();
}

Unit::Unit(UnitId id,
        BigSerial buildingId,
        BigSerial householdId,
        UnitType type,
        bool available,
        double area,
        int storey,
        int lastRemodulationYear,
        double fixedPrice,
        double taxExempt,
        double distanceToCBD,
        bool hasGarage,
        double weightPriceQuality,
        double weightStorey,
        double weightDistanceToCBD,
        double weightType,
        double weightArea,
        double weightTaxExempt,
        double weightYearLastRemodulation) :
id(id),
buildingId(buildingId),
householdId(householdId),
type(type),
storey(storey),
lastRemodulationYear(lastRemodulationYear),
area(area),
fixedPrice(fixedPrice),
taxExempt(taxExempt),
hedonicPrice(0),
distanceToCBD(distanceToCBD),
hasGarage(hasGarage),
weightPriceQuality(weightPriceQuality),
weightStorey(weightStorey),
weightDistanceToCBD(weightDistanceToCBD),
weightType(weightType),
weightArea(weightArea),
weightTaxExempt(weightTaxExempt),
weightYearLastRemodulation(weightYearLastRemodulation),
available(available),
owner(nullptr) {
    hedonicPrice = CalculateHedonicPrice(*this);
    reservationPrice = hedonicPrice;
}

Unit::Unit() :
id(INVALID_ID),
buildingId(INVALID_ID),
householdId(INVALID_ID),
type(UNKNOWN_UNIT_TYPE),
storey(0),
lastRemodulationYear(0),
area(0),
fixedPrice(0),
taxExempt(0),
hedonicPrice(0),
distanceToCBD(0),
hasGarage(false),
weightPriceQuality(0),
weightStorey(0),
weightDistanceToCBD(0),
weightType(0),
weightArea(0),
weightTaxExempt(0),
weightYearLastRemodulation(0),
reservationPrice(0),
available(false),
owner(nullptr) {
}

Unit::Unit(const Unit& source) {
    this->id = source.id;
    this->buildingId = source.buildingId;
    this->householdId = source.householdId;
    this->type = source.type;
    this->storey = source.storey;
    this->lastRemodulationYear = source.lastRemodulationYear;
    this->area = source.area;
    this->fixedPrice = source.fixedPrice;
    this->taxExempt = source.taxExempt;
    this->hedonicPrice = source.hedonicPrice;
    this->distanceToCBD = source.distanceToCBD;
    this->hasGarage = source.hasGarage;
    this->weightPriceQuality = source.weightPriceQuality;
    this->weightStorey = source.weightStorey;
    this->weightDistanceToCBD = source.weightDistanceToCBD;
    this->weightType = source.weightType;
    this->weightArea = source.weightArea;
    this->weightTaxExempt = source.weightTaxExempt;
    this->weightYearLastRemodulation = source.weightYearLastRemodulation;
    this->reservationPrice = source.reservationPrice;
    this->available = source.available;
}

Unit::~Unit() {
}

Unit& Unit::operator=(const Unit& source) {
    this->id = source.id;
    this->buildingId = source.buildingId;
    this->householdId = source.householdId;
    this->type = source.type;
    this->storey = source.storey;
    this->lastRemodulationYear = source.lastRemodulationYear;
    this->area = source.area;
    this->fixedPrice = source.fixedPrice;
    this->taxExempt = source.taxExempt;
    this->hedonicPrice = source.hedonicPrice;
    this->distanceToCBD = source.distanceToCBD;
    this->hasGarage = source.hasGarage;
    this->weightPriceQuality = source.weightPriceQuality;
    this->weightStorey = source.weightStorey;
    this->weightDistanceToCBD = source.weightDistanceToCBD;
    this->weightType = source.weightType;
    this->weightArea = source.weightArea;
    this->weightTaxExempt = source.weightTaxExempt;
    this->weightYearLastRemodulation = source.weightYearLastRemodulation;
    this->reservationPrice = source.reservationPrice;
    this->available = source.available;
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

double Unit::GetReservationPrice() const {
    SharedReadLock(mutex);
    return reservationPrice;
}

void Unit::SetReservationPrice(double price) {
    SharedWriteLock(mutex);
    reservationPrice = price;
}

UnitId Unit::GetId() const {
    return id;
}

UnitHolder* Unit::GetOwner() {
    SharedReadLock(mutex);
    return this->owner;
}

void Unit::SetOwner(UnitHolder* receiver) {
    SharedWriteLock(mutex);
    this->owner = receiver;
}

BigSerial Unit::GetBuildingId() const {
    return buildingId;
}

BigSerial Unit::GetHouseholdId() const {
    return householdId;
}

UnitType Unit::GetType() const {
    return type;
}

int Unit::GetStorey() const {
    return storey;
}

int Unit::GetLastRemodulationYear() const {
    return lastRemodulationYear;
}

double Unit::GetArea() const {
    return area;
}

double Unit::GetFixedPrice() const {
    return fixedPrice;
}

double Unit::GetTaxExempt() const {
    return taxExempt;
}

double Unit::GetHedonicPrice() const {
    return hedonicPrice;
}

double Unit::GetDistanceToCBD() const {
    return distanceToCBD;
}

bool Unit::HasGarage() const {
    return hasGarage;
}

double Unit::GetWeightPriceQuality() const {
    return weightPriceQuality;
}

double Unit::GetWeightStorey() const {
    return weightStorey;
}

double Unit::GetWeightDistanceToCBD() const {
    return weightDistanceToCBD;
}

double Unit::GetWeightType() const {
    return weightType;
}

double Unit::GetWeightArea() const {
    return weightArea;
}

double Unit::GetWeightTaxExempt() const {
    return weightTaxExempt;
}

double Unit::GetWeightYearLastRemodulation() const {
    return weightYearLastRemodulation;
}