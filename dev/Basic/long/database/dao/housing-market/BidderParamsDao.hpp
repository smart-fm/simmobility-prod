/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   BidderParamsDao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on 25 June, 2013, 3:59 PM
 */
#pragma once

#include "database/dao/AbstractDao.hpp"
#include "database/entity/housing-market/BidderParams.hpp"


namespace sim_mob {
    namespace long_term {

        DAO_DECLARE_CALLBACKS(BidderParams);
        /**
         * Data Access Object to BidderParams table on datasource.
         */
        class BidderParamsDao : public db::AbstractDao<BidderParams> {
        public:
            BidderParamsDao(db::DBConnection* connection);
            virtual ~BidderParamsDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row. 
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void FromRow(db::Row& result, BidderParams& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void ToRow(BidderParams& data, db::Parameters& outParams, bool update);
        };
    }
}

