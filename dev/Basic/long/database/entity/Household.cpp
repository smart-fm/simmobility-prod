//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Household.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *
 * Created on April 22, 2013, 5:54 PM
 */

#include "Household.hpp"
#include "util/Utils.hpp"
#include "database/dao/SqlAbstractDao.hpp"

using namespace sim_mob::long_term;

Household::Household( BigSerial id, BigSerial lifestyleId, BigSerial unitId, BigSerial ethnicityId, BigSerial vehicleCategoryId,  int size, int childUnder4, int childUnder15, double income,
					  int housingDuration,int workers, int ageOfHead, bool twoRoomHdbEligibility, bool threeRoomHdbEligibility, bool fourRoomHdbEligibility, int familyType, bool taxiAvailability, int vehicleOwnershipOptionId): id(id),
					  lifestyleId(lifestyleId), unitId(unitId), ethnicityId(ethnicityId), vehicleCategoryId(vehicleCategoryId),size(size), childUnder4(childUnder4), childUnder15(childUnder15), income(income),
					  housingDuration(housingDuration), workers(workers), ageOfHead(ageOfHead), twoRoomHdbEligibility(twoRoomHdbEligibility),
					  threeRoomHdbEligibility(threeRoomHdbEligibility), fourRoomHdbEligibility(fourRoomHdbEligibility),familyType(familyType), taxiAvailability(taxiAvailability), vehicleOwnershipOptionId(vehicleOwnershipOptionId){}

Household::Household(): id(0), lifestyleId(0), unitId(0), ethnicityId(0), vehicleCategoryId(0),size(0), childUnder4(0), childUnder15(0), income(0), housingDuration(0), workers(0), ageOfHead(0),
						twoRoomHdbEligibility(0), threeRoomHdbEligibility(0), fourRoomHdbEligibility(0), familyType(0),taxiAvailability(false), vehicleOwnershipOptionId(0){}

Household::~Household() {}

Household& Household::operator=(const Household& source)
{
    this->id = source.id;
    this->lifestyleId = source.lifestyleId;
    this->unitId = source.unitId;
    this->ethnicityId = source.ethnicityId;
    this->vehicleCategoryId = source.vehicleCategoryId;
    this->income = source.income;
    this->size = source.size;
    this->childUnder4 = source.childUnder15;
    this->childUnder15 = source.childUnder15;
    this->housingDuration = source.housingDuration;
    this->workers = source.workers;
    this->ageOfHead = source.ageOfHead;
    this->taxiAvailability = source.taxiAvailability;
    this->vehicleOwnershipOptionId = source.vehicleOwnershipOptionId;
    return *this;
}

void Household::setAgeOfHead(int ageOfHead)
{
    this->ageOfHead = ageOfHead;
}

int Household::getAgeOfHead() const
{
    return ageOfHead;
}

void Household::setWorkers(int workers)
{
    this->workers = workers;
}

int Household::getWorkers() const
{
    return workers;
}

void Household::setHousingDuration(int housingDuration)
{
    this->housingDuration = housingDuration;
}

int Household::getHousingDuration() const
{
    return housingDuration;
}

void Household::setIncome(double income)
{
    this->income = income;
}

double Household::getIncome() const
{
    return income;
}

void Household::setChildUnder4(int childUnder4)
{
    this->childUnder4 = childUnder4;
}

void Household::setChildUnder15(int childUnder15)
{
    this->childUnder15 = childUnder15;
}

int Household::getChildUnder4() const
{
    return childUnder4;
}

int Household::getChildUnder15() const
{
    return childUnder15;
}

void Household::setSize(int size)
{
    this->size = size;
}

int Household::getSize() const
{
    return size;
}

void Household::setVehicleCategoryId(BigSerial vehicleCategoryId)
{
    this->vehicleCategoryId = vehicleCategoryId;
}

BigSerial Household::getVehicleCategoryId() const
{
    return vehicleCategoryId;
}

