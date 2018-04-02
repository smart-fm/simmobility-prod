/*
 * ResidentialWTP_Coefs.cpp
 *
 *  Created on: 29 Mar 2018
 *      Author: gishara
 */

#include "ResidentialWTP_Coefs.hpp"

using namespace sim_mob::long_term;

ResidentialWTP_Coefs::ResidentialWTP_Coefs(int id,std::string propertyType, double sde, double m2, double s2, double constant,double logArea,double logsumTaz,double age,double ageSquared,double missingAgeDummy,
										   double carDummy, double carIntoLogsumTaz, double distanceMall,double mrt200m_400m,double matureDummy,double matureOtherDummy,double floorNumber,double logIncome,
										   double logIncomeIntoLogArea, double freeholdApartment, double freeholdCondo, double freeholdTerrace, double freeholdDetached, double bus_200m_400m_Dummy, double oneTwoFullTimeWorkerDummy,
										   double fullTimeWorkersTwoIntoLogArea,double hhSizeworkersDiff):id(id),propertyType(propertyType), sde(sde), m2(m2), s2(s2), constant(constant), logArea(logArea), logsumTaz(logsumTaz),
										   age(age), ageSquared(ageSquared),missingAgeDummy(missingAgeDummy), carDummy(carDummy), carIntoLogsumTaz(carIntoLogsumTaz), distanceMall(distanceMall), mrt200m_400m(mrt200m_400m),
										   matureDummy(matureDummy),matureOtherDummy(matureOtherDummy), floorNumber(floorNumber), logIncome(logIncome), logIncomeIntoLogArea(logIncomeIntoLogArea),freeholdApartment(freeholdApartment),
										   freeholdCondo(freeholdCondo),freeholdTerrace(freeholdTerrace) ,freeholdDetached(freeholdDetached), bus_200m_400m_Dummy(bus_200m_400m_Dummy), oneTwoFullTimeWorkerDummy(oneTwoFullTimeWorkerDummy)
										   ,fullTimeWorkersTwoIntoLogArea(fullTimeWorkersTwoIntoLogArea), hhSizeworkersDiff(hhSizeworkersDiff){}

ResidentialWTP_Coefs::~ResidentialWTP_Coefs(){}

ResidentialWTP_Coefs::ResidentialWTP_Coefs(const ResidentialWTP_Coefs& source)
{
	this->id = source.id;
	this->propertyType = source.propertyType;
	this->sde = source.sde;
	this->m2 = source.m2;
	this->s2 = source.s2;
	this->constant = source.constant;
	this->logArea = source.logArea;
	this->logsumTaz = source.logsumTaz;
	this->age = source.age;
	this->ageSquared = source.ageSquared;
	this->missingAgeDummy = source.missingAgeDummy;
	this->carDummy = source.carDummy;
	this->carIntoLogsumTaz = source.carIntoLogsumTaz;
	this->distanceMall = source.distanceMall;
	this->mrt200m_400m = source.mrt200m_400m;
	this->matureDummy = source.matureDummy;
	this->matureOtherDummy = source.matureOtherDummy;
	this->floorNumber = source.floorNumber;
	this->logIncome = source.logIncome;
	this->logIncomeIntoLogArea = source.logIncomeIntoLogArea;
	this->freeholdApartment = source.freeholdApartment;
	this->freeholdCondo = source.freeholdCondo;
	this->freeholdTerrace = source.freeholdTerrace;
	this->freeholdDetached = source.freeholdDetached;
	this->bus_200m_400m_Dummy = source.bus_200m_400m_Dummy;
	this->oneTwoFullTimeWorkerDummy = source.oneTwoFullTimeWorkerDummy;
	this->fullTimeWorkersTwoIntoLogArea = source.fullTimeWorkersTwoIntoLogArea;
	this->hhSizeworkersDiff = source.hhSizeworkersDiff;


}

