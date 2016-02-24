//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * PopulationPerPlanningAreaDao.cpp
 *
 *  Created on: 13 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/PopulationPerPlanningAreaDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

PopulationPerPlanningAreaDao::PopulationPerPlanningAreaDao(DB_Connection& connection): SqlAbstractDao<PopulationPerPlanningArea>( connection, DB_FUNC_GET_POPULATION_PER_PLANNING_AREA, "", "", "",
																									  DB_GETALL_POPULATION_PER_PLANNING_AREA, DB_GETBYID_POPULATION_PER_PLANNING_AREA){}

PopulationPerPlanningAreaDao::~PopulationPerPlanningAreaDao(){}

void PopulationPerPlanningAreaDao::fromRow(Row& result, PopulationPerPlanningArea& outObj)
{
    outObj.planningAreaId	= result.get<BigSerial>( "planning_area_id", 0);
    outObj.population		= result.get<BigSerial>( "population", 		 0);
    outObj.ethnicityId		= result.get<BigSerial>( "ethnicity_id", 	 0);
    outObj.ageCategoryId	= result.get<BigSerial>( "age_category_id",  0);
    outObj.avgIncome		= result.get<double>( "avg_income", 		 0);
    outObj.avgHhSize		= result.get<BigSerial>( "avg_hhsize", 		 0);
    outObj.unitType			= result.get<int>( "unit_type",		 0);
    outObj.floorArea		= result.get<int>( "floor_area",	 0);
}

void PopulationPerPlanningAreaDao::toRow(PopulationPerPlanningArea& data, Parameters& outParams, bool update) {}

