/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   PotentialProject.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 31, 2014, 2:51 PM
 */

#include "PotentialProject.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

PotentialUnit::PotentialUnit(BigSerial unitTypeId,int numUnits,double floorArea,int freehold, double profitPerUnit, double demolitionCostPerUnit): unitTypeId(unitTypeId),numUnits(numUnits),floorArea(floorArea),freehold(freehold),profitPerUnit(profitPerUnit),demolitionCostPerUnit(demolitionCostPerUnit){

}

PotentialUnit::PotentialUnit( const PotentialUnit& source)
{
	this->unitTypeId = source.unitTypeId;
	this->numUnits = source.numUnits;
	this->floorArea = source.floorArea;
	this->freehold = source.freehold;
	this->profitPerUnit = source.profitPerUnit;
	this->demolitionCostPerUnit = source.demolitionCostPerUnit;
}

PotentialUnit& PotentialUnit::operator=( const PotentialUnit& source)
{
	this->unitTypeId = source.unitTypeId;
	this->numUnits = source.numUnits;
	this->floorArea = source.floorArea;
	this->freehold = source.freehold;
	this->profitPerUnit = source.profitPerUnit;
	this->demolitionCostPerUnit = source.demolitionCostPerUnit;

	return *this;
}

PotentialUnit::~PotentialUnit() {
}

BigSerial PotentialUnit::getUnitTypeId() const {
    return unitTypeId;
}

double PotentialUnit::getFloorArea() const {
    return floorArea;
}

void PotentialUnit::setFloorArea(double area)
{
	this->floorArea = area;
}

int PotentialUnit::isFreehold() const {
    return freehold;
}

int PotentialUnit::getNumUnits() const {
	return numUnits;
}

void PotentialUnit::setNumUnits(int units){
	this->numUnits = units;
}

int PotentialUnit::getNumUnits(){
	return numUnits;
}

void PotentialUnit::setUnitTypeId(int typeId){
	this->unitTypeId = typeId;
}

void PotentialUnit::setUnitProfit(double unitProfit)
{
	this->profitPerUnit = unitProfit;
}

double PotentialUnit::getUnitProfit() const
{
	return this->profitPerUnit;
}

void PotentialUnit::setDemolitionCostPerUnit(double demolitionCost)
{
	this->demolitionCostPerUnit = demolitionCost;
}

double PotentialUnit::getDemolitionCostPerUnit()
{
	return this->demolitionCostPerUnit;
}

PotentialProject::PotentialProject(const DevelopmentTypeTemplate* devTemplate, const Parcel* parcel,double constructionCost, double grossArea,double tempSelectProbability,double investmentReturnRatio, double demolitionCost, double expRatio,int totalUnits)
								  : devTemplate(devTemplate), parcel(parcel), profit(0) , constructionCost(constructionCost),grossArea(grossArea),tempSelectProbability(tempSelectProbability),
								    investmentReturnRatio(investmentReturnRatio), demolitionCost(demolitionCost), expRatio(expRatio),totalUnits(totalUnits) {}

PotentialProject::PotentialProject( const PotentialProject &source)
{
	this->devTemplate = source.devTemplate;
	this->parcel = source.parcel;
	this->profit = source.profit;
	this->constructionCost = source.constructionCost;
	this->grossArea = source.grossArea;
	this->tempSelectProbability = source.tempSelectProbability;
	this->investmentReturnRatio = source.investmentReturnRatio;
	this->demolitionCost = source.demolitionCost;
	this->expRatio = source.expRatio;
	this->totalUnits = source.totalUnits;
	this->units = source.units;
	for (int i=0; i < source.units.size(); i++)
	{
		PotentialUnit unit;
		unit.setNumUnits( source.units[i].getNumUnits());
		unit.setUnitProfit(source.units[i].getUnitProfit());
		unit.setUnitTypeId( source.units[i].getUnitTypeId());
		unit.setFloorArea(source.units[i].getFloorArea());
		this->units[i]= unit;
	}
}

