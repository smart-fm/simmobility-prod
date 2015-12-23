/*
 * VehicleOwnershipChangesDao.hpp
 *
 *  Created on: Dec 22, 2015
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/VehicleOwnershipChanges.hpp"


namespace sim_mob
{
    namespace long_term
    {
        /**
         * Data Access Object to VehicleOwnershipChanges table on datasource.
         */
        class VehicleOwnershipChangesDao : public db::SqlAbstractDao<VehicleOwnershipChanges>
        {
        public:
        	VehicleOwnershipChangesDao(db::DB_Connection& connection);
            virtual ~VehicleOwnershipChangesDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, VehicleOwnershipChanges& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(VehicleOwnershipChanges& data, db::Parameters& outParams, bool update);

        public:
            /*
            * Get the parcels of given parcel id
            */

            void insertVehicleOwnershipChanges(VehicleOwnershipChanges& devPlan,std::string schema);
        };
    }
}


