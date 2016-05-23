/*
 * IndvidualVehicleOwnershipLogsum.hpp
 *
 *  Created on: Jan 20, 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{

    namespace long_term
    {
    	class IndvidualVehicleOwnershipLogsum
		{
		public:
    		IndvidualVehicleOwnershipLogsum(BigSerial householdId = INVALID_ID, BigSerial individualId = INVALID_ID, double logsumTransit = 0, double logsumCar = 0);
			virtual ~IndvidualVehicleOwnershipLogsum();


			/**
			* Assign operator.
			* @param source to assign.
			* @return LogSumVehicleOwnership instance reference.
			*/
			 IndvidualVehicleOwnershipLogsum& operator=(const IndvidualVehicleOwnershipLogsum& source);

			/**
			* Operator to print the TaxiAccessCoefficients data.
			*/
			friend std::ostream& operator<<(std::ostream& strm, const IndvidualVehicleOwnershipLogsum& data);

			/*
			 * setters and getters
			 */
			BigSerial getHouseholdId() const;
			BigSerial getIndividualId() const;
			double getLogsumCar() const;
			double getLogsumTransit() const ;

			void setHouseholdId(BigSerial householdId);
			void setIndividualId(BigSerial individualId);
			void setLogsumCar(double logsumCar);
			void setLogsumTransit(double logsumTransit);

		private:
			friend class IndvidualVehicleOwnershipLogsumDao;

			BigSerial householdId;
			BigSerial individualId;
			double logsumTransit;
			double logsumCar;
		};

    }
    }