ResidentialWTP_Coefs& ResidentialWTP_Coefs::operator=(const ResidentialWTP_Coefs& source)
{
	this->id = source.id;
	this->propertyType = source.propertyType;
	this->sde = source.sde;
	this->m2 = source.m2;
	this->s2 = source.s2;
	this->constant = source.constant;
	this->logArea = source.logArea;
	this->logsumTaz = source.logsumTaz;
	this->age = source.age;
	this->ageSquared = source.ageSquared;
	this->missingAgeDummy = source.missingAgeDummy;
	this->carDummy = source.carDummy;
	this->carIntoLogsumTaz = source.carIntoLogsumTaz;
	this->distanceMall = source.distanceMall;
	this->mrt200m_400m = source.mrt200m_400m;
	this->matureDummy = source.matureDummy;
	this->matureOtherDummy = source.matureOtherDummy;
	this->floorNumber = source.floorNumber;
	this->logIncome = source.logIncome;
	this->logIncomeIntoLogArea = source.logIncomeIntoLogArea;
	this->freeholdApartment = source.freeholdApartment;
	this->freeholdCondo = source.freeholdCondo;
	this->freeholdTerrace = source.freeholdTerrace;
	this->freeholdDetached = source.freeholdDetached;
	this->bus_200m_400m_Dummy = source.bus_200m_400m_Dummy;
	this->oneTwoFullTimeWorkerDummy = source.oneTwoFullTimeWorkerDummy;
	this->fullTimeWorkersTwoIntoLogArea = source.fullTimeWorkersTwoIntoLogArea;
	this->hhSizeworkersDiff = source.hhSizeworkersDiff;

	return *this;
}

double ResidentialWTP_Coefs::getAge() const {
	return age;
}

void ResidentialWTP_Coefs::setAge(double age) {
	this->age = age;
}

double ResidentialWTP_Coefs::getAgeSquared() const {
	return ageSquared;
}

void ResidentialWTP_Coefs::setAgeSquared(double ageSquared) {
	this->ageSquared = ageSquared;
}

double ResidentialWTP_Coefs::getBus200m400mDummy() const {
	return bus_200m_400m_Dummy;
}

void ResidentialWTP_Coefs::setBus200m400mDummy(double bus200m400mDummy) {
	bus_200m_400m_Dummy = bus200m400mDummy;
}

double ResidentialWTP_Coefs::getCarDummy() const {
	return carDummy;
}

void ResidentialWTP_Coefs::setCarDummy(double carDummy) {
	this->carDummy = carDummy;
}

double ResidentialWTP_Coefs::getCarIntoLogsumTaz() const {
	return carIntoLogsumTaz;
}

void ResidentialWTP_Coefs::setCarIntoLogsumTaz(double carIntoLogsumTaz) {
	this->carIntoLogsumTaz = carIntoLogsumTaz;
}

double ResidentialWTP_Coefs::getConstant() const {
	return constant;
}

void ResidentialWTP_Coefs::setConstant(double constant) {
	this->constant = constant;
}

double ResidentialWTP_Coefs::getDistanceMall() const {
	return distanceMall;
}

void ResidentialWTP_Coefs::setDistanceMall(double diistanceMall) {
	this->distanceMall = diistanceMall;
}

double ResidentialWTP_Coefs::getFloorNumber() const {
	return floorNumber;
}

void ResidentialWTP_Coefs::setFloorNumber(double floorNumber) {
	this->floorNumber = floorNumber;
}

double ResidentialWTP_Coefs::getFreeholdApartment() const {
	return freeholdApartment;
}

void ResidentialWTP_Coefs::setFreeholdApartment(double freeholdApartment) {
	this->freeholdApartment = freeholdApartment;
}

double ResidentialWTP_Coefs::getFreeholdCondo() const {
	return freeholdCondo;
}

void ResidentialWTP_Coefs::setFreeholdCondo(double freeholdCondo) {
	this->freeholdCondo = freeholdCondo;
}

double ResidentialWTP_Coefs::getFreeholdDetached() const {
	return freeholdDetached;
}

void ResidentialWTP_Coefs::setFreeholdDetached(double freeholdDetached) {
	this->freeholdDetached = freeholdDetached;
}

double ResidentialWTP_Coefs::getFreeholdTerrace() const {
	return freeholdTerrace;
}

