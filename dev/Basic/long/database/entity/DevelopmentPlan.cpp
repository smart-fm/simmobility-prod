/*
 * DevelopmentPlan.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: gishara
 */

#include "DevelopmentPlan.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

DevelopmentPlan::DevelopmentPlan(BigSerial fmParcelId, int unitTypeId, int numUnits, std::tm simulationDate) :
		fmParcelId(fmParcelId), unitTypeId(unitTypeId), numUnits(numUnits) , simulationDate(simulationDate){
}

DevelopmentPlan::~DevelopmentPlan() {
}

DevelopmentPlan::DevelopmentPlan( const DevelopmentPlan &source)
{
	this->fmParcelId = source.fmParcelId;
	this->unitTypeId = source.unitTypeId;
	this->numUnits = source.numUnits;
	this->simulationDate = source.simulationDate;
}

DevelopmentPlan& DevelopmentPlan::operator=(const DevelopmentPlan& source)
{
	this->fmParcelId = source.fmParcelId;
	this->unitTypeId = source.unitTypeId;
	this->numUnits = source.numUnits;
	this->simulationDate = source.simulationDate;

    return *this;
}

BigSerial DevelopmentPlan::getFmParcelId() const
{
	return fmParcelId;
}

void DevelopmentPlan::setFmParcelId(BigSerial fmParcelId)
{
	this->fmParcelId = fmParcelId;
}

int DevelopmentPlan::getNumUnits() const
{
	return numUnits;
}

void DevelopmentPlan::setNumUnits(int numUnits)
{
	this->numUnits = numUnits;
}

const std::tm& DevelopmentPlan::getSimulationDate() const
{
	return simulationDate;
}

void DevelopmentPlan::setSimulationDate(const std::tm& simulationDate)
{
	this->simulationDate = simulationDate;
}

int DevelopmentPlan::getUnitTypeId() const
{
	return unitTypeId;
}

void DevelopmentPlan::setUnitTypeId(int unitTypeId)
{
	this->unitTypeId = unitTypeId;
}
