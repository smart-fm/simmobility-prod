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
			Job(BigSerial id=INVALID_ID, BigSerial establishmentId=INVALID_ID);

			virtual ~Job();

			void setId(BigSerial val);
			void setEstablishmentId(BigSerial val);

			BigSerial getId() const;
			BigSerial getEstablishmentId() const;

		private:
			friend class JobDao;

			BigSerial id;
			BigSerial establishmentId;
		};
	}
}
