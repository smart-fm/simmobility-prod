/*
 * CreateOutputSchemaDao.cpp
 *
 *  Created on: Nov 23, 2015
 *      Author: gishara
 */

#include "CreateOutputSchemaDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

CreateOutputSchemaDao::CreateOutputSchemaDao(DB_Connection& connection): SqlAbstractDao<CreateOutputSchema>(connection, "","", "", "","SELECT * FROM " + connection.getSchema()+"create_output_schema", "")
{}

CreateOutputSchemaDao::~CreateOutputSchemaDao() {}

void CreateOutputSchemaDao::fromRow(Row& result, CreateOutputSchema& outObj)
{
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.tableName = result.get<std::string>("table_name", EMPTY_STR);
    outObj.query = result.get<std::string>("query", EMPTY_STR);
}

void CreateOutputSchemaDao::toRow(CreateOutputSchema& data, Parameters& outParams, bool update) {}



