/*
 * IndvidualVehicleOwnershipLogsumDao.hpp
 *
 *  Created on: Jan 20, 2016
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/IndvidualVehicleOwnershipLogsum.hpp"


namespace sim_mob
{
    namespace long_term
    {
        /**
         * Data Access Object to get individual logsums from datasource.
         */
        class IndvidualVehicleOwnershipLogsumDao : public db::SqlAbstractDao<IndvidualVehicleOwnershipLogsum>
        {
        public:
        	IndvidualVehicleOwnershipLogsumDao(db::DB_Connection& connection);
            virtual ~IndvidualVehicleOwnershipLogsumDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, IndvidualVehicleOwnershipLogsum& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(IndvidualVehicleOwnershipLogsum& data, db::Parameters& outParams, bool update);

        public:
            std::vector<IndvidualVehicleOwnershipLogsum*> getIndvidualVehicleOwnershipLogsumsByHHId(const long long householdId);

        };
    }
}
