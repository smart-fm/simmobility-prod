/*
 * LagPrivate_T.cpp
 *
 *  Created on: 1 Sep 2016
 *      Author: gishara
 */
#include "LagPrivateT.hpp"

using namespace sim_mob::long_term;

LagPrivateT::LagPrivateT( BigSerial propertyTypeId, double intercept, double T4) :
							  propertyTypeId(propertyTypeId),intercept(intercept),T4(T4){}

LagPrivateT::~LagPrivateT() {}

BigSerial LagPrivateT::getPropertyTypeId() const
{
	return propertyTypeId;
}

void LagPrivateT::setPropertyTypeId(BigSerial id)
{
	this->propertyTypeId = id;
}

double  LagPrivateT::getIntercept() const
{
	return intercept;
}

void  LagPrivateT::setIntercept(double intercept)
{
	this->intercept = intercept;
}

double  LagPrivateT::getT4() const
{
	return T4;
}

void  LagPrivateT::setT4(double t4)
{
	T4 = t4;
}
