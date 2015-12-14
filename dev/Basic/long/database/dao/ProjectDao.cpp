/*
 * PrjectDao.cpp
 *
 *  Created on: Aug 21, 2014
 *      Author: gishara
 */

#include "ProjectDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

ProjectDao::ProjectDao(DB_Connection& connection)
: SqlAbstractDao<Project>(connection, DB_TABLE_PROJECT,
DB_INSERT_PROJECT, DB_UPDATE_PROJECT, DB_DELETE_PROJECT,
DB_GETALL_PROJECTS, EMPTY_STR) {}

ProjectDao::~ProjectDao() {}

void ProjectDao::fromRow(Row& result, Project& outObj)
{
	 outObj.projectId = result.get<BigSerial>( "fm_project_id", INVALID_ID);
	 outObj.parcelId = result.get<BigSerial>("fm_parcel_id", INVALID_ID);
	 outObj.developerId = result.get<BigSerial>( "developer_id", INVALID_ID);
	 outObj.templateId = result.get<BigSerial>("template_id", INVALID_ID);
	 outObj.projectName = result.get<std::string>( "project_name", EMPTY_STR );
	 outObj.constructionDate = result.get<std::tm>( "construction_date", std::tm() );
	 outObj.completionDate = result.get<std::tm>( "completion_date", std::tm() );
	 outObj.constructionCost = result.get<double>( "construction_cost", .0 );
	 outObj.demolitionCost = result.get<double>( "demolition_cost", .0 );
	 outObj.totalCost = result.get<double>( "total_cost", .0);
	 outObj.fmLotSize = result.get<double>( "fm_lot_size", .0 );
	 outObj.grossRatio = result.get<std::string>( "gross_ratio", EMPTY_STR );
	 outObj.grossArea = result.get<double>("gross_area",.0);
	 outObj.plannedDate = result.get<std::tm>("planned_date",std::tm());
	 outObj.projectStatus = result.get<std::string>("project_status",EMPTY_STR);
}

void ProjectDao::toRow(Project& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getProjectId());
	outParams.push_back(data.getParcelId());
	outParams.push_back(data.getDeveloperId());
	outParams.push_back(data.getTemplateId());
	outParams.push_back(data.getProjectName());
	outParams.push_back(data.getConstructionDate());
	outParams.push_back(data.getCompletionDate());
	outParams.push_back(data.getConstructionCost());
	outParams.push_back(data.getDemolitionCost());
	outParams.push_back(data.getTotalCost());
	outParams.push_back(data.getFmLotSize());
	outParams.push_back(data.getGrossRatio());
	outParams.push_back(data.getGrossArea());
	outParams.push_back(data.getPlannedDate());
	outParams.push_back(data.getProjectStatus());
}

void ProjectDao::insertProject(Project& project,std::string schema)
{

	const std::string DB_INSERT_PROJECT_OP = "INSERT INTO " + APPLY_SCHEMA(schema, ".fm_project")
                        		+ " (" + "fm_project_id" + ", " + "fm_parcel_id" + ", " + "developer_id"
                        		+ ", " + "template_id" + ", " + "project_name" + ", "
                        		+ "construction_date" + ", " + "completion_date"  + ", "+ "construction_cost" + ", " + "demolition_cost" + ", "
                        		+ "total_cost" + ", " + "fm_lot_size" + ", " + "gross_ratio"  + ", " + "gross_area"
                        		+ ", "+ "planned_date"  + ", " + "project_status"
                        		+ ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7, :v8, :v9, :v10, :v11, :v12, :v13, :v14, :v15)";
	insertViaQuery(project,DB_INSERT_PROJECT_OP);

}

