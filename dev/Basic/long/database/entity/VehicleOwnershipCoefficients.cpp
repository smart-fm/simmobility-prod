/*
 * VehicleOwnershipCoefficients.cpp
 *
 *  Created on: Feb 24, 2015
 *      Author: gishara
 */
#include "VehicleOwnershipCoefficients.hpp"

using namespace sim_mob::long_term;

VehicleOwnershipCoefficients::VehicleOwnershipCoefficients(BigSerial parameterId,double coefficientEstimate):
								parameterId(parameterId),coefficientEstimate(coefficientEstimate) {}

VehicleOwnershipCoefficients::~VehicleOwnershipCoefficients() {}

VehicleOwnershipCoefficients& VehicleOwnershipCoefficients::operator=(const VehicleOwnershipCoefficients& source)
{
	this->parameterId 			= source.parameterId;
	this->coefficientEstimate	= source.coefficientEstimate;
    return *this;
}

double VehicleOwnershipCoefficients::getCoefficientEstimate() const
{
		return coefficientEstimate;
}

void VehicleOwnershipCoefficients::setCoefficientEstimate(double coefficientEstimate)
{
		this->coefficientEstimate = coefficientEstimate;
}

BigSerial VehicleOwnershipCoefficients::getParameterId() const
{
		return parameterId;
}

void VehicleOwnershipCoefficients::setParameterId(BigSerial parameterId)
{
		this->parameterId = parameterId;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const VehicleOwnershipCoefficients& data)
        {
            return strm << "{"
						<< "\"parameterId \":\"" << data.parameterId 	<< "\","
						<< "\"coefficientEstimate \":\"" 	<< data.coefficientEstimate 	<< "\","
						<< "}";
        }
    }
}



