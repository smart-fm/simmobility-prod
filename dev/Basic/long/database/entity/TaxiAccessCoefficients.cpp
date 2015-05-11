/*
 * TaxiAccessCoefficients.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: gishara
 */

#include "TaxiAccessCoefficients.hpp"

using namespace sim_mob::long_term;

TaxiAccessCoefficients::TaxiAccessCoefficients(BigSerial parameterId,double coefficientEstimate):
								parameterId(parameterId),coefficientEstimate(coefficientEstimate) {}

TaxiAccessCoefficients::~TaxiAccessCoefficients() {}

TaxiAccessCoefficients& TaxiAccessCoefficients::operator=(const TaxiAccessCoefficients& source)
{
	this->parameterId 			= source.parameterId;
	this->coefficientEstimate	= source.coefficientEstimate;
    return *this;
}

double TaxiAccessCoefficients::getCoefficientEstimate() const
{
		return coefficientEstimate;
}

void TaxiAccessCoefficients::setCoefficientEstimate(double coefficientEstimate)
{
		this->coefficientEstimate = coefficientEstimate;
}

BigSerial TaxiAccessCoefficients::getParameterId() const
{
		return parameterId;
}

void TaxiAccessCoefficients::setParameterId(BigSerial parameterId)
{
		this->parameterId = parameterId;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const TaxiAccessCoefficients& data)
        {
            return strm << "{"
						<< "\"parameterId \":\"" << data.parameterId 	<< "\","
						<< "\"coefficientEstimate \":\"" 	<< data.coefficientEstimate 	<< "\","
						<< "}";
        }
    }
}


