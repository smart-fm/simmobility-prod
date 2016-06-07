/*
 * HHCoordinatesDao.hpp
 *
 *  Created on: 11 Mar 2016
 *      Author: gishara
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/HHCoordinates.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to household coordinates view on datasource.
         */
        class HHCoordinatesDao : public db::SqlAbstractDao<HHCoordinates> {
        public:
        	HHCoordinatesDao(db::DB_Connection& connection);
            virtual ~HHCoordinatesDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, HHCoordinates& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(HHCoordinates& data, db::Parameters& outParams, bool update);
        };
    }
}
