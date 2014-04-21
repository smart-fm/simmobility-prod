//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   TemplateUnitTypeDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 25, 2014, 5:17 PM
 */

#include "TemplateUnitTypeDao.hpp"
#include "DatabaseHelper.hpp"
#include "database/entity/TemplateUnitType.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

TemplateUnitTypeDao::TemplateUnitTypeDao(DB_Connection& connection)
: SqlAbstractDao<TemplateUnitType>(connection, 
        DB_TABLE_TEMPLATE_UNIT_TYPE,
        EMPTY_STR, EMPTY_STR, EMPTY_STR,
        DB_GETALL_TEMPLATE_UNIT_TYPE, 
        DB_GETBYID_TEMPLATE_UNIT_TYPE) {
}

TemplateUnitTypeDao::~TemplateUnitTypeDao() {
}

void TemplateUnitTypeDao::fromRow(Row& result, TemplateUnitType& outObj) {
    outObj.templateId = result.get<BigSerial>(DB_FIELD_TEMPLATE_ID, INVALID_ID);
    outObj.unitTypeId = result.get<BigSerial>(DB_FIELD_UNIT_TYPE_ID, INVALID_ID);
    outObj.proportion = result.get<int>(DB_FIELD_PROPORTION, 0);
}

void TemplateUnitTypeDao::toRow(TemplateUnitType& data, Parameters& outParams, bool update) {
}