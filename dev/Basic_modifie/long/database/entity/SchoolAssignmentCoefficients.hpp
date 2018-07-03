/*
 * SchoolAssignmentCoefficients.hpp
 *
 *  Created on: 9 Mar 2016
 *      Author: gishara
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
    	class SchoolAssignmentCoefficients
		{
		public:
    		SchoolAssignmentCoefficients(BigSerial parameterId = INVALID_ID, double coefficientEstimate = 0);
			virtual ~SchoolAssignmentCoefficients();

			double getCoefficientEstimate() const;
			void setCoefficientEstimate(double coefficientEstimate);
			BigSerial getParameterId() const;
			void setParameterId(BigSerial parameterId);

			/**
			* Assign operator.
			* @param source to assign.
			* @return Building instance reference.
			*/
			SchoolAssignmentCoefficients& operator=(const SchoolAssignmentCoefficients& source);

		private:
			friend class SchoolAssignmentCoefficientsDao;

			BigSerial parameterId;
			double coefficientEstimate;
		};

    }
    }
