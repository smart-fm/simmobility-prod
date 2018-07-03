/*
 * LogSumVehicleOwnershipDao.hpp
 *
 *  Created on: May 21, 2015
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/LogSumVehicleOwnership.hpp"


namespace sim_mob
{
    namespace long_term
    {
        /**
         * Data Access Object to get avg car ownership logsums per hh from lt_logsum_carownership table on datasource.
         */
        class LogSumVehicleOwnershipDao : public db::SqlAbstractDao<LogSumVehicleOwnership>
        {
        public:
        	LogSumVehicleOwnershipDao(db::DB_Connection& connection);
            virtual ~LogSumVehicleOwnershipDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, LogSumVehicleOwnership& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(LogSumVehicleOwnership& data, db::Parameters& outParams, bool update);

        };
    }
}

