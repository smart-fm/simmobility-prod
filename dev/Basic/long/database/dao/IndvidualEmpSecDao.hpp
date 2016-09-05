/*
 * IndvidualEmpSecDao.hpp
 *
 *  Created on: 11 Jul 2016
 *      Author: gishara
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/IndvidualEmpSec.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to get employment sec ids per individual from datasource.
         */
        class IndvidualEmpSecDao : public db::SqlAbstractDao<IndvidualEmpSec> {
        public:
        	IndvidualEmpSecDao(db::DB_Connection& connection);
            virtual ~IndvidualEmpSecDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, IndvidualEmpSec& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(IndvidualEmpSec& data, db::Parameters& outParams, bool update);
        };
    }
}
