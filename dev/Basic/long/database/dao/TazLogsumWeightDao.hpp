//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   TemplateDao.cpp
 * Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *
 * Created on: 25 Jun, 2015
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/TazLogsumWeight.hpp"

using namespace boost;
namespace sim_mob
{
    namespace long_term
    {
        /**
         * Data Access Object to TazLogsumWeight table on datasource.
         */
        class TazLogsumWeightDao : public db::SqlAbstractDao<TazLogsumWeight>
        {
        public:
            TazLogsumWeightDao(db::DB_Connection& connection);
            virtual ~TazLogsumWeightDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, TazLogsumWeight& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(TazLogsumWeight& data, db::Parameters& outParams, bool update);
        };
    }
}
