/*
 * HedonicCoeffsByUnitType.cpp
 *
 *  Created on: 15 May 2017
 *      Author: gishara
 */
#include "HedonicCoeffsByUnitType.hpp"

using namespace sim_mob::long_term;

HedonicCoeffsByUnitType::HedonicCoeffsByUnitType( BigSerial unitTypeId, double intercept, double logSqrtArea ,double freehold, double logsumWeighted,double pms_1km,
							  double distanceMallKm,double mrt_200m,double mrt_2_400m,double express_200m,double bus2_400m, double busGt400m,  double age,
							  double ageSquared, double misage) :
							  unitTypeId(unitTypeId),intercept(intercept),logSqrtArea(logSqrtArea),freehold(freehold),logsumWeighted(logsumWeighted),
							  pms_1km(pms_1km),distanceMallKm(distanceMallKm),mrt_200m(mrt_200m),mrt_2_400m(mrt_2_400m),express_200m(express_200m),bus2_400m(bus2_400m),
							  busGt400m(busGt400m),age(age),ageSquared(ageSquared),misage(misage){}

HedonicCoeffsByUnitType::~HedonicCoeffsByUnitType() {}

double HedonicCoeffsByUnitType::getAge() const
{
	return age;
}

void  HedonicCoeffsByUnitType::setAge(double age)
{
	this->age = age;
}

double  HedonicCoeffsByUnitType::getBus2400m() const
{
	return bus2_400m;
}

void  HedonicCoeffsByUnitType::setBus2400m(double bus2400m)
{
	bus2_400m = bus2400m;
}

double  HedonicCoeffsByUnitType::getBusGt400m() const
{
	return busGt400m;
}

void  HedonicCoeffsByUnitType::setBusGt400m(double busGt400m)
{
	this->busGt400m = busGt400m;
}

double  HedonicCoeffsByUnitType::getDistanceMallKm() const
{
	return distanceMallKm;
}

void  HedonicCoeffsByUnitType::setDistanceMallKm(double distanceMallKm)
{
	this->distanceMallKm = distanceMallKm;
}

double  HedonicCoeffsByUnitType::getExpress200m() const
{
	return express_200m;
}

void  HedonicCoeffsByUnitType::setExpress200m(double express200m)
{
	express_200m = express200m;
}

double  HedonicCoeffsByUnitType::getFreehold() const
{
	return freehold;
}

void  HedonicCoeffsByUnitType::setFreehold(double freehold)
{
	this->freehold = freehold;
}

double  HedonicCoeffsByUnitType::getIntercept() const
{
	return intercept;
}

void  HedonicCoeffsByUnitType::setIntercept(double intercept)
{
	this->intercept = intercept;
}

double  HedonicCoeffsByUnitType::getAgeSquared() const
{
	return ageSquared;
}

void  HedonicCoeffsByUnitType::setAgeSquared(double ageSquared)
{
	this->ageSquared = ageSquared;
}

double  HedonicCoeffsByUnitType::getLogSqrtArea() const
{
	return logSqrtArea;
}

void  HedonicCoeffsByUnitType::setLogSqrtArea(double logSqrtArea)
{
	this->logSqrtArea = logSqrtArea;
}

double  HedonicCoeffsByUnitType::getLogsumWeighted() const
{
	return logsumWeighted;
}

void  HedonicCoeffsByUnitType::setLogsumWeighted(double logsumWeighted)
{
	this->logsumWeighted = logsumWeighted;
}

double  HedonicCoeffsByUnitType::getMisage() const
{
	return misage;
}

void  HedonicCoeffsByUnitType::setMisage(double misage)
{
	this->misage = misage;
}

double  HedonicCoeffsByUnitType::getMrt2400m() const
{
	return mrt_2_400m;
}

void  HedonicCoeffsByUnitType::setMrt2400m(double mrt2400m)
{
	mrt_2_400m = mrt2400m;
}

double  HedonicCoeffsByUnitType::getMrt200m() const
{
	return mrt_200m;
}

void HedonicCoeffsByUnitType::setMrt200m(double mrt200m)
{
	mrt_200m = mrt200m;
}

double HedonicCoeffsByUnitType::getPms1km() const
{
	return pms_1km;
}

void HedonicCoeffsByUnitType::setPms1km(double pms1km)
{
	pms_1km = pms1km;
}

BigSerial HedonicCoeffsByUnitType::getUnitTypeId() const
{
	return unitTypeId;
}

void HedonicCoeffsByUnitType::setUnitTypeId(BigSerial unitTypeId)
{
	this->unitTypeId = unitTypeId;
}
