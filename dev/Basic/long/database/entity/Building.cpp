//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Building.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 8, 2013, 3:04 PM
 */

#include "Building.hpp"

using namespace sim_mob::long_term;

Building::Building( BigSerial fmBuildingId, BigSerial fmProjectId, BigSerial fmParcelId, int storeysAboveGround, int storeysBelowGround,
					std::tm fromDate, std::tm toDate, int buildingStatus, float	grossSqMRes, float grossSqMOffice,
					float grossSqMRetail, float grossSqMWarehouse, float grossSqMIndustrial, float grossSqMOther, float grossSqMCivic,std::tm lastChangedDate,int freehold,float floorSpace,std::string buildingType,BigSerial slaAddressId) :
					fmBuildingId(fmBuildingId), fmProjectId(fmProjectId), fmParcelId(fmParcelId), storeysAboveGround(storeysAboveGround),storeysBelowGround(storeysBelowGround), fromDate(fromDate),
					toDate(toDate), buildingStatus(buildingStatus),grossSqMRes(grossSqMRes),grossSqMOffice(grossSqMOffice), grossSqMRetail(grossSqMRetail), grossSqMWarehouse(grossSqMWarehouse),
					grossSqMIndustrial(grossSqMIndustrial),grossSqMOther(grossSqMOther),grossSqMCivic(grossSqMCivic),
					lastChangedDate(lastChangedDate),freehold(freehold),floorSpace(floorSpace),buildingType(buildingType),slaAddressId(slaAddressId){}

Building::~Building() {}

Building::Building( const Building &source)
{
	this->fmBuildingId 			= source.fmBuildingId;
	this->fmProjectId			= source.fmProjectId;
	this->fmParcelId 			= source.fmParcelId;
	this->storeysAboveGround	= source.storeysAboveGround;
	this->storeysBelowGround  	= source.storeysBelowGround;
	this->fromDate				= source.fromDate;
	this->toDate				= source.toDate;
	this->buildingStatus		= source.buildingStatus;
	this->grossSqMRes			= source.grossSqMRes;
	this->grossSqMOffice		= source.grossSqMOffice;
	this->grossSqMRetail		= source.grossSqMRetail;
	this->grossSqMWarehouse     = source.grossSqMWarehouse;
	this->grossSqMIndustrial    = source.grossSqMIndustrial;
	this->grossSqMOther			= source.grossSqMOther;
	this->grossSqMCivic         = source.grossSqMCivic;
	this->lastChangedDate       = source.lastChangedDate;
	this->freehold              = source.freehold;
	this->floorSpace            = source.freehold;
	this->buildingType          = source.buildingType;
	this->slaAddressId          = source.slaAddressId;
}

Building& Building::operator=(const Building& source)
{
	this->fmBuildingId 			= source.fmBuildingId;
		this->fmProjectId			= source.fmProjectId;
		this->fmParcelId 			= source.fmParcelId;
		this->storeysAboveGround	= source.storeysAboveGround;
		this->storeysBelowGround  	= source.storeysBelowGround;
		this->fromDate				= source.fromDate;
		this->toDate				= source.toDate;
		this->buildingStatus		= source.buildingStatus;
		this->grossSqMRes			= source.grossSqMRes;
		this->grossSqMOffice		= source.grossSqMOffice;
		this->grossSqMRetail		= source.grossSqMRetail;
		this->grossSqMWarehouse     = source.grossSqMWarehouse;
		this->grossSqMIndustrial    = source.grossSqMIndustrial;
		this->grossSqMOther			= source.grossSqMOther;
		this->grossSqMCivic         = source.grossSqMCivic;
		this->lastChangedDate       = source.lastChangedDate;
		this->freehold              = source.freehold;
		this->floorSpace            = source.freehold;
		this->buildingType          = source.buildingType;
		this->slaAddressId          = source.slaAddressId;

    return *this;
}


BigSerial Building::getFmBuildingId() const
{
	return fmBuildingId;
}

BigSerial Building::getFmProjectId() const
{
	return fmProjectId;
}

BigSerial Building::getFmParcelId() const
{
	return fmParcelId;
}

int	Building::getStoreysAboveGround() const
{
	return storeysAboveGround;
}

int Building::getStoreysBelowGround() const
{
	return storeysBelowGround;
}

std::tm Building::getFromDate() const
{
	return fromDate;
}

std::tm Building::getToDate() const
{
	return toDate;
}

int Building::getBuildingStatus() const
{
	return buildingStatus;
}


float Building::getGrossSqmRes() const
{
	return grossSqMRes;
}

float Building::getGrossSqmOffice() const
{
	return grossSqMOffice;
}

