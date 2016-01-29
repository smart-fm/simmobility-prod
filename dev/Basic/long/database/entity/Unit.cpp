//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Property.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <rogbeer@mit.edu>
 * 
 * Created on March 11, 2013, 3:05 PM
 */

#include "Unit.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

Unit::Unit( BigSerial id, BigSerial building_id, BigSerial sla_address_id, int unit_type, int storey_range, int constructionStatus, double floor_area, int storey,
			double monthlyRent, int ownership, std::tm sale_from_date, std::tm physical_from_date, int sale_status, int occupancyStatus, std::tm lastChangedDate,
			double totalPrice,std::tm valueDate,int tenureStatus,int biddingMarketEntryDay, int timeOnMarket, int timeOffMarket, double lagCoefficient, int zoneHousingType, int dwellingType)
		   : id(id), building_id(building_id), sla_address_id(sla_address_id), unit_type(unit_type), storey_range(storey_range), constructionStatus(constructionStatus),
		     floor_area(floor_area), storey(storey), monthlyRent(monthlyRent), ownership(ownership), sale_from_date(sale_from_date), physical_from_date(physical_from_date),
			 sale_status(sale_status), occupancyStatus(occupancyStatus), lastChangedDate(lastChangedDate),totalPrice(totalPrice),valueDate(valueDate),tenureStatus(tenureStatus),
			 biddingMarketEntryDay(biddingMarketEntryDay),timeOnMarket(timeOnMarket), timeOffMarket(timeOffMarket), lagCoefficient(lagCoefficient),
			 zoneHousingType(zoneHousingType), dwellingType(dwellingType){}


Unit::Unit(const Unit& source)
{
    this->id  = source.id;
    this->building_id  = source.building_id;
    this->sla_address_id  = source.sla_address_id;
    this->unit_type  = source.unit_type;
    this->storey_range  = source.storey_range;
    this->constructionStatus  = source.constructionStatus;
    this->floor_area  = source.floor_area;
    this->storey  = source.storey;
    this->monthlyRent  = source.monthlyRent;
    this->ownership = source.ownership;
    this->sale_from_date  = source.sale_from_date;
    this->physical_from_date  = source.physical_from_date;
    this->sale_status  = source.sale_status;
    this->occupancyStatus = source.occupancyStatus;
    this->lastChangedDate = source.lastChangedDate;
    this->totalPrice = source.totalPrice;
    this->valueDate = source.valueDate;
    this->tenureStatus = source.tenureStatus;
    this->biddingMarketEntryDay = source.biddingMarketEntryDay;
    this->timeOnMarket = source.timeOnMarket;
    this->timeOffMarket = source.timeOffMarket;
    this->lagCoefficient = source.lagCoefficient;
    this->zoneHousingType = source.zoneHousingType;
    this->dwellingType = source.dwellingType;

}

Unit::~Unit() {}

