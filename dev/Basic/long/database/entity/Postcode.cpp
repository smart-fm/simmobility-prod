//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Postcode.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Feb 11, 2014, 3:05 PM
 */

#include "Postcode.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

Postcode::Postcode( BigSerial address_id, std::string sla_postcode, BigSerial taz_id, float longitude, float latitude, bool primary_postcode ):
					address_id(address_id), sla_postcode( sla_postcode), taz_id(taz_id), longitude(longitude), latitude(latitude),
					primary_postcode(primary_postcode){}
            
Postcode::Postcode(const Postcode& source)
{
	this->address_id = source.address_id;
	this->sla_postcode = source.sla_postcode;
	this->taz_id = source.taz_id;
	this->longitude = source.longitude;
	this->latitude = source.latitude;
	this->primary_postcode = source.primary_postcode;
}

Postcode::~Postcode() {}

Postcode& Postcode::operator=(const Postcode& source)
{
	this->address_id = source.address_id;
	this->sla_postcode = source.sla_postcode;
	this->taz_id = source.taz_id;
	this->longitude = source.longitude;
	this->latitude = source.latitude;
	this->primary_postcode = source.primary_postcode;

    return *this;
}

BigSerial Postcode::getAddressId() const
{
	return address_id;
}

std::string Postcode::getSlaPostcode() const
{
	return sla_postcode;
}

BigSerial Postcode::getTazId() const
{
	return taz_id;
}


float Postcode::getLongitude() const
{
	return longitude;
}

float Postcode::getLatitude() const
{
	return latitude;
}

bool Postcode::getPrimaryPostcode() const
{
	return primary_postcode;
}

void Postcode::setAddressId(BigSerial addressId)
{
	address_id = addressId;
}

void Postcode::setLatitude(float latitude)
{
	this->latitude = latitude;
}

void Postcode::setLongitude(float longitude)
{
	this->longitude = longitude;
}

void Postcode::setPrimaryPostcode(bool primaryPostcode)
{
	primary_postcode = primaryPostcode;
}

void Postcode::setSlaPostcode(const std::string& slaPostcode)
{
	sla_postcode = slaPostcode;
}

void Postcode::setTazId(BigSerial tazId)
{
	taz_id = tazId;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const Postcode& data) {
            return strm << "{"
            		<< "\"address_id \":\"" << data.address_id << "\","
            		<< "\"sla_postcode \":\"" << data.sla_postcode << "\","
            		<< "\"taz_id \":\"" << data.taz_id << "\","
            		<< "\"longitude \":\"" << data.longitude << "\","
            		<< "\"latitude \":\"" << data.latitude << "\","
            		<< "\"primary_postcode \":\"" << data.primary_postcode << "\""
                    << "}";
        }
    }
}
