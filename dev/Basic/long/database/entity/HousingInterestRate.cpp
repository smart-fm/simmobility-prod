/*
 * HousingInterestRate.cpp
 *
 *  Created on: 4 May, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/HousingInterestRate.hpp>

using namespace sim_mob::long_term;

HousingInterestRate::HousingInterestRate(BigSerial _id,
										int _year,
										int _quarter,
										std::string _yq,
										float _infl_tminus1,
										float _infl_tplus1,
										float _interest_rate,
										float _gdp_growth,
										float _rate_real,
										std::string _source):  id(_id),  
																year(_year),
																yq(_yq),
																infl_tminus1(_infl_tminus1),
																infl_tplus1(_infl_tplus1),
																interest_rate(interest_rate),
																gdp_growth(_gdp_growth),
																rate_real(_rate_real),
																source(source){}

HousingInterestRate::~HousingInterestRate(){}

void HousingInterestRate::setInterestRate( float val)
{
	interest_rate = val;
}


void HousingInterestRate::setId( BigSerial val)
{
	id = val;
}

float  HousingInterestRate::getInterestRate() const
{
	return interest_rate;
}

BigSerial HousingInterestRate::getId() const
{
	return id;
}

int HousingInterestRate::getYear() const
{
	return year;
}

int HousingInterestRate::getQuarter() const
{
	return quarter;
}

std::string HousingInterestRate::getYq() const
{
	return yq;
}

float HousingInterestRate::getInfl_tminus1() const
{
	return infl_tminus1;
}

float HousingInterestRate::getInfl_tplus1() const
{
	return infl_tplus1;
}

float HousingInterestRate::getGdp_growth() const
{
	return gdp_growth;
}

float HousingInterestRate::getRate_real() const
{
	return rate_real;
}

std::string HousingInterestRate::getSource() const
{
	return source;
}


HousingInterestRate& HousingInterestRate::operator=(const HousingInterestRate& source)
{
	this->id = source.id;
	this->interest_rate = source.interest_rate;

	return *this;
}


namespace sim_mob
{
	namespace long_term
	{
		std::ostream& operator<<(std::ostream& strm, const HousingInterestRate& data)
		{
			return strm << "{" << "\"id \":\"" << data.id << "\","
						<< "\"interestRate \":\"" 		<< data.interest_rate << "\"" << "}";
		}
	}
}


