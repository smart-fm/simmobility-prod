//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licenced under of the terms of the MIT licence, as described in the file:
//licence.txt (www.opensource.org\licences\MIT)

/*
 * IndividualDao.h
 *
 *  Created on: 3 Sep, 2014
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/Individual.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class IndividualDao : public db::SqlAbstractDao<Individual>
		{
		public:
			IndividualDao( db::DB_Connection& connection );
			virtual ~IndividualDao();

		private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, Individual& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(Individual& data, db::Parameters& outParams, bool update);

		public:
            std::vector<Individual*> getPrimarySchoolIndividual(std::tm currentSimYear);
            std::vector<Individual*> getPreSchoolIndividual(std::tm currentSimYear);
		};
	}
}

