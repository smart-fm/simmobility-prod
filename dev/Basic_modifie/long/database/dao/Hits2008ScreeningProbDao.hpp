//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   Hits2008ScreeningProbDao.hpp
 * Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *
 * Created on 31 Jul, 2015, 5:17 PM
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/Hits2008ScreeningProb.hpp"

namespace sim_mob
{
    namespace long_term
    {
        /**
         * Data Access Object to Hits2008ScreeningProb table on datasource.
         */
        class Hits2008ScreeningProbDao : public db::SqlAbstractDao<Hits2008ScreeningProb>
        {
        public:
        	Hits2008ScreeningProbDao(db::DB_Connection& connection);
            virtual ~Hits2008ScreeningProbDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, Hits2008ScreeningProb& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(Hits2008ScreeningProb& data, db::Parameters& outParams, bool update);
        };
    }
}



