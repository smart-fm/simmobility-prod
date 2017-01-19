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

Establishment::Establishment(BigSerial id, BigSerial firmId,
  			     BigSerial buildingId,
			     BigSerial lifestyleId,
			     double revenue)
			    :id(id), firmId(firmId), buildingId(buildingId), lifestyleId(lifestyleId), revenue(revenue){}

Establishment::~Establishment(){}

Establishment::Establishment(const Establishment& source)
{
	this->id 			= source.id;
	this->firmId 		= source.firmId;
	this->buildingId 	= source.buildingId;
	this->lifestyleId 	= source.lifestyleId;
	this->revenue 		= source.revenue;
}


Establishment& Establishment::operator=(const Establishment& source)
{
	this->id 			= source.id;
	this->firmId 		= source.firmId;
	this->buildingId 	= source.buildingId;
	this->lifestyleId 	= source.lifestyleId;
	this->revenue 		= source.revenue;

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

void Establishment::setRevenue(double val)
{
	revenue = val;
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

double Establishment::getRevenue() const
{
	return revenue;
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
						<< "\"revenue \":\"" 		<< data.revenue 		<< "\","
						<< "}";
        }
    }
}



