/*
 * LogSumVehicleOwnership.cpp
 *
 *  Created on: May 21, 2015
 *      Author: gishara
 */

#include "LogSumVehicleOwnership.hpp"

using namespace sim_mob::long_term;

LogSumVehicleOwnership::LogSumVehicleOwnership(BigSerial householdId,double logsum):
		householdId(householdId),logsum(logsum) {}

LogSumVehicleOwnership::~LogSumVehicleOwnership() {}

LogSumVehicleOwnership& LogSumVehicleOwnership::operator=(const LogSumVehicleOwnership& source)
{
	this->householdId 			= source.householdId;
	this->logsum	= source.logsum;
    return *this;
}

BigSerial LogSumVehicleOwnership::getHouseholdId() const
{
		return this->householdId;
}

double LogSumVehicleOwnership::getLogsum() const
{
		return this->logsum;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const LogSumVehicleOwnership& data)
        {
            return strm << "{"
						<< "\"householdId \":\"" << data.householdId 	<< "\","
						<< "\"avgLogsum \":\"" 	<< data.logsum 	<< "\","
						<< "}";
        }
    }
}



