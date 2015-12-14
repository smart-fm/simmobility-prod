/*
 * Project.hpp
 *
 *  Created on: Aug 20, 2014
 *      Author: gishara<gishara@smart.mit.edu>
 */
#pragma once

#include <ctime>
#include "Types.hpp"

namespace sim_mob
{

namespace long_term
{

/*
 * Project plain object
 */
class Project
{
public:
	Project(BigSerial projectId = INVALID_ID,
			BigSerial parcelId = INVALID_ID,
			BigSerial developerId = INVALID_ID,
			BigSerial templateId = INVALID_ID,
			std::string projectName = EMPTY_STR,
			std::tm constructionDate = std::tm(),
			std::tm completionDate = std::tm(),
			double constructionCost = .0f,
			double demolitionCost = .0f,
			double totalCost = .0f,
			double fmLotSize = .0f,
			std::string grossRatio = EMPTY_STR,
			double grossArea = .0f,
			int currTick = 0,
			std::tm plannedDate = std::tm(),
			std::string projectStatus = EMPTY_STR)	;
	virtual ~Project();

	Project( const Project &source);
	Project& operator=(const Project& source);


	double getFmLotSize() const;
	void setFmLotSize(double fmLotSize);
	std::tm getCompletionDate() const;
	void setCompletionDate(std::tm completionDate);
	double getConstructionCost() const;
	void setConstructionCost(double constructionCost);
	std::tm getConstructionDate() const;
	void setConstructionDate(std::tm constructionDate);
	double getDemolitionCost() const;
	void setDemolitionCost(double demolitionCost);
	BigSerial getDeveloperId() const;
	void setDeveloperId(BigSerial developerId);
	double getGrossArea() const;
	void setGrossArea(double grossArea);
	std::string getGrossRatio() const;
	void setGrossRatio(std::string grossRatio);
	BigSerial getParcelId() const;
	void setParcelId(BigSerial parcelId);
	BigSerial getProjectId() const;
	void setProjectId(BigSerial projectId);
	std::string getProjectName() const;
	void setProjectName(std::string projectName);
	BigSerial getTemplateId() const;
	void setTemplateId(BigSerial templateId);
	double getTotalCost() const;
	void setTotalCost(double totalCost);
	int getCurrTick() const;
	void setCurrTick(int currentTick);
	std::tm getPlannedDate() const;
	void setPlannedDate(std::tm lastPlannedDate);
	std::string getProjectStatus() const;
	void setProjectStatus(std::string prjStatus);

	/* Operator to print the Parcel Match data.*/
	friend std::ostream& operator<<(std::ostream& strm, const Project& data);



private:
	friend class ProjectDao;
	BigSerial projectId;
	BigSerial parcelId;
	BigSerial developerId;
	BigSerial templateId;
	std::string projectName;
	std::tm constructionDate;
	std::tm completionDate;
	double constructionCost;
	double demolitionCost;
	double totalCost;
	double fmLotSize;
	std::string grossRatio;
	double grossArea;
	int currTick;
	std::tm plannedDate;
	std::string projectStatus;

};


}

}
