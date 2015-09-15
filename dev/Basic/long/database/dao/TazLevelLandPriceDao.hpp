//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
* TazLevelLandPriceDao.hpp
 *
 *  Created on: Sep 3, 2015
 *      Author: gishara
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/TazLevelLandPrice.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to TazLevelLandPrice table on datasource.
         */
        class TazLevelLandPriceDao : public db::SqlAbstractDao<TazLevelLandPrice> {
        public:
        	TazLevelLandPriceDao(db::DB_Connection& connection);
            virtual ~TazLevelLandPriceDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, TazLevelLandPrice& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(TazLevelLandPrice& data, db::Parameters& outParams, bool update);
        };
    }
}
