/*
 * HousingInterestRate.cpp
 *
 *  Created on: 4 May, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/HousingInterestRate.hpp>

using namespace sim_mob::long_term;

HousingInterestRate::HousingInterestRate(BigSerial id, std::tm from_date, std::tm to_date, float interestRate):id(id), from_date(from_date), to_date(to_date),
										 interestRate(interestRate){}

HousingInterestRate::~HousingInterestRate(){}

void HousingInterestRate::setInterestRate( float val)
{
	interestRate = val;
}

void HousingInterestRate::setFromDate( std::tm val)
{
	from_date = val;
}

void HousingInterestRate::setToDate( std::tm val)
{
	to_date = val;
}

void HousingInterestRate::setId( BigSerial val)
{
	id = val;
}

float  HousingInterestRate::getInterestRate() const
{
	return interestRate;
}

std::tm  HousingInterestRate::getFromDate() const
{
	return from_date;
}

std::tm	 HousingInterestRate::getToDate() const
{
	return to_date;
}

BigSerial HousingInterestRate::getId() const
{
	return id;
}


HousingInterestRate& HousingInterestRate::operator=(const HousingInterestRate& source)
{
	this->id = source.id;
	this->interestRate = source.interestRate;
	this->from_date = source.from_date;
	this->to_date = source.to_date;

	return *this;
}


namespace sim_mob
{
	namespace long_term
	{
		std::ostream& operator<<(std::ostream& strm, const HousingInterestRate& data)
		{
			return strm << "{" << "\"id \":\"" << data.id << "\","
						<< "{" << "\"from_date \":\"" << data.from_date.tm_year << " " << data.from_date.tm_mon << " " << data.from_date.tm_mday << "\","
						<< "{" << "\"to_date \":\"" << data.to_date.tm_year << " " << data.to_date.tm_mon << " " << data.to_date.tm_mday << "\","
						<< "\"interestRate \":\"" 		<< data.interestRate << "\"" << "}";
		}
	}
}


