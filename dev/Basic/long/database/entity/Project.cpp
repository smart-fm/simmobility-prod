/*
 * Project.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: gishara
 */
#include "Project.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

Project::Project(BigSerial projectId,BigSerial parcelId,BigSerial developerId,
	   BigSerial templateId,std::string projectName,std::tm constructionDate,
       std::tm completionDate,double constructionCost,double demolitionCost,double totalCost,
       double fmLotSize,std::string grossRatio,double grossArea,int currTick,std::tm plannedDate, std::string projectStatus):
	   projectId(projectId),parcelId(parcelId),developerId(developerId),templateId(templateId),
	   projectName(projectName),constructionDate(constructionDate),completionDate(completionDate),
	   constructionCost(constructionCost),demolitionCost(demolitionCost),totalCost(totalCost),
	   fmLotSize(fmLotSize),grossRatio(grossRatio),grossArea(grossArea),currTick(currTick),plannedDate(plannedDate),projectStatus(projectStatus){

}

Project::~Project() {}

Project::Project( const Project &source)
{
	this->projectId = source.projectId;
	this->parcelId = source.parcelId;
	this->developerId = source.developerId;
	this->templateId = source.templateId;
	this->projectName = source.projectName;
	this->constructionDate = source.constructionDate;
	this->completionDate = source.completionDate;
	this->constructionCost = source.constructionCost;
	this->demolitionCost = source.demolitionCost;
	this->totalCost = source.totalCost;
	this->fmLotSize = source.fmLotSize;
	this->grossRatio = source.grossRatio;
	this->grossArea = source.grossArea;
	this->currTick = source.currTick;
	this->plannedDate = source.plannedDate;
	this->projectStatus = source.projectName;

}

Project& Project::operator=(const Project& source)
{
	this->projectId = source.projectId;
	this->parcelId = source.parcelId;
	this->developerId = source.developerId;
	this->templateId = source.templateId;
	this->projectName = source.projectName;
	this->constructionDate = source.constructionDate;
	this->completionDate = source.completionDate;
	this->constructionCost = source.constructionCost;
	this->demolitionCost = source.demolitionCost;
	this->totalCost = source.totalCost;
	this->fmLotSize = source.fmLotSize;
	this->grossRatio = source.grossRatio;
	this->grossArea = source.grossArea;
	this->currTick = source.currTick;
	this->plannedDate = source.plannedDate;
	this->projectStatus = source.projectName;

	return *this;
}

double Project::getFmLotSize() const{
	return fmLotSize;
}

void Project::setFmLotSize(double fmLotSize) {
	this->fmLotSize = fmLotSize;
}

std::tm Project::getCompletionDate() const{
	return completionDate;
}

void Project::setCompletionDate(
		std::tm completionDate) {
	this->completionDate = completionDate;
}

double Project::getConstructionCost() const{
	return constructionCost;
}

void Project::setConstructionCost(double constructionCost) {
	this->constructionCost = constructionCost;
}

std::tm Project::getConstructionDate() const{
	return constructionDate;
}

void Project::setConstructionDate(std::tm constructionDate) {
	this->constructionDate = constructionDate;
}

double Project::getDemolitionCost() const{
	return demolitionCost;
}

void Project::setDemolitionCost(
		double demolitionCost) {
	this->demolitionCost = demolitionCost;
}

BigSerial Project::getDeveloperId() const{
	return this->developerId;
}

void Project::setDeveloperId(BigSerial developerId) {
	this->developerId = developerId;
}

double Project::getGrossArea() const{
	return grossArea;
}

void Project::setGrossArea(double grossArea) {
	this->grossArea = grossArea;
}

std::string Project::getGrossRatio() const{
	return grossRatio;
}

void Project::setGrossRatio(std::string grossRatio) {
	this->grossRatio = grossRatio;
}

BigSerial Project::getParcelId() const{
	return parcelId;
}

void Project::setParcelId(BigSerial parcelId) {
	this->parcelId = parcelId;
}

BigSerial Project::getProjectId() const{
	return projectId;
}

void Project::setProjectId(BigSerial projectId) {
	this->projectId = projectId;
}

std::string Project::getProjectName() const{
	return projectName;
}

void Project::setProjectName(std::string projectName){
	this->projectName = projectName;
}

BigSerial Project::getTemplateId() const{
	return templateId;
}

void Project::setTemplateId(BigSerial templateId) {
	this->templateId = templateId;
}

double Project::getTotalCost() const{
	return totalCost;
}

void Project::setTotalCost(double totalCost) {
	this->totalCost = totalCost;
}

int Project::getCurrTick() const{
	return currTick;
}

void Project::setCurrTick(int currentTick) {
	this->currTick = currentTick;
}

std::tm Project::getPlannedDate() const{
	return this->plannedDate;
}

void Project::setPlannedDate(std::tm lastPlannedDate)
{
	this->plannedDate = lastPlannedDate;
}

std::string Project::getProjectStatus() const
{
	return this->projectStatus;
}

void Project::setProjectStatus(std::string prjStatus)
{
	this->projectStatus = prjStatus;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const Project& data)
        {
            return strm << "{"
						<< "\"fm_project_id\":\"" << data.projectId << "\","
						<< "\"fm_parcel_id\":\"" << data.parcelId << "\","
						<< "\"developer_id\":\"" << data.developerId << "\","
						<< "\"template_id\":\"" << data.templateId << "\","
						<< "\"project_name\":\"" << data.projectName << "\","
						<< "\"construction_date\":\"" << data.constructionDate.tm_year << "\","
						<< "\"construction_cost\":\"" << data.constructionCost << "\","
						<< "\"demolition_cost\":\"" << data.demolitionCost << "\","
						<< "\"total_cost\":\"" << data.totalCost << "\","
						<< "\"fmLotSize\":\"" << data.fmLotSize << "\","
						<< "\"gross_ratio\":\"" << data.grossRatio << "\","
						<< "\"gross_area\":\"" << data.grossArea << "\","
						<< "\"planned_date\":\"" << data.plannedDate.tm_year << "\","
						<< "\"project_status\":\"" << data.projectStatus << "\","
						<< "\"plannedDate\":\"" << data.plannedDate.tm_year << "\","
						<< "}";
       }
    }
}


