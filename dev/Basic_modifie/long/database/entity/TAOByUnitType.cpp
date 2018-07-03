/*
 * TAOByUnitType.cpp
 *
 *  Created on: 15 May 2017
 *      Author: gishara
 */
#include "TAOByUnitType.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

TAOByUnitType::TAOByUnitType(BigSerial id, std::string quarter, double treasuryBillYield1Year,double gdpRate, double inflation, double tCondo12, double tCondo13,double tCondo14,double tCondo15,double tCondo16, double tApartment7,double tApartment8,
							double tApartment9, double tApartment10, double tApartment11):
							id(id),quarter(quarter),treasuryBillYield1Year(treasuryBillYield1Year),gdpRate(gdpRate), inflation(inflation), tCondo12(tCondo12),tCondo13(tCondo13),tCondo14(tCondo14), tCondo15(tCondo15),tCondo16(tCondo16), tApartment7(tApartment7), tApartment8(tApartment8), tApartment9(tApartment9), tApartment10(tApartment10), tApartment11(tApartment11) {}

TAOByUnitType::~TAOByUnitType() {}

TAOByUnitType& TAOByUnitType::operator=(const TAOByUnitType& source)
{
	this->id = source.id;
	this->quarter = source.quarter;
	this->treasuryBillYield1Year = source.treasuryBillYield1Year;
	this->inflation = source.inflation;
	this->gdpRate = source.gdpRate;
	this->tCondo12	= source.tCondo12;
	this->tCondo13	= source.tCondo13;
	this->tCondo14	= source.tCondo14;
	this->tCondo15	= source.tCondo15;
	this->tCondo16	= source.tCondo16;
	this->tApartment7 = source.tApartment7;
	this->tApartment8 = source.tApartment8;
	this->tApartment9 = source.tApartment9;
	this->tApartment10 = source.tApartment10;
	this->tApartment11 = source.tApartment11;

    return *this;
}

double TAOByUnitType::getTApartment10() const
{
	return tApartment10;
}

void TAOByUnitType::setTApartment10(double apartment10)
{
	this->tApartment10 = apartment10;
}

double TAOByUnitType::getTApartment11() const
{
	return tApartment11;
}

void TAOByUnitType::setTApartment11(double apartment11)
{
	this->tApartment11 = apartment11;
}

double TAOByUnitType::getTApartment7() const
{
	return tApartment7;
}

void TAOByUnitType::setTApartment7(double apartment7)
{
	this->tApartment7 = apartment7;
}

double TAOByUnitType::getTApartment8() const
{
	return tApartment8;
}

void TAOByUnitType::setTApartment8(double apartment8)
{
	this->tApartment8 = apartment8;
}

double TAOByUnitType::getTApartment9() const
{
	return tApartment9;
}

void TAOByUnitType::setTApartment9(double apartment9)
{
	this->tApartment9 = apartment9;
}

double TAOByUnitType::getTCondo12() const
{
	return tCondo12;
}

void TAOByUnitType::setTCondo12(double condo12)
{
	this->tCondo12 = condo12;
}

double TAOByUnitType::getTCondo13() const
{
	return tCondo13;
}

void TAOByUnitType::setTCondo13(double condo13)
{
	this->tCondo13 = condo13;
}

double TAOByUnitType::getTCondo14() const
{
	return tCondo14;
}

void TAOByUnitType::setTCondo14(double condo14)
{
	this->tCondo14 = condo14;
}

double TAOByUnitType::getTCondo15() const
{
	return tCondo15;
}

void TAOByUnitType::setTCondo15(double condo15)
{
	this->tCondo15 = condo15;
}

double TAOByUnitType::getTCondo16() const
{
	return tCondo16;
}

void TAOByUnitType::setTCondo16(double condo16)
{
	this->tCondo16 = condo16;
}

const std::string& TAOByUnitType::getQuarter() const
{
	return quarter;
}

void TAOByUnitType::setQuarter(const std::string& quarter)
{
	this->quarter = quarter;
}

BigSerial TAOByUnitType::getId() const
{
	return id;
}

void TAOByUnitType::setId(BigSerial id)
{
	this->id = id;
}

double TAOByUnitType::getGdpRate() const
{
	return gdpRate;
}

void TAOByUnitType::setGdpRate(double gdpRate)
{
	this->gdpRate = gdpRate;
}

double TAOByUnitType::getInflation() const
{
	return inflation;
}

void TAOByUnitType::setInflation(double inflation)
{
	this->inflation = inflation;
}

double TAOByUnitType::getTreasuryBillYield1Year() const
{
	return treasuryBillYield1Year;
}

void TAOByUnitType::setTreasuryBillYield1Year(double treasuryBillYield1Year)
{
	this->treasuryBillYield1Year = treasuryBillYield1Year;
}


