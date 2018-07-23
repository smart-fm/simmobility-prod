/*
 * HousingInterestRate.hpp
 *
 *  Created on: 4 May, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include "Common.hpp"
#include "Types.hpp"
#

#pragma once

namespace sim_mob
{
	namespace long_term
	{
		class HousingInterestRate
		{
		public:
			HousingInterestRate(BigSerial id=0,
								int year = 0,
								int quarter = 0,
								std::string yq = "",
								float infl_tminus1 = 0,
								float infl_tplus1 = 0,
								float interest_rate = 0,
								float gdp_growth = 0,
								float rate_real = 0,
								std::string source = "");
			virtual ~HousingInterestRate();
			void setInterestRate( float val);
			void setId( BigSerial id);

			
			BigSerial getId() const;
			int getYear() const;
			int getQuarter() const;
			std::string getYq() const; 
			float getInfl_tminus1() const;
			float getInfl_tplus1() const;
			float getInterestRate() const;
			float getGdp_growth() const;
			float getRate_real() const;
			std::string getSource() const;

			HousingInterestRate& operator=(const HousingInterestRate& source);

            /**
             * Operator to print the HouseholdInterestRate data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const HousingInterestRate& data);


		private:
			friend class HousingInterestRateDao;

			BigSerial id;
			int year;
			int quarter;
			std::string yq; 
			float infl_tminus1;
			float infl_tplus1;
			float interest_rate;
			float gdp_growth;
			float rate_real;
			std::string source;

		};
	}
}
