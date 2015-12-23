/*
 * DevelopmentPlanDao.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: gishara
 */
//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DevelopmentPlanDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

DevelopmentPlanDao::DevelopmentPlanDao(DB_Connection& connection): SqlAbstractDao<DevelopmentPlan>( connection, EMPTY_STR,EMPTY_STR, EMPTY_STR, EMPTY_STR,EMPTY_STR, EMPTY_STR ) {}

DevelopmentPlanDao::~DevelopmentPlanDao() {}

void DevelopmentPlanDao::fromRow(Row& result, DevelopmentPlan& outObj)
{
    outObj.fmParcelId 		= result.get<BigSerial>("fm_parcel_id",INVALID_ID);
    outObj.unitTypeId		= result.get<int>("unit_type_id",0);
    outObj.numUnits = result.get<int>("num_units",0);
    outObj.simulationDate = result.get<std::tm>("simulation_date",std::tm());
}

void DevelopmentPlanDao::toRow(DevelopmentPlan& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getFmParcelId());
	outParams.push_back(data.getUnitTypeId());
	outParams.push_back(data.getNumUnits());
	outParams.push_back(data.getSimulationDate());
}

void DevelopmentPlanDao::insertDevelopmentPlan(DevelopmentPlan& devPlan,std::string schema)
{

	const std::string DB_INSERT_DEV_PLAN = "INSERT INTO " + APPLY_SCHEMA(schema, ".development_plans")
	        		+ " (" + "fm_parcel_id" + ", " + "unit_type_id" + ", " + "num_units" + ", " + "simulation_date"
	        		+ ") VALUES (:v1, :v2, :v3, :v4)";
	insertViaQuery(devPlan,DB_INSERT_DEV_PLAN);

}




