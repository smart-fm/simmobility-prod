/*
 * TravelTimeDao.hpp
 *
 *  Created on: 11 Mar 2016
 *      Author: gishara
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/TravelTime.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to taravel_time table on datasource.
         */
        class TravelTimeDao : public db::SqlAbstractDao<TravelTime> {
        public:
        	TravelTimeDao(db::DB_Connection& connection);
            virtual ~TravelTimeDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, TravelTime& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(TravelTime& data, db::Parameters& outParams, bool update);

        public:
            const TravelTime* getTravelTimeByOriginDest(BigSerial origin, BigSerial destination);
        };
    }
}




