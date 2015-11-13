//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * SimulationVersionDao.hpp
 *
 *  Created on: Nov 6, 2015
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/SimulationVersion.hpp"


namespace sim_mob
{
    namespace long_term
    {
        /**
         * Data Access Object to SimulationVersion table on datasource.
         */
        class SimulationVersionDao : public db::SqlAbstractDao<SimulationVersion>
        {
        public:
        	SimulationVersionDao(db::DB_Connection& connection);
            virtual ~SimulationVersionDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, SimulationVersion& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(SimulationVersion& data, db::Parameters& outParams, bool update);

        };
    }
}
