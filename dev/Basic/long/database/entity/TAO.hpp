/*
 * TAO.hpp
 *
 *  Created on: Jul 30, 2015
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{

    namespace long_term
    {
    	class TAO
		{
		public:
    		TAO(BigSerial id = 0, std::string quarter = std::string(), double condo = 0, double apartment = 0, double terrace = 0, double semi = 0, double detached = 0, double ec = 0);
			virtual ~TAO();

			double getApartment() const;
			double getCondo() const;
			double getDetached() const;
			double getEc() const;
			const std::string& getQuarter();
			double getSemi() const;
			double getTerrace() const;
			BigSerial getId() const;

			void setApartment(double apartment);
			void setCondo(double condo);
			void setDetached(double detached);
			void setEc(double ec);
			void setQuarter(const std::string& quarter);
			void setSemi(double semi);
			void setTerrace(double terrace);
			void setId(BigSerial id);
			/**
			* Assign operator.
			* @param source to assign.
			* @return TAO instance reference.
			*/
			TAO& operator=(const TAO& source);

			/**
			* Operator to print the TAO data.
			*/
			friend std::ostream& operator<<(std::ostream& strm, const TAO& data);

		private:
			friend class TAO_Dao;

			BigSerial id;
			std::string quarter;
			double condo;
			double apartment;
			double terrace;
			double semi;
			double detached;
			double ec;
		};

    }
    }
