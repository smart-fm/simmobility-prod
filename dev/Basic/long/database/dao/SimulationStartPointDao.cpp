/*
 * SimulationVersionDao.cpp
 *
 *  Created on: Nov 6, 2015
 *      Author: gishara
 */

#include "SimulationStartPointDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

SimulationStartPointDao::SimulationStartPointDao(DB_Connection& connection): SqlAbstractDao<SimulationStartPoint>( connection, DB_TABLE_SIM_START_POINT,EMPTY_STR,EMPTY_STR, EMPTY_STR,DB_GETALL_SIMSTARTPOINT, EMPTY_STR){}

SimulationStartPointDao::~SimulationStartPointDao() {}


void SimulationStartPointDao::fromRow(Row& result, SimulationStartPoint& outObj)
{
	outObj.id 	= result.get<BigSerial>( 	"id",   INVALID_ID	);
	outObj.scenario = result.get<std::string>(  "scenario", EMPTY_STR	);
	outObj.simulationStartDate = result.get<std::tm>("simulation_start_date",std::tm());
	outObj.mainSchemaVersion = result.get<std::string>(  "main_schema_version", EMPTY_STR	);
	outObj.configSchemaVersion = result.get<std::string>(  "configuration_schema_version", EMPTY_STR	);
	outObj.calibrationSchemaVersion = result.get<std::string>(  "calibration_schema_version", EMPTY_STR	);
	outObj.geometrySchemaVersion = result.get<std::string>(  "geometry_schema_version", EMPTY_STR	);
	outObj.simStoppedTick = result.get<int>("simulation_stopped_tick",0);

}

void SimulationStartPointDao::toRow(SimulationStartPoint& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getId());
	outParams.push_back(data.getScenario());
	outParams.push_back(data.getSimulationStartDate());
	outParams.push_back(data.getMainSchemaVersion());
	outParams.push_back(data.getConfigSchemaVersion());
	outParams.push_back(data.getCalibrationSchemaVersion());
	outParams.push_back(data.getGeometrySchemaVersion());
	outParams.push_back(data.getSimStoppedTick());
}

void SimulationStartPointDao::insertSimulationStartPoint(SimulationStartPoint& objToInsert,std::string schema)
{

	const std::string DB_INSERT_SIM_VERSION = "INSERT INTO " + APPLY_SCHEMA(schema, ".simulation_start_point")
	        		+ " (" + "id" + ", " + "scenario" + ", " + "simulation_start_date" + ", " + "main_schema_version" + ", " + "configuration_schema_version" + ", " + "calibration_schema_version" + ", " + "geometry_schema_version" + ", " + "simulation_stopped_tick"
	        		+ ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7, :v8)";
	insertViaQuery(objToInsert,DB_INSERT_SIM_VERSION);

}

std::vector<SimulationStartPoint*> SimulationStartPointDao::getAllSimulationStartPoints(std::string schema)
{
	const std::string queryStr = "SELECT * FROM " + APPLY_SCHEMA(schema, ".simulation_start_point") + LIMIT;
	std::vector<SimulationStartPoint*> simulationStartPointsList;
	getByQuery(queryStr,simulationStartPointsList);
	return simulationStartPointsList;
}



