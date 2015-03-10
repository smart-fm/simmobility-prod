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

PotentialUnit::PotentialUnit(BigSerial unitTypeId,int numUnits,double floorArea,int freehold): unitTypeId(unitTypeId),numUnits(numUnits),floorArea(floorArea),freehold(freehold) {

}

PotentialUnit::PotentialUnit( const PotentialUnit& source)
{
	this->unitTypeId = source.unitTypeId;
	this->numUnits = source.numUnits;
	this->floorArea = source.floorArea;
	this->freehold = source.freehold;
}

PotentialUnit& PotentialUnit::operator=( const PotentialUnit& source)
{
	this->unitTypeId = source.unitTypeId;
	this->numUnits = source.numUnits;
	this->floorArea = source.floorArea;
	this->freehold = source.freehold;

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

PotentialProject::PotentialProject(const DevelopmentTypeTemplate* devTemplate, const Parcel* parcel,double constructionCost, double grossArea,double tempSelectProbability,double investmentReturnRatio, double demolitionCost, double expRatio)
								  : devTemplate(devTemplate), parcel(parcel), profit(0) , constructionCost(constructionCost),grossArea(grossArea),tempSelectProbability(tempSelectProbability),
								    investmentReturnRatio(investmentReturnRatio), demolitionCost(demolitionCost), expRatio(expRatio) {}

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
	this->units = source.units;
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
	this->units = source.units;

	return *this;
}

PotentialProject::~PotentialProject() {
}

void PotentialProject::addUnit(const PotentialUnit& unit) {
    units.push_back(unit);
}

void PotentialProject::addTemplateUnitType(TemplateUnitType* templateUnitType) {

	//PrintOut("BEFORE ADDING"<<templateUnitTypes.size());
	templateUnitTypes.push_back(templateUnitType);
	//PrintOut("AFTER ADDING"<<templateUnitTypes.size());
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

const std::vector<PotentialUnit>& PotentialProject::getUnits() const {
    return units;
}

double PotentialProject::getProfit() const {
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

        std::ostream& operator<<(std::ostream& strm, const PotentialProject& data)
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
