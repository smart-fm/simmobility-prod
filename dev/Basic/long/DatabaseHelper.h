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

/**
 * DELETE
 */
const string DB_DELETE_HOUSEHOLD = "DELETE FROM " + DB_TABLE_HOUSEHOLD + " WHERE " + DB_FIELD_HOUSEHOLD_ID + "=:id";

/**
 * GET ALL
 */
const string DB_GETALL_HOUSEHOLD = "select * from " + DB_TABLE_HOUSEHOLD + " limit 10";

/**
 * GET BY ID
 */
const string DB_GETBYID_HOUSEHOLD = "select * from " + DB_TABLE_HOUSEHOLD + " where " + DB_FIELD_HOUSEHOLD_ID + "=:id";