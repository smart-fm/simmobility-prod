/*
 * IndvidualVehicleOwnershipLogsum.cpp
 *
 *  Created on: Jan 20, 2016
 *      Author: gishara
 */


#include "IndvidualVehicleOwnershipLogsum.hpp"

using namespace sim_mob::long_term;

IndvidualVehicleOwnershipLogsum::IndvidualVehicleOwnershipLogsum(BigSerial householdId, BigSerial individualId, double logsumTransit, double logsumCar):
		householdId(householdId),individualId(individualId),logsumTransit(logsumTransit),logsumCar(logsumCar){}

IndvidualVehicleOwnershipLogsum::~IndvidualVehicleOwnershipLogsum() {}

IndvidualVehicleOwnershipLogsum& IndvidualVehicleOwnershipLogsum::operator=(const IndvidualVehicleOwnershipLogsum& source)
{
	this->householdId 			= source.householdId;
	this->individualId = source.individualId;
	this->logsumTransit = source.logsumTransit;
	this->logsumCar = source.logsumCar;
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

double IndvidualVehicleOwnershipLogsum::getLogsumCar() const
{
	return logsumCar;
}

void IndvidualVehicleOwnershipLogsum::setLogsumCar(double logsumCar)
{
	this->logsumCar = logsumCar;
}

double IndvidualVehicleOwnershipLogsum::getLogsumTransit() const
{
	return logsumTransit;
}

void IndvidualVehicleOwnershipLogsum::setLogsumTransit(double logsumTransit)
{
	this->logsumTransit = logsumTransit;
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
						<< "\"logsumTransit \":\"" 	<< data.logsumTransit 	<< "\","
						<< "\"logsumCar \":\"" 	<< data.logsumCar 	<< "\","
						<< "}";
        }
    }
}





