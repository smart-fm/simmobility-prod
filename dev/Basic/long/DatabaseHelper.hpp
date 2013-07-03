/* 
 * File:   DatabaseHelper.h
 * Author: gandola
 *
 * Created on April 24, 2013, 12:14 PM
 */

#pragma once

#define APPLY_SCHEMA(schema, field) std::string(schema)+std::string(field)

/**
 * Schemas
 */
#define DB_SCHEMA_EMPTY   ""
#define DB_SCHEMA_BASELINE_2001   "baseline_2001."
const std::string CURRENT_SCHEMA = DB_SCHEMA_EMPTY;

/**
 * Tables
 */
const std::string DB_TABLE_UNIT_TYPE = APPLY_SCHEMA(CURRENT_SCHEMA, "unit_type");
const std::string DB_TABLE_HOUSEHOLD = APPLY_SCHEMA(CURRENT_SCHEMA, "household");
const std::string DB_TABLE_BUILDING_TYPE = APPLY_SCHEMA(CURRENT_SCHEMA, "building_type");
const std::string DB_TABLE_BUILDING = APPLY_SCHEMA(CURRENT_SCHEMA, "building");
const std::string DB_TABLE_UNIT = APPLY_SCHEMA(CURRENT_SCHEMA, "unit");

/**
 * Fields
 */

//NEW DATABASE
const std::string DB_FIELD_ID = "id";
const std::string DB_FIELD_UNIT_ID = "unit_id";
const std::string DB_FIELD_PROJECT_ID = "project_id";
const std::string DB_FIELD_PARCEL_ID = "parcel_id";
const std::string DB_FIELD_BUILDING_ID = "building_id";
const std::string DB_FIELD_ESTABLISMENT_ID = "establishment_id";
const std::string DB_FIELD_TYPE_ID = "type_id";
const std::string DB_FIELD_INCOME = "income";
const std::string DB_FIELD_FLOOR_AREA = "floor_area";
const std::string DB_FIELD_YEAR = "year";
const std::string DB_FIELD_STOREY = "storey";
const std::string DB_FIELD_RENT = "rent";
const std::string DB_FIELD_SIZE = "size";
const std::string DB_FIELD_CHILDREN = "children";
const std::string DB_FIELD_CAR_OWNERSHIP = "car_ownership";
const std::string DB_FIELD_HOUSING_DURATION = "housing_duration";
const std::string DB_FIELD_BUILT_YEAR = "built_year";
const std::string DB_FIELD_STOREYS = "storeys";
const std::string DB_FIELD_PARKING_SPACES = "parking_spaces";
const std::string DB_FIELD_NAME = "name";


/**
 * INSERT
 */
const std::string DB_INSERT_HOUSEHOLD = "INSERT INTO " + DB_TABLE_HOUSEHOLD + " ("
        + DB_FIELD_ID + ", "
        + DB_FIELD_UNIT_ID + ", "
        + DB_FIELD_SIZE + ", "
        + DB_FIELD_CHILDREN + ", "
        + DB_FIELD_INCOME + ", "
        + DB_FIELD_CAR_OWNERSHIP + ", "
        + DB_FIELD_HOUSING_DURATION + ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7)";

const std::string DB_INSERT_UNIT_TYPE = ""; // not defined yet...
const std::string DB_INSERT_BUILDING_TYPE = ""; // not defined yet...
const std::string DB_INSERT_BUILDING = ""; // not defined yet...
const std::string DB_INSERT_UNIT = ""; // not defined yet...

/**
 * UPDATE
 */
const std::string DB_UPDATE_HOUSEHOLD = "UPDATE " + DB_TABLE_HOUSEHOLD + " SET "
        + DB_FIELD_UNIT_ID + "= :v1, "
        + DB_FIELD_SIZE + "= :v2, "
        + DB_FIELD_CHILDREN + "= :v3, "
        + DB_FIELD_INCOME + "= :v4, "
        + DB_FIELD_CAR_OWNERSHIP + "= :v5, "
        + DB_FIELD_HOUSING_DURATION + "= :v6 WHERE " + DB_FIELD_ID + "=:v7";

