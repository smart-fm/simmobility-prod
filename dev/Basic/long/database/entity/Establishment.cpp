/*
 * Establishment.cpp
 *
 *  Created on: 23 April, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include "Establishment.hpp"

using namespace sim_mob::long_term;

Establishment::Establishment(BigSerial id, BigSerial firmId, BigSerial buildingId, BigSerial lifestyleId, BigSerial businessTypeId, int size, double revenue, double grossSqM, BigSerial slaAddressId)
							:id(id), firmId(firmId), buildingId(buildingId), lifestyleId(lifestyleId), businessTypeId(businessTypeId), size(size), revenue(revenue), grossSqM(grossSqM), slaAddressId(slaAddressId){}

Establishment::~Establishment(){}

Establishment& Establishment::operator=(const Establishment& source)
{
	this->id 			= source.id;
	this->firmId 		= source.firmId;
	this->buildingId 	= source.buildingId;
	this->lifestyleId 	= source.lifestyleId;
	this->businessTypeId = source.businessTypeId;
	this->size 			= source.size;
	this->revenue 		= source.revenue;
	this->grossSqM 		= source.grossSqM;
	this->slaAddressId 	= source.slaAddressId;

	return *this;
}

void Establishment::setId(BigSerial val)
{
	id = val;
}

void Establishment::setFirmId(BigSerial val)
{
	firmId = val;
}

void Establishment::setBuildingId(BigSerial val)
{
	buildingId = val;
}

void Establishment::setLifestyleId(BigSerial val)
{
	lifestyleId = val;
}

void Establishment::setBusinessTypeId(BigSerial val)
{
	businessTypeId = val;
}

void Establishment::setSize(int val)
{
	size = val;
}

void Establishment::setRevenue(double val)
{
	revenue = val;
}

void Establishment::setGrossSqM(double val)
{
	grossSqM = val;
}

void Establishment::setSlaAddressId(BigSerial val)
{
	slaAddressId = val;
}

BigSerial Establishment::getId() const
{
	return id;
}

BigSerial Establishment::getFirmId() const
{
	return firmId;
}

BigSerial Establishment::getBuildingId() const
{
	return buildingId;
}

BigSerial Establishment::getLifestyleId() const
{
	return lifestyleId;
}

BigSerial Establishment::getBusinessTypeId() const
{
	return businessTypeId;
}

int Establishment::getSize() const
{
	return size;
}

double Establishment::getRevenue() const
{
	return revenue;
}

double Establishment::getGrossSqM() const
{
	return grossSqM;
}

BigSerial Establishment::getSlaAddressId() const
{
	return slaAddressId;
}


namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const Establishment& data)
        {
            return strm << "{"
						<< "\"id \":\"" 			<< data.id 				<< "\","
						<< "\"firmId \":\"" 		<< data.firmId 			<< "\","
						<< "\"buildingId \":\"" 	<< data.buildingId 		<< "\","
						<< "\"lifestyleId \":\"" 	<< data.lifestyleId 	<< "\","
						<< "\"businessTypeId \":\"" << data.businessTypeId 	<< "\","
						<< "\"size \":\"" 			<< data.size 			<< "\","
						<< "\"revenue \":\"" 		<< data.revenue 		<< "\","
						<< "\"grossSqM \":\"" 		<< data.grossSqM 		<< "\","
						<< "\"slaAddressId \":\"" 	<< data.slaAddressId 	<< "\""
						<< "}";
        }
    }
}



