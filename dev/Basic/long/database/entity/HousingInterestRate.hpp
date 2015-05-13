/*
 * HousingInterestRate.hpp
 *
 *  Created on: 4 May, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include "Common.hpp"
#include "Types.hpp"

#pragma once

namespace sim_mob
{
	namespace long_term
	{
		class HousingInterestRate
		{
		public:
			HousingInterestRate(BigSerial id=0, std::tm from_date = std::tm(), std::tm to_date = std::tm(), float interestRate=0);
			virtual ~HousingInterestRate();

			void setInterestRate( float val);
			void setFromDate( std::tm val);
			void setToDate( std::tm val);
			void setId( BigSerial id);

			float     getInterestRate() const;
			std::tm   getFromDate() const;
			std::tm	  getToDate() const;
			BigSerial getId() const;

			HousingInterestRate& operator=(const HousingInterestRate& source);

            /**
             * Operator to print the HouseholdInterestRate data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const HousingInterestRate& data);


		private:
			friend class HousingInterestRateDao;

			BigSerial id;
			std::tm from_date;
			std::tm to_date;
			float interestRate;

		};
	}
}
