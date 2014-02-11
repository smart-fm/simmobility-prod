//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DatabaseHelper.h
 * Author: gandola
 *
 * Created on April 24, 2013, 12:14 PM
 */

#pragma once

namespace sim_mob {

    namespace long_term {

#define APPLY_SCHEMA(schema, field) std::string(schema)+std::string(field)

        /**
         * Schemas
         */
        const std::string DB_EMPTY_QUERY = "";
        const std::string DB_SCHEMA_EMPTY = "";
        const std::string MAIN_SCHEMA = "main.";
        const std::string CALIBRATION_SCHEMA = "calibration.";
        const std::string LIMIT_10 = " limit 10";
        const std::string LIMIT_ALL = "";
        const std::string LIMIT = LIMIT_10;

        /**
         * Tables
         */
        const std::string DB_TABLE_HOUSEHOLD = APPLY_SCHEMA(MAIN_SCHEMA, "household");
        const std::string DB_TABLE_BUILDING = APPLY_SCHEMA(MAIN_SCHEMA, "building");
        const std::string DB_TABLE_UNIT = APPLY_SCHEMA(MAIN_SCHEMA, "unit");
        const std::string DB_TABLE_POSTCODE = APPLY_SCHEMA(MAIN_SCHEMA, "postcode");
        const std::string DB_TABLE_POSTCODE_AMENITIES = APPLY_SCHEMA(MAIN_SCHEMA, "postcode_amenities");
        
        /**
         * Views
         */
        const std::string DB_VIEW_UNIT = APPLY_SCHEMA(MAIN_SCHEMA, "view_unit");

        /**
         * Functions API
         */
        const std::string DB_FUNC_DEL_HOUSEHOLD_BY_ID = APPLY_SCHEMA(MAIN_SCHEMA, "deleteHouseholdById(:id)");
        const std::string DB_FUNC_GET_HOUSEHOLDS = APPLY_SCHEMA(MAIN_SCHEMA, "getHouseholds()");
        const std::string DB_FUNC_GET_HOUSEHOLD_BY_ID = APPLY_SCHEMA(MAIN_SCHEMA, "getHouseholdById(:id)");
        const std::string DB_FUNC_DEL_UNIT_BY_ID = APPLY_SCHEMA(MAIN_SCHEMA, "deleteUnitById(:id)");
        const std::string DB_FUNC_GET_UNITS = APPLY_SCHEMA(MAIN_SCHEMA, "getUnits()");
        const std::string DB_FUNC_GET_UNIT_BY_ID = APPLY_SCHEMA(MAIN_SCHEMA, "getUnitById(:id)");
        const std::string DB_FUNC_DEL_BUILDING_BY_ID = APPLY_SCHEMA(MAIN_SCHEMA, "deleteBuildingById(:id)");
        const std::string DB_FUNC_GET_BUILDINGS = APPLY_SCHEMA(MAIN_SCHEMA, "getBuildings()");
        const std::string DB_FUNC_GET_BUILDING_BY_ID = APPLY_SCHEMA(MAIN_SCHEMA, "getBuildingById(:id)");
        const std::string DB_FUNC_DEL_POSTCODE_BY_ID = DB_EMPTY_QUERY;
        const std::string DB_FUNC_GET_POSTCODES = APPLY_SCHEMA(MAIN_SCHEMA, "getPostcodes()");
        const std::string DB_FUNC_GET_POSTCODE_BY_ID = APPLY_SCHEMA(MAIN_SCHEMA, "getPostcodeById(:id)");

        const std::string DB_FUNC_DEL_POSTCODE_AMENITIES_BY_ID = DB_EMPTY_QUERY;
        const std::string DB_FUNC_GET_POSTCODES_AMENITIES = APPLY_SCHEMA(MAIN_SCHEMA, "getPostcodeAmenities()");
        const std::string DB_FUNC_GET_POSTCODE_AMENITIES_BY_ID = APPLY_SCHEMA(MAIN_SCHEMA, "getPostcodeAmenitiesById(:id)");
        /**
         * Fields
         */

