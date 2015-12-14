/*
 * StatusOfWorldDao.cpp
 *
 *  Created on: Nov 13, 2015
 *      Author: gishara
 */
#include "EncodedParamsBySimulationDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

EncodedParamsBySimulationDao::EncodedParamsBySimulationDao(DB_Connection& connection): SqlAbstractDao<EncodedParamsBySimulation>(connection, DB_TABLE_ENCODED_PARAMS_BY_SIMULATION, EMPTY_STR,EMPTY_STR, EMPTY_STR, EMPTY_STR, EMPTY_STR) {
}

EncodedParamsBySimulationDao::~EncodedParamsBySimulationDao() {
}

void EncodedParamsBySimulationDao::fromRow(Row& result, EncodedParamsBySimulation& outObj)
{
	outObj.simVersionId = result.get<BigSerial>("sim_version_id", INVALID_ID);
	outObj.postcode = result.get<BigSerial>("postcode", INVALID_ID);
	outObj.buildingId = result.get<BigSerial>("building_id", INVALID_ID);
	outObj.unitId = result.get<BigSerial>("unit_id",INVALID_ID);
	outObj.projectId = result.get<BigSerial>("project_id",INVALID_ID);
}

void EncodedParamsBySimulationDao::toRow(EncodedParamsBySimulation& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getSimVersionId());
	outParams.push_back(data.getPostcode());
	outParams.push_back(data.getBuildingId());
	outParams.push_back(data.getUnitId());
	outParams.push_back(data.getProjectId());
}

void EncodedParamsBySimulationDao::insertEncodedParams(EncodedParamsBySimulation& encodedParams,std::string schema)
{

	const std::string DB_INSERT_ENCODED_PARAMS = "INSERT INTO " + APPLY_SCHEMA(schema, ".encoded_params_by_simulation")
	        		+ " (" + "sim_version_id" + ", " + "postcode" + ", " + "building_id"+ ", " + "unit_id" + ", " + "project_id"
	        		+ ") VALUES (:v1, :v2, :v3, :v4, :v5)";
	insertViaQuery(encodedParams,DB_INSERT_ENCODED_PARAMS);

}



