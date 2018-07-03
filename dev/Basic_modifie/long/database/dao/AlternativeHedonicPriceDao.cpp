//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * PopulationPerPlanningAreaDao.cpp
 *
 *  Created on: 13 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/AlternativeHedonicPriceDao.hpp>
#include "DatabaseHelper.hpp"
#include <string>

using namespace sim_mob::db;
using namespace sim_mob::long_term;

AlternativeHedonicPriceDao::AlternativeHedonicPriceDao(DB_Connection& connection): SqlAbstractDao<AlternativeHedonicPrice>( connection, "", "", "", "",
																															"SELECT * FROM " + connection.getSchema()+"getAlternativeHedonicPrice()",""){}

AlternativeHedonicPriceDao::~AlternativeHedonicPriceDao(){}

void AlternativeHedonicPriceDao::fromRow(Row& result, AlternativeHedonicPrice& outObj)
{
    outObj.planning_area	= result.get<std::string>( "planning_area_name",  "");
    outObj.dwelling_type	= result.get<int>( "dwelling_type",	 0);
    outObj.total_price		= result.get<double>( "total_price", 0.0);
    outObj.planning_area_id	= result.get<BigSerial>( "planning_area_id",  0);

	std::string strId = std::to_string( outObj.planning_area_id) + std::to_string( outObj.dwelling_type);
	int id = atoi( strId.c_str());
	outObj.id = id;
}

void AlternativeHedonicPriceDao::toRow(AlternativeHedonicPrice& data, Parameters& outParams, bool update) {}

