/*
 * HousingInterestRateDao.hpp
 *
 *  Created on: 4 May, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/HousingInterestRate.hpp"


namespace sim_mob
{
	namespace long_term
	{
		class HousingInterestRateDao : public db::SqlAbstractDao<HousingInterestRate>
		{
		        public:
					HousingInterestRateDao(db::DB_Connection& connection);
		            virtual ~HousingInterestRateDao();

		        private:
		            /**
		             * Fills the given outObj with all values contained on Row.
		             * @param result row with data to fill the out object.
		             * @param outObj to fill.
		             */
		            void fromRow(db::Row& result, HousingInterestRate& outObj);

		            /**
		             * Fills the outParam with all values to insert or update on datasource.
		             * @param data to get values.
		             * @param outParams to put the data parameters.
		             * @param update tells if operation is an Update or Insert.
		             */
		            void toRow(HousingInterestRate& data, db::Parameters& outParams, bool update);
		};
	}
}
