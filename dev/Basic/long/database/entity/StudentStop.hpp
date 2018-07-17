/*
 * StudentStop.hpp
 *
 *  Created on: 13 Mar 2018
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class StudentStop
		{
		public:
			StudentStop(BigSerial homeStopEzLinkId = 0, BigSerial schoolStopEzLinkId = 0);
			~StudentStop();

			StudentStop( const StudentStop& source);

			StudentStop& operator=(const StudentStop& source);

			BigSerial getHomeStopEzLinkId() const;
			void setHomeStopEzLinkId(BigSerial homeStopEzLinkId);
			BigSerial getSchoolStopEzLinkId() const;
			void setSchoolStopEzLinkId(BigSerial schoolStopEzLinkId);

			BigSerial homeStopEzLinkId;
			BigSerial schoolStopEzLinkId;

		};
	}
}