const std::string DB_UPDATE_UNIT_TYPE = ""; // not defined yet...
const std::string DB_UPDATE_BUILDING_TYPE = ""; // not defined yet...
const std::string DB_UPDATE_BUILDING = ""; // not defined yet...
const std::string DB_UPDATE_UNIT = ""; // not defined yet...

/**
 * DELETE
 */
const std::string DB_DELETE_UNIT_TYPE = "DELETE FROM " + DB_TABLE_UNIT_TYPE + " WHERE " + DB_FIELD_ID + "=:id";
const std::string DB_DELETE_HOUSEHOLD = "DELETE FROM " + DB_TABLE_HOUSEHOLD + " WHERE " + DB_FIELD_ID + "=:id";
const std::string DB_DELETE_BUILDING_TYPE = "DELETE FROM " + DB_TABLE_BUILDING_TYPE + " WHERE " + DB_FIELD_ID + "=:id";
const std::string DB_DELETE_BUILDING = "DELETE FROM " + DB_TABLE_BUILDING + " WHERE " + DB_FIELD_ID + "=:id";
const std::string DB_DELETE_UNIT = "DELETE FROM " + DB_TABLE_UNIT + " WHERE " + DB_FIELD_ID + "=:id";

/**
 * GET ALL
 */
const std::string DB_GETALL_UNIT_TYPE = "SELECT * FROM " + DB_TABLE_UNIT_TYPE + " LIMIT 10";
const std::string DB_GETALL_HOUSEHOLD = "SELECT * FROM " + DB_TABLE_HOUSEHOLD + " LIMIT 10";
const std::string DB_GETALL_BUILDING_TYPE = "SELECT * FROM " + DB_TABLE_BUILDING_TYPE + " LIMIT 10";
const std::string DB_GETALL_BUILDING = "SELECT * FROM " + DB_TABLE_BUILDING + " LIMIT 10";
const std::string DB_GETALL_UNIT = "SELECT * FROM " + DB_TABLE_UNIT + " LIMIT 10";
//const std::string DB_GETALL_UNIT = "SELECT A.*, B.* FROM " + DB_TABLE_UNIT + " AS A, "+DB_TABLE_BUILDING+" AS B WHERE A." + DB_FIELD_BUILDING_ID + " = B."+ DB_FIELD_ID + " LIMIT 10";

/**
 * GET BY ID
 */
const std::string DB_GETBYID_UNIT_TYPE = "SELECT * FROM " + DB_TABLE_UNIT_TYPE + " WHERE " + DB_FIELD_ID + "=:id";
const std::string DB_GETBYID_HOUSEHOLD = "SELECT * FROM " + DB_TABLE_HOUSEHOLD + " WHERE " + DB_FIELD_ID + "=:id";
const std::string DB_GETBYID_BUILDING_TYPE = "SELECT * FROM " + DB_TABLE_BUILDING_TYPE + " WHERE " + DB_FIELD_ID + "=:id";
const std::string DB_GETBYID_BUILDING = "SELECT * FROM " + DB_TABLE_BUILDING + " WHERE " + DB_FIELD_ID + "=:id";
const std::string DB_GETBYID_UNIT = "SELECT * FROM " + DB_TABLE_UNIT + " WHERE " + DB_FIELD_ID + "=:id";
//const std::string DB_GETBYID_UNIT = "SELECT A.*, B.* FROM " + DB_TABLE_UNIT + " AS A, "+DB_TABLE_BUILDING+" AS B WHERE A." + DB_FIELD_BUILDING_ID + " = B."+ DB_FIELD_ID + " AND A." + DB_FIELD_ID + " =:id";