float Building::getGrossSqmRetail() const
{
	return grossSqMRetail;
}

float Building::getGrossSqmOther() const
{
	return grossSqMOther;
}

std::tm Building::getLastChangedDate() const
{
	return lastChangedDate;
}
void Building::setBuildingStatus(int buildingStatus) {
	this->buildingStatus = buildingStatus;
}

void Building::setFmBuildingId(BigSerial fmBuildingId) {
	this->fmBuildingId = fmBuildingId;
}

void Building::setFmParcelId(BigSerial fmParcelId) {
	this->fmParcelId = fmParcelId;
}

void Building::setFmProjectId(BigSerial fmProjectId) {
	this->fmProjectId = fmProjectId;
}

void Building::setFromDate(const std::tm& fromDate) {
	this->fromDate = fromDate;
}

void Building::setGrossSqMOffice(float grossSqMOffice) {
	this->grossSqMOffice = grossSqMOffice;
}

void Building::setGrossSqMOther(float grossSqMOther) {
	this->grossSqMOther = grossSqMOther;
}

void Building::setGrossSqMRes(float grossSqMRes) {
	this->grossSqMRes = grossSqMRes;
}

void Building::setGrossSqMRetail(float grossSqMRetail) {
	this->grossSqMRetail = grossSqMRetail;
}

void Building::setStoreysAboveGround(int storeysAboveGround) {
	this->storeysAboveGround = storeysAboveGround;
}

void Building::setStoreysBelowGround(int storeysBelowGround) {
	this->storeysBelowGround = storeysBelowGround;
}

void Building::setToDate(const std::tm& toDate) {
	this->toDate = toDate;
}

const std::string& Building::getBuildingType() const
{
	return buildingType;
}

void Building::setBuildingType(const std::string& buildingType)
{
	this->buildingType = buildingType;
}

float Building::getFloorSpace() const
{
	return floorSpace;
}

void Building::setFloorSpace(float floorSpace)
{
	this->floorSpace = floorSpace;
}

int Building::getFreehold() const
{
	return freehold;
}

void Building::setFreehold(int freehold)
{
	this->freehold = freehold;
}

BigSerial Building::getSlaAddressId() const
{
	return slaAddressId;
}

void Building::setSlaAddressId(BigSerial slaAddressId)
{
	this->slaAddressId = slaAddressId;
}

float Building::getGrossSqMCivic() const
{
	return grossSqMCivic;
}

void Building::setGrossSqMCivic(float grossSqMCivic)
{
	this->grossSqMCivic = grossSqMCivic;
}

float Building::getGrossSqMIndustrial() const
{
	return grossSqMIndustrial;
}

void Building::setGrossSqMIndustrial(float grossSqMIndustrial)
{
	this->grossSqMIndustrial = grossSqMIndustrial;
}

float Building::getGrossSqMWarehouse() const
{
	return grossSqMWarehouse;
}

void Building::setGrossSqMWarehouse(float grossSqMWarehouse)
{
	this->grossSqMWarehouse = grossSqMWarehouse;
}

void Building::setLastChangedDate(const std::tm& lastChangedDate)
{
	this->lastChangedDate = lastChangedDate;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const Building& data)
        {
            return strm << "{"
						<< "\"fm_building_id \":\"" << data.fmBuildingId 	<< "\","
						<< "\"fm_project_id \":\"" 	<< data.fmProjectId 	<< "\","
						<< "\"fm_parcel_id \":\"" 	<< data.fmParcelId 	<< "\","
						<< "\"storeys_above_ground \":\"" << data.storeysAboveGround << "\","
						<< "\"storeys_below_ground \":\"" << data.storeysBelowGround << "\","
						<< "\"from_date \":\"" 	<< data.fromDate.tm_year  	<< data.fromDate.tm_wday 	<< data.fromDate.tm_mon << "\","
						<< "\"to_date \":\"" 	<< data.toDate.tm_year 	<< data.toDate.tm_mon 		<< data.toDate.tm_wday  << "\","
						<< "\"building_status \":\"" << data.buildingStatus 	<< "\","
						<< "\"gross_sq_m_res \":\"" 	<< data.grossSqMRes 	<< "\","
						<< "\"gross_sq_m_office \":\"" 	<< data.grossSqMOffice 	<< "\","
						<< "\"gross_sq_m_retail \":\"" 	<< data.grossSqMRetail 	<< "\","
						<< "\"gross_sq_m_other \":\"" 	<< data.grossSqMOther 	<< "\","
						<< "\"last_changed_date \":\"" 	<< data.lastChangedDate.tm_year  	<< data.lastChangedDate.tm_wday 	<< data.lastChangedDate.tm_mon << "\""
						<< "}";
        }
    }
}
