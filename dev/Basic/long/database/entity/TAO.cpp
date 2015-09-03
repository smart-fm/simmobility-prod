/*
 * TAO.cpp
 *
 *  Created on: Jul 30, 2015
 *      Author: gishara
 */
#include "TAO.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

TAO::TAO(BigSerial id, std::string quarter, double condo, double apartment, double terrace, double semi, double detached, double ec):
		id(id),quarter(quarter),condo(condo),apartment(apartment),terrace(terrace), semi(semi),detached(detached), ec(ec) {}

TAO::~TAO() {}

TAO& TAO::operator=(const TAO& source)
{
	this->id = source.id;
	this->quarter = source.quarter;
	this->condo	= source.condo;
	this->apartment = source.apartment;
	this->terrace = source.terrace;
	this->semi = source.semi;
	this->detached = source.detached;
	this->ec = source.ec;

    return *this;
}

double TAO::getApartment() const
{
		return apartment;
}

void TAO::setApartment(double apartment)
{
		this->apartment = apartment;
}

double TAO::getCondo() const
{
		return condo;
}

void TAO::setCondo(double condo) {
	this->condo = condo;
}

double TAO::getDetached() const {
	return detached;
}

void TAO::setDetached(double detached) {
	this->detached = detached;
}

double TAO::getEc() const {
	return ec;
}

void TAO::setEc(double ec) {
	this->ec = ec;
}

const std::string& TAO::getQuarter() {
	return quarter;
}

void TAO::setQuarter(const std::string& quarter) {
	this->quarter = quarter;
}

double TAO::getSemi() const {
	return semi;
}

void TAO::setSemi(double semi) {
	this->semi = semi;
}

double TAO::getTerrace() const {
	return terrace;
}

void TAO::setTerrace(double terrace) {
	this->terrace = terrace;
}

BigSerial TAO::getId() const
{
	return id;
}

void TAO::setId(BigSerial id)
{
	this->id = id;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const TAO& data) {
            return strm << "{"
            		<< "\"id\":\"" << data.id<< "\","
                    << "\"quarter\":\"" << data.quarter<< "\","
                    << "\"condo\":\"" << data.condo << "\","
                    << "\" apartment\":\"" << data.apartment << "\""
                    << "\" terrace\":\"" << data.terrace << "\""
                    << "\" semi\":\"" << data.semi << "\""
                    << "\" detached\":\"" << data.detached << "\""
                    << "\" ec\":\"" << data.ec << "\""
                    << "}";
        }
    }
}


