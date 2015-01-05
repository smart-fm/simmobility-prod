/*
 * Project.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: gishara
 */
#include "Project.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

Project::Project(BigSerial projectId,BigSerial parcelIid,BigSerial developerId,
	   BigSerial templateId,std::string projectName,std::tm constructionDate,
       std::tm completionDate,double constructionCost,double demolitionCost,double totalCost,
       double fmLotSize,std::string grossRatio,double grossArea,int currTick):
	   projectId(projectId),parcelId(parcelIid),developerId(developerId),templateId(templateId),
	   projectName(projectName),constructionDate(constructionDate),completionDate(completionDate),
	   constructionCost(constructionCost),demolitionCost(demolitionCost),totalCost(totalCost),
	   fmLotSize(fmLotSize),grossRatio(grossRatio),grossArea(grossArea),currTick(currTick){

}

Project::~Project() {
}

double Project::getFmLotSize(){
	return fmLotSize;
}

void Project::setFmLotSize(double fmLotSize) {
	this->fmLotSize = fmLotSize;
}

std::tm Project::getCompletionDate() {
	return completionDate;
}

void Project::setCompletionDate(
		std::tm completionDate) {
	this->completionDate = completionDate;
}

double Project::getConstructionCost(){
	return constructionCost;
}

void Project::setConstructionCost(double constructionCost) {
	this->constructionCost = constructionCost;
}

std::tm Project::getConstructionDate(){
	return constructionDate;
}

void Project::setConstructionDate(std::tm constructionDate) {
	this->constructionDate = constructionDate;
}

double Project::getDemolitionCost(){
	return demolitionCost;
}

void Project::setDemolitionCost(
		double demolitionCost) {
	this->demolitionCost = demolitionCost;
}

BigSerial Project::getDeveloperId(){
	return this->developerId;
}

void Project::setDeveloperId(BigSerial developerId) {
	this->developerId = developerId;
}

double Project::getGrossArea(){
	return grossArea;
}

void Project::setGrossArea(double grossArea) {
	this->grossArea = grossArea;
}

std::string Project::getGrossRatio(){
	return grossRatio;
}

void Project::setGrossRatio(std::string grossRatio) {
	this->grossRatio = grossRatio;
}

BigSerial Project::getParcelId(){
	return parcelId;
}

void Project::setParcelId(BigSerial parcelId) {
	this->parcelId = parcelId;
}

BigSerial Project::getProjectId(){
	return projectId;
}

void Project::setProjectId(BigSerial projectId) {
	this->projectId = projectId;
}

std::string Project::getProjectName(){
	return projectName;
}

void Project::setProjectName(std::string projectName){
	this->projectName = projectName;
}

BigSerial Project::getTemplateId(){
	return templateId;
}

void Project::setTemplateId(BigSerial templateId) {
	this->templateId = templateId;
}

double Project::getTotalCost() {
	return totalCost;
}

void Project::setTotalCost(double totalCost) {
	this->totalCost = totalCost;
}

int Project::getCurrTick() {
	return currTick;
}

void Project::setCurrTick(int currentTick) {
	this->currTick = currentTick;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const Project& data) {
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
						<< "}";
        	}
        }
    }


