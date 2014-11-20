/*
 * ParcelMatchDao.hpp
 *
 *  Created on: Aug 25, 2014
 *      Author: gishara
 */
#pragma once

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/ParcelMatch.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Data Access Object to parcel_match table on data source.
         */
        class ParcelMatchDao : public db::SqlAbstractDao<ParcelMatch> {
        public:
            ParcelMatchDao(db::DB_Connection& connection);
            virtual ~ParcelMatchDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, ParcelMatch& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(ParcelMatch& data, db::Parameters& outParams, bool update);

        };
    }
}
