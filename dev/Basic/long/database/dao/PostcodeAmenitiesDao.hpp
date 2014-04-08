//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   PostcodeAmenitiesDao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on 11 Feb, 2014, 3:59 PM
 */
#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/PostcodeAmenities.hpp"


namespace sim_mob {
    namespace long_term {
        
        /**
         * Data Access Object to PostcodeAmenities table on datasource.
         */
        class PostcodeAmenitiesDao : public db::SqlAbstractDao<PostcodeAmenities> {
        public:
            PostcodeAmenitiesDao(db::DB_Connection& connection);
            virtual ~PostcodeAmenitiesDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row. 
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, PostcodeAmenities& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(PostcodeAmenities& data, db::Parameters& outParams, bool update);
        };
    }
}