void Household::setEthnicityId(BigSerial ethnicityId)
{
    this->ethnicityId = ethnicityId;
}

BigSerial Household::getEthnicityId() const
{
    return ethnicityId;
}

void Household::setUnitId(BigSerial unitId)
{
    this->unitId = unitId;
}

BigSerial Household::getUnitId() const
{
    return unitId;
}

void Household::setLifestyleId(BigSerial lifestyleId)
{
    this->lifestyleId = lifestyleId;
}

BigSerial Household::getLifestyleId() const
{
    return lifestyleId;
}

void Household::setId(BigSerial id)
{
    this->id = id;
}

BigSerial Household::getId() const
{
    return id;
}

std::vector<BigSerial> Household::getIndividuals() const
{
	return individuals;
}

void Household::setIndividual( BigSerial individual )
{
	individuals.push_back( individual );
}


bool  Household::getTwoRoomHdbEligibility() const
{
	return twoRoomHdbEligibility;
}

bool  Household::getThreeRoomHdbEligibility() const
{
	return threeRoomHdbEligibility;
}

bool  Household::getFourRoomHdbEligibility() const
{
	return fourRoomHdbEligibility;
}

void  Household::setTwoRoomHdbEligibility(bool eligibility)
{
	twoRoomHdbEligibility = true;
}

void  Household::setThreeRoomHdbEligibility(bool eligibility)
{
	threeRoomHdbEligibility = true;
}

void  Household::setFourRoomHdbEligibility(bool eligibility)
{
	fourRoomHdbEligibility = true;
}

void Household::setFamilyType( int type )
{
	familyType = type;
}

int Household::getFamilyType()
{
	return familyType;
}

bool Household::getTaxiAvailability()
{
	return this->taxiAvailability;
}

void Household::setTaxiAvailability(bool taxiAvailable)
{
	taxiAvailability = taxiAvailable;
}

void Household::setVehicleOwnershipOptionId(int vehicleOwnershipOption)
{
	this->vehicleOwnershipOptionId = vehicleOwnershipOption;
}

int Household::getVehicleOwnershipOptionId()
{
	return this->vehicleOwnershipOptionId;
}

void Household::setTaxiAvailability(bool taxiAvailable)
{
	taxiAvailability = taxiAvailable;
}

void Household::setVehicleOwnershipOptionId(int vehicleOwnershipOption)
{
	this->vehicleOwnershipOptionId = vehicleOwnershipOption;
}

int Household::getVehicleOwnershipOptionId()
{
	return this->vehicleOwnershipOptionId;
}

void Household::setAffordabilityAmount(double value)
{
	householdAffordabilityAmount = value;
}

double Household::getAffordabilityAmount() const
{
	return householdAffordabilityAmount;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const Household& data)
        {
            return strm << "{"
                    << "\"id\":\"" << data.id << "\","
                    << "\"lifestyleId\":\"" << data.lifestyleId << "\","
                    << "\"unitId\":\"" << data.unitId << "\","
                    << "\"ethnicityId\":\"" << data.ethnicityId << "\","
                    << "\"vehicleCategoryId\":\"" << data.vehicleCategoryId << "\","
                    << "\"size\":\"" << data.size << "\","
                    << "\"childUnder4\":\"" << data.childUnder4 << "\","
                    << "\"childUnder15\":\"" << data.childUnder15 << "\","
                    << "\"income\":\"" << data.income << "\","
                    << "\"housingDuration\":\"" << data.housingDuration << "\","
                    << "\"workers\":\"" << data.workers << "\","
                    << "\"ageOfHead\":\"" << data.ageOfHead << "\""
                    << "\"taxiAvailability\":\"" << data.taxiAvailability << "\""
                    <<"\"vehicleOwnershipOptionId\":\"" << data.vehicleOwnershipOptionId << "\""
                    << "}";
        }
    }
}
