/*
 * StatusOfWorldDao.hpp
 *
 *  Created on: Nov 13, 2015
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/SimulationStoppedPoint.hpp"


namespace sim_mob {
    namespace long_term {
        /**
         * Data Access Object to SlaParcel table on data source.
         */
        class SimulationStoppedPointDao : public db::SqlAbstractDao<SimulationStoppedPoint> {
        public:
        	SimulationStoppedPointDao(db::DB_Connection& connection);
            virtual ~SimulationStoppedPointDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, SimulationStoppedPoint& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(SimulationStoppedPoint& data, db::Parameters& outParams, bool update);

        public:
            void insertSimulationStoppedPoints(SimulationStoppedPoint& encodedParams,std::string schema);
        };
    }
}
