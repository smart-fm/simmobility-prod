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
                                                                source(_source){}

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


HousingInterestRate& HousingInterestRate::operator=(const HousingInterestRate& sourcerhs)
{
    this->id = sourcerhs.id;
    this->interest_rate = sourcerhs.interest_rate;
    this->id  = sourcerhs.id;
    this->year = sourcerhs.year;
    this->quarter  = sourcerhs.quarter;
    this->yq  = sourcerhs.yq;
    this->infl_tminus1 = sourcerhs.infl_tminus1;
    this->infl_tplus1 = sourcerhs.infl_tplus1;
    this->interest_rate = sourcerhs.interest_rate;
    this->gdp_growth = sourcerhs.gdp_growth;
    this->rate_real = sourcerhs.rate_real;
    this->source = sourcerhs.source;


    return *this;
}


namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const HousingInterestRate& data)
        {
            return strm << "{" << "\"id \":\"" << data.id << "\","
                        << "\"interestRate \":\""       << data.interest_rate << "\"" << "}";
        }
    }
}


