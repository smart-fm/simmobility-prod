//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   TemplateDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 5, 2013, 5:17 PM
 */

#include "TemplateDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

TemplateDao::TemplateDao(DB_Connection& connection)
: SqlAbstractDao<Template>(connection, DB_TABLE_TEMPLATE,
EMPTY_STR, EMPTY_STR, EMPTY_STR,
DB_GETALL_TEMPLATES, DB_GETBYID_TEMPLATE) {
}

TemplateDao::~TemplateDao() {
}

void TemplateDao::fromRow(Row& result, Template& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.name = result.get<std::string>(DB_FIELD_NAME, EMPTY_STR);
}

void TemplateDao::toRow(Template& data, Parameters& outParams, bool update) {
}