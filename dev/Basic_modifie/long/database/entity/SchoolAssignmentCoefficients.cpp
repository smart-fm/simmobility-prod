/*
 * SchoolAssignmentCoefficients.cpp
 *
 *  Created on: 9 Mar 2016
 *      Author: gishara
 */
#include "SchoolAssignmentCoefficients.hpp"

using namespace sim_mob::long_term;

SchoolAssignmentCoefficients::SchoolAssignmentCoefficients(BigSerial parameterId,double coefficientEstimate):
								parameterId(parameterId),coefficientEstimate(coefficientEstimate) {}

SchoolAssignmentCoefficients::~SchoolAssignmentCoefficients() {}

SchoolAssignmentCoefficients& SchoolAssignmentCoefficients::operator=(const SchoolAssignmentCoefficients& source)
{
	this->parameterId 			= source.parameterId;
	this->coefficientEstimate	= source.coefficientEstimate;
    return *this;
}

double SchoolAssignmentCoefficients::getCoefficientEstimate() const
{
		return coefficientEstimate;
}

void SchoolAssignmentCoefficients::setCoefficientEstimate(double coefficientEstimate)
{
		this->coefficientEstimate = coefficientEstimate;
}

BigSerial SchoolAssignmentCoefficients::getParameterId() const
{
		return parameterId;
}

void SchoolAssignmentCoefficients::setParameterId(BigSerial parameterId)
{
		this->parameterId = parameterId;
}






