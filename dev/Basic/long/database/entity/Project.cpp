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
       double area,double grossRatio,double grossArea):
	   projectId(projectId),parcelId(parcelIid),developerId(developerId),templateId(templateId),
	   projectName(projectName),constructionDate(constructionDate),completionDate(completionDate),
	   constructionCost(constructionCost),demolitionCost(demolitionCost),totalCost(totalCost),
	   area(area),grossRatio(grossRatio),grossArea(grossArea){

}

Project::~Project() {
}

double sim_mob::long_term::Project::getArea(){
	return area;
}

void sim_mob::long_term::Project::setArea(double area) {
	this->area = area;
}

std::tm sim_mob::long_term::Project::getCompletionDate() {
	return completionDate;
}

void sim_mob::long_term::Project::setCompletionDate(
		std::tm completionDate) {
	this->completionDate = completionDate;
}

double sim_mob::long_term::Project::getConstructionCost(){
	return constructionCost;
}

void sim_mob::long_term::Project::setConstructionCost(double constructionCost) {
	this->constructionCost = constructionCost;
}

std::tm sim_mob::long_term::Project::getConstructionDate(){
	return constructionDate;
}

void sim_mob::long_term::Project::setConstructionDate(std::tm constructionDate) {
	this->constructionDate = constructionDate;
}

double sim_mob::long_term::Project::getDemolitionCost(){
	return demolitionCost;
}

void sim_mob::long_term::Project::setDemolitionCost(
		double demolitionCost) {
	this->demolitionCost = demolitionCost;
}

BigSerial sim_mob::long_term::Project::getDeveloperId(){
	return this->developerId;
}

void sim_mob::long_term::Project::setDeveloperId(BigSerial developerId) {
	this->developerId = developerId;
}

double sim_mob::long_term::Project::getGrossArea(){
	return grossArea;
}

void sim_mob::long_term::Project::setGrossArea(double grossArea) {
	this->grossArea = grossArea;
}

double sim_mob::long_term::Project::getGrossRatio(){
	return grossRatio;
}

void sim_mob::long_term::Project::setGrossRatio(double grossRatio) {
	this->grossRatio = grossRatio;
}

BigSerial sim_mob::long_term::Project::getParcelId(){
	return parcelId;
}

void sim_mob::long_term::Project::setParcelId(BigSerial parcelId) {
	this->parcelId = parcelId;
}

BigSerial sim_mob::long_term::Project::getProjectId(){
	return projectId;
}

void sim_mob::long_term::Project::setProjectId(BigSerial projectId) {
	this->projectId = projectId;
}

std::string sim_mob::long_term::Project::getProjectName(){
	return projectName;
}

BigSerial sim_mob::long_term::Project::getTemplateId(){
	return templateId;
}

void sim_mob::long_term::Project::setTemplateId(BigSerial templateId) {
	this->templateId = templateId;
}

double sim_mob::long_term::Project::getTotalCost() {
	return totalCost;
}

void sim_mob::long_term::Project::setTotalCost(double totalCost) {
	this->totalCost = totalCost;
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
						<< "\"area\":\"" << data.area << "\","
						<< "\"gross_ratio\":\"" << data.grossRatio << "\","
						<< "\"gross_area\":\"" << data.grossArea << "\","
						<< "}";
        	}
        }
    }


