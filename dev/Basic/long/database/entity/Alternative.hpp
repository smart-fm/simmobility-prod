//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * Alternative.hpp
 *
 *  Created on: 31 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class Alternative
		{
		public:
			Alternative( BigSerial id = 0, BigSerial planAreaId =0, std::string planAreaName="", BigSerial dwellingTypeId = 0, std::string dwellingTypeName="",
					 	 double avgHouseholdSize = 0, double avgHouseholdIncome = 0, int unitTypeCounter = 0, int populationByUnitType = 0, double medianHedonicPrice = 0, double sumFloorArea = 0 );

			virtual ~Alternative();

			Alternative(const Alternative& source);
			Alternative& operator=(const Alternative& source);

			BigSerial getId() const;
			BigSerial getPlanAreaId() const;
			std::string getPlanAreaName() const;
			BigSerial getDwellingTypeId() const;
			std::string getDwellingTypeName() const;

			double getAvgHouseholdSize()const;
			double getAvgHouseholdIncome()const;
			int getUnitTypeCounter()const;
			int getPopulationByUnitType()const;

			int getMedianHedonicPrice() const;
			void setMedianHedonicPrice(double value);

			void setAvgHouseholdSize( double value );
			void setAvgHouseholdIncome( double value );
			void setUnitTypeCounter( int value );
			void setPopulationByUnitType( int value );

			double getSumFloorArea();
			void setSumFloorArea( double value);

			friend std::ostream& operator<<(std::ostream& strm, const Alternative& data);

		private:
			friend class AlternativeDao;

			BigSerial id;
			BigSerial planAreaId;
			std::string planAreaName;
			BigSerial dwellingTypeId;
			std::string dwellingTypeName;

			//The four variables below are computed at run time.
			double avgHouseholdSize;
			double avgHouseholdIncome;
			int unitTypeCounter;
			int populationByUnitType;
			double medianHedonicPrice;
			double sumFloorArea;

		};
	}
}
