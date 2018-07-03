//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * AwakeningDao.cpp
 *
 *  Created on: 24 Nov, 2014
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *
 */


#include "AwakeningDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

AwakeningDao::AwakeningDao(DB_Connection& connection): SqlAbstractDao<Awakening>( connection, "", "", "", "", "SELECT * FROM " + connection.getSchema()+"hh_awakening_probability", ""){}



AwakeningDao::~AwakeningDao(){}

void AwakeningDao::fromRow(Row& result, Awakening& outObj)
{
    outObj.id	= result.get<BigSerial>( "syn12", INVALID_ID);
    outObj.class1	= result.get<double>( "class1", .0);
    outObj.class2	= result.get<double>( "class2", .0);
    outObj.class3	= result.get<double>( "class3", .0);

    outObj.awakenClass1	= result.get<double>( "awake_cl1", .0);
    outObj.awakenClass2	= result.get<double>( "awake_cl2", .0);
    outObj.awakenClass3	= result.get<double>( "awake_cl3", .0);
}

void AwakeningDao::toRow(Awakening& data, Parameters& outParams, bool update) {}


