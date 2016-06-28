//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * ROILimits.hpp
 *
 *  Created on: 17 May 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class ROILimits
		{
		public:
			ROILimits(BigSerial buildingTypeId = 0, double roiLimit = .0 );
			virtual ~ROILimits();

			ROILimits(const ROILimits& source);
			ROILimits& operator=(const ROILimits& source);

			friend std::ostream& operator<<(std::ostream& strm, const ROILimits& data);

			BigSerial getBuildingTypeId() const;
			double getRoiLimit() const;

			void setBuildingTypeId(BigSerial developmentTypeId);
			void setRoiLimit(double roiLimit);

		private:
			friend class ROILimitsDao;

			BigSerial buildingTypeId;
			double roiLimit;
		};
	}

}

