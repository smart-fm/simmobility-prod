/*
 * LogSumVehicleOwnership.hpp
 *
 *  Created on: May 21, 2015
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{

    namespace long_term
    {
    	class LogSumVehicleOwnership
		{
		public:
    		LogSumVehicleOwnership(BigSerial householdId = INVALID_ID, double avgLogsum = 0);
			virtual ~LogSumVehicleOwnership();

			 BigSerial getHouseholdId() const;
			 double getAvgLogsum() const;

			/**
			* Assign operator.
			* @param source to assign.
			* @return LogSumVehicleOwnership instance reference.
			*/
			LogSumVehicleOwnership& operator=(const LogSumVehicleOwnership& source);

			/**
			* Operator to print the TaxiAccessCoefficients data.
			*/
			friend std::ostream& operator<<(std::ostream& strm, const LogSumVehicleOwnership& data);

		private:
			friend class LogSumVehicleOwnershipDao;

			BigSerial householdId;
			double avgLogsum;
		};

    }
    }

