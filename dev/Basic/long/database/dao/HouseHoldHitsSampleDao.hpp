/*
 * HouseHoldHitsSampleDao.hpp
 *
 *  Created on: Jun 24, 2015
 *      Author: gishara
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/HouseHoldHitsSample.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to household_hits_sample table on datasource.
         */
        class HouseHoldHitsSampleDao : public db::SqlAbstractDao<HouseHoldHitsSample> {
        public:
        	HouseHoldHitsSampleDao(db::DB_Connection& connection);
            virtual ~HouseHoldHitsSampleDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, HouseHoldHitsSample& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(HouseHoldHitsSample& data, db::Parameters& outParams, bool update);
        };
    }
}
