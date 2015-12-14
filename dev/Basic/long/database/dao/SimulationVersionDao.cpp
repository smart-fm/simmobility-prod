/*
 * SimulationVersionDao.cpp
 *
 *  Created on: Nov 6, 2015
 *      Author: gishara
 */

#include "SimulationVersionDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

SimulationVersionDao::SimulationVersionDao(DB_Connection& connection): SqlAbstractDao<SimulationVersion>( connection, EMPTY_STR,EMPTY_STR,EMPTY_STR, EMPTY_STR,DB_GETALL_SIMVERSION, EMPTY_STR){}

SimulationVersionDao::~SimulationVersionDao() {}


void SimulationVersionDao::fromRow(Row& result, SimulationVersion& outObj)
{
	outObj.id 	= result.get<BigSerial>( 	"id",   INVALID_ID	);
	outObj.scenario = result.get<std::string>(  "scenario", EMPTY_STR	);
	outObj.simulationStartDate = result.get<std::tm>("simulation_start_date",std::tm());
	outObj.simStoppedTick = result.get<int>("sim_stopped_tick",0);

}

void SimulationVersionDao::toRow(SimulationVersion& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getId());
	outParams.push_back(data.getScenario());
	outParams.push_back(data.getSimulationStartDate());
	outParams.push_back(data.getSimStoppedTick());
}

void SimulationVersionDao::insertSimulationVersion(SimulationVersion& objToInsert,std::string schema)
{

	const std::string DB_INSERT_SIM_VERSION = "INSERT INTO " + APPLY_SCHEMA(schema, ".simulation_version")
	        		+ " (" + "id" + ", " + "scenario" + ", " + "simulation_start_date" + ", " + "sim_stopped_tick"
	        		+ ") VALUES (:v1, :v2, :v3, :v4)";
	insertViaQuery(objToInsert,DB_INSERT_SIM_VERSION);

}

std::vector<SimulationVersion*> SimulationVersionDao::getAllSimulationVersions(std::string schema)
{
	const std::string queryStr = "SELECT * FROM " + APPLY_SCHEMA(schema, ".simulation_version") + LIMIT;
	std::vector<SimulationVersion*> simulationVersionList;
	getByQuery(queryStr,simulationVersionList);
	return simulationVersionList;
}



