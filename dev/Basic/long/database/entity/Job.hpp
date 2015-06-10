/*
 * Job.hpp
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
		class Job
		{
		public:
			Job(BigSerial id=INVALID_ID, BigSerial establishmentId=INVALID_ID, BigSerial sectorId=INVALID_ID, int timeRestriction = 0,bool isStudent = false,
				bool fixedWorkplace =false);

			virtual ~Job();

			void setId(BigSerial val);
			void setEstablishmentId(BigSerial val);
			void setSectorId(BigSerial val);
			void setTimeRestriction(int val);
			void setIsStudent(bool val);
			void setFixedWorkplace(bool val);

			BigSerial getId() const;
			BigSerial getEstablishmentId() const;
			BigSerial getSectorId() const;
			int getTimeRestriction() const;
			bool getIsStudent() const;
			bool getFixedWorkplace() const;

		private:
			friend class JobDao;

			BigSerial id;
			BigSerial establishmentId;
			BigSerial sectorId;
			int timeRestriction;
			bool isStudent;
			bool fixedWorkplace;
		};
	}
}
