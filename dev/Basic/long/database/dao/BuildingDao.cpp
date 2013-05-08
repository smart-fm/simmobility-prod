/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   IndividualDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "BuildingDao.hpp"
#include "DatabaseHelper.h"

using namespace sim_mob;
using namespace sim_mob::long_term;

BuildingDao::BuildingDao(DBConnection* connection)
: AbstractDao<Building>(connection, DB_TABLE_BUILDING,
DB_INSERT_BUILDING, DB_UPDATE_BUILDING, DB_DELETE_BUILDING,
DB_GETALL_BUILDING, DB_GETBYID_BUILDING) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(Building, BuildingDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(Building, BuildingDao::ToRow);
}

BuildingDao::~BuildingDao() {
}

void BuildingDao::FromRow(Row& result, Building& outObj) {
    outObj.id = result.get<int>(DB_FIELD_BUILDING_ID);
    outObj.improvementValue = result.get<int>(DB_FIELD_IMPROVEMENT_VALUE);
    outObj.landArea = result.get<double>(DB_FIELD_LAND_AREA);
    outObj.nonResidentialSqft = result.get<int>(DB_FIELD_NON_RESIDENTIAL_SQFT);
    outObj.parcelId = result.get<int>(DB_FIELD_PARCEL_ID);
    outObj.qualityId = result.get<int>(DB_FIELD_BUILDING_QUALITY_ID);
    outObj.residentialUnits = result.get<int>(DB_FIELD_RESIDENTIAL_UNITS);
    outObj.stories = result.get<int>(DB_FIELD_STORIES);
    outObj.taxExempt = (result.get<int>(DB_FIELD_TAX_EXEMPT) == 0) ? false : true;
    outObj.templateId = result.get<int>(DB_FIELD_TEMPLATE_ID);
    outObj.typeId = result.get<int>(DB_FIELD_BUILDING_TYPE_ID);
    outObj.unitSqft = result.get<int>(DB_FIELD_SQFT_PER_UNIT);
    outObj.year = result.get<int>(DB_FIELD_YEAR_BUILT);
}

void BuildingDao::ToRow(Building& data, Parameters& outParams, bool update) {
}