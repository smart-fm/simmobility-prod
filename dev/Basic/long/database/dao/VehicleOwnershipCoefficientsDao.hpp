/*
 * vehicleOwnershipCoefficientsDao.hpp
 *
 *  Created on: Feb 24, 2015
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/VehicleOwnershipCoefficients.hpp"


namespace sim_mob
{
    namespace long_term
    {
        /**
         * Data Access Object to vehicle_ownership_coefficients table on datasource.
         */
        class VehicleOwnershipCoefficientsDao : public db::SqlAbstractDao<VehicleOwnershipCoefficients>
        {
        public:
        	VehicleOwnershipCoefficientsDao(db::DB_Connection& connection);
            virtual ~VehicleOwnershipCoefficientsDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, VehicleOwnershipCoefficients& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(VehicleOwnershipCoefficients& data, db::Parameters& outParams, bool update);

        };
    }
}