        //NEW DATABASE
        const std::string DB_FIELD_ID = "id";
        const std::string DB_FIELD_UNIT_ID = "unit_id";
        const std::string DB_FIELD_HOUSEHOLD_ID = "household_id";
        const std::string DB_FIELD_PROJECT_ID = "project_id";
        const std::string DB_FIELD_PARCEL_ID = "parcel_id";
        const std::string DB_FIELD_BUILDING_ID = "building_id";
        const std::string DB_FIELD_ESTABLISMENT_ID = "establishment_id";
        const std::string DB_FIELD_TYPE_ID = "type_id";
        const std::string DB_FIELD_POSTCODE_ID = "postcode_id";
        const std::string DB_FIELD_TAZ_ID = "taz_id";
        const std::string DB_FIELD_LIFESTYLE_ID = "lifestyle_id";
        const std::string DB_FIELD_VEHICLE_CATEGORY_ID = "vehicle_category_id";
        const std::string DB_FIELD_ETHNICITY_ID = "ethnicity_id";
        const std::string DB_FIELD_TENURE_ID = "tenure_id";
        const std::string DB_FIELD_INCOME = "income";
        const std::string DB_FIELD_FLOOR_AREA = "floor_area";
        const std::string DB_FIELD_YEAR = "year";
        const std::string DB_FIELD_STOREY = "storey";
        const std::string DB_FIELD_RENT = "rent";
        const std::string DB_FIELD_SIZE = "size";
        const std::string DB_FIELD_CHILDREN = "children";
        const std::string DB_FIELD_WORKERS = "workers";
        const std::string DB_FIELD_AGE_OF_HEAD = "age_of_head";
        const std::string DB_FIELD_HOUSING_DURATION = "housing_duration";
        const std::string DB_FIELD_BUILT_YEAR = "built_year";
        const std::string DB_FIELD_STOREYS = "storeys";
        const std::string DB_FIELD_PARKING_SPACES = "parking_spaces";
        const std::string DB_FIELD_RESIDENTIAL_UNITS = "residential_units";
        const std::string DB_FIELD_LANDED_AREA = "landed_area";
        const std::string DB_FIELD_IMPROVEMENT_VALUE = "improvement_value";
        const std::string DB_FIELD_TAX_EXEMPT = "tax_exempt";
        const std::string DB_FIELD_NON_RESIDENTIAL_SQFT = "non_residential_sqft";
        const std::string DB_FIELD_SQFT_PER_UNIT = "sqft_per_unit";
        const std::string DB_FIELD_LATITUDE = "latitude";
        const std::string DB_FIELD_LONGITUDE = "longitude";
        const std::string DB_FIELD_NAME = "name";
        const std::string DB_FIELD_TYPE = "type";
        const std::string DB_FIELD_CODE = "code";
        const std::string DB_FIELD_POSTCODE = "postcode";
        const std::string DB_FIELD_BUILDING_NAME = "building_name";
        const std::string DB_FIELD_ROAD_NAME = "road_name";
        const std::string DB_FIELD_UNIT_BLOCK = "unit_block";
        const std::string DB_FIELD_MTZ_NUMBER = "mtz_number";
        const std::string DB_FIELD_MRT_STATION = "mrt_station";
        const std::string DB_FIELD_DISTANCE_TO_MRT = "distance_mrt";
        const std::string DB_FIELD_DISTANCE_TO_BUS = "distance_bus";
        const std::string DB_FIELD_DISTANCE_TO_PMS30 = "distance_pms30";
        const std::string DB_FIELD_DISTANCE_TO_CBD = "distance_cbd";
        const std::string DB_FIELD_DISTANCE_TO_MALL = "distance_mall";
        const std::string DB_FIELD_DISTANCE_TO_JOB = "distance_job";
        const std::string DB_FIELD_DISTANCE_TO_EXPRESS = "distance_express";
        const std::string DB_FIELD_MRT_200M = "mrt_200m";
        const std::string DB_FIELD_MRT_400M = "mrt_400m";
        const std::string DB_FIELD_EXPRESS_200M = "express_200m";
        const std::string DB_FIELD_BUS_200M = "bus_200m";
        const std::string DB_FIELD_BUS_400M = "bus_400m";
        const std::string DB_FIELD_PMS_1KM = "pms_1km";
        const std::string DB_FIELD_APARTMENT = "apartment";
        const std::string DB_FIELD_CONDO = "condo";
        const std::string DB_FIELD_TERRACE = "terrace";
        const std::string DB_FIELD_SEMI = "semi";
        const std::string DB_FIELD_DETACHED = "detached";
        const std::string DB_FIELD_EC = "ec";
        const std::string DB_FIELD_PRIVATE = "private";
        const std::string DB_FIELD_HDB = "hdb";

