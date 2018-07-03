/*
 * ParcelAmenitiesDao.hpp
 *
 *  Created on: Dec 12, 2014
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/ParcelAmenities.hpp"


namespace sim_mob {
    namespace long_term {

        /**
         * Data Access Object to ParcelAmenities table on datasource.
         */
        class ParcelAmenitiesDao : public db::SqlAbstractDao<ParcelAmenities> {
        public:
        	ParcelAmenitiesDao(db::DB_Connection& connection);
            virtual ~ParcelAmenitiesDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, ParcelAmenities& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(ParcelAmenities& data, db::Parameters& outParams, bool update);
        };
    }
}
