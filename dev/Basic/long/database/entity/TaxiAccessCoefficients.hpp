/*
 * TaxiAccessCoefficients.hpp
 *
 *  Created on: Apr 13, 2015
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{

    namespace long_term
    {
    	class TaxiAccessCoefficients
		{
		public:
    		TaxiAccessCoefficients(BigSerial parameterId = INVALID_ID, double coefficientEstimate = 0);
			virtual ~TaxiAccessCoefficients();

			double getCoefficientEstimate() const;
			void setCoefficientEstimate(double coefficientEstimate);
			BigSerial getParameterId() const;
			void setParameterId(BigSerial parameterId);

			/**
			* Assign operator.
			* @param source to assign.
			* @return TaxiAccessCoefficients instance reference.
			*/
			TaxiAccessCoefficients& operator=(const TaxiAccessCoefficients& source);

			/**
			* Operator to print the TaxiAccessCoefficients data.
			*/
			friend std::ostream& operator<<(std::ostream& strm, const TaxiAccessCoefficients& data);

		private:
			friend class TaxiAccessCoefficientsDao;

			BigSerial parameterId;
			double coefficientEstimate;
		};

    }
    }
