/*
 * LagPrivate_TByUnitType.cpp
 *
 *  Created on: 17 May 2017
 *      Author: gishara
 */
#include "LagPrivate_TByUnitType.hpp"

using namespace sim_mob::long_term;

LagPrivate_TByUnitType::LagPrivate_TByUnitType( BigSerial unitTypeId, double intercept, double t4 , double t5, double t6, double t7, double gdpRate ) :
		unitTypeId(unitTypeId),intercept(intercept),t4(t4),t5(t5), t6(t6), t7(t7),gdpRate(gdpRate){}

LagPrivate_TByUnitType::~LagPrivate_TByUnitType() {}

double LagPrivate_TByUnitType::getGdpRate() const
{
	return gdpRate;
}

void LagPrivate_TByUnitType::setGdpRate(double gdpRate)
{
	this->gdpRate = gdpRate;
}

double LagPrivate_TByUnitType::getIntercept() const
{
	return intercept;
}

void LagPrivate_TByUnitType::setIntercept(double intercept)
{
	this->intercept = intercept;
}

double LagPrivate_TByUnitType::getT4() const
{
	return t4;
}

void LagPrivate_TByUnitType::setT4(double t4)
{
	this->t4 = t4;
}

double LagPrivate_TByUnitType::getT5() const
{
	return t5;
}

void LagPrivate_TByUnitType::setT5(double t5)
{
	this->t5 = t5;
}

double LagPrivate_TByUnitType::getT6() const
{
	return t6;
}

void LagPrivate_TByUnitType::setT6(double t6)
{
	this->t6 = t6;
}

double LagPrivate_TByUnitType::getT7() const
{
	return t7;
}

void LagPrivate_TByUnitType::setT7(double t7)
{
	this->t7 = t7;
}

BigSerial LagPrivate_TByUnitType::getUnitTypeId() const
{
	return unitTypeId;
}

	void LagPrivate_TByUnitType::setUnitTypeId(BigSerial unitTypeId) {
		this->unitTypeId = unitTypeId;
	}