Unit& Unit::operator=(const Unit& source)
{
    this->id  = source.id;
    this->building_id  = source.building_id;
    this->sla_address_id  = source.sla_address_id;
    this->unit_type  = source.unit_type;
    this->storey_range  = source.storey_range;
    this->constructionStatus  = source.constructionStatus;
    this->floor_area  = source.floor_area;
    this->storey  = source.storey;
    this->monthlyRent  = source.monthlyRent;
    this->ownership = source.ownership;
    this->sale_from_date  = source.sale_from_date;
    this->physical_from_date  = source.physical_from_date;
    this->sale_status  = source.sale_status;
    this->occupancyStatus = source.occupancyStatus;
    this->lastChangedDate = source.lastChangedDate;
    this->totalPrice = source.totalPrice;
    this->valueDate = source.valueDate;
    this->tenureStatus = source.tenureStatus;
    this->biddingMarketEntryDay = source.biddingMarketEntryDay;
    this->timeOnMarket = source.timeOnMarket;
    this->timeOffMarket = source.timeOffMarket;
    this->lagCoefficient = source.lagCoefficient;
    this->zoneHousingType = source.zoneHousingType;
    this->dwellingType = source.dwellingType;

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

int Unit::getConstructionStatus() const
{
	return constructionStatus;
}

double Unit::getFloorArea() const
{
    return floor_area;
}

int Unit::getStorey() const
{
    return storey;
}

double Unit::getMonthlyRent() const
{
		return monthlyRent;
}

int Unit::getOwnership() const
{
	return ownership;
}

std::tm Unit::getSaleFromDate() const
{
	return sale_from_date;
}

std::tm Unit::getPhysicalFromDate() const
{
	return physical_from_date;
}

int Unit::getPhysicalFromYear() const
{
	return physical_from_date.tm_year;
}


int Unit::getSaleStatus() const
{
	return sale_status;
}

std::tm Unit::getLastChangedDate() const
{
	return lastChangedDate;
}

int Unit::getDwellingType() const
{
	return dwellingType;
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

void Unit::setMonthlyRent(double monthlyRent)
{
		this->monthlyRent = monthlyRent;
}

void Unit::setOwnership(int ownership)
{
	this->ownership = ownership;
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

void Unit::setConstructionStatus(int unitStatus) {
	this->constructionStatus = unitStatus;
}

void Unit::setUnitType(int unitType) {
	this->unit_type = unitType;
}

void Unit::setLastChangedDate(std::tm date)
{
	this->lastChangedDate = date;
}

int Unit::getOccupancyStatus() const
{
		return occupancyStatus;
}

void Unit::setOccupancyStatus(int occStatus)
{
		this->occupancyStatus = occStatus;
}

int Unit::getTenureStatus() const
{
		return tenureStatus;
}

void Unit::setTenureStatus(int tenureStatus)
{
		this->tenureStatus = tenureStatus;
}

double Unit::getTotalPrice() const
{
		return totalPrice;
}

void Unit::setTotalPrice(double totalPrice)
{
		this->totalPrice = totalPrice;
}

const std::tm& Unit::getValueDate() const
{
		return valueDate;
}

void Unit::setValueDate(const std::tm& valueDate)
{
		this->valueDate = valueDate;
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

void Unit::setLagCoefficient(double lag)
{
	lagCoefficient = lag;
}
double Unit::getLagCoefficient() const
{
	return lagCoefficient;
}

int Unit::getZoneHousingType() const
{
	return zoneHousingType;
}

void Unit::setZoneHousingType(int value)
{
	zoneHousingType = value;
}

void Unit::setDwellingType( int value)
{
	dwellingType = value;
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
						<< "\"unit_status \":\"" << data.constructionStatus << "\","
						<< "\"floor_area \":\"" << data.floor_area << "\","
						<< "\"storey \":\"" << data.storey << "\","
						<< "\"monthlyRent \":\"" << data.monthlyRent<< "\","
						<< "\"ownership \":\"" << data.ownership << "\","
						<< "\"sale_from_date \":\"" << data.sale_from_date.tm_year << data.sale_from_date.tm_mon << data.sale_from_date.tm_wday << "\","
						<< "\"physical_from_date \":\"" << data.physical_from_date.tm_year << data.physical_from_date.tm_mon << data.physical_from_date.tm_wday << "\","
						<< "\"sale_status \":\"" << data.sale_status << "\","
						<< "\"occupancy_status \":\"" << data.occupancyStatus << "\","
						<< "\"biddingMarketEntryDay\":\"" << data.biddingMarketEntryDay << "\""
						<< "\"timeOnMarket\":\"" << data.timeOnMarket << "\""
            			<< "\"timeOffMarket\":\"" << data.timeOffMarket << "\""
						<< "\"lastChangedDate\":\"" << data.lastChangedDate.tm_year << data.lastChangedDate.tm_mon << data.lastChangedDate.tm_wday << "\","
						<< "\"lagCoefficient\":\"" << data.lagCoefficient << "\","
						<< "\"zoneHousingType\":\"" << data.zoneHousingType << "\","
						<< "\"dwellingType\":\"" << data.dwellingType << "\""
						<< "}";
        }
    }
}
