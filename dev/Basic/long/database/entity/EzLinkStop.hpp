/*
 * EzLinkStop.hpp
 *
 *  Created on: 13 Mar 2018
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class EzLinkStop
		{
		public:
			EzLinkStop(	BigSerial id = 0, double xCoord = 0, double yCoord = 0, BigSerial nearestSchoolId = 0);
			~EzLinkStop();

			EzLinkStop( const EzLinkStop& source);

			EzLinkStop& operator=(const EzLinkStop& source);

			BigSerial getId() const;
			void setId(BigSerial id);
			BigSerial getNearestSchoolId() const;
			void setNearestSchoolId(BigSerial nearestSchoolId);
			double getXCoord() const;
			void setXCoord(double coord);
			double getYCoord() const;
			void setYCoord(double coord);

			BigSerial 	id;
			double xCoord;
			double yCoord;
			BigSerial nearestSchoolId;

		};
	}
}

