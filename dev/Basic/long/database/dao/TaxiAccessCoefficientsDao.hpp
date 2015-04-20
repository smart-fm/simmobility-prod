/*
 * TaxiAccessCoefficientsDao.hpp
 *
 *  Created on: Apr 13, 2015
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/TaxiAccessCoefficients.hpp"


namespace sim_mob
{
    namespace long_term
    {
        /**
         * Data Access Object to vehicle_ownership_coefficients table on datasource.
         */
        class TaxiAccessCoefficientsDao : public db::SqlAbstractDao<TaxiAccessCoefficients>
        {
        public:
        	TaxiAccessCoefficientsDao(db::DB_Connection& connection);
            virtual ~TaxiAccessCoefficientsDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, TaxiAccessCoefficients& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(TaxiAccessCoefficients& data, db::Parameters& outParams, bool update);

        };
    }
}
