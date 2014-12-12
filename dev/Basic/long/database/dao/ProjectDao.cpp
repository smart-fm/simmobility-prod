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
	 outObj.lotSize = result.get<double>( "area", .0 );
	 outObj.grossRatio = result.get<std::string>( "gross_ratio", EMPTY_STR );
	 outObj.grossArea = result.get<double>( "gross_area", .0 );
}

void ProjectDao::toRow(Project& data, Parameters& outParams, bool update) {
}