        /**
         * INSERT
         */
        const std::string DB_INSERT_HOUSEHOLD = "INSERT INTO " 
                + DB_TABLE_HOUSEHOLD + " ("
                + DB_FIELD_ID + ", "
                + DB_FIELD_UNIT_ID + ", "
                + DB_FIELD_SIZE + ", "
                + DB_FIELD_CHILDREN + ", "
                + DB_FIELD_INCOME + ", "
                + DB_FIELD_HOUSING_DURATION 
                + ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7)";

        const std::string DB_INSERT_BUILDING = DB_EMPTY_QUERY;
        const std::string DB_INSERT_UNIT = DB_EMPTY_QUERY;
        const std::string DB_INSERT_POSTCODE = DB_EMPTY_QUERY;
        const std::string DB_INSERT_POSTCODE_AMENITIES = DB_EMPTY_QUERY;

        /**
         * UPDATE
         */
        const std::string DB_UPDATE_HOUSEHOLD = "UPDATE " 
                + DB_TABLE_HOUSEHOLD + " SET "
                + DB_FIELD_UNIT_ID + "= :v1, "
                + DB_FIELD_SIZE + "= :v2, "
                + DB_FIELD_CHILDREN + "= :v3, "
                + DB_FIELD_INCOME + "= :v4, "
                + DB_FIELD_HOUSING_DURATION 
                + "= :v5 WHERE " 
                + DB_FIELD_ID + "=:v6";

        const std::string DB_UPDATE_BUILDING = DB_EMPTY_QUERY;
        const std::string DB_UPDATE_UNIT = DB_EMPTY_QUERY;
        const std::string DB_UPDATE_POSTCODE = DB_EMPTY_QUERY;
        const std::string DB_UPDATE_POSTCODE_AMENITIES = DB_EMPTY_QUERY;

        /**
         * DELETE
         */
        const std::string DB_DELETE_HOUSEHOLD = "SELECT * FROM " 
                + DB_FUNC_DEL_HOUSEHOLD_BY_ID;
        const std::string DB_DELETE_BUILDING = "SELECT * FROM "
                + DB_FUNC_DEL_BUILDING_BY_ID;
        const std::string DB_DELETE_UNIT = "SELECT * FROM " 
                + DB_FUNC_DEL_UNIT_BY_ID;
        const std::string DB_DELETE_POSTCODE = DB_EMPTY_QUERY;
        const std::string DB_DELETE_POSTCODE_AMENITIES = DB_EMPTY_QUERY;

        /**
         * GET ALL
         */
        const std::string DB_GETALL_HOUSEHOLD = "SELECT * FROM "
                + DB_FUNC_GET_HOUSEHOLDS
                + LIMIT;
        const std::string DB_GETALL_BUILDING = "SELECT * FROM "
                + DB_FUNC_GET_BUILDINGS
                + LIMIT;
        const std::string DB_GETALL_UNIT = "SELECT * FROM "
                + DB_FUNC_GET_UNITS
                + LIMIT;
        const std::string DB_GETALL_POSTCODE = "SELECT * FROM "
                + DB_FUNC_GET_POSTCODES
                + LIMIT;
        const std::string DB_GETALL_POSTCODE_AMENITIES = "SELECT * FROM "
                + DB_FUNC_GET_POSTCODES_AMENITIES
                + LIMIT;
        
        /**
         * GET BY ID
         */
        const std::string DB_GETBYID_HOUSEHOLD = "SELECT * FROM "
                + DB_FUNC_GET_HOUSEHOLD_BY_ID;
        const std::string DB_GETBYID_BUILDING = "SELECT * FROM " 
                + DB_FUNC_GET_BUILDING_BY_ID;
        const std::string DB_GETBYID_UNIT = "SELECT * FROM " +
                DB_FUNC_GET_UNIT_BY_ID;
        const std::string DB_GETBYID_POSTCODE = "SELECT * FROM " +
                DB_FUNC_GET_POSTCODE_BY_ID;
        const std::string DB_GETBYID_POSTCODE_AMENITIES = "SELECT * FROM " +
                DB_FUNC_GET_POSTCODE_AMENITIES_BY_ID;
    }
}
