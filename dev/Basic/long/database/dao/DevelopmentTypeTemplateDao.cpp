//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DevelopmentTypeTemplateDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 25, 2014, 5:17 PM
 */

#include "DevelopmentTypeTemplateDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

DevelopmentTypeTemplateDao::DevelopmentTypeTemplateDao(DB_Connection& connection)
: SqlAbstractDao<DevelopmentTypeTemplate>(connection, 
        DB_TABLE_DEVELOPMENT_TYPE_TEMPLATE,
        EMPTY_STR, EMPTY_STR, EMPTY_STR,
        DB_GETALL_DEVELOPMENT_TYPE_TEMPLATES, 
        DB_GETBYID_DEVELOPMENT_TYPE_TEMPLATE) {
}

DevelopmentTypeTemplateDao::~DevelopmentTypeTemplateDao() {
}

void DevelopmentTypeTemplateDao::fromRow(Row& result, DevelopmentTypeTemplate& outObj) {
    outObj.developmentTypeId = result.get<BigSerial>(DB_FIELD_DEVELOPMENT_TYPE_ID, INVALID_ID);
    outObj.templateId = result.get<BigSerial>(DB_FIELD_TEMPLATE_ID, INVALID_ID);
    outObj.density = result.get<double>(DB_FIELD_DENSITY, 0);
}

void DevelopmentTypeTemplateDao::toRow(DevelopmentTypeTemplate& data, Parameters& outParams, bool update) {
}