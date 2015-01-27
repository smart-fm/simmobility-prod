//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Property.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 3:05 PM
 */

#include "Unit.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

Unit::Unit( BigSerial id, BigSerial building_id, BigSerial sla_address_id, int unit_type, int storey_range, std::string unit_status, double floor_area, int storey,
			double rent, std::tm sale_from_date, std::tm physical_from_date, int sale_status, int physical_status, int biddingMarketEntryDay, int timeOnMarket, int timeOffMarket)
		   : id(id), building_id(building_id), sla_address_id(sla_address_id), unit_type(unit_type), storey_range(storey_range), unit_status(unit_status),
		     floor_area(floor_area), storey(storey), rent(rent), sale_from_date(sale_from_date), physical_from_date(physical_from_date), sale_status(sale_status),
		     physical_status(physical_status), biddingMarketEntryDay(biddingMarketEntryDay),timeOnMarket(timeOnMarket), timeOffMarket(timeOffMarket){}


Unit::Unit(const Unit& source)
{
    this->id  = source.id;
    this->building_id  = source.building_id;
    this->sla_address_id  = source.sla_address_id;
    this->unit_type  = source.unit_type;
    this->storey_range  = source.storey_range;
    this->unit_status  = source.unit_status;
    this->floor_area  = source.floor_area;
    this->storey  = source.storey;
    this->rent  = source.rent;
    this->sale_from_date  = source.sale_from_date;
    this->physical_from_date  = source.physical_from_date;
    this->sale_status  = source.sale_status;
    this->physical_status  = source.physical_status;
    this->biddingMarketEntryDay = source.biddingMarketEntryDay;
    this->timeOnMarket = source.timeOnMarket;
    this->timeOffMarket = source.timeOffMarket;
}

Unit::~Unit() {}

Unit& Unit::operator=(const Unit& source)
{
    this->id  = source.id;
    this->building_id  = source.building_id;
    this->sla_address_id  = source.sla_address_id;
    this->unit_type  = source.unit_type;
    this->storey_range  = source.storey_range;
    this->unit_status  = source.unit_status;
    this->floor_area  = source.floor_area;
    this->storey  = source.storey;
    this->rent  = source.rent;
    this->sale_from_date  = source.sale_from_date;
    this->physical_from_date  = source.physical_from_date;
    this->sale_status  = source.sale_status;
    this->physical_status  = source.physical_status;
    this->biddingMarketEntryDay = source.biddingMarketEntryDay;

    return *this;
}

BigSerial Unit::getId() const
{
    return id;
}

BigSerial Unit::getBuildingId() const
{
    return building_id;
}

BigSerial Unit::getSlaAddressId() const
{
    return sla_address_id;
}

int Unit::getUnitType() const
{
    return unit_type;
}

int Unit::getStoreyRange() const
{
	return storey_range;
}

std::string Unit::getUnitStatus() const
{
	return unit_status;
}

double Unit::getFloorArea() const
{
    return floor_area;
}

int Unit::getStorey() const
{
    return storey;
}

double Unit::getRent() const
{
    return rent;
}
std::tm Unit::getSaleFromDate() const
{
	return sale_from_date;
}

std::tm Unit::getPhysicalFromDate() const
{
	return physical_from_date;
}

int Unit::getSaleStatus() const
{
	return sale_status;
}

int Unit::getPhysicalStatus() const
{
	return physical_status;
}

void Unit::setBuildingId(BigSerial buildingId) {
	this->building_id = buildingId;
}

void Unit::setFloorArea(double floorArea) {
	this->floor_area = floorArea;
}

void Unit::setId(BigSerial id) {
	this->id = id;
}

void Unit::setPhysicalFromDate(const std::tm& physicalFromDate) {
	this->physical_from_date = physicalFromDate;
}

void Unit::setPhysicalStatus(int physicalStatus) {
	this->physical_status = physicalStatus;
}

void Unit::setRent(double rent) {
	this->rent = rent;
}

void Unit::setSaleFromDate(const std::tm& saleFromDate) {
	this->sale_from_date = saleFromDate;
}

void Unit::setSaleStatus(int saleStatus) {
	this->sale_status = saleStatus;
}

void Unit::setSlaAddressId(BigSerial slaAddressId) {
	this->sla_address_id = slaAddressId;
}

void Unit::setStorey(int storey) {
	this->storey = storey;
}

void Unit::setStoreyRange(int storeyRange) {
	this->storey_range = storeyRange;
}

void Unit::setUnitStatus(const std::string& unitStatus) {
	this->unit_status = unitStatus;
}

void Unit::setUnitType(int unitType) {
	this->unit_type = unitType;
}

int Unit::getbiddingMarketEntryDay() const
{
	return biddingMarketEntryDay;
}

void Unit::setbiddingMarketEntryDay( int day )
{
	biddingMarketEntryDay = day;
}

int  Unit::getTimeOnMarket() const
{
	return timeOnMarket;
}

void Unit::setTimeOnMarket(int day )
{
	timeOnMarket = day;
}

int  Unit::getTimeOffMarket() const
{
	return timeOffMarket;
}

void Unit::setTimeOffMarket(int day )
{
	timeOffMarket = day;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const Unit& data)
        {
            return strm << "{"
						<< "\"id \":\"" << data.id << "\","
						<< "\"building_id \":\"" << data.building_id << "\","
						<< "\"sla_address_id \":\"" << data.sla_address_id << "\","
						<< "\"unit_type \":\"" << data.unit_type << "\","
						<< "\"storey_range \":\"" << data.storey_range << "\","
						<< "\"unit_status \":\"" << data.unit_status << "\","
						<< "\"floor_area \":\"" << data.floor_area << "\","
						<< "\"storey \":\"" << data.storey << "\","
						<< "\"rent \":\"" << data.rent << "\","
						<< "\"sale_from_date \":\"" << data.sale_from_date.tm_year << data.sale_from_date.tm_mon << data.sale_from_date.tm_wday << "\","
						<< "\"physical_from_date \":\"" << data.physical_from_date.tm_year << data.physical_from_date.tm_mon << data.physical_from_date.tm_wday << "\","
						<< "\"sale_status \":\"" << data.sale_status << "\","
						<< "\"physical_status \":\"" << data.physical_status << "\","
						<< "\"biddingMarketEntryDay\":\"" << data.biddingMarketEntryDay << "\""
						<< "}";
        }
    }
}
