/*
 * StatusOfWorldDao.hpp
 *
 *  Created on: Nov 13, 2015
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/EncodedParamsBySimulation.hpp"


namespace sim_mob {
    namespace long_term {
        /**
         * Data Access Object to SlaParcel table on data source.
         */
        class EncodedParamsBySimulationDao : public db::SqlAbstractDao<EncodedParamsBySimulation> {
        public:
        	EncodedParamsBySimulationDao(db::DB_Connection& connection);
            virtual ~EncodedParamsBySimulationDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, EncodedParamsBySimulation& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(EncodedParamsBySimulation& data, db::Parameters& outParams, bool update);

        public:
            void insertEncodedParams(EncodedParamsBySimulation& encodedParams,std::string schema);
        };
    }
}
