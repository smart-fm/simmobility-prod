/*
 * VehicleOwnershipCoefficients.hpp
 *
 *  Created on: Feb 24, 2015
 *      Author: gishara
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
    	class VehicleOwnershipCoefficients
		{
		public:
    		VehicleOwnershipCoefficients(BigSerial parameterId = INVALID_ID, double coefficientEstimate = 0);
			virtual ~VehicleOwnershipCoefficients();

			double getCoefficientEstimate() const;
			void setCoefficientEstimate(double coefficientEstimate);
			BigSerial getParameterId() const;
			void setParameterId(BigSerial parameterId);

			/**
			* Assign operator.
			* @param source to assign.
			* @return Building instance reference.
			*/
			VehicleOwnershipCoefficients& operator=(const VehicleOwnershipCoefficients& source);

			/**
			* Operator to print the VehicleOwnershipCoefficients data.
			*/
			friend std::ostream& operator<<(std::ostream& strm, const VehicleOwnershipCoefficients& data);

		private:
			friend class VehicleOwnershipCoefficientsDao;

			BigSerial parameterId;
			double coefficientEstimate;
		};

    }
    }
