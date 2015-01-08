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
					std::tm fromDate, std::tm toDate, std::string buildingStatus, float	grossSqMRes, float grossSqMOffice,
					float grossSqMRetail, float grossSqMOther) :
					fmBuildingId(fmBuildingId), fmProjectId(fmProjectId), fmParcelId(fmParcelId), storeysAboveGround(storeysAboveGround),
					storeysBelowGround(storeysBelowGround), fromDate(fromDate), toDate(toDate), buildingStatus(buildingStatus),grossSqMRes(grossSqMRes),
					grossSqMOffice(grossSqMOffice), grossSqMRetail(grossSqMRetail), grossSqMOther(grossSqMOther){}

Building::~Building() {}

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
	this->grossSqMOther			= source.grossSqMOther;

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

std::string Building::getBuildingStatus() const
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

void Building::setBuildingStatus(const std::string& buildingStatus) {
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
						<< "\"gross_sq_m_other \":\"" 	<< data.grossSqMOther 	<< "\""
						<< "}";
        }
    }
}
