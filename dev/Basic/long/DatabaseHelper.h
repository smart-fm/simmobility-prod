/* 
 * File:   DatabaseHelper.h
 * Author: gandola
 *
 * Created on April 24, 2013, 12:14 PM
 */

#pragma once
#include "boost/format.hpp"

using std::string;
using boost::format;

#define APPLY_SCHEMA(schema, field) string(schema)+string(field)

/**
 * Schemas
 */
#define DB_SCHEMA_EMPTY   ""
#define DB_SCHEMA_BASELINE_2001   "baseline_2001."
const string CURRENT_SCHEMA = DB_SCHEMA_EMPTY;

/**
 * Tables
 */
const string DB_TABLE_HOUSEHOLD = APPLY_SCHEMA(CURRENT_SCHEMA, "household");
const string DB_TABLE_BUILDING = APPLY_SCHEMA(CURRENT_SCHEMA, "building");
const string DB_TABLE_UNIT = APPLY_SCHEMA(CURRENT_SCHEMA, "unit");

/**
 * Fields
 */
const string DB_FIELD_CARS = "cars";

const string DB_FIELD_PERSONS = "persons";
const string DB_FIELD_RACE_ID = "race_id";
const string DB_FIELD_PERSON_ID = "person_id";
const string DB_FIELD_AGE = "age";
const string DB_FIELD_EMPLOYMENT_STATUS = "employment_status";
const string DB_FIELD_HBO_TRIPS = "hbo_trips";
const string DB_FIELD_HBW_TRIPS = "hbw_trips";
const string DB_FIELD_JOB_ID = "job_id";
const string DB_FIELD_ZONE_ID = "zone_id";
const string DB_FIELD_MODE = "mode";
const string DB_FIELD_WORK_AT_HOME = "work_at_home";
const string DB_FIELD_SEX = "sex";
const string DB_FIELD_EARNING = "earning";
const string DB_FIELD_BUILDING_TYPE_ID = "building_type_id";
const string DB_FIELD_GENERIC_BUILDING_TYPE_ID = "generic_building_type_id";
const string DB_FIELD_UNIT_NAME = "unit_name";
const string DB_FIELD_BUILDING_TYPE_NAME = "building_type_name";
const string DB_FIELD_IS_RESIDENTIAL = "is_residential";
const string DB_FIELD_GENERIC_BUILDING_TYPE_DESCRIPTION = "generic_building_type_description";
const string DB_FIELD_DESCRIPTION = "description";
const string DB_FIELD_RESIDENTIAL_UNITS = "residential_units";
const string DB_FIELD_YEAR_BUILT = "year_built";
const string DB_FIELD_PARCEL_ID = "parcel_id";

const string DB_FIELD_BUILDING_QUALITY_ID = "building_quality_id";
const string DB_FIELD_IMPROVEMENT_VALUE = "improvement_value";
const string DB_FIELD_TEMPLATE_ID = "template_id";
const string DB_FIELD_SQFT_PER_UNIT = "sqft_per_unit";
const string DB_FIELD_LAND_USE_TYPE_ID = "land_use_type_id";
const string DB_FIELD_LAND_USE_NAME = "land_use_name";
const string DB_FIELD_GENERIC_LAND_USE_NAME = "generic_land_use_type_name";
const string DB_FIELD_GENERIC_LAND_USE_TYPE_ID = "generic_land_use_type_id";


//NEW DATABASE
const string DB_FIELD_ID = "id";
const string DB_FIELD_BUILDING_ID = "building_id";
const string DB_FIELD_HOUSEHOLD_ID = "household_id";
const string DB_FIELD_TYPE = "type";
const string DB_FIELD_FIXED_PRICE = "fixed_price";
const string DB_FIELD_YEAR_OF_LAST_REMODULATION = "year_of_last_remodulation";
const string DB_FIELD_TAX_EXEMPT = "tax_exempt";
const string DB_FIELD_HAS_GARAGE = "has_garage";
const string DB_FIELD_INCOME = "income";
const string DB_FIELD_AREA = "area";
const string DB_FIELD_YEAR = "year";
const string DB_FIELD_STOREY = "storey";
const string DB_FIELD_HEDONIC_PRICE = "hedonic_price";
const string DB_FIELD_AVERAGE_INCOME = "average_income";
const string DB_FIELD_MAIN_RACE = "main_race";
const string DB_FIELD_DISTANCE_TO_CDB = "distance_to_cbd";
const string DB_FIELD_AGE_OF_HEAD = "age_of_head";
const string DB_FIELD_NUMBER_OF_CHILDREN = "number_of_children";
const string DB_FIELD_NUMBER_OF_WORKERS = "number_of_workers";
const string DB_FIELD_NUMBER_OF_CARS = "number_of_cars";
const string DB_FIELD_NUMBER_OF_INDIVIDUALS = "number_of_individuals";
const string DB_FIELD_NUMBER_OF_UNITS = "number_of_units";
const string DB_FIELD_NUMBER_OF_RESIDENTIAL_UNITS = "number_of_residential_units";
const string DB_FIELD_NUMBER_OF_BUSINESS_UNITS = "number_of_business_units";
const string DB_FIELD_NUMBER_OF_STORIES = "number_of_stories";
const string DB_FIELD_RACE = "race";
const string DB_FIELD_WEIGHT_PRICE_QUALITY = "weight_price_quality";
const string DB_FIELD_WEIGHT_STOREY = "weight_storey";
const string DB_FIELD_WEIGHT_DISTANCE_TO_CBD = "weight_distance_to_cbd";
const string DB_FIELD_WEIGHT_TYPE = "weight_type";
const string DB_FIELD_WEIGHT_AREA = "weight_area";
const string DB_FIELD_WEIGHT_TAX_EXEMPT = "weight_tax_exempt";
const string DB_FIELD_WEIGHT_YEAR_LAST_REMODULATION = "weight_year_last_remodulation";


