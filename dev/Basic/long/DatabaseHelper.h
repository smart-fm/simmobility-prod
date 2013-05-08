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
#define DB_SCHEMA_BASELINE_2001   "baseline_2001."
const string CURRENT_SCHEMA = DB_SCHEMA_BASELINE_2001;

/**
 * Tables
 */
const string DB_TABLE_HOUSEHOLD = APPLY_SCHEMA(CURRENT_SCHEMA, "households");
const string DB_TABLE_INDIVIDUAL = APPLY_SCHEMA(CURRENT_SCHEMA, "persons");
const string DB_TABLE_BUILDING_TYPE = APPLY_SCHEMA(CURRENT_SCHEMA, "building_types");

/**
 * Fields
 */
const string DB_FIELD_HOUSEHOLD_ID = "household_id";
const string DB_FIELD_BUILDING_ID = "building_id";
const string DB_FIELD_CARS = "cars";
const string DB_FIELD_WORKERS = "workers";
const string DB_FIELD_AGE_OF_HEAD = "age_of_head";
const string DB_FIELD_CHILDREN = "children";
const string DB_FIELD_INCOME = "income";
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


/**
 * INSERT
 */
const string DB_INSERT_HOUSEHOLD = "INSERT INTO " + DB_TABLE_HOUSEHOLD + " ("
        + DB_FIELD_HOUSEHOLD_ID + ", "
        + DB_FIELD_BUILDING_ID + ", "
        + DB_FIELD_CARS + ", "
        + DB_FIELD_WORKERS + ", "
        + DB_FIELD_AGE_OF_HEAD + ", "
        + DB_FIELD_CHILDREN + ", "
        + DB_FIELD_INCOME + ", "
        + DB_FIELD_PERSONS + ", "
        + DB_FIELD_RACE_ID + ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7, :v8, :v9)";

const string DB_INSERT_INDIVIDUAL = ""; // not defined yet...
const string DB_INSERT_BUILDING_TYPE = ""; // not defined yet...

/**
 * UPDATE
 */
const string DB_UPDATE_HOUSEHOLD = "UPDATE " + DB_TABLE_HOUSEHOLD + " SET "
        + DB_FIELD_BUILDING_ID + "= :v1, "
        + DB_FIELD_CARS + "= :v2, "
        + DB_FIELD_WORKERS + "= :v3, "
        + DB_FIELD_AGE_OF_HEAD + "= :v4, "
        + DB_FIELD_CHILDREN + "= :v5, "
        + DB_FIELD_INCOME + "= :v6, "
        + DB_FIELD_PERSONS + "= :v7, "
        + DB_FIELD_RACE_ID + "= :v8 WHERE " + DB_FIELD_HOUSEHOLD_ID + "=:v9";

const string DB_UPDATE_INDIVIDUAL = ""; // not defined yet...
const string DB_UPDATE_BUILDING_TYPE = ""; // not defined yet...


/**
 * DELETE
 */
const string DB_DELETE_HOUSEHOLD = "DELETE FROM " + DB_TABLE_HOUSEHOLD + " WHERE " + DB_FIELD_HOUSEHOLD_ID + "=:id";
const string DB_DELETE_INDIVIDUAL = "DELETE FROM " + DB_TABLE_INDIVIDUAL + " WHERE " + DB_FIELD_PERSON_ID + "=:id";
const string DB_DELETE_BUILDING_TYPE = "DELETE FROM " + DB_TABLE_BUILDING_TYPE + " WHERE " + DB_FIELD_BUILDING_TYPE_ID + "=:id";

/**
 * GET ALL
 */
const string DB_GETALL_HOUSEHOLD = "select * from " + DB_TABLE_HOUSEHOLD + " limit 10";
const string DB_GETALL_INDIVIDUAL = "select * from " + DB_TABLE_INDIVIDUAL + " limit 10";
const string DB_GETALL_BUILDING_TYPE = "select * from " + DB_TABLE_BUILDING_TYPE + " limit 10";

/**
 * GET BY ID
 */
const string DB_GETBYID_HOUSEHOLD = "select * from " + DB_TABLE_HOUSEHOLD + " where " + DB_FIELD_HOUSEHOLD_ID + "=:id";
const string DB_GETBYID_INDIVIDUAL = "select * from " + DB_TABLE_INDIVIDUAL + " where " + DB_FIELD_PERSON_ID + "=:id";
const string DB_GETBYID_BUILDING_TYPE = "select * from " + DB_TABLE_BUILDING_TYPE + " where " + DB_FIELD_BUILDING_TYPE_ID + "=:id";