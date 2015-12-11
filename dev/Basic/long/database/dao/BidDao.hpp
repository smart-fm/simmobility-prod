/*
 * BidDao.hpp
 *
 *  Created on: Nov 30, 2015
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/Bid.hpp"


namespace sim_mob {
    namespace long_term {

        /**
         * Data Access Object to Bids table on datasource.
         */
        class BidDao : public db::SqlAbstractDao<Bid> {
        public:
        	BidDao(db::DB_Connection& connection);
            virtual ~BidDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, Bid& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(Bid& data, db::Parameters& outParams, bool update);

        public:
            void insertBid(Bid& bid,std::string schema);
        };
    }
}

