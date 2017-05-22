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
    		IndvidualVehicleOwnershipLogsum(BigSerial householdId = INVALID_ID, BigSerial individualId = INVALID_ID, double logsum0 = 0, double logsum1 = 0, double logsum2 = 0, double logsum3 = 0, double logsum4 = 0, double logsum5 = 0);
			virtual ~IndvidualVehicleOwnershipLogsum();


			/**
			* Assign operator.
			* @param source to assign.
			* @return LogSumVehicleOwnership instance reference.
			*/
			 IndvidualVehicleOwnershipLogsum& operator=(const IndvidualVehicleOwnershipLogsum& source);

			/**
			* Operator to print the IndvidualVehicleOwnershipLogsum data.
			*/
			friend std::ostream& operator<<(std::ostream& strm, const IndvidualVehicleOwnershipLogsum& data);

			/*
			 * setters and getters
			 */
			BigSerial getHouseholdId() const;
			BigSerial getIndividualId() const;
			double getLogsum0() const;
			double getLogsum1() const;
			double getLogsum2() const;
			double getLogsum3() const;
			double getLogsum4() const;
			double getLogsum5() const;


			void setHouseholdId(BigSerial householdId);
			void setIndividualId(BigSerial individualId);
			void setLogsum0(double logsum0);
			void setLogsum1(double logsum1);
			void setLogsum2(double logsum2);
			void setLogsum3(double logsum3);
			void setLogsum4(double logsum4);
			void setLogsum5(double logsum5);

		private:
			friend class IndvidualVehicleOwnershipLogsumDao;

			BigSerial householdId;
			BigSerial individualId;
			double logsum0;
			double logsum1;
			double logsum2;
			double logsum3;
			double logsum4;
			double logsum5;
		};

    }
    }

