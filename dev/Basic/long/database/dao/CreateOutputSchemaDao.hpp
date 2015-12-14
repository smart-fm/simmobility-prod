/*
 * CreateOutputSchemaDao.hpp
 *
 *  Created on: Nov 23, 2015
 *      Author: gishara
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/CreateOutputSchema.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to CreateOutputSchema table on datasource.
         */
        class CreateOutputSchemaDao : public db::SqlAbstractDao<CreateOutputSchema> {
        public:
        	CreateOutputSchemaDao(db::DB_Connection& connection);
            virtual ~CreateOutputSchemaDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, CreateOutputSchema& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(CreateOutputSchema& data, db::Parameters& outParams, bool update);
        };
    }
}




