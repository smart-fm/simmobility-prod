/*
 * LogSumVehicleOwnership.cpp
 *
 *  Created on: May 21, 2015
 *      Author: gishara
 */

#include "LogSumVehicleOwnership.hpp"

using namespace sim_mob::long_term;

LogSumVehicleOwnership::LogSumVehicleOwnership(BigSerial householdId,double avgLogsum):
		householdId(householdId),avgLogsum(avgLogsum) {}

LogSumVehicleOwnership::~LogSumVehicleOwnership() {}

LogSumVehicleOwnership& LogSumVehicleOwnership::operator=(const LogSumVehicleOwnership& source)
{
	this->householdId 			= source.householdId;
	this->avgLogsum	= source.avgLogsum;
    return *this;
}

BigSerial LogSumVehicleOwnership::getHouseholdId() const
{
		return this->householdId;
}

double LogSumVehicleOwnership::getAvgLogsum() const
{
		return this->avgLogsum;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const LogSumVehicleOwnership& data)
        {
            return strm << "{"
						<< "\"householdId \":\"" << data.householdId 	<< "\","
						<< "\"avgLogsum \":\"" 	<< data.avgLogsum 	<< "\","
						<< "}";
        }
    }
}



