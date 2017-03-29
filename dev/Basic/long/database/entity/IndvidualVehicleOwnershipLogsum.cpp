/*
 * IndvidualVehicleOwnershipLogsum.cpp
 *
 *  Created on: Jan 20, 2016
 *      Author: gishara
 */


#include "IndvidualVehicleOwnershipLogsum.hpp"

using namespace sim_mob::long_term;

IndvidualVehicleOwnershipLogsum::IndvidualVehicleOwnershipLogsum(BigSerial householdId, BigSerial individualId, double logsum0, double logsum1, double logsum2, double logsum3, double logsum4, double logsum5):
		householdId(householdId),individualId(individualId),logsum0(logsum0),logsum1(logsum1),logsum2(logsum2),logsum3(logsum3),logsum4(logsum4),logsum5(logsum5){}

IndvidualVehicleOwnershipLogsum::~IndvidualVehicleOwnershipLogsum() {}

IndvidualVehicleOwnershipLogsum& IndvidualVehicleOwnershipLogsum::operator=(const IndvidualVehicleOwnershipLogsum& source)
{
	this->householdId 			= source.householdId;
	this->individualId = source.individualId;
	this->logsum0 = source.logsum0;
	this->logsum1 = source.logsum1;
	this->logsum2 = source.logsum2;
	this->logsum3 = source.logsum3;
	this->logsum4 = source.logsum4;
	this->logsum5 = source.logsum5;
	return *this;
}

BigSerial IndvidualVehicleOwnershipLogsum::getHouseholdId() const
{
	return householdId;
}

void IndvidualVehicleOwnershipLogsum::setHouseholdId(BigSerial householdId)
{
	this->householdId = householdId;
}

BigSerial IndvidualVehicleOwnershipLogsum::getIndividualId() const
{
	return individualId;
}

void IndvidualVehicleOwnershipLogsum::setIndividualId(BigSerial individualId)
{
	this->individualId = individualId;
}

double IndvidualVehicleOwnershipLogsum::getLogsum0() const
{
	return logsum0;
}

void IndvidualVehicleOwnershipLogsum::setLogsum0(double logsum0)
{
	this->logsum0 = logsum0;
}

double IndvidualVehicleOwnershipLogsum::getLogsum1() const
{
	return logsum1;
}

void IndvidualVehicleOwnershipLogsum::setLogsum1(double logsum1)
{
	this->logsum1 = logsum1;
}

double IndvidualVehicleOwnershipLogsum::getLogsum2() const
{
	return logsum2;
}

void IndvidualVehicleOwnershipLogsum::setLogsum2(double logsum2)
{
	this->logsum2 = logsum2;
}

double IndvidualVehicleOwnershipLogsum::getLogsum3() const
{
	return logsum3;
}

void IndvidualVehicleOwnershipLogsum::setLogsum3(double logsum3)
{
	this->logsum3 = logsum3;
}

double IndvidualVehicleOwnershipLogsum::getLogsum4() const
{
	return logsum4;
}

void IndvidualVehicleOwnershipLogsum::setLogsum4(double logsum4)
{
	this->logsum4 = logsum4;
}

double IndvidualVehicleOwnershipLogsum::getLogsum5() const
{
	return logsum5;
}

void IndvidualVehicleOwnershipLogsum::setLogsum5(double logsum5)
{
	this->logsum5 = logsum5;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const IndvidualVehicleOwnershipLogsum& data)
        {
            return strm << "{"
						<< "\"householdId \":\"" << data.householdId 	<< "\","
						<< "\"individualId \":\"" 	<< data.individualId 	<< "\","
						<< "\"logsum0 \":\"" 	<< data.logsum0 	<< "\","
						<< "\"logsum1 \":\"" 	<< data.logsum1 	<< "\","
						<< "\"logsum2 \":\"" 	<< data.logsum2 	<< "\","
						<< "\"logsum3 \":\"" 	<< data.logsum3 	<< "\","
						<< "\"logsum4 \":\"" 	<< data.logsum4 	<< "\","
						<< "\"logsum5 \":\"" 	<< data.logsum5 	<< "\","
						<< "}";
        }
    }
}





