//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * Establishment.cpp
 *
 *  Created on: 23 April, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include "Establishment.hpp"

using namespace sim_mob::long_term;

Establishment::Establishment(BigSerial 	id,	BigSerial buildingId,BigSerial firmId,BigSerial	firmFoundationYear,int	industryTypeId,
							 double floorArea,BigSerial jobSize,double revenue,double capital,BigSerial	establishmentLifestyleId):
							id(id),buildingId(buildingId),	firmId(firmId),firmFoundationYear(firmFoundationYear),industryTypeId(industryTypeId),
							floorArea(floorArea),jobSize(jobSize),revenue(revenue),capital(capital),establishmentLifestyleId(establishmentLifestyleId){}

Establishment::~Establishment(){}

Establishment::Establishment(const Establishment& source)
{
	this->id = source.id;
	this->buildingId = source.buildingId;
	this->firmId = source.firmId;
	this->firmFoundationYear = source.firmFoundationYear;
	this->industryTypeId = source.industryTypeId;
	this->floorArea = source.floorArea;
	this->jobSize = source.jobSize;
	this->revenue = source.revenue;
	this->capital = source.capital;
	this->establishmentLifestyleId = source.establishmentLifestyleId;
}


Establishment& Establishment::operator=(const Establishment& source)
{
	this->id = source.id;
	this->buildingId = source.buildingId;
	this->firmId = source.firmId;
	this->firmFoundationYear = source.firmFoundationYear;
	this->industryTypeId = source.industryTypeId;
	this->floorArea = source.floorArea;
	this->jobSize = source.jobSize;
	this->revenue = source.revenue;
	this->capital = source.capital;
	this->establishmentLifestyleId = source.establishmentLifestyleId;

	return *this;
}

void Establishment::setId(BigSerial val)
{
	id = val;
}

void Establishment::setBuildingId(BigSerial val)
{
	buildingId = val;
}

void Establishment::setFirmId(BigSerial val)
{
	firmId = val;
}

void Establishment::setFirmFoundationYear(BigSerial val)
{
	firmFoundationYear = val;
}

void Establishment::setIndustryTypeId(int val)
{
	industryTypeId = val;
}

void Establishment::setFloorArea(double val)
{
	floorArea = val;
}

void Establishment::setJobSize(BigSerial  val)
{
	jobSize = val;
}


void Establishment::setRevenue(double val)
{
	revenue = val;
}

void Establishment::setEstablishmentLifestyleId(BigSerial val)
{
	establishmentLifestyleId = val;
}

BigSerial Establishment::getId() const
{
	return id;
}

BigSerial Establishment::getBuildingId() const
{
	return buildingId;
}

BigSerial Establishment::getFirmId() const
{
	return firmId;
}

BigSerial Establishment::getFirmFoundationYear() const
{
	return firmFoundationYear;
}

int	Establishment::getIndustryTypeId() const
{
	return industryTypeId;
}


double Establishment::getFloorArea() const
{
	return floorArea;
}

BigSerial Establishment::getJobSize() const
{
	return jobSize;
}

double Establishment::getRevenue() const
{
	return revenue;
}

BigSerial Establishment::getEstablishmentLifestyleId() const
{
	return establishmentLifestyleId;
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
						<< "\"revenue \":\"" 		<< data.revenue 		<< "\","
						<< "}";
        }
    }
}