/**
 * INSERT
 */
const string DB_INSERT_HOUSEHOLD = "INSERT INTO " + DB_TABLE_HOUSEHOLD + " ("
        + DB_FIELD_ID + ", "
        + DB_FIELD_NUMBER_OF_INDIVIDUALS + ", "
        + DB_FIELD_NUMBER_OF_WORKERS + ", "
        + DB_FIELD_NUMBER_OF_CHILDREN + ", "
        + DB_FIELD_NUMBER_OF_CARS + ", "
        + DB_FIELD_INCOME + ", "
        + DB_FIELD_AGE_OF_HEAD + ", "
        + DB_FIELD_RACE_ID + ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7, :v8, :v9)";

const string DB_INSERT_BUILDING = ""; // not defined yet...
const string DB_INSERT_UNIT = ""; // not defined yet...

/**
 * UPDATE
 */
const string DB_UPDATE_HOUSEHOLD = "UPDATE " + DB_TABLE_HOUSEHOLD + " SET "
        + DB_FIELD_NUMBER_OF_INDIVIDUALS + "= :v1, "
        + DB_FIELD_NUMBER_OF_WORKERS + "= :v2, "
        + DB_FIELD_NUMBER_OF_CHILDREN + "= :v3, "
        + DB_FIELD_NUMBER_OF_CARS + "= :v4, "
        + DB_FIELD_INCOME + "= :v5, "
        + DB_FIELD_AGE_OF_HEAD + "= :v6, "
        + DB_FIELD_RACE_ID + "= :v7 WHERE " + DB_FIELD_ID + "=:v8";

const string DB_UPDATE_BUILDING = ""; // not defined yet...
const string DB_UPDATE_UNIT = ""; // not defined yet...


/**
 * DELETE
 */
const string DB_DELETE_HOUSEHOLD = "DELETE FROM " + DB_TABLE_HOUSEHOLD + " WHERE " + DB_FIELD_ID + "=:id";
const string DB_DELETE_BUILDING = "DELETE FROM " + DB_TABLE_BUILDING + " WHERE " + DB_FIELD_ID + "=:id";
const string DB_DELETE_UNIT = "DELETE FROM " + DB_TABLE_UNIT + " WHERE " + DB_FIELD_ID + "=:id";

/**
 * GET ALL
 */
const string DB_GETALL_HOUSEHOLD = "SELECT * FROM " + DB_TABLE_HOUSEHOLD + " LIMIT 10";
const string DB_GETALL_BUILDING = "SELECT * FROM " + DB_TABLE_BUILDING + " LIMIT 10";
const string DB_GETALL_UNIT = "SELECT A.*, B.* FROM " + DB_TABLE_UNIT + " AS A, "+DB_TABLE_BUILDING+" AS B WHERE A." + DB_FIELD_BUILDING_ID + " = B."+ DB_FIELD_ID + " LIMIT 10";

/**
 * GET BY ID
 */
const string DB_GETBYID_HOUSEHOLD = "SELECT * FROM " + DB_TABLE_HOUSEHOLD + " WHERE " + DB_FIELD_ID + "=:id";
const string DB_GETBYID_BUILDING = "SELECT * FROM " + DB_TABLE_BUILDING + " WHERE " + DB_FIELD_ID + "=:id";
const string DB_GETBYID_UNIT = "SELECT A.*, B.* FROM " + DB_TABLE_UNIT + " AS A, "+DB_TABLE_BUILDING+" AS B WHERE A." + DB_FIELD_BUILDING_ID + " = B."+ DB_FIELD_ID + " AND A." + DB_FIELD_ID + " =:id";