PotentialProject& PotentialProject::operator=(const PotentialProject& source)
{
	this->devTemplate = source.devTemplate;
	this->parcel = source.parcel;
	this->profit = source.profit;
	this->constructionCost = source.constructionCost;
	this->grossArea = source.grossArea;
	this->tempSelectProbability = source.tempSelectProbability;
	this->investmentReturnRatio = source.investmentReturnRatio;
	this->demolitionCost = source.demolitionCost;
	this->expRatio = source.expRatio;
	this->totalUnits = source.totalUnits;
	this->units.resize(source.units.size());

	for (int i=0; i < source.units.size(); i++)
		{
			PotentialUnit unit;
			unit.setNumUnits( source.units[i].getNumUnits());
			unit.setUnitProfit(source.units[i].getUnitProfit());
			unit.setUnitTypeId( source.units[i].getUnitTypeId());
			unit.setFloorArea(source.units[i].getFloorArea());
			this->units[i]= unit;
		}
	return *this;
}

PotentialProject::~PotentialProject() {
}

void PotentialProject::addUnit(const PotentialUnit& unit) {
    units.push_back(unit);
}

void PotentialProject::addTemplateUnitType(TemplateUnitType* templateUnitType) {

	templateUnitTypes.push_back(templateUnitType);
}

void PotentialProject::addUnits(int unitType,int numUnits) {

	this->unitMap.insert(std::make_pair(unitType, numUnits));
}


const DevelopmentTypeTemplate* PotentialProject::getDevTemplate() const {
    return devTemplate;
}

const Parcel* PotentialProject::getParcel() const {
    return parcel;
}

std::vector<PotentialUnit>& PotentialProject::getUnits(){
    return units;
}

double PotentialProject::getProfit() {
    return profit;
}

void PotentialProject::setProfit(const double profit) {
    this->profit = profit;
}

double PotentialProject::getConstructionCost() const {
    return constructionCost;
}

void PotentialProject::setConstructionCost(const double constructionCost) {
    this->constructionCost = constructionCost;
}

double PotentialProject::getGrosArea() const {
    return grossArea;
}

void PotentialProject::setGrossArea(const double grossArea) {
    this->grossArea = grossArea;
}

double PotentialProject::getInvestmentReturnRatio() const
{
	return this->investmentReturnRatio;
}

void PotentialProject::setInvestmentReturnRatio(double inReturnRatio)
{
	this->investmentReturnRatio = inReturnRatio;
}

double PotentialProject::getExpRatio() const
{
	return this->expRatio;
}

void PotentialProject::setExpRatio(double exRatio)
{
	this->expRatio = exRatio;
}

double PotentialProject::getTempSelectProbability() const
{
	return this->tempSelectProbability;
}

void PotentialProject::setTempSelectProbability(double probability)
{
	this->tempSelectProbability = probability;
}

double PotentialProject::getDemolitionCost() const
{
	return this->demolitionCost;
}

void PotentialProject::setDemolitionCost(double demCost)
{
	this->demolitionCost = demCost;
}

int PotentialProject::getTotalUnits()
{
	return this->totalUnits;
}

void PotentialProject::setTotalUnits (int totUnits)
{
	this->totalUnits = totUnits;
}
namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const PotentialUnit& data)
        {
            return strm << "{"
                    << "\"unitTypeId\":\"" << data.getUnitTypeId() << "\","
                    << "\"floorArea\":\"" << data.getFloorArea() << "\","
                    << "\"freehold\":\"" << data.isFreehold() << "\""
                    << "}";
        }

        std::ostream& operator<<(std::ostream& strm, PotentialProject& data)
        {
            std::stringstream unitsStr;
            unitsStr << "[";
            std::vector<PotentialUnit>::const_iterator itr;
            const std::vector<PotentialUnit>& units = data.getUnits();

            for (itr = units.begin(); itr != units.end(); itr++)
            {
                unitsStr << (*itr) << (((itr + 1) != units.end()) ? "," : "");
            }

            unitsStr << "]";
            return strm << "{"
                    << "\"templateId\":\"" << data.getDevTemplate()->getTemplateId() << "\","
                    << "\"parcelId\":\"" << data.getParcel()->getId() << "\","
                    << "\"gpr\":\"" << data.getParcel()->getGpr() << "\","
                    << "\"landUseTypeId\":\"" << data.getDevTemplate()->getLandUseTypeId() << "\","
                    << "\"profit\":\"" << data.getProfit() << "\","
                    << "\"units\":\"" << unitsStr.str() << "\""
                    << "}";
        }
    }
}
