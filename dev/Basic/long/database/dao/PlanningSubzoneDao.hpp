//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


/*
 * PlanningSubzoneDao.hpp
 *
 *  Created on: 31 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/PlanningSubzone.hpp"


namespace sim_mob
{
    namespace long_term
    {

        /**
         * Data Access Object to Unit table on datasource.
         */
        class PlanningSubzoneDao : public db::SqlAbstractDao<PlanningSubzone>
        {
        public:
        	PlanningSubzoneDao(db::DB_Connection& connection);
            virtual ~PlanningSubzoneDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, PlanningSubzone& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(PlanningSubzone& data, db::Parameters& outParams, bool update);
        };
    }
}

