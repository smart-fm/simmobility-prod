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
			Establishment(BigSerial id =INVALID_ID,
					BigSerial firmId =INVALID_ID,
					BigSerial buildingId=INVALID_ID,
					BigSerial lifestyleId=INVALID_ID,
					double revenue=0);
			~Establishment();

			Establishment( const Establishment& source);

			Establishment& operator=(const Establishment& source);

			void setId(BigSerial val);
			void setFirmId(BigSerial val);
			void setBuildingId(BigSerial val);
			void setLifestyleId(BigSerial val);
			void setRevenue(double val);

			BigSerial getId() const;
			BigSerial getFirmId() const;
			BigSerial getBuildingId() const;
			BigSerial getLifestyleId() const;
			double	  getRevenue() const;

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
			double 		revenue;
		};
	}
}

