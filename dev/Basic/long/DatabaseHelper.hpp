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
        const std::string LIMIT = LIMIT_10;

        /**
         * Tables
         */
        const std::string DB_TABLE_GLOBAL_PARAMS = APPLY_SCHEMA(CURRENT_SCHEMA, "global_params");
        const std::string DB_TABLE_UNIT_TYPE = APPLY_SCHEMA(CURRENT_SCHEMA, "unit_type");
        const std::string DB_TABLE_HOUSEHOLD = APPLY_SCHEMA(CURRENT_SCHEMA, "household");
        const std::string DB_TABLE_BUILDING_TYPE = APPLY_SCHEMA(CURRENT_SCHEMA, "building_type");
        const std::string DB_TABLE_BUILDING = APPLY_SCHEMA(CURRENT_SCHEMA, "building");
        const std::string DB_TABLE_UNIT = APPLY_SCHEMA(CURRENT_SCHEMA, "unit");
        /**
         * Views
         */ 
        const std::string DB_VIEW_UNIT = APPLY_SCHEMA(CURRENT_SCHEMA, "view_unit");

        // housing market
        const std::string DB_TABLE_SELLER_PARAMS = APPLY_SCHEMA(CURRENT_SCHEMA, "hm_seller_params");
        const std::string DB_TABLE_BIDDER_PARAMS = APPLY_SCHEMA(CURRENT_SCHEMA, "hm_bidder_params");

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
        const std::string DB_FIELD_LAND_AREA = "land_area";
        const std::string DB_FIELD_IMPROVEMENT_VALUE = "improvement_value";
        const std::string DB_FIELD_TAX_EXEMPT = "tax_exempt";
        const std::string DB_FIELD_NON_RESIDENTIAL_SQFT = "non_residential_sqft";
        const std::string DB_FIELD_SQFT_PER_UNIT = "sqft_per_unit";
        const std::string DB_FIELD_LATITUDE = "latitude";
        const std::string DB_FIELD_LONGITUDE = "longitude";

        const std::string DB_FIELD_NAME = "name";
        const std::string DB_FIELD_TYPE = "type";

        /**
         * INSERT
         */
        const std::string DB_INSERT_GLOBAL_PARAMS = "";
        const std::string DB_INSERT_HOUSEHOLD = "INSERT INTO " + DB_TABLE_HOUSEHOLD + " ("
                + DB_FIELD_ID + ", "
                + DB_FIELD_UNIT_ID + ", "
                + DB_FIELD_SIZE + ", "
                + DB_FIELD_CHILDREN + ", "
                + DB_FIELD_INCOME + ", "
                + DB_FIELD_HOUSING_DURATION + ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7)";

        const std::string DB_INSERT_UNIT_TYPE = DB_EMPTY_QUERY;
        const std::string DB_INSERT_BUILDING_TYPE = DB_EMPTY_QUERY;
        const std::string DB_INSERT_BUILDING = DB_EMPTY_QUERY; 
        const std::string DB_INSERT_UNIT = DB_EMPTY_QUERY;

        /**
         * UPDATE
         */
        const std::string DB_UPDATE_GLOBAL_PARAMS = "";
        const std::string DB_UPDATE_HOUSEHOLD = "UPDATE " + DB_TABLE_HOUSEHOLD + " SET "
                + DB_FIELD_UNIT_ID + "= :v1, "
                + DB_FIELD_SIZE + "= :v2, "
                + DB_FIELD_CHILDREN + "= :v3, "
                + DB_FIELD_INCOME + "= :v4, "
                + DB_FIELD_HOUSING_DURATION + "= :v5 WHERE " + DB_FIELD_ID + "=:v6";

        const std::string DB_UPDATE_UNIT_TYPE = DB_EMPTY_QUERY;
        const std::string DB_UPDATE_BUILDING_TYPE = DB_EMPTY_QUERY;
        const std::string DB_UPDATE_BUILDING = DB_EMPTY_QUERY;
        const std::string DB_UPDATE_UNIT = DB_EMPTY_QUERY;

        /**
         * DELETE
         */
        const std::string DB_DELETE_GLOBAL_PARAMS = "";
        const std::string DB_DELETE_UNIT_TYPE = "DELETE FROM " + DB_TABLE_UNIT_TYPE + " WHERE " + DB_FIELD_ID + "=:id";
        const std::string DB_DELETE_HOUSEHOLD = "DELETE FROM " + DB_TABLE_HOUSEHOLD + " WHERE " + DB_FIELD_ID + "=:id";
        const std::string DB_DELETE_BUILDING_TYPE = "DELETE FROM " + DB_TABLE_BUILDING_TYPE + " WHERE " + DB_FIELD_ID + "=:id";
        const std::string DB_DELETE_BUILDING = "DELETE FROM " + DB_TABLE_BUILDING + " WHERE " + DB_FIELD_ID + "=:id";
        const std::string DB_DELETE_UNIT = "DELETE FROM " + DB_TABLE_UNIT + " WHERE " + DB_FIELD_ID + "=:id";

        /**
         * GET ALL
         */
        const std::string DB_GETALL_GLOBAL_PARAMS = "SELECT * FROM " + DB_TABLE_GLOBAL_PARAMS + " ORDER BY " + DB_FIELD_ID + LIMIT;
        const std::string DB_GETALL_UNIT_TYPE = "SELECT * FROM " + DB_TABLE_UNIT_TYPE + " ORDER BY " + DB_FIELD_ID + LIMIT;
        const std::string DB_GETALL_HOUSEHOLD = "SELECT * FROM " + DB_TABLE_HOUSEHOLD + " ORDER BY " + DB_FIELD_ID + LIMIT;
        const std::string DB_GETALL_BUILDING_TYPE = "SELECT * FROM " + DB_TABLE_BUILDING_TYPE + " ORDER BY " + DB_FIELD_ID + LIMIT;
        const std::string DB_GETALL_BUILDING = "SELECT * FROM " + DB_TABLE_BUILDING + " ORDER BY " + DB_FIELD_ID + LIMIT;
        const std::string DB_GETALL_UNIT = "SELECT * FROM " + DB_VIEW_UNIT + " ORDER BY " + DB_FIELD_ID + LIMIT;

        /**
         * GET BY ID
         */
        const std::string DB_GETBYID_GLOBAL_PARAMS = "SELECT * FROM " + DB_TABLE_GLOBAL_PARAMS + " WHERE " + DB_FIELD_ID + "=:id";
        const std::string DB_GETBYID_UNIT_TYPE = "SELECT * FROM " + DB_TABLE_UNIT_TYPE + " WHERE " + DB_FIELD_ID + "=:id";
        const std::string DB_GETBYID_HOUSEHOLD = "SELECT * FROM " + DB_TABLE_HOUSEHOLD + " WHERE " + DB_FIELD_ID + "=:id";
        const std::string DB_GETBYID_BUILDING_TYPE = "SELECT * FROM " + DB_TABLE_BUILDING_TYPE + " WHERE " + DB_FIELD_ID + "=:id";
        const std::string DB_GETBYID_BUILDING = "SELECT * FROM " + DB_TABLE_BUILDING + " WHERE " + DB_FIELD_ID + "=:id";
        const std::string DB_GETBYID_UNIT = "SELECT * FROM " + DB_VIEW_UNIT + " WHERE " + DB_FIELD_ID + "=:id";



        // HOUSING MARKET

        const std::string DB_INSERT_SELLER_PARAMS = "";
        const std::string DB_INSERT_BIDDER_PARAMS = "";
        const std::string DB_UPDATE_SELLER_PARAMS = "";
        const std::string DB_UPDATE_BIDDER_PARAMS = "";
        const std::string DB_DELETE_SELLER_PARAMS = "DELETE FROM " + DB_TABLE_SELLER_PARAMS + " WHERE " + DB_FIELD_HOUSEHOLD_ID + "=:id";
        const std::string DB_DELETE_BIDDER_PARAMS = "DELETE FROM " + DB_TABLE_BIDDER_PARAMS + " WHERE " + DB_FIELD_HOUSEHOLD_ID + "=:id";
        const std::string DB_GETALL_SELLER_PARAMS = "SELECT * FROM " + DB_TABLE_SELLER_PARAMS;
        const std::string DB_GETALL_BIDDER_PARAMS = "SELECT * FROM " + DB_TABLE_BIDDER_PARAMS;
        const std::string DB_GETBYID_SELLER_PARAMS = "SELECT * FROM " + DB_TABLE_SELLER_PARAMS + " WHERE " + DB_FIELD_HOUSEHOLD_ID + "=:id";
        const std::string DB_GETBYID_BIDDER_PARAMS = "SELECT * FROM " + DB_TABLE_BIDDER_PARAMS + " WHERE " + DB_FIELD_HOUSEHOLD_ID + "=:id";
    }
}
