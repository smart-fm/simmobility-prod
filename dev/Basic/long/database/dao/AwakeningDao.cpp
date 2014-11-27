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

AwakeningDao::AwakeningDao(DB_Connection& connection): SqlAbstractDao<Awakening>( connection, DB_TABLE_AWAKENING, DB_INSERT_AWAKENING, DB_UPDATE_AWAKENING, DB_DELETE_AWAKENING, DB_GETALL_AWAKENING, DB_GETBYID_AWAKENING ){}

AwakeningDao::~AwakeningDao(){}

void AwakeningDao::fromRow(Row& result, Awakening& outObj)
{
    outObj.id	= result.get<BigSerial>( "id", INVALID_ID);
    outObj.class1	= result.get<double>( "class1", .0);
    outObj.class2	= result.get<double>( "class2", .0);
    outObj.class3	= result.get<double>( "class3", .0);

    outObj.awaken_class1	= result.get<double>( "awaken_class1", .0);
    outObj.awaken_class2	= result.get<double>( "awaken_class2", .0);
    outObj.awaken_class3	= result.get<double>( "awaken_class3", .0);
}

void AwakeningDao::toRow(Awakening& data, Parameters& outParams, bool update) {}


