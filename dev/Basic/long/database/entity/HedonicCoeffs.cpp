/*
 * HedonicCoeffs.cpp
 *
 *  Created on: 30 Aug 2016
 *      Author: gishara
 */

#include "HedonicCoeffs.hpp"

using namespace sim_mob::long_term;

HedonicCoeffs::HedonicCoeffs( BigSerial proprtyTypeId, double intercept, double logSqrtArea ,double freehold, double logsumWeighted,double pms_1km,
							  double distanceMallKm,double mrt_200m,double mrt_2_400m,double express_200m,double bus2_400m, double busGt400m,  double age,
							  double logAgeSquared, double agem25_50, double agem50, double misage, double age_30m) :
							  proprtyTypeId(proprtyTypeId),intercept(intercept),logSqrtArea(logSqrtArea),freehold(freehold),logsumWeighted(logsumWeighted),
							  pms_1km(pms_1km),distanceMallKm(distanceMallKm),mrt_200m(mrt_200m),mrt_2_400m(mrt_2_400m),express_200m(express_200m),bus2_400m(bus2_400m),
							  busGt400m(busGt400m),age(age),logAgeSquared(logAgeSquared),agem25_50(agem25_50),agem50(agem50),misage(misage),age_30m(age_30m){}

HedonicCoeffs::~HedonicCoeffs() {}

double HedonicCoeffs::getAge() const
{
	return age;
}

void HedonicCoeffs::setAge(double age)
{
	this->age = age;
}

double HedonicCoeffs::getAge30m() const
{
	return age_30m;
}

void HedonicCoeffs::setAge30m(double age30m)
{
	age_30m = age30m;
}

double HedonicCoeffs::getAgem25_50() const
{
	return agem25_50;
}

void HedonicCoeffs::setAgem25_50(double agem)
{
	agem25_50 = agem;
}

double HedonicCoeffs::getAgem50() const
{
	return agem50;
}

void HedonicCoeffs::setAgem50(double agem50)
{
	this->agem50 = agem50;
}

double HedonicCoeffs::getBus400m() const
{
	return bus2_400m;
}

void HedonicCoeffs::setBus400m(double bus400m)
{
	bus2_400m = bus400m;
}

double HedonicCoeffs::getBusGt400m() const
{
	return busGt400m;
}

void HedonicCoeffs::setBusGt400m(double busGt400m)
{
	this->busGt400m = busGt400m;
}

double HedonicCoeffs::getDistanceMallKm() const
{
	return distanceMallKm;
}

void HedonicCoeffs::setDistanceMallKm(double distanceMallKm)
{
	this->distanceMallKm = distanceMallKm;
}

double HedonicCoeffs::getExpress200m() const
{
	return express_200m;
}

void HedonicCoeffs::setExpress200m(double express200m)
{
	express_200m = express200m;
}

double HedonicCoeffs::getFreehold() const
{
	return freehold;
}

void HedonicCoeffs::setFreehold(double freehold)
{
	this->freehold = freehold;
}

BigSerial HedonicCoeffs::getPropertyTypeId() const
{
	return proprtyTypeId;
}

void HedonicCoeffs::setPropertyTypeId(BigSerial id)
{
	this->proprtyTypeId = id;
}

double HedonicCoeffs::getIntercept() const
{
	return intercept;
}

void HedonicCoeffs::setIntercept(double intercept)
{
	this->intercept = intercept;
}

double HedonicCoeffs::getLogAgeSquared() const
{
	return logAgeSquared;
}

void HedonicCoeffs::setLogAgeSquared(double logAgeSquared)
{
	this->logAgeSquared = logAgeSquared;
}

double HedonicCoeffs::getLogSqrtArea() const
{
	return logSqrtArea;
}

void HedonicCoeffs::setLogSqrtArea(double logSqrtArea)
{
	this->logSqrtArea = logSqrtArea;
}

double HedonicCoeffs::getLogsumWeighted() const
{
	return logsumWeighted;
}

void HedonicCoeffs::setLogsumWeighted(double logsumWeighted)
{
	this->logsumWeighted = logsumWeighted;
}

double HedonicCoeffs::getMisage() const
{
	return misage;
}

void HedonicCoeffs::setMisage(double misage)
{
	this->misage = misage;
}

double HedonicCoeffs::getMrt_2_400m() const
{
	return mrt_2_400m;
}

void HedonicCoeffs::setMrt_2_400m(double mrt2400m)
{
	mrt_2_400m = mrt2400m;
}

double HedonicCoeffs::getMrt200m() const
{
	return mrt_200m;
}

void HedonicCoeffs::setMrt200m(double mrt200m)
{
	mrt_200m = mrt200m;
}

double HedonicCoeffs::getPms1km() const
{
	return pms_1km;
}

void HedonicCoeffs::setPms1km(double pms1km)
{
	pms_1km = pms1km;
}
