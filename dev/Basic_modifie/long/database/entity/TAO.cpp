/*
 * TAO.cpp
 *
 *  Created on: Jul 30, 2015
 *      Author: gishara
 */
#include "TAO.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

TAO::TAO(BigSerial id, std::string quarter, double condo, double apartment, double terrace, double semi, double detached, double ec, double hdb12, double hdb3, double hdb4, double hdb5, double exec):
		id(id),quarter(quarter),condo(condo),apartment(apartment),terrace(terrace), semi(semi),detached(detached), ec(ec), hdb12(hdb12), hdb3(hdb3), hdb4(hdb4), hdb5(hdb5), exec(exec) {}

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
	this->hdb12 = source.hdb12;
	this->hdb3 = source.hdb3;
	this->hdb4 = source.hdb4;
	this->hdb5 = source.hdb5;
	this->exec = source.exec;

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


double TAO::getHdb12() const
{
	return hdb12;
}
double TAO::getHdb3() const
{
	return hdb3;
}
double TAO::getHdb4() const
{
	return hdb4;
}
double TAO::getHdb5() const
{
	return hdb5;
}
double TAO::getExec() const
{
	return exec;
}


void TAO::setHdb12(double value)
{
	hdb12 = value;
}

void TAO::setHdb3(double value)
{
	hdb3 = value;
}

void TAO::setHdb4(double value)
{
	hdb4 = value;
}

void TAO::setHdb5(double value)
{
	hdb5 = value;
}

void TAO::setExec(double value)
{
	exec = value;
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
					<< "\" hdb12\":\"" << data.hdb12 << "\""
					<< "\" hdb3\":\"" << data.hdb4 << "\""
					<< "\" hdb4\":\"" << data.hdb4 << "\""
					<< "\" hdb5\":\"" << data.hdb5 << "\""
					<< "\" exec\":\"" << data.exec << "\""
                    << "}";
        }
    }
}


