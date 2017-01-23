//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

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
			Establishment(	BigSerial 	id = 0,
							BigSerial 	buildingId = 0,
							BigSerial 	firmId = 0,
							BigSerial	firmFoundationYear = 0,
							int			industryTypeId = 0,
							double		floorArea = 0,
							BigSerial 	jobSize = 0,
							double 		revenue = 0,
							double		capital = 0,
							BigSerial	establishmentLifestyleId = 0);
			~Establishment();

			Establishment( const Establishment& source);

			Establishment& operator=(const Establishment& source);

			void setId(BigSerial val);
			void setBuildingId(BigSerial val);
			void setFirmId(BigSerial val);
			void setFirmFoundationYear(BigSerial val);
			void setIndustryTypeId(int val);
			void setFloorArea(double val);
			void setJobSize(BigSerial  val);
			void setRevenue(double val);
			void setEstablishmentLifestyleId(BigSerial val);

			BigSerial getId() const;
			BigSerial getBuildingId() const;
			BigSerial getFirmId() const;
			BigSerial getFirmFoundationYear() const;
			int		  getIndustryTypeId() const;
			double	  getFloorArea() const;
			BigSerial getJobSize() const;
			double	  getRevenue() const;
			BigSerial getEstablishmentLifestyleId() const;

		   /**
		    * Operator to print the Household data.
		   */
		   friend std::ostream& operator<<(std::ostream& strm, const Establishment& data);

		private:
		   friend class EstablishmentDao;

			BigSerial 	id;
			BigSerial 	buildingId;
			BigSerial 	firmId;
			BigSerial	firmFoundationYear;
			int			industryTypeId;
			double		floorArea;
			BigSerial 	jobSize;
			double 		revenue;
			double		capital;
			BigSerial	establishmentLifestyleId;

		};
	}
}

