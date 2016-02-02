/*
 * DevelopmentPlan.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: gishara
 */

#include "DevelopmentPlan.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

DevelopmentPlan::DevelopmentPlan(BigSerial fmParcelId, BigSerial templateId, int unitTypeId, int numUnits, std::tm simulationDate,std::tm constructionStartDate,std::tm launchDate) :
		fmParcelId(fmParcelId), templateId(templateId), unitTypeId(unitTypeId), numUnits(numUnits) , simulationDate(simulationDate),constructionStartDate(constructionStartDate),launchDate(launchDate){
}

DevelopmentPlan::~DevelopmentPlan() {
}

DevelopmentPlan::DevelopmentPlan( const DevelopmentPlan &source)
{
	this->fmParcelId = source.fmParcelId;
	this->templateId = source.templateId;
	this->unitTypeId = source.unitTypeId;
	this->numUnits = source.numUnits;
	this->simulationDate = source.simulationDate;
	this->constructionStartDate = source.constructionStartDate;
	this->launchDate = source.launchDate;
}

DevelopmentPlan& DevelopmentPlan::operator=(const DevelopmentPlan& source)
{
	this->fmParcelId = source.fmParcelId;
	this->templateId = source.templateId;
	this->unitTypeId = source.unitTypeId;
	this->numUnits = source.numUnits;
	this->simulationDate = source.simulationDate;
	this->constructionStartDate = source.constructionStartDate;
	this->launchDate = source.launchDate;

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

const std::tm& DevelopmentPlan::getLaunchDate() const
{
	return launchDate;
}

void DevelopmentPlan::setLaunchDate(const std::tm& launchDate)
{
	this->launchDate = launchDate;
}

const std::tm& DevelopmentPlan::getConstructionStartDate() const
{
	return constructionStartDate;
}

void DevelopmentPlan::setConstructionStartDate(const std::tm& constructionStartDate)
{
	this->constructionStartDate = constructionStartDate;
}

BigSerial DevelopmentPlan::getTemplateId() const
{
	return templateId;
}

void DevelopmentPlan::setTemplateId(BigSerial templateId)
{
	this->templateId = templateId;
}
