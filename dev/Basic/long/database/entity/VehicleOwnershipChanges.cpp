/*
 * VehicleOwnershipChanges.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: gishara
 */
#include "VehicleOwnershipChanges.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

VehicleOwnershipChanges::VehicleOwnershipChanges(BigSerial householdId, int oldVehicleOwnershipOptionId, int newVehicleOwnershipOptionId,std::tm startDate) :
		householdId(householdId), oldVehicleOwnershipOptionId(oldVehicleOwnershipOptionId), newVehicleOwnershipOptionId(newVehicleOwnershipOptionId), startDate(startDate){
}

VehicleOwnershipChanges::~VehicleOwnershipChanges() {
}

VehicleOwnershipChanges::VehicleOwnershipChanges( const VehicleOwnershipChanges &source)
{
	this->householdId = source.householdId;
	this->oldVehicleOwnershipOptionId = source.oldVehicleOwnershipOptionId;
	this->newVehicleOwnershipOptionId = source.newVehicleOwnershipOptionId;
	this->startDate = source.startDate;
}

VehicleOwnershipChanges& VehicleOwnershipChanges::operator=(const VehicleOwnershipChanges& source)
{
	this->householdId = source.householdId;
	this->oldVehicleOwnershipOptionId = source.oldVehicleOwnershipOptionId;
	this->newVehicleOwnershipOptionId = source.newVehicleOwnershipOptionId;
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

int VehicleOwnershipChanges::getOldVehicleOwnershipOptionId() const
{
	return oldVehicleOwnershipOptionId;
}

void VehicleOwnershipChanges::setOldVehicleOwnershipOptionId(int vehicleOwnershipOptionId)
{
	this->oldVehicleOwnershipOptionId = vehicleOwnershipOptionId;
}

int VehicleOwnershipChanges::getNewVehicleOwnershipOptionId() const
{
	return newVehicleOwnershipOptionId;
}

void VehicleOwnershipChanges::setNewVehicleOwnershipOptionId(int vehicleOwnershipOptionId)
{
	this->newVehicleOwnershipOptionId = vehicleOwnershipOptionId;
}
