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
: SqlAbstractDao<DevelopmentTypeTemplate>(connection, "", "", "", "", "SELECT * FROM " + connection.getSchema()+"development_type_template", "")
{}

DevelopmentTypeTemplateDao::~DevelopmentTypeTemplateDao() {
}

void DevelopmentTypeTemplateDao::fromRow(Row& result, DevelopmentTypeTemplate& outObj)
{
    outObj.developmentTypeId = result.get<BigSerial>(DB_FIELD_DEVELOPMENT_TYPE_ID, INVALID_ID);
    outObj.templateId = result.get<BigSerial>(DB_FIELD_TEMPLATE_ID, INVALID_ID);
    outObj.landUseTypeId = result.get<int>(DB_FIELD_LAND_USE_ID, 0);
}

void DevelopmentTypeTemplateDao::toRow(DevelopmentTypeTemplate& data, Parameters& outParams, bool update) {}
