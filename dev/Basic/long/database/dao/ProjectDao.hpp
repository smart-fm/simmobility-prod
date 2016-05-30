/*
 * ProjectDao.hpp
 *
 *  Created on: Aug 21, 2014
 *      Author: gishara
 */
#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/Project.hpp"

namespace sim_mob {
    namespace long_term {
        /**
         * Data Access Object to Project table on data source.
         */
        class ProjectDao : public db::SqlAbstractDao<Project> {
        public:
            ProjectDao(db::DB_Connection& connection);
            virtual ~ProjectDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, Project& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(Project& data, db::Parameters& outParams, bool update);

        public:
            void insertProject(Project& project,std::string schema);
            std::vector<Project*> loadOngoingProjects(std::string schema);
        };
    }
}
