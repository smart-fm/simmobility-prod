//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PotentialProjectDao.hpp
 *
 *  Created on: Dec 17, 2015
 *      Author: gishara
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/PotentialProject.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {

        /**
         * Data Access Object to potential_project table on datasource.
         */
        class PotentialProjectDao : public db::SqlAbstractDao<PotentialProject> {
        public:
        	PotentialProjectDao(db::DB_Connection& connection);
            virtual ~PotentialProjectDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, PotentialProject& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(PotentialProject& data, db::Parameters& outParams, bool update);

        	public:

            void insertPotentialProject(PotentialProject& potentialProject,std::string schema);
        };
    }
}
