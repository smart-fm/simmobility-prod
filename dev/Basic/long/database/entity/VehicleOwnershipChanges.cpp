/*
 * VehicleOwnershipChanges.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: gishara
 */
#include "VehicleOwnershipChanges.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

VehicleOwnershipChanges::VehicleOwnershipChanges(BigSerial householdId, int vehicleOwnershipOptionId, std::tm startDate) :
		householdId(householdId), vehicleOwnershipOptionId(vehicleOwnershipOptionId), startDate(startDate){
}

VehicleOwnershipChanges::~VehicleOwnershipChanges() {
}

VehicleOwnershipChanges::VehicleOwnershipChanges( const VehicleOwnershipChanges &source)
{
	this->householdId = source.householdId;
	this->vehicleOwnershipOptionId = source.vehicleOwnershipOptionId;
	this->startDate = source.startDate;
}

VehicleOwnershipChanges& VehicleOwnershipChanges::operator=(const VehicleOwnershipChanges& source)
{
	this->householdId = source.householdId;
	this->vehicleOwnershipOptionId = source.vehicleOwnershipOptionId;
	this->startDate = source.startDate;

    return *this;
}

BigSerial VehicleOwnershipChanges::getHouseholdId() const
{
	return householdId;
}

void VehicleOwnershipChanges::setHouseholdId(BigSerial householdId)
{
	this->householdId = householdId;
}

const std::tm& VehicleOwnershipChanges::getStartDate() const
{
	return startDate;
}

void VehicleOwnershipChanges::setStartDate(const std::tm& startDate)
{
	this->startDate = startDate;
}

int VehicleOwnershipChanges::getVehicleOwnershipOptionId() const
{
	return vehicleOwnershipOptionId;
}

void VehicleOwnershipChanges::setVehicleOwnershipOptionId(int vehicleOwnershipOptionId)
{
	this->vehicleOwnershipOptionId = vehicleOwnershipOptionId;
}
