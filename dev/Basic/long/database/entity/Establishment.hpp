/*
 * Establishment.hpp
 *
 *  Created on: 23 Apr, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class Establishment
		{
		public:
			Establishment(BigSerial id =0, BigSerial firmId =0, BigSerial buildingId=0, BigSerial lifestyleId=0, BigSerial businessTypeId=0, int size=0, double revenue=0,
						  double grossSqM=0, BigSerial slaAddressId =0);
			~Establishment();
			Establishment& operator=(const Establishment& source);

			void setId(BigSerial val);
			void setFirmId(BigSerial val);
			void setBuildingId(BigSerial val);
			void setLifestyleId(BigSerial val);
			void setBusinessTypeId(BigSerial val);
			void setSize(int val);
			void setRevenue(double val);
			void setGrossSqM(double val);
			void setSlaAddressId(BigSerial val);

			BigSerial getId() const;
			BigSerial getFirmId() const;
			BigSerial getBuildingId() const;
			BigSerial getLifestyleId() const;
			BigSerial getBusinessTypeId() const;
			int		  getSize() const;
			double	  getRevenue() const;
			double	  getGrossSqM() const;
			BigSerial getSlaAddressId() const;

		   /**
		    * Operator to print the Household data.
		   */
		   friend std::ostream& operator<<(std::ostream& strm, const Establishment& data);

		private:
		   friend class EstablishmentDao;

			BigSerial 	id;
			BigSerial 	firmId;
			BigSerial 	buildingId;
			BigSerial 	lifestyleId;
			BigSerial 	businessTypeId;
			int 		size;
			double 		revenue;
			double 		grossSqM;
			BigSerial 	slaAddressId;
		};
	}
}