void ResidentialWTP_Coefs::setFreeholdTerrace(double freeholdTerrace) {
	this->freeholdTerrace = freeholdTerrace;
}

double ResidentialWTP_Coefs::getFullTimeWorkersTwoIntoLogArea() const {
	return fullTimeWorkersTwoIntoLogArea;
}

void ResidentialWTP_Coefs::setFullTimeWorkersTwoIntoLogArea(double fullTimeWorkersTwooIntoLogArea) {
	this->fullTimeWorkersTwoIntoLogArea = fullTimeWorkersTwooIntoLogArea;
}

double ResidentialWTP_Coefs::getHhSizeworkersDiff() const {
	return hhSizeworkersDiff;
}

void ResidentialWTP_Coefs::setHhSizeworkersDiff(double hhSizeworkersDiff) {
	this->hhSizeworkersDiff = hhSizeworkersDiff;
}

int ResidentialWTP_Coefs::getId() const {
	return id;
}

void ResidentialWTP_Coefs::setId(int id) {
	this->id = id;
}

double ResidentialWTP_Coefs::getLogArea() const {
	return logArea;
}

void ResidentialWTP_Coefs::setLogArea(double logArea) {
	this->logArea = logArea;
}

double ResidentialWTP_Coefs::getLogIncome() const {
	return logIncome;
}

void ResidentialWTP_Coefs::setLogIncome(double logIncome) {
	this->logIncome = logIncome;
}

double ResidentialWTP_Coefs::getLogIncomeIntoLogArea() const {
	return logIncomeIntoLogArea;
}

void ResidentialWTP_Coefs::setLogIncomeIntoLogArea(double logIncomeIntoLogArea) {
	this->logIncomeIntoLogArea = logIncomeIntoLogArea;
}

double ResidentialWTP_Coefs::getLogsumTaz() const {
	return logsumTaz;
}

void ResidentialWTP_Coefs::setLogsumTaz(double logsumTaz) {
	this->logsumTaz = logsumTaz;
}

double ResidentialWTP_Coefs::getM2() const {
	return m2;
}

void ResidentialWTP_Coefs::setM2(double m2) {
	this->m2 = m2;
}

double ResidentialWTP_Coefs::getMatureDummy() const {
	return matureDummy;
}

void ResidentialWTP_Coefs::setMatureDummy(double matureDummy) {
	this->matureDummy = matureDummy;
}

double ResidentialWTP_Coefs::getMatureOtherDummy() const {
	return matureOtherDummy;
}

void ResidentialWTP_Coefs::setMatureOtherDummy(double matureOtherDummy) {
	this->matureOtherDummy = matureOtherDummy;
}

double ResidentialWTP_Coefs::getMissingAgeDummy() const {
	return missingAgeDummy;
}

void ResidentialWTP_Coefs::setMissingAgeDummy(double missingAgeDummy) {
	this->missingAgeDummy = missingAgeDummy;
}

double ResidentialWTP_Coefs::getMrt200m400m() const {
	return mrt200m_400m;
}

void ResidentialWTP_Coefs::setMrt200m400m(double mrt200m400m) {
	mrt200m_400m = mrt200m400m;
}

double ResidentialWTP_Coefs::getOneTwoFullTimeWorkerDummy() const {
	return oneTwoFullTimeWorkerDummy;
}

void ResidentialWTP_Coefs::setOneTwoFullTimeWorkerDummy(double oneTwoFullTimeWorkerDummy) {
	this->oneTwoFullTimeWorkerDummy = oneTwoFullTimeWorkerDummy;
}

std::string ResidentialWTP_Coefs::getPropertyType() const {
	return propertyType;
}

void ResidentialWTP_Coefs::setPropertyType(std::string propertyType) {
	this->propertyType = propertyType;
}

double ResidentialWTP_Coefs::getS2() const {
	return s2;
}

void ResidentialWTP_Coefs::setS2(double s2) {
	this->s2 = s2;
}

double ResidentialWTP_Coefs::getSde() const {
	return sde;
}

void ResidentialWTP_Coefs::setSde(double sde) {
	this->sde = sde;
}


