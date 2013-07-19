/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   SellerParamsDao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on 25 June, 2013, 3:59 PM
 */
#pragma once

#include "database/dao/AbstractDao.hpp"
#include "database/entity/housing-market/SellerParams.hpp"


namespace sim_mob {
    namespace long_term {

        DAO_DECLARE_CALLBACKS(SellerParams);
        /**
         * Data Access Object to SellerParams table on datasource.
         */
        class SellerParamsDao : public db::AbstractDao<SellerParams> {
        public:
            SellerParamsDao(db::DBConnection* connection);
            virtual ~SellerParamsDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row. 
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void FromRow(db::Row& result, SellerParams& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void ToRow(SellerParams& data, db::Parameters& outParams, bool update);
        };
    }
}

