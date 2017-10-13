/*
 * IndLogsumJobAssignmentDao.cpp
 *
 *  Created on: 28 Aug 2017
 *      Author: gishara
 */

#include "IndLogsumJobAssignmentDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

IndLogsumJobAssignmentDao::IndLogsumJobAssignmentDao(DB_Connection& connection): SqlAbstractDao<IndLogsumJobAssignment>(connection, EMPTY_STR,EMPTY_STR, EMPTY_STR, EMPTY_STR,"SELECT * FROM " + connection.getSchema()+"ind_logsums_job_assignment", EMPTY_STR) {}

IndLogsumJobAssignmentDao::~IndLogsumJobAssignmentDao() {}

void IndLogsumJobAssignmentDao::fromRow(Row& result, IndLogsumJobAssignment& outObj)
{
    outObj.individualId  = result.get<BigSerial>("individual_id",INVALID_ID);
    outObj.tazId = result.get<std::string>("taz_id",std::string());
    outObj.logsum = result.get<double>("logsum",.0);
}

void IndLogsumJobAssignmentDao::toRow(IndLogsumJobAssignment& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getIndividualId());
	outParams.push_back(data.getTazId());
	outParams.push_back(data.getLogsum());
}

std::vector<IndLogsumJobAssignment*> IndLogsumJobAssignmentDao::loadLogsumByIndividualId(BigSerial individualId)
{
	const std::string DB_GET_LOGSUMS_BY_INDIVIDUAL_ID      = "SELECT * FROM " + connection.getSchema() + "ind_logsums_job_assignment" + "  WHERE individual_id = :v1;";
	db::Parameters params;
	params.push_back(individualId);
	std::vector<IndLogsumJobAssignment*> logsumList;
	getByQueryId(DB_GET_LOGSUMS_BY_INDIVIDUAL_ID,params,logsumList);
	return logsumList;

}


