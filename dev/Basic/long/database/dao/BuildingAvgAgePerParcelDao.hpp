/*
 * BuildingAvgAgePerParcelDao.hpp
 *
 *  Created on: 23 Feb 2016
 *      Author: gishara
 */

#pragma once

#include <database/entity/BuildingAvgAgePerParcel.hpp>
#include "database/dao/SqlAbstractDao.hpp"


namespace sim_mob {
    namespace long_term {

        /**
         * Data Access Object to BuildingAvgAge table on datasource.
         */
        class BuildingAvgAgePerParcelDao : public db::SqlAbstractDao<BuildingAvgAgePerParcel> {
        public:
        	BuildingAvgAgePerParcelDao(db::DB_Connection& connection);
            virtual ~BuildingAvgAgePerParcelDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, BuildingAvgAgePerParcel& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(BuildingAvgAgePerParcel& data, db::Parameters& outParams, bool update);

        };
    }
}


