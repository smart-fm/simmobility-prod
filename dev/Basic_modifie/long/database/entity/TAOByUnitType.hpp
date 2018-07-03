/*
 * TAOByUnitType.hpp
 *
 *  Created on: 15 May 2017
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{

    namespace long_term
    {
    	class TAOByUnitType
		{
		public:
    		TAOByUnitType(BigSerial id = 0, std::string quarter = std::string(), double treasuryBillYield1Year = 0,double gdpRate = 0, double inflation = 0, double tCondo12 = 0 , double tCondo13 = 0,double tCondo14 = 0,double tCondo15 = 0,double tCondo16 = 0, double tApartment7 = 0,double tApartment8 = 0, double tApartment9 = 0, double tApartment10 = 0, double tApartment11 = 0);
			virtual ~TAOByUnitType();


			/*
			 * getters and setters for apartment and condo unit types
			 */
			double getTApartment7() const;
			double getTApartment8() const;
			double getTApartment9() const;
			double getTApartment10() const;
			double getTApartment11() const;
			double getTCondo12() const;
			double getTCondo13() const;
			double getTCondo14() const;
			double getTCondo15() const;
			double getTCondo16() const;
			const std::string& getQuarter() const;
			BigSerial getId() const;
			double getGdpRate() const;
			double getInflation() const;
			double getTreasuryBillYield1Year() const;


			void setTApartment7(double apartment7);
			void setTApartment8(double apartment8);
			void setTApartment9(double apartment9);
			void setTApartment10(double apartment10);
			void setTApartment11(double apartment11);
			void setTCondo12(double condo12);
			void setTCondo13(double condo13);
			void setTCondo14(double condo14);
			void setTCondo15(double condo15);
			void setTCondo16(double condo16);
			void setQuarter(const std::string& quarter);
			void setId(BigSerial id);
			void setGdpRate(double gdpRate);
			void setInflation(double inflation);
			void setTreasuryBillYield1Year(double treasuryBillYield1Year);

			TAOByUnitType& operator=(const TAOByUnitType& source);

		private:
			BigSerial id;
			std::string quarter;
			double treasuryBillYield1Year;
			double gdpRate;
			double inflation;
			double tCondo12;
			double tCondo13;
			double tCondo14;
			double tCondo15;
			double tCondo16;
			double tApartment7;
			double tApartment8;
			double tApartment9;
			double tApartment10;
			double tApartment11;
		};

    }
    }

