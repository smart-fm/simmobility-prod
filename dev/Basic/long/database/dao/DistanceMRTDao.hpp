/*
 * DistanceMRTDao.hpp
 *
 *  Created on: Jun 2, 2015
 *      Author: gishara
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/DistanceMRT.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to dist_mrt table on datasource.
         */
        class DistanceMRTDao : public db::SqlAbstractDao<DistanceMRT> {
        public:
        	DistanceMRTDao(db::DB_Connection& connection);
            virtual ~DistanceMRTDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, DistanceMRT& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(DistanceMRT& data, db::Parameters& outParams, bool update);
        };
    }
}


